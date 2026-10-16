# Euro96 retail presentation batch — 0.55.0

## Integrated

- Retail default/alternate heads, exposed skin and goalkeeper/referee texture pages. The importer checks the original and retail body UV tables, installs the 180 changed head coordinates and applies the executable's medium/dark skin remaps. The original meshes and gameplay remain in use; this does not import the individual animated menu portraits into live players.
- All 32 English country introduction phrases, including Croatia and Turkey, using the disc's three-letter country banks. TTX/TTY filenames map to the existing home/away introduction roles. The existing welcome-ending clips and event scheduler are retained; the complete retail welcome dispatcher has not been independently recovered.
- The disc's 22050 Hz stereo menu track, without resampling or replacement music. Existing result music remains in use.
- F2 match setup now uses recovered menu backgrounds/fonts. Pausing retains the clean match view behind its restored menu. Generic help/development strips are hidden during normal entrances, exits and frontend flows. Diagnostic views remain available explicitly.

## Evidence and boundaries

`tools/import_euro96_presentation.py` verifies the input executable, archives and audio banks against pinned SHA-256 hashes. It checks the 32 retail team descriptors, executable instruction anchors, atlas dimensions and compatible body UV ranges. It creates optional overlays; it does not modify SOURCE or saved rosters.

[Import provenance](euro96-presentation-import.json) records all output and individual country-clip hashes. Skin indices 32–63 are reduced by six for the retail medium-tone country flag, or twelve for alternate dark appearances, clamped at 32. Default medium-tone head hair indices 5–31 are reduced by four. Matched player appearance flags from the earlier kit import still select the alternate heads.

[Render contact sheet](euro-presentation-players.png): rows England, France, Italy and Turkey; columns keeper front/back and two starting outfield players front/back. These are nearest-neighbour enlargements of native renderer captures, not replacement artwork.

## Remaining media audit

[Disc inventory](euro96-presentation-inventory.json) lists 174 FAP animations (including eight stadium previews), 32 home/away-labelled GDV trophy sequences and one menu WAV. The WAV is now integrated. FAP previews and GDV trophy sequences still require decoder/screen/event verification; no filename-only assignment was made. They do not imply eight additional playable stadiums.

The retail E_MCX bank contains 24 additional phrases. Their sequence mapping is unverified, so the existing known welcome endings remain. This release is still a restoration combining preserved Euro96 code and verified Actua Soccer/Euro96 media, not a claim of complete retail presentation parity.

## Validation

The new retail contract checks malformed/absent media, heads and bodies across selected starters/keepers, mirrored poses and both halves, all 256 country-introduction pairings, and mixer PCM against independently extracted Croatia, Turkey and menu samples. Existing material tests cover all 16 teams, 22 starting slots and both half material bindings. Existing menu tests now also capture both ends of the match-setup navigation.

All 93 regression checks passed (535.15 seconds). The isolated portable package passed all eight smoke checks and all 42 file hashes, with no optional media absent. The importer reproduced identical output bytes on a second run. Visual inspection covered default, medium and dark appearances, goalkeeper front/back views, match setup and the paused game.

[Regression results](euro-presentation-regression.txt), [portable package checks](euro-presentation-package.txt), [save retention](euro-presentation-save-retention.json) and [427 unchanged SOURCE files](euro-presentation-source-retention.json) record the release checks. Use the usual Play-Actua96 launcher on the next start; publishing does not restart an active match.
