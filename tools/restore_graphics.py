"""Recover renderer bodies and geometry without modifying preserved source files."""
from pathlib import Path
import sys,re,struct,json,hashlib
from omf import Object

def extract(text,name):
 m=re.search(r'^(?:void|float|int)\s+'+name+r'\s*\([^;]*?\)[^{;]*\{',text,re.M)
 if not m:raise ValueError(name)
 p=m.end();depth=1
 while depth:depth+=(text[p]=='{')-(text[p]=='}');p+=1
 return text[m.start():p],text[:m.start()].count('\n')+1

def generate(src,out,manifest):
 src,out=Path(src),Path(out);out.mkdir(parents=True,exist_ok=True)
 metadata=json.loads(Path(manifest).read_text());ro=Object(src/'3DENG.OBJ');data=Object(src/'DATA.OBJ')
 if hashlib.sha256((src/'3DENG.OBJ').read_bytes()).hexdigest()!=metadata['object_sha256']:raise ValueError('Unknown graphics object revision')
 if hashlib.sha256((src/'DATA.OBJ').read_bytes()).hexdigest()!=metadata['data_object_sha256']:raise ValueError('Unknown mesh data revision')
 for row in metadata['motions']:
  off=row['evidence_offset'];raw=bytes.fromhex(row['evidence'])
  if bytes(ro.segments[1]['data'][off:off+len(raw)])!=raw:raise ValueError('Motion loader evidence changed')
 def values(obj,name,fmt,n=None):
  p=obj.named(name);seg=obj.segments[p['segment']];off=p['offset']
  end=min([q['offset'] for q in obj.publics if q['segment']==p['segment'] and q['offset']>off]+[seg['length']])
  size=struct.calcsize(fmt);count=n or (end-off)//size
  if not all(seg['written'][off:off+count*size]):raise ValueError('Uninitialized geometry '+name)
  return struct.unpack_from('<'+fmt*count,seg['data'],off)
 names=['goal1a_p','goal2a_p','player_f1','player_fw','pitch_p','pitch_f','footy_p','footy_f','lines_p','lines_f','circle_pa','circle_fa','l1_p1','l2_p1','l3_p1','l4_p1','l5_p1','l6_p1','l_f1a','l_f3a','s_circle_p1a','s_circle_p2a','s_circle_f1a','s_circle_f2a']
 names+=['goal1ax_p','goal2ax_p','goal3a_p','goal3ax_p','goal_f1a','goal_f2a','spot_p','spot_f','plshad_p','plshad_f','ballshad_p','ballshad_f','player_f2','player_f1x','player_f2x','player_fg1','player_fg2','player_fg1x','player_fg2x']
 names+=['flag_p','flag_f']
 names+=['player_f1b','player_f2b','player_f1xb','player_f2xb']
 names+=['player_fr','player_fl','plhi_p','plhi1_f1','plhi1_f2'] # Original selected-player marker, init3d plhilight[1][0].
 # Resolve all five goal detail entries and player face pointers from the
 # pinned initialized DATA segment. LIDATA/COMDAT payloads are not geometry.
 fixups=data.fixups(skip_comdat=True,skip_lidata=True)
 def pointer(name,offset):
  p=data.named(name);f=next(f for f in fixups if f['segment']==p['segment'] and f['offset']==p['offset']+offset)
  if f['target_method']!=2 or f['displacement'] or f['relative']:raise ValueError('Unexpected scene pointer')
  return data.externs[f['target_index']].split('$')[0].removeprefix('W?')
 goals=[]
 for side in ['a','b']:
  for number in range(1,5):
   name=f'goal{number}_{side}';levels=[]
   for level in range(5):
    points=pointer(name,level*12+4);faces=pointer(name,level*12+8);scale=values(data,name,'f',15)[level*3]
    for mesh in [points,faces]:
     if mesh not in names:names.append(mesh)
    levels.append(dict(scale=scale,points=points,faces=faces))
   if [level['scale'] for level in levels]!=sorted([level['scale'] for level in levels],reverse=True) or levels[-1]['scale']!=0:raise ValueError('Invalid goal detail order')
   goals.append(dict(descriptor=name,levels=levels))
 for index,name in {11:'player_f1b',12:'player_f2b',16:'player_f1xb',17:'player_f2xb',3:'player_fr',4:'player_fl',1:'player_f1',2:'player_f2',5:'player_fg1',6:'player_f1x',7:'player_f2x',10:'player_fg1x',15:'player_fg2',20:'player_fg2x'}.items():
  if pointer('playertypes',index*4)!=name:raise ValueError('Unexpected original player face table')
 # Native archive IDs must come from the Euro object, not older FILES.C macros.
 texture_evidence=[]
 for label,offset,record,page in [('alternate_limbs_heads',0x13e46,38,12),('keeper_torso',0x13d06,35,4),('selection_markers',0x13d24,36,6),('keeper_limbs',0x14483,108,5),('away_keeper_limbs',0x13eb8,39,13),('away_keeper_torso',0x13ed6,35,14),('goal_nets',0x13ef4,40,15)]:
  raw=bytes(ro.segments[1]['data'][offset:offset+11]);expected=b'\xb8'+struct.pack('<I',record*8)+b'\x81\xc2'+struct.pack('<I',page*65536)
  if raw!=expected:raise ValueError('Unverified player texture '+label)
  texture_evidence.append(dict(name=label,offset=offset,record=record,page=page,bytes=raw.hex()))
 parts=['#pragma once','#include <array>','#include "graphics_types.hpp"','namespace actua::recovered {']
 shirt_evidence=[]
 render_hash=hashlib.sha256((src/'RENDER.OBJ').read_bytes()).hexdigest()
 if render_hash!='eb6931875ecc489e8b9616f3f36f9e0990fa20082d96a28cd6f1c7a6d24ef571':raise ValueError('Unverified transparent rasterizer')
 raster=Object(src/'RENDER.OBJ');transparent=bytes(raster.segments[1]['data'][0x9339:0x9343])
 if transparent.hex()!='678a07fec874038845ff':raise ValueError('Unverified polyt transparency encoding')
 for offset,expected in [(0x11d79,-2549),(0x11d96,-2564)]:
  raw=bytes(ro.segments[1]['data'][offset:offset+5])
  if raw!=b'\xba'+struct.pack('<i',expected):raise ValueError('Unverified original shirt-number binding')
  shirt_evidence.append(dict(offset=offset,first_face_material=expected,bytes=raw.hex()))
 parts.append('inline constexpr int shirt_home=549,shirt_away=564;')
 for name in names:
  typ='float' if name.endswith('_p') or name=='circle_pa' or re.search(r'_p[12]a?$',name) else 'short';fmt='f' if typ=='float' else 'h';v=values(data,name,fmt)
  parts.append('inline '+typ+' '+name+'[]={'+','.join(repr(x)+'f' if typ=='float' else str(x) for x in v)+'};')
 parts.append('inline legacy::det goals[8][5]={'+','.join('{'+','.join('{'+repr(level['scale'])+'f,'+level['points']+','+level['faces']+'}' for level in g['levels'])+'}' for g in goals)+'};')
 # Retained for historical callers; scene rendering now uses complete goal faces.
 face=values(data,'goal_f1a','h');at=1;lines=[]
 for _ in range(face[0]):
  n=face[at]
  if n==2:lines.extend(face[at:at+2+n])
  at+=2+n
 if lines!=[2,22,2,3]:raise ValueError('Unexpected original goal frame')
 parts.append('inline short goal_frame[]={1,'+','.join(map(str,lines))+'};')
 # All 16 kit descriptors are recovered from this Euro revision, not FILES.C's older team table.
 kits=values(ro,'teamlist','I',16*8);parts.append('inline unsigned kits[16][8]={'+','.join('{'+','.join(str(x//8) for x in kits[i:i+8])+'}' for i in range(0,len(kits),8))+'};')
 frames=values(data,'mcaps8','h',264)
 motions=[(r['animation'],r['record'],frames[r['animation']*2+1]) for r in metadata['motions']]
 parts.append('inline int motions[][3]={'+','.join('{'+','.join(map(str,r))+'}' for r in motions)+'};')
 stad=ro.named('stadlist');raw=ro.segments[stad['segment']]['data'];base=stad['offset']
 descriptors=[]
 for offset in [24,44,64,84]:
  xyz=struct.unpack_from('<3f',raw,base+offset);ids=struct.unpack_from('<2i',raw,base+offset+12)
  descriptors.append('{'+','.join(repr(x)+'f' for x in xyz)+','+','.join(str(x//8) for x in ids)+'}')
 width,length,height=struct.unpack_from('<3i',raw,base+104)
 parts.append(f'inline constexpr int stad_width={width},stad_length={length},stad_height={height};')
 parts.append('struct Stand {float x,y,z;unsigned points,faces;}; inline Stand stands[]={'+','.join(descriptors)+'};')
 # The first stadium's original high-resolution software ground resources.
 pitch_bitmap=struct.unpack_from('<I',raw,base)[0]//8
 sky_type=raw[base+8];sky_symbol=ro.named('skylist')
 sky_bitmap,sky_palette=struct.unpack_from('<II',ro.segments[sky_symbol['segment']]['data'],sky_symbol['offset']+sky_type*16)
 sky_bitmap//=8;sky_palette//=8
 parts.append(f'inline constexpr int pitch_bitmap={pitch_bitmap},sky_bitmap={sky_bitmap},sky_palette={sky_palette};')
 parts.append('}')
 (out/'graphics_data.hpp').write_text('\n'.join(parts)+'\n')
 types=(src/'3DENG.H').read_text(encoding='latin1').replace('\x1a','')
 (out/'graphics_types.hpp').write_text('#pragma once\nnamespace actua::legacy {\n'+types+'\n}\n')
 text=(src/'3DENG.C').read_text(encoding='latin1')
 routines=['matcop','matmul','addpoly','addline','add3dcmap','add3demap','addpols','addobjy','addobjyf','addobjxy','objdepd','addpolyc','addlinec','addpolsc','addobjyc','addobjfc','addobjnc','addobjsp']
 cpp=['#include "original_graphics.hpp"','#include <algorithm>','#include <cmath>','namespace actua::legacy {','using std::min; using std::sqrt;', '#define SCREENDIST 15','#define CUTOFFDIST 5','#define objdep(ob) ((ob).x*rot.e.r20+(ob).y*rot.e.r21+(ob).z*rot.e.r22+rot.e.r23)']
 report=[]
 for name in routines:
  body,line=extract(text,name)
  original_body=body
  if name=='addpolyc':
   marker='// Texture map...'
   body=body.replace(marker,marker+'\n        if (np>4 || clip_tex+2>=2000) throw std::runtime_error("Original clipped UV pool capacity exceeded");')
  cpp.extend([f'#line {line} "3DENG.C"',re.sub(r'^(void|float|int)\s+'+name+r'\b',r'\1 Core::'+name,body,count=1)])
  report.append(dict(file='3DENG.C',name=name,line=line,sha256=hashlib.sha256(original_body.encode()).hexdigest()))
 # Intact pose-blend branch, including the original mirrored limb mapping.
 start=text.index('if ((xf&&!xfo)||(!xf&&xfo))',text.index('// Tweening'))
 end=text.index('\n\t\t\t\t\t}',text.index('*(tw++)=*(tw1++)*pt1+*(tw2++)*pt2;',start))
 blend=text[start:end]
 cpp.append('void blend_original_pose(float* output,const float* target,const float* source,float amount,bool xf,bool xfo) {\nstruct {float tween;} state{amount};auto* plyrpto=&state;float pt1,pt2;int j;auto* tw=output+1;auto* tw1=target+1;auto* tw2=source+1;output[0]=28;\n'+blend+'\n}')
 report.append(dict(file='3DENG.C',name='blend_original_pose',line=text[:start].count('\n')+1,sha256=hashlib.sha256(blend.encode()).hexdigest(),scope='Intact cross-animation blend branch; native wrapper provides pose buffers, fraction and mirror flags'))
 cpp.append('}')
 (out/'graphics_original.cpp').write_text('\n'.join(cpp)+'\n')
 report=dict(presentation_adaptations=['3D_UPD2.CPP realtime_coords position/facing/frame equations adapted to match_player and pure snapshots','3DENG.C tween rate uses absolute elapsed logic cycles for render-rate independence; interrupted blends start from the last logic-boundary pose','Native ownership gate keeps captured and free ball in the same presentation timeline; gameplay is unchanged'],routines=report,mapped_motions=len(motions),inputs={name:hashlib.sha256((src/name).read_bytes()).hexdigest() for name in ['3DENG.C','3D_UPD2.CPP','3DENG.H','DATA.OBJ','3DENG.OBJ','EUROREND.DAT','EUROREND.OFF']})
 report['environment']=dict(pitch_bitmap=pitch_bitmap,sky_bitmap=sky_bitmap,sky_palette=sky_palette,sky_type=sky_type,source='3DENG.C ground() and init3d; stadlist/skylist from pinned 3DENG.OBJ',adaptations='Native bounded pixel buffer replaces DOS scanline assembly; original high-detail 64x64 tile and 640x480 sky projection; first supported stadium sky variant avoids consuming gameplay RNG')
 report['pitch_tracker']='Original addobjsp projection; sprite3d palette IDs 31/2 drawn as enlarged SDL dots for modern displays. Original depth sort retained.'
 report['scene']=dict(goals=goals,texture_evidence=texture_evidence,geometry=names,scope='Original five-level goal geometry, texture/filter net faces and objdepd selection; original pitch/shadow geometry; native SDL alpha replaces palette quantization for filters 256/257')
 report['scene']['shirt_numbers']=dict(evidence=shirt_evidence,scope='Original render3d face[82] binding, number atlases from kit table; match uses actual squad shirts 1-15, including the original four substitutes; native copies avoid mutating shared face data')
 report['scene']['shirt_numbers']['transparency']=dict(object='RENDER.OBJ',sha256=render_hash,offset=0x9339,bytes=transparent.hex(),meaning='polyt loads an encoded byte, decrements it, skips resulting zero, otherwise writes the decremented palette index; number archive pixels already use this +1 encoding')
 report['presentation_adaptations']+=['Original addpolyc including textured near-plane clipping with native-owned bounded UV scratch pool; addobjnc for stadium, addobjyc for pitch and addobjfc for ground markings','Original init3d pitch perimeter extension from stadlist dimensions','Invalid ball capture sentinels are not blended; native subframe sampling and capture/physics handoffs','Ball orientation follows new_update_coords; render-only shortest-arc interpolation between simulation angles','Original two-team texture packing and palette remapping; original face selection for team, keeper and mirror state']
 (out/'graphics-recovery.json').write_text(json.dumps(report,indent=2)+'\n')
 print('Recovered',len(routines),'original graphics routines and',len(motions),'motion IDs')
if __name__=='__main__':generate(*sys.argv[1:])
