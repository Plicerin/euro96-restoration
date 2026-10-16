# Historical README snapshots

These notes describe prior releases. Use [the completion checklist](COMPLETION-CHECKLIST.md) for current scope and remaining work.

## Retail names and extra-time speech — 0.63.0

All 320 players now have verified appearance mappings and spoken-name coverage (959 Euro tones plus one existing Actua fallback). Original score-specific extra-time announcements now play at the actual 90-minute knockout transition. [Evidence](docs/RETAIL-IDENTITY-AUDIO.md).

## All player previews resolved — 0.62.1

All 320 preserved roster entries now have their authored retail previews. DOS character decoding and independently verified player identities resolve the 23 previously unmatched names, including Neno. [Evidence](docs/PREVIEW-IDENTITIES.md).

## Retail previews and trophy films — 0.62.0

Original animated player and venue previews, plus the Euro 96 championship trophy and credits films with stereo audio. See [recovery and validation](docs/EURO96-PREVIEWS.md).

# UEFA Euro 96 — Windows restoration

**Target: the PC/DOS UEFA Euro 96 game.** The preserved Euro 96 gameplay engine, squads and frontend are the foundation. Remaining Actua Soccer media is temporary compatibility material, not the intended identity of the finished restoration. [Current asset provenance and replacement priorities](docs/EURO96-TARGET.md).

Use `../Play-Euro96.cmd`; the existing `Play-Actua96.cmd` shortcut still works. Save locations, replay formats and the internal executable name remain compatible.

## Kickoff selection fix — 0.61.1

Kickoff now selects the centre taker before the first gameplay tick. When the opponent kicks off, the nearest active outfield defender is selected. This applies to both local controllers and every change of ends. The original automatic centre pass is preserved, and live auto-selection resumes on its first eligible tick. [Reproduction and validation](docs/KICKOFF-SELECTION.md).

## Euro 96 quarter-final speech — 0.61.0

All four original quarter-final introductions now play in original-draw tournaments. The retail qualifier table proves that the bracket pairings match the preserved engine, including two reversed home/away listings. Each match plays its complete STT recording without an appended compatibility ending. [Evidence and boundaries](docs/EURO96-SEQUENCES.md).

## Euro 96 tournament speech — 0.60.0

Original-draw group introductions now follow the retail fixture lookup. Semi-finals and the final link the original opening to the correct countries. Championship, shootout and extra-time winners use their country-specific retail calls; decisive regulation knockout results use FLL. Quarter-final introductions remain on compatibility audio pending bracket mapping. [Evidence and boundaries](docs/EURO96-SEQUENCES.md).

## Euro 96 period commentary — 0.59.0

Half-time and regulation results now select verified Euro 96 period recordings: score-specific HLF/FLL takes and the separate END bank for draws. Missing or ambiguous scores and knockout endings retain generic compatibility calls. [Evidence and remaining work](docs/EURO96-PERIODS.md).

## Euro 96 match audio — 0.58.0

Thirty match-commentary categories now use verified Euro 96 recordings, alongside all 132 retail score-announcement takes. Pitch/crowd audio comes from the Euro 96 disc with its original foul-reaction level. [Evidence, compatibility and remaining audio work](docs/EURO96-AUDIO.md).

## Goalkeeper recovery and match panels — 0.57.1

Human keepers complete ground pickups and get-up animations before waiting in the original standing hold pose. R3/F6 match panels use recovered Euro 96 artwork instead of the development tabs and sidebar. [Fix details and validation](docs/KEEPER-PANEL-FIX.md).

## Euro 96 stadiums — 0.57.0

Match setup offers all eight retail Euro 96 stadiums, with Wembley as the default. Matched meshes, textures, palettes, pitches, sky and tunnel positions come from the verified Euro96 disc. Existing saves and replays retain their venue IDs. [Recovery, compatibility and validation](docs/EURO96-STADIUMS.md).

## Euro 96 startup — 0.56.0

Normal launches enter the Euro 96 frontend directly. The Actua intro/making-of entries and bundled films are removed from the Euro 96 release, along with unused Actua title images. Existing local preservation files and explicit film diagnostics remain. [Evidence and validation](docs/EURO96-STARTUP.md).

The release notes below describe earlier states.

