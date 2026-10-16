"""Optional restoration check: compare native player movement with preserved x86 code in Unicorn.
Install unicorn into an isolated tool environment, never into the game's runtime.
Only math instructions are supplied as library shims. Unknown executed imports fail.
"""
from pathlib import Path
import argparse,csv,io,json,struct,subprocess,sys,math,hashlib
from omf import Object

def compare(source,executable):
 from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE
 from unicorn.x86_const import UC_X86_REG_ESP,UC_X86_REG_EIP,UC_X86_REG_EAX
 objects={n:Object(source/(n+'.OBJ')) for n in ['ACTIONS','MATHS','FOOTBALL','RULES']}
 uc=Uc(UC_ARCH_X86,UC_MODE_32);uc.mem_map(0x100000,0x1000000)
 bases={};symbols={};next_base=0x100000
 for module,obj in objects.items():
  for index,seg in enumerate(obj.segments):
   if seg and seg['name'] in ['_TEXT','_DATA','_BSS','CONST','CONST2']:
    bases[module,index]=next_base;uc.mem_write(next_base,bytes(seg['data']));next_base+=(max(1,seg['length'])+4095)&~4095
  for p in obj.publics:
   if (module,p['segment']) in bases:symbols[p['name']]=bases[module,p['segment']]+p['offset']
 # x87 trig/sqrt entry points take and return ST(0), as in the preserved Watcom ABI.
 math_shims={'IF@DSIN':b'\xd9\xfe\xc3','IF@DCOS':b'\xd9\xff\xc3','IF@DSQRT':b'\xd9\xfa\xc3'}
 unresolved={};extra=0x900000
 for obj in objects.values():
  for name in obj.externs[1:]:
   if name not in symbols:
    symbols[name]=extra;extra+=512
    if name in math_shims:uc.mem_write(symbols[name],math_shims[name])
    elif name.startswith('W?') and '(' not in name or name=='__chipbug':pass
    else:unresolved[symbols[name]]=name;uc.mem_write(symbols[name],b'\xcc')
 for module in ['ACTIONS','MATHS','RULES']:
  obj=objects[module]
  for f in obj.fixups(skip_comdat=True):
   if (module,f['segment']) not in bases:continue
   if f['kind']!=9:raise ValueError('Unsupported relocation '+str(f))
   loc=bases[module,f['segment']]+f['offset'];imm=struct.unpack('<I',uc.mem_read(loc,4))[0]
   if f['target_method']==2:target=symbols[obj.externs[f['target_index']]]
   elif f['target_method']==0:target=bases[module,f['target_index']]
   else:raise ValueError('Unsupported relocation target '+str(f))
   value=target+f['displacement']+imm-(loc+4 if f['relative'] else 0)
   uc.mem_write(loc,struct.pack('<I',value&0xffffffff))
 def hook(uc,address,size,user):
  if address in unresolved:raise RuntimeError('Executed unresolved import '+unresolved[address])
 uc.hook_add(UC_HOOK_CODE,hook)
 by_name={k.split('$')[0][2:]:v for k,v in symbols.items() if k.startswith('W?')}
 def put(name,value,fmt='f'):uc.mem_write(by_name[name],struct.pack('<'+fmt,value))
 def get(name,fmt='f'):return struct.unpack('<'+fmt,uc.mem_read(by_name[name],struct.calcsize(fmt)))[0]
 def call(name,player_address):
  stack=0x100ff00;end=0x100f000
  uc.mem_write(stack,struct.pack('<I',end));uc.reg_write(UC_X86_REG_ESP,stack)
  uc.reg_write(UC_X86_REG_EAX,player_address)
  uc.emu_start(by_name[name],end,count=50000)
  if uc.reg_read(UC_X86_REG_EIP)!=end:raise RuntimeError('Instruction budget exceeded')
 # Snapshot all loaded state before the first scenario.
 pristine=bytes(uc.mem_read(0x100000,0x1000000))
 trace=[]
 for rate in [0,32,64,96,127]:
  for mode in range(5):
   uc.mem_write(0x100000,pristine)
   uc.mem_write(0x100e000,b'\xdb\xe3');uc.emu_start(0x100e000,0x100e002)
   player=by_name['teams']+203
   def field(offset,value,fmt='f'):uc.mem_write(player+offset,struct.pack('<'+fmt,value))
   def readfield(offset,fmt='f'):return struct.unpack('<'+fmt,uc.mem_read(player+offset,struct.calcsize(fmt)))[0]
   field(0,2,'h');field(0x46,rate,'B');field(6,1);field(2,500);field(10,400);field(0xa8,1200);field(0xac,400)
   put('user_controlled',1,'b')
   if mode==1:put('ball_poss',2,'i')
   if mode==2:field(0xbd,1,'b');field(0xa0,2);field(0xa4,-1)
   if mode==3:field(0xbf,1,'h');field(0xc1,5,'h');put('ball_poss',2,'i')
   if mode==4:put('burst_timer',10,'h')
   for tick in range(1,21):
    call('go_forward',player)
    trace.append([rate,mode,tick]+[readfield(offset) for offset in [2,10,0xa0,0xa4]]+[readfield(0x61,'i')])
 native=subprocess.check_output([str(executable.resolve()),'--movement-trace'],text=True)
 rows=list(csv.reader(io.StringIO(native)))
 if len(rows)!=len(trace):raise AssertionError('Trace length mismatch')
 max_error=0
 for row,expected in zip(rows,trace):
  actual=list(map(float,row))
  if len(actual)!=8:raise AssertionError('Trace field count mismatch')
  for i,(a,b) in enumerate(zip(actual,expected)):
   error=abs(a-b);max_error=max(max_error,error)
   tolerance=0 if i in (0,1,2,7) else 0.002
   if not math.isfinite(a) or not math.isfinite(b) or error>tolerance:raise AssertionError(f'Divergence {expected[:3]} field {i}: native {a}, original {b}')
 return dict(inputs={name:hashlib.sha256((source/name).read_bytes()).hexdigest() for name in ['ACTIONS.OBJ','MATHS.OBJ','FOOTBALL.OBJ','RULES.OBJ']},native_executable_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),scenarios=25,ticks=500,max_absolute_error=max_error,tolerance=0.002,
 scope='Five player speeds, each tested free running, possession, sidestepping, interception and burst. Twenty movement steps per scenario. Position, displacement, and work counter.',
 limitations='Tests original movement routines, not the native practice steering, assisted receiving or diagnostic kicks. COMDAT code omitted; no omitted code is invoked.')

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('executable',type=Path);p.add_argument('--report',type=Path);a=p.parse_args()
 result=compare(a.source,a.executable);s=json.dumps(result,indent=2);print(s)
 if a.report:a.report.write_text(s+'\n',encoding='utf-8')
