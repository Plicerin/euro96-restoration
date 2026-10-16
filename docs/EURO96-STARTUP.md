Historical release record. For current completion, compatibility boundaries and test gates, use [COMPLETION-CHECKLIST.md](COMPLETION-CHECKLIST.md).

# Euro 96 startup — 0.56.0

The normal game opens directly to the Euro 96 frontend and menu music. Its six entries are Exhibition, Championship, Resume, Saved Replays, Options and Quit. Keyboard/controller navigation and pointer targets use the same entry list, so removing the film entries does not leave invisible choices. The former I/M and controller North/LB film shortcuts do nothing at the normal title, including when old Actua films remain in a local media directory.

## Why there is no replacement film

The supplied Euro 96 disc inventory contains 32 GDV files, all country-specific HOUTS/AOUTS sequences, and no separate intro/making-of video. The retail executable's readable GDV filenames are the same 32 names. Earlier sample decoding identified trophy sequences. The Wireplay CLIENT/ANIMATE resources belong to the separate network client and are not evidence of a game intro.

The preserved Euro 96 `SOURCE/TEST.CPP` initializes `Menu=MAIN`, `LogicState=BLANK_SCREEN` and `ChangeMenu=-2`, then builds the frontend and fades its palette up. The old Actua film-discovery routine is commented out. `EURO_GDV.CPP` dispatches its player from the stadium-flythrough page, not the startup shown in this source.

These findings support removing the Actua startup added by the restoration. They do **not** prove every step or timing of the retail executable's startup, nor rule out animation stored inside an archive. The six-entry Windows navigation remains a native adaptation around original Euro 96 artwork. No replacement video was invented or a trophy video reassigned as an intro. [Inventory, filename offsets and source hashes](euro96-startup-evidence.json).

## Preservation and delivery

New packages and launcher snapshots exclude `VIDEO/INTRO.AMV`, `VIDEO/MAKING.AMV`, `FRONTEND/TITLE.IDX` and `FRONTEND/TITLE.PAL`. The normal frontend already reads its artwork from `SOURCE/EURO.DAT`; audio discovery still has the installed pitch bank. Original disc images, local imports, old snapshots and the bounded film decoder remain untouched.

Explicit `--intro-smoke`, `--movie-smoke` and film-capture diagnostics can still inspect the old locally imported films. They are not exposed through ordinary title navigation, and they do not restore automatic startup playback. Fresh Euro 96 package verification runs a direct-startup test instead of requiring the omitted Actua movies.

Save paths, squads, gameplay and the working old/new launcher aliases remain unchanged. Publishing does not restart a running match. The next asset replacement priority is the retail Euro 96 stadium set.

## Validation

The startup smoke check runs once with old Actua files present in development media and again without them in the isolated package. It verifies direct title entry, no film playback, inactive keyboard/controller film shortcuts, options navigation and controller entry to exhibition. Menu artwork tests verify keyboard wrap to Quit, the sixth mouse target, and the remaining frontend pages. Existing legacy film checks continue validating the preserved decoder separately.

All [six focused checks](euro-startup-focused.txt) and [12 frontend/tournament/replay checks](euro-startup-regression.txt) passed. The fresh portable package passed [39 hashes and seven isolated checks](euro-startup-package.txt), including startup with no legacy films. The prior 0.55.0 full gameplay regression remains the baseline; gameplay was not changed. [Reviewed title capture](euro-startup-title.png).

Version 0.56.0 is [published](euro-startup-published.txt), and the [launcher verifies](euro-startup-launcher.txt). Built, packaged and published executable hashes match; the tournament save and 427 SOURCE files are unchanged. All four omitted local imports remain intact. [Release evidence](euro-startup-release.json).
