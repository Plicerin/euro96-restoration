"""Recover verified retail player atlases, country introductions and menu music."""
from pathlib import Path
import argparse, hashlib, json, struct, subprocess
from import_euro96 import ROOT, EXPECTED, CODES
from import_cd_graphics import records
from inspect_cd_audio import inspect_mad

EXE_HASH='2a473a002a294c1bb28ea39e94a36afc34497dd7e44f0ebfeda0ea21f652b62d'

def recover(iso):
    inventory=json.loads((ROOT/'docs/euro96-cd-audio-inspection.json').read_text())
    hashes={r['path']:r['sha256'] for r in inventory['audio']}
    hashes.update(EXPECTED);hashes['EURO96.EXE']=EXE_HASH
    def extract(name):
        b=subprocess.run(['7z','e','-so',str(iso),name],capture_output=True,check=True).stdout
        if hashlib.sha256(b).hexdigest()!=hashes[name]:raise ValueError('Unverified retail input: '+name)
        return b
    exe=extract('EURO96.EXE')
    cd=records(extract('EUROREND.DAT'),extract('EUROREND.OFF'))
    old=records((ROOT/'SOURCE/EUROREND.DAT').read_bytes(),(ROOT/'SOURCE/EUROREND.OFF').read_bytes())
    # Retail teamlist: 32 records, six dwords each; final field selects the
    # medium-tone remap. Values are byte offsets into the archive's OFF file.
    flags=[]
    for i in range(32):
        row=struct.unpack_from('<6I',exe,0x13ad3c+i*24)
        if row[:5]!=(16,(3+i)*8,280,856,(61+i)*8) or row[5] not in (0,1):raise ValueError('Unknown retail team descriptor')
        flags.append(row[5])
    if flags[:16]!=flags[16:]:raise ValueError('Alternate skin mapping differs')
    # File-offset evidence, not relocated runtime addresses: subtract six for
    # medium team skin; subtract twelve for alternate dark pages, clamp at 32.
    evidence=[]
    for address,expected in [(0xaedb6,'80eb06'),(0xaef53,'80ed0c'),
                             (0xacaa7,'b820010000'),(0xacac5,'b828010000'),
                             (0xacbf5,'b840010000'),(0xacc7b,'b848010000'),
                             (0xacc99,'b828010000'),(0xad347,'b850030000')]:
        offset=328192+address-0x10000
        if exe[offset:offset+len(bytes.fromhex(expected))]!=bytes.fromhex(expected):raise ValueError('Retail renderer evidence changed')
        evidence.append(dict(file_offset=offset,bytes=expected))
    for start,count,shift in [(357,176,0),(585,176,9),(821,176,9)]:
        if old[1][(start-1)*32:(start-1+count)*32]!=cd[1][(start-1+shift)*32:(start-1+shift+count)*32]:raise ValueError('Body/keeper UV compatibility changed')
    uv=cd[1][:120*32]+cd[1][769*32:829*32]
    pages=[]
    for record,size in [(2,65536),(36,17152),(37,65536),(40,65536),(41,65536),(106,65536)]:
        if len(cd[record])!=size:raise ValueError('Unexpected player page size')
        pages.append(cd[record]+bytes(65536-size))
    players=b'EU96PLY1'+cd[0]+bytes(flags[:16])+uv+b''.join(pages)
    # Preserve the existing event phrases; replace only the linked welcome
    # categories with exact three-letter retail country recordings.
    base=(ROOT/'local-data/COMMENTARY/ENGLISH3.BIN').read_bytes()
    expected=json.loads((ROOT/'docs/commentary-recovery.json').read_text())['pack_sha256']
    if hashlib.sha256(base).hexdigest()!=expected:raise ValueError('Unverified base commentary')
    at=12;clips={}
    for _ in range(struct.unpack_from('<I',base,8)[0]):
        c,n,size=struct.unpack_from('<HHI',base,at);at+=8
        if not 64<=c<96:clips[c,n]=base[at:at+size]
        at+=size
    if at!=len(base):raise ValueError('Invalid base commentary')
    welcome=[]
    for team,code in enumerate(CODES):
        name=f'DIGI/TEAMS/E/E_{code}.MAD';bank=extract(name);entries=inspect_mad(bank)
        for second,suffix in enumerate(('ttx','tty')):
            filename=f'e_{code.lower()}{suffix}.wav';found=[e for e in entries if e['name'].lower()==filename]
            if len(found)!=1:raise ValueError('Ambiguous or missing country introduction '+filename)
            e=found[0]
            if (e['rate'],e['channels'],e['sample_width'])!=(11025,1,1):raise ValueError('Unexpected country PCM')
            wav=bank[e['offset']:e['offset']+e['size']];category=64+16*second+team;clips[category,1]=wav
            welcome.append(dict(country=code,category=category,file=filename,sha256=hashlib.sha256(wav).hexdigest()))
    voice=b'AC96COM1'+struct.pack('<I',len(clips))+b''.join(struct.pack('<HHI',c,n,len(b))+b for (c,n),b in sorted(clips.items()))
    outputs={'GRAPHICS/PLAYERS.BIN':players,'COMMENTARY/EURO_ENGLISH.BIN':voice,'DIGI/MUSIC/EURO96.WAV':extract('DIGI/MUSIC/EURO96.WAV')}
    for name,b in outputs.items():
        p=ROOT/'local-data'/name
        if p.exists() and p.read_bytes()!=b:raise ValueError('Different presentation overlay exists: '+name)
    for name,b in outputs.items():
        p=ROOT/'local-data'/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(b)
    report=dict(executable_sha256=EXE_HASH,team_table_file_offset=0x13ad3c,medium_teams=flags[:16],renderer_evidence=evidence,head_uv_ranges=[[1,120],[770,829]],page_records=[2,36,37,40,41,106],skin_remap='Indices 32..63: subtract 6 for medium teams, 12 for dark appearance, clamp to 32. Medium default heads also subtract 4 from hair indices 5..31.',welcome=welcome,outputs={n:dict(bytes=len(b),sha256=hashlib.sha256(b).hexdigest()) for n,b in outputs.items()})
    (ROOT/'docs/euro96-presentation-import.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print('Recovered six player pages, 180 head coordinates, 32 country phrases, preserved welcome endings and Euro96 menu music.')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('iso',type=Path);recover(p.parse_args().iso.resolve(strict=True))
