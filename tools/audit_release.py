"""Compare active original action dispatch cases with the generated adapter.

This is a narrow parity check, not a claim of complete engine restoration.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re


def body(path, function):
    text = path.read_text(encoding="latin-1")
    text = re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)
    match = re.search(r"void\s+" + re.escape(function) + r"\s*\([^)]*\)\s*\{", text)
    if not match:
        raise ValueError(f"Function not found: {function}")
    start = match.end()
    depth = 1
    for end in range(start, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end]
    raise ValueError(f"Unclosed function: {function}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("generated", type=Path)
    parser.add_argument("report", type=Path)
    args = parser.parse_args()
    original = args.source / "ACTIONS.CPP"
    restored = args.generated / "player_original.cpp"
    branches = lambda value: dict(re.findall(r"case\s+(\w+)\s*:\s*(\w+)\s*\(", value))
    expected = branches(body(original, "do_action"))
    actual = branches(body(restored, "PlayerEngine::dispatch_action"))
    if not expected or expected != actual:
        raise ValueError(f"Action dispatch differs: original={expected}; restored={actual}")
    report = {
        "scope": "Active do_action case names and called handlers only; not complete game parity.",
        "original_sha256": hashlib.sha256(original.read_bytes()).hexdigest(),
        "generated_sha256": hashlib.sha256(restored.read_bytes()).hexdigest(),
        "active_cases": expected,
        "matched_count": len(expected),
        "note": "TURN_ACT is commented out in the preserved original dispatcher too.",
    }
    args.report.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"All {len(expected)} active original action-dispatch cases call the corresponding restored handler.")


if __name__ == "__main__":
    main()
