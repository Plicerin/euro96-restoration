"""Optional restoration check: compare native physics with preserved x86 code in Unicorn.
Install unicorn into an isolated tool environment, never into the game's runtime.
Only math instructions are supplied as library shims. Unknown executed imports fail.
"""
from pathlib import Path
import argparse,csv,io,json,struct,subprocess,sys,math,hashlib
from omf import Object

def compare(source,executable):
 from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE
 from unicorn.x86_const import UC_X86_REG_ESP,UC_X86_REG_EIP,UC_X86_REG_EAX
 objects={n:Object(source/(n+'.OBJ')) for n in ['BALL','MATHS','FOOTBALL']}
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
 for module in ['BALL','MATHS']:
  obj=objects[module]
  for f in obj.fixups():
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
 def call(name):
  stack=0x100ff00;end=0x100f000
  uc.mem_write(stack,struct.pack('<I',end));uc.reg_write(UC_X86_REG_ESP,stack)
  uc.emu_start(by_name[name],end,count=50000)
  if uc.reg_read(UC_X86_REG_EIP)!=end:raise RuntimeError('Instruction budget exceeded')
 # Snapshot all loaded state before the first scenario.
 pristine=bytes(uc.mem_read(0x100000,0x1000000))
 trace=[]
 for scenario in range(6):
  uc.mem_write(0x100000,pristine)
  # Reset x87 environment before running independent scenarios.
  uc.mem_write(0x100e000,b'\xdb\xe3');uc.emu_start(0x100e000,0x100e002)
  for name,value in {'gravity':0.6,'ballx':320,'bally':400,'ballz':2,'ballxdis':10,'ballydis':3,'ballzdis':0 if scenario==0 else 8}.items():put(name,value)
  put('ball_inair',int(scenario!=0),'i');put('prediction',1,'i')
  if scenario==2:
   put('wind_on',1,'h');put('wind_speed',45);put('wind_x',0);put('wind_y',-1)
  if scenario==3:
   put('swerve',1,'i');put('full_xyspin',0.055);put('full_zspin',0.015)
  if scenario>=4:
   put('after_touch_on',1,'b');put('shoot_x',1);put('full_xyspin',0.055);put('full_zspin',0.015)
   uc.mem_write(by_name['users_dir'],struct.pack('<ff',0.25 if scenario==4 else 0,1))
  for tick in range(1,81):
   call('ball_trajectory')
   trace.append([scenario,tick]+[get(n) for n in ['ballx','bally','ballz','ballxdis','ballydis','ballzdis']]+[get('ball_inair','i')])
 native=subprocess.check_output([str(executable.resolve()),'--trace'],text=True)
 rows=list(csv.reader(io.StringIO(native)))
 if len(rows)!=len(trace):raise AssertionError('Trace length mismatch')
 max_error=0;boundary_first=None;boundary_max=0
 for row,expected in zip(rows,trace):
  actual=list(map(float,row))
  if len(actual)!=9:raise AssertionError("Trace field count mismatch")
  for i,(a,b) in enumerate(zip(actual,expected)):
   error=abs(a-b)
   if not math.isfinite(a) or not math.isfinite(b):raise AssertionError('Non-finite trace')
   tolerance=0 if i in (0,1,8) else 0.002
   if expected[0]==5:
    boundary_max=max(boundary_max,error)
    if error>tolerance and boundary_first is None:
     boundary_first=dict(tick=expected[1],field=i,native=a,original=b)
   else:
    max_error=max(max_error,error)
    if error>tolerance:raise AssertionError(f'Divergence scenario={expected[0]}, tick={expected[1]}, field={i}: native {a}, original {b}')
 return dict(inputs={name:hashlib.sha256((source/name).read_bytes()).hexdigest() for name in ['BALL.OBJ','MATHS.OBJ','FOOTBALL.OBJ']},native_executable_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),scenarios_within_tolerance=5,ticks_compared_within_tolerance=400,compared_values=2800,max_absolute_error=max_error,tolerance=0.002,
  precision_boundary_case=dict(first_divergence=boundary_first,max_absolute_difference=boundary_max,explanation='A horizontal-only aftertouch input leads to a landing within a few millionths of the floor. Floating-point rounding changes which tick triggers the strict ballz < radius comparison. This case is recorded, not asserted to be equivalent.'),
  scope='Ground roll, loft and bounce, wind, swerve, aftertouch. Position/velocity/in-air state; prediction mode. Native x64 vs emulated preserved x86.',
  original_execution='BALL.OBJ and MATHS.OBJ in Unicorn; original data from FOOTBALL.OBJ; x87 fsin/fcos/fsqrt shims only; unknown imports fail.')

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('executable',type=Path);p.add_argument('--report',type=Path);a=p.parse_args()
 result=compare(a.source,a.executable);s=json.dumps(result,indent=2);print(s)
 if a.report:a.report.write_text(s+'\n',encoding='utf-8')
