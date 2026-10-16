## Release-candidate audit — 0.63.0 RC1

The [completion checklist](COMPLETION-CHECKLIST.md) is the current backlog. The dated checkpoints below preserve the development history; their missing-feature statements do not describe the latest build.

## Current checkpoint: 0.63.0 — retail names and extra-time speech

All 320 players now have verified appearance mappings and spoken-name coverage (959 Euro tones plus one existing Actua fallback). Original score-specific extra-time announcements now play at the actual 90-minute knockout transition. [Evidence](RETAIL-IDENTITY-AUDIO.md).

## Historical checkpoint: 0.62.1 — all player previews resolved

All 320 preserved roster entries now have their authored retail previews. DOS character decoding and independently verified player identities resolve the 23 previously unmatched names, including Neno. [Evidence](PREVIEW-IDENTITIES.md).

## Historical checkpoint: 0.62.0 — retail previews and trophy films

Original animated player and venue previews, plus the Euro 96 championship trophy and credits films with stereo audio. See [recovery and validation](EURO96-PREVIEWS.md).

## Historical checkpoint: 0.61.1 — kickoff selection fix

Kickoff now selects the centre taker before the first gameplay tick. When the opponent kicks off, the nearest active outfield defender is selected. This applies to both local controllers and every change of ends. The original automatic centre pass is preserved, and live auto-selection resumes on its first eligible tick. [Reproduction and validation](KICKOFF-SELECTION.md).

## Historical checkpoint: 0.61.0 — retail quarter-final speech

All four original quarter-final introductions now play in original-draw tournaments. The retail qualifier table proves that the bracket pairings match the preserved engine, including two reversed home/away listings. Each match plays its complete STT recording without an appended compatibility ending. [Evidence and boundaries](EURO96-SEQUENCES.md).

## Historical checkpoint: 0.60.0 — retail tournament speech

Original-draw group introductions now follow the retail fixture lookup. Semi-finals and the final link the original opening to the correct countries. Championship, shootout and extra-time winners use their country-specific retail calls; decisive regulation knockout results use FLL. Quarter-final introductions remain on compatibility audio pending bracket mapping. [Evidence and boundaries](EURO96-SEQUENCES.md).

## Historical checkpoint: 0.59.0 — retail period commentary

Half-time and regulation results now select verified Euro 96 period recordings: score-specific HLF/FLL takes and the separate END bank for draws. Missing or ambiguous scores and knockout endings retain generic compatibility calls. [Evidence and remaining work](EURO96-PERIODS.md).

## Historical checkpoint: 0.58.0 — retail match audio

Thirty English event categories, all 132 retail score variants and the Euro 96 pitch/crowd bank are integrated. Source event IDs, gameplay and difficulty remain unchanged. [Verified mappings, tests and remaining audio boundaries](EURO96-AUDIO.md).

## Historical checkpoint: 0.57.1 — goalkeeper recovery and match panels

Human goalkeeper pickup/get-up transitions now finish in the original standing hold pose. R3/F6 information panels use original menu artwork, and hidden development-tab mouse targets are disabled during team matches. [Fix and validation](KEEPER-PANEL-FIX.md).

## Historical checkpoint: 0.57.0 — retail Euro 96 stadiums

All eight retail venue sets are integrated, with their own stadium palette and descriptor tunnel positions. Normal selection defaults to Wembley and cycles the Euro 96 venues. Legacy IDs remain available to old saves and replays. [Evidence, implementation boundaries and validation](EURO96-STADIUMS.md).

## Historical checkpoint: 0.56.0 — Euro 96 startup

The normal launcher enters the Euro 96 frontend directly. Actua films and unused title images are excluded from current packages. [Evidence and validation](EURO96-STARTUP.md).

## Restoration target — UEFA Euro 96

The agreed target is the PC/DOS Euro 96 game. Actua Soccer assets still present are compatibility material pending verified replacement. [Asset audit and priorities](EURO96-TARGET.md).

## Previous checkpoint: 0.55.0 - retail player and audio presentation

Retail heads, skin remaps, keeper artwork, 32 country-introduction phrases and Euro96 menu music are integrated. Match setup and match transitions received a presentation cleanup. The remaining FAP/GDV files are inventoried, not yet wired to gameplay. [Evidence and scope](EURO96-PRESENTATION.md).

## Retail shirt-number integration - 0.54.1

The CD’s complete light/dark number sets 1–20 are now bound to the original player number faces, including reserves, keepers, mirrored animations and changed ends. The optional atlas occupies a separate native texture page. Contrast selection is a native adaptation; the glyphs are original. [Implementation and validation](SHIRT-NUMBERS.md).

# Restoration findings and next work

## Previous checkpoint: 0.54.0 - Euro96 CD media

All 32 retail outfield strips, the retail kit-choice table, 296 matched appearance flags and 308 voiced player identities are integrated. Preserved head/skin and goalkeeper artwork remain in use; no roster or save-format changes. [Scope and evidence](EURO96-IMPORT.md).

## Previous checkpoint: 0.53.0 - presentation fidelity and validation

Close walk-off views now clip players and shadows against original stadium surfaces while preserving the tunnel opening. Country selection uses the original 64 carousel poses, direction and acceleration with native display pacing. All 89 regression checks pass. [Scope, evidence and remaining work](PRESENTATION-RELEASE.md).

## Previous checkpoint: 0.52.0 - original menu presentation

Recovered frontend fonts, button frames, country sprites, squad graphics and screen backgrounds now replace the normal game's placeholder menu styling. Pause, substitutions and replay controls share that artwork. Tournament fixture/result history and native settings remain accessible. [Scope and source boundaries](MENU-FIDELITY.md).

## Previous checkpoint: 0.51.2 - tunnel exit occlusion

Period-end exit rendering now sorts the original stadium faces with the players, allowing the tunnel roof and crowd to cover players entering the stand. Original tunnel routes, timings and live match state are unchanged. [Evidence and scope](TUNNEL-OCCLUSION.md).

## Previous checkpoint: 0.51.1 - player switching controls

LB and RB now switch players during human matches; RB no longer silently hands an exhibition to the CPU. Pause identifies automatic selection or CPU spectator mode. The original selection rules remain intact. [Investigation and validation](AUTOMATIC-SELECTION.md).

