"""Import only the validated original pitch bank into local-data, outside SOURCE."""
from pathlib import Path
import argparse,hashlib,subprocess
from inspect_cd_audio import inspect_mad,original_pitch_names
p=argparse.ArgumentParser(description=__doc__);p.add_argument('iso',type=Path);args=p.parse_args()
root=Path(__file__).resolve().parents[1]
data=subprocess.run(['7z','e','-so',str(args.iso.resolve(strict=True)),'DIGI/PITCH/PITCH.MAD'],capture_output=True,check=True).stdout
entries=inspect_mad(data)
assert set(original_pitch_names(root/'SOURCE'))<={e['name'].upper() for e in entries}
path=root/'local-data/DIGI/PITCH/PITCH.MAD'
if path.exists() and path.read_bytes()!=data:raise ValueError('Different pitch bank already installed; not overwritten')
path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(data)
print(f'Imported {len(entries)} original samples: {path}; SHA256 {hashlib.sha256(data).hexdigest()}')
