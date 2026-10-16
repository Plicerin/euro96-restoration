# Knockout matches, version 0.23.0

Version 0.24.0 connects manual shootout controls; see [penalty controls](PENALTY-CONTROL.md).
The CPU-only descriptions below document the original 0.23.0 checkpoint.

The next five-feature batch connects extended matches and their presentation:

1. Knockout format: a level score after 90 minutes starts extra time.
2. Two original 15-minute extra periods, including the 105-minute change of ends.
3. Optional golden goal: the first extra-time goal wins after its celebration/replay.
4. Original penalty shootout ordering, early-clinch rules and paired sudden death.
5. Penalties-only setup, per-kick history, separate penalty totals and winner/results UI.

## Controls and testing

Open the existing View-Original-Actua96.cmd launcher when ready. In match setup,
select the new **Format** row: Regulation, Knockout, Golden goal or Penalties only.
Keyboard arrows or gamepad D-pad change it; Enter/South watches CPU teams and
Y/North starts a human-controlled match. F2/L3 returns to setup during a match.
Changing the selected format takes effect when starting the next match.

Penalties only skips directly to a shootout. Both teams' penalty takers and
keepers use the original CPU routines, including in an otherwise human-controlled
match. Manual penalty aiming/keeper control is not connected. P/Start pauses;
R (or R3 while paused) restarts. O marks a scored kick and X marks a miss. Long
sudden-death histories show the latest twelve kicks per side.

Match goals remain visible separately from shootout goals. A drawn 2-2 match
won 4-3 on penalties stays 2-2 on the match score, with a separate penalty result.
Regulation remains the default and can still finish as a draw. Match length is
the existing original accelerated 90-minute duration; extra time adds one third
more active play, plus intervals and stoppages. There is no competition bracket
or tournament persistence in this batch.

CLI examples (same data/executable layout as the existing launcher):

- `--cpu-teams --match-format knockout`
- `--cpu-teams --match-format golden`
- `--cpu-teams --match-format penalties`

## Preserved source and native boundaries

SOURCE remains unchanged. `tools/knockout_recovery.py` extends build-time recovery;
`native/original_knockout.cpp` provides fixed-step scheduling and Windows state/UI.

FOOTBALL `watch_match_time` supplies the exact 105/120-minute predicates and
`nothing_happening` whistle gate. Existing 45/90-minute predicates and original
`swap_teams` are reused. The native interval scheduler extends its existing
45-minute end swap to the 90/105 transitions. Names, kits, control, tactics,
fatigue, substitutions and disciplinary state retain stable team identities.
BALL `go_right_goal` supplies the intact golden-goal flag block, identical to
the left-goal branch. Native scheduling ends the match after goal presentation.

RULES `do_penalties`, `pen_to_team_a` and `pen_to_team_b` retain the original
five-kick counts, mathematical clinch checks, alternation, paired sudden death
and descending active outfield taker rotation. Original `init_match` initializers
set five attempts per side and taker cursors 11/22. Standalone shootouts begin
with an odd end orientation, as a shootout after the fourth match period does,
so the preserved home/away attempt counters align with the original goal totals.
The source's keeper-excluding taker cycle and lack of equal-player-count
normalization are preserved historical behavior.

ACTIONS `user_pen_kick` runs its original computer branch: oscillating goal-line
aim, stat-based CPU timing, low/high selection and power/height values.
`init_runup_act`, `user_runup`, `taker_nkick` and RULES `ready_set_kick` connect
that choice to the original run-up, shot animation, contact and trajectory.
The native code supplies the setup/reset state from original `computer_play`
and omits only DOS `draw_tracker`. I_RUNUP=15 and SPACT_NEWK=9 are checked against
immediate operands in the pinned ACTIONS.OBJ at 0x1b35/0x1b9e. Original keeper
stance, prediction, dive choice and save contact remain active.

Native shootout scheduling permits only the kicker and defending keeper to
interact, prevents follow-up shots, and resolves an attempt on a goal, held
save, out ball, settled ball or ten-second resolution limit. It waits three seconds
to display an outcome, then calls original sequencing. The DOS replay/menu
scheduler and its timer are not reused. Match clock, substitutions and manual
replays are held during shootouts; the live match score/statistics are retained
outside the original counters reused for penalty scoring. Full-match replays
continue unchanged. CPU aiming has a fail-closed timeout rather than silently
inventing a kick if an unsupported state is reached.

## Verification

`knockout_tests` exercises 90/105/120 transitions, end identity, wins in regulation
and extra time, golden-goal completion, early clinches, sudden-death fairness,
dismissed taker exclusion and separate match/penalty scores and statistics.
Twelve seeded complete shootouts must produce both scores and misses and a
winner; four extended CPU matches forced level near 90 minutes must complete.
The existing SDL setup smoke now selects all formats by keyboard, starts a
shootout, then uses a virtual gamepad to return to a human regulation match,
checking that no shootout state leaks into it.

The release checkpoint records full-suite results and visual checks. Original
audio remains blocked by the absent DIGI/PITCH/PITCH.MAD assets.
