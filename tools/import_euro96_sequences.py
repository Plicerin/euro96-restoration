"""Recover verified tournament introductions and country-specific result calls."""
from pathlib import Path
import argparse, hashlib, json, re, struct, subprocess
from import_euro96 import ROOT, EXPECTED, CODES, unmangle
from import_euro96_presentation import EXE_HASH
from inspect_cd_audio import inspect_mad


def recover(iso):
    inventory=json.loads((ROOT/'docs/euro96-cd-audio-inspection.json').read_text())
    hashes={r['path']:r['sha256'] for r in inventory['audio']}
    hashes.update(EXPECTED);hashes['EURO96.EXE']=EXE_HASH
    def extract(name):
        data=subprocess.run(['7z','e','-so',str(iso),name],capture_output=True,check=True).stdout
        if hashlib.sha256(data).hexdigest()!=hashes[name]:raise ValueError('Unverified retail input: '+name)
        return data
    exe=extract('EURO96.EXE');evidence=[]
    for at,raw in [(0x98126,'0fb6058fb20400'),(0x9812d,'8a8064a20000'),(0x98133,'a29eb60400'),
                   (0x101bf7,'803d9db6040000'),(0x101bfe,'0f8570060000'),
                   (0x101c0b,'83f801'),(0x101c0e,'0f8560060000'),(0x101c1b,'83f81d'),
                   (0x101c2d,'bb365d0000'),(0x101c32,'a0901e0b00'),
                   (0x101d08,'bb3b5d0000'),(0x101d0d,'a01c1c0b00'),
                   (0x101e6d,'a09eb60400'),(0x101e91,'b8455d0000'),
                   (0x105193,'8b049d34250b00'),(0x1051d1,'4a'),
                   (0xa7269,'b101880da5b60400'),(0xa72b3,'b001a2a4b60400'),
                   (0x103fb1,'83fb1f'),(0x103fbf,'bbb65d0000'),
                   (0x103fcd,'803da4b6040000'),(0x103fdf,'bbbb5d0000'),
                   (0x103fed,'803da5b6040000'),(0x103fff,'bbc05d0000'),
                   (0x8089f,'6bc006'),(0x808a2,'0fb68084a20000'),(0x808a9,'e850e6ffff'),
                   (0x7ef27,'8a80dc160300'),(0x7ef2d,'3a45fc'),
                   (0x80939,'80c210'),(0x80940,'8890849a0400'),
                   (0x7eed5,'0fbe90849a0400'),(0x7eee0,'39c2')]:
        b=bytes.fromhex(raw)
        if exe[at:at+len(b)]!=b:raise ValueError(f'Instruction changed at {at:#x}: {exe[at:at+len(b)].hex()}')
        evidence.append(dict(file_offset=at,bytes=raw))
    base=0x12f854
    for at,value in [(0x5d36,'Y'),(0x5d38,'TT'),(0x5d3b,'Z'),(0x5d3d,'TT'),(0x5d45,'STT'),
                     (0x5db6,'C'),(0x5db8,'TT'),(0x5dbb,'G'),(0x5dbd,'TT'),(0x5dc0,'V'),(0x5dc2,'TT')]:
        if exe[base+at:].split(b'\0',1)[0]!=value.encode():raise ValueError('Unknown sequence suffix')
    speech=list(exe[base+0xa264:base+0xa264+31])
    if sorted(speech)!=list(range(1,32)):raise ValueError('Unknown fixture/speech permutation')
    script=unmangle(extract('SCRIPT96.ENG')).decode('latin1')
    draw_rows=re.findall(r'draw:\s*([1-4])([a-d])\s+digi:\s*"([A-Z]{3})"',script)
    draw={CODES.index(code):(ord(group)-97)*4+int(pos)-1 for pos,group,code in draw_rows}
    if sorted(draw)!=list(range(16)) or sorted(draw.values())!=list(range(16)):raise ValueError('Unknown retail draw')
    source_script=(ROOT/'SOURCE/SCRIPT.96').read_text(encoding='latin1')
    source_draw=[(ord(g)-97)*4+int(p)-1 for p,g in re.findall(r'draw:\s*([1-4])([a-d])',source_script)]
    if source_draw!=[draw[i] for i in range(16)]:raise ValueError('Preserved default draw differs from retail')
    inverse={slot:team for team,slot in draw.items()}
    defs=(ROOT/'SOURCE/EURO_DEF.H').read_text(encoding='latin1')
    codes={k:int(v) for k,v in re.findall(r'#define\s+(TEAM_\w+)\s+(\d+)',defs)}
    var=(ROOT/'SOURCE/EURO_VAR.CPP').read_text(encoding='latin1');block=var[var.index('fixture_info\tFixtures'):var.index('char\tMatchVenues')]
    source=[(codes[a],codes[b]) for a,b in re.findall(r'(TEAM_\w+),\s*(TEAM_\w+),',block)]
    # Retail writes qualifier-table index + 16 to GroupDrawInfo[country],
    # after resolving that row's group position through LeaguePositions.
    # Its quarter/semi winners subsequently use 24..29, versus source 16..21.
    qualifiers=[exe[base+0xa284+6*i] for i in range(8)]
    if qualifiers!=[0,1,4,5,8,9,12,13]:raise ValueError('Unknown retail qualifier table')
    def source_slot(slot):
        return qualifiers[slot-16] if 16<=slot<24 else slot-8 if 24<=slot<30 else slot
    fixtures=[]
    for i,number in enumerate(speech):
        slots=tuple(exe[0x139910+12*i:0x139912+12*i])
        if i<24 and slots!=source[i]:raise ValueError('Preserved group fixture differs from retail')
        resolved=tuple(source_slot(slot) for slot in slots) if i>=24 else slots
        if sorted(resolved)!=sorted(source[i]):raise ValueError('Preserved knockout bracket differs from retail')
        home,away=(inverse[slots[0]],inverse[slots[1]]) if i<24 else (255,255)
        fixtures.append(dict(fixture=i,number=number,home=home,away=away,retail_slots=list(slots),source_slots=list(source[i]),resolved_retail_slots=list(resolved)))
    records={};clips=[]
    def add(category,number,bank,data,entry):
        if (entry['rate'],entry['channels'],entry['sample_width'])!=(11025,1,1):raise ValueError('Unsupported sequence PCM')
        wav=data[entry['offset']:entry['offset']+entry['size']]
        if (category,number) in records:raise ValueError('Duplicate sequence clip')
        records[category,number]=wav
        clips.append(dict(category=category,number=number,bank=bank,filename=entry['name'],sha256=entry['sha256']))
    data=extract('DIGI/PHRASES/E/E_STT.MAD');entries=inspect_mad(data)
    for entry in entries:
        m=re.fullmatch(r'e_stt(\d{2})\.wav',entry['name'].lower())
        if not m or not 1<=int(m[1])<=31:raise ValueError('Unknown STT recording')
        add(99,int(m[1]),'STT',data,entry)
    for team,code in enumerate(CODES):
        data=extract(f'DIGI/TEAMS/E/E_{code}.MAD');entries=inspect_mad(data)
        for category,suffix in [(100,'ttz'),(101,'ttc'),(102,'ttg'),(103,'ttv')]:
            found=[e for e in entries if e['name'].lower()==f'e_{code.lower()}{suffix}.wav']
            if len(found)!=1:raise ValueError('Missing/ambiguous country sequence')
            add(category,team+1,code,data,found[0])
    if len(records)!=94 or (99,20) in records:raise ValueError('Unexpected sequence inventory')
    packed=b'AC96SEQ1'+struct.pack('<I',len(records))+bytes(v for f in fixtures for v in (f['number'],f['home'],f['away']))
    packed+=b''.join(struct.pack('<HHI',c,n,len(wav))+wav for (c,n),wav in sorted(records.items()))
    target=ROOT/'local-data/COMMENTARY/EURO_SEQUENCES.BIN'
    if target.exists() and target.read_bytes()!=packed:raise ValueError('Different sequence overlay exists')
    target.write_bytes(packed)
    report=dict(executable_sha256=EXE_HASH,script_sha256=EXPECTED['SCRIPT96.ENG'],instruction_evidence=evidence,
                source_inputs={name:hashlib.sha256((ROOT/'SOURCE'/name).read_bytes()).hexdigest() for name in ('SCRIPT.96','EURO_VAR.CPP','EURO_DEF.H')},
                fixture_lookup_file_offset=base+0xa264,fixture_table_file_offset=0x139910,fixtures=fixtures,
                qualifier_table_file_offset=base+0xa284,retail_qualifier_positions=qualifiers,
                retail_draw=[draw[i] for i in range(16)],clips=clips,
                boundaries='Original-draw group/quarter introductions and semi/final composites verified. Retail qualifier codes map to preserved bracket pairings; quarters 1 and 4 reverse sides. STT20 absent: original country X/Y fragments only. Retail exhibitions/custom draws return without a welcome; French/absent-supplement installations retain compatibility sequences. TTC final winner precedes TTG penalties and TTV extra time; ordinary knockout wins use FLL.',
                outputs={'COMMENTARY/EURO_SEQUENCES.BIN':dict(bytes=len(packed),sha256=hashlib.sha256(packed).hexdigest())})
    (ROOT/'docs/euro96-sequences-import.json').write_text(json.dumps(report,indent=2)+'\n')
    print(f'Imported {len(records)} original sequence clips and verified {len(fixtures)} fixture lookup entries.')


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('iso',type=Path);recover(p.parse_args().iso.resolve(strict=True))
