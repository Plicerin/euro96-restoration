"""Verify the authored venue schedule against the supplied retail executable.

This is an offline provenance check, not a build dependency on copyrighted media.
Addresses are file offsets; retail data-relative addresses use base 0x12f854.
"""
from pathlib import Path
import argparse
import hashlib
import json
import re

EXE_HASH = '2a473a002a294c1bb28ea39e94a36afc34497dd7e44f0ebfeda0ea21f652b62d'
CITIES = ['London', 'Birmingham', 'Leeds', 'Newcastle', 'Manchester', 'Liverpool', 'Sheffield', 'Nottingham']
NAMES = ['Wembley', 'Villa Park', 'Elland Road', 'St James Park', 'Old Trafford', 'Anfield', 'Hillsborough', 'City Ground']
SCHEDULE = [0, 2, 4, 6, 1, 3, 5, 7, 3, 1, 7, 5, 0, 2, 4, 6, 3, 2, 0, 1, 7, 6, 5, 4, 0, 5, 4, 1, 4, 0, 0]

def verify(executable, source):
    exe = executable.read_bytes()
    if hashlib.sha256(exe).hexdigest() != EXE_HASH:
        raise ValueError('Unverified Euro96 executable')
    raw = source.read_bytes()
    block = re.search(r'char\s+MatchVenues\[32\]\s*=\s*\{(.*?)\};', raw.decode('latin1'), re.S).group(1)
    names = [s.strip() for s in re.sub(r'//[^\n]*', '', block).split(',') if s.strip()]
    if [CITIES.index(s) for s in names] != SCHEDULE:
        raise ValueError('Source venue schedule differs')
    if exe[0x139a98:0x139ab8] != bytes(SCHEDULE + [0]):
        raise ValueError('Retail venue schedule differs')
    labels = [f'{city.upper()} {name.upper()}' for city, name in zip(CITIES, NAMES)]
    label_bytes = b''.join(s.encode() + b'\0' for s in labels)
    if exe[0x13347d:0x13347d + len(label_bytes)] != label_bytes:
        raise ValueError('Retail venue ID order differs')
    anchors = []
    for address, expected, meaning in [
        (0x9904f, '0fb6058fb20400', 'Read current fixture byte at data+0x4b28f'),
        (0x99056, '660fb68044a20000', 'Read MatchVenues[fixture] at data+0xa244'),
        (0x9905e, '66a3f4680800', 'Store venue into setup word at data+0x868f4'),
        (0xe8917, '668b15f4680800', 'Read that setup venue for stadium dispatch'),
        (0xe891e, '6683fa07', 'Dispatch accepts retail venues 0-7'),
    ]:
        value = bytes.fromhex(expected)
        if exe[address:address + len(value)] != value:
            raise ValueError(f'Retail instruction differs at {address:#x}')
        anchors.append(dict(file_offset=hex(address), bytes=expected, meaning=meaning))
    return dict(executable_sha256=EXE_HASH, source_sha256=hashlib.sha256(raw).hexdigest(),
                schedule_file_offset='0x139a98', schedule_data_offset='0xa244',
                venue_labels_file_offset='0x13347d', anchors=anchors,
                fixtures=[dict(fixture=i+1, retail_id=v, native_id=v+5, city=CITIES[v], stadium=NAMES[v]) for i, v in enumerate(SCHEDULE)],
                compatibility='New complete-retail-media championships use this schedule; versions 1-3 remain fixed-venue. Native IDs 0-4 remain legacy.')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('executable', type=Path)
    parser.add_argument('source', type=Path)
    parser.add_argument('report', type=Path)
    args = parser.parse_args()
    report = verify(args.executable, args.source)
    args.report.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print('Verified 31 source/retail venue assignments, eight labels and five runtime instruction anchors.')
