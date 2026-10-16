# Original CD making-of film - 0.40.0

**Updated in 0.43.0:** the separate startup intro is now restored. See [CD-INTRO.md](CD-INTRO.md). The decoder limitation below records the original 0.40.0 investigation. French commentary and player tunnel exits have also since shipped.

The title screen now offers **M / LB - Making Of**. This plays the complete original `VIDEO/VWHOLE_S.GDV` film from the supplied Actua Soccer CD: 2,869 frames at 12 fps, 320x240, approximately 3 minutes 59 seconds, with its original 22,050 Hz unsigned 8-bit mono soundtrack.

The batch covers original video recovery, soundtrack playback, title-screen integration, keyboard/gamepad playback controls, and verified delivery through the existing launcher/portable package. It is an original CD extra; it is not new gameplay or a replacement intro.

| Action | Keyboard | Gamepad |
| --- | --- | --- |
| Open from title | M | LB |
| Pause/resume | P | Start |
| Seek back/forward five seconds | Left/Right | LB/RB |
| Restart film | R | North |
| Return to title | Space, Enter or Escape | South, East or Back |
| Fullscreen | F11 | Keyboard F11 |
| Mute | F8 | Set Sound in title options |

Movie volume follows the existing Music level. Menu music is silenced while the film is open and resumes on return. Focus loss or active-controller disconnection pauses playback; returning focus does not automatically resume it. Skipping, seeking and closing clear queued movie audio. The video remains 4:3 with nearest-neighbour scaling, and controls sit below the complete image. No movie is forced on startup.

## Recovery and evidence

`tools/import_cd_movie.py` extracts the original file with 7-Zip and decodes it with the installed FFmpeg **during import only**. It rejects decoder errors, missing/extra frames, and unexpected audio/video metadata. The decoded indexed frames are stored losslessly as PNG records; the original PCM audio is copied without resampling. `VIDEO/MAKING.AMV` contains bounded frame offsets plus PCM. The player reads one frame at a time, retaining only the small index, soundtrack and current texture; it does not decompress the entire film into RAM.

FFmpeg and 7-Zip are not runtime dependencies and are not bundled. SDL's PNG/audio APIs perform playback. No external player, network service or mounted ISO is needed after import. [Hashes, decoder version, frame count and independent sample checks](cd-movie-recovery.json). [FFmpeg's GDV demuxer](https://www.ffmpeg.org/doxygen/8.0/libavformat_2gdv_8c_source.html) and [decoder](https://www.ffmpeg.org/doxygen/7.1/libavcodec_2gdv_8c_source.html) were inspected while investigating the disc variants.

The original Euro wrapper `SOURCE/EURO_GDV.CPP` delegates playback to the unavailable DOS animation library (`InitAnim`, `StartAnim`, `CloseAnim`); `SOURCE/CGDV.H` also describes the original DOS player interface. This is a native playback boundary for original media, not recovered DOS decoding code. SOURCE and the ISO remain unchanged.

## Explicit remaining limits

The four disc filenames represent short/long and VGA/high-colour variants, not four verified distinct movies. `VFULL_S.GDV`, the 33-second introduction, produces decode errors with the available decoder. The `S` high-colour variants do not decode reliably through its palette-only path. They are not imported, substituted, silently frame-dropped or advertised as working. Full startup-intro restoration remains open. The complete VGA making-of film decodes with zero errors and is the only movie exposed in this batch. French commentary and tunnel sequences are also still separate work.

## Validation

All 2,869 imported PNG frames are decoded by the native test. Five full-frame RGBA hashes at independently selected points match the importer. Tests cover the exact soundtrack size, SDL audio queuing, mute, pause, seek bounds, focus loss/return, completion/re-entry and malformed header rejection. A hidden executable test sends actual SDL keyboard and virtual-gamepad events from title to film and back. Portable verification now runs that movie test whenever the movie is included, from an isolated copy outside the repository.

The [full regression](movie-regression.txt) passed all 71 tests. After the final image/control-strip spacing adjustment, the [three affected checks](movie-final-tests.txt) passed again. The [portable package](movie-package.txt) passed all five isolated checks, including movie playback. The [published save probe](movie-published.txt) confirms the existing tournament save hash and preferences remain unchanged. Audio checks use SDL's dummy device; listening and lip-sync on real hardware remain a manual check.

Published snapshot: `9af5519fc878d4ea-2b4b128d-0703f4d7-dbdb9748`, version **0.40.0**, through the existing `Play-Actua96.cmd`. No interactive game was restarted. Screens: [title](cd-title.png), [movie and controls](cd-movie.png).