## Previous checkpoint: 0.51.0 - menu fade-down/up

Retained frontend pages now use the recovered 25-step down/up curve, with a black handoff, guarded buffered input, focus hold, target-reset recovery and exact settled text at fractional scale. The atlas audit confirms that all available number styles lack 16-20 artwork. [Evidence, controls and boundaries](MENU-TRANSITIONS.md).

## Previous checkpoint: 0.50.0 - original presentation finish

Original period-end referee/assistant dispatch now remains visible during player exits. Frontend menus use the recovered 25-step fade-up curve through the native compositor. An audit corrects the assumed missing referee tunnel walks and records why unused winner clips remain unassigned. [Evidence and boundaries](PRESENTATION-FINISH.md).

## Previous checkpoint: 0.49.0 - local head-to-head

Two pinned controllers now feed independent original user contexts into the shared engine. Both teams support selection, shots/passes, awarded restarts, distributions, substitutions and shootouts. Matching-side input persists across half-time; controller loss pauses and replacement explicitly joins the vacancy. [Controls and implementation boundaries](HEAD-TO-HEAD.md).

## Previous checkpoint: 0.48.0 - original first-time passing

The original both-button pass request now reaches its chip, standing-header and jumping-header records. Receiver selection, trajectory feasibility, animation and release still run through the preserved engine. This is a documented correction to three contradictory conditions in generated code. [Details and validation](FIRST-TIME-PASSING.md).

## Previous checkpoint: 0.47.0 - saved replay library

Native replay files persist the original visual buffer, with isolated viewing, preserved cameras/interpolation, controller navigation and safe return to the paused live match. File validation and failed-save handling protect existing clips. Valid reserve identities 16-20 no longer crash the renderer; their unavailable back-number overlay is omitted using the original no-number face count. [Controls and original/native boundaries](SAVED-REPLAYS.md).

## Previous checkpoint: 0.46.0 - individual walk-offs and referee restart movement

The original init_off_int/remove_player route and camera 17 now show substitutions and dismissals on presentation copies. Native match decisions remain authoritative and frozen throughout. Referee/assistant restart movement uses goto_target; at_target corrects the source's X-for-Y typo and handles coincident targets safely. [Batch details and source boundaries](WALK-OFFS.md).

## Previous checkpoint: 0.45.0 - original pre-match lineup

The original init_team presentation positions, init_officials, animate_players and pre-match camera expressions now run on a squad copy before kickoff. FOOTBALL.OBJ confirms the 180-tick countdown; the last 40 ticks hold the camera. Native skip/pause/focus controls and held-action protection preserve the prepared live match. [Source boundaries and combined test](PRE-MATCH.md).

## Previous checkpoint: 0.44.0 - original automatic selection

The intact USER.CPP `clear_auto`, `auto_select_a` and `auto_select_b` routines now choose the human player's team member. Original selection circles and receiver/interceptor/path priorities are preserved. A native single-user tick boundary retains manual-switch priority and protects keeper/contact/set-piece control. [Source scope and five-case playtest](AUTOMATIC-SELECTION.md).

## Previous checkpoint: 0.43.2 - controller Back safety

Removed the legacy Back-to-demonstration shortcut from live/paused original-engine play. Start still pauses/resumes, and Back remains available inside menus/replays. Virtual-controller frontend checks cover both live and paused Back presses and subsequent Start resume. Gameplay routines are unchanged.

## Previous checkpoint: 0.43.1 - tournament stabilization

The complete frontend tournament flow now has a 31-match session check covering 62 natural tunnel exits, automatic goal replays, period handoffs and exact saved-state reloads. Tournament Resume now falls back to a validated backup when the primary is invalid or missing; later saves preserve that backup. [Current status, evidence and remaining human checks](RELEASE-CANDIDATE.md).

## Previous checkpoint: 0.43.0 - original startup intro

The complete original VFULL_S film now plays at startup with its original PCM soundtrack and title-screen replay. All 399 decoded frames match an independent strict FFmpeg pass after equivalent normalization of valid early-end commands. Native controls include skip, pause, focus/disconnect pause and clean title return. [Recovery and limits](CD-INTRO.md).

## Previous checkpoint: 0.42.0 - original tunnel exits

Half-time/full-time player exits now use original tunnel_pos, movement and motion capture with the five CD stadiums' tunnel descriptors. A presentation copy preserves the live match, with pause/focus/skip and team-talk/results handoffs. [Recovery and limits](TUNNEL-EXITS.md). Pre-match/referee entrances/exits and individual walk-offs remain separate.

## Previous checkpoint: 0.41.0 - original French commentary

Original French match events, introductions, scores and matched player names now follow the saved Voice Language setting in Audio Options. Existing settings migrate without changing tournament progress. [Recovery, controls and validation](FRENCH-COMMENTARY.md). Tunnel/intro/menu presentation remains incomplete.

## Previous checkpoint: 0.40.0 - original CD film

The complete original VGA making-of film and PCM soundtrack now play from the title screen using native SDL video/audio. The import verifies every frame and rejects decoding errors. The short intro and high-colour variants remain unsupported. [Recovery, controls and limits](CD-MOVIE.md).

## Previous checkpoint: 0.39.0 - replay controls

The replay viewer now connects original speed levels and manual ending behavior, with exact frame stepping, bounded seeking, a timeline and optional looping. Playback uses recorded original states and a separate engine copy. [Controls and fidelity boundaries](REPLAY-CONTROLS.md).

## Previous checkpoint: 0.38.0 - original human ball controls

Four more original routines now process held buttons for full-team play. The five-item batch covers shots, passing/crossing, first-touch/aerial requests, aftertouch and keyboard/gamepad help. See [mappings, validation and explicit source limitations](HUMAN-CONTROLS.md). The generated shot-release boundary corrects the aftertouch direction initialization; SOURCE remains unchanged.

## Previous checkpoint: 0.37.1 - release audit

See [the current assessment, validation and concrete finish list](RELEASE-AUDIT.md). Match settings, tournament continuity and tactics are integrated; release packaging now has isolated verification and an additional 32-fixture matrix. [Current portable controls](QUICKSTART.txt).

