"""Recover frontend image rectangles and country-carousel coordinates from Euro source."""
import hashlib
import json
import re
import sys
from pathlib import Path
from menu_loop_timing import recover as recover_loop_timing

src, out = map(Path, sys.argv[1:3])
loop_timing = recover_loop_timing(src)
script = (src / 'SCRIPT.96').read_text(encoding='latin1')
symbols = {k: int(v) for k, v in re.findall(r'#define\s+(\w+)\s+(\d+)', (src / 'EURO_SYM.H').read_text())}
images = []
group = ''
for line in script.split('[ Text Data ]')[0].splitlines():
    symbol = re.search(r'\x7f\s+(\w+)', line)
    if symbol:
        group = symbol[1]
        if group in symbols and symbols[group] != len(images):
            raise ValueError(f'Image symbol mismatch: {group}')
    m = re.match(r'\s*x:\s*(\d+)\s+y:\s*(\d+)\s+w:\s*(\d+)\s+h:\s*(\d+)\s+p:\s*(\d+)', line)
    if not m:
        continue
    x, y, w, h, page = map(int, m.groups())
    # SetupExtraTexturePages reuses page slots for each frontend screen.
    if page == 0: record = 85
    elif group in ('KIT_IMAGES', 'INFO_NBAR', 'FORM_IMGS', 'FBAL_SHDS', 'SMLL_GOTH'): record = 84 + page
    elif group in ('TSEL_IMGS', 'GERL_SBAR', 'EURO_SELC', 'CNTY_OUTS'): record = 86 + page
    elif group == 'EURO_RDRW': record = 92 + page
    elif group in ('SQUD_BALS', 'UBAR_PANL'): record = 97
    elif group == 'PSBX_PANL': record = 100
    elif group in ('BCNT_IMGS', 'TBAR_STAT', 'TBAR_SPLT'): record = 94 + page
    elif group == 'LRGE_GOTH': record = 99 + page
    elif group in ('PLRG_GOTH', 'PLRG_GSHD'): record = 99 + page
    elif group == 'EUCP_IMGS': record = 105 + page
    else: record = -1  # Runtime scratch/background rectangles are not atlas assets.
    # One carousel frame extends one row beyond its 480-line source page.
    # Keep the available 70 rows; never sample the following texture page.
    if len(images) == 754:
        assert (x, y, w, h, record) == (561, 410, 33, 71, 90)
        h = 70
    images.append((x, y, w, h, record))
assert len(images) == 1159
var = (src / 'EURO_VAR.CPP').read_text(encoding='latin1')
body = re.search(r'CountryImageDEFN\[8\*64\]\s*=\s*\{(.*?)\};', var, re.S)[1]
body = re.sub(r'//[^\n]*', '', body).replace('NULL', '0')
values = []
for value in body.split(','):
    value = value.strip()
    if value:
        assert re.fullmatch(r'[\d\s+\-]+', value)
        values.append(sum(int(x.replace(' ', '')) for x in re.findall(r'[+-]?\s*\d+', value)))
assert len(values) == 64 * 8 * 3
aim = [int(v) for v in re.search(r'AimTBL\[16\]\s*=\s*\{(.*?)\}', var, re.S)[1].split(',') if v.strip()]
selection = (src / 'EURO_SEL.CPP').read_text(encoding='latin1')
direction = re.search(r'void\s+CalculateDir\(.*?\)\s*\{(.*?)\n\s*\}\s*//', selection, re.S)[1]
direction = direction.replace('SelectDir', 'direction').replace('SelectionCounter', 'counter').replace('SelectionAim', 'target_aim')
direction = direction.replace('direction\t=\t1;', 'int direction = 1;')
assert 'int direction = 1;' in direction
for pattern in (r'SelectionSpeed\s*=\s*\.2;', r'SelectionSpeed\s*\*=\s*2;', r'SelectionSpeed\s*=\s*1\.0;', r'fmod\(\s*SelectionCounter\+64,\s*64\)'):
    assert re.search(pattern, selection), 'Country motion source changed'
out.mkdir(parents=True, exist_ok=True)
header = '#pragma once\n#include <array>\nnamespace actua::menu_art {\nstruct Image {int x,y,w,h,record;};\n'
header += 'inline constexpr Image images[]={' + ','.join('{' + ','.join(map(str, v)) + '}' for v in images) + '};\n'
header += 'struct Country {int image,x,y;};\ninline constexpr Country carousel[]={' + ','.join('{' + ','.join(map(str, values[i:i+3])) + '}' for i in range(0, len(values), 3)) + '};\n'
header += 'inline constexpr int aim[]={' + ','.join(map(str, aim)) + '};\n'
header += f'inline constexpr double country_step_seconds=double({loop_timing["wait_ticks"]})/{loop_timing["timer_hz"]};\n'
header += 'inline int country_direction(float counter,int target_aim){' + direction + '\nreturn direction;\n}\n'
header += '\n'.join(f'inline constexpr int {name}={symbols[name]};' for name in ('GEOFONT16','GEOFONT22','GEOFONT30','INFO_NBAR','FORM_IMGS','BUTN_WHTE','BUTN_BLUE','BUTN_WHBL','SMLL_GOTH','SQUD_BALS','GERL_SBAR','UBAR_PANL','LRGE_GOTH','EUCP_IMGS')) + '\n}\n'
(out / 'menu_art_data.hpp').write_text(header, encoding='utf-8')
(out / 'menu-loop-timing.json').write_text(json.dumps(loop_timing, indent=2)+'\n', encoding='utf-8')
(out / 'menu-art-recovery.json').write_text(json.dumps(dict(clipped_sprites={"754":"Carousel frame height 71 to 70: only 70 source rows exist."},images=len(images),carousel_frames=64,fonts=3,loop_timing=loop_timing,inputs={name:hashlib.sha256((src/name).read_bytes()).hexdigest() for name in ('SCRIPT.96','EURO_SYM.H','EURO_VAR.CPP','EURO_GRF.CPP','EURO_GEN.CPP','EURO_SEL.CPP')},boundary='Original image rectangles, page mappings, glyph advance, carousel positions and direction function; source-checked .2/.4/.8/1 acceleration and nominal 30 ms loop cadence. Immediate input selection, menu navigation and extension layouts remain native.'), indent=2)+'\n')
