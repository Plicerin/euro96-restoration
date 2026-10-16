"""Pin the retail reception branches behind short-pass and set-piece speech."""
from pathlib import Path
import hashlib, json, struct, sys
from verify_retail_result_music import EXE_SHA


def verify(path):
    exe=path.read_bytes()
    if hashlib.sha256(exe).hexdigest()!=EXE_SHA:raise ValueError('Unverified retail executable')
    anchors=[]
    for at,raw,meaning in [
        (0xba34f,'53525683ec1489c2','Reception routine entry and receiver argument'),
        (0xba357,'a1d9a60700c1f81885c0','Signed comm_pass gate'),
        (0xba367,'f60570ab070038','Original commentary random gate'),
        (0xba374,'833da4a707000b','Previous toucher side test'),
        (0xba39e,'31c0a0dda6070083f801','Pass kind 1 branch'),
        (0xba3aa,'66833d9890070000','Set-piece flag in kind 1 branch'),
        (0xba3b4,'31f666893598900700e9bc000000','Clear flag, jump directly to reception cleanup'),
        (0xba3cc,'83f8027522','Pass kind 2 branch'),
        (0xba3d1,'66833d9890070000','Same set-piece flag in kind 2 branch'),
        (0xba3db,'31db66891d98900700e995000000','Clear flag, jump directly to reception cleanup'),
        (0xba442,'e8bd140100d95c240cb80c000000eb27','Distance result stored, then event 12 unconditionally selected'),
        (0xba479,'e836890400','Shared commentary dispatcher call'),
        (0xba47e,'c605dca60700ff','Clear comm_pass without another commentary call'),
    ]:
        if exe[at:at+len(bytes.fromhex(raw))]!=bytes.fromhex(raw):raise ValueError(f'Changed pass instruction at {at:x}')
        anchors.append(dict(file_offset=hex(at),bytes=raw,meaning=meaning))
    descriptor=0x1424b0+11*6
    if struct.unpack_from('<4sH',exe,descriptor)!=(b'CPA\0',17):raise ValueError('Unexpected retail completed-pass bank')
    # Decode the relative jumps independently of their descriptive labels.
    for at,target in [(0xba3bd,0xba47e),(0xba3e4,0xba47e)]:
        if at+5+struct.unpack_from('<i',exe,at+1)[0]!=target:raise ValueError('Set-piece branch does not bypass speech')
    if 0xba450+2+struct.unpack_from('<b',exe,0xba451)[0]!=0xba479:raise ValueError('Ordinary pass misses dispatcher')
    return dict(executable_sha256=EXE_SHA,reception_file_range=['0xba34f','0xba532'],anchors=anchors,
                retail_completed_pass=dict(event=12,bank='CPA',count=17,descriptor_file_offset=hex(descriptor)),
                source_routes={'13':'existing CPA bank','14':'same CPA bank and shared speech cooldown as 13',
                               '18':'silent for retail events; original routine still clears comsetp'},
                compatibility='Apply only when EURO_EVENTS.BIN is the active English voice pack. Older English and French retain their distinct events.',
                limits='This establishes these two routes only. Source pass classification, failed-pass behavior, other special events, random gating and all gameplay remain unchanged.')


if __name__=='__main__':
    executable,report=map(Path,sys.argv[1:])
    report.write_text(json.dumps(verify(executable),indent=2)+'\n')
    print('Verified retail ordinary-pass CPA routing and both silent set-piece branches.')
