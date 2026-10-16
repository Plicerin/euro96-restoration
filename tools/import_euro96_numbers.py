"""Recover both original Euro96 shirt-number sets, including 16-20."""
from pathlib import Path
import argparse
import hashlib
import json
import struct
import subprocess
from import_euro96 import ROOT, EXPECTED
from import_cd_graphics import records

def recover(iso):
    inputs={}
    for name in ('EUROREND.DAT','EUROREND.OFF'):
        data=subprocess.run(['7z','e','-so',str(iso),name],capture_output=True,check=True).stdout
        if hashlib.sha256(data).hexdigest()!=EXPECTED[name]:
            raise ValueError('Unsupported retail number revision: '+name)
        inputs[name]=data
    cd=records(inputs['EUROREND.DAT'],inputs['EUROREND.OFF'])
    if len(cd[38])!=65536 or len(cd[0])!=768:
        raise ValueError('Invalid retail number resources')
    # Preserve original rectangles; do not import unrelated fonts from this page.
    atlas=bytearray(65536);uvs=bytearray();entries=[]
    for variant in range(2):
        for number in range(1,21):
            index=533+variant*20+number-1
            raw=cd[1][(index-1)*32:index*32];uv=struct.unpack('<8I',raw)
            values=[(v&0xffffff)>>16 for v in uv]
            x=(number-1)*17 if number<=15 else (number-16+variant*5)*17
            y=181+variant*25 if number<=15 else 231
            if uv[0]>>24!=6 or values!=[y,y,y+24,y+24,x,x+16,x+16,x]:
                raise ValueError('Unexpected retail number coordinates')
            pixels=bytes(cd[38][row*256+col] for row in range(y,y+25) for col in range(x,x+17))
            if max(pixels)>31 or 0 not in pixels or not any(pixels):
                raise ValueError('Invalid original glyph indices/transparency')
            for row in range(y,y+25):atlas[row*256+x:row*256+x+17]=cd[38][row*256+x:row*256+x+17]
            uvs.extend(raw)
            entries.append(dict(variant=variant,number=number,texture_id=index,x=x,y=y,width=17,height=25,sha256=hashlib.sha256(pixels).hexdigest(),opaque_pixels=sum(v!=0 for v in pixels)))
    palette=cd[0][:96]
    if max(palette)>63:raise ValueError('Invalid original grayscale palette')
    pack=b'EU96NUM1'+palette+uvs+atlas
    target=ROOT/'local-data/GRAPHICS/NUMBERS.BIN'
    if target.exists() and target.read_bytes()!=pack:raise ValueError('Different numbers pack exists')
    target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(pack)
    report=dict(inputs={n:EXPECTED[n] for n in inputs},atlas_record=38,uv_record=1,palette_record=0,transparency='Raw index zero is transparent; native RGBA preserves raw palette indices without the old atlas +1 encoding.',entries=entries,pack_bytes=len(pack),pack_sha256=hashlib.sha256(pack).hexdigest())
    (ROOT/'docs/euro96-numbers-import.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f'Imported 40 original glyphs (light/dark 1-20), {len(pack)} bytes.')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('iso',type=Path)
    recover(p.parse_args().iso.resolve(strict=True))