Everything below is historical checkpoint evidence. Pending-work statements describe their checkpoint, not the current build.

## Historical checkpoint: 0.31.1 - complete original CD stadium

The red band exposed an unverified visual limitation of the preserved Euro
stadium asset set. The finished original CD supplies a different, complete
high-detail England stadium. Its matched meshes, texture pages, UVs, dimensions,
pitch and sky now load together. No replacement textures or speculative colour
changes were applied. See [stadium provenance and validation](STADIUM-RECOVERY.md).

## Previous checkpoint: 0.31.0 - original graphics environment

The CD supplies six previously unavailable matching kits: France, Germany,
Holland, Portugal, Romania and Czech Republic. Fourteen Euro countries now use
matching original uniforms and crowd palettes. The preserved Euro archive supplies
the original detailed pitch tile and sky bitmap/palette. Native bounded projection
replaces the DOS ground scanline backend. Goal meshes now use the original
near-plane clipping call. SOURCE remains unchanged.
See [graphics provenance and verification](GRAPHICS-ENVIRONMENT-RECOVERY.md).

## Previous checkpoint: 0.30.0 - goal types and spoken scores

Original goal classification now selects header, woodwork, keeper error/deflection,
and powerful/curling-shot commentary. Thirty-three original recordings supply
spoken scores after celebrations, with all 48 supported score combinations checked.
The complete CPU regression remained 1:0 in 7,328 ticks; all 57 checks passed.
See [commentary recovery](COMMENTARY-RECOVERY.md).

## Previous checkpoint: 0.29.0 - original English commentary

AUDIO.H and pinned AUDIO.OBJ verify 35 phrase categories plus two fixed period
recordings. The supplied disc provides 363 unambiguous event clips plus 47
verified pre-match/country clips. Original keeper,
post/bar, foul/card, passing and restart calls feed native bounded speech playback;
existing native goal/period transitions supply generic goals and period messages.
F9 adjusts commentary; F8 remains master mute. Repetition/priority, background
ducking and pause/focus handling use a native scheduler. The CD executable verifies country introductions for 14 Euro teams; Croatia,
Turkey and individual player names remain unmatched. SOURCE is unchanged.
See [commentary provenance and controls](COMMENTARY-RECOVERY.md).

Verification: 57/57 regression checks passed, plus five focused checks after the
verified country introductions were added. The complete CPU match finished 1:0
in 7,328 ticks, with eight spoken sequences, no repeated clips and no engine
speech event loss. The published snapshot passed hidden commentary smoke and
media hash verification. See [test evidence](commentary-test-results.txt).

## Previous checkpoint: 0.28.0 - title, setup and match summaries

The original CD title and four music tracks now accompany native SDL navigation:
title, team selection, pre-match starting elevens/formation, half-time and results.
Euro 96 setup backgrounds and the restored match font retain the preserved data's
presentation. Human half-time waits for confirmation; CPU half-time continues
after eight seconds. Scores and original period transitions remain authoritative.
See [front-end provenance, controls and validation](FRONTEND-RECOVERY.md).

Verification: all 55 regression checks passed; three focused checks passed after
final layout/input corrections. Five screen states were visually reviewed. The
published snapshot passed its hidden front-end smoke and media hash verification.
All four original archives validate; SOURCE is unchanged. No interactive game was restarted.
See [test evidence](frontend-test-results.txt).

## Previous checkpoint: 0.27.0 - original match audio

The supplied CD's pitch samples now play through a native SDL mixer. AUDIO.OBJ
supplies the verified pitch/crowd mappings; original contact events and retained
referee calls drive effects. Native match transitions supply period whistles,
and native crowd scheduling adds ambience and event reactions. F8 mutes sound;
pause/focus loss, setup, squad and replay transitions discard stale playback.
At this checkpoint, commentary, menu music, videos and the original crowd-mood scheduler remained pending; menu/result music is connected in 0.28.0.
See [audio provenance, controls and tests](AUDIO-RECOVERY.md).

## Original CD inspected - 2026-10-10

The supplied Actua Soccer ISO resolves the missing original audio-assets blocker.
All 20 pitch SampleBank names match the disc; 183 MAD banks (7,400 PCM samples)
and eight music WAVs validate. At inspection time, native playback and commentary mapping remained
pending; match effects are now connected in 0.27.0. Graphics archives were kept separate from the Euro 96 snapshot.
See [CD inspection and reproduction](CD-INSPECTION.md). No game was restarted.

## Previous checkpoint: 0.26.0 - original match presentation

The high-resolution original radar, player/ball markers, team identity across
halftime, proportional match font, clock, score and pending-substitution sprites
are restored. FGFX.C tables match 3DENG.OBJ byte-for-byte; loader instructions
verify the EUROREND records. Native clipping and SDL replace the DOS blitter.

The match now uses a larger 4:3 viewport by default. Restoration panels can be
shown with F6 / right-stick click; Tab / F7 controls the radar. Pausing, set pieces
and manual shootout keeping retain relevant native controls. Original gameplay
and SOURCE remain unchanged. See [provenance and controls](PRESENTATION-RECOVERY.md).

Verification: all 51 checks verified. The full run passed 49; two controller
smoke checks exposed a new R3/full-time rematch conflict. After limiting the new
panel toggle to ongoing play, both passed, alongside penalty input and overlay
contracts. All three cameras, pause and set-piece captures were inspected. Four
source archives validate; SOURCE diff remains empty. See
[presentation test evidence](presentation-test-results.txt).

## Previous checkpoint: 0.25.2 - readable match HUD

Five presentation improvements extend the set-piece readability pass: prominent
score/clock, possession-specific action rows, explicit keeper pass/loft/punt
controls, compact CPU match statistics, and a pause reference panel for squad,
replay, setup and restarting. Full time offers new match/setup; penalty flight
has a distinct status. Original gameplay, input bindings and SOURCE are unchanged.

The SDL pixel style is retained with larger type, spaced key labels and a clear
score-to-controls hierarchy. Statistics use existing engine counters. The pause
panel documents existing bindings; it does not introduce clickable menus.
See [HUD behavior and review](MATCH-HUD.md).

