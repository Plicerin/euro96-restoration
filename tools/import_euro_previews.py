"""Import retail Euro 96 FAPs with executable-verified palette routing."""
import argparse, hashlib, json, re, struct, subprocess
from pathlib import Path
from import_cd_graphics import ROOT, records
from import_euro96 import CODES, EXPECTED, unmangle
from euro96_fap import decode
from preview_identity import player, match

def recover(iso):
    def extract(name):
        return subprocess.run(['7z','e','-so',str(iso),name],capture_output=True,check=True).stdout
    inputs={n:extract(n) for n in ('EURO96.EXE','EURO.DAT','EURO.OFF','SCRIPT96.ENG')}
    hashes={n:hashlib.sha256(b).hexdigest() for n,b in inputs.items()}
    if hashes['EURO96.EXE']!='2a473a002a294c1bb28ea39e94a36afc34497dd7e44f0ebfeda0ea21f652b62d' or hashes['SCRIPT96.ENG']!=EXPECTED['SCRIPT96.ENG']:
        raise ValueError('Unsupported Euro96 executable/script')
    if hashes['EURO.DAT']!='cc6f91d54f58195a2d22f94d0fdc88c7262bed67356afa3aa99380fcee071558' or hashes['EURO.OFF']!='09a1ceb1b2134560aef2300561f8915c2215f838e07e04edf203ed47ffea681e':
        raise ValueError('Unsupported Euro96 palette archive')
    rs=records(inputs['EURO.DAT'],inputs['EURO.OFF']);exe=inputs['EURO96.EXE'];base=0x12f854
    stadium_ids=[v//8 for v in struct.unpack_from('<8I',exe,base+0x7d40)]
    player_ids=[v//8 for v in struct.unpack_from('<16I',exe,base+0x7d60)]
    assert stadium_ids==list(range(161,169)) and player_ids==[169,171,170,*range(172,185)]
    symbols=[v.decode() for v in re.findall(rb'([A-Z]{3})PAL\x00',exe[base+0x96e:base+0x96e+16*7])]
    assert len(symbols)==16
    script=unmangle(inputs['SCRIPT96.ENG']).decode('cp850');retail=[]
    for line in script.splitlines():
        row=player(line,retail=True)
        if row and row['code'][:3] in CODES:retail.append(row)
    source=(ROOT/'SOURCE/SCRIPT.96').read_text(encoding='cp850')
    roster=[player(line) for line in source[source.index('PLYR_NMES'):source.index('TEAM_NMES')].splitlines() if re.search(r't:\s*"[^"]+"[^\r\n]*pace:[^\r\n]*name:',line)]
    assert len(roster)==320
    keys=[];mapping=[];missing=[];matches=[]
    for i,row in enumerate(roster):
        assert row and row['code'][:3]==CODES[i//20]
        found,method=match(row,retail)
        if not found: mapping.append(65535);missing.append(dict(index=i,name=row['name']));continue
        fap=found['animation'].upper();pal=symbols.index(found['palette']);key=(fap,player_ids[pal],159,83,140,285)
        matches.append(dict(index=i,name=row['name'],retail_name=found['name'],method=method,source_id=row['code'],retail_id=found['code'],dob=row['dob'],club=row['club'],retail_club=found['club'],position=row['position'],stats=list(row['stats']),identity_fields_equal=row['fingerprint'] is not None and row['fingerprint']==found['fingerprint'],animation=fap,palette=found['palette']))
        if key not in keys:keys.append(key)
        mapping.append(keys.index(key))
    venues=re.findall(r't:\s*"([^"]+)"\s+anim:\s*"(stadium\d+)"',script,re.I)
    assert [n.lower() for _,n in venues]==[f'stadium{i}' for i in range(1,9)]
    for i,(_,fap) in enumerate(venues):
        keys.append((fap.upper(),stadium_ids[i],112,86,320,240));mapping.append(len(keys)-1)
    chunks=[];rows=[];checks=[]
    def fnv(b):
        h=1469598103934665603
        for v in b:h=((h^v)*1099511628211)&((1<<64)-1)
        return h
    for i,(name,pal_id,offset,base_id,w,h) in enumerate(keys):
        data=extract('FAPS/'+name+'.FAP');frames=decode(data,w,h)
        pal=bytearray(unmangle(rs[base_id]));pal[:48]=unmangle(rs[77]);part=unmangle(rs[pal_id])
        assert len(pal)==768 and offset*3+len(part)<=768 and max(pal+part)<=63
        pal[offset*3:offset*3+len(part)]=part
        chunks.append(struct.pack('<HHI',w,h,len(data))+pal+data)
        samples={str(f):str(fnv(frames[f])) for f in sorted({0,len(frames)//2,len(frames)-1})}
        checks.extend(f'{i} {f} {value}' for f,value in samples.items())
        rows.append(dict(id=i,file='FAPS/'+name+'.FAP',sha256=hashlib.sha256(data).hexdigest(),palette_record=pal_id,palette_offset=offset,base_palette=base_id,width=w,height=h,frames=len(frames),samples=samples))
    pack=b'EU96PRV1'+struct.pack('<I328H',len(keys),*mapping)+b''.join(chunks)
    target=ROOT/'local-data/GRAPHICS/EURO_PREVIEWS.BIN';target.write_bytes(pack)
    report=dict(inputs=hashes,entries=rows,mapping=mapping,matches=matches,missing_players=missing,venues=venues,bytes=len(pack),sha256=hashlib.sha256(pack).hexdigest(),timing='Native 20.4 fps from retail increment .34 at 60 frontend ticks; exact DOS wall-clock cadence unverified.')
    (ROOT/'docs/euro96-previews.json').write_text(json.dumps(report,indent=2)+'\n')
    (ROOT/'docs/euro96-preview-samples.txt').write_text('\n'.join(checks)+'\n')
    print(f'Imported {len(keys)} previews: {320-len(missing)}/320 verified player matches, 8 venues, {len(pack)} bytes.',flush=True)

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('iso',type=Path);recover(p.parse_args().iso.resolve(strict=True))
