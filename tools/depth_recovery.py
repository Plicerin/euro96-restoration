"""Match depth recovery; DOS menus and tunnel scheduling remain native boundaries."""
from restore_ball import routine
from omf import Object
import re,struct,json

def extend(source,selected):
 def read(file,name):return routine((source/file).read_text(encoding='latin1'),name)
 def add(file,name,body=None,line=None):
  if body is None:body,line=read(file,name)
  selected[:]=[s for s in selected if s[1]!=name];selected.append((file,name,body,line))
 for name in ['inc_inj','inc_yellow','inc_red','send_off_plr','init_ref_stand','init_ref_run','init_ref_jog','init_refs_anim','anim_officials','at_target','goto_target','init_rtrot_anim']:
  body,line=read('RULES.CPP',name)
  if name=='at_target':
   body=body.replace('teams[referee->target-1].tm_x-referee->y','teams[referee->target-1].tm_y-referee->y')
   body=body.replace('bd=calc_dist(x,y);','bd=calc_dist(x,y);if(bd<.001f){referee->act=4;return;}')
  if name=='goto_target':body=body.replace('d=calc_dist(tx,ty);','d=calc_dist(tx,ty);if(d<.001f){init_ref_stand(offc);offc->act=3;return;}')
  if name=='init_rtrot_anim':body=body.replace('float d=calc_dist(x,y);','float d=calc_dist(x,y);if(d<.001f){init_ref_stand(lman);return;}')
  add('RULES.CPP',name,body,line)
 for name in ['remove_player','init_off_int']:
  body,line=read('INTELL.CPP',name)
  body=body.replace('stadlist[setup.stadium].tunlx','tunnel_x').replace('-stadlist[setup.stadium].tunlz','tunnel_y')
  body=body.replace('camera=sub_camera;', '(void)0; // Native presentation owns camera restoration.')
  add('INTELL.CPP',name,body,line)
 for name in ['big_yeller','ger_em_off']:
  body,line=read('RULES.CPP',name)
  body=re.sub(r'\b(?:do_booked_menu|do_off_menu)\([^;]*;', '(void)0;',body)
  body=re.sub(r'game_data\[[^;]+;', '(void)0;',body)
  body=body.replace('sub_camera=camera;','').replace('camera=17;','')
  body=body.replace('init_off_int(&teams[plr-1]);','teams[plr-1].guy_on=teams[plr-1].draw_me=teams[plr-1].control=0; player_on_off=0;')
  body=body.replace('card_is_out=', 'card_player=plr; ++card_events; card_is_out=',1)
  add('RULES.CPP',name,body,line)
 # Restore the card call previously excluded by the regulation presentation adapter.
 for i,(file,name,body,line) in enumerate(selected):
  if name=='player_ints':
   # Injury accounting exists in RULES/EURO_MAT but has no source caller.
   # Native tournament bridge: rare crunching tackles use the existing seed,
   # never consume a new random draw or affect the original foul decision.
   body=body.replace('init_foul(player->tm_player,TRUE);', 'if(tournament_conditions && (seed&31)==0) inc_inj(i,20+seed/2);\n init_foul(player->tm_player,TRUE);')
   selected[i]=(file,name,body,line)
  if name=='punish_foul':
   body=body.replace('if (direct)\n\t\t(void)0;', 'if (direct)\n\t\tsend_off_plr(last_touch,nastiness);')
   assert 'send_off_plr(last_touch,nastiness)' in body
   selected[i]=(file,name,body,line)
 for name in ['init_taunt_act','init_run_celeb','init_stationary_celeb','init_celeb_act','celeb_action','init_celeb_anim','init_shame_anim','scorer_go','go_to_scorer','player_shame','someone_has_scored']:
  body,line=read('ACTIONS.CPP',name)
  if name=='init_taunt_act':
   start=body.index('if (user_scorer)');end=body.index('switch(rand_range(4))',start)
   body=body[:start]+'r=rand_range(4);\n'+body[end:] # CPU selection for both control modes; avoids broken old input spin-loop.
  body=re.sub(r'#ifndef EURO96\n.*?#endif', '',body,flags=re.S)
  add('ACTIONS.CPP',name,body,line)
 add('MATHS.CPP','random_vector')
 body,line=read('FOOTBALL.CPP','player_stamina')
 start=body.index('int st=');end=body.index('/*',start)
 part=body[start:end].replace('teams[i]','*player')
 part=part.replace('*player.', 'player->')
 add('FOOTBALL.CPP','apply_stamina','void apply_stamina(match_player* player,int ir) {\n'+part+'\n}',line+body[:start].count('\n'))

