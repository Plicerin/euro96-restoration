"""Import original title pixels/palette and four music tracks without modifying SOURCE."""
from pathlib import Path
import argparse,hashlib,json,struct,subprocess
from inspect_cd_audio import wav_info
p=argparse.ArgumentParser(description=__doc__);p.add_argument('iso',type=Path);args=p.parse_args()
root=Path(__file__).resolve().parents[1];iso=args.iso.resolve(strict=True)
def extract(name):return subprocess.run(['7z','e','-so',str(iso),name],capture_output=True,check=True).stdout
report={'source_iso':iso.name,'assets':[]}
def save(name,data,origin):
 target=root/'local-data'/name
 if target.exists() and target.read_bytes()!=data:raise ValueError('Different asset already installed: '+name)
 target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
 report['assets'].append(dict(path=name,size=len(data),sha256=hashlib.sha256(data).hexdigest(),origin=origin))
data=extract('DATA.DAT');offsets=extract('DATA.OFF')
if len(offsets)!=125*8:raise ValueError('Unsupported CD data table')
def entry(index):
 offset,size=struct.unpack_from('<II',offsets,index*8)
 if offset>len(data) or size>len(data)-offset:raise ValueError('CD record outside archive')
 return data[offset:offset+size]
pixels,palette=entry(8),entry(15)
if len(pixels)!=640*480 or len(palette)!=768 or max(palette)>63:raise ValueError('Unsupported CD title layout')
save('FRONTEND/TITLE.IDX',pixels,'CD DATA.DAT record 8, 640x480 indexed; visually verified title')
save('FRONTEND/TITLE.PAL',palette,'CD DATA.DAT record 15, VGA palette; visually verified match')
for name in ['MENU','WIN','LOSE','DRAW']:
 path=f'DIGI/MUSIC/{name}.WAV';sample=extract(path);info=wav_info(sample)
 if (info['rate'],info['channels'],info['sample_width'])!=(22050,2,1):raise ValueError('Unsupported music format')
 save(path,sample,'Original CD '+path);report['assets'][-1]['wave']=info
report['data_dat_sha256']=hashlib.sha256(data).hexdigest();report['data_off_sha256']=hashlib.sha256(offsets).hexdigest()
(root/'docs/frontend-assets.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print('Imported original title and four music tracks into local-data')
