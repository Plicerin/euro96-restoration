"""Import the verified PC Euro96 revision as optional media overlays, never SOURCE."""
from pathlib import Path
import argparse
import hashlib
import json
import re
import struct
import subprocess
from collections import defaultdict
from import_cd_graphics import ROOT, records
from inspect_cd_audio import inspect_mad
from preview_identity import player, match

EXPECTED = {
    'EUROREND.DAT': 'f580229f73fe512a4519707f5b2500c58a28c0cacc7402e5ac0db8f403096cde',
    'EUROREND.OFF': '70164cc092c0503f92a49762158deef4f4058cf4c665914ecbc61596c2e5ef1f',
    'SCRIPT96.ENG': '15ff7b749c0fbf5431c5f0964643918adb458bf6cc41a3d71f059f30fc52d283',
}
CODES = 'BUL CRO CZC DMK ENG FRN GER HOL ITA POR ROM RUS SCT SPN SWZ TRK'.split()

def unmangle(data, limit=2*1024*1024):
    """SOURCE/UNMANGLE.C token operations, with bounded input/backreferences/output."""
    at, out = 0, bytearray()
    def take(n):
        nonlocal at
        if n > len(data)-at:
            raise ValueError('Truncated mangled input')
        b = data[at:at+n]; at += n
        return b
    while True:
        c = take(1)[0]
        if not c:
            if at != len(data):
                raise ValueError('Trailing mangled input')
            return bytes(out)
        if c & 128:
            if c & 64:
                if c & 32:
                    offset = ((c & 31) << 8) + take(1)[0] + 3
                    length = take(1)[0] + 5
                else:
                    offset = ((c & 3) << 8) + take(1)[0] + 3
                    length = ((c >> 2) & 7) + 4
            else:
                offset, length = (c & 63) + 3, 3
            if offset > len(out) or length > limit-len(out):
                raise ValueError('Invalid mangled backreference/output size')
            for _ in range(length):
                out.append(out[-offset])
        elif c & 64:
            width = 2 if c & 16 else 1
            length = (c & 15) + (2 if width == 2 else 3)
            if len(out) < width*(1 if c & 32 else 2) or width*length > limit-len(out):
                raise ValueError('Invalid mangled sequence/output size')
            value = int.from_bytes(out[-width:], 'little')
            diff = 0 if c & 32 else value-int.from_bytes(out[-2*width:-width], 'little')
            for _ in range(length):
                value = (value+diff) % (1 << (8*width))
                out.extend(value.to_bytes(width, 'little'))
        else:
            if c > limit-len(out):
                raise ValueError('Mangled output exceeds limit')
            out.extend(take(c))

def norm(name):
    # Deliberately conservative: do not invent aliases for changed squads.
    return re.sub('[^A-Z]', '', name.upper())

def read_names(data):
    if data[:8] != b'AC96COM1':
        raise ValueError('Unrecognized base names pack')
    count, = struct.unpack_from('<I', data, 8); at = 12; result = {}
    for _ in range(count):
        category, tone, n = struct.unpack_from('<HHI', data, at); at += 8
        if not 128 <= category < 448 or not 1 <= tone <= 3 or n > len(data)-at or (category,tone) in result:
            raise ValueError('Invalid base names record')
        result[category,tone] = data[at:at+n]; at += n
    if at != len(data):
        raise ValueError('Trailing base names data')
    return result

