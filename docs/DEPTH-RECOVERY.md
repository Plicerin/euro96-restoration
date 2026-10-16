# Match depth, version 0.22.0

Five features are connected to the original 11-v-11 match: discipline, visible
match officials, fatigue, substitutions, and goal celebrations with replays.
The preserved SOURCE tree remains unchanged.

## Playing this batch

Use the same View-Original-Actua96.cmd launcher when ready for a new match.
Publishing this update does not restart a match already running.

- F3 opens the squad panel and pauses play. Up/Down chooses team, outgoing
  player or incoming player; Left/Right changes that choice; Enter queues it.
  F3 or Escape closes the panel and restores its previous pause state.
- On a gamepad, pause with Start, then North (Y on Xbox) opens the squad.
  D-pad selects, South confirms, Back closes. Resume with Start if needed.
- Changes happen at the next restart. Each side has its original four match
  substitutes (shirts 12-15), with a three-change limit. Dismissed players
  cannot be replaced and used substitutes cannot return.
- The squad shows current pace as a percentage of that player's starting pace,
  bookings, dismissals, bench positions and availability.
- F4 starts a replay of the recorded recent play. Gamepad: pause, then West
  (X on Xbox). Space/South skips, P/Start pauses, V changes camera.
  Closing replay restores the match's prior pause state.
- Goals use original celebrations and the scorer camera, followed automatically
  by a replay and the conceding team's kickoff. Replays freeze the live match.

CPU teams make fatigue-based changes after 60, 70 and 80 match minutes, at the
next restart. This native management policy picks the most fatigued active
outfielder, prefers a matching bench position and never uses the reserve
keeper as an outfielder. Human-managed teams wait for a queued substitution.

## Provenance and adapter boundaries

`tools/depth_recovery.py` extends the existing build-time source recovery.
The generated player report records source routines, hashes and object evidence.

**Cards:** original RULES `send_off_plr`, `inc_yellow`, `inc_red`, `big_yeller`
and `ger_em_off` are connected through `punish_foul`. Their thresholds, second
yellow behavior, counters and set-piece taker updates remain original. A native
notice replaces the DOS menu/speech; removal is immediate instead of a tunnel
walk. The historical keeper immunity and five-dismissal-per-team cap remain.
No card-in-hand gesture or speech is claimed.

**Officials:** original referee/linesman meshes and motion captures are rendered.
Original `follow_play`, `lman_follow`, stand/run/jog/directional-trot animation
and facing routines move them. Native scheduling also ticks them during dead
balls and faces the referee toward the booked player. Zero-distance guards
prevent undefined direction calculations. They use the same interpolation as
players and are included in replay snapshots.

**Fatigue:** the intact mathematical tail of FOOTBALL `player_stamina` computes
pace from original stamina, time on the pitch and starting pace. The native
roster lookup supplies the unfatigued rating for the current shirt. Evaluation
runs each logic tick; original playing-time increments remain minute-based.
Substitutes receive their own original ratings and fresh time/action state.

**Substitutions:** the native squad panel and swap operation follow preserved
bench selection and `process_subs`/`init_player_stats` data assignments. The
preserved `SubstituePlayers` frontend functions are empty, so these are adapter
code, not a recovered complete substitution frontend or CPU manager. Team
statistics survive outgoing players, identity follows half-time end swaps,
and original number textures now include shirts 12-15 for both teams.

**Celebrations:** original ACTIONS celebration selection, run/group/taunt,
stationary celebrations and shame actions are connected, with animation speeds
read from pinned ACTIONS.OBJ. Original CPU celebration selection is also used
for the human scorer; the legacy user-input spin loop is excluded. The normal
220-tick goal interval is verified against BALL.OBJ. Original camera 15 follows
the scorer after the initial goal interval. Textured stadium near-plane clipping
now uses original `addobjnc` and full `addpolyc`; a bounded native UV pool
replaces the DOS mutable texture storage.

**Replays:** original 15-second/20-Hz sizing informs a native snapshot buffer.
Manual playback uses available history; automatic goal playback uses up to its
last eight seconds. Dead intervals and celebrations are excluded. A separate
engine copy supplies recorded players, officials, ball and end identity to the
renderer, without advancing the live clock or decisions. Snapshot ownership,
controls and automatic dispatch are native; full DOS replay controls, scrubbing
and replay file persistence are not restored.

## Verification

`depth_tests` checks original yellow/second-yellow/direct-red thresholds, the
live foul path, keeper exclusion, dismissal persistence through restart/render,
three visible officials, fatigue, substitution identity/fresh state/statistics,
bench eligibility, the three-change cap, and half-time roster identity. It
compares live player memory, clock, tick and ball state throughout replay.
Four goal fixtures cover both ends and own goals, actual celebration/shame actions,
scorer camera, automatic replay and kickoff without double scoring. Six complete
CPU matches exercise cards, substitutions, goals and all camera modes.

`native_match_depth_smoke` drives the real SDL event loop with keyboard and a
virtual gamepad, verifying queue/apply, squad pause restoration, replay entry
and skip. Scene tests cover textured near-plane clipping, UV/page/transparency
preservation and substitute number bindings. Full-suite results and release
verification are recorded in the restoration checkpoint after the final run.

Original audio still needs the missing DIGI/PITCH/PITCH.MAD game data and a
replacement audio backend. Tunnel walk-offs, competition persistence and
injuries remain outside this batch.