## Euro96 player and audio presentation - 0.55.0

Retail heads, skin remaps and goalkeeper artwork now join the recovered kits. The menus use the Euro96 disc track; all 16 countries have English introductions. Match setup uses the original menu artwork, and pauses/entrances/exits hide leftover development chrome. [Evidence and remaining media](docs/EURO96-PRESENTATION.md).

## Clean original film presentation - 0.54.2

Removed the restoration playback toolbar from the intro and making-of film. Both show the complete 4:3 image at full available height; pause, seek and skip controls still work. [Details](docs/CD-INTRO.md).

## Original shirt numbers 1–20 - 0.54.1

Recovered Euro96 light/dark number artwork now appears on both teams, keepers and substitutes, including shirts 16–20. The usual launcher picks up the update on its next start. [Artwork, rendering and validation](docs/SHIRT-NUMBERS.md).

## Euro96 CD kits and player names - 0.54.0

All 32 retail outfield strips now render, including Croatia and Turkey, with country-specific alternate kits. Exact-name matches expand English player-name commentary to 308 of the existing 320 players. Preserved squads and saves remain compatible. [Evidence, rendering boundaries and combined review](docs/EURO96-IMPORT.md).

## Presentation fidelity - 0.53.0

Individual substitution/sending-off views now respect the original stadium roof and crowd while keeping the tunnel opening visible. Team selection animates through the source country-carousel poses. [Scope, source boundaries and validation](docs/PRESENTATION-RELEASE.md).

## Original menu presentation - 0.52.0

The normal frontend now uses the recovered Euro 96 fonts, animated buttons, country graphics, formation pitch, group tables and knockout backgrounds. Pause, substitutions, options and replay controls share the original artwork. Keyboard, controller and clickable menu navigation remain available. [Screens, source boundaries and combined review](docs/MENU-FIDELITY.md).
## Tunnel exit occlusion - 0.51.2

At half-time and full-time, players now pass behind the original stadium roof and crowd as they enter the tunnel. The original routes and exit timing are retained. [Rendering correction and validation](docs/TUNNEL-OCCLUSION.md).

## Player switching controls - 0.51.1

LB and RB now switch players during human matches; RB no longer silently hands an exhibition to the CPU. Pause identifies automatic selection or CPU spectator mode. The original selection rules remain intact. [Investigation and validation](docs/AUTOMATIC-SELECTION.md).

## Complete menu fade sequence - 0.51.0

Frontend page changes now fade the retained outgoing page to black before fading in the destination. Buffered input cannot activate controls before that destination has been drawn, and settled text keeps its normal rendering at fractional window scales. [Source curve, controls and validation](docs/MENU-TRANSITIONS.md).

## Original presentation finish - 0.50.0

Frontend screens now fade up using the original 25-step linear curve, and all three officials remain visible with their original period-end behavior while the players leave. Source review corrected the earlier assumption of missing referee tunnel walks; winner-board clips remain unassigned without a verified match trigger. [Source evidence and native boundaries](docs/PRESENTATION-FINISH.md).

## Local head-to-head - 0.49.0

Two controllers can now play opposing teams in the same exhibition. Choose **H / West (Xbox X): 2 PLAYERS** at match setup. Player selection, button charges, passes, shots, restarts and shootout controls are independent; controller assignments stay with countries across changing ends. Disconnects pause until a replacement joins. [Controls, boundaries and combined validation](docs/HEAD-TO-HEAD.md).

## Original first-time passing - 0.48.0

Hold X + Space, or controller West + South (Xbox X + A), for a first-time pass on an incoming loose ball. A narrow correction enables the existing Euro 96 chip and header branches; the original engine still selects a reachable contact and eligible receiver. [Scope, controls and source correction](docs/FIRST-TIME-PASSING.md).

## Saved replays - 0.47.0

Save the original replay buffer with S / East, then reopen clips through Saved Replays (F10 at the title or while paused; L3 at the title, East while paused). Playback retains its five cameras, speed/seek controls and safe return to the live match. Native files live beside tournament saves. A reserve-number rendering crash for shirts 16-20 is also fixed, using the original body without unsupported number artwork. [Combined batch, controls and boundaries](docs/SAVED-REPLAYS.md).

## Original player walk-offs and referee transitions - 0.46.0

