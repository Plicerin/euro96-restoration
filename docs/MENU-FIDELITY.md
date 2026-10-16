Historical release record. For current completion, compatibility boundaries and test gates, use [COMPLETION-CHECKLIST.md](COMPLETION-CHECKLIST.md).

# Original frontend presentation — 0.52.0

Update: [0.53.0](PRESENTATION-RELEASE.md) adds source-based country-carousel movement. The inventory below records the original menu-art replacement batch.

The normal game frontend now uses the Euro 96 menu assets shipped with the preserved source: three proportional fonts, the original button animation frames, country carousel images, squad/formation graphics, screen backgrounds and final trophy. The prior plain panels and in-match HUD font have been replaced throughout the frontend, pause reference, substitutions and replay transport.

## Screens covered

- Title: selectable Exhibition, Championship, Resume, Saved Replays, Options, Introduction, Making Of and Quit entries. Arrows/D-pad choose; Enter/A confirms. Existing direct shortcuts remain available.
- Exhibition: country artwork, team settings, CPU watch, human play and local head-to-head.
- Lineup and tactics: perspective formation pitch, original numbered balls, all 20 squad names, formation selection, set-piece takers and player attributes. Tournament fitness, bans, bookings, swaps and opposition editing restrictions remain available.
- Championship: all four original group tables, two fixtures per group per round, and separate quarterfinal, semifinal and final screens. Fixtures cycles the three group rounds (plus automatic current-round selection); Rounds opens the knockouts. Previous/Next Page or Left/Right retains access to earlier results. Next, Sim and Auto retain the existing tournament behavior.
- Half-time and full-time: original statistics background, match figures, score, result and continuation/tactics/rematch controls.
- Options and saved replay library: the same recovered fonts, buttons and source-derived panel border, with the existing native settings and replay files.
- Pause, substitutions and replay transport: the same visual treatment, while retaining current controls, automatic-selection status, bench availability, cards, pace, queued-change notices and the replay seek target.
- On-pitch presentation notices: original frontend typography and panel artwork for opening lineups, exits and individual walk-offs.

Mouse targets are registered alongside their rendered controls. Hidden destinations retain the existing fade/input guard. The small cyan Back/Options labels at the foot of pages are clickable. The third group round and every knockout remain viewable after the championship finishes.

## What is original, and what is adapted

`tools/restore_menu_art.py` reads the 1,159 image rectangles from `SOURCE/SCRIPT.96`, checks symbol offsets against `EURO_SYM.H`, resolves the texture-page reuse in `EURO_GEN.CPP`, and recovers the 64 country-carousel poses from `EURO_VAR.CPP`. `EURO.DAT`/`EURO.OFF` supply the original indexed pixels and VGA palettes at runtime. Fonts retain the original glyph widths plus one-pixel advance and palette ramps. Squad views restore the source team/formation palette overlays. Knockout names/results use the original two-column positions in `Fixtures` and `DisplayFinalRoundFixtures`.

The provided source is the Euro 96 frontend. Its main artwork now makes the menu set consistent with that frontend; the imported Actua Soccer CD intro, making-of film, music, commentary and stadiums remain available. This is not a claim that the source and CD are the same retail edition.

Navigation, hit targets, settings storage, local two-controller setup, replay-file library, pause help and the composition of extension pages remain native Windows code. The navy panel border is reused from the original player-statistics screen, with its baked white exterior masked for overlays and its center extended. Country changes select the corresponding original carousel pose immediately. The original 25-step screen fade curve and native timing adaptation remain as documented in [MENU-TRANSITIONS.md](MENU-TRANSITIONS.md). This is an original-art frontend restoration, not a byte-for-byte port of the entire DOS frontend event loop or every unused retail screen.

One source rectangle, carousel sprite 754, requests 71 rows starting at row 410 of a 480-row page. Recovery keeps the 70 available rows instead of reading beyond the page. The generated recovery report records that correction; SOURCE is untouched.

The developer-only physics/practice/workbench screens retain their diagnostic interface. No replacement gameplay has been introduced. Online/modem setup, unused original menu branches and exact DOS menu timing remain outside this update.

## Validation and one combined review

`menu_art_tests` checks original header and glyph pixels, real mouse targets, kickoff ownership and redraw/state isolation, and captures all frontend pages plus the live overlays. Existing transition tests still check the retained outgoing page, black handoff, buffered keyboard/controller/pointer input, focus, renderer reset and scaled rendering. Animated pages are compared at the same animation time.

Use the usual Play-Actua96 launcher on the next start. One combined review is enough: navigate the title/settings, choose teams and inspect tactics, start a match and view pause/substitutions/replay, then resume a championship and browse its groups/rounds. Publishing does not restart the game already running.


## Final evidence

The [full 89-check run](menu-fidelity-regression.txt) passed 84 checks; the five obsolete input/animation fixtures were corrected. All [13 affected checks](menu-fidelity-final-checks.txt) then passed on the final executable. All [eight isolated package checks](menu-fidelity-package.txt), 36 hashes and the [normal launcher verification](menu-fidelity-launcher-check.txt) passed. Build/package/published [executable hashes match](menu-fidelity-hashes.json), and [SOURCE](menu-fidelity-source-retention.json) and the [user tournament save](menu-fidelity-save-retention.json) are unchanged.

Visually reviewed: [overview](menu-fidelity-overview.png), [team selection](menu-fidelity-selection.png), [player attributes](menu-fidelity-tactics-player.png), [set-piece takers](menu-fidelity-tactics-takers.png), [quarterfinals](menu-fidelity-quarterfinals.png), [champion](menu-fidelity-champion.png), [simulation](menu-fidelity-simulating.png), [results](menu-fidelity-full-time.png), [sound options](menu-fidelity-options-sound.png), [substitutions](menu-fidelity-substitutions.png), [pause](menu-fidelity-pause-controller.png), [replay transport](menu-fidelity-replay-controls.png) and [saved replay library](menu-fidelity-saved-replays.png).

The large next-match mascots use their original full-screen palette, with the home sprite mirrored as in `EURO_GEN.CPP`; applying the small squad mascot's palette overlay to them corrupts their faces and kit colors. The final source-pixel contract covers this distinction.
