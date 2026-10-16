"""Import verified retail pitch audio, mapped event phrases and score variants.

Preserved SOURCE event IDs stay stable. Retail IDs are deliberately not copied:
the retail category table inserts/removes events relative to this source revision.
"""
from pathlib import Path
from collections import Counter
import argparse, hashlib, json, re, struct, subprocess
from import_euro96 import ROOT
from import_euro96_presentation import EXE_HASH
from inspect_cd_audio import inspect_mad, original_pitch_names
from restore_commentary import recover as commentary_mapping

TABLE = 0x1424B0
DATA = 0x12F854
PREVIOUS_EVENTS_HASH = '4e2ff81fe6a5fd890cc13dc0913d11f069244e48d50c5b758e2bcf6af00dc4a0'


def source_pass_calls():
    """Count direct symbolic callers, excluding comments and string literals."""
    events = ('CP_SHORT', 'CP_SETPIECE', 'FP_SHORT', 'FP_LOOSE')
    result = {event: [] for event in events}
    tokens = re.compile(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')
    for path in sorted((ROOT/'SOURCE').iterdir()):
        if path.suffix.upper() not in ('.CPP', '.C', '.H'): continue
        source = path.read_text(encoding='latin1')
        active = tokens.sub(lambda m: '\n'*m[0].count('\n')+' ', source)
        for event in events:
            for call in re.finditer(r'\bPlayCommentaryMessage\s*\(\s*'+event+r'\s*\)', active):
                result[event].append(dict(file=path.name, line=active.count('\n', 0, call.start())+1))
    if not result['CP_SHORT'] or not result['CP_SETPIECE'] or result['FP_SHORT'] or result['FP_LOOSE']:
        raise ValueError('Source pass commentary callers changed; re-audit dormant categories')
    return result


def recover(iso):
    inventory = json.loads((ROOT/'docs/euro96-cd-audio-inspection.json').read_text())
    hashes = {row['path']: row['sha256'] for row in inventory['audio']}
    hashes['EURO96.EXE'] = EXE_HASH
    cache = {}

    def extract(name):
        if name not in cache:
            data = subprocess.run(['7z', 'e', '-so', str(iso), name], capture_output=True, check=True).stdout
            if hashlib.sha256(data).hexdigest() != hashes[name]:
                raise ValueError('Unverified retail input: '+name)
            cache[name] = data
        return cache[name]

    exe = extract('EURO96.EXE')
    # 46 six-byte descriptors. The retail loader indexes (event-1)*6 and
    # chooses numbered WAVs in [1,count]. One renamed bank is proven below.
    table = {}
    for i in range(46):
        bank, count = struct.unpack_from('<4sH', exe, TABLE+i*6)
        bank = bank.rstrip(b'\0').decode('ascii')
        if not re.fullmatch('[A-Z]{3}', bank) or bank in table or not 1 <= count <= 99:
            raise ValueError('Unknown retail commentary descriptor')
        table[bank] = dict(retail_id=i+1, count=count)
    evidence = []
    # File offsets, with exact instruction anchors for category selection and
    # score bank/decimal digits/A-D suffix construction, not guessed filenames.
    for at, raw in [(0x102E6C, '8d56ff'), (0x102E97, '0fbf82602c0100'),
                    (0x102E9E, '81c25c2c0100'), (0x102F15, '40'),
                    (0x104067, '83f807'), (0x104070, '83f905'),
                    (0x104097, 'b804000000'), (0x1040A5, '0441'),
                    (0x1040C3, '8b04ad702d0100')]:
        if exe[at:at+len(bytes.fromhex(raw))] != bytes.fromhex(raw):
            raise ValueError('Retail audio instruction changed at '+hex(at))
        evidence.append(dict(file_offset=at, bytes=raw))
    score_banks = [exe[DATA+p:].split(b'\0', 1)[0].decode('ascii')
                   for p in struct.unpack_from('<7I', exe, DATA+0x12D70)]
    if score_banks != ['SCO', 'LAT', 'FLL', 'HLF', 'MCX', 'STT', 'XTM']:
        raise ValueError('Unknown score/period bank table')

    # Both goal branches test the same rebound flag, select event 22, and
    # reach PlayCommentaryMessage. POS is not inferred from its filename.
    rebound_evidence = []
    for side, anchors in [('right', [(0xB7BF7, '803dd8a6070000'), (0xB7BFE, '741c'),
                                    (0xB7C12, 'b816000000'), (0xB7C17, 'e990000000'), (0xB7CAC, 'e803b10400')]),
                          ('left', [(0xB8050, '803dd8a6070000'), (0xB8057, '741c'),
                                   (0xB806B, 'b816000000'), (0xB8070, 'e96e000000'), (0xB80E3, 'e8ccac0400')])]:
        for at, raw in anchors:
            if exe[at:at+len(bytes.fromhex(raw))] != bytes.fromhex(raw):
                raise ValueError('Retail rebound-goal instruction changed at '+hex(at))
            rebound_evidence.append(dict(side=side, file_offset=at, bytes=raw))
    if table['POS'] != dict(retail_id=22, count=8):
        raise ValueError('Retail rebound-goal descriptor changed')
    pass_calls = source_pass_calls()

    base = (ROOT/'local-data/COMMENTARY/EURO_ENGLISH.BIN').read_bytes()
    base_hash = json.loads((ROOT/'docs/euro96-presentation-import.json').read_text())['outputs']['COMMENTARY/EURO_ENGLISH.BIN']['sha256']
    if hashlib.sha256(base).hexdigest() != base_hash:
        raise ValueError('Unverified compatibility commentary')
    clips = {}; at = 12
    if base[:8] != b'AC96COM1': raise ValueError('Unsupported compatibility pack')
    for _ in range(struct.unpack_from('<I', base, 8)[0]):
        category, number, length = struct.unpack_from('<HHI', base, at);at += 8
        if not length or length > len(base)-at or (category, number) in clips:
            raise ValueError('Invalid compatibility pack record')
        clips[category, number] = base[at:at+length];at += length
    if at != len(base): raise ValueError('Trailing compatibility pack data')
    mapped, retained, imported, excluded = [], [], [], []

    def entries(bank):
        name = f'DIGI/PHRASES/E/E_{bank}.MAD'
        data = extract(name)
        return data, inspect_mad(data)

    def add(category, number, data, entry, bank):
        if (entry['rate'], entry['channels'], entry['sample_width']) != (11025, 1, 1):
            raise ValueError('Unsupported retail PCM')
        wav = data[entry['offset']:entry['offset']+entry['size']]
        clips[category, number] = wav
        imported.append(dict(category=category, number=number, bank=bank,
                             filename=entry['name'], sha256=entry['sha256']))

    for row in commentary_mapping(ROOT/'SOURCE')['categories']:
        category, bank = row['id'], row['bank']
        source_bank = bank
        if category == 27:
            if (row['event'], bank) != ('GL_POST', 'GLF'):
                raise ValueError('Source rebound-goal category changed')
            bank = 'POS'
        if category > 35 or bank not in table:
            retained.append(dict(category=category, event=row['event'], bank=bank,
                                 direct_source_callers=pass_calls.get(row['event']),
                                 reason='Period dispatch differs' if category > 35 else
                                 'No active direct source caller; retained for compatibility' if category in (20, 21) else
                                 'Bank absent from retail category table'))
            continue
        data, rows = entries(bank)
        counts = Counter(e['name'].lower() for e in rows)
        for key in list(clips):
            if key[0] == category: del clips[key]
        used = []
        for e in rows:
            name = e['name'].lower()
            match = re.fullmatch(f'e_{bank.lower()}'+r'(\d{2})\.wav', name)
            if counts[name] != 1:
                excluded.append(dict(bank=bank, filename=name, reason='Ambiguous duplicate filename'))
                continue
            if not match or not 1 <= int(match[1]) <= table[bank]['count']:
                excluded.append(dict(bank=bank, filename=name, reason='Outside executable numbered range'))
                continue
            number = int(match[1]);add(category, number, data, e, bank);used.append(number)
        if not used: raise ValueError('No verified clips for '+bank)
        mapped.append(dict(source_id=category, event=row['event'], source_bank=source_bank, bank=bank, **table[bank], imported_numbers=used))

    for key in list(clips):
        if key[0] == 44: del clips[key]
    data, rows = entries('SCO')
    counts = Counter(e['name'].lower() for e in rows)
    for e in rows:
        name = e['name'].lower()
        match = re.fullmatch(r'e_sco([0-7])([0-5])([a-d])\.wav', name)
        if not match or int(match[1]) < int(match[2]) or counts[name] != 1:
            raise ValueError('Unexpected retail score recording: '+name)
        # COM2 adds 100*variant to the existing two decimal score digits.
        add(44, 10*int(match[1])+int(match[2])+100*(ord(match[3])-ord('a')), data, e, 'SCO')
    if sum(c == 44 for c, n in clips) != 132: raise ValueError('Incomplete score variants')
    voice = b'AC96COM2'+struct.pack('<I', len(clips))+b''.join(
        struct.pack('<HHI', c, n, len(wav))+wav for (c, n), wav in sorted(clips.items()))

    pitch = extract('DIGI/PITCH/PITCH.MAD')
    pitch_entries = inspect_mad(pitch)
    names = original_pitch_names(ROOT/'SOURCE')
    if [e['name'].upper() for e in pitch_entries] != names:
        raise ValueError('Pitch sample identities/order differ')
    original = {e['name'].upper(): e['sha256'] for e in inspect_mad((ROOT/'local-data/DIGI/PITCH/PITCH.MAD').read_bytes())}
    if any(original[e['name'].upper()] != e['sha256'] for e in pitch_entries):
        raise ValueError('Pitch samples are no longer byte-identical')
    mappings = json.loads((ROOT/'docs/audio-recovery.json').read_text())['tables']
    for table_name, at in [('PitchBank', 0x1422E0), ('CrowdBank', 0x142410)]:
        for i, row in enumerate(mappings[table_name]):
            ptr, amp, freq, flags = struct.unpack_from('<Ih2xiH2x', exe, at+16*i)
            expected_amp = 25600 if table_name == 'CrowdBank' and i == 9 else row['amp']
            if (ptr, amp, freq, flags) != (0x1293C+16*row['sample'], expected_amp, row['freq'], row['flags']):
                raise ValueError('Unexpected retail pitch/crowd event mapping')

    outputs = {'COMMENTARY/EURO_EVENTS.BIN': voice, 'DIGI/PITCH/EURO_PITCH.MAD': pitch}
    for name, data in outputs.items():
        target = ROOT/'local-data'/name
        if target.exists() and target.read_bytes() != data:
            if name != 'COMMENTARY/EURO_EVENTS.BIN' or hashlib.sha256(target.read_bytes()).hexdigest() != PREVIOUS_EVENTS_HASH:
                raise ValueError('Different audio overlay exists: '+name)
    for name, data in outputs.items():
        target = ROOT/'local-data'/name;target.parent.mkdir(parents=True, exist_ok=True);target.write_bytes(data)
    report = dict(executable_sha256=EXE_HASH, category_table_file_offset=TABLE, score_banks=score_banks,
                  instruction_evidence=evidence, mapped=mapped, retained=retained, clips=imported, excluded=excluded,
                  rebound_goal_evidence=rebound_evidence, source_pass_callers=pass_calls,
                  score_variants=132, pitch_samples=20, pitch_samples_identical=True,
                  crowd_foul_amplitude=dict(source=26624, retail=25600, file_offset=0x1424A4),
                  compatibility='Source-only pass clips remain in this compatibility overlay. From 0.69.0 the retail audio adapter routes CP_SHORT through CPA and suppresses CP_SETPIECE, as verified in retail-pass-audio-evidence.json; FP_SHORT/FP_LOOSE have no active direct source callers. Separate sequence/period packs override verified welcome and period paths. Ambiguous half-time scores and optional French retain compatibility audio.',
                  outputs={name: dict(bytes=len(data), sha256=hashlib.sha256(data).hexdigest()) for name, data in outputs.items()})
    (ROOT/'docs/euro96-audio-import.json').write_text(json.dumps(report, indent=2)+'\n')
    print(f'Imported {len(mapped)} mapped event categories, 132 score variants and 20 pitch samples; {len(clips)} total commentary clips.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__);parser.add_argument('iso', type=Path)
    recover(parser.parse_args().iso.resolve(strict=True))
