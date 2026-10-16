"""Stage a bounded, verified match asset pack; never copy user saves or CD films."""
import argparse
import hashlib
import json
import shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CORE = [f"{stem}.{ext}" for stem in ("EURO", "EUROREND", "FAP", "FAPF") for ext in ("DAT", "OFF")]
MEDIA = [
    "GRAPHICS/TEAMS.BIN", "GRAPHICS/EURO96.BIN", "GRAPHICS/NUMBERS.BIN", "GRAPHICS/PLAYERS.BIN",
    "GRAPHICS/EURO_BOARDS.BIN",
    *[f"GRAPHICS/EURO_STADIUM{i}.BIN" for i in range(8)],
    "COMMENTARY/EURO_EVENTS.BIN", "COMMENTARY/EURO_NAMES.BIN",
    "COMMENTARY/EURO_PERIODS.BIN", "COMMENTARY/EURO_SEQUENCES.BIN",
    "DIGI/PITCH/EURO_PITCH.MAD", "DIGI/MUSIC/EURO96.WAV",
    "DIGI/MUSIC/WIN.WAV", "DIGI/MUSIC/LOSE.WAV", "DIGI/MUSIC/DRAW.WAV",
]


def stage(output: Path):
    files = []
    for root, prefix, names in ((ROOT / "SOURCE", "data", CORE), (ROOT / "local-data", "audio", MEDIA)):
        for name in names:
            source = root / name
            if not source.is_file():
                raise FileNotFoundError(f"Required browser asset is missing: {source}")
            digest = hashlib.sha256(source.read_bytes()).hexdigest()
            relative = f"assets/{prefix}/{name}"
            target = output / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            if not target.exists() or hashlib.sha256(target.read_bytes()).hexdigest() != digest:
                shutil.copyfile(source, target)
            files.append(dict(path=f"/{prefix}/{name}", url=relative, bytes=source.stat().st_size, sha256=digest))
    manifest = dict(schema=1, restorationVersion="0.70.0", preview="browser-proof", files=files,
                    excluded=["Trophy films", "Animated menu previews", "French commentary", "Actua compatibility packs"])
    (output / "assets.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"Browser assets: {len(files)} files, {sum(f['bytes'] for f in files) / 1048576:.2f} MiB")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    stage(parser.parse_args().output.resolve())