def recover(source):
 o=Object(source/'ACTIONS.OBJ');defs=[];evidence=[]
 def constant(name,obj,seg,offset,fmt):
  raw=bytes(obj.segments[seg]['data'][offset:offset+struct.calcsize(fmt)]);value=struct.unpack('<'+fmt,raw)[0]
  defs.append(f'constexpr double {name}={value!r};')
  evidence.append(dict(name=name,object='ACTIONS.OBJ' if obj is o else 'RULES.OBJ',segment=seg,offset=offset,bytes=raw.hex(),value=value))
 for name,offset in [('MC_CELEB_FS',0xbe12),('MC_PUMP_FS',0xbe21),('MC_BABY_FS',0xbe30),('MC_REEVES_FS',0xbe3f),('MC_MILLER_FS',0xbe4e),('MC_PLANE_FS',0xbe5d),('MC_FINGER_FS',0xbe6c),('MC_KNEE_FS',0xbe7b),('MC_MOON_FS',0xbe8a),('MC_DUCK_FS',0xbe99),('MC_SHAMEW_FS',0x9d66)]:
  assert bytes(o.segments[1]['data'][offset-3:offset])==bytes.fromhex('c74073')
  constant(name,o,1,offset,'f')
 constant('MC_SHAME_FS',o,1,0x5474,'f')
 constant('MC_DUCK_SPD',o,2,0x168,'d')
 constant('MC_JOG_FS',Object(source/'RULES.OBJ'),1,0x58ed,'f')
 assert bytes(o.segments[1]['data'][0x30dd:0x30df])==b'\x10\x00'
 ball=Object(source/'BALL.OBJ');wait=struct.unpack_from('<I',ball.segments[1]['data'],0x1a75)[0];assert wait==220
 evidence.append(dict(name='ORIGINAL_SCORE_WAIT',object='BALL.OBJ',segment=1,offset=0x1a75,value=wait))
 defs+=[f'constexpr int ORIGINAL_SCORE_WAIT={wait};','constexpr int CELEB_ACT=16;', '#define CELEB_RUN_DIST (13*prat)']
 return '\n'.join(defs),evidence

def update_report(path):
 r=json.loads(path.read_text())
 r['excerpt_boundaries'].update(apply_stamina='Original player_stamina curve; native roster lookup supplies original unfatigued pace by current shirt, evaluated each logic tick against original accumulated match minutes.',ger_em_off='Original card count and set-piece taker updates; native immediate live removal, with original init_off_int/remove_player route on a separate presentation copy.',big_yeller='Original booking/count/timer, native card event replaces DOS menu and speech.',init_taunt_act='Original CPU choice for both human and CPU scorer; broken legacy user-input busy loop excluded.',punish_foul='Original foul and card decisions enabled; native event and UI replace DOS menus.',dispatch_action='Original action dispatch now includes celebrations')
 r['native_goal_boundary']='Original score recording, celebration decisions/actions and scorer camera; original 220-tick goal wait. Native isolated replay snapshots, automatic replay dispatch and kickoff transition replace DOS replay/menu scheduling.'
 r['walkoff_046']=dict(routines=['init_off_int','remove_player','goto_target','at_target'],adaptations=['CD tunnel coordinates replace stadlist lookups; native camera restoration replaces camera/sub_camera globals.','Live substitutions/discipline stay authoritative; native event copies run original routes without changing match state.','Native replacement identity/ratings are inserted at tunnel, following process_subs sequencing.','Referee facing corrects source tm_x used for Y; zero-distance target movement/facing stay finite.','Referee/assistant dead-ball targets now run goto_target; original DOS await_kicker state machine is not restored.'])
 r['native_team_boundary']='Original discipline, official movement/animation, fatigue and celebration bodies. Native substitutions, replay buffer/UI, fixed-step scheduling, dead-ball placement and end transitions; individual walk-off presentation uses original route bodies on isolated copies. See current release documentation for competition and audio scope.'
 path.write_text(json.dumps(r,indent=2)+'\n')

