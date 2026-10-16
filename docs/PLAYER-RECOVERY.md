Current checkpoint: 0.20.0 connects original regulation clock/halves, fouls and offside. See [REGULATION-RECOVERY.md](REGULATION-RECOVERY.md) for current scope. The generated inventory has 277 selected entries and 210 additional scalar fields.

Historical checkpoints follow.
Current checkpoint: 0.19.0 connects both original keepers, live saves/distribution, scores and automatic restarts to 11-v-11 play. The generated inventory has 241 selected routine entries and 185 additional scalar fields; its excerpt boundaries distinguish complete bodies from adapters. See [MATCH-RECOVERY.md](MATCH-RECOVERY.md) for current scope, [TEAM-RECOVERY.md](TEAM-RECOVERY.md) for outfield provenance and [CONTROL-RECOVERY.md](CONTROL-RECOVERY.md) for the retained two-player fixture.
The 0.14.1 non-upward-header control branch remains intact: original capture speed and ball limbo through chest control; upward-header control fails closed.
See [ACTION-RECOVERY.md](ACTION-RECOVERY.md) for the active attack/keeper/outlet chain and scope.
The following records the earlier movement and possession checkpoint.

# Player practice restoration, version 0.9.0

This checkpoint provides a two-versus-two opponent challenge and a two-player practice drill: control either player, move with the ball,
pass to the teammate, receive ground or lofted passes, and take an aimed shot. It is not an eleven-a-side match.

## Recovered original code

`tools/restore_player.py` extracts 21 complete function bodies without changing their contents:

- ACTIONS.CPP: full_spd, actual_spd, angle_to_xy, go_toward_target, go_forward, rotate_offs.
- BALLINT.CPP: hold_ball, init_specials, collect_ball, holder_lose_ball, ball_at_contact.
- INTELL.CPP: reset_ideas, reset_all_ideas, reset_leaves, pass_ahead.
- RULES.CPP: inc_work, inc_pass, inc_pwon, inc_twon, add_comp_pass, inc_shot.

Function scope is adapted to PlayerEngine, which owns its state and inherits the recovered ball engine.
Thirty-three additional scalar declarations use the earlier FOOTBALL.OBJ inventory. A few are conservative
extras because the identifier scan also sees comments. teams and burst_timer have explicit bounded storage.

ANDYDEFS.H contains the matching player layout when packed to one-byte alignment: **203 bytes**.
The build asserts size and important offsets: tm_x=2, tm_rate=0x46, tm_poss=0x90,
go_txdis=0xa0, int_move=0xbf. BALLINT.OBJ addresses those offsets and indexes teams by 0xcb.
Using normal modern alignment or the older EXTERNS.H structure would be incorrect.

The generator pins ACTIONS.OBJ, BALLINT.OBJ, INTELL.OBJ, and ANDYDEFS.H by SHA-256.
`player-recovery.json` records source/body hashes and the reconstructed constants. Constants include:

| Definition | Evidence |
| --- | --- |
| I_CELEB=16, celebration speed approximately 1.69 | ACTIONS.OBJ full_spd, _TEXT+0x1d and immediate at +0x25 |
| I_INTERCEPT=1, I_OFF=7, I_RUN_ON=8 | INTELL.OBJ reset_ideas comparisons |
| Maximum turn=(rate/8+14)*pi/180 | ACTIONS.OBJ go_forward, CONST+0x252 through +0x262 |
| Stand/run/tackle/save/throw/keeper-hold/control = 0/1/3/10/11/12/17 | BALLINT.OBJ hold_ball and collect_ball comparisons |
| PLAYER_HEIGHT=25, hand/feet distances=10, possession callback delay=4 | BALLINT.OBJ hold_ball constants and comparisons |
| Release counter=40 | BALLINT.OBJ holder_lose_ball _TEXT+0x2d78 |
| Low pass distance=18*prat | INTELL.OBJ pass_ahead CONST+0x699 |
| Pass friction uses double 0.986 / 0.965 | INTELL.OBJ CONST+0x6ad / +0x6bd |
| Basic running animation identifier=72 | DATA.H MC_RUN |

The object files are used as evidence and optional test inputs, never linked into the Windows executable.
The original SOURCE directory remains unchanged.

## Explicit native adapters

The following are new practice behavior, not claims that the corresponding complete original systems run:

- Keyboard/stick steering and turn limiting, pitch boundaries, selection, a simple visual gait phase,
  player markers, and pass trails. The original go_forward and actual_spd handle movement displacement.
  Analog stick magnitude scales that displacement after the original routine; keyboard and AI use
  full strength. SDL gamepad input adds a radial deadzone, right-stick shot aiming, hot-plugging,
  active-controller selection, controller-specific face-button prompts, and disconnect pause.
  Reset requires pause plus a right-stick click to avoid clearing scores during normal play.
- Assisted reception: a 16-unit foot-reach radius (increased from the original 8 for this practice
  adapter) checks relative ball/player motion and the low-height window at height 12. It handles
  moving receivers and balls descending during a tick without attracting distant or high balls.
  Original control difficulty, failed traps, deflections, tackling, and animation contact remain
  unfinished. Successful reception calls original collect_ball and hold_ball.
- Passing always targets the other practice player and switches control to the receiver. Original
  pass_ahead predicts a moving target; selected pass_ball equations set release velocity. Random
  inaccuracy, crossings, special kicks, and set pieces are omitted. Ground and loft are explicit choices.
  After automatic selection, held passer input is guarded during flight. Collection restores the
  latest held direction and analog strength for the next movement tick, without requiring neutral
  input. Immediate return passes reapply flight protection. Releasing and reapplying input still
  permits manual movement before the catch.
  The loft adapter now solves horizontal drag and vertical gravity for a whole number of ticks,
  shortening flight time to respect the vertical-speed cap. Long lobs previously bounced too early
  and passed above the receiver on the rebound. These launch corrections are native practice behavior;
  the restored trajectory functions are unchanged and the ball does not home after release.
- Shooting is an explicit diagnostic preset: speed 20 and vertical velocity 6, along the selected
  player's facing direction. Original aim_shot_at_goal, shot selection and kick animation are not restored.
- has_ball and reselect become counted events. init_run_anims records MC_RUN without displaying
  original motion capture. Unsupported motion-capture and keeper-hold callbacks throw a clear error.
- Native off-ball support in opponent mode evaluates nearby positions using defender clearance,
  passing-lane clearance, distance to the carrier, travel cost, and route safety. It reevaluates
  every six ticks with target hysteresis; steering avoids nearby markers and pitch boundaries.
  A teammate already in useful space can hold position. The selected player is never moved by this
  support planner, and an unselected holder is not automatically dribbled.
- Without blue possession, the unselected teammate moves toward the red passing lane at 65%
  of the carrier-to-outlet segment. With no active red outlet, he contains from the left goal
  side. On a loose ball he uses a short four-tick velocity lead. Targets are pitch-clamped;
  normal movement, turning and swept contact still apply. He does not automatically tackle
  a holder. Collection uses the existing control transfer; selecting him cancels AI steering.
  Own-team assisted receiving takes priority. Pause, restarts and solo mode retain their behavior.
- Passes to moving support players retain the original lead calculation. The receiver runs toward
  that fixed pass destination for up to 100 ticks, then returns fully to manual control. New direction
  input or manual selection cancels the receiving run; held passer input remains guarded only until collection.
  Collection, interception, reset, and mode changes clear this assistance. Ball flight is unchanged.
  This is new practice AI; the preserved original full-match support/interception system is not restored.
- Optional native opponent AI uses players 13 and 14 from the original opposing-team index range.
  One pressures the carrier while the other covers the passing lane; both pursue free balls.
  They use original movement at rate 64 versus the user's rate 96. A red holder carries toward a safe waypoint until close enough to it or blocked by nearby
  pressure. Waypoint changes have a 12-tick minimum interval. Defensive roles require a
  32-unit distance advantage and a 12-tick commitment before switching; support offsets retain
  their side throughout red possession instead of flipping at midfield.
- Native red passing decisions run every six ticks after at least 16 possession ticks, with a
  48-tick team cooldown after a kick. They require a 100-420-unit teammate spacing, an in-bounds
  led destination, a clear lane and receiver space. Pressure, forward progress toward the left
  goal, or a prolonged carry can trigger a ground pass. The original pass_ahead routine supplies
  the lead; the same ground-pass speed/drag and 16-unit swept reception as blue apply.
  The receiver runs to that fixed destination, while the passer offers support. Collection,
  interception, reset and restarts clear the receiving run. Red passes do not change blue
  selection/input. Original pass statistics and separate red attempts/completions are recorded.
  Red shooting is also a native adapter: within 360 units of the left goal and a suitable
  angle, with possession settled and no blue player blocking the lane, a red carrier shoots
  at the side away from the keeper. Against a centred keeper he carries inside 150 units first;
  an out-of-position keeper permits an earlier chance. A 40-tick cooldown guards repeat shots. Unpressured carriers
  make progress toward goal. Original full-team passing and shooting decisions remain unfinished.
