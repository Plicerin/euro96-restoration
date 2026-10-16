# Retail result music — 0.65.0

When the Euro theme is installed, post-match results now return to `EURO96.WAV`, regardless of win, loss, draw or shootout outcome. Moving from results to another frontend page continues the same song. Match play still stops music, trophy films retain their own audio, and the existing volume, commentary ducking, focus and mute controls apply.

## Retail evidence

The previous audit identified the theme loader but missed its main-loop caller. The expanded trace establishes the complete return path:

1. The frontend calls `PlayMatch` at file `0x77449`, targeting `0x97450`.
2. Before entering the match engine, `PlayMatch` stops music at `0x980ea` and clears its state at `0x980ef`.
3. The engine returns to `PlayMatch`, which updates results and returns at `0x9898d`. The ordinary exhibition branch selects the main menu; championships select their appropriate next page or trophy film.
4. The frontend back edge at `0x774bb` rejoins its loop at `0x76f4a`. Existing pages join the common update path, whose call at `0x770be` reaches the music handler at `0x8837c`.
5. With cleared music state, that handler passes the `EURO96` track name to the loader at `0x883ca`. At the end of playback it clears the state again, allowing the next loop to restart the theme. Trophy-page initialization explicitly stops music before film playback.

The [offline verifier](../tools/verify_retail_result_music.py) pins the supplied executable hash, 17 instruction anchors, the track string and the direct loader-call inventory in the inspected frontend/engine range. [Trace report](result-music-retail.json). Addresses are file offsets, not loaded runtime addresses. The earlier [partial trace](identity-audio-unresolved.json) remains historical evidence.

This establishes frontend music routing. The native post-match statistics page, its timing, audio gain/fades and ducking remain adapters; exact DOS mixing or frame timing is not claimed.

## Compatibility

The audio adapter resolves outcome requests to the Euro theme before checking whether a track has changed. Repeated frame-loop requests therefore preserve playback position, and results-to-menu navigation does not restart the song. Without `EURO96.WAV`, the existing Actua win/loss/draw stings remain available and play once. Missing media remains silent. No media file or save format changed.

## Stadium-board finding

The three preserved `XWNF`, `XWNG` and `XWNS` animations each have an opening and a looping continuation. Their pixels show a plain **WINNERS** treatment and gold/silver trophy treatments. They belong to the preserved 27-record FAP archive. The supplied Euro disc instead has a 16-record FAP archive and different animation handlers; none of those nine preserved animation/palette records has an exact retail record match.

That difference does not prove there are no equivalent retail animations. It does mean the old three-clip list cannot be treated as three missing Euro triggers. No new victory trigger was invented. Full retail board decoding and event routing remain a separate fidelity gap; the existing board adapter continues unchanged. [Archive hashes/comparison](result-music-retail.json), [inspected preserved frames](winner-board-archive-review.png).

## Validation

Focused checks compare all three result requests with the Euro theme across the entire track and a loop boundary, sample the decoded original WAV directly, retain playback through return to menus, and cover legacy one-shot stings, absent media, zero volume and dummy-device focus handling. Frontend contracts cover loss/draw/win, CPU spectators and shootout outcome requests. The native executable smoke checks the selected result track through its actual frontend loop and mute transition.

These are automated PCM/device checks. Listening through physical speakers and changing the Windows output device remain part of the combined human validation.

All [105 regression checks](result-music-regression.txt) passed in 222.44 seconds. The three [focused checks](result-music-focused.txt) also [passed on Gaiking](result-music-gaiking.json). The [package](result-music-package.txt) passed 53 file hashes and nine isolated smoke checks. [Published identity and retention evidence](result-music-release.json).
