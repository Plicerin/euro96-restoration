# Manual shootout controls, version 0.24.0

Patch 0.24.1 keeps ball/net physics and keeper recovery moving during the
three-second outcome interval. The ball resets when the next kick is set up;
scoring no longer zeroes its velocity or snaps it to ground height on the goal line.
Presentation snapshots stay continuous across the result. Scores are recorded once.

This batch adds five connected pieces: original human penalty timing, an aim/height
guide, original directional keeper saves, keyboard/gamepad input with safe charge
cancellation, and automatic control transfer between the home taker and keeper.

## One batch to try

Use the existing View-Original-Actua96.cmd launcher. Choose Format: Penalties only,
then Y on the keyboard or North/Y/Triangle on a controller to play as the home team.
Enter/South still watches the original CPU control both sides. These controls also
apply to shootouts reached at the end of a knockout or golden-goal match.

- Taking: the yellow aim marker sweeps automatically. Hold keyboard X or the
  controller's West button (Xbox X / PlayStation Square) to increase shot height.
  Release to commit the aim and start the original run-up. A short hold gives a low
  shot; longer holds raise it. Maximum charge is 30 original ticks (1.5 seconds).
- Keeping: hold Up/Down (W/S, vertical left stick or D-pad) and the same shot button
  to request a save toward that side of the pitch. Hold shot without a direction
  for a centre save. The original prediction routine decides when the ball can
  be intercepted; choosing the wrong side uses the original wrong-way animation.
- P/Start pauses. Pausing, opening setup, losing focus or disconnecting the active
  controller cancels an unfinished charge. Release the shot button before charging
  again. A run-up already committed continues when play resumes.
- The home taker/keeper is selected automatically. The opponent stays CPU controlled.
  O/X history and shootout totals remain separate from the regulation score.

The guide shows the engine's current goal-line aim and height setting, not a
promised point of impact. Original player ratings, power, trajectory, animation
contact and keeper prediction still determine the outcome. The guide's Up/Down
labels refer to pitch coordinates and correspond to the keeper controls; its
front-on diagram does not rotate with the selected match camera.

## Provenance and boundaries

SOURCE is untouched. ACTIONS.user_pen_kick already contains the original human
branch: uf1 charges f1_down/setp_hgt, release calls init_runup_act and sets
penalty_shot. Original user_runup/taker_nkick, make_shoot and contact/trajectory
routines release the ball. Native input supplies held button state at the existing
20 Hz rate, with a neutral-input gate after transitions. Human aiming has no timeout.
The original CPU branch and its existing fail-closed timeout are retained.

Build-time recovery enables the EURO96 branches in INTELL.go_to_save_path and
aim_shot_at_goal. The latter excludes penalties from the CPU swerve calculation,
preserving original get_power/get_traj at contact for human and CPU takers. This
fixes the previous integration overriding the penalty height with an automatic
normal-play shot. Release telemetry now waits for physical animation contact.
The native adapter supplies save_direction in place of the missing DOS users_dir
bitmask registry. Original good_choice predicates, save zone selection and
init_user_save wrong-way animation remain intact. The adapter maps vertical pitch
input through the defending end and retains the original penalty match_mode during
save selection, after ready_set_kick has cleared the general restart state. It
calls keeper_busy and the original action dispatcher for recovery, while CPU
intelligence continues to control the other keeper. It does not invent a new
save collision or force a successful catch.

Since 0.25.0, regulation penalty restarts also accept manual taker input; see
[set-piece controls](SET-PIECE-CONTROLS.md). There is no second-human goalkeeper.
The DOS draw_tracker is replaced with a native SDL guide. Source preservation,
physics and archive validation remain part of the release checks.

## Automated coverage

penalty_control_tests checks no automatic human kick or timeout, moving aim,
low/high trajectories, original run-up/contact, the 30-tick height cap, cancellation
and neutral rearming, stable home identity after swapping ends, and twelve complete
human-versus-CPU shootouts with real keeper saves. The SDL penalty smoke uses a
virtual gamepad through the actual event loop to charge, pause, resume with the
button held, rearm, cancel a second charge on focus loss, resume with the button held, rearm
and release a shot, then disconnect the controller. The existing CPU shootout,
extended-match and full regression suites remain enabled.

The penalty_result_tests regression reproduces the pre-fix freeze, then checks
goals at both ends, retained height/velocity, back-net rebounds, interpolated ball
motion, pause, single scoring, next-kick reset, wide misses and eight complete CPU
shootouts with actual keeper catches and continuing result animations. Optional
screenshot output uses SDL software rendering, without opening an interactive game.
