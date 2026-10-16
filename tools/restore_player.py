"""Generate original player routines, packed layout, and recovered scalar state."""
from pathlib import Path
import sys,re,json,hashlib,struct
from omf import Object
from restore_ball import routine
from action_constants import recover
from batch_constants import recover as recover_batch
from team_recovery import extend as extend_team, recover as recover_team, update_report as update_team_report
from match_recovery import extend as extend_match, recover as recover_match, update_report as update_match_report
from rules_recovery import extend as extend_rules, recover as recover_rules, update_report as update_rules_report
from depth_recovery import extend as extend_depth, recover as recover_depth, update_report as update_depth_report
from setpiece_recovery import extend as extend_setpiece, report as report_setpiece
from knockout_recovery import extend as extend_knockout, report as report_knockout
GROUPS={'ACTIONS.CPP':['full_spd','actual_spd','angle_to_xy','go_toward_target','go_forward','rotate_offs','init_stand_anim','init_kick_anim','init_trap_anim','init_chest_anim','init_dnhead_anim','init_kick_act','kick_action','fire_ball_off','init_stand_act','init_kphold_act','init_get_up','process_anims','control_action','get_closest_pred','init_control_act','init_wait_act'],
 'BALLINT.CPP':['hold_ball','init_specials','collect_ball','holder_lose_ball','ball_at_contact','get_mcball_coords','control_interact','standard_fstep'],
 'INTELL.CPP':['reset_ideas','reset_all_ideas','reset_leaves','pass_ahead','choose_pass','pass_ball','new_interceptor','fstep_factor'],
 'MATHS.CPP':['af_randomize','rand_range'],
 'RULES.CPP':['inc_work','inc_pass','inc_pwon','inc_twon','add_comp_pass','inc_shot']}
PINNED={'FOOTBALL.OBJ':'80386fd58625e1a5a8491cdf700096af782636923ba40b778746274c8663430a',
 'RULES.OBJ':'b2de700c0000e0d66eb7c559c012eec50404221b8565ac0894ef58a6fcf9b722',
 'BALL.OBJ':'f8dd68af5519f5fd70085de559d57418f24beea6dea9f9fb2cfd8e4627e76ec5',
 'ACTIONS.OBJ':'b02ab5c3177e8744c9c954c1d66148d6b3d9270fef647637192189ac4da87f31',
 'BALLINT.OBJ':'00bc42f30d1a75790e30d5f3b0565eda1e25c3114fa3b40888cc6d246187d378',
 'INTELL.OBJ':'aa57e892ff317cda7fbb372887af628372d5fe509eec3838ae012bf81335f2fd',
 'ANDYDEFS.H':'46fa596943584b099c3a2d3188cfaf54e8dcab25cd8df04734c81e5988da647c',
 'USER.CPP':'d2e531f07d3af8b62238bdc6f1e1947e54e4e74a4ba7ae8af66b62b02020b170'}
GROUPS['USER.CPP']=['clear_auto','auto_select_a','auto_select_b']
GROUPS['INTELL.CPP']+=['get_there_time','clear_strike','decide_on_face','go_to_path','can_i_intercept','intercept','strike_and_control','get_dir']
GROUPS['BALLINT.CPP']+=['predict_ball']
GROUPS['ACTIONS.CPP']+=['init_run_anims','init_stand_anims','init_trot_anims','init_run_anim','init_trot_anim','init_run_act','init_stop_act','stop_action','run_action','wait_action','init_strike_act','new_dir','dir_movement','dir_ball','dir_new_d','dir_must_face','dir_away','process_dir']
GROUPS.setdefault('INTELL.CPP',[]).extend(['first_time_strike', 'user_pass_check', 'shoot_ball', 'punt_ball', 'go_to_save_path', 'fake_ball_poss', 'kick_straight', 'pass_decide', 'punt_decide', 'restore_ball_poss', 'shoot_decide', 'aim_shot_at_goal', 'init_user_save', 'save_in_zone_a', 'save_in_zone_b', 'save_in_zone_c', 'angle_of_vis', 'can_i_pass', 'cross_angle', 'in_kicking_range', 'kopp_around', 'opp_around', 'plr_facing', 'plr_facing_goal', 'get_power', 'get_traj', 'sort_less_than', 'keeper_boxes'])
GROUPS.setdefault('ACTIONS.CPP',[]).extend(['init_first_time_act', 'strike_action', 'strike_ball_off', 'save_action', 'chip_strike', 'dvhead_strike', 'head_strike', 'kick_strike', 'overhead_strike', 'init_save_act', 'init_save_anim', 'init_shdr_anim', 'init_jhdr_anim', 'init_dhdr_anim', 'init_ohd_anim', 'init_voll_anim', 'init_getup_anim', 'init_getupf_anim', 'init_standb_anim', 'init_runb_anim'])
GROUPS.setdefault('BALL.CPP',[]).extend(['new_shot', 'real_player', 'ball_limbo'])
GROUPS.setdefault('BALLINT.CPP',[]).extend(['get_opp_near_ball','rebound_off_plr','player_distances','get_dist_pos'])
GROUPS['RULES.CPP']+=['inc_saves','inc_save']
GROUPS['ACTIONS.CPP']+=['init_kout_anim']

