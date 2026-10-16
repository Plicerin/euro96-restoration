# Update — 0.62.1

All 320 player previews are now mapped. The 23 exclusions below describe the historical 0.62.0 release; they were naming/encoding mismatches, not missing animations. [Identity verification](PREVIEW-IDENTITIES.md). The pack now contains 109 unique animations/palette pairs, checked against 327 frame samples.

# Retail previews and trophy films — 0.62.0

The supplied Euro 96 PC disc now supplies animated player statistics previews, all eight venue previews, and the championship trophy/credits films. No generated images, replacement actors or replacement audio are used.

## What is restored

- Player statistics: 297 of the preserved 320 player identities have a unique, same-country exact-name match in the retail script. Their authored animation and palette are used, including goalkeeper palette borrowing. Twenty-three entries remain unavailable; they do not inherit the previous player's animation. The 98 distinct player animation/palette pairs and eight stadium animations total 106 imported previews. Several retail players deliberately share an animation.
- Exhibition venue selection: the eight `stadium1` through `stadium8` animations map through the retail script to London, Birmingham, Leeds, Newcastle, Manchester, Liverpool, Sheffield and Nottingham (native venue IDs 5–12).
- Championship: all 16 home and 16 alternate-kit trophy/credits films are imported. On final completion, a simulated tournament opens the film; a watched/played final opens it after leaving the full-time result. The champion and the final's kit clash selection determine the film. A completed championship offers **TROPHY FILM** again, including after loading a save. Skipping or replaying a film does not record another result or advance the match.
- Films preserve all 983 frames at 12 fps and the original unsigned 8-bit stereo PCM at 22050 Hz. Pause, focus loss, mute, seek, skip and natural completion use the existing in-game movie controls. PNG frames and PCM are lossless; identical blobs share storage across films. FFmpeg is needed only during import.

## Recovery evidence

The pinned executable SHA-256 is `2a473a002a294c1bb28ea39e94a36afc34497dd7e44f0ebfeda0ea21f652b62d`. Addresses below are executable file offsets; data addresses use base `0x12f854`.

- FAP header read at `0x8dbe2` reads 256 little-endian frame lengths. Routines `0x9e60c` and `0x9e73f` implement initial literal/repeat frames and subsequent literal/skip or literal/repeat frames. The native and Python decoders enforce table, run, dimension and output bounds.
- Player animation IDs are country × 20 + roster entry (`0x8a294`); the script explicitly names each FAP and palette. Palette-symbol parsing starts at `0x7b7ba`; the selected palette byte is read at `0x90a43`. The sixteen palette-record selectors at data `0x7d60` map to records 169–184, with Croatia/Czech ordering explicitly preserved. Palette colours start at index 159. Base screen palette is record 83; global colours are record 77.
- Stadium animation IDs start at 344 (`0x83081`). The eight palette selectors at data `0x7d40` map to records 161–168, loaded at colour index 112 (`0x90af8`). Base screen palette is record 86; using the player-page base produces visibly incorrect colours even though the stadium palette is correct.
- Film names come from the 32-pointer executable table at file `0x1376a4` / data `0x7e50`, not inferred country filename abbreviations. The decoded films contain the trophy presentation followed by original credits.

The preview playback increment is `.34` in the retail executable. This port uses 60 frontend ticks per second (20.4 preview frames per second). The exact DOS wall-clock cadence is not verified. The native venue placement, statistics-page navigation and championship trigger are adaptations to the restored frontend, not a claim of a byte-identical retail frontend state machine. SOURCE gameplay and the preserved roster stay intact.

Full source hashes, mappings, unmatched identities and independent decoded-frame samples: [preview inventory](euro96-previews.json), [trophy inventory](euro96-trophy.json). Importers: `tools/import_euro_previews.py`, `tools/import_euro_trophy.py`.

## Validation

The native test decodes every imported FAP and all 31,456 film frames, compares 318 indexed-preview and 96 RGBA-film hashes with the independent importers, and exercises malformed media, looping, stereo audio, focus, pause, mute, seek, end, menu animation and championship replay. Captures and release results are recorded alongside this document after validation. No physical-controller session is claimed.

Twenty-four affected checks and four final rechecks passed. The published package verified 53 file hashes and nine isolated smoke checks. [Release evidence](retail-presentation-release.json). Rendered captures: [venue](retail-venue.png), [player](retail-player.png), [championship](retail-champion.png), [England trophy film](retail-england-trophy.png). The user save and all 427 SOURCE files are unchanged.