Verification: all eight focused existing checks passed (human/CPU control,
setup, squad/replay/pause, penalties, set pieces and the pitch tracker). Penalty
and depth checks passed again after final prompt corrections. Five live states
were visually reviewed with gamepad and keyboard prompts, plus a completed CPU
match. All four source archives validate and SOURCE is unchanged. The full
50-test suite was not repeated for this presentation-only update.
See [test evidence](hud-test-results.txt).

## Previous checkpoint: 0.25.1 - original on-pitch aiming guide

Recovered ACTIONS.CPP draw_tracker and 3DENG.C addobjsp restore animated flight
and ground dots for human penalties, free kicks, corners and goal kicks. Original
ball prediction runs on an isolated render snapshot. Dots retain original palette
IDs and depth sorting, enlarged in SDL for readability. Phase follows simulation
time, and interpolation stays within the 50-entry prediction table.

Large native power/height bars and a shorter contextual sidebar replace the
small numeric settings while aiming. Penalty power remains fixed by the original
engine. Throws retain their original direction rules. SOURCE is unchanged.
See [controls and provenance](SET-PIECE-CONTROLS.md).

Verification: all 50 checks verified (49 passed in the full run, followed by a
passing rerun of the controller disconnect smoke test). That test now checks
removal of its active virtual device and pausing without assuming no other
controller is connected. Both-end tracker tests cover three cameras, bounded
prediction, cancellation and render-independent gameplay. Free-kick and penalty
captures were visually inspected; all four source archives validate.
See [test evidence](tracker-test-results.txt).

## Previous checkpoint: 0.25.0 - manual original match restarts

Original human set-piece aiming, run-up and throw routines now receive SDL
keyboard/gamepad input in normal matches. Home-team control follows the selected
original taker across changes of ends. Keeper catches retain original recovery
before accepting pass, loft or punt input; selection returns to the outfield.
CPU restarts and automatic kickoffs retain their existing paths.

F5 starts/cycles a separate seven-situation practice match for this feature batch.
See [controls and provenance](SET-PIECE-CONTROLS.md). Native input/HUD and phase
scheduling surround recovered source routines; SOURCE remains untouched.

Verification: all 49 current checks verified (48 in the full run, followed by
a passing rerun of the updated home-keeper selection contract). The SDL virtual
gamepad test plays all seven F5 situations. Both-end source-action tests cover
release and cancellation; seeded CPU play supplies real catches for the manual
recovery/delivery checks. Final HUD capture inspected; four archives validate.
See [test evidence](setpiece-test-results.txt).

## Previous checkpoint: 0.24.1 - penalty result motion

Fixed the goal-line freeze: finish_penalty reset the ball immediately (zero
velocity, ground height and cleared goal latch), then the result wait returned
without advancing physics. It also reset presentation snapshots at that instant.

The result is now recorded once while retaining the original ball state. Original
trajectory/net collisions, kick completion, keeper save/recovery and held-ball
position continue during the three-second result interval. Original next-kick
setup performs the reset. Presentation snapshots remain continuous across scoring.
No SOURCE files or original physics formulas changed.

Verification: a new regression failed on 0.24.0's discarded height/velocity/net
state, then passed after the fix. All 47 checks pass. Focused coverage includes
both goal ends, back-net rebounds, interpolated presentation, pause, single goal
accounting, next-kick reset, wide misses and eight complete CPU shootouts with a
retained catch and continuing animations. The existing twelve CPU and twelve
human/CPU shootouts and four extended matches also pass. Offscreen captures at
both ends were inspected. All four source archives validate. The launcher points
to the patch for the next launch; the interactive game was not restarted.


## Previous checkpoint: 0.24.0 - manual shootout penalties and keeper control

The original human penalty hold/release branch now drives the run-up and shot.
A native aim/height guide shows its current settings. Home-team control alternates
between taker and goalkeeper; the original Euro keeper direction decision handles
correct-side saves and wrong-way dives. Keyboard and gamepad work through the
existing launcher. Pause, focus loss, menus and active-controller disconnect cancel
unfinished charges and require neutral input before charging again.

This also corrects the previous CPU penalty integration: the recovered Euro 96
exception now prevents automatic normal-play swerve/height selection overriding
penalty settings at contact. Tests check actual launch direction, original power
and height, not just the displayed meter. Original SOURCE remains untouched.
Manual controls currently cover shootouts; ordinary in-match penalty restarts
retain their existing CPU adapter. See [controls and provenance](PENALTY-CONTROL.md).

Verification: all 46 CTest checks passed; the final label cleanup and strengthened
launch-direction/power assertions passed a targeted two-check rerun. Twelve CPU
shootouts produced 105 goals and 36 misses. Twelve human/CPU scripted shootouts
completed with 109 manual kicks, 107 keeper dive starts and six recorded saves.
Four extended CPU matches completed. Low/high launch velocities match original
get_traj (1.33333 / 5.33333 for 4/16 ticks). Virtual-gamepad smoke covers charging,
pause, focus cancellation, neutral rearming, release and disconnect. The aim guide
was rendered and inspected. All four source archives validate; known missing
headers/source/audio assets remain documented. No interactive game was restarted.


## Previous checkpoint: 0.23.0 - knockout matches and penalty shootouts

Original extra-time thresholds, end swaps, golden-goal flag, penalty ordering,
clinch/sudden-death rules and Euro CPU penalty aiming/run-up now feed the native
match scheduler. Setup offers Regulation, Knockout, Golden goal and Penalties
only. Results separate the match score from shootout totals and identify the
winner; the penalty overlay shows each kick. See [knockout recovery](KNOCKOUT-RECOVERY.md)
for original routines, native scheduling boundaries and CPU-only penalties.

Verification: all 44 checks pass across the full run and targeted rerun. The
initial new score-preservation assertion attributed a pre-swap player's passes
to the wrong stable team; correcting that assertion verified that his passes
remain with his original team. The full existing 43-test set passed, and the
final knockout/setup rerun passed after that correction and UI label polish.
Twelve shootouts completed with 64 goals and 46 misses, including an eight-kick
sudden-death win. Four CPU matches forced level near 90 minutes completed both
extra periods and shootouts. Forced fixtures cover golden-goal completion and
wins at 90/120. Setup, a live penalty and the final result were visually reviewed.
SOURCE is unchanged and all four asset archives pass the source audit.

