"""Inventory reconstruction dependencies without pretending to preprocess every variant."""
import argparse
import hashlib
import json
import re
import struct
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("--source", type=Path, default=Path("SOURCE"))
parser.add_argument("--output", type=Path, default=Path("docs/source-audit.json"))
args = parser.parse_args()
root = args.source
files = sorted(p for p in root.rglob("*") if p.is_file())
names = {}
for p in files:
    names.setdefault(p.name.lower(), []).append(p.relative_to(root).as_posix())
def active_text(p):
    s = p.read_text(encoding="latin1")
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", s, flags=re.S)
system_headers = {"time.h", "memory.h"}
missing = {}
relocated = {}
for p in files:
    if p.suffix.lower() not in (".h", ".c", ".cpp"):
        continue
    for include in re.findall(r'^\s*#\s*include\s*"([^"]+)"', active_text(p), re.M):
        normalized = include.replace("\\", "/").lower()
        key = normalized.rsplit("/", 1)[-1]
        if key in system_headers:
            continue
        if key not in names:
            missing.setdefault(include, []).append(p.relative_to(root).as_posix())
        elif not (p.parent / include).is_file():
            relocated.setdefault(include, {"available": names[key], "used_by": []})["used_by"].append(p.relative_to(root).as_posix())
link = active_text(root / "INFO.LNK")
objects = re.findall(r"^FILE\s+(\S+)", link, re.M)
missing_objects = []
for obj in objects:
    stem = Path(obj).stem.lower()
    if not any(stem+ext in names for ext in (".c",".cpp",".asm")):
        missing_objects.append(obj)
archives = {}
for stem in ("EURO","EUROREND","FAP","FAPF"):
    d, o = (root/(stem+".DAT")).read_bytes(), (root/(stem+".OFF")).read_bytes()
    if len(o)%8: raise RuntimeError(stem+" has truncated offsets")
    records = list(struct.iter_unpack("<II", o))
    if any(at>len(d) or size>len(d)-at for at,size in records):
        raise RuntimeError(stem+" has out-of-bounds records")
    archives[stem]={"bytes":len(d),"records":len(records),"sha256":hashlib.sha256(d).hexdigest()}
result = {
    "upstream":"https://github.com/videogamepreservation/actuasoccer96",
    "upstream_commit":"01a824099a278043a9e4a6dfd55ea980dd4ce8bb",
    "method":"Quoted-include scan with comments removed. Conditional compilation branches are not evaluated. SDK/system includes are not exhaustively audited.",
    "missing_includes":missing,
    "relocated_includes":relocated,
    "linked_objects_without_matching_source":missing_objects,
    "validated_archives":archives
}
args.output.parent.mkdir(parents=True,exist_ok=True)
args.output.write_text(json.dumps(result,indent=2)+"\n",encoding="utf-8")
print(f"{len(missing)} missing include names; {len(missing_objects)} linked objects without source; 4 archives validated.")