Substitutions now show the outgoing player reaching the tunnel and the replacement returning to the touchline. Sent-off players leave without a replacement. These use the original routes, movement and player on/off camera on a presentation copy. Referees and assistants now move toward their original restart targets, with corrected referee facing. [Combined batch, controls and source boundaries](docs/WALK-OFFS.md).

## Original pre-match lineup - 0.45.0

Matches now open with the preserved team lines, referee/assistant animations and original orbiting camera. The nine-second sequence leads into the prepared kickoff without advancing the live match. Enter / South skips; P / Start pauses. [Five-part playtest, source evidence and limits](docs/PRE-MATCH.md).

## Original automatic player selection - 0.44.0

Human team play now uses the preserved automatic selection rules for defending, loose balls and intended receivers. Nearby players retain control to avoid constant switching; E / East / LB still selects manually. Movement carries through an automatic handoff, and goalkeeper distribution retains its dedicated controls. [One five-case playtest and source boundaries](docs/AUTOMATIC-SELECTION.md).

## Match Back-button fix - 0.43.2

Xbox Back / View no longer replaces a live or paused match with the old demonstration sequence. Start still pauses/resumes; Back still closes menus and replays. The usual launcher uses the fix on its next launch; running games are left alone.

## Tournament stabilization - 0.43.1

A complete 31-match frontend session now verifies natural half/full-time exits, goal replays, pause/hub return and exact checkpoint reloads. Resume recovers a valid backup when the main save is damaged or missing; saving recovered progress no longer replaces the good backup with a damaged file. [Current release status and final playtest](docs/RELEASE-CANDIDATE.md).

## Original startup intro - 0.43.0

The complete original 33-second CD intro and soundtrack now play before the title screen. Enter / Space / South skips; P / Start pauses. Replay it with I / North at the title. All 399 frames are recovered, including 77 valid early-ended delta frames rejected by the previous decoder. [Recovery, controls and validation](docs/CD-INTRO.md).

## Original tunnel exits - 0.42.0

Players now leave through the original stadium tunnel at half-time and full-time before the team-talk/results screen. Original routing, movement and motion capture run on a presentation copy that leaves the live match frozen. Enter / South skips; P / Start pauses. [Controls, source evidence and limits](docs/TUNNEL-EXITS.md).

## Original French commentary - 0.41.0

Options -> Audio -> Voice Language now selects the original English or French CD recordings, including match events, introductions, scores and matched player names. Language selection persists with existing settings; old preferences remain readable. [Controls, source evidence and remaining limits](docs/FRENCH-COMMENTARY.md).

## Original CD making-of film - 0.40.0

Press M / LB at the title to watch the complete original four-minute making-of film with its CD soundtrack. Native playback supports pause, seeking, restart and return; the usual launcher carries the imported media. The short startup intro is also restored as of 0.43.0. [Controls, asset evidence and limits](docs/CD-MOVIE.md).

## Replay controls - 0.39.0

Recent-play replays now support the original five moving speeds, exact frame stepping, timeline/one-second seeking and optional looping. A compact keyboard/gamepad panel shows playback and camera state; manual playback ends paused at the beginning, while automatic goal replays return to the match. Use F4, or Start then West on a pad. [Controls, source behavior and native additions](docs/REPLAY-CONTROLS.md).

## Original human ball controls - 0.38.0

The original shot/pass decisions and stationary button combinations now receive held keyboard/gamepad input in full-team play. This batch adds powered drives and passes, chips/high passes and crossing, first-touch strike/control requests, working aftertouch and a compact charge cue with P / Start help. Review all five together through the usual launcher. See [controls, source evidence and remaining limits](docs/HUMAN-CONTROLS.md).

## Release audit - 0.37.1

The native single-player/tournament loop now has an additional 32-fixture matrix across all countries and formations, and an original-action dispatcher comparison. Portable packages use fresh folders, required-file/hash validation, a mandatory SDL license and hidden checks outside the repository. The quickstart is current. See the [assessment and concrete finish list](docs/RELEASE-AUDIT.md); original human-input completeness, remaining presentation/assets and independent Windows validation are still open.

## Match options and saved preferences - 0.37.0

