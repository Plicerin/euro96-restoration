"""Original knockout thresholds and penalty ordering; native presentation boundary."""
from restore_ball import routine
from team_recovery import block
from omf import Object
import struct
import re,json

def extend(source,selected):
 o=Object(source/'ACTIONS.OBJ');raw=o.segments[1]['data'];assert struct.unpack_from('<H',raw,0x1b35)[0]==15 and raw[0x1b9e]==9
 def read(file,name):return routine((source/file).read_text(encoding='latin1'),name)
 def add(file,name,body,line):selected.append((file,name,body,line))
 body,line=read('FOOTBALL.CPP','watch_match_time')
 predicates=[]
 for minutes,half in [('90+15',2),('90+30',3)]:
  start=body.index('if ('+('!penalty_game && ' if half==3 else '')+'need_result && match_time.min>=('+minutes+'+injury_time)')
  end=body.index('nothing_happening())',start)+len('nothing_happening())')
  predicates.append(body[start:end]+' return '+str(half+1)+';')
 add('FOOTBALL.CPP','extra_period_end','int extra_period_end(){\n'+'\n'.join(predicates)+'\nreturn 0;}',line)
 for name in ['pen_to_team_a','pen_to_team_b','do_penalties']:
  body,line=read('RULES.CPP',name)
  body=re.sub(r'user_taker(?:2)?=user_\w+\([^;]*;', 'user_taker=0;',body)
  body=body.replace('init_match_mode();','(void)0; // Native scheduler calls original penalty placement.')
  body=body.replace('kill_ball();','reset_ball(); // Native owns the dead-ball transition.')
  add('RULES.CPP',name,body,line)
 for name in ['init_runup_act','user_runup','user_pen_kick','taker_nkick']:
  body,line=read('ACTIONS.CPP',name)
  # ACTIONS.OBJ: init_runup_act 0x1b2e stores 15; user_runup 0x1b98 stores 9.
  body=body.replace('I_RUNUP','15 /* I_RUNUP */').replace('SPACT_NEWK','9 /* SPACT_NEWK */')
  body=re.sub(r'draw_tracker\([^;]*;', '(void)0; // DOS aiming marker omitted.',body)
  add('ACTIONS.CPP',name,body,line)
 # Enable only the original penalty-direction branch. The native adapter supplies
 # save_direction instead of the absent DOS users_dir bitmask registry.
 for index,(file,name,body,line) in enumerate(selected):
  if name=='aim_shot_at_goal':
   assert body.count('#ifdef EURO96')==1
   body=body.replace('#ifdef EURO96','#if 1 // Preserve Euro penalty power and height instead of CPU swerve')
   selected[index]=(file,name,body,line)
  if name=='go_to_save_path':
   assert body.count('#ifdef EURO96')==1
   body=body.replace('#ifdef EURO96','#if 1 // Restored Euro penalty keeper input')
   body=re.sub(r'if \(users_dir\[user_controlled-1\]\.f&8\)\s*save_direction=-1;\s*if \(users_dir\[user_controlled-1\]\.f&4\)\s*save_direction=1;', '// Native input adapter supplies save_direction.',body)
   assert 'users_dir' not in body
   selected[index]=(file,name,body,line)
 body,line=read('RULES.CPP','ready_set_kick');add('RULES.CPP','ready_set_kick',body,line)
 body,line=read('BALL.CPP','go_right_goal')
 start=body.index('if (golden_goal');end=body.index('{',start)
 body=body[:end]+'{'+block(body,start)+'}'
 add('BALL.CPP','check_golden_goal','void check_golden_goal(){\n'+body[start:]+'\n}',line)

def report(path):
 r=json.loads(path.read_text());r['knockout_boundary']='Original 105/120-minute predicates, golden-goal flag, do_penalties clinch/sudden-death checks, taker rotation, Euro CPU aiming/power and run-up. ACTIONS.OBJ immediates: I_RUNUP=15 at 0x1b35; SPACT_NEWK=9 at 0x1b9e. Native periods, result UI and isolated kick lifecycle; Human takers use the original hold/release branch; aim_shot_at_goal selects the EURO96 penalty exception so get_power/get_traj govern human and CPU penalties; the original EURO96 keeper direction branch accepts native save_direction in place of DOS users_dir. Regular match score remains separate from shootout counters.'
 path.write_text(json.dumps(r,indent=2)+'\n')
