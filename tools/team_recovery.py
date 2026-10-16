"""Explicit live-play excerpts; preserved source and objects remain untouched."""
from restore_ball import routine
import struct
import json

def block(text,start):
    a=text.index('{',start);p=a+1;depth=1
    while depth:
        depth+=(text[p]=='{')-(text[p]=='}');p+=1
    return text[a+1:p-1]

def extend(source,selected):
    text=(source/'INTELL.CPP').read_text(encoding='latin1')
    body,line=routine(text,'got_ball')
    a=body.index('set_pos_flags(player);');b=body.rfind('\n\tset_piece_on=FALSE;')
    # Intact outfield branch; final brace closes its else block.
    selected.append(('INTELL.CPP','got_ball','void got_ball(match_player *player) {\nint p=0;\nif(player->tm_player==1 || player->tm_player==12)throw std::logic_error("Outfield possession only");\n{\n'+body[a:b]+'\nset_piece_on=FALSE;\n}',line))
    body,line=routine(text,'find_zonal_target')
    a=body.index('if (!player->int_cnt)')
    selected.append(('INTELL.CPP','find_zonal_target','void find_zonal_target(match_player *player,float px,float py) {\nif(player->tm_player==1 || player->tm_player==12)throw std::logic_error("Outfield tactics only");\nint ball_zn; float tx,ty;\n'+body[a:body.rfind('\n\t}')]+ '\n}',line))
    body,line=routine(text,'get_target')
    selected.append(('INTELL.CPP','get_target',body[:body.index('{')].replace('[(32*2)+6]','[70]')+'''{
if(dead_ball_cnt || match_mode || game_action || man2man_a || man2man_b || pn<1 || pn>10 || bz<0 || bz>=64)
    throw std::logic_error("Only live zonal tactics are connected");
float ox,oy;
'''+block(body,body.index('if (!flag2)'))+'\n}',line))

def recover(actions,intell,ballint):
    definitions=[];evidence=[]
    def number(name,obj,label,seg,offset,fmt,expected):
        raw=bytes(obj.segments[seg]['data'][offset:offset+struct.calcsize(fmt)])
        value=struct.unpack('<'+fmt,raw)[0]
        if abs(value-expected)>1e-8:raise ValueError('Unverified team constant '+name)
        definitions.append(f'constexpr {"double" if fmt in ("f","d") else "int"} {name}={value!r};')
        evidence.append(dict(name=name,object=label+'.OBJ',segment=seg,offset=offset,format=fmt,bytes=raw.hex(),value=value))
    for name,seg,off,fmt,value in [
        ('TACKLE_TURNS',1,0x49ee,'i',30),('MC_S_TACKLE_FS',1,0xb4fd,'f',.04),
        ('PS_SLIDING',1,0x4ace,'i',17),('STEAL_ACT',1,0x3b80,'h',14),
        ('STEAL_FOOT_FACTOR',2,0x297,'d',.5)]:number(name,actions,'ACTIONS',seg,off,fmt,value)
    for name,seg,off,fmt,value in [
        ('MIN_HELP_CHANCE',1,0x103bc,'B',2),('CALL_FACTOR',2,0xa1d,'d',60),
        ('I_DRIBBLE',1,0xc4a5,'h',2),('I_CLOSE_DOWN',1,0x107b3,'h',3),
        ('STEAL_FACTOR',2,0xa5c,'d',6),('BETWEEN_FACTOR',2,0xa74,'d',3),
        ('DIVE_FEET_FACTOR',2,0xa8c,'f',4),('STEAL_BODY_FACTOR',2,0xaac,'d',1.5),
        ('GO_TO_POS_FACTOR',2,0x591,'d',.8),('INCIDENT_FACTOR',2,0x53d,'f',10),
        ('I_STEAL',1,0x118a2,'B',9),('I_THROW',1,0x9168,'B',6),
        ('THROW_IN_B',1,0x7f25,'B',12),('EFFECTIVE_TACK',1,0x1107c,'B',16)]:number(name,intell,'INTELL',seg,off,fmt,value)
    for name,off,value in [('FALL_ACT',0x1c92,5),('RISE_ACT',0x1c98,6),('RIDE_ACT',0x1ca0,8),('PICKUP_ACT',0x1d1e,19)]:
        number(name,ballint,'BALLINT',1,off,'B',value)
    number('NEG_SAY_GIVE',intell,'INTELL',1,0x106fd,'h',-7)
    definitions.append('''
constexpr int SAY_GIVE=-NEG_SAY_GIVE;
#define CALL_DIST (prat*CALL_FACTOR)
#define STEAL_DIST (prat*STEAL_FACTOR)
#define STEAL_BODY_DIST (prat*STEAL_BODY_FACTOR)
#define STEAL_FOOT_DIST (prat*STEAL_FOOT_FACTOR)
#define BETWEEN_DIST (prat*BETWEEN_FACTOR)
#define DIVE_FEET_DIST (prat*DIVE_FEET_FACTOR)
#define GO_TO_POS_DIST (prat*GO_TO_POS_FACTOR)
#define INCIDENT_DIST (prat*INCIDENT_FACTOR)
#define LIMIT(v,lo,hi) ((v)<(lo)?(lo):((v)>(hi)?(hi):(v)))
''')
    return '\n'.join(definitions),evidence

def update_report(path):
    report=json.loads(path.read_text())
    report['compatibility_adaptations'] += [
        dict(routine='go_to_between',change='Hoist loop variable i to preserve Watcom for-scope'),
        dict(routine='get_near_path',change='Skip disabled roster slots before evaluating interception candidates'),
        dict(routine='we_have_ball',change='Use teams[i-1] for one-based player numbers in crossing-distance reads; prevents player-23 out-of-bounds access'),
        dict(routine='get_target declaration',change='Spell [(32*2)+6] as [70] for declaration extraction')]
    report['excerpt_boundaries'].update(
        got_ball='Intact outfield possession decision branch; rejects goalkeeper dispatch',
        find_zonal_target='Intact outfield positioning branch; rejects goalkeeper dispatch',
        get_target='Normal-play branch; rejects dead balls, match modes, game actions and man-to-man flags',
        dispatch_action='Eleven supported do_action cases including tackle/steal; player pointer replaces global player_num; unsupported cases throw',
        stand_action='Only pending first-touch branch; native team fixture dispatches recovered zonal movement for idle actors')
    report['native_team_boundary']='Twenty outfield slots, original tactics table, uniform fixture stats, native input/selection/camera and ball-before-team scheduler with alternating team order; live zone indices clamped to 0..31. Goalkeeper match dispatch, player-body collisions, fouls, offside, scoring/restarts and sound remain incomplete.'
    path.write_text(json.dumps(report,indent=2)+'\n')
