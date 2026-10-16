"""Import the pinned retail Euro96 stadium table, preserving legacy venue IDs."""
from pathlib import Path
import argparse,hashlib,json,struct,subprocess
from import_euro96 import ROOT,EXPECTED
from import_euro96_presentation import EXE_HASH
from import_cd_graphics import records

NAMES=['Wembley','Villa Park','Elland Road','St James Park','Old Trafford','Anfield','Hillsborough','City Ground']
BASE=0x13b06c
VENUE_LABELS=['LONDON WEMBLEY','BIRMINGHAM VILLA PARK','LEEDS ELLAND ROAD','NEWCASTLE ST JAMES PARK','MANCHESTER OLD TRAFFORD','LIVERPOOL ANFIELD','SHEFFIELD HILLSBOROUGH','NOTTINGHAM CITY GROUND']

def recover(iso):
    inputs={}
    for name,digest in {**EXPECTED,'EURO96.EXE':EXE_HASH}.items():
        if name=='SCRIPT96.ENG':continue
        b=subprocess.run(['7z','e','-so',str(iso),name],capture_output=True,check=True).stdout
        if hashlib.sha256(b).hexdigest()!=digest:raise ValueError('Unverified input '+name)
        inputs[name]=b
    exe=inputs['EURO96.EXE'];cd=records(inputs['EUROREND.DAT'],inputs['EUROREND.OFF'])
    labels=b''.join(s.encode('ascii')+b'\0' for s in VENUE_LABELS)
    if exe[0x13347d:0x13347d+len(labels)]!=labels:raise ValueError('Unexpected venue label order')
    # Retail loader addresses below are translated to file offsets; they are
    # disassembler display addresses, not claimed loaded runtime addresses.
    anchors=[]
    for address,expected in [(0xacce9,'8b048528b80000'),(0xacde5,'8b048518b80000'),(0xad382,'8b048524b80000'),(0xb013a,'8b048508b90000')]:
        at=328192+address-0x10000;raw=bytes.fromhex(expected)
        if exe[at:at+len(raw)]!=raw:raise ValueError('Stadium loader anchor differs')
        anchors.append(dict(file_offset=at,bytes=expected))
    sky_id,sky_pal=struct.unpack_from('<2I',exe,0x13b03c)
    if (sky_id,sky_pal)!=(800,792):raise ValueError('Unknown sky table')
    report=[];outputs={}
    for venue,name in enumerate(NAMES):
        at=BASE+venue*244;raw=exe[at:at+244]
        offsets=[12,20,24]+[28+i*20+j for i in range(4) for j in (12,16)]+[0,4]
        ids=[struct.unpack_from('<I',raw,o)[0]//8 for o in offsets]+[sky_id//8,sky_pal//8]
        if any(struct.unpack_from('<I',raw,o)[0]%8 for o in offsets):raise ValueError('Unaligned archive ID')
        parts=[cd[i] for i in ids]
        if [len(parts[i]) for i in [1,2,11,12,13,14]]!=[65536,65536,65536,48,307200,48]:raise ValueError('Unexpected stadium component size')
        if raw[8:10]!=bytes([0,1]) or struct.unpack_from('<I',raw,16)[0]!=60*8:raise ValueError('Unknown palette/sky mapping')
        # Retain the compatible native descriptor fields; retail adds one
        # stadium-palette file ID at offset 16. Keep the packed video address.
        desc=raw[:16]+raw[20:]
        palette=bytearray(cd[0]);palette[192*3:256*3]=cd[60];palette[128*3:144*3]=parts[12];palette[144*3:160*3]=parts[14]
        # The retail sky occupies 144..159; the preserved rasterizer uses
        # 208..223. Reindex the bitmap, keeping every source colour unchanged.
        if min(parts[13])<144 or max(parts[13])>159:raise ValueError('Unexpected sky indices')
        parts[13]=bytes(v+64 for v in parts[13])
        # ground() uses the first 64 rows with a 256-byte stride from its page.
        parts[11]=parts[11][:64*256]
        pack=b'EU96STD1'+desc+palette+b''.join(struct.pack('<I',len(x))+x for x in parts)
        file=f'GRAPHICS/EURO_STADIUM{venue}.BIN';outputs[file]=pack
        report.append(dict(name=name,venue_id=venue+5,descriptor_offset=at,descriptor_sha256=hashlib.sha256(raw).hexdigest(),records=ids,parts=[dict(record=i,source_sha256=hashlib.sha256(cd[i]).hexdigest()) for i in ids],path=file,sha256=hashlib.sha256(pack).hexdigest()))
    for file,b in outputs.items():
        p=ROOT/'local-data'/file
        if p.exists() and p.read_bytes()!=b:raise ValueError('Different stadium pack exists: '+file)
    for file,b in outputs.items():(ROOT/'local-data'/file).write_bytes(b)
    (ROOT/'docs/euro96-stadium-import.json').write_text(json.dumps(dict(inputs={n:hashlib.sha256(b).hexdigest() for n,b in inputs.items()},descriptor_stride=244,loader_anchors=anchors,venue_labels=dict(file_offset=0x13347d,labels=VENUE_LABELS),sky_variant=0,venues=report),indent=2)+'\n',encoding='utf-8')
    print('Imported eight retail Euro96 stadium sets; legacy venue IDs 0-4 retained.')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('iso',type=Path);recover(p.parse_args().iso.resolve(strict=True))
