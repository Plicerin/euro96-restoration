# National teams and match presentation (0.21.0)

Update in 0.31.0: the CD provides six more matching national kits, bringing the
total to fourteen. Only Croatia and Turkey still need substitutes. Original
pitch texture, sky and goal-net clipping are now connected; the limitations
below describe the earlier 0.21 checkpoint. See [graphics recovery](GRAPHICS-ENVIRONMENT-RECOVERY.md).


Version 0.22.0 adds [match depth](DEPTH-RECOVERY.md), including the four original
match substitutes and celebration/replay camera transitions. The limitations
below describe the 0.21.0 checkpoint.

This five-feature batch restores team selection, roster names/attributes, separate
formations, stadium surroundings in live matches, and original camera views.
The setup UI, input routing and platform timing are native Windows/SDL adapters.
SOURCE is untouched.

## Roster and engine integration

`tools/restore_roster_camera.py` extracts the 16 countries and 320 players from
SCRIPT.96's PLYR_NMES and TEAM_NMES sections. It retains the source order and eight
ratings. EURO_INT.CPP / EURO_MAT.CPP TransferDataToMatchInfo and NormalisePlayer
establish the team/slot layout; default SquadInfo is 0..19. Matches use slots 0..10.
The other nine players are retained in the generated catalog, but substitutions
and squad editing are not connected yet. Speech identifiers are not shirt numbers:
Portugal's twentieth player is named POR27 in the speech table.

FOOTBALL.CPP init_player_stats supplies the simulation-mode +28 conversion for
pace, power, control, flair, vision, accuracy, stamina and discipline. ACTIONS.CPP
supplies tm_mcspd and tm_jump formulas. The native adapter deliberately omits the
old init_team debug override that assigns player 2 all-120 ratings. Match AI,
contacts, kicks and ball physics remain the recovered routines. Diagnostic
start_team_play fixtures retain uniform ratings; user-facing start_match uses
national rosters. Active teams stay fixed while the next-match setup is edited.
Names, shirt numbers, stats, human control and kit side follow the halftime swap.

Each side independently selects EURO.DAT entries 111..120, using SCRIPT.96's
formation names and per-country defaults. The preserved defaults include unusual
formations such as 3-1-6; these are source data, not newly designed tactics.
The native load adapter implements FOOTBALL.CPP load_new_tactics' end-relative
assignment, loading the opposite slot after RULES.CPP swap_teams.

## Preserved kit limitation

The renderer object's 512-byte teamlist has 16 descriptors matching the first
16 FILES.C teams, not the Euro roster order. Directly indexing it by Euro country
would dress England as Belgium and Germany as Northern Ireland. Matching available
kits are mapped by FILES.C identity: Bulgaria=3, Denmark=5, England=14, Italy=12,
Russia=1, Scotland=15, Spain=2, Switzerland=9. Other countries have no matching
kit descriptor in this archive revision. They use preserved USA artwork at home
and Denmark artwork away, explicitly labelled as substitutes in setup and the
match view. This is not a reconstruction of their missing Euro uniforms. Automatic
kit-clash selection and individual player appearance remain pending.

## Stadium and cameras

Live matches now draw all four preserved stadium meshes/textures with the same
transforms used in the earlier overview. The pitch mesh receives init3d's perimeter
extension using the verified object stadlist dimensions. The original addobjyc/addpolsc/addpolyc/addlinec clipping path draws the untextured
pitch even when its mesh origin is behind the camera; textured near-plane clipping
is not connected. Static original geometry,
including its flat roof colors, is retained. Crowd animation, sky projection and
video-board playback are not implemented.

Original camera bodies are generated from 3D_UPD2.CPP update_3d: Auto (0), Follow
Ball (6), goal kicks (9/10), corners (11/12), throw-ins (13), and Isometric (14).
The pinned 3D_UPD2.OBJ supplies/verifies scale 440, distance 260 and height 100.
Auto mode selects the original set-piece views while the restart is active.
Stadium camera limits follow the source's half-perimeter bounds. Corner-camera
zero distance is guarded. Fixed 20 Hz damping and render interpolation replace
DOS timer/update counters; the original target/camera equations are preserved.
This is not the entire old camera manager: Wire, mouse, referee/player views,
celebration/replay camera transitions and zoom controls remain pending.

## Controls and verification

View-Original-Actua96.cmd opens setup without extracting another ZIP. Arrows or
D-pad navigate; Enter/South starts CPU v CPU, Y/North starts human home control.
F2/L3 opens setup during play; F2/L3/Back closes it. V cycles the three live cameras.
Existing B/RB takeover, P/Start pause, R rematch, and Q/Back replays remain available.

The additional native contract test checks all 16 teams, per-player engine ratings,
all three camera modes, stadium rendering, name/formation identity after halftime,
and ten complete seeded CPU matches covering all ten formations and all 16 teams.
The SDL setup smoke drives both keyboard and virtual-controller selection/start.
The existing full-match suite also runs twelve complete national-team CPU matches
and samples every player's rendered pose each tick. Tests do not constitute a
human visual watch-through of every match.

Build reports are generated in roster-camera-recovery.json and graphics-recovery.json.
Original sound still requires the missing DIGI/PITCH/PITCH.MAD game archive and a
native audio backend; no replacement sound effects are introduced here.

Release verification: all 41 CTest suites passed (126.56 seconds), including 22 complete CPU matches. Original SOURCE diff is empty; all four archives pass the source audit. Setup and all three cameras were captured and visually inspected; the behind-camera pitch regression is covered explicitly.
