"""Import verified original CD team textures; keep the Euro archive untouched."""
from pathlib import Path
import argparse,hashlib,json,re,struct,subprocess
from omf import Object
ROOT=Path(__file__).resolve().parents[1]
EXPECTED={"SOCCER.EXE":"cc6e2547c9f97050e1d610ba242476b7d3147f4a7a2e9a0284d6585691137543","ACTREND.DAT":"843c6da5abe934b547248ce45e0179ddb9518cff07729ff919cfc8a7c8464d7e","ACTREND.OFF":"af154fe774eb2386628457c79e53738dd990d09083a122761848f6dcbc4b3f9f"}
PARTS=['head','torso','limbs','numbers','kit_palette','skin_palette','home_crowd','away_crowd']
SIZES=[32768,65536,19968,13824,72,24,24,24]
def records(data,offsets):
 if not offsets or len(offsets)%8:raise ValueError('Invalid graphics offsets')
 result=[]
 for at,n in struct.iter_unpack('<II',offsets):
  if at>len(data) or n>len(data)-at:raise ValueError('Graphics record outside archive')
  result.append(data[at:at+n])
 return result
def recover(iso):
 inputs={}
 for name,digest in EXPECTED.items():
  data=subprocess.run(['7z','e','-so',str(iso),name],capture_output=True,check=True).stdout
  if hashlib.sha256(data).hexdigest()!=digest:raise ValueError('Unsupported CD graphics revision: '+name)
  inputs[name]=data
 cd=records(inputs['ACTREND.DAT'],inputs['ACTREND.OFF']);assert len(cd)==439
 text=(ROOT/'SOURCE/FILES.C').read_text(encoding='latin1')
 rows=re.findall(r'\{([^{}]+)\}\s*,?\s*//\s*\d+\s*:\s*([^\r\n]+)',text)[:47]
 assert len(rows)==47
 table=1255764;symbols={};kits=[];exe=inputs['SOCCER.EXE']
 for i,(row,name) in enumerate(rows):
  ids=struct.unpack_from('<8I',exe,table+i*32);macros=[m.strip() for m in row.split(',')];assert len(macros)==8
  for macro,value,size in zip(macros,ids,SIZES):
   assert value%8==0 and value//8<len(cd) and len(cd[value//8])==size
   assert macro not in symbols or symbols[macro]==value
   symbols[macro]=value
  # The adjacent, independently verified CD gameplay database establishes country order.
  if i<45:
   country=exe[1031292+i*872:1031292+i*872+22].split(b'\0')[0].decode('ascii')
   normalize=lambda s:re.sub('[^A-Z]','',s.upper()).replace('COLUMBIA','COLOMBIA')
   assert normalize(name)==normalize(country),(name,country)
  kits.append((name,[v//8 for v in ids]))
 # Every byte of all 128 known source kit components must match, not just dimensions.
 euro=records((ROOT/'SOURCE/EUROREND.DAT').read_bytes(),(ROOT/'SOURCE/EUROREND.OFF').read_bytes())
 obj=Object(ROOT/'SOURCE/3DENG.OBJ');symbol=obj.named('teamlist')
 assert hashlib.sha256((ROOT/'SOURCE/3DENG.OBJ').read_bytes()).hexdigest()==json.loads((ROOT/'docs/graphics-resource-map.json').read_text())['object_sha256']
 for i in range(16):
  old=struct.unpack_from('<8I',obj.segments[symbol['segment']]['data'],symbol['offset']+i*32)
  for before,after in zip(old,kits[i][1]):assert euro[before//8]==cd[after]
 countries=re.findall(r't:\s*"([^"]+)"\s+form:\s*[\d-]+',(ROOT/'SOURCE/SCRIPT.96').read_text(encoding='latin1'))
 assert len(countries)==16
 catalog={normalize(n):(i,ids) for i,(n,ids) in enumerate(kits[:45])};chunks=[];report=[]
 for team,name in enumerate(countries):
  key=normalize(name)
  if key not in catalog:report.append(dict(team=team,name=name,available=False));continue
  index,ids=catalog[key];parts=[cd[i] for i in ids]
  assert all(max(b)<=63 for b in parts[4:])
  chunks.append(struct.pack('<I',team)+b''.join(struct.pack('<I',len(b))+b for b in parts))
  report.append(dict(team=team,name=name,available=True,cd_team=index,descriptor_offset=table+index*32,parts=[dict(part=n,record=i,size=len(b),sha256=hashlib.sha256(b).hexdigest()) for n,i,b in zip(PARTS,ids,parts)]))
 assert len(chunks)==14
 # Original CD clash matrix, independently corroborated by the older commented source table.
 defs=(ROOT/'SOURCE/DEFINES.H').read_text(encoding='latin1')
 codes={n:int(v) for n,v in re.findall(r'#define\s+([A-Z]{3})\s+(\d+)\b',defs)}
 old=(ROOT/'SOURCE/GAMEDATA.CPP').read_text(encoding='latin1').split('team_b_kit[]={')[1].split('};')[0]
 old=[codes[n] for n in re.findall(r'\b[A-Z]{3}\b',old)]
 clash=list(struct.unpack_from('<2025I',exe,1108816))
 assert len(old)==2025 and [(i,a,b) for i,(a,b) in enumerate(zip(old,clash)) if a!=b]==[(1423,28,46)]
 assert all(v in (i%45,45,46) for i,v in enumerate(clash))
 choices=bytearray(256);appearances=bytearray([255]*320);appearance_report=[]
 country_ids={r['team']:r['cd_team'] for r in report if r['available']}
 for home,h in country_ids.items():
  for away,a in country_ids.items():
   value=clash[h*45+a];choices[home*16+away]=value-44 if value>=45 else 0
 for team,index in country_ids.items():
  cd_players=[]
  for slot in range(22):
   at=1031292+index*872+144+slot*33;b=exe[at:at+33]
   name=b[:20].split(b'\0')[0].decode('latin1')
   assert b[30]==slot and b[31] in range(4) and b[32] in (0,1)
   cd_players.append((name,b[32],at+32))
  script=(ROOT/'SOURCE/SCRIPT.96').read_text(encoding='latin1')
  roster=re.findall(r'^\s*t:\s*"([^"]+)"[^\n]*pace:[^\n]*name:',script[script.index('PLYR_NMES'):script.index('TEAM_NMES')],re.M)
  assert len(roster)==320
  norm=lambda n:re.sub('[^A-Z]','',n.upper())
  for slot,name in enumerate(roster[team*20:(team+1)*20]):
   # Initial plus exact surname only; no nationality-based guesses or fuzzy matches.
   parts=name.split();key=norm(parts[0][0]+''.join(parts[1:]))
   matches=[p for p in cd_players if norm(p[0])==key]
   if len(matches)==1:
    old_name,flag,at=matches[0];appearances[team*20+slot]=flag
    appearance_report.append(dict(team=team,slot=slot+1,name=name,cd_name=old_name,flag=flag,offset=at))
 alternate_assets=[]
 for team,index in [(16,45),(17,46)]:
  ids=kits[index][1];parts=[cd[i] for i in ids]
  alternate_assets.append(dict(pack_team=team,cd_team=index,descriptor_offset=table+index*32,parts=[dict(part=n,record=i,size=len(b),sha256=hashlib.sha256(b).hexdigest()) for n,i,b in zip(PARTS,ids,parts)]))
  chunks.append(struct.pack('<I',team)+b''.join(struct.pack('<I',len(b))+b for b in parts))
 pack=b'AC96KIT2'+struct.pack('<I',len(chunks))+b''.join(chunks)+choices+appearances
 target=ROOT/'local-data/GRAPHICS/TEAMS.BIN';target.parent.mkdir(parents=True,exist_ok=True)
 if target.exists() and target.read_bytes()!=pack:
  previous=json.loads((ROOT/'docs/cd-graphics-recovery.json').read_text())['pack_sha256']
  if hashlib.sha256(target.read_bytes()).hexdigest()!=previous:raise ValueError('Unrecognized installed kit pack')
 target.write_bytes(pack)
 result=dict(clash_offset=1108816,clash_source_matches=2024,alternate_kits=[45,46],alternate_assets=alternate_assets,appearance_matches=appearance_report,inputs=EXPECTED,table_offset=table,descriptor_stride=32,descriptors=47,verified_shared_components=128,pack_sha256=hashlib.sha256(pack).hexdigest(),pack_bytes=len(pack),teams=report)
 (ROOT/'docs/cd-graphics-recovery.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
 print(f'Imported {len(chunks)} verified original kits including two alternates, {len(pack)} bytes; 128 shared components match SOURCE exactly.')
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('iso',type=Path);recover(p.parse_args().iso.resolve(strict=True))
