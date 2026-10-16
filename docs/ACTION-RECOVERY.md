# Ball continuity correction - 0.14.1

User feedback identified erratic ball movement during goalkeeper distribution and
final control. The 0.14.0 default reception trace reproduced a 19.32-world-unit
jump within one 60 Hz frame: a no-ball marker (-10000) entered a pose blend,
forcing a ground-level physics fallback before the ball suddenly reappeared at
the chest. The control fixture also omitted part of the original contact branch.

Corrections:

- Extract the intact non-upward-header CONTROL_ACT branch of BALLINT.CPP's
  control_ball: collect possession, fix facing mode, restore standard_fstep and
  retain ball_limbo until the capture approaches completion. The helper guards
  its supported action; upward-header control remains unsupported.
- Preserve the fixture's selected receiver under neutral input after contact,
  preventing idle direction updates from pursuing the obsolete approach target.
- In the native presentation adapter, never interpolate absent captured-ball
  coordinates. Keep the extracted original blend function unchanged.
- Sample between adjacent original capture frames, without inventing a wrap at
  the final frame or interpolating between a valid ball and an absent marker.
- Use one displayed_ball path in both rendering and tests. On owner, capture or
  clip changes, interpolate between displayed endpoints for that logic interval.
  This is a native visual handoff; original flight/contact coordinates are unchanged.

The control fix retains default catch=59, distribution=171 and reception=208;
completion changes from tick 226 to 229 because the original post-contact capture
speed is now restored. Nearby outlets finish at ticks 230 and 235. The generator
now records 148 complete bodies plus seven excerpts/adapters (155 entries), with
the kick-contact excerpt recorded separately. SOURCE is unchanged.

The new ball-presentation regression checks actual displayed world coordinates,
missing markers, subframe sampling, continuity across logic boundaries, chest
height, final stability and three outlet scenarios at 30/60/144 Hz. At 60 Hz the
largest default reception step is now 3.90 world units (including incoming flight),
versus the erroneous 19.32-unit jump. Keeper tests distinguish keeper release
limbo from the newly restored receiver-control limbo. SDL rendering tests cover
both the keeper release and final reception.

Validation: all 29 CTests passed (37.10 seconds), including SDL rendering of the
complete sequence. The source/archive audit passed and SOURCE has no diff.

The entries below describe earlier checkpoints and their validation at the time.

---

# Animation presentation restoration - 0.14.0

This five-part batch addresses the slight hitching reported in 0.13.0:

1. Cross-animation pose blending from the original renderer.
2. Original mirrored-limb mapping, including left/right transition handling.
3. Position, direction and animation-phase interpolation between logic ticks.
4. Interrupted transitions, new-animation phases, looping frames and replay resets.
5. Captured-ball placement using the displayed pose and a shared ownership timeline.

`restore_graphics.py` extracts the blend branch of `3DENG.C` intact. Its 28-point
mapping swaps the original limb pairs and negates lateral coordinates when the
mirror state changes; the original captured-ball sentinel branch is retained.
The recovery report records the excerpt hash and also hashes `3D_UPD2.CPP`.

`OriginalPresentation` is a native render-state adapter. Its position, shortest-arc
facing and forward phase-wrap equations follow `3D_UPD2.CPP::realtime_coords`.
Transitions use the original rate `fstep * 2`, evaluated against absolute elapsed
logic cycles so extra renders do not advance a transition. This replaces the
original mutable per-render tween counter. Interrupted transitions freeze the
blended pose at the preceding logic boundary; capture selection still uses the
original discrete frame table. This is not a claim that the complete original
renderer or realtime_coords dispatcher has been ported unchanged.

Presentation interpolates the previous and current 20 Hz states (up to one logic
tick of display delay), without extrapolation or changes to simulation contacts.
A generation counter clears interpolation on replay, including a restart at the
same tick. The native fixture clears tm_newanim at the start of each logic tick
so snapshot consumers see one edge independently of display frequency. This
replaces the original render-time flag consumption. Sampling and drawing do not
mutate gameplay. Ownership changes occur at the displayed logic boundary; the
held/releasing ball uses the same blended pose and world transform as its player.
The ball remains subject to original animation sentinel values.

`original_presentation_contracts` checks every mirrored coordinate, missing-ball
handling, interrupted blend continuity, capture restart and wrap, shortest-arc
rotation, replay, pause, ownership boundaries, finite live poses and equal-time
samples at 30/60/144 Hz. It renders the live 60 Hz sequence through SDL software
rendering. Gameplay remains catch=59, distribution=171, reception=208 and end=226.
The existing keeper/attack/physics/input/archive tests remain enabled. Visual
improvement should be assessed with one combined playtest via the existing launcher;
no frame-time measurement or elimination of all possible hitching is claimed.

