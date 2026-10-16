"""Recover the finished CD high-detail England stadium as one matched asset set."""
from pathlib import Path
import argparse, hashlib, json, struct, subprocess
from import_cd_graphics import EXPECTED, ROOT, records

def recover(iso):
 inputs={}
 for name,digest in EXPECTED.items():
  b=subprocess.run(['7z','e','-so',str(iso),name],capture_output=True,check=True).stdout
  if hashlib.sha256(b).hexdigest()!=digest:raise ValueError('Unsupported CD revision: '+name)
  inputs[name]=b
 cd=records(inputs['ACTREND.DAT'],inputs['ACTREND.OFF']);exe=inputs['SOCCER.EXE']
 # FILES.C has 45 countries in each of three detail tiers; England is country 14.
 base=0x132fa4;index=90+14;at=base+index*240;descriptor=exe[at:at+240]
 assert struct.unpack_from('<3f',exe,base+24)==struct.unpack('<3f',struct.pack('<3f',1571.371,0,-399.495))
 assert struct.unpack_from('<3i',descriptor,104)==(360,390,820)
 expected=[(1669.585,-21,-386.554),(607.233,2,440.030),(-454.415,-21,-393.554),(599.570,1,-1224.024)]
 for i,xyz in enumerate(expected):assert struct.unpack_from('<3f',descriptor,24+i*20)==struct.unpack('<3f',struct.pack('<3f',*xyz))
 offsets=[12,16,20]+[24+i*20+j for i in range(4) for j in [12,16]]+[0,4]
 ids=[struct.unpack_from('<I',descriptor,o)[0]//8 for o in offsets]
 assert ids==[276,112,122,284,283,280,279,278,277,282,281,247,202]
 # skylist immediately follows the 47 kit descriptors, then pitchfile3DB.
 sky_base=1255764+47*32
 sky_type=descriptor[8]
 sky_ids=struct.unpack_from('<2I',exe,sky_base+sky_type*16)
 assert sky_type==2 and sky_ids==(226*8,220*8) # CLOUDY bitmap 226, palette 220
 ids += [v//8 for v in sky_ids]
 names=['uv','map1','map2']+[f'stand{i+1}_{part}' for i in range(4) for part in ['points','faces']]+['pitch','pitch_palette','sky','sky_palette']
 parts=[cd[i] for i in ids]
 assert len(parts[1])==len(parts[2])==65536 and len(parts[11])==16384 and len(parts[12])==48 and len(parts[13])==307200 and len(parts[14])==48
 pack=b'AC96STD1'+descriptor+b''.join(struct.pack('<I',len(b))+b for b in parts)
 dest=ROOT/'local-data/GRAPHICS/STADIUM.BIN';dest.parent.mkdir(parents=True,exist_ok=True)
 if dest.exists() and dest.read_bytes()!=pack:raise ValueError('Different stadium pack exists')
 dest.write_bytes(pack)
 report=dict(inputs=EXPECTED,descriptor_offset=at,descriptor_index=index,descriptor_stride=240,identity='Original CD high-detail England stadium (FILES.C tier 90 + country 14)',descriptor_sha256=hashlib.sha256(descriptor).hexdigest(),pack_sha256=hashlib.sha256(pack).hexdigest(),parts=[dict(name=n,record=i,size=len(b),sha256=hashlib.sha256(b).hexdigest()) for n,i,b in zip(names,ids,parts)])
 (ROOT/'docs/cd-stadium-recovery.json').write_text(json.dumps(report,indent=2)+'\n')
 print('Imported original CD stadium:',len(pack),'bytes')
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('iso',type=Path);recover(p.parse_args().iso.resolve(strict=True))
