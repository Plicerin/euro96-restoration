## Film presentation update - 0.54.2

The intro and making-of film now fill the available height at their original 4:3 aspect ratio, with black side margins. The restoration playback toolbar and timer have been removed; the complete original frame, including lower captions, remains visible. Keyboard/gamepad pause, seek, restart and skip controls are unchanged.

# Original startup intro - 0.43.0

The complete `VIDEO/VFULL_S.GDV` film from the supplied Actua Soccer CD now plays before the title screen: **399 frames, 320x240, 12 fps, 33.25 seconds**, with all **732,963 bytes** of its original 22,050 Hz unsigned 8-bit mono PCM. It includes the Actua Sports, Gremlin and Actua Soccer sequences. This is original Actua Soccer CD media, not a claimed Euro 96-specific intro.

This batch covers five connected areas: complete video recovery, original soundtrack, startup/title integration, keyboard/mouse/controller controls, and delivery through the existing launcher and portable package.

| Action | Keyboard / mouse | Gamepad |
| --- | --- | --- |
| Skip to title | Enter, Space, Escape or left click | South, East or Back |
| Pause / resume | P | Start |
| Replay intro from title | I or Intro button | North |
| Seek five seconds | Left / Right | LB / RB |
| Restart current film | R | North |
| Making-of film from title | M or Making Of button | LB |

Focus loss and active-controller removal pause the film; focus return does not resume automatically. Sound follows the saved Music level and Sound setting; F8 toggles mute. Menu music stays silent during films and returns afterward. Skipping does not also advance past the title. F11 still changes fullscreen. A missing or invalid optional intro falls back to the title so it cannot prevent starting a game. No preference or tournament save is changed by watching the intro. Automated menu captures bypass startup; an explicit `--frontend-capture intro` captures the film.

## Why the earlier decoder failed

The CD file contains every expected frame and audio record. Its 395 method-2 image records include **77 valid early end tags**, which leave the remaining pixels unchanged from the preceding frame. The installed FFmpeg decoder recognizes the tag but then rejects a frame unless its output cursor spans the entire image. That accounts for all 77 failures. The other records are one palette reset and three intentional unchanged frames.

The original format research by [Mike Melanson and Vladimir Gneushev](https://multimedia.cx/gdv-format.txt) describes method 2's explicit termination tag and unchanged runs. The [FFmpeg decoder implementation](https://github.com/FFmpeg/FFmpeg/blob/master/libavcodec/gdv.c) was inspected to locate the rejecting end condition. The old document's back-reference nibble ordering differs from the working decoder; this importer uses the ordering validated against FFmpeg and the supplied images. No FFmpeg implementation is copied into or linked with the game.

`tools/import_cd_intro.py` reads the original records with a bounded format reader. In a temporary verification copy only, it replaces early-end commands with equivalent explicit unchanged-pixel runs, followed by termination. It does not synthesize whole frames, remove frames, stretch timing, alter palettes or resample audio. The original ISO remains untouched. Unsupported coding modes, truncated operands, runs outside the image, wrong metadata and incomplete records are rejected.

The importer independently reconstructs every original frame, then requires **byte-for-byte agreement for all 399 PAL8 frames** with strict FFmpeg decoding of the equivalent full-span stream. It also compares every original PCM byte. The result is packed as lossless PNG records in `VIDEO/INTRO.AMV`, using the existing bounded native SDL player. FFmpeg and 7-Zip are import tools only; neither is a runtime dependency. [Hashes, early-end frame indices, sample pixel hashes and verification](cd-intro-recovery.json).

## Boundaries

The preserved `SOURCE/EURO_GDV.CPP` delegates playback to its DOS animation library. Native SDL provides the playback, controls and title handoff here. The authored film and sound are restored; this is not a claim that DOS decoding, fades, hardware modes or frontend scheduling are reproduced exactly. The 15-bit/high-colour `S` variants remain unsupported; the complete VGA intro and making-of film are available. Pre-match entrances, referee transitions and individual player walk-offs remain separate work.

## Validation

Native tests decode every intro PNG and compare five independently verified RGBA hashes, including a formerly rejected early-ended frame and the palette reset. Tests cover original PCM queuing, mute, pause, seek bounds, focus, natural completion, repeated entry and malformed headers. Synthetic compressed-stream tests cover unchanged tails, overlapping back-references, preamble references, truncated operands, unterminated input and out-of-range runs.

The hidden executable test enters through the startup path, checks keyboard and virtual-controller replay/skip/pause, same-frame focus loss and return, controller disconnection, natural completion, mouse skip, and switching to the making-of film. Portable verification runs the same check from an isolated package with no repository media overrides. Automated sound tests use SDL's dummy audio device; real-device listening remains part of playtesting.

Screenshots: [intro playback](intro-intro.png), [title and replay button](intro-title.png).

The [full regression](intro-regression.txt) passed all **77 tests** in 207.59 seconds. The [portable package](intro-package.txt) verified all 36 file hashes and passed all six isolated checks, including both films. The [published save probe](intro-published.txt) confirms the tournament hash and preferences are unchanged. SOURCE is unchanged.

Published version **0.43.0**, snapshot `666902bc46b05104-2b4b128d-0703f4d7-4e7d91ab`, through the existing `Play-Actua96.cmd`. No running game was restarted.
