"""Keeper and restart recovery. UI/referee boundaries are explicit excerpts."""
from restore_ball import routine
from team_recovery import block
from omf import Object
import re, struct, json

def extend(source, selected):
    def read(file,name):
        body,line=routine((source/file).read_text(encoding='latin1'),name)
        return body,line
    def add(file,name,body=None,line=None):
        if body is None:body,line=read(file,name)
        selected[:]=[s for s in selected if s[1]!=name]
        selected.append((file,name,body,line))
    for name in ['got_ball','find_zonal_target','close_angle','set_there_flags','centre_pos','gkick_pos','corner_pos','throw_in_pos']:
        add('INTELL.CPP',name)
    body,line=read('INTELL.CPP','get_target')
    body=body.replace('[(32*2)+6]','[70]')
    for call in ['tunnel_pos(flag,pn,x,y);','dfkick_pos(flag,pn,flag2,x,y,taker);','fkick_pos(flag,pn,flag2,x,y,taker);','penalty_pos(flag,pn,flag2,x,y,taker);']:
        body=body.replace(call,'throw std::logic_error("Restart outside recovered match modes");')
    # Corner/centre rows are discrete formations, never interpolated with live zones.
    body=body.replace('if ((analogue) && (!ball_out_of_play) &&','if (bz<64 && (analogue) && (!ball_out_of_play) &&')
    add('INTELL.CPP','get_target',body,line)
    for name in ['init_pickup_act','pickup_action','init_pickup_anim','init_throw_act']:
        add('ACTIONS.CPP',name)
    body,line=read('ACTIONS.CPP','throw_action')
    body='void throw_action(match_player *player) {\n'+body[body.index('if (user_controlled)'):]
    body=body.replace('user_throw(player);','throw std::logic_error("Native restarts use original CPU throw decisions");')
    add('ACTIONS.CPP','throw_action',body,line)
    for name in ['bounds_rules','throw_taker','get_taker','centre_takers','decide_set_kick']:
        body,line=read('RULES.CPP',name)
        if name=='centre_takers':body=body.replace('for (int pn=p;', 'int pn; for (pn=p;')
        if name=='get_taker':body=body.replace('int d,taker;', 'int d,taker=p;')
        add('RULES.CPP',name,body,line)
    add('BALL.CPP','reset_ball')
    score='void record_goal_score(int end) {\nif(end>0){\n'
    for name in ['go_right_goal','go_left_goal']:
        body,line=read('BALL.CPP',name)
        if name=='go_left_goal':score+='else{\n'
        score+=body[body.index('last_goal='):body.index('if (penalty_game)')]+'}\n}\n'
    add('BALL.CPP','record_goal_score',score+'}',line)
    # Keep original restart coordinates, tactics rows and taker selection; omit
    # menu/audio/referee/user registry side effects from these setup excerpts.
    for name,start,end in [('init_centre','reset_ball();','game_action=1;'),('init_corner','user_taker=0;','if (corner_taker>11)'),('init_gkick','dead_ball_cnt=100;','main_man=gkick_taker;'),('init_throw','stop_clock=TRUE;','if (throw_in_taker>11)')]:
        body,line=read('RULES.CPP',name)
        part=body[body.index(start):body.index(end)]
        if name=='init_corner':part='PlayCommentaryMessage(SP_CORNER);\n'+part
        if name=='init_gkick':part=body[body.index('{')+1:body.index(start)]+part
        part=re.sub(r'\s*do_\w+_menu\([^;]*;', '', part)
        part=re.sub(r'\s*refs\[0\]\.goto_x=[^;]*;', '', part)
        tails={'init_centre':'game_action=1;main_man=centre_guy_1;',
               'init_corner':'main_man=corner_taker;last_touch=main_man;pre_kp_touch=last_touch;ball_in_hands=FALSE;',
               'init_gkick':'',
               'init_throw':'main_man=throw_in_taker;'}
        if name=='init_gkick':tails[name]='main_man=gkick_taker;last_touch=main_man;pre_kp_touch=last_touch;user_taker=0;\n'+body[body.index('float ang='):body.rfind('}')]
        add('RULES.CPP',name,'void '+name+'() {\n'+part+tails[name]+'\n}',line)

