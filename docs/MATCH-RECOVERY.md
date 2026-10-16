# Live match recovery, version 0.19.0

Open View-Original-Actua96.cmd and press **B / controller RB** to watch both CPU
teams, or **Y / Start** to play blue. B/RB toggles takeover, P/Start pauses,
R resets the field and score, and Q/Back returns to replays. Controller reset
is R3 while paused. All restarts are automatic, including when controlling blue.

This batch connects five features:

1. Both original keeper slots and their zonal positioning/closing of shot angles.
2. Original live save selection, animation contact, catches, blocks and recovery.
3. Original keeper possession decisions and roll/throw/punt distribution.
4. Goal tallies and conceding-side kickoffs, with the original centre-taker pass.
5. Original boundary decisions and automatic throw-ins, corners and goal kicks.

The HUD shows the score, saves, deliveries and restarts alongside pass/shot
attempts. CPU mode uses all 22 players and follows the ball. Keepers remain
computer controlled when taking over a blue outfielder.

## Original code and native boundaries

SOURCE is unchanged. `tools/match_recovery.py` extends build-time extraction
from INTELL.CPP, ACTIONS.CPP, BALL.CPP and RULES.CPP. RULES.OBJ is now hash-pinned
alongside the previously pinned objects. Missing keeper/action/set-piece values
are checked against their object bytes and recorded in player-recovery.json.

`got_ball` and `find_zonal_target` now use their complete original bodies.
`close_angle`, pickup actions/animation, throw initialization, `bounds_rules`,
`throw_taker`, `get_taker`, `centre_takers`, `decide_set_kick`, and `reset_ball`
are recovered. The live loop also restores the shot-reaction countdown from
`process_ball`, calls `keeper_boxes`, and includes both keepers in its alternating
team schedule. Existing save/contact, hold/recovery and delivery routines are
shared with the approved keeper replay.

The original `go_left_goal`/`go_right_goal` score increments and `last_goal`
assignment are extracted into `record_goal_score`. The collision event is
latched once; live play records its score and gives centre to the conceding side.
Replay fixtures retain their isolated goal-event behavior. Full goal-scorer
attribution, celebration/menu dispatch and halftime processing are not connected.

Restart initialization excerpts retain original ball coordinates, formation
rows, taker choice and goal-kick run-up. Menu, referee, audio and DOS user-registry
callbacks are omitted explicitly. Throw action retains the original CPU scanning,
pass choice, rotation and animation release; its referee/audio prefix is omitted.
The native adapter supplies the thrower's picked-up hand state before entering
the original throw action. It does not simulate the referee's pickup/walk-up flow.

The native transition waits for the ball to settle (60 ticks after goals, 25
after other exits), then places actors at original set-piece targets and resets
render interpolation for that placement. Original setup timers run before the
automatic decision. Original support runs remain active when a set piece needs
a passing option. Kicks and throws still release at original capture contact;
there is no injected ball velocity or receiver teleport. Input is suppressed
during setup, and throw takers remain CPU controlled until their action ends.

`get_target` now supports the connected restart positions; unsupported foul,
penalty and tunnel positioning calls still fail explicitly. Discrete formation
rows 64–69 are excluded from live-zone interpolation. `centre_takers` hoists
its loop variable to preserve Watcom scope. `get_taker` initializes its fallback
to the enabled first slot of the requested team. Earlier compatibility fixes
remain documented in TEAM-RECOVERY.md.

This is a continuous match preview, not the complete original match system.
Fouls/cards, offside, player-body knockdowns, fatigue, substitutions, match
clock/halves, referee presentation and sound playback remain unconnected.

## Verification

The live match suite checks both touchlines, all four corner quadrants and all
four goal-kick quadrants, correct taker teams and automatic release. Both goal
ends are tested for one score only, conceding-side centre, and animation-timed
kickoff contact. Both keepers are tested for live saves and held-ball delivery.
Eight seeded five-minute CPU runs exercise repeated restarts and check that
play remains running with finite player positions. Team/input/replay tests are
retained, and SDL smoke checks require 11 players per side and both keeper meshes.

Release validation: all 38 CTest suites passed in 83.14 seconds. Source audit
validated all four data archives; `git diff --exit-code -- SOURCE` is empty.
The rendered match HUD was captured and inspected. A final keeper-delivery
counter correction received focused team, live-match and CPU smoke checks.

![Original live match preview](original-live-match.bmp)
