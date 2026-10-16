"""Exercise the packaged application's seed parsing and actual CPU openings."""
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys

exe, source, folder = map(lambda value: Path(value).resolve(), sys.argv[1:4])
folder.mkdir(parents=True, exist_ok=True)
env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_RENDER_DRIVER='software', SDL_AUDIODRIVER='dummy')


def run(arguments):
    return subprocess.run([str(exe), '--data', str(source), *arguments], cwd=folder,
                          env=env, capture_output=True, text=True, timeout=45,
                          creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)


captures = {}
for name, seed in [('default', None), ('fixed-a', '1'), ('fixed-b', '1'),
                   ('other', '2'), ('fresh-a', 'random'), ('fresh-b', 'random')]:
    image = folder / (name + '.bmp')
    args = ['--cpu-teams', '--match-ticks', '400', '--capture', str(image)]
    if seed is not None:
        args += ['--match-seed', seed]
    result = run(args)
    (folder / (name + '.log')).write_text(result.stdout + result.stderr, encoding='utf-8')
    assert result.returncode == 0, (name, result.stdout, result.stderr)
    matches = re.findall(r'Match start seed: (\d+) \((fixed|fresh)\)', result.stdout)
    assert len(matches) == 1, (name, matches)
    captures[name] = dict(seed=int(matches[0][0]), policy=matches[0][1],
                          sha256=hashlib.sha256(image.read_bytes()).hexdigest())

assert captures['default'] == captures['fixed-a'] == captures['fixed-b']
assert captures['default']['seed'] == 1 and captures['default']['policy'] == 'fixed'
assert captures['other']['seed'] == 2
assert captures['other']['sha256'] != captures['fixed-a']['sha256']
assert captures['fresh-a']['policy'] == captures['fresh-b']['policy'] == 'fresh'
assert captures['fresh-a']['seed'] != captures['fresh-b']['seed']
assert captures['fresh-a']['sha256'] != captures['fresh-b']['sha256']
for seed in ('0', '4294967295', 'random'):
    assert run(['--verify', '--match-seed', seed]).returncode == 0, seed
for seed in ('-1', '4294967296', '12junk', '', '+1', 'randomjunk'):
    result = run(['--verify', '--match-seed', seed])
    assert result.returncode != 0 and 'Match seed must be' in result.stderr, (seed, result.stderr)
result = run(['--verify', '--match-seed'])
assert result.returncode != 0 and 'Missing match seed' in result.stderr
(folder / 'report.json').write_text(json.dumps(captures, indent=2) + '\n', encoding='utf-8')
print('Seed CLI: repeated fixed-seed captures match, different/fresh seeds change CPU openings, bounds and malformed arguments verified.')