## Previous checkpoint: 0.22.0 - cards, officials, fatigue, substitutions and replays

This five-feature batch restores original discipline, visible officials with
original movement and animation, original fatigue, original celebration actions
and the scorer camera. Native squad controls connect original roster ratings and
bench choices at stoppages, preserve team totals, and support keyboard/gamepad.
A separate replay snapshot engine freezes the live match and resumes its exact
state. Full original textured stadium clipping fixes the close scorer view.
See [match depth recovery](DEPTH-RECOVERY.md) for controls, provenance, historical
rules and the explicit native substitution/replay boundaries.

Validation: 42 of the 43 CTest checks passed on the full run; the remaining
new test incorrectly expected a celebration for an own goal. After correcting
it to the original MC_SHAME -> MC_SHAMEW dispatch, that complete depth test
passed, giving 43 passing checks across the full run and targeted rerun. Its
six completed CPU matches produced 16 goals, 23 card events and 36 substitutions.
Normal goals and own goals at both ends completed animation, camera, replay and
kickoff without double scoring. Final 0.22.0 squad and celebration captures
were visually reviewed. The preserved SOURCE diff is empty; all four archives
passed the source audit. Published for the next launch without restarting play.

## Previous checkpoint: 0.21.1 - halfway-line clipping

Pitch markings now use the original `3DENG.C` `addobjfc` ground transform and
near-plane clipping, as in original `render3d`. The general object path discarded
an entire half-line mesh when its origin fell behind the camera cutoff, although
part of that line remained visible. Preserved source and geometry are unchanged.
Regression coverage checks both sides of midfield, centre-spot continuity and
retention of the far half and touchline. All five relevant graphics, scene,
national-team and setup tests pass; the automatic-camera kickoff capture was
visually checked. Published for the next launch without restarting the game.

## Previous checkpoint: 0.21.0 - national teams and match presentation

Sixteen original Euro rosters feed names and simulation ratings into regulation matches.
Each side selects one of ten preserved tactics tables; the tables follow the teams at half-time.
The original stadium and perimeter now surround live play. Original auto, isometric and
follow-ball cameras are selectable, with original set-piece views in auto mode.
Native match setup supports keyboard and gamepad. Eight Euro teams have matching kits
in the older preserved renderer table; missing kits are explicitly labelled substitutes.
See [national-team recovery](NATIONAL-RECOVERY.md).

## Previous checkpoint: 0.20.0 - regulation matches

The five-feature batch connects original clock scaling, half-time/end switching,
full-time results, tackle fouls/free kicks and offside/indirect restarts.
Penalty and defensive-wall setup are included. Blue identity, statistics and
human control follow the roster through the original half-time swap.
See [regulation recovery](REGULATION-RECOVERY.md) for provenance and boundaries.
Cards, substitutions, referee presentation, fatigue and sound remain pending.
## Previous checkpoint: 0.19.0 - live keepers and automatic restarts

The five-feature batch connects goalkeeper positioning, saves/catches/rebounds,
keeper distribution, score/kickoff continuation, and boundary restarts to the
22-player view. B/RB watches both original CPU teams; Y/Start plays blue.
Original routines determine saves, passes, restart type/side, placement targets
and set-piece decisions. Native transitions replace referee/menu walk-ups.
See [match recovery](MATCH-RECOVERY.md) for provenance and test coverage.
Fouls, offside, clock/halves, substitutions and audio remain pending.

## Previous checkpoint: 0.18.1 - CPU-versus-CPU viewing

B / controller RB gives all twenty outfield players to the recovered AI and
follows the ball with a wider camera. Press it again to take over blue without
resetting the action. The HUD shows each side's pass and shot attempts. Reset
retains the chosen control mode. Goals and balls out still require a practice
reset; full-match restart logic remains pending.

## Previous checkpoint: 0.18.0 - original outfield team AI

Y / controller Start opens team practice with twenty original outfield players.
The batch connects tactics-based setup, zonal/support movement, computer
possession decisions, opponent challenges, and useful team selection. See
[team recovery](TEAM-RECOVERY.md) for provenance and compatibility fixes.
Goalkeeper match dispatch, referee rules, scoring/restarts and audio remain
unfinished. The earlier replay and two-player control fixtures are retained.

## Previous checkpoint: 0.17.0 - playable original actions

T opens a two-player original 3D control fixture. One batch
connects running/turning, dribbling, ground passing, lofted passing and driven
shooting to keyboard and gamepad input. Original interception handles reception;
the selected player changes on collection. See [control recovery](CONTROL-RECOVERY.md)
for the six recovered bodies and explicit native input/fixture boundaries.
Full original team AI and match processing remain unfinished.

## Previous checkpoint: 0.16.0 - original goal collisions

Sixteen more original BALL.CPP routines restore goal-line detection, both post
and crossbar impacts, and inner/outer net responses. The attack continues into
the net after a goal. G/H run one five-case review at either end. These are
original collision algorithms; match scoring and restarts remain separate.
See [goal recovery](GOAL-RECOVERY.md) for source evidence and preview boundaries.
All 32 CTest suites passed, including the two new collision and window-review suites.

## Previous checkpoint: 0.15.0 - original scene batch

One combined batch restores separate team/keeper materials, complete action-view
pitch markings, both textured goals and original distance-based detail selection,
player/ball shadows, and original ball orientation. See [scene recovery](SCENE-RECOVERY.md).
The 0.14.1 ball-continuity fixes and original action timings are retained.

The original action fixture now includes first-time passing/shooting, keeper
catch/recovery/carry/distribution and chest control. It is not a complete match.
Next substantive engine work is the original user/team processing loop and
remaining action/rule branches. Audio still requires the absent original sample
archive; no substitute effects have been introduced.

## Previous checkpoint: 0.11.0 - original interception and movement

