"""Recover category and fixed period mappings from pinned AUDIO.H / AUDIO.OBJ."""
from pathlib import Path
import hashlib,json,re,struct,sys
from omf import Object

def recover(source):
 digest=hashlib.sha256((source/'AUDIO.OBJ').read_bytes()).hexdigest()
 if digest!='3bd8d90d2cef292aeda77ef86a9aee490ddb9dfbc0f2adef1fd493f4a5da33f9':raise ValueError('Unsupported AUDIO.OBJ')
 header=re.sub(r'//[^\n]*','',(source/'AUDIO.H').read_text())
 names=re.search(r'enum\s*\{\s*(CM_NULL.*?)\}',header,re.S)[1]
 names=[s.strip() for s in names.split(',') if s.strip()]
 assert names[1]=='SP_CORNER' and names[35]=='PM_CROSSBAR' and names[42:44]==['TI_HALFTIME','TI_FULLTIME']
 obj=Object(source/'AUDIO.OBJ');symbol=obj.named('CommentaryCategories');data=obj.segments[symbol['segment']]['data'];code=obj.segments[1]['data']
 rows=[]
 for i,name in enumerate(names[1:36]):
  a=symbol['offset']+i*6;bank=bytes(data[a:a+4]).rstrip(b'\0').decode('ascii');count=struct.unpack_from('<h',data,a+4)[0]
  expected=int(re.search(r'#define\s+MAX_'+name+r'\s+(\d+)',header)[1])
  assert count==expected and len(bank)==3
  rows.append(dict(id=i+1,event=name,bank=bank,count=count))
 # Loader loops 35 entries, advances category by six and bank by sixteen bytes.
 assert bytes(code[0x2d28:0x2d2e])==bytes.fromhex('83c50683c610')
 assert bytes(code[0x2d37:0x2d3a])==bytes.fromhex('83ff23')
 # Dispatch 42 -> 1e67 (TIM10); 43 -> 1db9 (TIM03).
 for at,value in [(0x1f2f,'83fb2a'),(0x1f3a,'83fb2b'),(0x1e9d,'6a0a'),(0x1de9,'6a03')]:assert bytes(code[at:at+len(bytes.fromhex(value))]).hex()==value
 const=obj.segments[2]['data']
 assert bytes(const[0x1d7:0x1db])==b'TIM\0' and bytes(const[0x1e0:0x1e4])==b'TIM\0'
 rows += [dict(id=42,event='TI_HALFTIME',bank='TIM',numbers=[10]),dict(id=43,event='TI_FULLTIME',bank='TIM',numbers=[3])]
 # Verify original welcome linking: 19 MCA variants; bank[1] Y, bank[0] X; LIFO playback.
 assert bytes(code[0x166f:0x1674])==bytes.fromhex('b813000000')
 assert bytes(code[0x16e4:0x16ee])==bytes.fromhex('69057402000068030000')
 assert bytes(const[0x19f:0x1a1])==b'Y\0' and bytes(const[0x1a4:0x1a6])==b'X\0'
 assert bytes(code[0x34e2:0x34e5])==bytes.fromhex('8b04ad') and code[0x3516]==0x4a
 # PlayScoreSample guards home<=7, away<=5, sorts larger first, and builds E_SCOxy.WAV.
 for at,value in [(0x25f2,'83f807'),(0x25fb,'83fd05'),(0x2622,'39cd7e10'),(0x2879,'89ee'),(0x289d,'8b3424')]:
  assert bytes(code[at:at+len(bytes.fromhex(value))]).hex()==value
 assert bytes(const[0x215:0x219])==b'SCO\0' and bytes(const[0x220:0x224])==b'SCO\0'
 return dict(audio_obj_sha256=digest,categories=rows,enum=names,score_evidence=dict(category=44,bank="SCO",home_max=7,away_max=5,order="larger digit first",dispatch="0x25f2,0x25fb,0x2622",filename="0x2879 first digit, 0x289d second digit"),period_evidence={'half_time':{'branch':'0x1f34 -> 0x1e67','number_instruction':'0x1e9d push 10'},'full_time':{'branch':'0x1f3d -> 0x1db9','number_instruction':'0x1de9 push 3'}})
if __name__=='__main__':
 source,output=map(Path,sys.argv[1:]);output.write_text(json.dumps(recover(source),indent=2)+'\n')
