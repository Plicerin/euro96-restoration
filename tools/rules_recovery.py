"""Original regulation-match rules; native presentation boundaries are explicit."""
from restore_ball import routine
from team_recovery import block
from omf import Object
import re, struct, json

def extend(source, selected):
    def read(file, name):
        return routine((source/file).read_text(encoding='latin1'), name)
    def add(file, name, body=None, line=None):
        if body is None: body,line=read(file,name)
        selected[:]=[s for s in selected if s[1]!=name]
        selected.append((file,name,body,line))
    def presentation(body):
        # Retain branches, replacing only DOS UI/audio/user registration calls.
        body=re.sub(r'\b(?:do_\w+_menu|init_speech|send_off_plr)\([^;]*;', '(void)0;', body)
        body=re.sub(r'user_taker(?:2)?=user_\w+\([^;]*;', 'user_taker=0;',body)
        return body
    for name in ['offside_rule','dfkick_pos','fkick_pos','penalty_pos','facing_opp','player_ints']:
        body,line=read('INTELL.CPP',name)
        if name=='player_ints':
            # Current player_num is native scheduler state; fix the source's y-for-z typo.
            body=body.replace('z=player->tm_y-teams[i-1].tm_y;', 'z=player->tm_z-teams[i-1].tm_z;')
            body=body.replace('if (teams[i-1].tm_act<=TURN_ACT)', 'if (i!=player->tm_player && teams[i-1].guy_on && teams[i-1].tm_act<=TURN_ACT)')
        add('INTELL.CPP',name,presentation(body),line)
    body,line=read('INTELL.CPP','tunnel_pos')
    body=body.replace('stadlist[setup.stadium].tunlx','tunnel_x').replace('-stadlist[setup.stadium].tunlz','tunnel_y').replace('float tz=stadlist[setup.stadium].tunly;','')
    add('INTELL.CPP','tunnel_pos',body,line)
    body,line=read('INTELL.CPP','get_target')
    body=body.replace('[(32*2)+6]','[70]').replace('tunnel_pos(flag,pn,x,y);','throw std::logic_error("Tunnel targets run in the presentation copy");')
    body=body.replace('if ((analogue) && (!ball_out_of_play) &&','if (bz<64 && (analogue) && (!ball_out_of_play) &&')
    body=body.replace('float xoff,yoff;', 'float xoff=0,yoff=0;')
    add('INTELL.CPP','get_target',body,line)
    for name in ['init_fall','fall_action','init_jump_act','jump_action','init_fallr_anim','init_ride_anim','init_barge_anim','init_penalty_anim']:
        add('ACTIONS.CPP',name)
    for name in ['follow_play','lman_follow']:
        add('RULES.CPP',name)
    for name in ['init_offside','penalty','init_foul','punish_foul','retake_foul','init_fkick','init_dfkick','init_penalty','find_wall_guys','clear_wall','inc_pfoul','inc_dfoul','inc_ifoul','add_player_time','match_clock','stop']:
        body,line=read('RULES.CPP',name)
        body=presentation(body)
        if name=='init_foul':
            body=body.replace('char see)', 'char see=FALSE)')
            body=body.replace('d=(ref_accuracy/d)*2*man_down;', 'd=(ref_accuracy/(d<0.001f?0.001f:d))*2*man_down;')
        if name=='punish_foul':body=body.replace('init_match_mode();','++rule_events; if(offside_now)++offside_events; else ++foul_events;')
        if name=='init_penalty':
            body=re.sub(r'#ifdef EURO96\n(.*?)#else\n.*?#endif',r'\1',body,flags=re.S)
        if name=='find_wall_guys':
            body=body.replace('x=wguys[wall_cnt-2].x', 'if(wall_cnt<2){keeper_pos_x=guys>11?pitch_len-prat:prat;keeper_pos_y=cntspot_y;return;}\n\tx=wguys[wall_cnt-2].x')
            body=body.replace('gy=y*gx/x;', 'gy=std::abs(x)>.001f?y*gx/x:0;')
        add('RULES.CPP',name,body,line)
    body,line=read('RULES.CPP','swap_teams')
    body=body[:body.index('\n\tchar t=')]+ '\n}'
    body=body.replace('swap_users();','').replace('memcpy(', 'std::memcpy(')
    add('RULES.CPP','swap_teams',body,line)
    body,line=read('FOOTBALL.CPP','nothing_happening')
    # The native period transition emits one whistle only after this gate passes.
    # Loose source emits a whistle before testing shot_pending, potentially every tick.
    body=re.sub(r'\bplay_fx\([^;]*;', '(void)0;', body)
    add('FOOTBALL.CPP','nothing_happening',presentation(body),line)
    body,line=read('FOOTBALL.CPP','watch_match_time')
    a=body.index('if (match_time.min>=(45+injury_time)')
    b=body.index('if (match_time.min>=(90+injury_time)')
    half=body[a:body.index('\n',a)].strip()
    full=body[b:body.index('\n',b)].strip()
    add('FOOTBALL.CPP','regulation_period_end','int regulation_period_end() {\n'+half+' return 1;\n'+full+' return 2;\nreturn 0;\n}',line+body[:a].count('\n'))
    # Original advantage/release timers from process_flags, independent of old UI.
    text=(source/'FOOTBALL.CPP').read_text(encoding='latin1')
    a=text.index('\tif (ball_released)');b=text.index('\n\tif (just_thrown)',a)
    add('FOOTBALL.CPP','update_rule_flags','void update_rule_flags() {\n'+text[a:b]+'\n}',text[:a].count('\n')+1)

