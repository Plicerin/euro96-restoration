"""Reproduce the limits of the supplied retail board data without repairing it."""
from pathlib import Path
import argparse
import hashlib
import json
import struct
from collections import Counter
from import_euro96_boards import decode
from verify_retail_result_music import EXE_SHA, FAP_SHA, OFF_SHA, records


def relocation_audit(exe):
    """Read every internal fixup in this pinned LE revision; reject other forms.

    Format reference: Open Watcom bld/watcom/h/exeflat.h. This DOS executable
    has a second MZ stub: LE data-page offsets are relative to that image.
    """
    stub, header = 0x26654, 0x292D4
    u32 = lambda at: struct.unpack_from('<I', exe, at)[0]
    if exe[stub:stub+2] != b'MZ' or stub+u32(stub+0x3c) != header or exe[header:header+4] != b'LE\0\0':
        raise ValueError('Unknown embedded LE image')
    pages, size = u32(header+0x14), u32(header+0x28)
    if (pages, size, u32(header+0x44)) != (209, 4096, 3):
        raise ValueError('Unknown LE page/object layout')
    objects = [struct.unpack_from('<6I', exe, header+u32(header+0x40)+24*i) for i in range(3)]
    if objects != [(0xb78cf,0x10000,0x2005,1,184,0), (0x93,0xd0000,5,185,1,0),
                   (0xc4150,0xe0000,0x2003,186,24,0)]:
        raise ValueError('Unknown LE objects')
    data = stub+u32(header+0x80)
    if data != 0x76854: raise ValueError('Unknown LE code base')
    page_map = header+u32(header+0x48)
    for page in range(pages):
        if exe[page_map+4*page:page_map+4*page+4] != (page+1).to_bytes(3,'big')+b'\0':
            raise ValueError('Noncontiguous LE page map')
    page_table, record_table = header+u32(header+0x68), header+u32(header+0x6c)
    boundaries = [u32(page_table+4*p) for p in range(pages+1)]
    if boundaries[0] or boundaries != sorted(boundaries) or record_table+boundaries[-1] != header+u32(header+0x70):
        raise ValueError('Invalid LE fixup boundaries')
    forms, references, state = Counter(), [], []
    handlers = [0xe0cdc,0xe0d98,0xe0e54,0xe0f10,0xe0fcc,0xe107c,0xe1128,0xe11d8]
    for page in range(pages):
        at, end = record_table+boundaries[page], record_table+boundaries[page+1]
        while at < end:
            start=at; source, flags=exe[at:at+2];at+=2
            if (source,flags) not in ((7,16),(7,0),(2,0),(6,16)):
                raise ValueError('Unsupported LE fixup form')
            forms[source,flags]+=1
            source_offset=struct.unpack_from('<h',exe,at)[0];at+=2
            obj=exe[at];at+=1
            target=0
            if source != 2:
                width=4 if flags&16 else 2
                target=int.from_bytes(exe[at:at+width],'little');at+=width
            if at>end or not 1<=obj<=3 or not -4<=source_offset<size:
                raise ValueError('Invalid LE fixup record')
            file_source=data+page*size+source_offset
            if not data<=file_source<len(exe): raise ValueError('Invalid LE fixup source')
            if source==7 and int.from_bytes(exe[file_source:file_source+4],'little') != target:
                raise ValueError('LE offset differs from fixup target')
            if obj==1 and target+data in handlers:
                references.append(dict(record=hex(start),source=hex(file_source),handler=hex(target+data)))
            if obj==3 and target==0x97318:
                state.append(hex(file_source))
        if at!=end: raise ValueError('LE fixup overrun')
    if forms != Counter({(7,16):29176,(7,0):7451,(2,0):3,(6,16):1}):
        raise ValueError('LE fixup inventory changed')
    expected_state = [0xd16de,0xe0cfb,0xe0db7,0xe0e73,0xe0f2f,0xe0feb,0xe109c,
                      0xe1147,0xe11f7,0xe8c36,0xeafb1,0xeb021,0xec661,0xf076f]
    state.sort(key=lambda value:int(value,16))
    if state != [hex(i) for i in expected_state] or references:
        raise ValueError(f'Board references changed: state={state}, handlers={references}')
    branches = {}
    for handler in handlers:
        calls=[];jumps=[]
        for at in range(data,data+objects[0][0]-4):
            if exe[at] in (0xe8,0xe9) and at+5+struct.unpack_from('<i',exe,at+1)[0]==handler:
                (calls if exe[at]==0xe8 else jumps).append(hex(at))
        branches[hex(handler)]=dict(call_candidates=calls,tail_jump_candidates=jumps)
    expected_calls = [[0xeafda],[0xeafe3],[0xeafec],[0xeaff5],[0xeaffe],
                      [0xb7543,0xb75c4,0xeb007],[0xeb010],[0xeb019]]
    for handler, calls in zip(handlers,expected_calls):
        if branches[hex(handler)] != dict(call_candidates=[hex(i) for i in calls],tail_jump_candidates=[]):
            raise ValueError('Board branch candidates changed')
    anchors=[]
    for at,raw in [(0xf0765,'31db'),(0xf076d,'891d18730900'),
                   (0xeafaf,'833d1873090000'),(0xeafb6,'0f8481000000'),
                   (0xe113b,'ba07000000'),(0xe1145,'891518730900'),
                   (0xe11eb,'ba08000000'),(0xe11f5,'891518730900'),
                   (0xb7541,'31c0e8349b0200'),(0xb75c2,'31c0e8b39a0200')]:
        if exe[at:at+len(bytes.fromhex(raw))]!=bytes.fromhex(raw): raise ValueError('Board state anchor changed')
        anchors.append(dict(file_offset=hex(at),bytes=raw))
    return dict(format_reference='https://github.com/open-watcom/open-watcom-v2/blob/master/bld/watcom/h/exeflat.h',
                embedded_mz=hex(stub),le_header=hex(header),code_file_base=hex(data),pages=pages,
                fixups=sum(forms.values()),forms={f'{s:02x}/{f:02x}':n for (s,f),n in forms.items()},
                relocated_handler_references=references,relocated_board_state_references=state,
                direct_branch_candidates=branches,state_anchors=anchors,
                positive_control='The same scan recovers both real goal triggers (own goal at 0xb7543 and normal goal at 0xb75c4), each calling the goal-board handler with zero to start loading.',
                conclusion='No relocated pointer to any of the eight board handlers. Winner call candidates remain confined to the renderer. Board state initializes to zero; identified nonzero setters are the handlers themselves.',
                limits='Static evidence suggests dormant paths, but does not exclude a computed address, aliased state write, external patch or another retail revision. This is not a verified match-end trigger.')


