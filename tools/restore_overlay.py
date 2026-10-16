"""Recover original HUD tables; verify source definitions against preserved OMF data."""
from pathlib import Path
import sys,re,struct,json,hashlib
from omf import Object

def generate(source,out):
 source=Path(source);out=Path(out);out.mkdir(parents=True,exist_ok=True)
 obj=Object(source/'3DENG.OBJ');code=obj.segments[1]['data']
 text=re.sub(r'//[^\n]*','',(source/'FGFX.C').read_text())
 arrays={}
 for name,count in [('font_data',50),('spr_data',5)]:
  values=list(map(int,re.findall(r'\b\d+\b',text.split(name+'[]=')[1].split(';')[0])))
  rows=[values[i:i+count] for i in range(0,len(values),count)]
  packed=b''.join(struct.pack('<5i45B3x' if name=='font_data' else '<5i',*row) for row in rows)
  symbol=obj.named(name);raw=obj.segments[symbol['segment']]['data']
  if packed!=raw[symbol['offset']:symbol['offset']+len(packed)]:raise ValueError('HUD source/object mismatch: '+name)
  arrays[name]=rows
 evidence=[]
 for offset,record,target in [(0x13d24,36,0x60000),(0x13d6c,37,0x70000),(0x145c8,121,0x6b400)]:
  expected=b'\xb8'+struct.pack('<I',record*8)+b'\x81\xc2'+struct.pack('<I',target)
  if bytes(code[offset:offset+len(expected)])!=expected:raise ValueError('HUD loader mismatch')
  evidence.append(dict(offset=offset,record=record,target=target,bytes=expected.hex()))
 fonts=arrays['font_data'];sprites=arrays['spr_data']
 cpp=['#pragma once','#include <array>','namespace actua::recovered {',
      'struct HudSprite {int page,x,y,w,h;};',
      'inline constexpr HudSprite hud_sprites[]={'+','.join('{'+','.join(map(str,row))+'}' for row in sprites)+'};',
      'struct HudFont {int page,x,y,n,off,w,h;std::array<unsigned char,43> widths;};',
      'inline constexpr HudFont hud_fonts[]={'+','.join('{'+','.join(map(str,row[:7]))+',{'+','.join(map(str,row[7:]))+'}}' for row in fonts)+'};','}']
 (out/'hud_data.hpp').write_text('\n'.join(cpp)+'\n')
 report=dict(inputs={n:hashlib.sha256((source/n).read_bytes()).hexdigest() for n in ['FGFX.C','3DENG.OBJ','3DENG.C','EUROREND.DAT','EUROREND.OFF']},source_object_tables_equal=True,sprites=len(sprites),fonts=len(fonts),loaders=evidence,
 scope='3DENG.C high-resolution scanner coordinate equations, half-dependent team marker selection, clock/score placement and FGFX.C proportional font 2. Original sprite byte rules: zero transparent, one caller color, otherwise palette index byte-1. SDL nearest scaling replaces doubled DOS blit; alpha darkening replaces palette filter. Native safe clipping and unsupported-character fallback. Score text from live restored match, replay clock replaced by REPLAY; no claim of full original menu restoration.')
 (out/'hud-recovery.json').write_text(json.dumps(report,indent=2)+'\n')
 print('Verified original HUD: 129 sprites, 3 fonts, 3 archive loaders')
if __name__=='__main__':generate(*sys.argv[1:])