def recover(source):
    objs={name:Object(source/(name+'.OBJ')) for name in ['INTELL','ACTIONS','RULES']}
    defs=[];evidence=[]
    for name,obj,seg,off,fmt,value in [
        ('SWAP_ENDS','INTELL',1,0x8a03,'B',19),('CLOSE_ANG_FACTOR','INTELL',2,0x33c,'f',16),('KP_OFFLINE','INTELL',1,0x8ad8,'f',16),
        ('BESIDE_BALL_TRIPLE','INTELL',1,0x7188,'f',15),('PEN_RUNUP_PLUS_FOUR','INTELL',1,0x71b7,'f',34),
        ('MC_PICKBALL_FS','ACTIONS',1,0xbf51,'f',0.04651162773370743),
        ('SETP_CORNER','RULES',1,0x54a1,'B',1),('SETP_CENTRE','RULES',1,0x54ef,'B',3),
        ('SETP_GKICK','RULES',1,0x551d,'B',7),('SETP_THROW','RULES',1,0x554b,'B',2),
        ('SETP_IFKICK','RULES',1,0x55b6,'B',5),('SETP_DFKICK','RULES',1,0x5604,'B',6),('SETP_PENALTY','RULES',1,0x5652,'B',4)]:
        raw=bytes(objs[obj].segments[seg]['data'][off:off+struct.calcsize(fmt)])
        found=struct.unpack('<'+fmt,raw)[0]
        if abs(found-value)>1e-7:raise ValueError(f'Unverified match constant {name}: {found}')
        defs.append(f'constexpr {"int" if fmt=="B" else "double"} {name}={found!r};')
        evidence.append(dict(name=name,object=obj+'.OBJ',segment=seg,offset=off,bytes=raw.hex(),value=found))
    defs.append('''
#define CLOSE_ANG_DIST (prat*CLOSE_ANG_FACTOR)
#define BESIDE_BALL (BESIDE_BALL_TRIPLE/3)
#define PEN_RUNUP_DIST (PEN_RUNUP_PLUS_FOUR-4)
constexpr int GOAL_KICK_BL=8,GOAL_KICK_TR=9,THROW_IN_A=11;
''')
    return '\n'.join(defs),evidence

def update_report(path):
    r=json.loads(path.read_text())
    for key in ['got_ball','find_zonal_target']:r['excerpt_boundaries'].pop(key,None)
    r['excerpt_boundaries'].update(get_target='Original positioning with unsupported foul/penalty/tunnel calls rejected; disable live interpolation for discrete set-piece rows',throw_action='Original CPU throw logic; referee/audio prefix excluded, human throw input rejected',init_centre='Original ball/formation/taker setup, no clock/menu/referee callbacks',init_corner='Original position, formation and taker setup, no menu/referee/user callbacks',init_gkick='Original position, taker and run-up setup, no menu/referee/user callbacks',init_throw='Original position and taker setup, no menu/referee/user/audio callbacks',dispatch_action='Thirteen original action cases, now including throw and pickup')
    r['native_team_boundary']='22-player scheduling; native dead-ball timing and formation placement call recovered restart setup, positions, decisions and animation contacts. Original boundary rules choose restart side. Score HUD counts original goal-crossing events. Referees, fouls, offside, match clock/halves and sound playback remain separate.'
    r['excerpt_boundaries']['record_goal_score']='Original last_goal and normal-play score increments from go_left_goal/go_right_goal; called once per live goal crossing, without menu/celebration callbacks'
    r['native_goal_boundary']='go_left_goal/go_right_goal latch the original collision event. The live scheduler calls recovered record_goal_score once and selects the conceding-side centre restart. Commentary-only original goal classification runs before reset_shot; replay fixtures keep their isolated goal latch.'
    r['compatibility_adaptations'] += [dict(routine='centre_takers',change='Hoist pn to preserve Watcom loop scope'),dict(routine='get_taker',change='Initialize fallback taker to enabled team first slot'),dict(routine='get_target',change='Discrete rows 64..69 do not enter live-zone interpolation')]
    path.write_text(json.dumps(r,indent=2)+'\n')
