# Original artwork and music front end — 0.28.0

The native SDL front end now connects five presentation pieces: the CD title,
original menu music, team selection, a starting-eleven/formation preview, and
half-time/full-time summaries with result music. It keeps the original 4:3
artwork, nearest-neighbour scaling and recovered proportional HUD font. Dark
panels keep controls readable over the detailed backgrounds.

## Controls and flow

Open the same `View-Original-Actua96.cmd` or `Play-Actua96.cmd` on your next launch.
Publishing a snapshot does not close or restart a running match.

- Title: Enter / South continues. Escape / Back exits.
- Team selection: Up/Down chooses a row; Left/Right changes it. D-pad equivalents
  are supported. Choose teams, formations, match length, camera and match format.
- Enter / South chooses CPU versus CPU; Y / North plays as the home team.
- Pre-match: Left/Right or Tab switches the displayed team. Enter / South kicks
  off; Escape / Back returns to selection. Starting elevens use the preserved
  Euro 96 roster. The diagram uses original centre-zone tactical targets, with
  a native goalkeeper marker; it is a preview, not a live positioning simulation.
- Half-time: Enter / South continues a human match. CPU matches continue after
  eight focused seconds, or immediately on confirmation. The original period
  transition still changes ends and resumes the correct restart.
- Full time: R / R3 reviews a rematch; Enter / South returns to team selection.
- F2 / L3 opens selection from a match. F2 / L3 again or Escape / Back returns to the paused or
  active match as it was. Clicking the visible buttons is also supported.
- F8 mutes both music and effects. Losing focus silences both; menu music resumes
  from its playback position. P / Start retains the existing live-match pause.

## Original assets and native decisions

The supplied Actua Soccer disc has a different front end from the preserved
Euro 96 snapshot. This build intentionally displays the original Actua Soccer
CD title and the Euro 96 setup backgrounds that match the restored roster.
It does not replace the preserved data archives or claim that this mixed flow
reproduces the original DOS menu implementation.

| Asset | Source and evidence |
| --- | --- |
| Actua Soccer title | CD `DATA.DAT` record 8, 640×480 indexed pixels |
| Title palette | CD `DATA.DAT` record 15, 768-byte VGA palette |
| Team selection | Preserved `EURO` screen 3 |
| Pre-match background | Preserved `EURO` screen 1 |
| Interval/result background | Preserved `EURO` screen 7 |
| Font | Recovered `EUROREND` HUD font, as in the match overlay |
| Formation preview | Preserved tactics tables, centre ball-zone row 20 |
| Music | CD `DIGI/MUSIC/MENU.WAV`, `WIN.WAV`, `LOSE.WAV`, `DRAW.WAV` |

The title/palette pairing was visually checked against alternative palettes;
its loader association has not been proved from the original executable.
The importer validates archive bounds, image/palette sizes and 6-bit palette
range. `frontend-assets.json` records every imported asset's size and SHA-256,
plus the source archive hashes. The CD identity is recorded in `CD-INSPECTION.md`.

All four music files are original unsigned 8-bit stereo PCM at 22,050 Hz.
SDL receives float stereo at the same rate, with native half-scale gain and
50 ms ramps to soften starts, endings and menu-loop edges. MENU loops over title,
selection, pre-match and half-time; live play has no music. Result stings play
once. Human-home wins/losses choose WIN/LOSE; draws choose DRAW. Spectator games
use WIN for either winner. Shootout scores select the shootout result.
These screen-to-track choices are native associations based on the asset names,
not a recovered original music scheduler. Pitch/referee sounds have their own
mixer; period-whistle tails can finish beneath the summary screen.

Navigation, panels, input handling, screen transitions and the eight-second CPU
interval are native C++20/SDL3 code. Scores, rosters, formations and statistics
come from the restored match. The original `SOURCE/` tree is unchanged.
Commentary, videos, a complete original menu implementation and the CD's separate
team database remain future work.

## Import, publication and validation

Run `python tools/import_cd_presentation.py PATH_TO_ISO`. It extracts only the named data/music,
without running an installer, and refuses to overwrite different local assets.
The optional CD assets remain under ignored `local-data/`; without them the title
falls back to the preserved Euro artwork and unavailable music is silent.

Publication copies the six media assets into the immutable playtest snapshot,
includes their aggregate content hash in the build ID, and records per-file
hashes in the manifest. The launcher verifies these before running. Portable
packages have a `Start-Actua96.cmd` entry point and the same optional media.

`original_frontend_contracts` covers team/formation navigation, human/CPU mode,
pre-match confirmation, cancel/resume, half-time transitions, result selection,
mouse navigation, music decoding, bounded output, one-shot endings and focus
handling. `native_frontend_smoke` exercises the integrated window event loop,
virtual gamepad, match transitions and mute toggle with dummy SDL drivers.
Rendered title, selection, lineup, interval and result states are reviewed as a
batch. The full regression result is saved in `frontend-test-results.txt`.