- Tackles are new drill logic. AI retains its 26-unit reach and facing dot >= 0.25, with
  eight ticks of visible wind-up. User tackles reach 40 units in the forward half-plane,
  stay active for eight ticks, and have a 12-tick repeat interval. Holding X retries while
  defending; it never auto-shoots after a regain. A pale blue ring shows the user's reach.
  Interceptions clear the pass-input release guard, so held movement works immediately on defence.
  New possession gets a 12-tick protection window; release still permits interception.
  Possession transfer uses original holder_lose_ball/collect_ball, including tackles-won statistics.
  These are not restored animated tackles, physical body collisions, or foul/referee decisions.
- Both teams use the approved 16-unit swept reception and height window. Earliest contact wins.
  Reds never become user-controlled. Blue/red wins count possession changes between teams.
  O toggles the drill with a reset; reset retains the chosen mode. Solo mode retains all 73 checks.
- Native goal detection in opponent mode checks the first whole-ball boundary crossing, the
  crossing-time height, an 84-unit goal mouth, and the recovered 34-unit goal height. Player
  contact before that boundary takes precedence. Right-end goals count for blue; left for red,
  regardless of last touch. Post/bar overlaps are misses; goal-frame rebound physics is pending.
- Goals freeze play for 40 ticks and restart at centre for the conceding team. Other out balls
  freeze for 20 ticks and restart near the exit for the opposite last-touch team. Scores and
  statistics survive; fresh restart possession is protected for 30 ticks. R and mode changes
  clear the score. These are small-sided drill rules, not the original referee/kickoff system.
- Native keepers are separate from the recovered outfield state: fixed depth 22 units from
  each goal line, four-tick reaction delay to incoming shots, lateral speed capped at 2.2
  units/tick, lateral bounds 364-436, contact radius
  14 and height limit 28. Their swept contact competes with outfield reception and whole-ball
  boundary timing. They cannot save a ball already behind their depth line.
  Central manageable contacts (speed <=18, lateral offset <10, height <22) become a protected
  20-tick catch; other valid contacts parry outward at speed 6-12 with lateral/vertical rebound.
  An eight-tick contact cooldown prevents repeated saves of the same rebound. Release chooses
  an outfield outlet using clearance and distance, with original pass lead and ground physics.
  Keeper catches use explicit native state rather than unsupported original hand/animation callbacks.
  Keepers stay automatic; user selection remains on blue outfield players. Save statistics
  survive restarts; new game clears them. Solo passing practice has no keepers.
- Native small-sided matches last 3600 active ticks (three minutes). Restarts and
  pauses do not consume the clock; keeper holds do. The final active tick resolves
  fully, including a goal, before entering the frozen full-time state. Score, shots
  and saves remain visible. R, Start/Options, or the result button resets the match.
  Solo practice is untimed. This timer is a native adapter, not recovered original timing.
- No full referee rules, official throw-ins/corners/goal kicks,
  original player models, or audio. Solo mode still pauses outside the pitch until Reset.

The snapshots are not perfectly consistent: the compiled holder_lose_ball/inc_work include aggregate
statistics calls absent from the loose source. This build preserves the source bodies; it does not
claim whole-engine binary equivalence.

## Verification

- Twenty-three CTest suites pass, including player interaction checks, 65 opponent checks, 31 support checks, 32 goal/restart checks, and nine restoration-tool tests.
- Movement, turn-speed bounds, held-ball following, pass/return pass, lob reception, aimed shots,
  possession gating, pause/reset, selection, pitch bounds, swept reception, high-ball rejection,
  independent sessions, and unsupported animation rejection have direct checks.
- Regression checks cover ground/lob distances 64-800, held input in eight movement directions,
  fresh receiver input, foot-reach misses, relative movement, and descending/high-ball contact windows.
- The native SDL smoke scenario injects pass, return-pass, and shoot events and verifies outcomes while rendering.
- Opponent checks include pressure/interception, first contact, high-ball rejection, a chase and
  tackle against a moving holder, wind-up/cancellation, range/facing/recovery, pause/reset, and toggling.
- Support checks cover marked players opening lanes, carrier/manual control, ground and lofted
  passes to moving receivers, held/fresh inputs, spacing at the touchline, and live opponent openings.
- Tackle regressions cover an early press connecting during approach, 16 live-AI chases with
  eight keyboard directions and 150/300 ms steering reactions, delayed pursuit from both players,
  no accidental held-button shot, no distant steal, pause, and movement after an intercepted pass.
