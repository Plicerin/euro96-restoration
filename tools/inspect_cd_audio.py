"""Read original CD audio through 7-Zip without mounting or running disc software."""
import argparse
from collections import Counter
import hashlib
import io
import json
from pathlib import Path
import shutil
import struct
import subprocess
import wave
from omf import Object


def wav_info(data):
    if data[:4] != b"RIFF" or data[8:12] != b"WAVE":
        raise ValueError("Not a RIFF WAVE sample")
    with wave.open(io.BytesIO(data), "rb") as wav:
        channels, width, rate, frames, compression, _ = wav.getparams()
        if compression != "NONE" or len(wav.readframes(frames)) != frames * channels * width:
            raise ValueError("Compressed or truncated WAV")
    return dict(channels=channels, sample_width=width, rate=rate, frames=frames,
                compression=compression, seconds=round(frames / rate, 3))


def inspect_mad(data):
    if len(data) < 24:
        raise ValueError("Truncated MAD directory")
    directory_size = struct.unpack_from("<I", data, 16)[0]
    if not 24 <= directory_size <= len(data) or directory_size % 24:
        raise ValueError("Invalid MAD directory size")
    entries = []
    end = directory_size
    for position in range(0, directory_size, 24):
        raw_name, offset, size = struct.unpack_from("<16sII", data, position)
        name = raw_name.split(b"\0", 1)[0].decode("ascii")
        if not name or offset != end or size < 44 or size > len(data) - offset:
            raise ValueError(f"Invalid or noncontiguous MAD entry: {name!r}")
        sample = data[offset:offset + size]
        entries.append(dict(name=name, offset=offset, size=size,
                            sha256=hashlib.sha256(sample).hexdigest(), **wav_info(sample)))
        end = offset + size
    if end != len(data):
        raise ValueError("Unexpected trailing MAD data")
    return entries


def original_pitch_names(source):
    obj = Object(source / "AUDIO.OBJ")
    symbol = obj.named("SampleBank")
    data = obj.segments[symbol["segment"]]["data"]
    fixups = {(f["segment"], f["offset"]): f for f in obj.fixups(True, True)}
    names = []
    # AUDIO.H: 20 memsam entries. Watcom's 32-bit memsam stride is 16 bytes.
    for index in range(20):
        offset = symbol["offset"] + index * 16
        fixup = fixups[(symbol["segment"], offset)]
        if fixup["kind"] != 9 or fixup["relative"] or fixup["target_method"] != 0:
            raise ValueError("Unexpected SampleBank relocation")
        target = obj.segments[fixup["target_index"]]["data"]
        start = struct.unpack_from("<I", data, offset)[0] + fixup["displacement"]
        name = bytes(target[start:]).split(b"\0", 1)[0].decode("ascii")
        if not name or len(name) > 8:
            raise ValueError("Unexpected pitch sample name")
        names.append(name + ".WAV")
    return names


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("iso", type=Path)
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--source", type=Path, default=Path(__file__).resolve().parents[1] / "SOURCE")
    args = parser.parse_args()
    seven_zip = shutil.which("7z")
    if not seven_zip:
        parser.error("7z must be installed and on PATH")
    iso = args.iso.resolve(strict=True)
    listing = subprocess.run([seven_zip, "l", "-slt", str(iso)], capture_output=True, check=True).stdout.decode("utf-8-sig")
    records = []
    for block in listing.replace("\r\n", "\n").split("----------\n", 1)[1].split("\n\n"):
        fields = dict(line.split(" = ", 1) for line in block.splitlines() if " = " in line)
        if fields.get("Folder") == "-":
            records.append(dict(path=fields["Path"].replace("\\", "/"), size=int(fields["Size"])))
    audio = [f for f in records if f["path"].upper().startswith("DIGI/") and Path(f["path"]).suffix.upper() in (".MAD", ".WAV")]
    with iso.open("rb") as stream:
        iso_hash = hashlib.file_digest(stream, "sha256").hexdigest()
    report = dict(iso=dict(name=iso.name, size=iso.stat().st_size, sha256=iso_hash), files=records, audio=[])
    for file in audio:
        data = subprocess.run([seven_zip, "e", "-so", str(iso), file["path"]], capture_output=True, check=True).stdout
        if len(data) != file["size"]:
            raise ValueError(f"Extraction size mismatch: {file['path']}")
        result = dict(**file, sha256=hashlib.sha256(data).hexdigest())
        if file["path"].upper().endswith(".MAD"):
            result["entries"] = inspect_mad(data)
            result["duplicate_names"] = [name for name, count in Counter(e["name"].lower() for e in result["entries"]).items() if count > 1]
        else:
            result.update(wav_info(data))
        report["audio"].append(result)
    pitch = next(bank for bank in report["audio"] if bank["path"].upper() == "DIGI/PITCH/PITCH.MAD")
    available = {entry["name"].upper() for entry in pitch["entries"]}
    expected = original_pitch_names(args.source)
    missing = sorted(set(expected) - available)
    report["source_pitch_compatibility"] = dict(expected=expected, missing=missing,
        audio_obj_sha256=hashlib.sha256((args.source / "AUDIO.OBJ").read_bytes()).hexdigest())
    if missing:
        raise ValueError(f"Missing original pitch samples: {missing}")
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"Validated {len(audio)} audio files; all {len(expected)} original pitch names match. Report: {args.report}")


if __name__ == "__main__":
    main()