def recover(iso):
    def extract(name):
        return subprocess.run(['7z','e','-so',str(iso),name], capture_output=True, check=True).stdout
    inputs = {name: extract(name) for name in EXPECTED}
    for name, digest in EXPECTED.items():
        if hashlib.sha256(inputs[name]).hexdigest() != digest:
            raise ValueError('Unsupported Euro96 revision: '+name)
    cd = records(inputs['EUROREND.DAT'], inputs['EUROREND.OFF'])
    source = records((ROOT/'SOURCE/EUROREND.DAT').read_bytes(), (ROOT/'SOURCE/EUROREND.OFF').read_bytes())
    # All outfield torso/limb UVs are identical, including the extended skin pages
    # (shifted nine entries in retail). Heads and keeper pages are NOT interchangeable.
    for old, new, count in [(120,120,236),(584,593,120),(704,713,56)]:
        if source[1][old*32:(old+count)*32] != cd[1][new*32:(new+count)*32]:
            raise ValueError('Unsupported outfield atlas coordinates')
    if len(cd) != 287 or len(cd[0]) != 768 or len(cd[35]) != 19968 or source[109] != cd[107]:
        raise ValueError('Unexpected Euro96 resource layout')
    for i in range(32):
        if len(cd[3+i]) != 65536 or len(cd[61+i]) != 96 or max(cd[61+i]) > 63:
            raise ValueError('Invalid retail kit')
    text = unmangle(inputs['SCRIPT96.ENG']).decode('cp850')
    original = (ROOT/'SOURCE/SCRIPT.96').read_text(encoding='cp850')
    roster = [p for line in original[original.index('PLYR_NMES'):original.index('TEAM_NMES')].splitlines() if (p := player(line))]
    if len(roster) != 320:
        raise ValueError('Unrecognized preserved roster')
    # Main and explicitly authored reserve entries can both establish identity.
    retail = []
    for line in text.splitlines():
        p = player(line, retail=True)
        skin = re.search(r'\[(WHT|MED|BLK)\]', line)
        if p and skin and p['code'][:3] in CODES:
            p['skin'] = skin[1]
            retail.append(p)
    choices = bytes(int(v)-1 for v in re.findall(r'\[kit([12])\]',text[text.index('[ Kit Xref Table ]'):]))
    if len(choices) != 256:
        raise ValueError('Invalid retail kit clash table')
    base = (ROOT/'local-data/COMMENTARY/NAMES.BIN').read_bytes()
    base_hash = json.loads((ROOT/'docs/cd-names-recovery.json').read_text())['pack_sha256']
    if hashlib.sha256(base).hexdigest() != base_hash:
        raise ValueError('Unverified base player names')
    clips = read_names(base)
    appearance = bytearray([255]*320); matches = []; missing = []; banks = []; cache = {}
    inventory = json.loads((ROOT/'docs/euro96-cd-audio-inspection.json').read_text())
    verified_banks = {b['path']: b['sha256'] for b in inventory['audio']}
    for team, code in enumerate(CODES):
        path = f'DIGI/TEAMS/E/E_{code}.MAD'; data = extract(path)
        if hashlib.sha256(data).hexdigest() != verified_banks[path]:
            raise ValueError('Unsupported Euro96 voice bank: '+path)
        entries = inspect_mad(data)
        bank = defaultdict(list)
        for e in entries:
            bank[e['name'].lower()].append(e)
        cache[code] = data, bank
        banks.append(dict(path=path,sha256=hashlib.sha256(data).hexdigest()))
    for index,source_player in enumerate(roster):
        team,slot = divmod(index,20)
        name = source_player['name']
        identity, method = match(source_player, retail)
        if identity is None:
            missing.append(dict(team=team,shirt=slot+1,name=name,reason='No unique verified same-country identity'))
            continue
        retail_name,skin,digi = identity['name'],identity['skin'],identity['code']
        # Duplicate authored identities must also agree on appearance.
        if any(r['code']==digi and r['name']==retail_name and r['skin']!=skin for r in retail):
            raise ValueError('Conflicting retail appearance: '+digi)
        appearance[index] = int(skin == 'BLK')
        data,bank = cache[digi[:3]]
        row = dict(team=team,shirt=slot+1,name=name,retail_name=retail_name,skin=skin,digi=digi,match_method=method,source_identity=source_player,retail_identity=identity,clips=[])
        for tone,suffix in enumerate('xyz',1):
            filename = 'e_'+digi.lower()+suffix+'.wav'
            found = bank.get(filename,[])
            if len(found) != 1:
                continue
            e = found[0]
            if (e['rate'],e['channels'],e['sample_width']) != (11025,1,1):
                raise ValueError('Unsupported Euro96 player PCM')
            wav = data[e['offset']:e['offset']+e['size']]
            clips[128+index,tone] = wav
            row['clips'].append(dict(tone=tone,file=filename,sha256=hashlib.sha256(wav).hexdigest()))
        matches.append(row)
    kit = b'EU96KIT1'+cd[0]+cd[35]+b''.join(cd[3+i]+cd[61+i] for i in range(32))+choices+appearance
    names = b'AC96COM1'+struct.pack('<I',len(clips))+b''.join(struct.pack('<HHI',c,t,len(w))+w for (c,t),w in sorted(clips.items()))
    outputs = {'GRAPHICS/EURO96.BIN':kit,'COMMENTARY/EURO_NAMES.BIN':names}
    # Only deterministic media derived from this pinned revision may replace an overlay.
    legacy = {'GRAPHICS/EURO96.BIN':'4fbe131fe704be40b16981fe78e39f911e761e8de8fc79e4f51333639359dc93',
              'COMMENTARY/EURO_NAMES.BIN':'4153cf75917928122950ed5fb3decf14139e9c64dcb004d4a4c69dfeb9a37cb5'}
    for path,data in outputs.items():
        target = ROOT/'local-data'/path
        if target.exists() and target.read_bytes() != data and hashlib.sha256(target.read_bytes()).hexdigest() != legacy[path]:
            raise ValueError('Different Euro96 overlay exists: '+path)
    for path,data in outputs.items():
        target = ROOT/'local-data'/path; target.parent.mkdir(parents=True,exist_ok=True); target.write_bytes(data)
    report = dict(inputs=EXPECTED,banks=banks,kit_order=CODES,home_records=list(range(3,19)),alternate_records=list(range(19,35)),palette_records=list(range(61,93)),verified_outfield_uv_records=412,clash_table=list(choices),matches=matches,unmatched=missing,appearance_matches=len(matches),retail_name_players=sum(len(r['clips'])==3 for r in matches),total_name_players=len(clips)//3,outputs={p:dict(bytes=len(b),sha256=hashlib.sha256(b).hexdigest()) for p,b in outputs.items()},adaptation='Retail uniform pixels and colours on verified original UVs. All 320 identities resolved by normalized names or independent metadata. WHT/MED select the existing default appearance; BLK selects the existing alternate appearance. Separate retail packs supply head/keeper/number artwork. No roster, statistics or save changes.')
    (ROOT/'docs/euro96-import.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Imported 32 kits, {len(matches)} matched appearances, {report['retail_name_players']} retail name sets; {len(clips)//3} total voiced players.")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('iso',type=Path)
    recover(parser.parse_args().iso.resolve(strict=True))