The original predictor and control-input interception chain now select and execute
the receiver's meeting point. The fixture's nearest-frame reception heuristic is
removed. Original run, turn, wait and control actions drive the approach and contact
timing. Space replays the straight pass, M changes the passer's initial animation
phase, and N shows an angled reception. The fixture supplies the original control
button; computer first-touch choices and full match AI are not restored.

27 additional complete source routines compile natively, bringing player/action
recovery to 73 bodies. The missing interception record and constants have object
provenance. One documented loop-scope compatibility edit preserves Watcom semantics.
Tests cover moving reception, both kicking feet, four angled passes, unreachable and
out-of-bounds paths, interrupted plans, original prediction and existing contact limits.
The stable launcher remains the update path. SOURCE stays untouched.

## Previous checkpoint: 0.10.1 - original animated actions



The player-facing conversion now includes both sign changes from the original coordinate

pipeline, correcting reversed models and displaced captured balls. Eight-direction

render-versus-gameplay coordinate checks cover this regression.



The original pass/trap chain is now visible in the 3D view. We compile 46 original

player/action routines (25 added this checkpoint) and nine renderer routines. The

motion-loader evidence now maps 92 animation IDs instead of 56; both EAX and EDX

addressing forms are handled, including the pass and trap captures needed here.



The two-player fixture calls original init_kick_act, kick_action, fire_ball_off,

pass_ball, process_anims, init_control_act, control_interact, collect_ball,

control_action, and get_mcball_coords. Animation IDs/contact fractions come from

DATA.H; missing speeds and displacement constants come from the pinned objects.

Original addobjyf handles mirrored poses. The renderer follows render3d's captured

ball position during a trap; after recovery the original hold_ball tween takes over.



This is still an integration fixture, not restored match AI. Native setup places two

players and supplies a predicted interception to the original control initializer.

The limited init_anim dispatcher fails explicitly for unconnected animation actions.

The full process_teams/do_logic loop, directional interception selection, shots,

keeper action chain, inter-animation visual tweening, and sound remain pending.

See ACTION-RECOVERY.md for detailed ownership and validation.



## Previous checkpoint: 0.9.0 â€” original graphics and contact restoration



The restoration target is the original match engine, graphics and audio. The native

practice AI/rules remain available for regression testing; further custom gameplay

expansion is not the current direction.



Eight complete original 3DENG.C bodies now compile natively: matcop, matmul,

addpoly, addline, add3dcmap, add3demap, addpols and addobjy. DATA.OBJ supplies the

original face/line/ball/pitch geometry. EUROREND.DAT supplies 95 captures containing

4,983 frames, the original indexed textures/palette, and stadium meshes. The original

kit table from 3DENG.OBJ has 16 entries; the older FILES.C table is not substituted.

The renderer object's filename strings identify EUROREND.DAT/OFF. A checked evidence

manifest ties 56 animation IDs to loader immediates; unidentified captures keep their

archive record labels. All frames are validated, not relabeled by visual guesswork.



The native inspection view draws textured players with the original cylinder/elliptical

polygon assembly and supports the original wireframe. A stadium overview uses original

positions, texture rebasing, palette overlay and pitch line geometry. SDL texture upload,

triangle rasterization, inspection camera and controls replace DOS display plumbing.

This is not pixel-identical DOS rasterization or the complete render3d path: full match

scene updates, action-driven tweening, shadows, sky, ground texture rasterization,

all goal detail levels and frontend integration still need restoration. Inspection

animations advance at a fixed 20 frames/second, not their gameplay-specific rates.



The missing ball.dta dependency was partially recovered: ACTIONS.OBJ retains all

132 save_offs triples. Original rotate_offs and ball_at_contact bodies now compile and

are tested, bringing restored player routines to 21. The original action state machine

is not yet driving the native practice mode. SOURCE/ remains untouched.



Audio: AUDIO.H and MY_AUDIO.CPP survive, but AUDIO.CPP and the sample archive are

absent. AUDIO.OBJ contains the original path DIGI/PITCH/PITCH.MAD. The repository's

four DAT/OFF archives contain graphics/animation/tactics, not that external audio

archive. Original game/CD files were requested; no replacement samples or speculative

MAD decoder were introduced.



Next restoration sequence: recover remaining motion/action dependencies and wire the

original kick/receive/keeper chain; restore FOOTBALL.CPP do_logic in its original order

(process_ball, match_rules, keeper_boxes, distances/nearest, process_teams and users);

connect original new_update_coords/realtime_coords to the native renderer; implement

the sound backend against actual original sample data. Validate recovered state against

preserved routines at each boundary. Avoid adding replacement AI or football rules.



## Previous checkpoint: 0.8.0



Opponent mode now has a three-minute active-play clock, a frozen full-time result

with final score/shots/saves, and keyboard, controller or mouse rematches. Pauses,

hidden/inactive views and restarts stop the clock; keeper possession consumes it.

Final-tick contacts and goals resolve before the result. Solo practice is untimed.



All 21 CTest suites pass. Six unattended matches cover 18 active minutes / 24,000

simulation ticks, checking valid state, possession, keeper release and restart

progress. Native SDL events cover full time and keyboard/mouse rematch; virtual gamepad

events cover Start rematch. Human assessment of the new match flow is pending.

The match timer is new adapter logic; SOURCE/ is unchanged.



## Previous checkpoint: 0.7.0



Two automatic keepers and deliberate red shots make this a two-way scoring drill. Keepers track

play with bounded lateral movement/reach, catch manageable shots, parry difficult contacts, and

release a real ground outlet after a one-second protected hold. Swept contacts are ordered with

outfield interceptions and goal crossings. Rebounds remain playable; catches, saves and shots

appear in the HUD. Unpressured red carriers advance toward goal and shoot through clear lanes.



Twenty test suites cover both-end saves, catches/releases, rebound recovery, contact ordering,

keeper misses, red shooting/scoring and blue kickoffs. The native-window smoke exercises both

teams scoring, a keeper catch and release, and keeper/outfield control separation. Keepers are

native practice adapters; original keeper animation and full match/referee systems are unfinished.

The earlier dribble/tackle contracts disable shooting only within their isolated test fixtures.



## Previous checkpoint: 0.6.1



The unselected blue teammate now covers the red passing lane during opponent possession and

pursues loose balls. An explicit early return previously left him stationary in both situations.