GROUPS['ACTIONS.CPP'] += ['kphold_action','init_tostand_anim','init_tostandb_anim','init_crtostb_anim','init_bounce_anim','init_hvoll_anim','init_throw_anim','init_roll_anim','init_waveup_anim','init_throwl_anim','init_throws_anim','init_crl_anim','init_crr_anim','init_kick_l_anim','init_kick_r_anim']
GROUPS['INTELL.CPP'] += ['make_pass','make_punt']
GROUPS['BALL.CPP'] += ['shot_passed','rebound_post','rebound_bar','hit_goal_post','hit_cross_bar',
 'hit_high_side_net','hit_low_side_net','hit_top_net','hit_inside_back_net','hit_inside_back_net_r',
 'hit_inside_top_net','hit_inside_high_side','hit_inside_low_side','outside_of_net','is_it_a_goal','ball_collision']
GROUPS['ACTIONS.CPP'] += ['user_stand','user_run']
GROUPS['INTELL.CPP'] += ['make_shoot']
GROUPS['BALLINT.CPP'] += ['control_difficulty','active_control','control_ball']
GROUPS['INTELL.CPP'] += ['thinking','run_back','help_chance','we_have_ball','kicking_dist','set_pos_flags','range_flags','cross_pos','make_run','go_dribble','avoid_boundary','get_opp_dir_tab','dribble_dir','get_facing_opp_dir','in_close','can_close_down','close_him_down','force_error','forceful_retrieve','go_to_side','go_to_between','get_tack_path','other_interceptor','opp_has_ball','get_near_path','free_ball','intelligence','get_tac']
GROUPS['ACTIONS.CPP'] += ['user_init_tackle','init_tackle_act','tackle_action','init_tack_anim','init_steal_act','steal_action']
GROUPS['BALLINT.CPP'] += ['ball_interact','get_nearest']
GROUPS['RULES.CPP'] += ['inc_tack']
GROUPS['INTELL.CPP'] += ['process_comments','player_comment']
GROUPS['BALLINT.CPP'] += ['has_ball']
GROUPS['INTELL.CPP'] += ['user_got_ball','user_opp_has_ball']
GROUPS['ACTIONS.CPP'] += ['user_spec_kick','rotate_around_ball']

