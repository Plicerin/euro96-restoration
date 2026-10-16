"""Recover bounded retail stadium animation streams and pin their event handlers."""
from pathlib import Path
import argparse, hashlib, json, struct
from verify_retail_result_music import EXE_SHA, FAP_SHA, OFF_SHA, records
from import_euro96 import EXPECTED

def digest(data):
    return hashlib.sha256(data).hexdigest()

def decode(data):
    # The match renderer at e0a70 skips the size table. Unlike the frontend
    # decoder it walks terminated streams: retail record 1 has 73 frames,
    # despite declaring 67. Never trust an archive length as a pixel bound.
    if len(data)<1026: raise ValueError('Truncated board header')
    at=1024; pixels=bytearray(128*80); frames=[]
    while len(frames)<256:
        kind=2
        if frames:
            if at>=len(data): raise ValueError('Missing board terminator')
            kind=data[at];at+=1
            if not kind:
                if at!=len(data): raise ValueError('Trailing board data')
                return frames
            if kind not in (1,2): raise ValueError('Invalid board frame type')
        pos=0
        while True:
            if at>=len(data): raise ValueError('Truncated board run')
            run=data[at];at+=1
            if not run:break
            if at+run>=len(data) or pos+run>len(pixels): raise ValueError('Board literal outside frame')
            pixels[pos:pos+run]=data[at:at+run];pos+=run;at+=run
            skip=data[at];at+=1
            if pos+skip>len(pixels): raise ValueError('Board repeat/skip outside frame')
            if kind==2:pixels[pos:pos+skip]=bytes([pixels[pos-1]])*skip
            pos+=skip
        if not frames and pos!=len(pixels): raise ValueError('Incomplete first board frame')
        frames.append(bytes(pixels))
    raise ValueError('Excessive board frames')

def recover(folder, destination, report):
    inputs={}
    for name,expected in {'EURO96.EXE':EXE_SHA,'FAP.DAT':FAP_SHA,'FAP.OFF':OFF_SHA,
                          'EUROREND.DAT':EXPECTED['EUROREND.DAT'],'EUROREND.OFF':EXPECTED['EUROREND.OFF']}.items():
        data=(folder/name).read_bytes()
        if digest(data)!=expected:raise ValueError('Unverified retail input '+name)
        inputs[name]=data
    exe=inputs['EURO96.EXE'];anchors=[]
    for at,raw,meaning in [
        (0xe0a70,'81c200040000','Match decoder skips 1024-byte frame table'),
        (0xe0bcd,'83fe01','Frame tag selects literal/skip versus literal/repeat'),
        (0xf07f9,'31c0','Initial board archive record zero'),
        (0xe0d62,'b840000000','Handler 1 red-card record 8'),(0xe0d6e,'b848000000','Handler 1 red-card record 9'),
        (0xe0e1f,'b818000000','Foul record 3'),(0xe0e2b,'b820000000','Foul record 4'),
        (0xe0edb,'b870000000','Handler 3 yellow-card record 14'),(0xe0ee7,'b878000000','Handler 3 yellow-card record 15'),
        (0xe0f97,'b850000000','Half time record 10'),(0xe0fa3,'b858000000','Half time record 11'),
        (0xe1051,'b838000000','Penalty record 7'),
        (0xe10fc,'b808000000','Goal record 1'),
        (0xe11ad,'b810000000','Championship winners record 2'),
        (0xe125f,'b868000000','Winners record 13'),(0xe126b,'b860000000','Winners record 12'),
    ]:
        if exe[at:at+len(bytes.fromhex(raw))]!=bytes.fromhex(raw):raise ValueError(f'Changed instruction {at:x}')
        anchors.append(dict(offset=hex(at),bytes=raw,meaning=meaning))
    raw=records(inputs['FAP.DAT'],inputs['FAP.OFF']);decoded={};inventory=[]
    for i,data in enumerate(raw):
        try:
            frames=decode(data);decoded[i]=frames
            inventory.append(dict(record=i,frames=len(frames),sha256=digest(data),decoded_sha256=digest(b''.join(frames))))
        except ValueError as error:
            inventory.append(dict(record=i,sha256=digest(data),excluded=str(error)))
    if set(decoded)!=set(range(16))-{7}:raise ValueError('Unexpected board coverage')
    rend=records(inputs['EUROREND.DAT'],inputs['EUROREND.OFF'])
    palette=bytearray(rend[0]);palette[192*3:]=rend[60]
    if len(palette)!=768 or max(palette)>63:raise ValueError('Invalid board palette')
    # Event IDs stay compatible with the native observer. Winner records are
    # decoded for review, but await a verified match/championship trigger.
    mapping={0:[0],2:[3,4],4:[10,11],6:[1],7:[14,15],8:[8,9]}
    rows=[(event,record) for event,ids in mapping.items() for record in ids]
    pack=b'EU96BRD1'+struct.pack('<I',len(rows))+palette
    for event,record in rows:
        frames=decoded[record];pack+=struct.pack('<III',event,record,len(frames))+b''.join(frames)
    if destination.exists() and destination.read_bytes()!=pack:raise ValueError('Different board pack already exists')
    destination.parent.mkdir(parents=True,exist_ok=True);destination.write_bytes(pack)
    report.write_text(json.dumps(dict(inputs={n:digest(b) for n,b in inputs.items()},anchors=anchors,
        inventory=inventory,event_records=mapping,output_sha256=digest(pack),
        boundaries=['Penalty record 7 exceeds pixel bounds after 20 decoded frames; retain original fallback.',
                    'Records 2/12/13 decode, but winner event routing is not connected.',
                    'Goal record 1 has 73 valid streamed frames despite 67 table entries.',
                    'Native observer connects confirmed card colours to booking/send-off events; retail handler IDs differ from preserved source.',
                    'Extra/full time keep existing fallback; winner routing remains unverified.',
                    'Variant rotation and 20 Hz playback are native timing adaptations.']),indent=2)+'\n')
    return decoded,bytes(palette)

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('folder','destination','report'):p.add_argument(name,type=Path)
    args=p.parse_args();recover(args.folder,args.destination,args.report)
    print('Decoded 15 retail board records; imported 10 streams for six event states; unsafe penalty excluded.')