Validation: 28/28 CTests passed, including the complete native sequence and new
presentation contracts. The archive/source audit passed and SOURCE has no diff.

Full match AI, scoring/restarts and original sound remain incomplete. SOURCE is
unchanged. This batch restores presentation behavior rather than adding gameplay.

---

# Original keeper continuation - 0.13.0

## 0.13.0 playtest feedback (addressed in 0.14.0)

The user accepted the 0.13.0 sequence on 2026-10-09, with slight hitching between
animations. Code inspection found a likely contributor: the native action renderer
selects a single capture frame directly from tm_anim/tm_frm and changes clips
immediately. It does not yet run the original transition tweening in 3DENG.C:7411
or the render-time position/direction/frame interpolation in 3D_UPD2.CPP:1984.
This establishes missing rendering behavior, not a measured frame-time diagnosis.

The feedback defined this five-part restoration batch:

1. Recover the original cross-animation pose tween and its timing.
2. Preserve the original mirrored-limb mapping during transitions.
3. Restore render interpolation between the 20 Hz simulation states.
4. Handle new-animation flags, looping frames and interrupted transitions as the original code does.
5. Keep the captured ball aligned with the displayed pose through possession and release.

Validate transitions, replay/reset, frame-rate independence and unchanged gameplay
contact timing automatically. Ask for one combined playtest when the batch is ready.
The feedback was initially recorded against the approved 0.13.0 build. The next
section above records the implemented 0.14.0 restoration.


The current five-part batch continues the 0.12 attack beyond the catch:

1. Original save recovery and get-up captures.
2. Original held-ball actions, including bounce/wait and carrying upfield.
3. Original keeper punt/pass decisions and rejection of marked or distant outlets.
4. Original roll, throw, half-volley and punt animation timing, including the ball-limbo release gate.
5. Original interception planning and control animation for the outlet receiver.

The default fixture catches at tick 59, starts recovery at 89, carries for four
movement ticks, selects a throw at 171, receives at 208 and completes control at
226. Nearby fixture variants exercise half-volley distribution. These outcomes
come from the recovered routines; the fixture does not grant a catch or reception.

The generator emits **148 complete preserved bodies plus six explicit excerpts or
adapters (154 entries)**, with the kick-contact excerpt recorded separately. This
batch adds `kphold_action`, `make_pass`, `make_punt` and fourteen animation
initializers. Missing animation rates, I_KP_BUSY and CP_HEAD are recovered from
pinned objects; their bytes and offsets are recorded in `player-recovery.json`.
TOO_CLOSE_TO_GOAL is the original `prat` comparison in ACTIONS.OBJ.

The extra excerpts are the keeper punt/pass prefix of `got_ball`, the applicable
busy countdown/expiry branches of `intelligence`, and the intact animation-release
block from `process_ball`. The keeper decision adapter is called only for an
in-box keeper holding the ball, with no busy timer and action <= TURN_ACT. The
busy adapter excludes off-field/run-on/steal side effects. Action dispatch now
supports KPHOLD_ACT as its ninth case. Other unsupported actions still fail closed.

**This is still a native integration fixture, not the complete original match loop.**
It supplies the roster, opening attack, a finite run for the outlet and a retreat
run for the shooter after the catch. The original movement routines execute those
runs; they are not restored off-ball tactical AI. The outlet run has a finite busy
count to stop before requesting unrestored zonal decisions. The fixture supplies
the original control-button input for reception. It advances `af_randomize` each
tick after the fixed attack setup, following FOOTBALL.do_logic; the opening
fixture retains its prior random schedule. Win32 CRT randomness is deterministic
for this fixture, not claimed bit-identical to the DOS runtime.

Player and ball animation-limbo gates prevent decisions and free flight before
release. Rendering uses the original capture's ball position while the ball is
held or in release limbo. Physical release uses original offsets and velocity.
The visible sequence and headless tests use the same adapter.

`original_keeper_continuation` checks the full chain and two outlet variants,
physical carrying, original get-up/holding captures, a frozen ball before release,
roll/throw/half-volley selection and release, forced punts, rejection of marked or
out-of-range outlets, busy-timer expiry and stable completion. The native SDL smoke
renders the whole sequence through the final receiver's possession.