def verify(folder):
    inputs = {}
    for name, expected in [('EURO96.EXE', EXE_SHA), ('FAP.DAT', FAP_SHA), ('FAP.OFF', OFF_SHA)]:
        data = (folder/name).read_bytes()
        if hashlib.sha256(data).hexdigest() != expected:
            raise ValueError('Unverified retail input: '+name)
        inputs[name] = data
    exe = inputs['EURO96.EXE']
    archive = records(inputs['FAP.DAT'], inputs['FAP.OFF'])
    coverage = {}
    for index, data in enumerate(archive):
        try:
            coverage[index] = dict(frames=len(decode(data)))
        except ValueError as error:
            coverage[index] = dict(excluded=str(error))
    if [i for i, item in coverage.items() if 'excluded' in item] != [7]:
        raise ValueError('Unexpected excluded records')
    # This independent walk identifies the first unsafe write, rather than
    # accepting an image because its earlier frames happened to look plausible.
    data = archive[7]
    at, frame, failure = 1024, 0, None
    while not failure:
        kind = 2 if frame == 0 else data[at]
        if frame: at += 1
        if kind not in (1, 2): raise ValueError('Unexpected penalty frame tag')
        pos = 0
        while True:
            run = data[at]; at += 1
            if not run: break
            pos += run; at += run
            if pos > 10240:
                failure = dict(frame=frame, operation='literal', end_pixel=pos, pixel_limit=10240, cursor=at)
                break
            skip = data[at]; at += 1; pos += skip
            if pos > 10240:
                raise ValueError('Unexpected first penalty failure')
        if not failure: frame += 1
    if failure != dict(frame=20, operation='literal', end_pixel=10442, pixel_limit=10240, cursor=68651):
        raise ValueError('Penalty boundary changed')
    # A confirmed switch table establishes the file-relative code address base.
    code_base, table = 0x76854, 0xe8b80
    entries = [code_base+x for x in struct.unpack_from('<8I', exe, table)]
    if entries != [0xeafd8, 0xeafe1, 0xeafea, 0xeaff3, 0xeaffc, 0xeb005, 0xeb00e, 0xeb017]:
        raise ValueError('Board dispatcher changed')
    candidates = {}
    for target, expected_call in [(0xe1128, 0xeb010), (0xe11d8, 0xeb019)]:
        calls = [i for i in range(len(exe)-5) if exe[i] == 0xe8 and i+5+struct.unpack_from('<i', exe, i+1)[0] == target]
        if calls != [expected_call]: raise ValueError('Winner call candidates changed')
        candidates[hex(target)] = [hex(i) for i in calls]
    return dict(inputs={name:hashlib.sha256(data).hexdigest() for name, data in inputs.items()}, records=coverage,
                penalty_first_invalid_write=failure, winner_direct_call_candidates=candidates,
                retail_relocations=relocation_audit(exe),
                conclusion='Penalty cannot be decoded safely in full. Winner direct-call candidates lead only to the board dispatcher; a match/championship trigger is still unverified.',
                limits='The complete LE relocation audit finds no extra handler pointers. Computed or aliased accesses and other revisions remain outside that proof. No new winner trigger or speculative data repair is justified.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('folder', type=Path)
    parser.add_argument('report', type=Path)
    args = parser.parse_args()
    args.report.write_text(json.dumps(verify(args.folder), indent=2)+'\n', encoding='utf-8')
    print('Verified all 16 records, exact unsafe penalty boundary and bounded winner-call evidence.')
