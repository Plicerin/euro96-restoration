"""Compile unchanged, self-contained original routines without DOS headers."""
from pathlib import Path
import re
import sys

source, output = map(Path, sys.argv[1:])
groups = {"EURO_GRF.CPP": ["straightcopy", "spritecopy", "flipsprcopy", "pseudostore"],
          "MATHS.CPP": ["calc_dist"]}
parts = ['// Generated from the preserved SOURCE files. Do not edit.',
         '#include "archive.hpp"', '#include <cmath>', 'using std::sqrt;']
for filename, names in groups.items():
    text = (source / filename).read_text(encoding="latin1")
    for name in names:
        match = re.search(r"^(?:void|float)\s+" + name + r"\s*\([^;]*?\)\s*\{", text, re.M)
        if not match:
            raise SystemExit(f"Missing original routine {filename}:{name}")
        start = match.start()
        position = match.end()
        depth = 1
        while depth and position < len(text):
            depth += (text[position] == "{") - (text[position] == "}")
            position += 1
        if depth:
            raise SystemExit(f"Unterminated routine {name}")
        line = text[:start].count("\n") + 1
        parts += [f'#line {line} "{filename}"', text[start:position]]
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text("\n".join(parts) + "\n", encoding="utf-8")

