"""Recover original X/Y/Z player-name clips using exact cross-revision roster matches."""
from pathlib import Path
import argparse,hashlib,json,re,struct,subprocess
from collections import Counter
from import_cd_graphics import ROOT,EXPECTED
from inspect_cd_audio import inspect_mad
from omf import Object
def recover(iso,language='english'):
 prefix='E' if language=='english' else 'F'
 exe=subprocess.run(['7z','e','-so',str(iso),'SOCCER.EXE'],capture_output=True,check=True).stdout
 assert hashlib.sha256(exe).hexdigest()==EXPECTED['SOCCER.EXE']
 obj=Object(ROOT/'SOURCE/AUDIO.OBJ');code=obj.segments[1]['data'];const=obj.segments[2]['data']
 assert hashlib.sha256((ROOT/'SOURCE/AUDIO.OBJ').read_bytes()).hexdigest()=='3bd8d90d2cef292aeda77ef86a9aee490ddb9dfbc0f2adef1fd493f4a5da33f9'
 # Loader uses squad_number+1, X/Y/Z suffixes and 33-byte player records.
 for at,raw in [(0x11c6,'8a421e40'),(0x1272,'83c121'),(0x15bf,'83fb01'),(0x15c6,'83fb02')]:assert bytes(code[at:at+len(bytes.fromhex(raw))]).hex()==raw
 assert bytes(const[0x146:0x151])==b'%.2d\0X\0Y\0Z\0'
 countries=re.findall(r't:\s*"([^"]+)"\s+form:\s*[\d-]+',(ROOT/'SOURCE/SCRIPT.96').read_text(encoding='latin1'))
 script=(ROOT/'SOURCE/SCRIPT.96').read_text(encoding='latin1');roster=re.findall(r'^\s*t:\s*"([^"]+)"[^\n]*pace:[^\n]*name:',script[script.index('PLYR_NMES'):script.index('TEAM_NMES')],re.M)
 norm=lambda n:re.sub('[^A-Z]','',n.upper())
 table={norm(exe[1031292+i*872:1031292+i*872+22].split(b'\0')[0].decode()):i for i in range(45)}
 clips=[];report=[];banks=[]
 for team,country in enumerate(countries):
  if norm(country) not in table:continue
  cd_team=table[norm(country)];base=1031292+cd_team*872;code_name=exe[base+136:base+139].split(b'\0')[0].decode();assert re.fullmatch('[A-Z]{2}',code_name)
  path=f'DIGI/TEAMS/{prefix}/{prefix}_'+code_name+'.MAD';data=subprocess.run(['7z','e','-so',str(iso),path],capture_output=True,check=True).stdout;entries=inspect_mad(data);counts=Counter(e['name'].lower() for e in entries)
  bank={e['name'].lower():e for e in entries if counts[e['name'].lower()]==1};banks.append(dict(path=path,sha256=hashlib.sha256(data).hexdigest()))
  cd_names=[exe[base+144+i*33:base+144+i*33+20].split(b'\0')[0].decode('latin1') for i in range(22)]
  for slot in range(20):
   name=roster[team*20+slot];parts=name.split();key=norm(parts[0][0]+''.join(parts[1:]));matches=[i for i,n in enumerate(cd_names) if norm(n)==key]
   if len(matches)!=1:continue
   cd_slot=matches[0];number=exe[base+144+cd_slot*33+30]+1
   for tone,suffix in enumerate('xyz',1):
    filename=f'{prefix.lower()}_{code_name.lower()}{suffix}{number:02d}.wav'
    if filename not in bank:continue
    e=bank[filename];assert (e['rate'],e['channels'],e['sample_width'])==(11025,1,1)
    wav=data[e['offset']:e['offset']+e['size']];category=128+team*20+slot;clips.append((category,tone,wav));report.append(dict(team=team,shirt=slot+1,name=name,cd_name=cd_names[cd_slot],file=filename,tone=tone,sha256=hashlib.sha256(wav).hexdigest()))
 pack=b'AC96COM1'+struct.pack('<I',len(clips))+b''.join(struct.pack('<HHI',c,t,len(w))+w for c,t,w in clips)
 target=ROOT/('local-data/COMMENTARY/NAMES.BIN' if prefix=='E' else 'local-data/COMMENTARY/FRENCH_NAMES.BIN');target.parent.mkdir(parents=True,exist_ok=True)
 if target.exists() and target.read_bytes()!=pack:raise ValueError('Different names pack exists')
 target.write_bytes(pack);(ROOT/('docs/cd-names-recovery.json' if prefix=='E' else 'docs/french-names-recovery.json')).write_text(json.dumps(dict(banks=banks,matches=report,pack_sha256=hashlib.sha256(pack).hexdigest(),evidence='AUDIO.OBJ LoadTeamSamples 0x11c6,0x11cb,0x11e8,0x1217,0x1241; PlayTeamSample 0x1560'),indent=2)+'\n');print('Imported',len(clips),'original player-name recordings')
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('iso',type=Path);p.add_argument('--language',choices=['english','french'],default='english');args=p.parse_args();recover(args.iso,args.language)
