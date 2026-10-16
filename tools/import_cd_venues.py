"""Import five complete CD venue sets with pinned descriptor and archive evidence."""
from pathlib import Path
import argparse,hashlib,json,struct,subprocess
from import_cd_graphics import EXPECTED,ROOT,records
VENUES=[(14,'England','STADIUM.BIN'),(15,'Scotland','SCOTLAND.BIN'),(2,'Spain','SPAIN.BIN'),(12,'Italy','ITALY.BIN'),(36,'Brazil','BRAZIL.BIN')]
def recover(iso):
 inputs={}
 for name,digest in EXPECTED.items():
  b=subprocess.run(['7z','e','-so',str(iso),name],capture_output=True,check=True).stdout
  if hashlib.sha256(b).hexdigest()!=digest:raise ValueError('Unsupported CD '+name)
  inputs[name]=b
 exe=inputs['SOCCER.EXE'];cd=records(inputs['ACTREND.DAT'],inputs['ACTREND.OFF']);report=[]
 for country,name,file in VENUES:
  assert exe[1031292+country*872:1031292+country*872+22].split(b'\0')[0].decode()==name
  at=0x132fa4+(90+country)*240;desc=exe[at:at+240]
  offsets=[12,16,20]+[24+i*20+j for i in range(4) for j in [12,16]]+[0,4]
  raw=[struct.unpack_from('<I',desc,o)[0] for o in offsets];assert all(v%8==0 and v//8<len(cd) for v in raw)
  assert desc[8] in (0,2)
  sky=struct.unpack_from('<2I',exe,1255764+47*32+desc[8]*16);raw+=list(sky)
  parts=[cd[v//8] for v in raw];assert [len(parts[i]) for i in [1,2,11,12,13,14]]==[65536,65536,16384,48,307200,48]
  pack=b'AC96STD1'+desc+b''.join(struct.pack('<I',len(b))+b for b in parts)
  target=ROOT/'local-data/GRAPHICS'/file;target.parent.mkdir(parents=True,exist_ok=True)
  if target.exists() and target.read_bytes()!=pack:raise ValueError('Different venue already exists: '+file)
  target.write_bytes(pack);report.append(dict(name=name,file=file,country=country,descriptor_offset=at,descriptor_sha256=hashlib.sha256(desc).hexdigest(),pack_sha256=hashlib.sha256(pack).hexdigest(),records=[v//8 for v in raw]))
 (ROOT/'docs/cd-venues-recovery.json').write_text(json.dumps(dict(inputs=EXPECTED,venues=report),indent=2)+'\n')
 print('Imported',len(report),'complete original CD venues')
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('iso',type=Path);recover(p.parse_args().iso)
