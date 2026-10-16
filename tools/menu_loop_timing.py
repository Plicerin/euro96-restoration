"""Recover the nominal frontend loop cadence, separately from palette fades."""
import hashlib
import re


def recover(source):
    names = ('DEFINES.H', 'TEST.CPP', 'NETWORK.C', 'TIMER.H')
    raw = {name: (source/name).read_text(encoding='latin1') for name in names}
    text = {name: re.sub(r'/\*.*?\*/|//[^\n]*', '', value, flags=re.S)
            for name, value in raw.items()}

    def need(pattern, name):
        match = re.search(pattern, text[name], re.S)
        if not match:
            raise ValueError('Frontend timer evidence changed: '+name)
        return match

    real = int(need(r'#define\s+REAL_SPEED\s+(\d+)', 'DEFINES.H')[1])
    fraction = int(need(r'#define\s+TIMER_FRAC\s+(\d+)', 'DEFINES.H')[1])
    need(r'#define\s+TIMER_SPEED\s+\(TIMER_FRAC\*REAL_SPEED\)', 'DEFINES.H')
    need(r'AddTimer\(\s*TIMER_SPEED\s*,\s*nethandler\s*,\s*NetworkHandle\s*\)', 'TEST.CPP')
    need(r'#define\s+AddTimer\(rate,handler,handle\)\s+sosTIMERRegisterEvent\(rate,handler,&handle\)', 'TIMER.H')
    need(r'void\s+__far\s+nethandler\(void\)\s*\{\s*int i;\s*count\+\+;', 'NETWORK.C')
    loop = need(r'while\s*\(current_menu==new_menu\)\s*\{\s*do\s*\{\s*\}\s*while\s*\(count<TIMER_SPEED/(\d+)\);\s*count\s*=\s*0;', 'TEST.CPP')
    body = text['TEST.CPP'][loop.end():]
    # Team movement is called once per paced frontend iteration, not by the
    # separate 63 Hz palette interrupt. Pin that call's surrounding sequence.
    need(r'GetConsoleInput\(\);\s*ControlAnimations\(\);.*?DoSquadSelection\([^;]+;\s*DoTeamSelection\(\s*Menu,\s*Bounding_box\s*\);', 'TEST.CPP')
    if body.count('DoTeamSelection(') != 1:
        raise ValueError('Frontend team selection call count changed')
    hz = real*fraction
    ticks = hz//int(loop[1])  # The source uses integer division: 200/30 == 6.
    if (hz, ticks) != (200, 6):
        raise ValueError('Unreviewed frontend timer cadence')
    return dict(timer_hz=hz, wait_ticks=ticks, seconds_per_step=ticks/hz,
                nominal_steps_per_second=hz/ticks,
                inputs={name: hashlib.sha256((source/name).read_bytes()).hexdigest() for name in names},
                evidence='TEST.CPP registers nethandler at TIMER_SPEED; NETWORK.C increments count unconditionally; the frontend waits for TIMER_SPEED/30 integer ticks, clears count, and calls DoTeamSelection once.',
                boundary='Nominal source cadence only. DOS workload delays, timer jitter, retail-binary equivalence and navigation are not claimed.')
