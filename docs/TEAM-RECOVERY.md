# Original outfield team checkpoint, version 0.18.1

**Historical checkpoint:** version 0.19.0 adds both live keepers, scores and
automatic restarts. See [MATCH-RECOVERY.md](MATCH-RECOVERY.md) for the current
11-v-11 behavior. The outfield recovery provenance below remains applicable.

Open the existing View-Original-Actua96.cmd launcher. In Original 3D, press **Y**
or controller **Start** to enter team practice. **T** retains the approved
two-player passing fixture. This batch connects five related behaviors:

1. Two ten-player outfield teams initialized from the preserved tactics table.
2. Original zonal movement and support/pass-request runs.
3. Original computer possession decisions: shoot, cross, pass, punt or dribble.
4. Original opponent closing, sliding tackles and ball-contact possession changes.
5. Team selection using original receiver/interceptor/path candidates, with
   control transferred to a blue player when he collects the ball.

## Controls

**B / controller RB** enters CPU-versus-CPU from the original view, or toggles
between watching and controlling blue during team practice. All twenty
outfield players use recovered `intelligence` while watching, including the
initial ball carrier and subsequent blue receivers. Movement, kick and switch
inputs are ignored. Takeover preserves the ball, clock and action animations.
The camera follows the ball, the human selection ring is hidden, and the panel
shows blue/red pass and shot attempts. The same practice end conditions apply:
press R after a goal or ball out. Reset preserves CPU control; no match restart
rules or new AI have been introduced. Direct launch: `actua96 --cpu-teams`.

WASD/arrows or left stick/D-pad moves. Space/controller south passes; C/north
lofts; X/west shoots with possession and tackles without it. E/east or LB switches
to a useful blue player. P/Start pauses. R resets the same mode; on a controller,
press R3 while paused. Q returns to the original replay tests. As of 0.43.2,
Back no longer replaces play with a demonstration; it only navigates menus/replays.

Blue attacks right; red attacks left. The original selection marker identifies
your player. Goals and balls out settle for three seconds, then wait for R.
There are **no goalkeepers in this team fixture**. Their separate catch,
distribution and recovery replays remain available.

## Provenance

The generator adds 39 complete routine bodies and three explicit excerpts to
the previous checkpoint: 209 complete bodies plus ten excerpt/adapters in total.
`SOURCE/` is unchanged. Bodies are extracted at build time from ACTIONS.CPP,
INTELL.CPP, BALLINT.CPP and RULES.CPP; missing constants are verified against
the pinned Watcom objects. `tools/team_recovery.py` records constant bytes,
segments and offsets in the generated player-recovery.json report.

Complete additions include `intelligence`, `free_ball`, `we_have_ball`,
`help_chance`, `make_run`, `dribble_dir`, `opp_has_ball`, `get_tack_path`,
`go_to_between`, `get_near_path`, `ball_interact`, `user_init_tackle`,
`init_tackle_act`, `tackle_action`, `init_steal_act`, `steal_action`,
`process_comments` and `player_comment`, plus their small dependencies.

The three excerpts are the outfield branch of `got_ball`, the outfield branch
of `find_zonal_target`, and the normal-play branch of `get_target`. Guards reject
keeper positioning, set pieces and man-to-man modes at these excerpt boundaries.
Original support timing, tactical interpolation, pressure calculations, pass
choice, contact reach, control difficulty and tackle deceleration are retained.
Original `plhi_p` and `plhi1_f1` selection geometry comes from DATA.OBJ; its binding
is documented by 3DENG.C's `init3d` initialization of `plhilight[1][0]`.
Its BM_EXTRA1 texture is Euro archive record 36 on page 6, verified at
3DENG.OBJ instruction 0x13d24. It is loaded before the number strips, using
the same original transparent raster encoding.

## Native boundaries and compatibility fixes

The native scheduler follows ball-before-team processing and alternates the
first team each tick, matching the original process_teams ordering. It supplies
the enabled roster, uniform fixture attributes, initial possession, SDL input,
manual selection, selected-player movement, camera and end-of-play pause. It
calls original AI for the remaining outfield players. The original full
`process_teams`, `user_play`, `computer_play` and USER.CPP user registry are not
connected wholesale. Idle native dispatch supplies the recovered zonal branch;
the previous partial stand-action dispatcher remains explicit.

`go_to_between` retains Watcom's old for-loop variable scope through declaration
hoisting. `get_near_path` skips disabled roster slots, preventing selection of
absent keepers. `we_have_ball` converts its one-based player number to an array
index in the two crossing-distance reads; the source otherwise reads player 23
for the last away player. Live ball-zone indices are clamped to 0..31 before
the original tactics lookup. The array declaration spelling `[70]` replaces
`[(32*2)+6]` only for generated-declaration parsing.

The AI uses its original busy timers. There is no new waypoint AI, pass homing,
receiver magnet, or automatic ball transfer on a tackle button press. A tackle
must reach the ball and pass the original control test. Player-body knockdowns,
fouls, offside, stamina, substitutions, match score/restarts and original audio
playback remain outside this fixture. Unsupported referee/user/exit dispatches
fail explicitly rather than silently imitating them.

## Verification

CPU checks run twelve seeded sequences and require both sides to pass, shoot
and recover possession, with no human-controlled slots. Those runs recorded
12/18 blue/red pass attempts, 4/3 shots and 97 collections. A separate paired
trace confirms movement, switch and kick inputs do not alter spectator play.
Takeover preserves the live ball and clock, reset retains CPU mode, and leaving
team practice clears it. The SDL CPU smoke exercises keyboard entry/takeover,
virtual-controller RB handoff, pause/reset, both rendered teams and replay exit.
Version 0.18.1 passed all 37 CTest suites in 74.24 seconds. A final controller
hint adjustment received another CPU smoke check. Source audit validated all
four archives and the preserved SOURCE tree has no diff.

Team contracts cover roster setup, off-ball movement on both teams, selection
without moving possession, both directions of contested low-ball contact,
distance rejection, tackle entry/recovery, and twelve seeded play sequences.
The sequences require the computer to pass, shoot and recover possession while
checking finite coordinates, normalized facing and selection restricted to blue.
The SDL smoke covers team entry, pass, switch, tackle, pause/reset, both team
meshes and return to replays. Existing controller mapping remains covered by
the virtual-controller suite; physical team-play feel needs user review.

Version 0.18.0 validation: all 36 CTest suites passed in 79.79 seconds. The source audit
validated all four archives and `git diff --exit-code -- SOURCE` confirmed the
preserved source was untouched. The actual rendered team view was reviewed;
camera and HUD refinements received focused scene, team and controller checks.

![Original outfield team practice](original-teams.bmp)