Space/R replay the complete batch; N retains the missed-save fixture; M retains
the two-player pass/control fixture. Original scoring/restarts, full tactical AI,
full match processing and audio playback remain incomplete. The preserved SOURCE
files are unchanged. Original audio still requires the missing sample archives.

---

# Previous checkpoint: original action recovery - 0.12.0

The stable launcher now opens a repeatable three-attacker/keeper fixture. The five
connected components are computer first-touch decisions, first-time passing,
shooting and shot state, goalkeeper planning/contact/possession, and the supported
original action-dispatch branches in match order. The default sequence completes
three releases, two first touches, one shot and one keeper catch. N changes only
the keeper's initial position and allows the same attack to beat him.

`restore_player.py` now emits 131 complete preserved routine bodies plus three
explicit excerpts/adapters (134 entries). New code includes `first_time_strike`,
`pass_decide`, `shoot_decide`, `shoot_ball`, `aim_shot_at_goal`, `new_shot`,
`strike_ball_off`, `go_to_save_path`, all three save-zone selectors, `save_action`,
`rebound_off_plr`, original statistics, and `player_distances`. Missing timing,
range and probability constants come from pinned objects through
`batch_constants.py`; every numeric recovery records its bytes and location.
The FOOTBALL.OBJ `.fn_init` initializer supplies the goal-post positions instead
of their zero-initialized BSS values.

The complete original `process_teams`/`do_logic` loop is **not** restored. A native
20 Hz fixture supplies the roster, starting kick, requested pass outlet, active
interceptor selection, stopping conditions, and seed 1 for repeatable replays.
Original `do_action` supplies eight supported case mappings; idle `stand_action`
contains only its pending first-touch branch. Other dispatcher actions fail closed.
The keeper-contact block is extracted intact from `ball_interact`; its caller
checks SAVE_ACT and possession. The shot-reaction counters follow `process_ball`.
Pitch/goal-mouth crossing text is a fixture observation, not original scoring,
refereeing, net collision or kickoff behavior. Catching ends this fixture before
keeper recovery/distribution. Control still uses the established collect adapter.

Compatibility adaptations are explicit: standard C++ default-argument placement,
a static qsort callback, and bounding `get_dist_pos` to eleven players per team
(the original loop crossed the team boundary and read player 23). SOURCE stays
unchanged. Audio callbacks record requests but do not play samples. The preserved
external audio archives are still missing. The numeric random stream uses the
Windows C runtime and is not claimed to match DOS bit for bit.

The combined camera renders all four actors and uses physical ball state when
unpossessed. Captured ball positions apply only to its owner. Goal-frame line
geometry is recovered from DATA.OBJ; the original net filter is still omitted.

`original_attack_contracts` checks the full chain, original shot/save statistics,
repeatability, nearby catch/miss outcomes, early/far/already-scored save rejection,
and original block/rebound behavior. `native_original_actions_smoke` renders the
full attack through keeper possession. Existing pass/control contracts remain.
Manual testing is batched: Space for the save, N for the beaten keeper, M for
control. No individual feature approval is required between these checks.

Next: complete the remaining original match update and scene coordinate paths,
then recover keeper distribution, full teams and match rules. Audio needs the
original sample archives and a native playback backend.

## Historical 0.11.0 checkpoint (superseded controls and boundaries)

Open `View-Original-Actua96.cmd` beside the project. Space restarts a standing pass;
M starts from a running animation phase. N starts an angled reception.
The receiver runs to a meeting point selected by original INTELL code.
P pauses, Up/Down rotates the camera,
V changes stadium/close-up, and Left/Right returns to the raw capture browser.
The console test runs both cases without needing manual playtesting.

## Restored code

This checkpoint adds 27 complete bodies (73 player/action routines in total):
BALLINT predict_ball; INTELL get_there_time, clear_strike, decide_on_face, go_to_path,
the EURO96 can_i_intercept variant, intercept, strike_and_control, get_dir;
ACTIONS init_run_anims, init_stand_anims, init_trot_anims, init_run_anim,
init_trot_anim, init_run_act, init_stop_act, stop_action, run_action, wait_action,
init_strike_act, new_dir, dir_movement, dir_ball, dir_new_d, dir_must_face,
dir_away, process_dir. get_there_time has one C++ compatibility edit: declaring
its loop counter before the loop preserves Watcom's old for-scope semantics.
Its calculation is unchanged. The report records this separately from source hashes.

The missing intercept_data record is reconstructed from INTELL.OBJ's 28-byte
stride and typed member accesses. The report lists offsets, evidence routines,
and object-derived animation, range and turning constants. Padding is explicit.

