# Original French commentary - 0.41.0

Open **Options (O / RB)**, switch to **Audio (Y / North)**, select **Voice Language**, then use Left/Right or D-pad to choose English or French. Clicking the row also changes it. The preference saves separately from the tournament and applies immediately. The interface and making-of film are not translated.

This batch connects five related parts: original French match-event speech, linked country introductions, exact spoken scores, matched player names with three intonations, and persistent language selection with keyboard/gamepad/mouse support.

## Original recordings and boundaries

The supplied CD contains separate `DIGI/PHRASES/F` and `DIGI/TEAMS/F` MAD banks. Import follows the category IDs, fixed half/full-time phrase numbers, larger-digit-first score selection, country codes and X/Y/Z name selectors already recovered from the pinned `AUDIO.H`, `AUDIO.OBJ` and CD `SOCCER.EXE`. It does not synthesize, translate or substitute speech. WAV bytes remain the original 11,025 Hz unsigned 8-bit mono PCM.

The French event pack contains 443 clips from 52 banks: 364 event/period recordings, 18 introduction endings, 28 country introduction segments and 33 score recordings. The name pack contains 543 recordings for the same 181 exact country/initial/surname roster matches used by English commentary. [Phrase and bank hashes](french-commentary-recovery.json), [player matching and clip hashes](french-names-recovery.json).

French `MCA01` is absent on this disc, so the introduction selects from `MCA02` through `MCA19`. Both ambiguous `GLA30` records are excluded, as in English. Missing numbered phrases are not invented. Croatia/Turkey introductions and unmatched player names remain silent. Changing language never borrows an English player name for a missing French name.

The native mixer and priority/cooldown scheduler are shared by both languages. Loading a new pack is staged before replacing the active one. A successful switch clears old queued speech, its repetition history and cooldowns, preserves volume/mute, and retains match observation so it does not replay kickoff. A missing phrase pack or malformed input keeps the working voice and reports that the requested voice is unavailable. A missing optional name pack leaves names silent. Match RNG and simulation do not change.

Preferences now use version 2. Version 1 remains readable with English as the default; existing match/audio values and valid backups survive migration. Tournament save format is unchanged. Both new media packs are included in the stable launcher and portable package inventories.

## Validation

The original commentary contracts run separately in English and French, including every imported event category, all 48 supported home/away score combinations, period announcements, welcome assembly and a complete CPU match. Additional checks compare score and player-name PCM with independent samples read directly from the CD, exercise repeated switching and missing/corrupt data, preserve mute/focus behavior, and verify settings migration and all three menu input methods. The actual executable also switches to French by keyboard and back to English with a virtual gamepad.

Automated audio uses SDL's dummy device. Real speaker/headphone listening remains part of the combined human validation, not something the automated checks establish.

Both complete CPU commentary runs finish **1:0 at 7,328 ticks**. English plays 58 phrases and French 57; differing original recording lengths affect the native speech queue but do not alter simulation. Re-importing English with the generalized importers reproduced both existing English packs byte-for-byte. [Audio Options screenshot](french-options.png).

All **73 regression tests passed** in 209.89 seconds, including the release match matrix. [Focused checks](language-focused.txt), [full regression](language-regression.txt). The [portable package](language-package.txt) has 35 inventoried files and passed all five isolated checks; its front-end check requires the French/English switching marker when French media is included. A final CMake adjustment restricts the optional language tests to installations containing their required packs; reconfiguration confirmed both tests remain registered and the tested executable hash is unchanged.

Published **0.41.0**, snapshot `5d109b4147d898b5-2b4b128d-0703f4d7-8b1ab7f2`, through the existing launcher. The [published save/preferences probe](language-published.txt) passed with the user's tournament hash unchanged and preferences still absent. SOURCE and the supplied ISO were not modified. No interactive game was restarted.

## Still open

This completes French language selection, not all original presentation. The short GDV intro has decoder errors; tunnel transitions, exact DOS menu transitions and win/boost stadium-board connections remain unfinished. Matching Euro assets and independent Windows-machine validation are also still needed for the broader restoration.