- Native opponent smoke verifies pressure turnover, X tackle, O toggle in both directions, and R reset.
- Goal checks cover both ends and own goals, partial crossings, wide/high shots, diagonal exits,
  fractional-tick height, interception before the line, pause/action guards, kickoff ownership,
  statistics, out restarts, reset, and possession recovery after a red kickoff. The SDL opponent
  smoke now injects a scoring shot and checks the celebration and subsequent kickoff.
- 58 virtual-controller checks exercise real SDL virtual-device axes/buttons, analog speed,
  D-pad release, passing/lobbing, continuous held movement through reception, right-stick shots,
  held tackles against moving AI, pause/reset,
  mode changes, Xbox/PlayStation labels, active-device selection, disconnection and reconnection.
  A separate native window smoke test drives movement, reception, aiming, pause/reset, mode
  switching and disconnect pause through SDL's event/polling path. gamepad.png was inspected.
- Receiver-flow checks cover both modes, both pass types, eight directions, partial/full analog
  strength, first-tick movement matching fresh manual input, one-touch returns, neutral input and
  direction changes during flight. The test failed on the previous build before the fix.
- A native Windows capture was visually inspected: opponents.png (previous solo capture: players.png).
- compare_original_player.py executes preserved ACTIONS, MATHS and RULES object code in Unicorn,
  with FOOTBALL object data, to compare 25 movement scenarios / 500 steps. It covers five player
  speeds with free running, possession, sidestepping, interception, and burst. Positions, displacement,
  and the work counter agree within 0.002; the measured maximum difference was about 0.0000005.
- The binary comparison covers the restored movement routines, not assisted reception or practice kick adapters.
- Seventeen opponent-passing checks cover forward and pressure-outlet decisions, moving
  catches, blue interception, blocked lanes, timing, user control, statistics and reset/solo modes.
  Live AI tests verify exchanges while the blue teammate defends; the 16 human-rate defensive
  scenarios recover possession by a chase or teammate interception. Native SDL smoke also renders
  a red pass/reception and checks that only the unselected blue player moves automatically.
- Defensive teammate regressions cover both player selections, lane closure, normal loose-ball
  collection, no automatic steal of held possession, immediate manual override, pause/restarts
  and solo preservation. The original static-defender regression failed before the fix.
- Keeper tests cover both ends, protected catches, automatic outlet reception, live rebounds,
  earliest-contact ordering, high/wide rejection, playable recovery, pause, reset and save statistics.
  Attack tests cover deliberate red goals and blue kickoffs, blocked/wide/long-range decisions,
  pause/restart guards and unscripted progress into shooting range. Native SDL smoke also renders
  catch/release and a red shot/goal/kickoff. An unattended two-minute default scenario produces
  five red shots, four goals and one blue save; it is a regression scenario, not a difficulty benchmark. Dribble-only motion and tackle fixtures disable red
  shooting to isolate those earlier contracts; attacking tests use the full decision flow.
- The previous ball-physics precision caveat remains. Physical controller testing is still pending.

## Next work

Restore the original animation contact data and the complete receive/kick action chain. Then connect
full team AI, original keeper actions and official rules to restore the original full match.

## Timed-match verification (0.8.0)

A separate test covers the 20 Hz countdown, restart and pause exclusions, keeper
possession, final-tick goals at both ends and a final-tick equalizer, result-state
input guards, mode changes and rematches. Six full games use idle and deterministic
movement/pass/shoot/tackle/switch inputs through the public controls. Across 24,000
simulation ticks (18 minutes of active match time), all games reached full time;
none exceeded keeper-hold or restart deadlines. These tests check stability and
progress, not human-perceived difficulty or controller feel. The SDL native-window
scenario verifies the rendered full-time state and keyboard/mouse rematches; virtual SDL
controller events verify Start rematches.

## Original contact positioning (0.9.0)

The missing ball.dta source is still absent, but its save_offs table survives as
1,584 initialized bytes in ACTIONS.OBJ: 132 triples. The pinned object is checked,
the data bounds/initialization are verified, and the exact floats are compiled into
the native engine. Original rotate_offs (ACTIONS.CPP) and ball_at_contact
(BALLINT.CPP) bodies are now extracted unchanged. They rotate the original contact
position with player facing and add player position/height. The throwing placeholder
for ball_at_contact is removed. Tests verify known offsets, axis signs, rotation,
height and finite results for all 132 animation entries.

This restores a dependency of the real action system. The practice controller still
releases passes/shots directly; it does not yet execute the complete original animated
kick/receive chain. get_mcball_coords and keeper animation callbacks remain gated.