He uses ordinary movement and swept ball contact, can intercept, and switches to attacking support

when blue regains possession. Selecting him immediately restores manual control; a held ball is

not automatically tackled. The assisted blue receiving run retains priority.



Eighteen suites cover attacking support, defensive movement with either player selected, lane

coverage, loose-ball recovery, manual override, pause/restarts and solo isolation. The native

window smoke checks teammate defence while the selected player remains stationary. Defensive

chase scenarios now accept legitimate interceptions by the teammate as team possession recovery.



## Previous checkpoint: 0.6.0



Opponents now make ground passes under pressure, to an open forward teammate, and after a

prolonged carry. They check the led destination, lane and receiver clearance, wait after gaining

possession, and use a team pass cooldown. The teammate meets the fixed destination with the same

local reception as blue; a user can intercept. Blue control is unaffected by red kicks.

The HUD shows completed/attempted red passes. Red shooting remains unfinished.



Seventeen test suites pass, including 17 opponent-passing checks and native-window pass/reception.

All 16 human-rate chase scenarios regain possession, including during red passing sequences.

The previous dribble-only path regression now disables the receiving teammate to isolate steering;

passing and interception are covered separately. Reopen the existing launcher for this update.



## Previous checkpoint: 0.5.3



Opponent movement now commits to safe dribble waypoints, retains support sides through midfield,

and changes the pressing defender only with a clear distance advantage after a short commitment.

The old per-tick boundary/centre fallback could reverse steering hundreds of times while carrying.

Five deterministic 30-second runs reduced the worst reversal count from 163 to 5. Player speed,

turn limits, tackle reach and reaction windows are unchanged. Existing chase/regain checks pass.



Sixteen test suites include extended dribble boundaries, role stability, meaningful role changes,

and support movement across midfield. These remain

small-sided native AI adapters; full original team AI, red passing and shooting are still pending.



## Previous checkpoint: 0.5.2



The receiver no longer waits for a stick/key release after collecting a pass. The input safeguard

now applies only during flight; possession restores the latest held direction and analog strength

for the next simulation tick. One-touch return passes retain flight protection, and neutral input

stops the player. The ball trajectory, foot-reach assistance and original movement speed are unchanged.



Fifteen suites cover the fix, including receiver-flow regressions across both modes, ground/lob

passes, eight directions and partial/full stick movement. Virtual-controller and native-window

tests also hold the stick through reception. Reopen the existing launcher to play this update.



## Previous checkpoint: 0.5.1



Gamepad controls now include analog movement, D-pad movement, right-stick shot aiming,

passing/lobbing, held-button tackling, player switching, pause, and a paused-only reset.

The active controller drives both axes and buttons. Xbox and PlayStation face-button prompts

follow the last input device. Controllers can connect during play; loss of the active pad pauses

play until manually resumed. The original source snapshot remains unchanged.



Fourteen suites pass, including 47 checks driven by SDL virtual Xbox/PlayStation controllers

and a native-window controller smoke test. The controller UI capture was visually inspected.

Physical hardware playtesting remains pending. Reopen the same local launcher for this update.



## Previous checkpoint: 0.5



The opponent challenge now has goals, a blue/red scoreboard, and automatic kickoffs after scoring.

Blue attacks right. Scoring checks the whole-ball crossing, mouth width, height at the crossing,

and earlier player contact. Own goals award the team attacking that end. The conceding team gets

centre possession after a two-second celebration. Out balls restart after one second near the exit

for the opposite last-touch team. Restarts preserve stats and score; R clears the game.



Twelve test suites pass, including 32 goal/restart checks. Native input/render testing includes a

shot, goal celebration, and conceding-team kickoff. New rules are small-sided practice adapters;

goalkeepers, opponent shooting/passing, official restarts/refereeing and match timing are pending.

The reusable local playtest launcher delivers this update without a ZIP.



## Previous checkpoint: 0.4.2



Possession recovery now uses an eight-tick active tackle window, forward 40-unit user reach,

and held-X retries at a 12-tick interval. The old instantaneous contact test was unforgiving

against moving opponents. An intercepted pass also clears its input-release guard, allowing

immediate defensive movement. Holding tackle never auto-shoots after a regain.

AI reach/wind-up and the approved passing/support behavior remain unchanged.



Eleven suites pass, including 65 opponent checks. All 16 keyboard chase scenarios regain

possession using eight-direction steering at 150/300 ms reaction intervals.



## Previous checkpoint: 0.4.1



The off-ball teammate now looks for open space and a clear passing lane in the opponent drill.

Position scoring balances defender clearance, lane visibility, useful spacing, and travel cost;

replanning uses hysteresis to avoid twitchy direction changes. The receiver automatically runs to

meet a moving-target pass unless fresh movement input or manual selection takes over. The ball

trajectory and the approved solo receiving behavior remain unchanged.



Eleven test suites pass, including 31 support checks alongside the 73 existing player and 28

opponent checks. Support AI and receiving runs are new drill logic, not restored full-match AI.



## Previous checkpoint: 0.4



Two-versus-two keep-ball now adds red opponents who pressure, intercept and dribble away.

The user can tackle to recover possession. Tackles have range/facing checks, visible AI wind-up,

recovery periods and a short possession-protection window. These are native drill adapters, not

claims that original full-match team AI or animated tackles have been restored.

The approved 0.3.1 passing/receiving behavior remains available with opponents off.



Ten CTest suites pass, including 73 passing/player checks and 28 opponent checks.

The native opponent UI capture was inspected; event tests exercise tackles, mode changes and reset.

Start-Opponent-Challenge.cmd opens the new drill. Keepers, scoring, full team AI and rules remain pending.



## Previous checkpoint: 0.3



A two-player practice drill now provides movement, dribbling, receiving, passing and shooting.

Nineteen more original routines and the packed 203-byte player layout are restored. Receiving is

assisted; steering and kick controls are native practice adapters. Full match play remains unfinished.

See [player recovery and limitations](PLAYER-RECOVERY.md).



Eight CTest suites pass. The player movement comparison covers 25 scenarios / 500 steps against

preserved compiled code. The native UI was captured and inspected. Start-Player-Practice.cmd opens the drill.