Original wind strengths, offside/foul/booking switches and referee accuracy/strictness are now configurable. Music, pitch effects, crowd and commentary have separate levels. Press O / RB from the menus, then Y / North to switch pages. Preferences save separately from tournament progress; match options apply next kickoff and audio changes immediately. [Controls and original behavior](docs/MATCH-SETTINGS.md).

## Tactics and match management - 0.36.0

Saved country formations, original position diagrams, half-time tactics, preferred set-piece takers and player attributes are available together. Press T / West at the lineup or half-time screen, then Y / North to change pages. Existing tournament saves remain supported. [Controls and original-code boundaries](docs/TACTICS-MANAGEMENT.md).

## Tournament continuity - 0.35.0

Tournament squads now retain player fitness, injuries, bookings and served suspensions. The pre-match screen supports selecting all 20 players and the four-player match bench. Existing saves migrate without losing results. [Controls, original-code evidence and explicit native policies](docs/TOURNAMENT-CONTINUITY.md).

# Restoration history and development reference

Native Windows x64 restoration of the preserved source snapshot at
[the original repository](https://github.com/videogamepreservation/actuasoccer96).

**Version 0.34.1 fixes tournament saves blocked by Windows Controlled Folder Access. Saves now use SDL's per-user application-data folder, with a production-directory write/reload test run by the game executable. Existing valid legacy checkpoints can be loaded without deleting the old file.**

The version notes below record historical checkpoints. For current capabilities and limits, use [the release status](docs/RELEASE-CANDIDATE.md) and [quickstart](docs/QUICKSTART.txt).

**Version 0.34.0 adds a complete 31-match Euro tournament: country or spectator entry, original fixtures and group ranking, knockout progression, and automatic saves between matches. T / West on the title screen starts; L / R3 resumes. See [tournament restoration and controls](docs/TOURNAMENT-RECOVERY.md).**

**Version 0.33.0 adds five selectable original CD venues, original corner flags, match-event notices, expanded half/full-time statistics and matched original player-name commentary. Review these together through the same launcher. See [the batch and fidelity boundaries](docs/PRESENTATION-BATCH-33.md).**

**Version 0.32.1 fixes invisible alternate-appearance players by restoring the original extended texture pages and skin/kit remaps. New rendered-pixel checks cover both teams, mirrored poses, all starting rosters and both half bindings, including the separate original goalkeeper kit. See [the fix and comparison](docs/PLAYER-MATERIAL-FIX.md).**

**Version 0.32.0 restores stadium event animations, CD kit-clash selections and alternate kits, matched player appearance flags, two additional original replay views, and original crowd-mood decisions. Test these together through the same launcher; V / R3 changes replay cameras. See [the five-feature batch and fidelity limits](docs/PRESENTATION-BATCH-32.md).**

**Version 0.31.1 replaces the red-banded Euro stadium with the complete original CD high-detail England stadium: matched meshes, textures, material coordinates, pitch and sky. This uses the CD stadium artwork, not a recolour of the old model. See [stadium recovery](docs/STADIUM-RECOVERY.md).**

**Version 0.31.0 restores six additional original CD national-team kits, the original textured pitch and sky, country-specific crowd palettes, and near-camera goal-net clipping. Fourteen countries now have matching original kits; Croatia and Turkey retain labelled substitutes. Use the same launcher on your next launch. See [graphics recovery](docs/GRAPHICS-ENVIRONMENT-RECOVERY.md).**

**Version 0.30.0 restores the original goal-type commentary and 33 spoken-score recordings. Headers, woodwork rebounds, keeper errors/deflections, and powerful/curling shots use their original classification. Scores play after celebrations. F9 adjusts commentary; F8 mutes all sound. See [commentary recovery](docs/COMMENTARY-RECOVERY.md).**

**Version 0.28.0 adds the original CD title and menu music, team selection, starting-eleven/formation previews, and half/full-time summaries with original result music. The same launcher opens this flow: Enter continues, then Enter watches or Y plays, followed by the pre-match screen. These are original assets in native SDL navigation. See [front-end recovery](docs/FRONTEND-RECOVERY.md).**

**Version 0.27.0 connects the original CD's match sounds: ball contacts, referee whistles, crowd ambience and goal/save/miss/card reactions. F8 mutes sound; pause and focus loss silence playback. Use the same launcher on your next launch. See [original audio](docs/AUDIO-RECOVERY.md).**

**Version 0.26.0 restores the original radar, team/ball sprites, halftime marker identity, match font and on-pitch score/clock. Matches use a larger view; F6 / R3 toggles panels, Tab / F7 toggles radar, and P / Start shows help. See [original presentation](docs/PRESENTATION-RECOVERY.md).**

**Version 0.25.2 extends the readable HUD to live matches: larger score/clock, possession-specific controls, keeper delivery prompts, compact CPU stats and a pause reference panel. Press P / Start for the additional controls.**

**Version 0.25.1 restores the original animated on-pitch flight/ground dots for aiming, adds large power/height bars, and reduces set-piece sidebar text. Use the same launcher on your next launch.**

**Version 0.25.0 connects original human controls for match penalties, free kicks, corners, throws, goal kicks and keeper deliveries. F5 cycles practice situations for one batch test. See [set-piece controls](docs/SET-PIECE-CONTROLS.md).**

**Version 0.24.1 fixes the penalty goal-line freeze: ball/net physics and keeper recovery continue through the result interval, with reset deferred until the next kick.**

**Version 0.24.0 adds manual shootout penalties, a live aim/height guide, original directional keeper saves, keyboard/gamepad controls and safe pause/focus handling. Choose Penalties only, then Y/North to play. See [penalty controls](docs/PENALTY-CONTROL.md).**

**Version 0.23.0 adds knockout extra time, optional golden goal, original CPU penalty shootouts, and a Penalties only format. Choose Format in match setup. See [knockout recovery](docs/KNOCKOUT-RECOVERY.md) for controls and provenance.**

**Version 0.22.0 adds original cards, visible officials, fatigue, substitutions, and goal celebrations with replays. F3 opens the squad; F4 replays recent play. See [match depth and controls](docs/DEPTH-RECOVERY.md) for the five-feature batch and original/native boundaries.**

Version 0.21.1 fixes disappearing halfway-line segments by restoring original ground-marking clipping.

**Version 0.21.0 adds original national-team selection, names and simulation ratings, independent formations, live stadium surroundings, and original match cameras. The same launcher now opens match setup. Choose teams and tactics, then Enter to watch or Y to play. See [national-team restoration](docs/NATIONAL-RECOVERY.md) for the five-feature batch and asset limitations.**

Development now prioritizes restoring the original engine. The two-versus-two practice
mode is retained as a regression tool; its new AI and rules are not the restoration target.
Open `View-Original-Actua96.cmd` beside this project, or select **5 Original 3D**.
Press **B** to watch the CPU control both teams. The camera follows the ball;
the panel shows each team's pass and shot attempts. **B** takes control without
resetting play. Goals and balls out select the original restart; home set pieces wait for your input. **R** resets the score
and field while retaining CPU mode. See [live match recovery](docs/MATCH-RECOVERY.md)
for the original routines and native restart transition boundaries.
Press **Y** for a regulation match or **T** for two-player passing: **WASD/arrows** move, **Space** passes,
**C** chips, **X** shoots in the direction you face (or tackles without possession in team play), **E** switches players,
**P** pauses and **R** resets. Control transfers on reception. **Q** returns to
replays. Gamepads are supported; see [team recovery and controls](docs/TEAM-RECOVERY.md) and [two-player controls](docs/CONTROL-RECOVERY.md).
The legacy keyboard Q preview shortcut opens the combined attack sequence: build-up pass, first-time chip,
first-time finish, keeper catch/get-up, carry, throw and teammate reception.
**Space** or **R** replays it; **N** shows
an alternative keeper position where the shot beats him; **M** retains pass/control.
**P** pauses, **Up/Down** rotates the camera, **V** switches stadium/close-up,
and **Left/Right** browses the recovered motion captures. Test this batch once,
using the same launcher; no extracting a new executable.
**G** plays the new post / bar / side-net / roof-net / goal batch; **H** tests
the opposite end. Each case advances automatically. See [goal restoration scope](docs/GOAL-RECOVERY.md).

![Original scene with restored kits, markings, goals and shadows](docs/original-scene.png)
![Recovered stadium and pitch geometry](docs/original-stadium.png)

This view uses original data and original transform/polygon assembly. SDL replaces the
DOS drawing backend. The action sequence is a controlled integration fixture, not a complete match: player
roster, initial kick, active actors, outlet/retreat runs, reception input and replay limits are supplied by the fixture; first-touch decisions and the selected action routines are original.
See [action recovery scope](docs/ACTION-RECOVERY.md) and [scene recovery scope](docs/SCENE-RECOVERY.md) for provenance and remaining work.
The user-provided CD now supplies `DIGI/PITCH/PITCH.MAD`: all 20 original pitch
sample names match, and all disc audio passed PCM validation. Match effects now
play through SDL; English event commentary, matched player-name speech and stadium event video are connected. Full-screen GDV movies remain pending. See
[CD inspection](docs/CD-INSPECTION.md) and [audio recovery](docs/AUDIO-RECOVERY.md).

The original `SOURCE/` snapshot is unchanged. New C++20/SDL3 code is under `native/`.

## Local playtesting

Open `../View-Original-Actua96.cmd` for the latest original-engine build, continue past the title, then Enter / South to watch or Y / North to play; confirm at the pre-match screen. Use this same launcher after each update;
no ZIP extraction or installation is needed. Reopening it closes only the game previously started
by this launcher and starts the latest snapshot, resetting the drill. Other running copies stay open.

`build-windows.ps1` publishes a new snapshot only after its test suite and source audit pass.
Snapshots live beside the launcher in `Actua96-Playtest/builds/`, with a `current.json` pointer.
The running game uses its own snapshot, so it does not lock the next build. Assets are shared from
`SOURCE/`; keep the project and playtest folders together. This is a local launch/restart workflow,
not live state-preserving hot reload. ZIPs are for explicit releases, not feature-by-feature testing.
Use `build-windows.ps1 -Package` when a portable package is needed.

## Portable releases

Run `package-windows.ps1`, then open `Start-Actua96.cmd` in the fresh `dist/actua96-<version>-<id>` folder it prints. The script checks the compiled version and package contents; it never overlays an older package. Run `verify_package.ps1 -Smoke` inside that package for isolated hidden checks. See [current controls](docs/QUICKSTART.txt).
`Start-Opponent-Challenge.cmd` retains the two-versus-two goal challenge.
`Start-Player-Practice.cmd` opens the approved passing drill; **O** toggles opponents and resets.
Keep `SDL3.dll` and `data/` beside the executable. The application does not use an emulator.

### Historical diagnostic controls

The following notes describe the earlier practice fixtures, not the current full-match controls or completeness.

- Arrows or WASD: move the selected player.
- Space: ground pass to the teammate; control switches to the receiver. Keep your direction held to run as soon as they receive.
- C: lofted pass. Click the pitch to aim, then X to shoot.
- Hold X without possession: keep tackling while chasing the ball carrier. A pale blue ring shows active reach.
- O: toggle opponents and reset. E: switch player. R: new game (clears scores). P: pause. F11: fullscreen. Escape: exit.
- 1 / 2 / 3 / 4 or Tab selects artwork, tactics, ball physics, or player practice.

Opponent games last three minutes of active play. Pausing, leaving the game view,
losing window focus, and goal/out-of-play restarts stop the clock. Keeper possession
counts as active play. At full time, the score, shots and saves remain visible;
press R, controller Start/Options, or click Rematch to play again. Solo practice is untimed.

Blue attacks the right goal. Each team has two outfield players and an automatic keeper.
Keepers track play, catch manageable shots, parry harder ones, and release to a teammate
after one second. Follow rebounds; aim away from the keeper to score. Red carries forward
when unpressured and shoots from an open lane near the left goal.
Valid whole-ball crossings score; the conceding team automatically
kicks off after two seconds. Out-of-play balls restart after one second for the opposite last-touch
team. These simplified restarts preserve score and statistics; R clears the game.

Blue 2 and 3 are teammates; red 13 and 14 are opponents. A gold ring marks the selected player.
Reds close down the carrier, cover the passing lane, intercept, and dribble away after a turnover.
Red carriers commit to safe waypoints; support positions stay on a consistent side and
defenders swap pressing roles only when another is clearly closer. An orange ring warns of their tackle. Dodge or pass; chase while holding X to win possession back.
When red has the ball, your unselected teammate covers the passing lane. He pursues loose balls
and can intercept; collection transfers control to him. Switching players immediately gives you
manual control. With blue possession, your off-ball teammate seeks space away from markers and blocked passing lanes, keeping
useful passing distance. After the kick, the receiver automatically meets the pass until fresh
movement input or E takes control. Held movement takes over at possession without a release/repress. The ball does not home. The approved solo drill remains available
with opponents off.
Receiving is assisted, with more forgiving foot reach and long lobs tuned to descend at the receiver.
The ball does not home after release. Kick controls are new practice adapters. The original full receive/kick
animation chain and eleven-a-side match are still incomplete.

Gamepad support includes analog movement, right-stick aiming, automatic button prompts, and hot-plugging.

| Action | Xbox | PlayStation |
| --- | --- | --- |
| Move / aim shot | Left stick or D-pad / right stick | Same |
| Ground / lofted pass | A / Y | Cross / Triangle |
| Shoot; hold to tackle without the ball | X | Square |
| Switch player | B or LB | Circle or L1 |
| Pause / resume | Start / Menu | Options |
| New game (while paused) | Click right stick | R3 |
| Toggle opponents and reset | Back / View | Share / Create |
| Cycle views | RB | R1 |

The active controller disconnecting pauses play; reconnect and press Start/Options to resume.
Keyboard/mouse and controller prompts switch when used. Virtual Xbox and PlayStation checks pass;
physical-controller testing remains pending. Full controls, including the earlier physics view,
are in [the quick start](docs/QUICKSTART.txt).

![Native timed match with keepers](docs/keepers.png)

![Full-time result after an unattended idle-player match](docs/full-time.png)

## Historical recovery notes

The build extracts 17 unchanged function bodies from `BALL.CPP`, plus `calc_dist` from `MATHS.CPP`.
These implement launch angles, flight, gravity, friction, bounce, spin, aftertouch, rolling orientation,
possession gating, and ball-zone lookup. Function scope is adapted to a separate engine instance.
Player/action recovery now records 209 complete original routines and ten explicit excerpts/adapters,
covering the restored movement, first-touch, keeper distribution and control sequence.
The matching packed player layout is verified at compile time. See [player recovery](docs/PLAYER-RECOVERY.md).
The earlier four original image-copy routines also remain in use/test coverage.

Missing physics constants and state types are recovered from the preserved Watcom object files.
The Windows program does **not** link those objects; MSVC compiles the recovered C++ bodies.
The build pins the binary evidence by SHA-256. See [engine recovery](docs/ENGINE-RECOVERY.md).

The pitch display, steering, assisted receiving, opponent decisions, tackles, and practice pass/shot controls are new adapters.
Original functions perform movement, held-ball positioning and possession transitions. Goal drawings
now detect goals in opponent mode. Full match AI, original keeper animations, refereeing, and official restarts remain unfinished.
Opponents now pass under pressure, to a useful forward option, or to an open teammate after
a long carry. They check the lane and receiver space, with a cooldown between passes.
The receiver meets the fixed pass destination, and blue can intercept. Red now shoots from clear close-range chances; the keepers and shot decisions are native practice adapters.
The solo passing drill still pauses on an out ball. The practice adapter still treats
post/bar overlaps as misses; the original 3D action sequence now uses the restored
goal-frame and net collisions. Audio is not restored.

The artwork view displays 15 original backgrounds with VGA palettes. The tactics view inspects
10 original formations with 70 target sets each; it does not run match AI.

## Build on Windows x64

Requirements: Visual Studio 2022 Build Tools with Desktop C++, Windows SDK, CMake 3.24+, and Python 3.

```powershell
.\build-windows.ps1
```

The script downloads a SHA-256-verified SDL3 3.4.18 archive into `_deps/`, builds,
runs the CTest regression suites, audits the source, and updates the local playtest launcher.
Add `-Package` to also create a fresh portable package.
It does not install dependencies globally or publish anything.

Equivalent manual build:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DSDL3_DIR="<SDL3>/cmake" -DPython3_EXECUTABLE="<python.exe>"
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.\package-windows.ps1
```

## Verification

All 67 current CTest checks passed in the release audit, including the new 32-fixture match matrix. Separate package contracts and isolated runtime checks also passed. [Results and limits](docs/RELEASE-AUDIT.md).

The suite checks assets, malformed inputs, original routine behavior, source-restoration tooling,
native navigation/rendering, and keyboard-driven ball simulation. The simulation advances at 20 Hz
independently of display rate and pauses when its view loses focus or is hidden.

An additional comparison executes the preserved x86 ball/math code in Unicorn during development.
Five scenarios match native positions, velocities, and airborne state across 400 steps within 0.002
original pitch units; observed maximum difference is about 0.000123. A sixth scenario deliberately
records a floating-point boundary difference that changes bounce timing. This is **not bit-exact parity**.
See [the comparison report](docs/ball-comparison.json) for scope and hashes.
Unicorn is optional tooling; it is not needed to build normally or run the game.

```powershell
# In a separate Python tool environment with unicorn 2.1.4 installed:
python tools/compare_original_ball.py SOURCE build/Release/ball_tests.exe --report docs/ball-comparison.json
```

A second comparison checks 25 original player movement scenarios (500 steps), with positions,
velocities and the work counter agreeing within 0.002. See [player comparison](docs/player-comparison.json).
Player practice checks, 65 opponent checks, 31 teammate-support checks, and 32 scoring/restart checks cover passing, receiving, interception, tackles,
reaction windows, a moving chase/regain, support spacing, marked-player movement, moving reception,
manual override, touchline play, and solo-mode preservation. Native SDL event/render tests
exercise both drills, including X tackles, the opponents toggle, and reset.
58 virtual-controller checks cover analog movement, aiming, passing, chasing/tackling, button labels,
controller selection, disconnects and reconnects. A separate SDL window test exercises controller
movement held through pass reception, an aimed shot, pause/reset, mode switching, and disconnect pause.
Opponent-motion regressions cover sustained dribbles, steering reversals, stable defensive roles,
and support runs across midfield. Seventeen opponent-passing checks cover moving reception,
pressure outlets, blocked lanes, interception, cooldowns, pause/reset, solo isolation and actual
AI exchanges. The rendered opponent smoke test also checks a red pass and completion.
Keeper/attack checks cover saves at both ends, catch/release, physical rebounds, recovery,
contact order, high/wide misses, red goals and blue kickoffs, blocked shooting lanes, and pause/reset.
The native window smoke includes a catch, keeper release, red shot/goal, blue kickoff,
full-time result and keyboard/mouse rematch. Timed-match checks verify final-tick goals,
clock pauses, protected result state, and reset. Six complete unattended games cover
18 active match minutes with idle, chasing, passing, shooting and switching inputs;
they check finite state, valid possession, timely keeper releases and bounded restarts.
Virtual-controller checks include Start rematches.
Defensive teammate checks cover both selected players, lane coverage, loose-ball recovery,
control switching, pause/restarts and solo-mode preservation. Receiver-flow regressions cover ground/lob reception in both modes, eight directions, partial/full
stick movement, immediate return passes, neutral input, and steering changes during flight.

The graphics test renders all 4,983 original animation frames, all 16 kits, the original
wireframe and the stadium. The player tests also exercise all 132 recovered contact offsets.

Useful command-line modes: `--original-3d`, `--stadium`, `--graphics-smoke`, `--opponents`, `--opponents-smoke`, `--gamepad-smoke`, `--practice`, `--practice-smoke`, `--physics`, `--tactics`, `--verify`, `--smoke-test`, `--physics-smoke`,
`--capture path.bmp`, `--result-capture path.bmp`, and `--data PATH`.
Result capture runs an idle-human match to completion before saving the actual result screen. Captures with `--physics` show a deterministic curved shot.

## Remaining restoration

The [current finish list](docs/RELEASE-CANDIDATE.md) tracks remaining original-game parity and hardware validation. Local head-to-head exhibitions, original human controls, CD movies, tunnel presentation, saved replays, French audio and tournament player-condition carryover are implemented. Same-team co-op, multiplayer tournaments and network play remain separate scope. The native renderer, menus, scheduling, saves and audio driver are explicitly described adapters around original source/assets.

Upstream snapshot: `01a824099a278043a9e4a6dfd55ea980dd4ce8bb`.
Original sources/assets retain their original attribution and terms. SDL has its separate license.
