"""Preserved human restart routines; SDL replaces the DOS input registry."""
from restore_ball import routine
from omf import Object
import re,json,struct

def extend(source,selected):
 raw=Object(source/'ACTIONS.OBJ').segments[1]['data']
 assert raw[0x16c4]==7 # taker_kick: cmp eax, SPACT_DRIVE
 # new_user_spec_kick's 1-based jump table: DRIVE, KICK and NEWK destinations.
 assert struct.unpack_from('<III',raw,0x2404)==(0x23bc,0x23cf,0x23e2)
 text=(source/'ACTIONS.CPP').read_text(encoding='latin1')
 for name in ['taker_pass_f','taker_pass_h','taker_tee_up','taker_kick','taker_chip','taker_grnd','new_user_spec_kick','user_throw']:
  body,line=routine(text,name)
  body=body.replace('I_RUNUP','15 /* I_RUNUP */').replace('SPACT_NEWK','9 /* SPACT_NEWK */')
  body=body.replace('SPACT_DRIVE','7 /* SPACT_DRIVE */').replace('SPACT_KICK','8 /* SPACT_KICK */')
  if name=='new_user_spec_kick':
   assert '#define ROT_SPEED (0.05)' in text
   body=body.replace('ROT_SPEED','0.05 /* source ROT_SPEED */')
   for mask,expr in [(8,'ux < -0.24f'),(4,'ux > 0.24f'),(32,'uy < -0.24f'),(16,'uy > 0.24f')]:
    body=body.replace('users_dir[user_controlled-1].f&'+str(mask),expr)
   body=body.replace('users_dir[user_controlled-1].x','ux').replace('users_dir[user_controlled-1].y','uy')
   body=re.sub(r'draw_tracker\([^;]*;', '(void)0; // Native HUD displays aim and power.',body)
  selected.append(('ACTIONS.CPP',name,body,line))
 # Render-only tracker, invoked on a snapshot so prediction never alters live state.
 body,line=routine(text,'draw_tracker')
 body=body.replace('float x,float y)','float x,float y,float fract)')
 body=body.replace('static float fract=0;','')
 body=body.replace('(i<first_bounce || i<mp)', '(i<49 && (i<first_bounce || i<mp))')
 body=re.sub(r'fract\+=0\.2;\s*if \(fract>=1\)\s*fract=0;', '',body)
 selected.append(('ACTIONS.CPP','draw_tracker',body,line))
 for i,(file,name,body,line) in enumerate(selected):
  if name=='throw_action':
   body=body.replace('throw std::logic_error("Native restarts use original CPU throw decisions");','user_throw(player);')
   selected[i]=(file,name,body,line)

def report(path):
 r=json.loads(path.read_text())
 r['human_restart_boundary']='Original new_user_spec_kick, taker helpers and user_throw. DOS direction registry mapped to native ux/uy; source ROT_SPEED=0.05; SPACT_DRIVE=7 verified ACTIONS.OBJ 0x16c4, KICK=8 and NEWK=9 verified in 1-based jump table at 0x2404. Original draw_tracker is restored on an isolated render snapshot; phase supplied by the 20 Hz presentation clock; interpolation bounded to the 50-entry prediction table. Native neutral gate, restart placement and keeper pass/punt buttons; original animation contacts and physics remain authoritative.'
 r['excerpt_boundaries']['throw_action']='Original human/CPU throw branches; referee/audio prefix excluded.'
 path.write_text(json.dumps(r,indent=2)+'\n')