## Previous checkpoint: 0.2



The original free-ball subsystem now runs natively: flight, friction, bounce, spin, aftertouch,

and zone lookup. An interactive physics view is available with key 3 or Start-Ball-Physics.cmd.

18 original physics/math routine bodies compile with MSVC. Missing constants and 61 engine fields

were recovered from the preserved objects, alongside an inventory of 440 scalar globals.

See [recovery evidence and comparison limits](ENGINE-RECOVERY.md).



Six CTest suites pass. Five 80-step trajectories agree closely with emulated preserved code;

a sixth records a known floating-point bounce-boundary difference. A full match is still not playable.

The next target is player state/actions and ball interaction, followed by full team AI and match rules.



## Earlier checkpoint: 0.1



The first native Windows x64 preview builds with MSVC 19.44 and SDL3 3.4.18.

It reads the preserved assets directly and displays the original frontend artwork and tactics data.

This is **not yet a playable match**. No replacement football simulation is presented as original gameplay.



The original SOURCE directory is unchanged. The build produces an independent executable, with no

linkage to the old DOS OBJ/LIB files.



## Verified asset inventory



| Archive | Bytes | Records |

| --- | ---: | ---: |

| EURO.DAT | 12,346,272 | 121 |

| EUROREND.DAT | 6,613,404 | 229 |

| FAP.DAT | 1,452,928 | 27 |

| FAPF.DAT | 279,252 | 4 |



All ranges in all four offset tables fit within their data files.

All 15 frontend background records have exactly 640 x 480 indexed pixels.

The palette decoder applies the original shared first-16-color overlay from EURO_GRF.CPP.

All ten embedded 5600-byte tactics tables match their corresponding standalone TAC files byte-for-byte.



The tactics layout is 70 sets x 10 outfield players x 2 signed 32-bit coordinates, based on

INTELL.CPP's get_target signature and EURO_DSK.CPP's LoadTactics.

The preview displays raw target sets. The 1280 x 800 pitch drawing is a diagnostic coordinate frame

inferred from the data bounds; the missing global definitions must be recovered before asserting simulation parity.

Team-dependent palette overlays and original dynamic text/sprites are not yet reconstructed.



## Confirmed source gaps



The generated source-audit.json identifies 23 quoted include names not present in the snapshot,

after excluding known standard-library headers. This is an inventory, not 23 unconditional build failures:

some references belong to disabled hardware/region variants or auxiliary tools.



Important missing match-engine dependencies include:



- global.h: shared declarations and gameplay configuration.

- glob_var.h: match global-state definitions used by FOOTBALL.CPP.

- front.h: frontend/game interface dependencies.

- ball.dta: data included by ACTIONS.CPP.

- mouse.h and editor.h: legacy platform/editor interface.

- actrend.equ: resource identifiers included by the rendering source.



The linker explicitly consumes nine object files with no matching C/C++/ASM source in the tree:

mouse.obj, data.obj, render.obj, spritasm.obj, vesa.obj, audio.obj, random.obj, ndebug.obj, apiin.obj.

LIB.ZIP was inspected and contains libraries, not the missing source files.



Several apparent gaps are only path problems: eurofrnt/euro_int.h, CGL.H, SOS headers, and NetNOW headers

have copies elsewhere in the snapshot. The audit distinguishes these from genuinely absent filenames.



The preserved main application is a Euro 96 development frontend. TEST.CPP is its entry point, not a test suite.

Existing binaries and objects should not be assumed to match every loose source file.



## Original routines compiled



tools/extract_routines.py extracts the complete original bodies of straightcopy, spritecopy, flipsprcopy,

pseudostore, and calc_dist without editing their contents. These compile with the modern toolchain.

The native image decoder uses straightcopy; all five routines have behavior checks.

Known warnings in the unchanged original code are an unused variable and a double-to-float conversion.



## Verification performed



- Release x64 build on this Windows 11 host.

- Three CTest suites: real asset contracts, archive verification, and native rendering/event dispatch.

- 7,060 assertions covering original assets, all tactics coordinates, malformed offset tables,

  truncated tactics, missing files, original routines, and navigation boundaries.

- SDL dummy/software smoke check renders all 15 screens and switches views with injected keyboard events.

- Native Windows renderer captured for both artwork and tactics views; images visually inspected.

- Physical controller input and a complete original match are not verified.



A DOSBox-X 2026.10.01 probe of the preserved TEST.EXE initialized DOS/4GW and reached a 640 x 480

graphics mode. Emulator logs also recorded BIOS/network warnings. No interactive match was verified,

so this is not evidence that the original game is fully playable. The probe process was stopped.



This host blocked certain programs from writing directly to Documents. Selecting the working Python

interpreter resolved generated-file writes; using a temporary capture directory resolved image output.

No Windows security settings were changed.



## Match restoration milestones (partially complete)



1. Recover/reconstruct the remaining match headers, structures, and ball animation data.

   Ball physics constants and scalar initial values are now inventoried; full player state is still incomplete.

   Use the preserved object/debug information as evidence; do not invent constants silently.

2. Extend the isolated simulation target beyond the recovered BALL.CPP routines to

   BALLINT.CPP, ACTIONS.CPP, INTELL.CPP, and RULES.CPP.

   Replace OS/audio dependencies at explicit interfaces and document unsupported behavior.

3. Establish reproducible 20 Hz simulation scenarios, matching REAL_SPEED in DEFINES.H.

   Check restart behavior, possession, passing, shooting, goal detection, and match duration.

4. Connect the simulation to a native pitch renderer and keyboard/controller input.

   A complete local match is the first gameplay milestone.

5. Reconstruct the remaining renderer/animation path, original menu flows, commentary, and then networking.



No public repository, release, or deployment was created.





## 0.12.0 combined original-action batch

The original first-touch decisions, first-time pass/shot actions, shot state and
keeper save routines now run together in a repeatable 3D fixture. The stable
original-view launcher opens it. Space replays a catch; N moves the keeper to a
position where the shot beats him. This does not yet run the full original match
loop. See ACTION-RECOVERY.md for the native adapter boundaries and validation.
User testing is now grouped into five connected features per batch.
