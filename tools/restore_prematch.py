"""Recover original lineup presentation; apply only to a copy of the chosen squad."""
from pathlib import Path
import hashlib, json, re, struct, sys
from omf import Object
from restore_ball import routine


def generate(source, output):
    source, output = Path(source), Path(output)
    football, actions = Object(source/'FOOTBALL.OBJ'), Object(source/'ACTIONS.OBJ')
    for name, digest in {
        'FOOTBALL.OBJ': '80386fd58625e1a5a8491cdf700096af782636923ba40b778746274c8663430a',
        'ACTIONS.OBJ': 'b02ab5c3177e8744c9c954c1d66148d6b3d9270fef647637192189ac4da87f31',
    }.items():
        if hashlib.sha256((source/name).read_bytes()).hexdigest() != digest:
            raise ValueError('Unknown lineup object '+name)
    code = football.segments[1]['data']
    assert bytes(code[0x947:0x94a]) == b'\x66\xc7\x05'
    total = struct.unpack_from('<H', code, 0x94e)[0]  # init_line_up immediate
    assert total == 180
    code = actions.segments[1]['data']
    assert bytes(code[0x48d:0x48f]) == b'\xc7\x05'
    plead = struct.unpack_from('<f', code, 0x493)[0]  # refs[0].fstep immediate
    assert plead == 1/64
    files = {name: (source/name).read_text(encoding='latin1') for name in
             ['ACTIONS.CPP', 'FOOTBALL.CPP', '3D_UPD2.CPP', 'RULES.CPP', 'EURO_GEN.CPP', 'EURO_DEF.H', 'TEST.CPP', 'TIMER.H']}
    team, team_line = routine(files['ACTIONS.CPP'], 'init_team')
    fields = ['tm_x', 'tm_y', 'tm_z', 'tm_xdis', 'tm_ydis', 'tm_zdis',
              'tm_limbo', 'control', 'tm_frm', 'tm_fstep', 'tm_anim',
              'contact', 'tm_newanim', 'tm_act', 'tm_poss']
    assignments = []
    for side in range(2):
        assignments.append(f'for(int i={side*11};i<{(side+1)*11};++i){{')
        for field in fields:
            found = re.findall(r'teams\[i\]\.'+field+r'=[^;]+;', team)
            assert len(found) == 2, (field, found)
            assignments.append(found[side])
        assignments.append('}')
    officials, official_line = routine(files['ACTIONS.CPP'], 'init_officials')
    animation, animation_line = routine(files['FOOTBALL.CPP'], 'animate_players')
    animation = animation.replace('for (i=0;', 'for (int i=0;')
    text = files['3D_UPD2.CPP']
    start = text.index('float rad=', text.index('// Pre-match Line-up!!!'))
    camera = text[start:text.index('/*', start)].strip()
    camera_line = text[:start].count('\n')+1
    ref, ref_line = routine(files['RULES.CPP'], 'process_ref')
    ref = ref[ref.index('if (match_mode==SWAP_ENDS)'):ref.index('else')].strip()
    assert ref == 'if (match_mode==SWAP_ENDS)\n\t\tinit_refs_anim(&refs[0],MC_STAND);'
    lman, lman_line = routine(files['RULES.CPP'], 'process_lman')
    lman = lman[lman.index('{')+1:-1]
    for name in ['lman_follow', 'goto_target']:
        lman = lman.replace(name+'(', 'e.'+name+'(')
    fade_steps = int(re.search(r'#define\s+FADE_COUNTER\s+(\d+)', files['EURO_DEF.H'])[1])
    assert fade_steps == 25
    assert 'red / FADE_COUNTER' in files['EURO_GEN.CPP']
    assert '(fixed) (*(a)) + (*(b++))' in files['EURO_GEN.CPP']
    assert '(fixed) (*(a)) - (*(b++))' in files['EURO_GEN.CPP']
    fade_hz = int(re.search(r'AddTimer\(\s*(\d+),\s*EUROinterupt,\s*EuroHandle\s*\)', files['TEST.CPP'])[1])
    assert fade_hz == 63
    assert 'sosTIMERRegisterEvent(rate,handler,&handle)' in files['TIMER.H']
    (output/'menu_fade_data.hpp').write_text('#pragma once\nnamespace actua { inline constexpr int original_menu_fade_steps='+str(fade_steps)+'; inline constexpr int original_menu_fade_hz='+str(fade_hz)+'; }\n')
    cpp = ['#include "original_prematch.hpp"', '#include "original_exit.hpp"', '#include "action_constants.hpp"',
           '#include <algorithm>', '#include <cmath>', 'namespace actua {',
           'using namespace original::action_constants;', 'constexpr bool TRUE=true;constexpr float PI=3.1415926536f;',
           f'constexpr int LINE_UP_DELAY={total-40};', f'constexpr float MC_PLEAD_FS={plead}f;',
           'void init_original_lineup(original::PlayerEngine& e){',
           'auto& teams=e.teams;auto& refs=e.refs;',
           'const auto cntspot_x=e.cntspot_x,cntspot_y=e.cntspot_y,pitch_wid=e.pitch_wid;const auto prat=e.prat;',
           f'#line {team_line} "ACTIONS.CPP"', *assignments,
           f'#line {official_line} "ACTIONS.CPP"', officials[officials.index('{')+1:-1], '}',
           'void animate_original_lineup(original::PlayerEngine& e){',
           'auto& teams=e.teams;auto& refs=e.refs;constexpr int players=22;',
           f'#line {animation_line} "FOOTBALL.CPP"', animation[animation.index('{')+1:-1], '}',
           'CameraPose original_lineup_camera(const original::PlayerEngine& e,int remaining){',
           'const int line_up=std::clamp(remaining,40,LINE_UP_DELAY+40);',
           'const float ballx=e.ballx,bally=e.bally,ball_diam=e.ball_diam;',
           'float tx,ty,tz,camera_x,camera_y,camera_z;',
           f'#line {camera_line} "3D_UPD2.CPP"', camera,
           'return {camera_x,camera_y,camera_z,tx,ty,tz,440};', '}',
           'void step_original_exit_officials(original::PlayerEngine& e){',
           'const auto previous_mode=e.match_mode;e.match_mode=SWAP_ENDS;',
           'auto& refs=e.refs;const auto match_mode=e.match_mode;',
           f'#line {ref_line} "RULES.CPP"', ref.replace('init_refs_anim(', 'e.init_refs_anim('),
           'for(int i=1;i<3;++i){auto* offc=&refs[i];', f'#line {lman_line} "RULES.CPP"', lman, '}',
           'for(auto& ref:refs)e.anim_officials(&ref);e.match_mode=previous_mode;', '}', '}']
    (output/'prematch_original.cpp').write_text('\n'.join(cpp)+'\n')
    (output/'prematch-recovery.json').write_text(json.dumps(dict(
        countdown=total, orbit_ticks=total-40, hold_ticks=40, plead_fstep=plead,
        inputs={name: hashlib.sha256((source/name).read_bytes()).hexdigest() for name in
                [*files, 'FOOTBALL.OBJ', 'ACTIONS.OBJ']},
        source=dict(players=team_line, officials=official_line, animation=animation_line, camera=camera_line, exit_referee=ref_line, exit_assistants=lman_line),
        menu_fade=dict(steps=fade_steps, hz=fade_hz, registration='TEST.CPP AddTimer(63, EUROinterupt, EuroHandle), TIMER.H sosTIMERRegisterEvent', curve='EURO_GEN.CPP EUROinterupt: channel/FADE_COUNTER subtracted on fade-down and added on fade-up', native_boundary='Recovered 63 Hz nominal fade cadence with SDL composition of retained outgoing and incoming RGB menus; exact DOS palette quantization, interrupt jitter and menu navigation are not reproduced.'),
        adaptations=['Copy selected squad; retain stats, shirts and availability.',
                     'Original integer camera radius at 20 Hz; native interpolation between ticks.',
                     'Hold final orbit pose for the last 40 ticks, as source does.',
                     'Native pause/skip; live kickoff state held unchanged during presentation.',
                     'Source officials use MC_PLEAD for referee and MC_STAND for assistants; object revision assigns animation 78 to all three.']), indent=2)+'\n')


if __name__ == '__main__':
    generate(*sys.argv[1:])