`tools/restore_player.py` extracts complete original bodies without modifying SOURCE:
init_stand_anim, init_kick_anim, init_trap_anim, init_chest_anim, init_dnhead_anim,
init_kick_act, kick_action, fire_ball_off, init_stand_act, init_kphold_act,
init_get_up, process_anims, control_action, get_closest_pred, init_control_act,
init_wait_act, get_mcball_coords, control_interact, standard_fstep, choose_pass,
pass_ball, new_interceptor, fstep_factor, af_randomize, rand_range.

The kick-contact branch of BALLINT.CPP ball_interact is also extracted intact as
update_kick_contact, with separate line/hash provenance. This excerpt is not counted
as a complete restored routine. ACTIONS.OBJ save_offs remains the contact table.
DATA.H provides contact fractions and IDs. `tools/action_constants.py` records
object locations and bytes for the missing animation speeds and displacement.
The renderer now also compiles original addobjyf for mirrored animation poses.

The normal pass uses original init_team's animation-speed multiplier from flair and
rate. Original process_anims advances at the game's 20 Hz logic rate. The selected
sequence preserves ball processing before player processing, and animation advance
before contact checks before action execution. The ball's release speed, spin reset,
recipient registration, possession and pass statistics all use original routines.
The trap must satisfy the original animation phase, horizontal and height checks.

Original render3d's ball-point transform is integrated into the native draw adapter:
when the capture contains a ball point, that position drives its displayed geometry.
Logical ball state remains separate, as in the original renderer. Original
get_mcball_coords and hold_ball perform the post-trap recovery into dribbling position.

## Native integration boundaries

OriginalActions is a two-player fixture, not new gameplay AI. It supplies positions,
a pass command, an initial running pose for the second scenario, and the original
receiver control-button input (uf2, no directional input). predict_ball generates all
50 prediction samples; go_to_path evaluates travel, turning, animation time and contact
offsets, then intercept starts the selected action. The fixture no longer searches for
a nearby frame or assigns a reception time. Prediction refreshes each free-ball tick,
and selection executes its first action in the same tick, following user_play's order.
Original run_action, wait_action, stop_action and process_dir execute the approach.
The fixture holds facing during possession and ends after stationary trap recovery;
the full free-dribbling input and intention lifecycle are not connected.
The supported trap branch calls original collect_ball; full ball_interact/control_ball
and the full match loop are not claimed as restored. No automatic opponent AI is added.

init_anim dispatch currently supports stand, run, trot, straight kick/pass/chip/backheel, trap,
chest and downward-head initializers. Other actions throw explicitly. Punt/shot/cross
tracking is not connected. Computer first_time_strike, user first-time passing,
first-time strike execution, zone AI, celebrations and free-running input throw
explicitly if reached. They are not substitute implementations. Restored but
unexercised branches (set pieces, keepers,
chest/head control) are not exposed as working gameplay. Native match_info contains
only the GameType field used by pass_ball; it does not claim the missing full layout.
The fixture uses arcade passing, and original randomization is compiled against the
native C runtime, so simulation-mode random sequences are not DOS-bit-identical.

Motion mappings are validated against original loader instruction bytes and DATA.OBJ
frame counts. The loader manifest now maps 92 IDs; unmapped archive clips retain their
record labels. Mirrored IDs share the original unmirrored capture. The original
display transform is used; inter-animation pose blending is still pending.

## Facing correction in 0.10.1

The native draw adapter incorrectly copied render3d's negative cosine without the
preceding realtime_coords negation. Direct gameplay-to-object conversion must use
positive tm_xdis and tm_ydis. Both the mesh and captured ball now use the same object
transform. Regression checks compare rendered capture coordinates with the original
get_mcball_coords routine in eight facing directions and check mirrored forward motion.

## Validation

The original_action_contracts suite checks both kicking feet, actual receiver travel,
the selected trap plan, original waiting, four angled passes, unreachable/out-of-bounds
trajectories, cancellation after an intervening touch, prediction versus free flight,
preservation of live ball state, one release and one
reception, completion of recovery, rejection of premature/far/high contacts, cancelling
a kick when possession is lost, and rotation of the captured ball into world space.
The SDL smoke test runs and renders a complete pass/trap without user input. Existing
physics, practice, controller, AI, graphics and asset regression suites remain enabled.
The original source audit is mandatory before the stable launcher is updated.

Next: connect computer first-touch decisions and the original action dispatcher into
process_teams/do_logic, then connect the full scene coordinate update. Sound still
requires the original external sample archives and a native backend.


