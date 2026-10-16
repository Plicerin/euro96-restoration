"""Missing action constants recovered from the pinned ACTIONS/BALLINT/INTELL objects.
DATA.H still supplies animation IDs and contact fractions. No source file is edited.
"""
import re,struct

def recover(source,actions,ballint,intell):
 text=(source/'DATA.H').read_text(encoding='latin1')
 definitions=[]
 for line in text.splitlines():
  m=re.match(r'\s*#define\s+(MC\w+)\s+([^/]*(?:/[^/]+)?)',line.split('//')[0])
  if m and m[1]!='MC_NO':definitions.append('#define '+m[1]+' '+m[2].strip())
 evidence=[]
 def number(name,obj,segment,offset,fmt):
  raw=bytes(obj.segments[segment]['data'][offset:offset+struct.calcsize(fmt)])
  value=struct.unpack('<'+fmt,raw)[0]
  # Byte immediates are action/message IDs, including switch case labels.
  # MSVC accepted the old double declaration; standard C++/Clang rejects it.
  definitions.append(f'constexpr {"int" if fmt == "B" else "double"} {name}={value!r};')
  evidence.append(dict(name=name,object="ACTIONS.OBJ" if obj is actions else "BALLINT.OBJ" if obj is ballint else "INTELL.OBJ",segment=segment,offset=offset,format=fmt,bytes=raw.hex(),value=value))
 # Float assignments at these instruction immediates; double multipliers in CONST.
 for name,offset in [('MC_STAND_FS',0x9cd3),('MC_PASS_FS',0xa881),('MC_CHIP_FS',0xa890),('MC_BACKHEEL_FS',0xa89f),('MC_SHOOT_FS',0xa8ae),('MC_CROSS_FS',0xa8ba)]:
  # All stores use c7 40 73 imm32.
  if bytes(actions.segments[1]['data'][offset-3:offset])!=b'\xc7\x40\x73':raise ValueError('Animation instruction mismatch '+name)
  number(name,actions,1,offset,'f')
 for name,offset in [('MC_TRAP_FS',0x5cb),('MC_CHEST_FS',0x6ab),('MC_D_HEAD_FS',0x6b3),('MC_PASS_DIST',0x27b)]:number(name,actions,2,offset,'d')
 for name,offset in [('MC_VOLLEY_FS',0x18a8),('MC_OVERHEAD_FS',0x18b7),('MC_DV_HEAD_FS',0x18c6),('MC_S_HEAD_FS',0x18d5),('MC_J_HEAD_FS',0x18e4)]:number(name,ballint,1,offset,'f')
 number('LONG_PASS_FACTOR',intell,2,0x6f1,'f')
 number('TACKLE_DECEL',ballint,2,0x18a,'d')
 for name,offset in [('MC_RUN_SPD',0x3bb),('MC_RUN_FS',0x3c3),('MC_TROTA_FS',0x6bb)]:number(name,actions,2,offset,'d')
 number('IM_THERE_DIST',actions,1,0x3f61,'f')
 number('STEP_RANGE_FACTOR',actions,2,0x1ac,'d')
 number('MIN_SHOOT_FACTOR',intell,2,0x473,'d')
 # MAX_TURN2's statistical turn formula and action/communication IDs.
 for name,obj,segment,offset,fmt,expected in [
  ('MAX_TURN_RATE_FACTOR',intell,2,0,'f',.125),('MAX_TURN_BASE',intell,2,4,'f',14),
  ('MAX_TURN_PI',intell,2,8,'d',3.1415926536),('MAX_TURN_DEGREES',intell,2,0x10,'d',180),
  ('TURN_ACT',intell,1,0x137,'B',2),('STOP_ACT',actions,1,0x706b,'B',20),
  ('SAY_PASS',actions,1,0x844d,'B',2),('SAY_CROSS',actions,1,0x845e,'B',6)]:
  value=struct.unpack_from('<'+fmt,obj.segments[segment]['data'],offset)[0]
  if value!=expected:raise ValueError('Interception constant mismatch '+name)
  number(name,obj,segment,offset,fmt)
 if struct.unpack_from('<d',actions.segments[2]['data'],0x5d3)[0]!=72.001:raise ValueError('Animation comparison tolerance changed')
 definitions += ['''
constexpr int KICK_ACT=15,WAIT_ACT=18,I_KICKED=12,I_RECEIVE=13;
constexpr int KT_PASS=1,KT_PUNT=2,KT_SHOOT=3,REAL_SPEED=20;
constexpr double MC_RUNWITHB_FS=1.0/15;
constexpr int NORMAL_PLAY=0,CORNER_TL=1,CORNER_TR=2,CORNER_BL=3,CORNER_BR=4,CENTRE_A=5,CENTRE_B=6;
constexpr int IF_KICK_A=13,IF_KICK_B=14,DF_KICK_A=15,DF_KICK_B=16,PEN_KICK_A=17,PEN_KICK_B=18;
constexpr int SPACT_FPASS=1,SPACT_HPASS=2,SPACT_TEE_UP=3,SPACT_DPASS=4,SPACT_GRND=5,SPACT_CHIP=6;
constexpr int PS_LONGWHISTLE=1,PS_SHORTWHISTLE=2;
constexpr int PS_SOFTKICK=5,PS_MEDKICK=6,PS_HARDKICK=7;
constexpr double PASS_ACC_ARC=0.1,D2RAD=3.1415926536/180;
#define LONG_PASS_DIST (prat*LONG_PASS_FACTOR)
#define STEP_RANGE (prat*STEP_RANGE_FACTOR)
#define MIN_SHOOT_DIST (prat*MIN_SHOOT_FACTOR)
#define AFEQS(a,b) (std::abs(int(a))>(b)-0.001 && std::abs(int(a))<(b)+0.001)
''']
 return '\n'.join(definitions),evidence
