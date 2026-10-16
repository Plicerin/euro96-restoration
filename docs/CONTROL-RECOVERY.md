# Playable original actions - 0.17.0

This batch connects five related behaviors in one original 3D practice view:

1. Directional running, turning and stopping.
2. Dribbling with the original stride-dependent ball placement.
3. Ground passing with original kick choice and contact timing.
4. Lofted passing with original ball flight and reception animations.
5. Driven shooting in the player's facing direction, including goal/frame/net collisions.

Open the existing View-Original-Actua96.cmd and press **T** to take control.
The command-line equivalent is `--original-play`. From version 0.18.0,
controller **Start** from replays enters [team practice](TEAM-RECOVERY.md);
Start within either playable mode pauses/resumes it.
Both players are teammates. Control transfers to the receiver on collection.

| Action | Keyboard | Controller |
| --- | --- | --- |
| Move / face | WASD or arrows | Left stick or D-pad |
| Ground pass | Space | A / Cross |
| Lofted pass | C | Y / Triangle |
| Driven shot | X | X / Square |
| Switch player | E | B / Circle or LB / L1 |
| Pause / resume | P | Start / Options |
| Reset practice | R | R3 while paused |
| Return to action demonstration | Q | Unmapped (Back shortcut removed in 0.43.2) |

The shot follows facing; turn before kicking. Original shooting range is enforced.
Passes use the original receiver/distance/angle decisions, so an out-of-range
request can be rejected and a long ground-pass request can select a chip.
Lofted passes require a compatible facing angle. Held movement resumes when the
receiver's original control animation finishes. A loose ball can be recovered by
moving into the original foot-contact range. Goals and out balls settle briefly,
then R resets the two-player setup. There are no match restarts in this view.

## Original routines and native boundaries

Six more complete bodies compile from the preserved sources: ACTIONS.user_stand
and user_run; INTELL.make_shoot; and BALLINT.control_difficulty, active_control
and control_ball. Together with earlier work, player/action recovery contains
170 complete routines plus seven explicitly documented excerpts/adapters.
Source files remain unchanged; generated reports record the body hashes.

The new native/original_control.cpp connects SDL input to these routines and the
already recovered init_run_act, go_forward, process_dir, hold_ball, pass_decide,
make_pass, shoot_decide, kick_action, interception and ball-physics routines.
The original movement constants, turning limits, kick captures, contact offsets,
shot speed, driven-shot elevation, and receive decisions are retained.

This is a native two-player input fixture, not the full original user_play or
process_teams loop. It supplies the roster, initial possession, button mapping,
input normalization, neutral-input stopping, selected player, receiving control
button, reset and end-of-play pause. The second player waits under neutral input
until a pass requests original interception; full off-ball team AI is not active.
The original defensive speed burst is not exposed as a general sprint button.

The fixture dispatches original animated control contact and a restricted normal
foot-contact path. The latter preserves ball_interact's reach, low-ball height,
previous-height and kick-busy gates before calling control_ball; PLAYER_HEIGHT/2
retains integer division. Tackles, fouls, keeper interaction and general body/head
collisions are outside this fixture. The single shot button selects the original
driven-shot branch. It does not reproduce the original two-button special-kick
menu. No new pass-homing or receiver-magnet behavior was introduced.

The camera, HUD and SDL keyboard/gamepad mapping are native presentation/input.
Camera tracking samples interpolated player positions and looks ahead toward the
attacking goal. Focus loss or active-controller disconnection pauses practice;
pending actions are discarded. Reset while paused on a controller stays paused.

## Verification

Control contracts cover eight movement directions, facing convergence, dribble
distance, neutral stopping, invalid input, off-ball switching, ground and lofted
reception, held movement after reception, six successive passes, queued-action
cancellation, loose-ball recovery, shot count/direction/range, out-of-play stop
and reset. The native SDL smoke uses a virtual gamepad for movement, both passes,
shooting, pause, paused reset and disconnect, plus keyboard entry, selection and
exit. Physical-controller feel still needs user testing.

All prior action, animation, physics, goal collision and practice tests remain
part of the full suite. The full original match loop, team AI, scoring/restarts
and audio playback remain unfinished.

Release validation: all 34 CTest suites passed in 72.79 seconds. The source audit
passed, and `git diff --exit-code -- SOURCE` confirmed that the preserved source
was unchanged. The captured practice view was visually reviewed for player,
ball, goal and HUD visibility.

![Original action practice](original-control.bmp)
