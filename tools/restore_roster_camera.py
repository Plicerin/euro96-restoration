"""Recover Euro 96 rosters and selected camera bodies directly from preserved text."""
from pathlib import Path
import re,json,sys,hashlib
from omf import Object

def generate(src,out):
 src,out=Path(src),Path(out)
 script=(src/'SCRIPT.96').read_text(encoding='latin1')
 formations=['3-1-6','4-0-6','4-1-5','4-2-4','4-3-3','4-4-2','5-1-4','5-2-3','5-3-2','6-3-1']
 countries=re.findall(r't:\s*"([^"]+)"\s+form:\s*([\d-]+)',script)
 players=[]
 for line in script[script.index('PLYR_NMES'):script.index('TEAM_NMES')].splitlines():
  if 'pace:' not in line or 'name:' not in line or 't:' not in line:continue
  match=re.search(r'^\s*t:\s*"([^"]+)"',line)
  if not match:continue
  name=match.group(1)
  ratings=[int(re.search(r'\b'+key+r':\s*(\d+)',line).group(1)) for key in ['pace','pow','cntl','flar','visn','acc','stam','disc']]
  code=re.search(r'name:\s*"([A-Z]+)(\d+)"',line)
  assert code and all(0<=v<=100 for v in ratings)
  position=re.search(r'posn:\s*(GK|DF|MD|FD)',line).group(1)
  players.append((name,ratings,code.group(1),int(code.group(2)),position,int(re.search(r"\bfit:\s*(\d+)",line).group(1))))
 assert len(countries)==16 and len(players)==320
 # The preserved renderer has the first sixteen FILES.C kits, not Euro roster order.
 kit_names=re.findall(r'//\s*\d+\s*:\s*([^\r\n]+)',(src/'FILES.C').read_text(encoding='latin1'))[:16]
 assert [n.strip().upper() for n in kit_names][14]=='ENGLAND'
 kit_lookup={n.strip().upper():i for i,n in enumerate(kit_names)}
 # Check repeated symbolic resources against the matching descriptor layout.
 import struct
 renderer=Object(src/'3DENG.OBJ');table=renderer.named('teamlist')
 rows=re.findall(r'\{([^{}]+)\}\s*,?\s*//\s*\d+\s*:',(src/'FILES.C').read_text(encoding='latin1'))[:16]
 symbols={}
 for i,row in enumerate(rows):
  values=struct.unpack_from('<8I',renderer.segments[table['segment']]['data'],table['offset']+32*i)
  names=[v.strip() for v in row.split(',')]
  assert len(names)==8
  for name,value in zip(names,values):
   assert name not in symbols or symbols[name]==value
   symbols[name]=value
 quote=lambda s:json.dumps(s,ensure_ascii=True)
 cpp=['#include "original_roster.hpp"','#include "action_constants.hpp"','namespace actua {','const std::array<OriginalTeam,16> original_teams={{']
 for t,(name,formation) in enumerate(countries):
  squad=players[t*20:(t+1)*20]
  assert len({p[2] for p in squad})==1 # Speech IDs are not squad numbers (POR27 is slot 20).
  cpp.append('{'+quote(name)+','+quote(squad[0][2])+','+str(formations.index(formation))+','+str(kit_lookup.get(name,-1))+',{{'+','.join('{'+quote(p[0])+','+','.join(map(str,p[1]))+','+quote(p[4])+','+str(p[5])+'}' for p in squad)+'}}},')
 cpp+=['}};','void apply_original_stats(original::match_player& player,const OriginalPlayer& stats){']
 football=(src/'FOOTBALL.CPP').read_text(encoding='latin1')
 assignments=re.findall(r'teams\[p\]\.(tm_\w+)=game_data\[setup.team_a\]\.players\[ps\]\.(\w+)\+28;',football)
 assert len(assignments)==8
 cpp += [f'player.{field}=stats.{stat}+28;' for field,stat in assignments]
 actions=(src/'ACTIONS.CPP').read_text(encoding='latin1')
 for field in ['tm_mcspd','tm_jump']:
  statement=re.search(r'teams\[i\]\.'+field+r'=[^;]+;',actions).group(0)
  cpp.append(statement.replace('teams[i]','player').replace('PLAYER_HEIGHT','25 /* PLAYER_HEIGHT, restored engine constant */'))
 cpp+=['}','}']
 (out/'roster_original.cpp').write_text('\n'.join(cpp)+'\n')
 camera_object=Object(src/'3D_UPD2.OBJ')
 if hashlib.sha256((src/'3D_UPD2.OBJ').read_bytes()).hexdigest()!='077d9bc5d09edad734d1988ef67c44a65d985710c0fcce760364853a9c9cdcdb':raise ValueError('Unknown camera object')
 assert camera_object.value('scale','f')==440 and camera_object.value('set_camera_dist','f')==260 and camera_object.value('set_camera_height','f')==100
 text=(src/'3D_UPD2.CPP').read_text(encoding='latin1')
 cameras=[]
 for mode in [0,1,2,6,9,10,11,12,13,14,15,17]:
  start=text.index('if (camera=='+str(mode)+')',text.index('void update_3d()'))
  b=text.index('{',start);end=b+1;depth=1
  while depth:depth+=(text[end]=='{')-(text[end]=='}');end+=1
  body=text[start:end]
  cameras.append(dict(mode=mode,line=text[:start].count('\n')+1,sha256=hashlib.sha256(body.encode()).hexdigest()))
  body=body.replace('teams[ball_poss-1]','e.teams[ball_poss-1]')
  if mode in [1,2]:
   body=body.replace('static float dx=0;','float& dx=state.dx;').replace('static float dy=0;','float& dy=state.dy;')
   body=body.replace('cam_buffer=FALSE;','').replace('cam_limit=FALSE;','')
   body=re.sub(r'if \(game_speed\).*?move_rate=0.2;', 'move_rate=0.2; // fixed native 20 Hz',body,flags=re.S)
  if mode==14:body=body.replace('cam_limit=TRUE;','')
  if mode==17:body=body.replace('cam_buffer=TRUE;','').replace('player_on_off','e.player_on_off').replace('-stadlist[setup.stadium].tunlz','e.tunnel_y').replace('teams[p]','e.teams[p]')
  if mode==15:body=body.replace('cam_buffer=TRUE;','').replace('teams[goal_scorer-1]','e.teams[e.goal_scorer-1]')
  if mode==6:body=body.replace('ABS(','std::abs(')
  if mode in [11,12]:body=body.replace('d=calc_dist(bxd,byd);','d=std::max(0.001f,calc_dist(bxd,byd));')
  cameras[-1]['body']=body
 cpp=['#include "original_camera.hpp"','#include "action_constants.hpp"','#include <algorithm>','#include <cmath>','namespace actua {','using namespace original::action_constants;','using datapt=float;',
 'CameraPose original_camera(int camera,const original::PlayerEngine& e,float targ_x,float targ_y,float targ_z,const CameraPose& old,CameraMotion* motion){',
 'CameraMotion temporary;auto& state=motion?*motion:temporary;float& cxs=state.cxs;float& cys=state.cys;const float camera_inertia=.05f;const bool ball_still=e.ball_still!=0,use_fixed=false;',
 'const float pitch_len=e.pitch_len,pitch_wid=e.pitch_wid,cntspot_x=e.cntspot_x,cntspot_y=e.cntspot_y,prat=e.prat;',
 'const float set_camera_dist=260,set_camera_height=100,eff_ballx=targ_x,eff_bally=targ_y,eff_ballz=targ_z,bally=targ_y;',
 'const int ball_poss=e.ball_poss,match_mode=e.match_mode,last_touch=e.last_touch,throw_in_taker=e.throw_in_taker,corner_taker=e.corner_taker;',
 'const float ballxdis=e.ballxdis,ballydis=e.ballydis,old_tx=old.tx,old_ty=old.ty,old_tz=old.tz,old_camx=old.x,old_camy=old.y;',
 'float tx=targ_x,ty=targ_y,tz=targ_z,camera_x=targ_x,camera_y=targ_y+400,camera_z=200,scale=440;']
 for c in cameras:cpp+=[f'#line {c["line"]} "3D_UPD2.CPP"',c.pop('body')]
 cpp+=['return {camera_x,camera_y,camera_z,tx,ty,tz,scale};','}','}']
 (out/'camera_original.cpp').write_text('\n'.join(cpp)+'\n')
 (out/'roster-camera-recovery.json').write_text(json.dumps(dict(teams=16,players=320,kits={name:kit_lookup.get(name,-1) for name,_ in countries},camera_defaults=dict(scale=440,distance=260,height=100),cameras=cameras,inputs={f:hashlib.sha256((src/f).read_bytes()).hexdigest() for f in ['SCRIPT.96','FOOTBALL.CPP','ACTIONS.CPP','3D_UPD2.CPP','3D_UPD2.OBJ','FILES.C']},adaptations=['Roster uses first eleven of original default squad order; simulation stats +28, original animation speed/jump formulas; omits init_team debug override for player 2','Camera native timing replaces DOS timer; original camera bodies 0,1,2,6,9-15,17; camera 17 takes CD tunnel coordinate and player_on_off from the presentation engine; per-renderer plan/goal-line inertia and fixed 20 Hz timing; corner zero-distance guard']),indent=2)+'\n')
if __name__=='__main__':generate(*sys.argv[1:])
