# Manual match restarts and aiming (0.25.1)

Home-team penalties, direct/indirect free kicks, corners, throws and goal kicks
now accept human input during a normal match. Away restarts and CPU-versus-CPU
matches retain their original CPU decisions. Kickoffs remain automatic.

## Try the batch

Open the existing stable launcher when ready. In Original 3D, press **F5** to
start a separate practice match. Further F5 presses cycle penalty, direct free
kick, indirect free kick, corner, throw-in, goal kick and keeper distribution.
F5 resets this practice match; do not use it to preserve an ongoing match.
Use F2 to return to match setup. The current running executable is not replaced
or restarted when a new build is published.

| Situation | Keyboard | Gamepad |
| --- | --- | --- |
| Free kick, corner, goal kick: rotate aim | Left/right or A/D | Stick/D-pad left/right |
| Adjust kick power/run-up | Up/down or W/S | Stick/D-pad up/down |
| Raise shot height, then kick | Hold X, release | Hold West, release |
| Pass from a dead ball | Hold Space until positioned, release | Hold South, release |
| Lofted pass from a dead ball | Hold C until positioned, release | Hold North, release |
| Penalty | Time the sweeping aim; hold X for height, release | Hold West for height, release |
| Throw-in | Direction + Space to teammate, X directional, C high | Direction + South / West / North |
| Keeper holding ball | Aim with movement; Space pass, C loft, X punt | Aim; South pass, North loft, West punt |
| Pause/resume | P | Start |

Throw direction follows the original inward/outward rule: outward input selects
a high throw and reflects it back into the pitch. Neutral direction uses the
taker's inward facing. For kicks, animated bright dots show the predicted flight
path and dark dots show its projection on the grass. Large POWER and HEIGHT bars
sit beside the pitch; penalties use fixed original power, labeled POWER: AUTO.
The sidebar shows only the relevant controls while taking a set piece. Pause,
focus loss and disconnected controllers cancel a pending charge; release all
action buttons before rearming.

Keepers finish the original catch/get-up animation before accepting delivery.
Control returns to an outfield player after the release. During normal-match
penalties the opposing keeper remains CPU controlled and rebounds continue in
live play. The separate shootout mode retains its manual home keeper controls.

## Original/native boundary

Recovered from ACTIONS.CPP: new_user_spec_kick, taker_pass_f, taker_pass_h,
taker_tee_up, taker_kick, taker_chip, taker_grnd and user_throw. Existing
user_pen_kick, user_runup, ready_set_kick, make_pass, make_punt, keeper recovery,
ball contact, trajectory and collision routines supply the actual behavior.

If no suitable keeper outlet is available, pass buttons use the original
directional pass fallback with the original roll/throw animation and physics.

Native code supplies SDL input, neutral-button gating, phase scheduling, HUD,
practice placement, selection and keeper delivery buttons. Original DOS user
registry reads map to native direction fields. ROT_SPEED comes from the preserved
source definition. The recovered draw_tracker routine uses original predict_ball
physics on an isolated engine snapshot. Original addobjsp projects the dots into
each camera and the renderer retains original depth sorting and palette IDs
31/2. SDL enlarges the original single-pixel dots for readability. Native bars
and simplified text supplement that original guide; they are not recovered DOS
widgets. The phase follows the 20 Hz simulation/presentation clock, freezes on
pause, and interpolation is bounded to the original 50-point prediction table.
The guide also appears at minimum power and disappears when the run-up begins.
The source's set-piece routine differs from the object snapshot; restored
behavior follows the available source body, not a claimed exact reconstruction
of the later object implementation. SOURCE remains unchanged.

Verification covers all restart types at both ends, ground/loft/shot release,
waiting without auto-kicking, pause cancellation, aim without ball movement,
normal match versus shootout accounting, keeper deliveries and real CPU-match
catch/recovery. A virtual SDL controller exercises F5 and all seven cases through
the application's real input loop.

Tracker checks cover both ends, all three cameras, minimum/maximum height,
bounded dot/shadow pairs, shootouts, pause cancellation without a logic tick,
and exact live ball/random-state equality with and without repeated rendering.
