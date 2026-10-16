from pathlib import Path
import sys,re,json,hashlib
from restore_graphics import extract
from omf import Object
import struct

def generate(source,out):
 out.mkdir(parents=True,exist_ok=True)
 text=(source/'FOOTBALL.CPP').read_text(encoding='latin1')
 cpp=['#include "original_crowd.hpp"','#include "action_constants.hpp"','#include <cmath>','namespace actua {','using namespace original::action_constants;',
 'enum {CN_OFF,CN_NORMAL,CN_PENSIVE,CN_ENCOURAGING,CN_EXCITED,CN_CELEBRATE,CN_BOO,CN_SAVE,CN_MISS,CN_LAUGH,CN_FOWL};', 'constexpr int KP_A=1,KP_B=12;']
 inputs=[]
 for name in ['reset_possession','accumulate_possession','process_crowd_noise']:
  body,line=extract(text,name);inputs.append(dict(name=name,line=line,sha256=hashlib.sha256(body.encode()).hexdigest()))
  body=body.replace('void '+name+'()', 'void OriginalCrowd::'+name+'(const original::PlayerEngine& e)' if name!='reset_possession' else 'void OriginalCrowd::reset_possession()')
  body=body.replace('accumulate_possession();','accumulate_possession(e);').replace('ABS(','std::abs(')
  for var in ['ball_poss','last_touch','match_half','match_mode','shot_pending','shot_acknowledged','ballxdis','ballx','prat','pitch_len','ball_in_hands','cntspot_x','just_scored','goal_scorer','men_in_wall']:
   body=re.sub(r'\b'+var+r'\b','e.'+var,body)
  cpp.extend([f'#line {line} "FOOTBALL.CPP"',body])
 cpp.append('}')
 (out/'crowd_original.cpp').write_text('\n'.join(cpp)+'\n')
 obj=Object(source/'AUDIO.OBJ');assert hashlib.sha256((source/'AUDIO.OBJ').read_bytes()).hexdigest()=='3bd8d90d2cef292aeda77ef86a9aee490ddb9dfbc0f2adef1fd493f4a5da33f9'
 data=obj.segments[1]['data'];jumps=struct.unpack_from('<11I',data,0xca8)
 assert jumps==(0xd3d,0xd31,0xe28,0xe39,0xe4a,0xe5b,0xeb4,0xec5,0xf1e,0xf77,0xfd6)
 banks=[0,1,2,3,4,4,6,4,4,4,4]
 for mood,at in enumerate(jumps):
  if mood:assert data[at:at+5]==b'\xba'+struct.pack('<I',banks[mood])
 (out/'crowd_mapping.hpp').write_text('#pragma once\n#include <array>\nnamespace actua {inline constexpr std::array<int,11> crowd_ambience_bank={'+','.join(map(str,banks))+'};}\n')
 (out/'crowd-recovery.json').write_text(json.dumps(dict(functions=inputs,mood_jump_table=0xca8,ambient_banks=banks,adaptations=['Own presentation counters and possession; no gameplay mutations','Once per native simulation tick; fixed 0.5 second SDL crossfade, not the DOS device scheduler']),indent=2)+'\n')
if __name__=='__main__':generate(*map(Path,sys.argv[1:]))
