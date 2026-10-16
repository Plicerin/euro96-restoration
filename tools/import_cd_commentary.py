"""Validate and import original English or French phrase records without modifying SOURCE."""
from pathlib import Path
import argparse,hashlib,json,re,struct,subprocess
from collections import Counter
from inspect_cd_audio import inspect_mad
from restore_commentary import recover
p=argparse.ArgumentParser(description=__doc__);p.add_argument('iso',type=Path);p.add_argument('--language',choices=['english','french'],default='english');args=p.parse_args()
prefix='E' if args.language=='english' else 'F'
root=Path(__file__).resolve().parents[1];mapping=recover(root/'SOURCE')
records=[];banks=[];cache={}
for row in mapping['categories']:
 path=f'DIGI/PHRASES/{prefix}/{prefix}_'+row['bank']+'.MAD'
 if path not in cache:
  data=subprocess.run(['7z','e','-so',str(args.iso.resolve(strict=True)),path],capture_output=True,check=True).stdout
  entries=inspect_mad(data);cache[path]=(data,entries)
  banks.append(dict(path=path,sha256=hashlib.sha256(data).hexdigest(),records=len(entries)))
 data,entries=cache[path];counts=Counter(e['name'].lower() for e in entries)
 for e in entries:
  m=re.fullmatch(prefix.lower()+'_'+row['bank'].lower()+r'(\d{2})\.wav',e['name'].lower())
  if not m or counts[e['name'].lower()]!=1:continue # Both ambiguous duplicate filenames are excluded.
  number=int(m[1])
  if number not in row.get('numbers',range(1,row.get('count',0)+1)):continue
  if (e['rate'],e['channels'],e['sample_width'])!=(11025,1,1):raise ValueError('Unsupported phrase WAV')
  records.append((row['id'],number,data[e['offset']:e['offset']+e['size']],e))
 if not any(r[0]==row['id'] for r in records):raise ValueError('Empty verified category '+row['event'])
# Country names and two-letter speech codes are embedded as team_info records.
exe=subprocess.run(['7z','e','-so',str(args.iso.resolve(strict=True)),'SOCCER.EXE'],capture_output=True,check=True).stdout
exe_hash=hashlib.sha256(exe).hexdigest()
if exe_hash!='cc6e2547c9f97050e1d610ba242476b7d3147f4a7a2e9a0284d6585691137543':raise ValueError('Unsupported CD executable data')
base=1031292;teams={}
for i in range(45):
 row=exe[base+i*872:base+(i+1)*872];name=row[:22].split(b'\0')[0].decode('ascii');code=row[136:139].split(b'\0')[0].decode('ascii')
 if not re.fullmatch('[A-Z]{2}',code):raise ValueError('Invalid original country code')
 teams[re.sub('[^A-Z]','',name.upper())]=(name,code,base+i*872)
# The original LinkStack plays home X, away Y, then a random MCA closing phrase.
def extra(bank,category,pattern):
 path='DIGI/'+bank
 data=subprocess.run(['7z','e','-so',str(args.iso.resolve(strict=True)),path],capture_output=True,check=True).stdout
 entries=inspect_mad(data);banks.append(dict(path=path,sha256=hashlib.sha256(data).hexdigest(),records=len(entries)))
 counts=Counter(e['name'].lower() for e in entries)
 matches=[e for e in entries if re.fullmatch(pattern,e['name'].lower()) and counts[e['name'].lower()]==1]
 for e in matches:
  if (e['rate'],e['channels'],e['sample_width'])!=(11025,1,1):raise ValueError('Unsupported linked phrase PCM')
  number=int(re.search(r'(\d{2})\.wav$',e['name'])[1]) if category==36 else 1
  records.append((category,number,data[e['offset']:e['offset']+e['size']],e))
 return len(matches)
# French bank starts at MCA02; do not fabricate the absent MCA01 recording.
assert extra(f'PHRASES/{prefix}/{prefix}_MCA.MAD',36,prefix.lower()+r'_mca(0[1-9]|1[0-9])\.wav')==(19 if prefix=='E' else 18)
countries=re.findall(r't:\s*"([^"]+)"\s+form:\s*[\d-]+',(root/'SOURCE/SCRIPT.96').read_text(encoding='latin1'))
assert len(countries)==16
welcome=[]
for i,name in enumerate(countries):
 key=re.sub('[^A-Z]','',name)
 if key not in teams:
  welcome.append(dict(euro_index=i,name=name,available=False));continue
 cd_name,code,offset=teams[key]
 assert extra(f'TEAMS/{prefix}/{prefix}_'+code+'.MAD',64+i,prefix.lower()+'_'+code.lower()+r'xtt\.wav')==1
 assert extra(f'TEAMS/{prefix}/{prefix}_'+code+'.MAD',80+i,prefix.lower()+'_'+code.lower()+r'ytt\.wav')==1
 welcome.append(dict(euro_index=i,name=name,cd_name=cd_name,code=code,offset=offset,available=True))
mapping.update(welcome=welcome,cd_executable_sha256=exe_hash,welcome_order='Home XTT, away YTT, MCA; AUDIO.OBJ PlayWelcomeMessage pushes reverse order and UpdateCommentary pops LinkStack.')
# E_SCOxy is one complete recording, not a country or player-name sequence.
path=f'DIGI/PHRASES/{prefix}/{prefix}_SCO.MAD'
data=subprocess.run(['7z','e','-so',str(args.iso.resolve(strict=True)),path],capture_output=True,check=True).stdout
entries=inspect_mad(data);counts=Counter(e['name'].lower() for e in entries)
banks.append(dict(path=path,sha256=hashlib.sha256(data).hexdigest(),records=len(entries)))
for e in entries:
 m=re.fullmatch(prefix.lower()+r'_sco([0-7])([0-5])\.wav',e['name'].lower())
 if not m or int(m[1])<int(m[2]) or counts[e['name'].lower()]!=1:continue
 if (e['rate'],e['channels'],e['sample_width'])!=(11025,1,1):raise ValueError('Unsupported score PCM')
 records.append((44,int(m[1])*10+int(m[2]),data[e['offset']:e['offset']+e['size']],e))
assert sum(r[0]==44 for r in records)==33
pack=b'AC96COM1'+struct.pack('<I',len(records))+b''.join(struct.pack('<HHI',id,num,len(data))+data for id,num,data,e in records)
target=root/('local-data/COMMENTARY/ENGLISH3.BIN' if prefix=='E' else 'local-data/COMMENTARY/FRENCH.BIN');target.parent.mkdir(parents=True,exist_ok=True)
if target.exists() and target.read_bytes()!=pack:raise ValueError('Different commentary pack already installed')
target.write_bytes(pack)
banks=list({b['path']:b for b in banks}.values())
mapping.update(banks=banks,pack_sha256=hashlib.sha256(pack).hexdigest(),pack_bytes=len(pack),clips=[dict(id=id,number=num,**e) for id,num,data,e in records],excluded='Missing numbered clips and all ambiguous duplicate filenames; unmatched Croatia/Turkey and all individual player names are not imported.')
(root/('docs/commentary-recovery.json' if prefix=='E' else 'docs/french-commentary-recovery.json')).write_text(json.dumps(mapping,indent=2)+'\n',encoding='utf-8')
print(f'Imported {len(records)} verified phrase clips from {len(banks)} original banks ({len(pack)} bytes)')
