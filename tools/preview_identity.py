"""Conservative cross-revision identities for retail previews, names and appearance."""
import re
import unicodedata

def normal(value):
    return re.sub('[^A-Z0-9]', '', unicodedata.normalize('NFKD', value).encode('ascii', 'ignore').decode().upper())

def player(line, retail=False):
    def field(pattern):
        m=re.search(pattern,line)
        return m[1] if m else None
    name=field(r't:\s*"([^"]+)"')
    code=field(r'digi:\s*"([^"]+)"' if retail else r'name:\s*"([^"]+)"')
    if not name or not code or not re.fullmatch('[A-Z]{3}[0-9]{2}',code):return None
    club=field(r'club:\s*"([^"]+)"');dob=field(r'dob:\s*([0-9]{2}/[0-9]{2}/[0-9]{2})')
    position=field(r'\[POS_([a-z]+)\]' if retail else r'posn:\s*([A-Z]+)')
    if position:position={'FW':'FD'}.get(position.upper(),position.upper())
    stats=tuple(field(r'\b'+key+r':\s*([0-9]+)\b') for key in ('pace','pow','cntl','flair' if retail else 'flar','visn','accr' if retail else 'acc','stam','disc'))
    fingerprint=(code,dob,normal(club),position,*map(int,stats)) if club and dob and position and all(v is not None for v in stats) else None
    animation=field(r'anim:\s*"([^"]+)"');palette=field(r'\[([A-Z]+)PAL\]')
    return dict(name=name,code=code,club=club,dob=dob,position=position,stats=stats,fingerprint=fingerprint,animation=animation,palette=palette)

def match(source, retail):
    country=[r for r in retail if r['code'][:3]==source['code'][:3] and r['animation'] and r['palette']]
    exact=[r for r in country if normal(r['name'])==normal(source['name'])]
    # Multiple authored records must agree on both the identity and its preview.
    identity=lambda r:(normal(r['name']),r['code'],r['animation'],r['palette'])
    if len({identity(r) for r in exact})==1:return exact[0],'normalized_name'
    candidates=[r for r in (exact or country) if source['fingerprint'] is not None and r['fingerprint']==source['fingerprint']]
    identities={identity(r) for r in candidates}
    if len(identities)!=1:return None,None
    return candidates[0],'verified_identity'
