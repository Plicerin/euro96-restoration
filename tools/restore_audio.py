"""Recover original pitch/crowd tables; no DOS object code is linked."""
from pathlib import Path
import hashlib,json,struct,sys
from omf import Object
from inspect_cd_audio import original_pitch_names

def generate(source,out):
    digest=hashlib.sha256((source/'AUDIO.OBJ').read_bytes()).hexdigest()
    if digest!='3bd8d90d2cef292aeda77ef86a9aee490ddb9dfbc0f2adef1fd493f4a5da33f9':
        raise ValueError('Unsupported AUDIO.OBJ')
    obj=Object(source/'AUDIO.OBJ');fixups={(f['segment'],f['offset']):f for f in obj.fixups(True,True)}
    # Both original routines multiply the one-based event ID by 16 then subtract 16.
    code=obj.segments[1]['data']
    assert bytes(code[0x8e5:0x8f0])==bytes.fromhex('c1e004bb0000000083e810')
    assert bytes(code[0xc53:0xc59])==bytes.fromhex('c1e20483ea10')
    names=original_pitch_names(source);tables={}
    for name,count in [('PitchBank',19),('CrowdBank',10)]:
        symbol=obj.named(name);data=obj.segments[symbol['segment']]['data'];rows=[]
        for i in range(count):
            offset=symbol['offset']+i*16;f=fixups[(symbol['segment'],offset)]
            assert f['kind']==9 and not f['relative'] and f['target_method']==2
            assert obj.externs[f['target_index']]==obj.named('SampleBank')['name'] and f['displacement']==0
            pointer,amp,freq,flags=struct.unpack_from('<Ih2xiH2x',data,offset)
            assert pointer%16==0 and pointer//16<len(names) and 0<=amp<=32767 and freq==32768 and flags in (0,3)
            rows.append(dict(event=i+1,sample=pointer//16,name=names[pointer//16],amp=amp,freq=freq,flags=flags))
        tables[name]=rows
    out.mkdir(parents=True,exist_ok=True)
    cpp=['// Generated from pinned AUDIO.OBJ tables. Event IDs are one-based.','#pragma once','#include <array>','namespace actua::audio_data {','struct Mapping {int sample,amp,freq,flags;};',
         'inline constexpr std::array<const char*,20> names={'+','.join(json.dumps(n) for n in names)+'};']
    for name,rows in tables.items():
        cpp.append('inline constexpr std::array<Mapping,'+str(len(rows))+'>'+name+'={{'+','.join('{'+','.join(str(r[k]) for k in ('sample','amp','freq','flags'))+'}' for r in rows)+'}};')
    cpp.append('}')
    (out/'audio_data.hpp').write_text('\n'.join(cpp)+'\n')
    (out/'audio-recovery.json').write_text(json.dumps(dict(audio_obj_sha256=digest,tables=tables,frequency_variation='PlayPitchSample: nominal + rand_range(1024) - 512; native audio-only PRNG'),indent=2)+'\n')
if __name__=='__main__':generate(*map(Path,sys.argv[1:]))
