"""Extract Euro fixture slots, default draw and ranking body from preserved source."""
from pathlib import Path
import re,sys,json,hashlib
src,out=map(Path,sys.argv[1:]);out.mkdir(parents=True,exist_ok=True)
var=(src/'EURO_VAR.CPP').read_text(encoding='latin1');defs=(src/'EURO_DEF.H').read_text(encoding='latin1');script=(src/'SCRIPT.96').read_text(encoding='latin1');fix=(src/'EURO_FIX.CPP').read_text(encoding='latin1')
codes={k:int(v) for k,v in re.findall(r'#define\s+(TEAM_\w+)\s+(\d+)',defs)}
block=var[var.index('fixture_info\tFixtures'):var.index('char\tMatchVenues')]
fixtures=[(codes[a],codes[b]) for a,b in re.findall(r'(TEAM_\w+),\s*(TEAM_\w+),',block)]
draw=[codes['TEAM_'+code] for code in re.findall(r't:\s*"[^"]+"\s+form:\s*[\d-]+\s+draw:\s*(\d[a-d])',script)]
# Retail descriptor/label order, independently pinned by verify_euro96_venues.py.
# EURO_DEF.H's city defines are commented out in the preserved snapshot.
cities=['London','Birmingham','Leeds','Newcastle','Manchester','Liverpool','Sheffield','Nottingham']
venue_block=re.search(r'char\s+MatchVenues\[32\]\s*=\s*\{(.*?)\};',var,re.S).group(1)
venue_block=re.sub(r'//[^\n]*','',venue_block)
venues=[cities.index(name.strip())+5 for name in venue_block.split(',') if name.strip()]
assert len(fixtures)==len(venues)==31 and sorted(draw)==list(range(16))
a=fix.index('void CalculateLeagueTables()');b=fix.index('//****************************************************************',a);body=fix[a:b].strip()
# Retain all calculation loops, excluding the optional diagnostic printf.
body='void CalculateLeagueTables() {\n'+body[body.index('for ( char t='):]
body=re.sub(r'if \( EUROverbose != 0 \)\s*printf\(.*?;', '', body, flags=re.S)
body=body.replace('fflush(stdout);','')
body=body.replace('void CalculateLeagueTables()', 'void OriginalTournament::rank()')
body=body.replace('LeaguePositions','positions').replace('GroupDrawInfo','draw').replace('LeagueTable','table').replace('WinPts','3').replace('DrawPts','1')
# Above brace removal must retain points and goalDiff assignments.
assert 'table[t].points' in body and 'table[t].goalDiff' in body and 'char scans' in body
cpp=['#include "original_tournament.hpp"','namespace actua {', 'const std::array<int,16> OriginalTournament::default_draw={'+','.join(map(str,draw))+'};','const std::array<std::array<int,2>,31> OriginalTournament::fixture_slots={{'+','.join('{%d,%d}'%f for f in fixtures)+'}};','const std::array<int,31> OriginalTournament::fixture_venues={'+','.join(map(str,venues))+'};',body,'}']
(out/'tournament_original.cpp').write_text('\n'.join(cpp),encoding='utf-8')
(out/'tournament-recovery.json').write_text(json.dumps(dict(fixtures=fixtures,default_draw=draw,fixture_venues=venues,venue_id_mapping='retail 0-7 to native 5-12',ranking='Original points, wins, goal difference; equal ties retain source draw positions',inputs={f:hashlib.sha256((src/f).read_bytes()).hexdigest() for f in ['EURO_FIX.CPP','EURO_VAR.CPP','EURO_DEF.H','EURO_MAT.CPP','EURO_DSK.CPP','SCRIPT.96']},adaptations=['Remove verbose diagnostics; use owned arrays and int statistics','Accumulate goals instead of EURO_MAT overwriting last-match goals','Native result ledger, validated atomic save, menu navigation and original-engine background simulation']),indent=2),encoding='utf-8')
