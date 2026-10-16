"""Recover retail half-time, full-time score calls and drawn-match endings."""
from pathlib import Path
from collections import Counter
import argparse, hashlib, json, re, struct, subprocess
from import_euro96 import ROOT
from import_euro96_presentation import EXE_HASH
from inspect_cd_audio import inspect_mad


def recover(iso):
    inventory = json.loads((ROOT/'docs/euro96-cd-audio-inspection.json').read_text())
    hashes = {row['path']: row['sha256'] for row in inventory['audio']}
    hashes['EURO96.EXE'] = EXE_HASH

    def extract(name):
        data = subprocess.run(['7z', 'e', '-so', str(iso), name], capture_output=True, check=True).stdout
        if hashlib.sha256(data).hexdigest() != hashes[name]:
            raise ValueError('Unverified retail input: '+name)
        return data

    exe = extract('EURO96.EXE')
    evidence = []
    # Calls after the 45- and ordinary 90-minute whistles pass selectors 3/2
    # to the score dispatcher. Equal full-time scores dispatch event 101 (END).
    for at, raw in [(0xA71A0, 'b803000000'), (0xA71A5, 'e80acd0500'),
                    (0xA7327, 'b802000000'), (0xA732C, 'e883cb0500'),
                    (0x103F06, '83fd02'), (0x103F0F, '39c8'),
                    (0x103F6F, 'b865000000'), (0x103F74, 'e83beeFFFF'),
                    (0x104067, '83f807'), (0x104070, '83f905'),
                    (0x104097, 'b804000000'), (0x1040A5, '0441'),
                    (0x1040C3, '8b04ad702d0100'),
                    (0xA7269, 'b101880da5b60400'), (0xA728C, 'b802000000'),
                    (0xA7291, 'e81ecc0500'), (0x103F24, '803da5b6040000'),
                    (0x103F2D, '83f808'), (0x103F32, '40'),
                    (0x103F34, '68a85d0000'), (0x103F48, 'b818000000'),
                    (0x103F57, '83c009'), (0x103F88, 'b8b25d0000')]:
        expected = bytes.fromhex(raw)
        if exe[at:at+len(expected)] != expected:
            raise ValueError(f'Retail period instruction changed at {at:#x}: {exe[at:at+len(expected)].hex()}')
        evidence.append(dict(file_offset=at, bytes=expected.hex()))
    data_base = 0x12F854
    if exe[data_base+0x5DA8:data_base+0x5DB6] != b'%.2d\0%.2d\0XTM\0':
        raise ValueError('Unknown extra-time filename formatting')
    banks = [exe[data_base+p:].split(b'\0', 1)[0].decode('ascii')
             for p in struct.unpack_from('<7I', exe, data_base+0x12D70)]
    if banks != ['SCO', 'LAT', 'FLL', 'HLF', 'MCX', 'STT', 'XTM']:
        raise ValueError('Unknown period bank table')
    if exe[data_base+0x12D16+6:data_base+0x12D16+12] != b'END\0\x0d\0':
        raise ValueError('Unknown event 101 END descriptor')

    records, imported, excluded, coverage = {}, [], [], {}
    for category, bank in [(96, 'HLF'), (97, 'FLL'), (98, 'END'), (104, 'XTM')]:
        data = extract(f'DIGI/PHRASES/E/E_{bank}.MAD')
        entries = inspect_mad(data); counts = Counter(e['name'].lower() for e in entries)
        for entry in entries:
            name = entry['name'].lower()
            if counts[name] != 1:
                excluded.append(dict(bank=bank, filename=name, reason='Ambiguous duplicate filename'))
                continue
            match = re.fullmatch('e_'+bank.lower()+r'([0-7])([0-5])([a-d])\.wav', name)
            if category in (98, 104):
                match = re.fullmatch('e_'+bank.lower()+r'(\d{2})\.wav', name)
                if not match or not 1 <= int(match[1]) <= (13 if category==98 else 32): raise ValueError('Unexpected '+bank+' clip')
                number = int(match[1])
            else:
                if not match or int(match[1]) < int(match[2]): raise ValueError('Unexpected period score: '+name)
                number = 10*int(match[1])+int(match[2])+100*(ord(match[3])-97)
            if (entry['rate'], entry['channels'], entry['sample_width']) != (11025, 1, 1):
                raise ValueError('Unsupported period PCM')
            wav = data[entry['offset']:entry['offset']+entry['size']]
            records[category, number] = wav
            imported.append(dict(category=category, number=number, bank=bank, filename=name, sha256=entry['sha256']))
        coverage[bank] = sorted({n % 100 for c, n in records if c == category})
    result = b'AC96PER1'+struct.pack('<I', len(records))+b''.join(
        struct.pack('<HHI', c, n, len(wav))+wav for (c, n), wav in sorted(records.items()))
    target = ROOT/'local-data/COMMENTARY/EURO_PERIODS.BIN'
    if target.exists() and target.read_bytes() != result and hashlib.sha256(target.read_bytes()).hexdigest() != '0457e4a1e0af62df089e2c76646bd1ea761bb6954933ba046f1f9a764365e7d5': raise ValueError('Different period overlay exists')
    target.write_bytes(result)
    report = dict(executable_sha256=EXE_HASH, instruction_evidence=evidence, banks=banks,
                  clips=imported, excluded=excluded, coverage=coverage,
                  routing='HLF: first interval; FLL: decisive regulation result; END: drawn regulation result; XTM: tied 90-minute knockout interval, score 0..8 selects clip 1..9, larger draws select available original clips in 9..32. Missing XTM keeps the whistle-only transition; missing period scores retain compatibility calls. Winner sequences are supplied separately.',
                  outputs={'COMMENTARY/EURO_PERIODS.BIN': dict(bytes=len(result), sha256=hashlib.sha256(result).hexdigest())})
    (ROOT/'docs/euro96-periods-import.json').write_text(json.dumps(report, indent=2)+'\n')
    print(f'Imported {len(records)} period recordings; coverage: {coverage}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__); parser.add_argument('iso', type=Path)
    recover(parser.parse_args().iso.resolve(strict=True))
