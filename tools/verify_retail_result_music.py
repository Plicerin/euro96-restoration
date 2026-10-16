"""Pin the retail match-return/frontend-theme route and distinguish board archives."""
from pathlib import Path
import argparse
import hashlib
import json
import struct

EXE_SHA = '2a473a002a294c1bb28ea39e94a36afc34497dd7e44f0ebfeda0ea21f652b62d'
FAP_SHA = 'c1b917dd6319cd0eb94cf8db5391283afdafd846f86c48000a0fa5a3b86471b6'
OFF_SHA = 'ff93223b63dcc20fd9b115a2d9e96edb00bd38781d064a78190c929720e21ddb'

def digest(b):
    return hashlib.sha256(b).hexdigest()

def records(data, offsets):
    if len(offsets) % 8:
        raise ValueError('Misaligned archive offsets')
    result = []
    for i in range(0, len(offsets), 8):
        at, size = struct.unpack_from('<II', offsets, i)
        if at + size > len(data):
            raise ValueError('Archive record exceeds file')
        result.append(data[at:at+size])
    return result

def verify(exe_path, retail_dir, source):
    exe = exe_path.read_bytes()
    if digest(exe) != EXE_SHA:
        raise ValueError('Unverified retail executable')
    anchors = []
    for at, raw, meaning in [
        (0x77449, 'e802000200', 'Frontend calls PlayMatch at file 0x97450'),
        (0x980ea, 'e815800600', 'Match launch calls StopMusic at file 0x100104'),
        (0x980ef, 'c605d07e000000', 'Match launch clears frontend music state'),
        (0x9813e, 'e8fcf70000', 'PlayMatch calls the match engine at file 0xa793f'),
        (0x98922, 'c605d4a4000000', 'Ordinary non-championship return selects main menu'),
        (0x9898d, 'c95f5e595bc3', 'PlayMatch returns to its frontend caller'),
        (0x774bb, 'e98afaffff', 'Frontend back edge returns to loop at file 0x76f4a'),
        (0x76f99, 'e911010000', 'Existing frontend pages join common update path at file 0x770af'),
        (0x770be, 'e8b9120100', 'Main loop calls music handler at file 0x8837c'),
        (0x883af, '803dd07e000000', 'Music handler checks cleared/active music state'),
        (0x883c5, 'b8e80d0000', 'Loader argument is EURO96 at data-relative 0xde8'),
        (0x883ca, 'e8357b0700', 'StartMusic call at file 0xfff04'),
        (0x883cf, 'c605d07e000001', 'Music state becomes active'),
        (0x883f8, 'e8477d0700', 'CheckMusic updates playback state'),
        (0x8840b, 'e8f47c0700', 'Finished music is stopped'),
        (0x88410, 'c605d07e000000', 'Finished theme becomes eligible for next-loop restart'),
        (0x776d9, 'e8268a0800', 'Trophy page stops music before film playback'),
    ]:
        expected = bytes.fromhex(raw)
        if exe[at:at+len(expected)] != expected:
            raise ValueError(f'Instruction differs at {at:#x}')
        anchors.append(dict(file_offset=hex(at), bytes=raw, meaning=meaning))
    if exe[0x13063c:0x130643] != b'EURO96\0':
        raise ValueError('Unexpected track name')
    calls = [i for i in range(0x76f4a,0x12f854-5) if exe[i] == 0xe8 and i+5+struct.unpack_from('<i',exe,i+1)[0] == 0xfff04]
    if calls != [0x883ca]:
        raise ValueError('Unexpected direct music loader calls in inspected frontend/engine range')
    retail_data = (retail_dir/'FAP.DAT').read_bytes()
    retail_off = (retail_dir/'FAP.OFF').read_bytes()
    if digest(retail_data) != FAP_SHA or digest(retail_off) != OFF_SHA:
        raise ValueError('Unverified retail board archives')
    original_data = (source/'FAP.DAT').read_bytes()
    original_off = (source/'FAP.OFF').read_bytes()
    retail_records = records(retail_data, retail_off)
    original_records = records(original_data, original_off)
    if len(retail_records) != 16 or len(original_records) != 27:
        raise ValueError('Unexpected board archive layout')
    winners = []
    retail_hashes = {digest(record) for record in retail_records}
    for label, ids in [('XWNF', [18,19,20]), ('XWNG', [21,22,23]), ('XWNS', [24,25,26])]:
        hashes = [digest(original_records[i]) for i in ids]
        if any(h in retail_hashes for h in hashes):
            raise ValueError('Unexpected exact retail winner-record match')
        winners.append(dict(source_name=label, source_records=ids, sha256=hashes, exact_retail_record_matches=[]))
    return dict(executable_sha256=EXE_SHA, music=dict(
        anchors=anchors, track_name='EURO96', track_name_file_offset='0x13063c',
        direct_loader_calls_in_frontend_engine_range=[hex(x) for x in calls],
        conclusion='Match launch stops and resets the theme. Returning to the frontend reaches the common theme handler again; this path does not select a win/loss/draw tune.',
        adaptation='Native post-match statistics are a frontend page; outcome music requests resolve to the looping Euro theme. Native gain/fade/ducking and menu timing remain adaptations.'),
        boards=dict(source_records=len(original_records), retail_records=len(retail_records),
                    source_data_sha256=digest(original_data), source_offsets_sha256=digest(original_off),
                    retail_data_sha256=FAP_SHA, retail_offsets_sha256=OFF_SHA, winners=winners,
                    conclusion='The three preserved two-part WINNERS animations are not exact records in the supplied retail archive. Retail uses different handlers and data; no new victory trigger is assigned by this audit. This does not establish that no equivalent retail animation exists.'))

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('executable', 'retail_directory', 'source', 'report'):
        parser.add_argument(name, type=Path)
    args = parser.parse_args()
    report = verify(args.executable, args.retail_directory, args.source)
    args.report.write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    print('Verified 17 music-route anchors, EURO96 track selection, and distinct 27/16-record board archives.')
