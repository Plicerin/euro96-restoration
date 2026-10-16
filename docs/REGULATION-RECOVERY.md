# Regulation matches, version 0.20.0

This records the 0.20.0 checkpoint. [National-team recovery](NATIONAL-RECOVERY.md)
[match depth](DEPTH-RECOVERY.md) and [knockout recovery](KNOCKOUT-RECOVERY.md) supersede the uniform-roster, invisible
official, missing-card/fatigue/substitution limitations described below.

The original 11-v-11 mode now starts at kickoff and ends at a results screen.
Open View-Original-Actua96.cmd and press B / RB to watch CPU teams, or Y / Start
to play blue. B / RB switches control without resetting the match. P / Start
pauses; R resets; Q / Back returns to replays. R3 resets while paused, or starts
a new match directly from the results screen.

This batch connects five features:

1. Original accelerated match clock, with the original 5/10/20/40/90-minute choices.
2. Half-time, swapping ends and the second-half kickoff.
3. Full-time and a persistent score/pass/shot results panel.
4. Original tackle knockdowns, referee foul/advantage decisions and free kicks.
5. Original offside recognition, interference checks and indirect restarts.

Penalties are included as a necessary foul outcome inside the defending box.
Close direct free kicks use the original defensive-wall selection/positions.
Keeper penalty stance, fall/get-up and tackle-riding animations use original
motion captures and recovered action timing.

The default is five minutes of active play across both halves. Restart setup,
the five-second half-time interval, pause and loss of focus add wall time.
The source's whistle gate lets a pending shot/crucial play resolve, so the clock
can pass 45:00 or 90:00. Extra time and penalty shootouts are not enabled: a
regulation draw is a final result. L cycles the duration for the next match;
press R to start it. CLI: `--cpu-teams --match-minutes 10` (or `--original-teams`).
For repeatable screenshots, `--capture PATH --match-ticks N` advances a CPU
match by N original 20 Hz logic ticks, stopping play naturally at full-time.

## Original routines and native boundaries

SOURCE remains unchanged. tools/rules_recovery.py extends build-time recovery.
The original RULES match_clock/add_player_time are used, including the
90/(time_factor*REAL_SPEED) increment. regulation_period_end extracts the
regulation half/full-time predicates from FOOTBALL.watch_match_time and calls
its original nothing_happening gate. DOS menus, sound calls, tunnel walk-off,
competition extra-time/shootout dispatch and results presentation are outside
those excerpts. Injury time remains zero; crucial-play completion still applies.

RULES.swap_teams retains the original roster exchange, player renumbering and
half increment. Native input, kit/number materials and HUD counters follow stable
blue/red identities across that exchange. Both teams currently use the same
decoded formation and uniform player attributes, as in the prior preview.
Native placement calls original centre targets at kickoff after each interval.

INTELL.player_ints/facing_opp connect tackle/steal body contact, fall and jump
recovery. This does not yet connect the separate general player_tussles system.
RULES.init_foul retains visibility/strictness and advantage; punish_foul retains
direct/indirect/penalty classification and taker choice. A native rule event
replaces its DOS init_match_mode callback. Cards, sending off, speech, old user
registration and menus are excluded explicitly. The original delayed-advantage
resolution and ball-release countdown are extracted from FOOTBALL.

INTELL.offside_rule and RULES.init_offside use the original defensive line,
release window, linesman accuracy and interference distance. These are the
preserved game's rules, not a rewrite against modern football regulations.
Original follow_play and lman_follow update invisible official positions for
visibility decisions. Their animation callback records the animation ID only;
officials are not drawn. Native starting positions initialize those observers.

init_fkick/init_dfkick/init_penalty, wall selection, and all three original
set-piece positioning helpers are connected. Existing decide_set_kick CPU
branches choose the release; kick contacts and ball velocities remain original.
Penalty release currently uses that original automatic branch, not the separate
EURO user/CPU set-piece power-bar UI. Native transitions place players at the
original targets and automatically release all restarts, even in human mode.
The original last-touch value must survive the native reset before foul setup.

Compatibility fixes: player_ints uses z for the height difference (the source
uses y), skips itself and disabled slots; referee visibility guards zero
distance; wall projection guards missing wall members/zero denominator; tactical
offsets are initialized for special centre positions. Earlier bounds/scope fixes
remain. New animation constants are checked against the pinned ACTIONS.OBJ;
the player-contact radius is the prat relocation in INTELL.OBJ at 0xa3ec.

## Verification

regulation_tests checks all five clock speeds, stopped kickoff clocks,
half-time freeze, roster/stat identity, second-half human selection and scoring,
full-time freeze, rematch, both sides' indirect/direct/penalty releases, defensive
walls, offside calls/exemptions, tackle knockdown/recovery and the pending-shot
whistle gate. Twelve seeded CPU matches run to full-time, checking finite ball
and player coordinates and bounded restart duration. SDL CPU smoke additionally
renders half-time and results, swaps ends and starts a controller rematch.

The earlier continuous-play fixtures remain available to regression tests via
start_team_play; user-facing team entry points use start_match. Original action,
graphics, keeper and goal replays remain separate fixtures.

Still pending: visible referees, cards, substitutions, fatigue, competition
formats, general body tussles, walk-up presentation and original sound playback.
The source snapshot lacks the required original sound archive.

Release verification: all 39 CTest suites passed in the final clean build
(87.06 seconds). Twelve full CPU matches produced 12 goals, 65 foul awards and
11 offsides. The tests sample every player's interpolated pose each tick. This
caught two previously omitted keeper captures: MC_AFOOTC/MC_ABODYB now map to
archive records 135/136 using checked original renderer loader bytes and frame
counts. The source audit validated all four archives; SOURCE has no changes.

The results screenshot is from a complete CPU match, not a forced result.

![Completed CPU match](original-regulation-results.bmp)

Windows rejected compiler writes in the project build directory, so the final
build used a fresh temporary directory. tools/publish_playtest.ps1 accepts an
optional -BuildDirectory for that case. The stable launcher still uses verified
immutable snapshots beside the project. This task's work/regulation-build-path.txt
records the isolated build location. Its final test log is in work/structure-final-test-evidence.log.
