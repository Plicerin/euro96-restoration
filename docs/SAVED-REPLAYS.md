# Saved replays - 0.47.0

The original 15-second replay buffer can now be saved and reopened after quitting the game. Recording, motion capture, ball presentation and the five camera calculations remain the existing restored replay path. File persistence, library navigation and failure handling are native additions; no standalone replay-file implementation was found in the preserved source.

## One combined playtest

1. During a match, open recent play with **F4**, or pause with **Start** and press **West / X**. In the replay, press **S** or **East / B** to save. `REPLAY SAVED` confirms success. Each save creates a separate clip; existing clips are never replaced.
2. Return to the match. While paused, press **F10** or **East / B** to open Saved Replays. It is also available from the title using **F10** or **L3**, and with F10 on the result screen. The title has a clickable Saved Replays button.
3. Use arrows / D-pad or the mouse wheel to select a clip. **Enter / South** plays it. The mouse can select a row and use Play; PgUp/PgDn pages through longer lists. **R / North** refreshes the list.
4. Use the existing replay controls: pause, five speeds, frame stepping, timeline seek, loop and five cameras. **Space / Escape / South / Back** returns to the library. Focus loss and controller disconnect pause playback; returning focus does not resume it.
5. **Escape / Back** returns from the library to its originating screen. A match stays paused at the same moment. Quit and reopen the game, then open the same saved clip from the title to check persistence.

Clips are stored beside tournament saves in `%APPDATA%\Actua96 Restoration\Actua96\replays`, with `.a96r` extensions and UTC timestamps. Copy these files between installations with the same recovered assets. They contain visual state, not graphics/audio assets, a video stream, a playable match checkpoint or a DOS-compatible save. Replays remain silent. Stadium video-board animation is not recorded; saved playback uses an idle board. A clip requiring unavailable venue media reports an error.

## Preservation and isolation

`3D_UPD2.CPP` records/interpolates player and ball coordinates; `FOOTBALL.CPP` supplies the 15-second, 20 Hz buffer. `3DENG.C::replay` supplies the five moving playback speeds and manual rewind/pause ending. The existing native replay adapter supplies frame stepping, scrubbing, interpolation and looping. This release persists that adapter's visual records without rerunning football AI or physics.

Saved playback owns a separate scene. The live match object and its recording are retained, including RNG, score, squad, ball, pending substitutions, clock and tournament state. On return, the original scene, venue, camera, pause state and recording are restored. The action-release guard prevents held replay exit buttons becoming a kick.

Schema 1 uses explicit little-endian 32-bit integers and IEEE float fields; it never dumps a C++ engine structure. A header records teams, score, venue, camera, wind and the replay camera's stationary-ball state. Each of 2-300 frames contains the 22 players' visual identities, poses and animation fields, three officials, ball position/orientation/velocity, ownership and half. Exact extent, a 1 MiB read limit, field/index/finite-number checks, loaded animation availability and a checksum are validated before entering the scene. The checksum detects corruption, not authenticity. Original animation phases can accumulate beyond one cycle; their finite fractional part is sampled by the existing renderer.

Windows writes use a uniquely created temporary file, complete write and flush, then a rename that does not replace an existing file. A failed save retains previous clips and the current viewer. Incomplete temporary files are ignored. Damaged entries are labelled and fail in the library without replacing the live match. The library is bounded to 1,000 clips.

## Reserve-number crash found during validation

The tournament supports reserve identities 16-20, while the recovered renderer has number textures only for 1-15. Drawing a valid reserve previously threw `Shirt outside preserved number atlas`. Those identities now use the original `render3d` no-number face count (12 body faces), preserving their model, kit, identity and stats. This is an explicit fallback: their back-number artwork remains missing. Source files and original number textures are unchanged.

## Verification

The five-group contract covers immutable unique files, loading into a fresh application instance, exact rendered parity across all five cameras and at an interpolated subframe, live engine retention, venue/wind/half/shirt/card/keeper metadata, malformed inputs, failed writes, empty/mixed libraries and navigation. The executable smoke drives the actual SDL keyboard and virtual-controller routes, including focus loss/disconnect. Screenshots are reviewed alongside automated comparisons.

[Focused results](replay-file-focused.txt). Final regression, package and Gaiking results are recorded in [release status](RELEASE-CANDIDATE.md).

Reviewed Gaiking captures: [library](replay-file-library.png), [saved match frame](replay-file-frame.png).
