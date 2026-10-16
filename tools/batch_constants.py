"""Attack/save constants from pinned Watcom objects; offsets are segment-relative."""
import struct

def recover(actions,ballint,intell,ball):
    definitions=[]; evidence=[]
    objects={'ACTIONS':actions,'BALLINT':ballint,'INTELL':intell,'BALL':ball}
    def number(name,obj,seg,offset,fmt='d'):
        raw=bytes(objects[obj].segments[seg]['data'][offset:offset+struct.calcsize(fmt)])
        value=struct.unpack('<'+fmt,raw)[0]
        definitions.append(f'constexpr {"int" if fmt in ("B","i","h") else "double"} {name}={value!r};')
        evidence.append(dict(name=name,object=obj+'.OBJ',segment=seg,offset=offset,format=fmt,bytes=raw.hex(),value=value))
    # Each zone branch compares arrival time, then multiplies its animation rate.
    for label,offset in [('AFOOTB',0x50),('AFOOTC',0x70),('ABODYB',0x98),('ABODYC',0xb8),('AHEADB',0xe0),('AHEADC',0x100),('AJUMPB',0x128),('AJUMPC',0x148),('BFOOTB',0x168),('BFOOTC',0x188),('BBODYB',0x1b0),('BBODYC',0x1d0),('BHEADB',0x1f8),('BHEADC',0x218),('BJUMPB',0x240),('CFOOTB',0x260),('CFOOTC',0x280),('CBODYB',0x2a8),('CBODYC',0x2c8),('CHEADB',0x2f0),('CJUMPB',0x318)]:
        number('SAVE_'+label+'_TIME','INTELL',2,offset)
        number('MC_'+label+'_FS','INTELL',2,offset+16)
    for name,offset in [('MC_GETUP_FS',0xb865),('MC_GETUPF_FS',0xb824),('MC_STANDB_FS',0x9da7),('MC_KICKOUT_FS',0xb954)]:
        assert bytes(actions.segments[1]['data'][offset-3:offset])==b'\xc7\x40\x73'
        number(name,'ACTIONS',1,offset,'f')
    for name,offset in [('HEADER_FACTOR',0x425),('PASS_FACTOR',0x93d),('KICK_AHEAD_ANG',0x951),('FACING_ANGLE',0x28),('DANGER_FACTOR',0x655),('KP_PASS_FACTOR',0x999),('OPTIMUM_FACTOR',0x9b1),('MAX_XY_CSW',0x857),('MAX_Z_CSW',0x857),('MAX_XY_USW',0x8d7),('MAX_Z_USW',0x8e7),('SW_HOLD_FACTOR',0x877),('CHEST_FACTOR',0x90),('HEAD_FACTOR',0xd8),('JUMP_FACTOR',0x120),('ZONE_B_FACTOR',0x401),('ZONE_C_FACTOR',0x409)]:number(name,'INTELL',2,offset)
    number('DIAG','INTELL',2,0x68d)
    number('VERT_BALL_DAMP','BALLINT',2,0xda)
    number('TOUCHB_BOX','BALLINT',2,0x162,'f')
    number('PM_BLOCK','BALLINT',1,0x2030,'i')
    number('PM_CATCH','BALLINT',1,0x20bc,'i')
    number('SAVE_CONTACT','BALLINT',2,0xc2)
    number('SAVE_DECEL','ACTIONS',2,0x327)
    number('DRIB_FACTOR','BALLINT',2,0x192)
    number('MEGA_FACTOR','BALLINT',2,0x19a)
    number('LONG_RANGE_FACTOR','BALL',2,0xd0,'f')
    # Rebound multipliers and commentary IDs in the pinned BALL.OBJ.
    for offset,encoding in [(0x1751,'dc0d24010000'),(0x1876,'dc0d3c010000'),
                            (0x179a,'b822000000'),(0x19ba,'b823000000')]:
        if bytes(ball.segments[1]['data'][offset:offset+len(bytes.fromhex(encoding))])!=bytes.fromhex(encoding):
            raise ValueError('Unverified goal-collision instruction at '+hex(offset))
    if bytes(ball.segments[2]['data'][0x124:0x12c])!=bytes(ball.segments[2]['data'][0x13c:0x144]):
        raise ValueError('Post and bar rebound factors disagree')
    number('REBOUND_FACTOR','BALL',2,0x124)
    number('PM_POST','BALL',1,0x179b,'i')
    number('PM_CROSSBAR','BALL',1,0x19bb,'i')
    # Immediate operands verified in init_first_time_act/init_save_act and decision branches.
    for name,obj,offset,fmt in [('STRIKE_ACT','ACTIONS',0x7b87,'B'),('I_SAVE_WAIT','ACTIONS',0x5083,'B'),('VISION_MULT','INTELL',0xe2f2,'B'),('GREED_FACTOR','INTELL',0xec54,'B'),('MIN_PUNT_PROB','INTELL',0xca1e,'B'),('MIN_SHOOT_PROB','INTELL',0xdcb1,'B')]:
        number(name,obj,1,offset,fmt)
    for name,offset in [('MC_STOS_FS',0xbc39),('MC_STOSB_FS',0xbc93),('MC_CRTOSTB_FS',0xbb5c),('MC_BOUNCE_FS',0xb913),('MC_HALFVOLL_FS',0xb995),('MC_THROW_FS',0xb9d6),('MC_ROLL_FS',0xba17),('MC_WAVEUP_FS',0xba99),('MC_THROWL_FS',0xbada),('MC_THROWS_FS',0xbb1b),('MC_DIAGPASS_FS',0xac3d),('MC_DIAGBACK_FS',0xac49),('MC_SIDEPASS_FS',0xac55)]:
        assert bytes(actions.segments[1]['data'][offset-3:offset])==b'\xc7\x40\x73'
        number(name,'ACTIONS',1,offset,'f')
    number('I_KP_BUSY','ACTIONS',1,0x6d6e,'h')
    number('CP_HEAD','INTELL',1,0xbdf4,'i')
    evidence.append(dict(name='TOO_CLOSE_TO_GOAL',object='ACTIONS.OBJ',routine='kphold_action',instructions=['0x6cab fcomp prat','0x6cc7 fsub prat'],expression='prat'))
    definitions.append('#define TOO_CLOSE_TO_GOAL (prat)\n')
    definitions.append('''
constexpr int GOAL_KICK_BR=10,GOAL_KICK_TL=7,MAX_PASS_PROB=5,MIN_PASS_PROB=125;
// Literal definitions immediately before their original routine in preserved source.
constexpr int DIVE_POINT=3;
constexpr double JUMP_POINT=.1;
#define MIN_HEADER_RNG (prat*HEADER_FACTOR)
#define MIN_PASS_DIST (prat*PASS_FACTOR)
#define DANGER_AREA (prat*DANGER_FACTOR)
#define KP_MIN_PASS_DIST (prat*KP_PASS_FACTOR)
#define OPTIMUM_PASS (prat*OPTIMUM_FACTOR)
#define LONG_RANGE (prat*LONG_RANGE_FACTOR)
#define DRIB_DANGER_AREA (prat*DRIB_FACTOR)
#define MEGA_DANGER_AREA (prat*MEGA_FACTOR)
#define SAVE_FEET_HGT (prat)
#define SAVE_CHEST_HGT (prat*CHEST_FACTOR)
#define SAVE_HEAD_HGT (prat*HEAD_FACTOR)
#define SAVE_JUMP_HGT (prat*JUMP_FACTOR)
#define SELECTION_CIRCLE (prat*10)
#define SAVE_ZONE_A (prat)
#define SAVE_ZONE_B (prat*ZONE_B_FACTOR)
#define SAVE_ZONE_C (prat*ZONE_C_FACTOR)
''')
    return '\n'.join(definitions),evidence