def generate(source,out):
 for name,digest in PINNED.items():
  if hashlib.sha256((source/name).read_bytes()).hexdigest()!=digest:raise ValueError('Unverified player source/object '+name)
 football=Object(source/"FOOTBALL.OBJ")
 if struct.unpack_from("<f",football.segments[2]["data"],0)[0]!=4.0:raise ValueError("Unexpected goal-post initializer")
 selected=[]
 for filename,names in GROUPS.items():
  text=(source/filename).read_text(encoding='latin1')
  for name in names:
   extract=text
   if name in ('user_got_ball','user_opp_has_ball','user_spec_kick','rotate_around_ball'):
    # Historical commented-out branches have unbalanced braces; retain line positions.
    extract=re.sub(r'/\*.*?\*/|//[^\n]*',lambda m:re.sub(r'[^\n]',' ',m[0]),text,flags=re.S)
   body,line=routine(extract,name);selected.append((filename,name,body,line))
 # Source excerpts keep contact and dispatch logic intact, with explicit adapter boundaries.
 text=(source/'BALLINT.CPP').read_text(encoding='latin1')
 body,line=routine(text,'control_ball')
 start=body.index('holder_lose_ball();');a=body.rfind('{',0,start);depth=1;end=a+1
 while depth:
  depth+=(body[end]=='{')-(body[end]=='}');end+=1
 selected.append(('BALLINT.CPP','collect_controlled_ball','void collect_controlled_ball(match_player *player) {\nif(player->tm_act!=CONTROL_ACT || player->tm_anim==MC_U_HEAD)throw std::logic_error("Control contact outside restored fixture");\ncan_be_offside=TRUE;\n'+body[a+1:end-1]+'\n}',line+body[:start].count('\n')))
 marker='if (player->tm_frm>(keeper_contact-0.00001))'
 start=text.index(marker);a=text.index('{',start);depth=1;end=a+1
 while depth:
  depth+=(text[end]=='{')-(text[end]=='}');end+=1
 selected.append(('BALLINT.CPP','update_save_contact','void update_save_contact(match_player *player) {\n'+text[start:end]+'\n}',text[:start].count('\n')+1))
 text=(source/'ACTIONS.CPP').read_text(encoding='latin1')
 body,line=routine(text,'do_action')
 supported={'STAND_ACT','RUN_ACT','SAVE_ACT','KICK_ACT','CONTROL_ACT','STRIKE_ACT','WAIT_ACT','STOP_ACT','KPHOLD_ACT','TACKLE_ACT','STEAL_ACT','THROW_ACT','PICKUP_ACT','FALL_ACT','JUMP_ACT','CELEB_ACT'}
 cases=re.findall(r'case (\w+):\s*(\w+)_action\(&teams\[player_num-1\]\);\s*break;',body)
 dispatch='void dispatch_action(match_player *player) { switch(player->tm_act) {\n'
 for act,fn in cases:
  if act in supported:dispatch+='case '+act+': '+fn+'_action(player); break;\n'
 dispatch+='default: throw std::logic_error("Action outside restored fixture"); }}'
 selected.append(('ACTIONS.CPP','dispatch_action',dispatch,line))
 body,line=routine(text,'stand_action');start=body.index('{');end=body.index('\n\telse',start)
 selected.append(('ACTIONS.CPP','stand_action',body[:end]+'\n}',line))
 text=(source/'INTELL.CPP').read_text(encoding='latin1')
 body,line=routine(text,'got_ball');start=body.index('if (punt_decide(player))');end=body.index('\n\t\t\telse',start)
 selected.append(('INTELL.CPP','distribute_held_ball','void distribute_held_ball(match_player *player) {\n'+body[start:end]+'\n}\n}',line+body[:start].count('\n')))
 body,line=routine(text,'intelligence')
 start=body.index('if (player->int_cnt)');cut=body.index('/*',start)
 end=body.index('\n\t\tif (player->tm_act<=TURN_ACT)',cut)
 tail=body.index('\n\t\t\telse\n\t\t\t{\n\t\t\t\tplayer->int_cnt=-1;',cut)
 selected.append(('INTELL.CPP','keeper_busy','int keeper_busy(match_player *player) {\nint busy=0,pnum=player->tm_player;\n'+body[start:cut]+'}\n}'+body[tail:end]+'\nreturn busy;\n}',line+body[:start].count('\n')))
 text=(source/'BALL.CPP').read_text(encoding='latin1');body,line=routine(text,'process_ball')
 start=body.index('if (ball_limbo_on>0)');end=body.index('\n\tif ((!ball_limbo_on)',start)
 selected.append(('BALL.CPP','update_ball_limbo','void update_ball_limbo() {\n'+body[start:end]+'\n}',line+body[:start].count('\n')))
 # Recover the commentary-only decision tree before the native goal latch resets shot state.
 # Retain BALL.CPP's asymmetric left/right power and wall classifications.
 for end in ['left','right']:
  original,line=routine(text,'go_'+end+'_goal')
  own=re.search(r'if \(last_touch([<>]\d+)\)',original)[1]
  keeper=re.search(r'if \(last_touch==(KP_[AB])\)',original)[1]
  a=original.index('if (headed_ball)');b=original.index('do_goal_menu(',a)
  attack=original[a:b].rstrip()
  body='void comment_'+end+'_goal() {\nif(prediction || match_mode!=NORMAL_PLAY)return;\nif(last_touch<1 || last_touch>22){PlayCommentaryMessage(GL_GENERIC);return;}\n'
  body+='if(last_touch'+own+'){if(last_touch=='+keeper+')PlayCommentaryMessage(GL_KEEPER);else PlayCommentaryMessage(GL_DEFLECT);}else{\n'+attack+'\n}}'
  selected.append(('BALL.CPP','comment_'+end+'_goal',body,line))
 extend_team(source,selected)
 extend_match(source,selected)
 extend_rules(source,selected)
 extend_depth(source,selected)
 extend_knockout(source,selected)
 extend_setpiece(source,selected)
 # Observe each original near-miss classification without changing its legacy timer.
 for i,(file,name,body,line) in enumerate(selected):
  if name=='can_i_intercept':
   # EURO96 computes pass_option and its receiver, but the three pass-capable
   # records still require shot-only input. Keep every feasibility check and
   # enable only the chip/standing-header/jumping-header records with metadata.
   for action in (10,9,7):
    pattern=r'if \(strike_option \|\| !user_controlled\)(\s*\{\s*strike\['+str(action)+r'\]\.t=i;)'
    body,count=re.subn(pattern,r'if (strike_option || pass_option || !user_controlled)\1',body)
    assert count==1,('first-time pass gate',action,count)
   selected[i]=(file,name,body,line)
  if name=='user_spec_kick':
   body=body.replace('SPACT_DRIVE','7 /* SPACT_DRIVE */').replace('SPACT_KICK','8 /* SPACT_KICK */')
   selected[i]=(file,name,body,line)
  if name=='shoot_ball':
   marker='ballydis=shot_speed*yoff/d;'
   assert body.count(marker)==1
   # aim_shot_at_goal uses the old velocity for its aftertouch basis. Capture
   # the actual release direction once both final horizontal components exist.
   body=body.replace(marker,marker+'\nif(after_touch_on){float speed=calc_dist(ballxdis,ballydis);shoot_x=ballxdis/speed;shoot_y=ballydis/speed;}')
   selected[i]=(file,name,body,line)
  if name=='predict_ball':
   # The DOS predictor leaves prediction set until process_ball. Native event
   # guards need its scope to end here, before real player actions resume.
   body=body.replace('{','{\nconst auto live_prediction=prediction;',1)
   body=body[:-1]+'\nprediction=live_prediction;\n}'
   selected[i]=(file,name,body,line)
  if name=='has_ball':
   body=body.replace('{','{\n++possession_events; if(prediction || prev_touch<1 || prev_touch>22)return;',1)
   selected[i]=(file,name,body,line)
  if name=='shot_passed':
   body=body[:-1]+'\nif(!prediction && shot_missed>0 && (ballx<0 || ballx>pitch_len))++near_miss_events;\n}'
   selected[i]=(file,name,body,line)
 identifiers=set(re.findall(r'\b\w+\b','\n'.join(s[2] for s in selected)))
 base=(out/'ball_original.hpp').read_text()
 recovered=json.loads((out/'engine-recovery.json').read_text())['globals']
 fields=[]
 for item in recovered:
  n=item['name']
  if n in identifiers and not re.search(r'\b'+n+r'\s*=',base):
   value=repr(item['value'])+('f' if item['type']=='float' else '')
   if n in ('top_post_y','bot_post_y'):value='int(cntspot_y'+('-' if n=='top_post_y' else '+')+'prat*4.0f)'
   fields.append(f"    {item['type']} {n} = {value};")
 declarations=[('static ' if n=='sort_less_than' else '')+b[:b.index(')')+1].strip().replace('=FALSE','=0')+';' for _,n,b,_ in selected]
 layout=(source/'ANDYDEFS.H').read_text(encoding='latin1')
 header='''// Generated from preserved sources. Read docs/PLAYER-RECOVERY.md for scope.
#pragma once
#include "ball_original.hpp"
#include <cstddef>
#include <vector>
#include <stdexcept>
namespace actua::original {
#pragma pack(push,1)
'''+layout+'''
#pragma pack(pop)
static_assert(sizeof(match_player)==203);
static_assert(offsetof(match_player,tm_x)==2);
static_assert(offsetof(match_player,tm_rate)==0x46);
static_assert(offsetof(match_player,tm_poss)==0x90);
static_assert(offsetof(match_player,go_txdis)==0xa0);
static_assert(offsetof(match_player,int_move)==0xbf);
// Missing header record recovered from INTELL.OBJ member loads/stores and 28-byte stride.
struct intercept_data {
    short t=0,free=0,ptype=0,pplr=0;
    short gt=0,padding=0;
    float x=0,y=0,atime=0;
    char stop=0;
    char reserved[3]{};
};
static_assert(sizeof(intercept_data)==28);
static_assert(offsetof(intercept_data,x)==12 && offsetof(intercept_data,atime)==20 && offsetof(intercept_data,stop)==24);
class PlayerEngine:public BallEngine {
public:
'''+ '\n'.join(fields)+'''
    std::array<match_player,22> teams{};
    float tunnel_x=0,tunnel_y=0; // Native stadium descriptor boundary for original tunnel_pos.
    // Native records for the missing header's named fields; no packed I/O.
    struct {int min=0;float sec=0;} match_time;
    struct officials {float x=0,y=0,goto_x=0,goto_y=0,dir_x=1,dir_y=0;float frm=0,fstep=0,z=0;int act=0,target=0,anim=0,go=0;bool newanim=false;};
    std::array<officials,3> refs{};

    struct WallPlayer {int guy=0;float x=0,y=0;};
    std::array<WallPlayer,10> wguys{};
    struct {bool freekicks=false,bookings=false;} EUROmatch_info;
    bool tournament_conditions=false; // Native tournament injury event policy.
    int rule_events=0,foul_events=0,offside_events=0,card_events=0,card_player=0;
    std::array<short,20> burst_timer{};
    std::array<char,22> sel_circle{};
    // Named fields used by USER.CPP selectors; native single-user adapter,
    // never serialized as the missing DOS user_config record.
    struct SelectionUser {short plr=0;bool chng=false;};
    std::array<SelectionUser,20> users{};
    int match_tactics1[70][10][2]{},match_tactics2[70][10][2]{},man2man[10][2]{};
    float opp_dir_tab[11][3]{};
    const char* action="";
    struct PitchSprite {short x,y,z,ref;};
    std::vector<PitchSprite> tracker_sprites;
    void add_pitch_sprite(short x,short y,short z,short ref){tracker_sprites.push_back({x,y,z,ref});}
    int possession_events=0,selection_events=0,animation_events=0;
    std::array<intercept_data,11> strike{};
    void reselect() { ++selection_events; } // Practice view handles selection explicitly.
    // The native loader owns validated capture memory; the original lookup uses these tables.
    struct Capture {int cappts=0;short capfrms=0;};
    std::array<Capture,132> mcaps{};
    struct Frames:std::vector<std::array<float,85>> {
        auto& operator[](std::size_t n) { return at(n); }
    } player_p;
    std::array<std::array<float,3>,50> ball_pred_tab{};
    std::array<std::array<float,3>,50> ns_ball_pred_tab{};
    struct {int GameType=0;} match_info; // Only the field consumed by restored pass_ball.
    void init_anim(match_player*,int);
    void update_kick_contact(match_player*);
    int team_sample_events=0,commentary_events=0,near_miss_events=0;
    void PlayTeamSample(int player,int tone) {
        if(prediction || player<1 || player>22 || tone<0 || tone>2 || ((player-1)%11==0 && tone==2))return;
        ++team_sample_events;
        const int slot=(match_half&1)?(player<=11?player+10:player-12):player-1;
        const int shirt=teams[slot].shirt;
        if(shirt>=1 && shirt<=20)PlayCommentaryMessage(512+((player-1)/11)*60+(shirt-1)*3+tone);
    } // Original callback carries stable team/actual squad identity, not CD slot number.
    std::array<int,128> commentary_queue{};
    unsigned commentary_count=0,commentary_dropped=0;
    void PlayCommentaryMessage(int id) {
        if(prediction)return;
        ++commentary_events;
        if(commentary_count<commentary_queue.size())commentary_queue[commentary_count++]=id;
        else ++commentary_dropped;
    }
    // Original PlayScoreSample sorts the digits after its home<=7 / away<=5 guards.
    // Values >=256 carry an exact score recording through the same bounded queue.
    void PlayScoreSample(int home,int away) {
        if(home<0 || home>7 || away<0 || away>5)return;
        PlayCommentaryMessage(256+(home>=away?home*10+away:away*10+home));
    }
    // Goal detection calls this preview boundary instead of the unrestored
    // scoring, statistics, celebration, menu and restart dispatcher.
    int goal_events=0,goal_end=0;
    void go_left_goal() { if(!ball_in_goal){ball_in_goal=1;goal_end=-1;++goal_events;comment_left_goal();reset_shot();} }
    void go_right_goal() { if(!ball_in_goal){ball_in_goal=1;goal_end=1;++goal_events;comment_right_goal();reset_shot();} }

    // Fail closed on match systems outside the restored control-input fixture.


    void user_intelligence(match_player*) { throw std::logic_error("Use native user input adapter"); }
    void user_conts(match_player*) { throw std::logic_error("Use native user input adapter"); }
    void reselect_a() { throw std::logic_error("Use native team selection adapter"); }
    void reselect_b() { throw std::logic_error("Use native team selection adapter"); }

'''+ '\n'.join('    '+d for d in declarations)+'''\n};\n}\n'''
 # All numeric values below verified against pinned objects; expressions retain original precision.
 actions=Object(source/'ACTIONS.OBJ');ballint=Object(source/'BALLINT.OBJ');intell=Object(source/'INTELL.OBJ')
 celebration=struct.unpack_from('<f',actions.segments[1]['data'],0x25)[0]
 if abs(celebration-1.69)>0.00001:raise ValueError('Unexpected celebration speed')
 air=struct.unpack_from('<d',intell.segments[2]['data'],0x6ad)[0]
 ground=struct.unpack_from('<d',intell.segments[2]['data'],0x6bd)[0]
 if air!=0.986 or ground!=0.965:raise ValueError('Unexpected pass friction constants')
 contact=actions.named('save_offs');seg=actions.segments[contact['segment']];start=contact['offset']
 if seg['length']-start!=132*3*4 or not all(seg['written'][start:]):raise ValueError('Unverified ball contact table')
 offsets=struct.unpack_from('<396f',seg['data'],start)
 contact_cpp='constexpr float save_offs[]={'+','.join(repr(x)+'f' for x in offsets)+'};\n'
 constants=contact_cpp+'''
constexpr int FALSE=0,TRUE=1,KP_A=1,KP_B=12;
constexpr int I_CELEB=16,I_INTERCEPT=1,I_GET_UP=10,I_OFF=7,I_RUN_ON=8;
constexpr int STAND_ACT=0,RUN_ACT=1,TACKLE_ACT=3,SAVE_ACT=10,THROW_ACT=11,KPHOLD_ACT=12,CONTROL_ACT=17;
constexpr int PLAYER_HEIGHT=25,IN_HANDS_DIST=10,AT_FEET_DIST=10,HAS_BALL_DELAY=4,OFFSIDE_REL_CNT=40;
constexpr double PI=3.1415926536;
#define MAX_TURN (((float)player->tm_rate/8+14)*PI/180)
#define MAX_TURN2 (((float)teams[p_num-1].tm_rate/8+14)*PI/180)
#define LOW_PASS_DIST (prat*18.0)
#define ABS(x) ((x)<0?-(x):(x))
'''+f'constexpr float MC_CELEB_SPD={celebration}f;\nconstexpr double AIR_FRICTION={air},GRND_FRICTION={ground};\n'
 ball_object=Object(source/'BALL.OBJ')
 for offset in [0x16c,0x174]:
  if struct.unpack_from('<d',ball_object.segments[2]['data'],offset)[0]!=0.038:raise ValueError('Unverified goal curl threshold')
 constants+='\nconstexpr double GOOD_SWERVE=0.038;\n'
 extra,constant_evidence=recover(source,actions,ballint,intell)
 batch,batch_evidence=recover_batch(actions,ballint,intell,Object(source/"BALL.OBJ"))
 extra+="\n"+batch
 constant_evidence+=batch_evidence
 team,team_evidence=recover_team(actions,intell,ballint)
 extra+="\n"+team
 constant_evidence+=team_evidence
 constants+=extra
 match,match_evidence=recover_match(source)
 extra+='\n'+match
 constants+='\n'+match
 constant_evidence+=match_evidence
 rules,rules_evidence=recover_rules(source)
 extra+='\n'+rules
 constants+='\n'+rules
 constant_evidence+=rules_evidence
 depth,depth_evidence=recover_depth(source)
 extra+='\n'+depth
 constants+='\n'+depth
 constant_evidence+=depth_evidence
 # Existing constants were verified individually; add newly retained AUDIO.H IDs.
 audio_header=re.sub(r'//[^\n]*','',(source/'AUDIO.H').read_text())
 audio_names=re.search(r'enum\s*\{\s*(CM_NULL.*?)\}',audio_header,re.S)[1]
 for id,name in enumerate(v.strip() for v in audio_names.split(',') if v.strip()):
  if name in identifiers and not re.search(r'\b'+name+r'\s*=',constants):constants+=f'\nconstexpr int {name}={id};'
 cpp=['#include "player_original.hpp"','#include <cmath>','#include <cstdlib>','#include <cstring>','namespace actua::original {',constants]
 for filename,name,body,line in selected:
  body=re.sub(r'=FALSE\b','',body[:body.index('{')])+body[body.index('{'):]
  # Watcom's pre-standard for-scope leaves i visible after the loop. Preserve that scope.
  if name=='get_dist_pos':body=body.replace('i<pn+12','i<pn+11') # Original off-by-one reads player 23; each team has eleven players.
  if name=='get_there_time':
   old='for (int i=0; i<50; i++)'
   if body.count(old)!=1:raise ValueError('Unexpected get_there_time loop')
   body=body.replace(old,'int i; for (i=0; i<50; i++)')
  if name=='go_to_between':body=body.replace('for (int i=1; i<50; i++)','int i; for (i=1; i<50; i++)')
  if name=='get_near_path':body=body.replace('if (teams[i].tm_notme)','if (!teams[i].guy_on)continue;\n\t\tif (teams[i].tm_notme)')
  if name=='we_have_ball':body=body.replace('teams[i].tm_dist','teams[i-1].tm_dist')
  cpp.extend([f'#line {line} "{filename}"',re.sub(r'^(\w+)\s+'+name+r'\b',r'\1 PlayerEngine::'+name,body,count=1)])
 cpp.extend(['#undef ABS','#undef MAX_TURN','#undef LOW_PASS_DIST','}'])
 # This is the intact kick-contact branch of ball_interact, with an adapter entry point.
 text=(source/'BALLINT.CPP').read_text(encoding='latin1')
 start=text.index('if ((player->contact>0) && (player->tm_act==KICK_ACT))')
 a=text.index('{',start);end=text.index('}',a)+1
 cpp.insert(-4,'#line '+str(text[:start].count('\n')+1)+' "BALLINT.CPP"\nvoid PlayerEngine::update_kick_contact(match_player* player) {'+text[start:end]+'}')
 (out/'action_constants.hpp').write_text('#pragma once\nnamespace actua::original::action_constants {\n'+extra+'\n}\n')
 (out/'player_original.hpp').write_text(header,encoding='utf-8')
 (out/'player_original.cpp').write_text('\n'.join(cpp)+'\n',encoding='utf-8')
 (out/'player-recovery.json').write_text(json.dumps(dict(inputs={n:hashlib.sha256((source/n).read_bytes()).hexdigest() for n in list(PINNED)+list(GROUPS)+['DATA.H']},routines=[dict(file=f,name=n,line=l,sha256=hashlib.sha256(b.encode()).hexdigest()) for f,n,b,l in selected],fields=fields,layout_size=203,kick_contact_excerpt=dict(file='BALLINT.CPP',line=text[:start].count('\n')+1,sha256=hashlib.sha256(text[start:end].encode()).hexdigest()),contact_table=dict(symbol='save_offs',object='ACTIONS.OBJ',segment=contact['segment'],offset=contact['offset'],records=132,sha256=hashlib.sha256(seg['data'][contact['offset']:]).hexdigest()),constants_cpp=constants,action_constant_evidence=constant_evidence,
  native_goal_boundary='go_left_goal/go_right_goal are preview callbacks: latch ball_in_goal, count one crossing, record end, reset_shot. Original scoring/statistics/celebrations/restarts are not restored.',
  compatibility_adaptations=[dict(routine='get_there_time',change='Hoist int i before its for loop to preserve Watcom for-scope; no algorithm changes'),dict(routine='get_dist_pos',change='Bound each team scan to eleven players (pn+11); original pn+12 crossed team boundary and read nonexistent player 23'),dict(routine='sort_less_than',change='Static member for the original qsort callback'),dict(routine='default arguments',change='Keep default arguments in generated declarations only')],
  runtime_initialization=dict(object='FOOTBALL.OBJ',symbol='.fn_init',code_range=[0x50,0x8e],constant_segment=2,constant_offset=0,goal_posts='int(cntspot_y +/- prat*4.0f)'),
  excerpt_boundaries=dict(collect_controlled_ball='Intact non-upward-header CONTROL_ACT branch of control_ball plus its offside flag; caller enforces supported control contact',keeper_busy='intelligence countdown and expiry branches; off-field/run-on/steal side effects excluded; caller uses keeper save, recovery and holding only',update_ball_limbo='Intact animation release gate from process_ball',distribute_held_ball='Keeper punt/pass decision prefix of got_ball; caller requires in-box keeper holding ball, action <= TURN_ACT and no busy timer',update_save_contact='Intact SAVE_ACT contact branch of ball_interact; caller enforces SAVE_ACT and unpossessed ball',dispatch_action='Nine supported do_action cases; player pointer replaces global player_num; unsupported cases throw',stand_action='Only the pending first-touch branch; idle tactical decisions remain outside fixture'),
  interception_variant='First can_i_intercept body, under original EURO96 conditional',
  interception_layout=dict(object='INTELL.OBJ',size=28,stride_instruction='0x60f6',members={'t':['short',0],'free':['short',2],'ptype':['short',4],'pplr':['short',6],'gt':['short',8],'x':['float',12],'y':['float',16],'atime':['float',20],'stop':['char',24]},evidence=['clear_strike','strike_and_control','can_i_intercept'])),indent=2)+'\n')
 update_team_report(out/'player-recovery.json')
 update_match_report(out/'player-recovery.json')
 update_rules_report(out/'player-recovery.json')
 update_depth_report(out/'player-recovery.json')
 report_knockout(out/'player-recovery.json')
 report_setpiece(out/'player-recovery.json')
 report=json.loads((out/'player-recovery.json').read_text())
 report['automatic_selection_044']=dict(routines=['USER.CPP clear_auto','USER.CPP auto_select_a','USER.CPP auto_select_b'],
  rules='Intact receiver/interceptor/near-path priority, teammate possession, selection-circle retention, fall/disabled-player and keeper filters.',
  boundary='Native single-user registry synchronized at tick boundaries. Original new_users ball_travel++ > select_cnt (10) cadence is 12 ticks; also used for loose-ball polling because DOS user_conts dispatch remains disconnected. Original reselect notifications trigger a next-tick check. Manual switching reserves one cadence; keeper possession/contact, dead balls and shootouts retain their dedicated adapters. auto_users_a/b stay zero outside the extracted selector calls; multiplayer registry and DOS input dispatch are not enabled.')
 report['tunnel_exit_042']=dict(routine='INTELL.CPP tunnel_pos',stadium_boundary='Original CD descriptor int tunnel x/y/z at 224/228/232; world y is negative z; unused height excluded from helper.',presentation_boundary='Original run and direction routines operate on a cloned engine at native 20 Hz; live get_target tunnel dispatch stays guarded. No live clock, statistics or RNG advancement.')
 report['human_controls_038']=dict(routines=['user_got_ball','user_opp_has_ball','user_spec_kick','rotate_around_ball'],
  input_boundary='Native single-user held-button, direction, selection and pause adapter; original possession decisions and kick counters run at 20 Hz.',
  extraction='Strip comments while preserving newlines in these four routines before brace matching; commented-out branches contain unmatched braces.',
  aftertouch_fix='After shoot_ball assigns final ballxdis/ballydis, refresh shoot_x/shoot_y when after_touch_on is set. Original aim_shot_at_goal computed this basis before assigning the new shot velocity. Original spin formulas and limits remain unchanged.',
  first_time_fix='EURO96 can_i_intercept: allow existing pass_option at exactly the chip (10), standing-header (9) and jumping-header (7) record gates. Original user_pass_check, reach/timing, receiver metadata and animation/contact routines are retained. This corrects the contradictory shot-only gates; it does not add new pass trajectories or claim byte-identical DOS behavior.',
  compatibility='C/North and diagnostic request_play_kick remain native shortcuts to original kicks; keeper and set-piece adapters remain separate.')
 (out/'player-recovery.json').write_text(json.dumps(report,indent=2)+'\n')
 print(f'Restored {len(selected)} player routines and {len(fields)} additional scalar fields.')
if __name__=='__main__':generate(*map(Path,sys.argv[1:]))