def recover(source):
    obj=Object(source/'ACTIONS.OBJ');defs=[];ev=[]
    for name,seg,off,fmt,value in [('FALL_RATE',2,0x1e1,'f',4),('MC_FALLR_FS',1,0xb741,'f',1/17),('MC_RIDE_FS',2,0x67b,'d',1/14),('MC_BARGE_FS',2,0x3af,'d',2/27),('MC_PENALTY_FS',1,0x9d25,'f',1/32)]:
        raw=bytes(obj.segments[seg]['data'][off:off+struct.calcsize(fmt)])
        found=struct.unpack('<'+fmt,raw)[0]
        if abs(found-value)>1e-7:raise ValueError(f'Unverified rule constant {name}: {found}')
        defs.append(f'constexpr double {name}={found!r};')
        ev.append(dict(name=name,object='ACTIONS.OBJ',segment=seg,offset=off,bytes=raw.hex(),value=found))
    raw=bytes(obj.segments[1]['data'][0x45a5:0x45a7])
    if raw!=b'\x04\x00':raise ValueError('Unverified jump action')
    defs.append('constexpr int JUMP_ACT=4;\n#define PLAYER_SIZE (prat)')
    ev.append(dict(name='JUMP_ACT',object='ACTIONS.OBJ',segment=1,offset=0x45a5,bytes=raw.hex(),value=4))
    ev.append(dict(name='PLAYER_SIZE',object='INTELL.OBJ',instruction='0xa3ec',value='prat',evidence='player_ints loads the prat relocation immediately after calc_dist for the contact-radius comparison'))
    return '\n'.join(defs),ev

def update_report(path):
    r=json.loads(path.read_text())
    r['excerpt_boundaries'].update(regulation_period_end='Original regulation 45/90-minute thresholds and nothing_happening gate; native half-time/results presentation replaces DOS menus. No extra time or shootout.',swap_teams='Original roster exchange, player-number reassignment and half increment; native input/material identity follows the swapped roster. Both sides use the same loaded tactics.',init_foul='Original referee visibility and advantage; audio/speech omitted, freekicks enabled by native regulation mode.',punish_foul='Original direct/indirect/penalty decision and taker selection; UI/audio/user registry/cards omitted, rule event replaces init_match_mode.',get_target='Original live and set-piece positioning; live tunnel dispatch stays guarded; original tunnel_pos runs on a native presentation copy, discrete formation rows excluded from interpolation.',dispatch_action='Fifteen original action cases, including falls and jumps')
    r['compatibility_adaptations'] += [dict(routine='player_ints',change='Fix y-for-z height difference; skip self and disabled slots'),dict(routine='init_foul',change='Protect zero referee-to-ball distance'),dict(routine='find_wall_guys',change='Guard fewer than two wall players and zero projection denominator'),dict(routine='get_target',change='Initialize offsets for special centre formation')]
    r['native_team_boundary']='Regulation clock and thresholds, roster swap, original offside, tackle contacts, foul/advantage and set-piece routines. Native fixed-step scheduler, half-time interval/results, dead-ball placement and invisible referee observation positions. Cards, referee rendering, substitutions, fatigue, extra time/shootouts and audio remain unconnected.'
    path.write_text(json.dumps(r,indent=2)+'\n')
