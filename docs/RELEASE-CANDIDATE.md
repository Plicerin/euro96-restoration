# Current build — 0.70.0

The 105-minute interval uses the half-time stadium board; country selection follows the preserved frontend's nominal 30 ms cadence. Team selection remains immediate. No new retail period artwork is inferred, and audio, gameplay, media and save formats are unchanged. [Changes and evidence](PERIOD-PRESENTATION.md), [recovered cadence](period-menu-timing.json).

All **109 regression checks passed** in **577.30 seconds**. Five focused checks passed in **10.53 seconds**, including both board asset paths, all country transitions and render-rate independence. The current team-selection capture was visually reviewed. [Full suite](period-presentation-regression.txt), [focused checks](period-presentation-focused.txt), [capture](period-selection-moving.png).

The portable package passed **54 hashes and nine isolated smoke checks**. Version 0.70.0 is published for the usual launcher's next start; the running game was not restarted. SOURCE, imported media and user saves/preferences are unchanged. [Package](period-presentation-package-smoke.txt), [launcher](period-presentation-launcher-verify.txt), [release identity](period-presentation-release.json). Physical head-to-head is deferred; audible device-switching validation remains pending.

# Previous build — 0.69.0

Short and long pass reactions now share the verified Euro CPA bank and cooldown. The two special set-piece reception branches stay silent, matching their retail jumps past the commentary call. This resolves the former two live pass-category gaps. Older English and French retain their original routes. All imported media, preserved gameplay source and save formats are unchanged. [Branch evidence and scope](RETAIL-PASS-AUDIO.md).

All **108 regression checks passed** in **266.88 seconds**. Six focused checks passed in **53.73 seconds**, including all 17 CPA takes, twelve actual reception cases, shared cooldown, English/French changes and legacy fallback. The fallback fixture uses take 2 because the older disc lacks CPB01. [Full suite](pass-audio-regression.txt), [focused checks](pass-audio-focused.txt), [retail instructions](retail-pass-audio-evidence.json).

The portable package passed **54 hashes and nine isolated smoke checks**. Version 0.69.0 is published for the usual launcher's next start; no running game was restarted. SOURCE, imported media and user saves/preferences are unchanged. [Package](pass-audio-package-smoke.txt), [launcher](pass-audio-launcher-verify.txt), [release identity and retention](pass-audio-release.json). Disposable package-test copies were removed after verification, retaining their logs and original releases. Physical controller/audio validation remains pending.

# Previous build — 0.68.0

Post-release investigation: the [winner-board audit](WINNER-BOARD-AUDIT.md) now covers all 36,631 retail relocation records, direct call/tail-jump candidates and the two working goal-board callers. It finds no verified match-end winner trigger. Winner behavior is now labeled unverified / possibly dormant. Six audit-tool checks passed; no game binary, imported media or launcher version changed.

Eight verified Euro 96 recordings now supply commentary for goals scored after a rebound. Both retail goal branches establish the mapping; the other 919 records in the event overlay are byte-identical. Two previously listed audio gaps have no active direct source callers, leaving two live pass categories without verified retail replacements. [Evidence and remaining audio work](REBOUND-AUDIO.md), [current closeout](RESTORATION-CLOSEOUT.md).

All four focused audio/goal checks passed in **43.97 seconds**. The full **108-check** run initially passed 101 and failed seven while C: had about 55 MB free. Removing three disposable package-test copies recovered **1,809,273,690 bytes**, preserving logs, original packages and published builds. All seven failed checks then passed serially in **35.57 seconds**, without code changes. This is 108 checks with passing results after resource recovery, not a clean initial run. [Initial full run](rebound-audio-regression.txt), [seven-check recovery run](rebound-audio-resource-recheck.txt), [focused checks](rebound-audio-focused.txt).

The portable package passed **54 hashes and nine isolated smoke checks**. Version 0.68.0 is published for the usual launcher's next start; no running game was restarted. Preserved SOURCE and user saves/preferences remain unchanged. [Package checks](rebound-audio-package-smoke.txt), [launcher verification](rebound-audio-launcher-verify.txt), [release identity and retention](rebound-audio-release.json). Physical audio/device and two-controller validation remain pending.

# Previous build — 0.67.0

Frontend fades now use the source's nominal 63 Hz timer registration with the recovered 25 steps per phase. Statistics headings and scores have clearer spacing, and the interval panel correctly labels extra time instead of always saying HALF TIME. Gameplay and imported media are unchanged. [Changes, remaining fidelity work and physical-validation boundaries](RESTORATION-CLOSEOUT.md).

All **108 regression checks passed** in **209.13 seconds**; three focused frontend/menu checks passed in **10.68 seconds**. Tests cover the exact black handoff, refresh-independent completion, existing input guards and state isolation. Full-time and extra-time captures were reviewed, along with actual application interval/result captures. [Full suite](closing-fidelity-regression.txt), [focused checks](closing-fidelity-focused.txt), [source cadence](closing-fidelity-fade-source.json), [application captures](closing-app-captures.json).

The portable package passed **54 hashes and nine isolated smoke checks**. Version 0.67.0 is published and verified for the usual launcher's next start; no running game was restarted. [Package](closing-fidelity-package-smoke.txt), [launcher](closing-fidelity-launcher-verify.txt), [release identity and state retention](closing-fidelity-release.json).

The board re-audit reproduces the unsafe penalty write and limits the winner-routing claim to verified evidence. No guessed animation trigger or commentary mapping was added. [Audit](retail-board-limits.json). Exact retail parity and the outstanding physical controller/audio checks are not marked complete.

# Previous build — 0.66.0

Retail stadium-board streams now supply idle, fouls, goals, half-time branding and yellow/red cards on visible supported boards. The live power meter and compact match-help line use the recovered Euro menu font and panel styling. Exhibition/custom-draw welcomes follow the verified retail restrictions; original-draw tournament introductions remain. The meter layout and menu navigation remain native adaptations. [Scope, evidence and unresolved fidelity differences](FIDELITY-BATCH.md).

All **108 regression checks passed** in **232.43 seconds**, with **six focused checks** passing in **37.63 seconds**. After the menu capture fixture was updated to load retail media, its check passed again in **5.96 seconds**. The 18-page menu sheet and actual application power, pause and spectator captures were visually reviewed using software rendering. [Full suite](fidelity-batch-regression.txt), [focused checks](fidelity-batch-focused.txt), [menu recheck](fidelity-menu-final-check.txt), [menu sheet](fidelity-menu-audit.png), [application capture report](fidelity-app-captures.json).

The portable package passed **54 file hashes and nine isolated smoke checks**. Version 0.66.0 is published and verified for the usual launcher's next start; no running game was restarted. [Package](fidelity-batch-package.txt), [isolated checks](fidelity-batch-package-smoke.txt), [launcher](fidelity-batch-launcher-verify.txt), [release identity and retention](fidelity-batch-release.json). Physical controller/audio validation remains pending; this batch does not claim exact retail frontend timing or complete board/commentary parity.

# Previous build — 0.65.5

Removed the duplicate yellow-on-black booking/send-off strip from both clean and expanded match views. The restored event panel still supplies the card announcement, player name and whistle/card artwork. Referee logic, card timing, walkoffs and the pause menu are unchanged.

Five existing presentation, overlay, frontend, walkoff and match-discipline checks passed in 65.50 seconds, including six complete CPU matches. The event-panel captures were reviewed. [Check log](card-banner-checks.txt).

The portable package passed 53 file hashes and nine isolated smoke checks. Version 0.65.5 is published and verified for the usual launcher's next start; the running game was not restarted. [Package](card-banner-package.txt), [launcher](card-banner-launcher.txt).

# Previous build — 0.65.4

Normal matches, rematches and championship fixtures now receive fresh starting seeds instead of always resetting the original gameplay RNG to seed 1. Diagnostic fixtures remain deterministic; `--match-seed N` supports reproducing an opening and `--match-seed random` explicitly selects fresh seeds. Original referee severity, card thresholds and preferences are unchanged. Early straight reds remain possible under those original rules. [Behavior and scope](MATCH-RANDOMNESS.md).

All **107 regression checks passed** in **311.12 seconds**, including 24 fresh starts/rematches, exact 400-tick fixed-seed reproduction, real application capture comparisons, original discipline, complete championships, controls and replays. Repeated fixed-seed captures match exactly; different/fresh seeds change the opening. [Full log](match-randomness-regression.txt), [capture report](match-randomness-cli-report.json).

The portable package passed **53 hashes and nine isolated smoke checks**. Version 0.65.4 is published for the usual launcher's next start. No running game was restarted. SOURCE, media and user save/preferences retention are recorded in the [release evidence](match-randomness-release.json). [Package](match-randomness-package.txt), [launcher](match-randomness-launcher.txt).

# Previous build — 0.65.3

The original rotating menu selection circles now receive a light sprite-local coverage filter, smoothing both their rims and rotating detail. All twelve original frames, palette colours and animation timing remain in use. Inactive blue circles receive the same treatment. Source assets and menu text are unchanged. [Scope and all-frame visual comparison](SELECTION-CIRCLE.md).

Four existing menu/art/transition/frontend checks passed in 8.64 seconds. A hidden Direct3D title capture shows changes only within the circle column; all menu labels remain pixel-identical to 0.65.2. [Checks](selection-circle-checks.txt), [pixel comparison](selection-circle-pixel-check.json), [release evidence](selection-circle-release.json).

The portable package passed 53 file hashes and nine isolated smoke checks. Version 0.65.3 is available through the usual launcher's next start. No running game was restarted. [Package](selection-circle-package.txt), [launcher verification](selection-circle-launcher.txt).

# Previous build — 0.65.2

Selected blue text and cyan headings now use the readable white font's shading tinted brighter cyan. This removes the pale-rim/dark-centre appearance of the original cyan ramp while preserving original glyphs, spacing and the existing smoothing. It is a deliberate readability adaptation of the in-memory font palette. White menu labels are pixel-identical in the Direct3D title comparison. [Details and captures](TEXT-SMOOTHING.md), [pixel comparison](selected-text-pixel-check.json).

All four relevant menu/art/transition/frontend checks passed in 7.55 seconds. Title and pause captures were visually reviewed in hidden Direct3D 11 windows. [Test log](selected-text-checks.txt), [release verification](selected-text-release.json).

The portable package passed 53 file hashes and nine isolated smoke checks. Version 0.65.2 is available through the usual launcher on its next start. No running game was restarted. [Package log](selected-text-package.txt), [launcher verification](selected-text-launcher.txt).

# Previous build — 0.65.1

Pause/menu text and the match HUD now use smooth scaling of the original bitmap fonts. Premultiplied-alpha uploads prevent dark fringes around transparent lettering; menu fades also retain smooth scaling. Authored pixels, font metrics and interaction targets are unchanged. The composed menu artwork and HUD panels soften along with the lettering; the 3D scene's texture sampling is unchanged. [Scope and before/after captures](TEXT-SMOOTHING.md).

All **12 affected presentation/menu/HUD checks passed** in **18.33 seconds**. Hidden Direct3D 11 captures of pause, title and match HUD were visually reviewed at 1200 × 830, with software-rendered controller pause coverage at 960 × 664. The full 105-test suite remains the preceding 0.65.0 baseline. [Focused checks](text-smoothing-checks.txt).

The package passed **53 file hashes and nine isolated smoke checks**. The stable launcher picks up 0.65.1 on its next start. No running game was restarted. [Package](text-smoothing-package.txt), [launcher verification](text-smoothing-launcher.txt), [identity and state retention](text-smoothing-release.json).

# Previous build — 0.65.0

Post-match results now resume the original Euro theme rather than the Actua outcome stings when Euro music is installed. The completed retail match returns to the main frontend loop, which restarts `EURO96.WAV`; 17 pinned instruction anchors establish this route. Results-to-menu navigation continues the song without restarting. Older-media fallback retains the one-shot stings. [Behavior, evidence and board findings](RESULT-MUSIC.md).

All **105 regression checks passed** in **222.44 seconds**, including the complete championship and new full-song PCM/loop checks. Three focused checks and their Gaiking repeat also passed: result audio, frontend contracts and the native frontend smoke. Gaiking used dummy audio/video; physical listening and output-device switching remain pending. [Full suite](result-music-regression.txt), [focused tests](result-music-focused.txt), [remote report](result-music-gaiking.json).

The package passed **53 hashes and nine isolated smoke checks**. Version 0.65.0 is published for the usual launcher's next start; build, remote, package and published executable identity and unchanged media/SOURCE/user state are checked in the [release evidence](result-music-release.json). [Package checks](result-music-package.txt), [launcher](result-music-launcher.txt). No running game was restarted.

The stadium-board audit corrected the remaining-work description: the three preserved WINNERS animations are from a 27-record archive, while this Euro disc has a different 16-record archive and different handlers. No speculative victory event was connected. Retail board decoding/routing remains open in the [current completion checklist](COMPLETION-CHECKLIST.md).

# Previous build — 0.64.0

New championships now follow the original 31-fixture venue schedule, visiting all eight Euro stadiums and finishing at Wembley. The preserved table matches the retail executable, whose setup instructions also select that fixture's stadium. Existing version 1–3 saves keep their fixed venue; scheduled championships persist in version 4. [Behavior and compatibility](TOURNAMENT-VENUES.md), [retail proof](venue-schedule-retail.json).

All **104 regression checks passed** in **217.72 seconds**. The watched championship loaded the correct stadium before all 31 openings and resumed all 31 checkpoints. Venue checks also covered 39 older-format/venue combinations, away-country control and unavailable media. The screenshot helper was then corrected to capture the destination menu rather than the outgoing fade; its tournament test passed again, without changing the game binary. [Full regression](venue-schedule-regression.txt), [capture recheck](venue-schedule-captures.txt).

Gaiking passed the same venue/save contracts and full watched championship using the new test executables and verified unchanged media. The complete session produced 31 natural openings, 62 tunnel exits and a Wembley final; user state stayed unchanged. Dummy audio/video was used. [Remote report](venue-schedule-gaiking.json), [fixture-by-fixture session](venue-schedule-gaiking-session.txt).

The portable package passed **53 hashes and nine isolated smoke checks**. Version 0.64.0 is delivered through the usual launcher on its next start. SOURCE and user save/preferences retention, build/package/published identity and launcher verification are recorded in the [release evidence](venue-schedule-release.json). [Package checks](venue-schedule-package.txt), [launcher](venue-schedule-launcher.txt). No running game was restarted. Physical controller and audible-device checks remain pending in the [current completion checklist](COMPLETION-CHECKLIST.md).

# Previous candidate — 0.63.0 RC1

The final audit found no failure in the **103-test local suite** (207.45 seconds). All 16 active original action-dispatch cases still call restored handlers. Package verifier contracts accepted the valid fixture and rejected seven damaged/path-invalid fixtures. [Full suite](final-candidate-regression.txt), [dispatcher audit](final-candidate-action-audit.json), [verifier contracts](final-candidate-package-contracts.txt).

Gaiking Windows 11 validation passed **nine isolated package checks**, a **31-match tournament**, the **32-match matrix**, save recovery, all **1,920 player-name callback/PCM cases**, and **746 period-audio cases**. Four hidden **Direct3D 11 desktop checks** also passed; title, selection and result captures were visually reviewed. Audio used dummy devices and desktop rendering used the existing RDP session. The temporary task was removed. [Remote report](final-candidate-gaiking-report.json), [desktop report](final-candidate-gaiking-desktop.json), [final remote verification](final-candidate-gaiking-final.txt).

The frozen portable package contains **53 verified files**, passes all **nine isolated smoke checks**, and has corrected current instructions. Its executable/media are identical to the published 0.63.0 build; only READ-ME.txt and its manifest checksum changed. The usual launcher remains on the tested build. All 427 SOURCE files and local/remote user save/preferences state were preserved. No running game was restarted. [Candidate identity and hashes](final-candidate-release.json), [package checks](final-candidate-package.txt), [launcher](final-candidate-launcher.txt).

This is an **automatically validated release candidate**, not final physical-hardware sign-off or exact retail parity. The [completion checklist](COMPLETION-CHECKLIST.md) is the authoritative current backlog. Physical two-controller, audible output/device switching and the combined human session remain pending. Kuznetsov's accepted fallback is closed. Menu timing/navigation, per-fixture venue assignment and remaining audio routes are documented fidelity differences. No new gameplay features were added in this audit.

# Previous feature release — 0.63.0

All 320 players now have verified appearance mappings and three spoken-name tones: 959 retail Euro recordings plus one existing Actua fallback for Kuznetsov's missing normal tone. Original extra-time entry speech now plays at the actual tied 90-minute knockout transition. [Evidence and unresolved audio](RETAIL-IDENTITY-AUDIO.md).

All **23 affected checks passed** in **68.08 seconds**, including 1,920 complete name PCM/callback cases, all 320 appearance mappings, every available XTM recording, both knockout rules, language/absent-media behavior, full tournament progression, 31 save/reload checkpoints, penalty outcomes, automatic selection, local head-to-head and trophy playback. [Regression](identity-audio-regression.txt), [runtime detail](identity-audio-runtime.txt).

Version **0.63.0** is published as `773bb8f8af584c1f-2b4b128d-0703f4d7-7dba4a40`. The package passed **53 file hashes and nine isolated smoke checks**; the usual launcher verifies. Build, package and published media/executable hashes match. All 427 SOURCE files and the user save remain unchanged. No running game was restarted. [Release evidence](identity-audio-release.json), [package](identity-audio-package.txt), [launcher](identity-audio-launcher.txt). No physical audio/controller session is claimed.

# Previous release — 0.62.1

All 320 preserved roster entries now have their authored retail previews. DOS character decoding and independently verified player identities resolve the 23 previously unmatched names, including Neno. [Evidence](PREVIEW-IDENTITIES.md).

All **five focused checks passed**, including six identity-matching unit cases, all 109 preview animations, 327 pixel samples and real menu captures for Neno, Klinsmann and Osman. All 305 previously available player/venue mappings retain identical palette/animation bytes. [Tests](preview-identities-tests.txt), [identity evidence](preview-identity-resolution.json).

Version **0.62.1** is published as `b86b017ecc36c9e7-2b4b128d-0703f4d7-13c73d11`. The package passed **53 file hashes and nine isolated smoke checks**; the usual launcher verifies. All 427 SOURCE files and the user tournament save are unchanged. No game was restarted and no physical-controller session was performed. [Release evidence](preview-identities-release.json), [package](preview-identities-package.txt), [launcher](preview-identities-launcher.txt).

# Previous release — 0.62.0

Original animated player and venue previews, plus the Euro 96 championship trophy and credits films with stereo audio. See [recovery and validation](EURO96-PREVIEWS.md).

**24 affected checks passed**, followed by **four final rechecks** after the venue layout correction and watched-final test. The retail media test decoded all 106 FAP loops and all 31,456 trophy frames, comparing 318 preview and 96 film pixel hashes against independent importers. [Regression](retail-presentation-regression.txt), [final verification](retail-presentation-final.txt).

The package passed **53 file hashes and nine isolated smoke checks**. Version **0.62.0** is published as `45bd41de8b22b2c7-2b4b128d-0703f4d7-0919909e`; [launcher verification](retail-presentation-launcher.txt) and [package checks](retail-presentation-package.txt) passed. Build, portable package and published executable/media hashes match. All 427 preserved SOURCE files and the user's tournament save are unchanged. No running game was restarted and no physical-controller session was performed. [Release evidence](retail-presentation-release.json).

# Previous release — 0.61.1

Kickoff now selects the centre taker before the first gameplay tick. When the opponent kicks off, the nearest active outfield defender is selected. This applies to both local controllers and every change of ends. The original automatic centre pass is preserved, and live auto-selection resumes on its first eligible tick. [Reproduction and validation](KICKOFF-SELECTION.md).

**22 affected checks passed**, covering 32 kickoff combinations, two full matches of automatic selection, natural/skipped intro completion, awarded restarts, shootouts, tournaments, and both local controllers. The original failure was reproduced first ([before](kickoff-selection-before.txt), [focused checks](kickoff-selection-focused.txt)). Eighteen broader checks passed initially; two old halftime expectations were corrected to require the kickoff taker while retaining controller/team ownership, and both passed on rerun ([regression](kickoff-selection-regression.txt), [recheck](kickoff-selection-recheck.txt)). The [package](kickoff-selection-package.txt) passed 51 file hashes and nine isolated smokes.

Version **0.61.1** is [published](kickoff-selection-published.txt), snapshot `36ae0613a4f5ca8f-2b4b128d-0703f4d7-92ade261`. The [usual launcher verifies](kickoff-selection-launcher.txt); build, package and published executable hashes match. The player save and all 427 preserved SOURCE files are unchanged ([release evidence](kickoff-selection-release.json)). A rendered capture confirms the marker at centre. No running game was restarted; no physical-controller session was performed.

# Previous release — 0.61.0

All four original quarter-final introductions now play in original-draw tournaments. The retail qualifier table proves that the bracket pairings match the preserved engine, including two reversed home/away listings. Each match plays its complete STT recording without an appended compatibility ending. [Evidence and boundaries](EURO96-SEQUENCES.md).

All **21 affected checks passed** in **92.95 seconds** ([test log](euro-quarters-focused.txt)). Coverage includes **224 exact introductions** (128 quarter-final pairings) plus **eight saved/resumed quarter-final kickoffs**, **160 country/side/result cases**, all **592 period score cases**, complete CPU tournaments, frontend progression/save recovery, language changes, saved replays and head-to-head ([runtime details](euro-quarters-runtime.txt)). The [portable package](euro-quarters-package.txt) passed **51 file hashes and nine isolated smoke checks**. Importing again produced identical media. Validation was scoped to the changed introduction routing; the previous full run remains 97/97 at 0.58.0.

Version **0.61.0** is [published](euro-quarters-published.txt), snapshot `a0fbd5544e05fac3-2b4b128d-0703f4d7-92ade261`. The [usual launcher verifies](euro-quarters-launcher.txt); executable and supplement hashes match across build, package and publication. The tournament save and all 427 tracked SOURCE files are unchanged ([release evidence](euro-quarters-release.json)). No running game was restarted. Audio tests use dummy devices and exact PCM checks; no physical listening session was performed.

# Previous release — 0.60.0

Original-draw group introductions now follow the retail fixture lookup. Semi-finals and the final link the original opening to the correct countries. Championship, shootout and extra-time winners use their country-specific retail calls; decisive regulation knockout results use FLL. Quarter-final introductions remain on compatibility audio pending bracket mapping. [Evidence and boundaries](EURO96-SEQUENCES.md).

All **21 affected checks passed** in **78.81 seconds** ([test log](euro-sequences-focused.txt)). Coverage includes **96 exact introductions**, **160 country/side/result cases**, all **592 period score cases**, complete CPU tournaments, frontend progression/save recovery, language changes, saved replays and head-to-head ([runtime details](euro-sequences-runtime.txt)). The [portable package](euro-sequences-package.txt) passed **51 file hashes and nine isolated smoke checks**. Importing again produced identical media. Validation was scoped to the changed audio and presentation context; the previous full run remains 97/97 at 0.58.0.

Version **0.60.0** is [published](euro-sequences-published.txt), snapshot `8e562be1f47d59ed-2b4b128d-0703f4d7-92ade261`. The [usual launcher verifies](euro-sequences-launcher.txt); executable and supplement hashes match across build, package and publication. The tournament save and all 427 tracked SOURCE files are unchanged ([release evidence](euro-sequences-release.json)). No running game was restarted. Audio tests use dummy devices and exact PCM checks; no physical listening session was performed.

# Previous release — 0.59.0

Half-time and regulation results now select verified Euro 96 period recordings: score-specific HLF/FLL takes and the separate END bank for draws. Missing or ambiguous scores and knockout endings retain generic compatibility calls. [Evidence and remaining work](EURO96-PERIODS.md).

All **eight affected checks passed** in **34.57 seconds**, including **592 period score/order/take cases**, exact full-phrase PCM comparisons, actual whistle transitions, complete English/French CPU matches and language-switch checks ([test log](euro-periods-focused.txt)). The [portable package](euro-periods-package.txt) passed **50 file hashes and nine isolated smoke checks**. The importer reproduced identical output on a second run. The gameplay suite was not repeated for this audio-only change; the preceding 0.58.0 full run passed 97/97 checks.

Version **0.59.0** is [published](euro-periods-published.txt), snapshot `e83faa116629a237-2b4b128d-0703f4d7-205074c5`. The [launcher verifies](euro-periods-launcher.txt), with matching build/package/publication hashes. The tournament save and all 427 tracked SOURCE files remain unchanged ([release evidence](euro-periods-release.json)). No running game was restarted. Physical-device listening remains a user check; automated validation uses dummy devices.

# Previous release — 0.58.0

Thirty match-commentary categories now use verified retail Euro 96 recordings. All 132 score-announcement takes are integrated without changing score identities or consuming gameplay RNG. The Euro 96 pitch bank is preferred, including its quieter foul reaction. [Scope and remaining audio boundaries](EURO96-AUDIO.md).

All **97 regression checks passed** in **512.87 seconds** ([full log](euro-audio-regression.txt)), following the [focused audio/language checks](euro-audio-focused.txt). The [portable package](euro-audio-package.txt) passed **49 file hashes and nine isolated smoke checks**, including native audio and commentary. The importer reproduced identical output on a second run.

Version **0.58.0** is [published](euro-audio-published.txt), snapshot `f4ef35d71bb72b4e-2b4b128d-0703f4d7-f074cfff`. The [launcher verifies](euro-audio-launcher.txt); executable and new-media hashes match across build, package and publication. The tournament save and all 427 tracked SOURCE files remain unchanged ([release evidence](euro-audio-release.json)). No running game was restarted. Validation used dummy audio devices; a physical listening session was not performed.

# Previous release — 0.57.1

Human keepers now finish ground pickups and get-up animations in the original standing hold pose. The R3/F6 information panel uses recovered Euro96 title-bar, panel and font assets; development tabs and their hidden mouse targets are removed from team matches. This remains a native layout, not an exact DOS options screen. [Fix evidence](KEEPER-PANEL-FIX.md) and [actual executable capture](keeper-panel-live.png).

All [13 affected regression checks](keeper-panel-checks.txt) passed, with [three rendering/input checks](keeper-panel-art-checks.txt) rerun successfully after the final title-bar adjustment. The [portable package](keeper-panel-package.txt) passed 47 file hashes and seven isolated smoke checks. The preceding 0.57.0 release passed all 95 tests; this patch used focused validation.

Version **0.57.1** is [published](keeper-panel-published.txt), snapshot `8bc38b0d95ffa1c6-2b4b128d-0703f4d7-f0805be4`. The [launcher verifies](keeper-panel-launcher.txt), executable hashes match, and the tournament save and 427 tracked SOURCE files remain unchanged ([release evidence](keeper-panel-release.json)). No running game was restarted.

# Previous release — 0.57.0

All eight retail Euro 96 stadiums are selectable, with Wembley as the default. Matched models, textures, palettes, pitch/sky data and tunnel positions come from the verified Euro96 disc. Legacy venue IDs remain available to existing tournaments and saved replays. [Recovery and boundaries](EURO96-STADIUMS.md); [reviewed venue/tunnel images](euro-stadium-review.png).

All **95 regression checks passed** in **445.94 seconds** ([log](euro-stadium-regression.txt)), including 24 retail camera views, both-half exits at eight venues, save/replay round-trips and malformed packs. The first focused run caught a corner-flag atlas mismatch; restoring the preserved flag atlas separately from retail ground data fixed it, and the full run passes the flag animation contract. All **47 package hashes and seven isolated smoke checks** passed ([package log](euro-stadium-package.txt)).

Version **0.57.0** is [published](euro-stadium-published.txt), snapshot `7efb18fec5003381-2b4b128d-0703f4d7-f0805be4`, and the [launcher verifies](euro-stadium-launcher.txt). Built, packaged and published executable hashes match. The tournament save and all 427 tracked SOURCE files remain unchanged ([release evidence](euro-stadium-release.json)). No running game was restarted. Visual checks used the local SDL software renderer; this batch did not include a new Gaiking or physical-controller session.

# Previous release — 0.56.0

Normal startup now enters the Euro 96 frontend directly. The Actua intro/making-of entries, films and unused title images are excluded from new releases. Original local imports and explicit legacy decoder diagnostics remain intact. [Evidence and boundaries](EURO96-STARTUP.md).

All 18 affected checks and seven isolated package checks passed; all 39 packaged file hashes verified. The normal launcher is published, saves/SOURCE are unchanged, and no running game was restarted. [Release evidence](euro-startup-release.json).

# Previous release — 0.55.1

The agreed restoration target is **PC/DOS UEFA Euro 96**. Window/header/error labels and current instructions now reflect that identity. `Play-Euro96.cmd` and portable `Start-Euro96.cmd` are the preferred launchers; the old names remain working aliases. Saves, internal executable/build identifiers and gameplay are unchanged. [Current provenance audit and replacement priorities](EURO96-TARGET.md).

All [three focused frontend/save/menu checks](euro-target-checks.txt) passed in 6.69 seconds. The [portable package](euro-target-package.txt) passed all 43 file hashes and eight isolated smoke checks. The immediately preceding 0.55.0 release passed all 93 regression checks; this naming-only executable change did not require another full gameplay run. [Publication](euro-target-published.txt), [launcher verification](euro-target-launcher.txt) and [matching release hashes, aliases, save and SOURCE retention](euro-target-release.json).

# Previous release — 0.55.0

Retail heads, skin remaps, keeper artwork, country introductions and Euro96 menu music were integrated, alongside match-setup/pause/transition presentation cleanup. [Scope and remaining media](EURO96-PRESENTATION.md). All [93 regression checks](euro-presentation-regression.txt) and [eight package smoke checks](euro-presentation-package.txt) passed.

# Previous release - 0.54.2

The restoration's bottom playback toolbar and timer are removed from the intro and making-of film. The complete original 4:3 frame fills the available height with black side margins; pause, seek, restart and skip remain available through existing keyboard/gamepad controls. Reviewed captures: [intro](intro-clean.png), [making-of](movie-clean.png).

All **six focused checks passed** in **10.62 seconds** ([log](intro-clean-checks.txt)): original movie/intro decoding and PCM contracts, actual executable film/input flows, frontend entry and original menu artwork. This presentation-only change did not require another full gameplay regression; the preceding 0.54.1 run passed all 92 checks. The [final package](intro-clean-package.txt) passed **39 hashes and eight isolated smoke checks**.

Version **0.54.2** is [published](intro-clean-published.txt), snapshot `ac72911d34933fa7-2b4b128d-0703f4d7-d0ae0815`, and the [launcher verifies](intro-clean-launcher.txt). Build, package and published executable [hashes match](intro-clean-hashes.json): `AC72911D34933FA70805D31115C19CB2CB3F22384FDE939C6365EE29FF1360C9`. Portable folder: `dist/actua96-0.54.2-43670ae5adb64a0ca0b381981ad76dae`.

The tournament [save](intro-clean-save-retention.json) and all 427 tracked [SOURCE files](intro-clean-source-retention.json) remain unchanged. No running game was restarted.

# Previous release - 0.54.1

The recovered Euro96 CD shirt numbers **1-20**, in light and dark variants, now render on both teams, keepers and substitutes. Original glyphs use a separate transparent texture page and retain their bindings across mirrored poses, changed ends and stadium changes. Light/dark contrast selection is a native adaptation, not a verified DOS colour-choice table. [Implementation and provenance](SHIRT-NUMBERS.md); [rendered shirts 16-20](shirt-number-render-review.png).

All **92 regression checks passed** in **427.96 seconds** ([full run](shirt-number-regression.txt)). The new rendered contract covers all forty glyphs, all twenty shirt identities on both teams and keepers, mirrored poses, both halves, malformed packs, a real number-18 substitution, five venues and media reloads. Existing full tournament, replay, head-to-head, menus, language and material checks passed. The frozen-frame test refreshes the presentation snapshot when changing shirts and compares only numeral changes when varying stadiums, excluding background differences.

The final [portable package](shirt-number-package.txt) passed **39 file hashes and all eight isolated checks**. Version 0.54.1 is [published](shirt-number-published.txt), snapshot `d0739269127a998f-2b4b128d-0703f4d7-d0ae0815`, and the [usual launcher verifies](shirt-number-launcher.txt). Built, packaged and published executable [hashes match](shirt-number-release-hashes.json): `D0739269127A998FCBA21E66C04F858CEC92AAB90273BE10EA7B11116060C86C`. Portable folder: `dist/actua96-0.54.1-68021f19d54f422983ed8fd8559d62f6`.

The tournament [save](shirt-number-save-retention.json) and all 427 tracked [SOURCE files](shirt-number-source-retention.json) remain unchanged. No running game was restarted. The update is available at the next normal launch; no separate hands-on test session is requested.

# Previous release - 0.54.0

**Post-release finding:** all 1-20 shirt-number images exist in retail EUROREND record 38. The release's missing 16-20 display is an integration gap, not missing disc artwork. [Atlas and coordinate evidence](EURO96-NUMBER-DISCOVERY.md).

The supplied Euro96 CD now supplies all **32 outfield strips**, country-specific alternate choices, 296 exact-name appearance matches and English player-name coverage for **308 of the existing 320 players**. Preserved squads, saves, heads, exposed-skin artwork, keepers and French commentary remain compatible. Numbers 16-20 are still absent. [Scope, provenance and rendering boundaries](EURO96-IMPORT.md).

All **91 regression checks are verified**: the full [472.49-second run](euro96-regression.txt) passed 90 checks; the remaining language test still expected the older Actua Shearer recording. Its expected PCM hash was independently read from Euro96 E_ENG.MAD/e_eng09z.wav, then the rebuilt [language contract passed](euro96-language-check.txt) in 1.59 seconds. This changed only the test oracle, not production code. Coverage includes the full tournament frontend session, saved replays, head-to-head, all starters/keepers in both halves, 32 retail strips from front and back, new-team speech in both halves and malformed-media rejection. [Rendered kit review](euro96-kit-render-review.png).

The [portable package](euro96-package.txt) passed **38 file hashes and all eight isolated checks**. Version 0.54.0 is [published](euro96-published.txt), snapshot `d6ec048069f771f4-2b4b128d-0703f4d7-fea80ba1`, and the [usual launcher verifies](euro96-launcher.txt). Its allowlist now includes both Euro96 overlay files. Build, package and published executable [hashes match](euro96-release-hashes.json): `D6EC048069F771F437578E62FBFA7D3A3F8B183B044A2E46991B5888459658F3`. Portable folder: `dist/actua96-0.54.0-8bafa53b046748c9954f55c1cace5fb2`.

The user's tournament [save](euro96-save-retention.json) and all 427 tracked [SOURCE files](euro96-source-retention.json) remain unchanged. No running game was restarted. Physical controller and audible-listening checks were not performed in this batch; imported speech was validated as original PCM and exercised through the real callbacks and mixer.

# Previous release - 0.53.0

Close substitution/sending-off rendering now clips players and shadows against opaque stadium geometry, preserving the actual tunnel opening. Team selection moves through the original country-carousel poses with source-derived direction and acceleration. [Implementation boundaries, remaining work and combined review](PRESENTATION-RELEASE.md).

All **89 regression checks passed** on the final build in **379.52 seconds** ([full log](presentation-release-regression.txt)). These include the complete tournament/save paths, menu transitions, replays, controller dispatch, 60 natural close-tunnel observations, 25 entrance/crowd probes and all 256 country-selection transitions. Disabling the occlusion fix reproduces an 80-pixel crowd overdraw ([red check](presentation-close-tunnel-red.txt)). Reviewed images: [crowd before](presentation-roof-before.png), [after](presentation-roof-after.png), [visible tunnel opening](presentation-tunnel-opening.png), [carousel moving](presentation-country-moving.png), [settled](presentation-country-settled.png).

Version **0.53.0** is [published for the usual launcher's next start](presentation-release-published.txt), snapshot `e9aad11d3f0affbe-2b4b128d-0703f4d7-4e7d91ab`. The portable package `dist/actua96-0.53.0-b76808f7320348c0b027e80d0930343b` passed **36 hashes and all eight isolated checks** ([package log](presentation-release-package.txt)). Build/package/published executable [hashes match](presentation-release-hashes.json): `E9AAD11D3F0AFFBEF27A7191BDF4707E2894E29F207CE95E0755589A182D026C`. The [launcher verification](presentation-release-launcher.txt) passed. The user's tournament [save](presentation-release-save-retention.json) and all 427 tracked [SOURCE files](presentation-release-source-retention.json) remain unchanged. No running game was restarted. This batch was tested locally; physical controller/audio and a new Gaiking session were not performed.

# Previous release - 0.52.0

The normal game frontend, pause reference, substitutions and replay controls now use the original Euro 96 frontend artwork and fonts. This includes country sprites, the formation pitch, group fixtures and separate knockout/final pages. Native Windows settings, local play and replay-file functions remain available in that visual style. [Source boundaries and screen inventory](MENU-FIDELITY.md); [four-screen overview](menu-fidelity-overview.png).

All **89 checks are covered by passing runs**. The [full regression](menu-fidelity-regression.txt) passed 84/89 in 234.79 seconds. Four failures used superseded mouse coordinates or did not render controls before clicking; the fifth compared animated buttons at different times. Updated tests exercise the new visible controls and compare fade/direct composition at the same animation time. The [13 affected checks](menu-fidelity-final-checks.txt) all passed on the final build in 17.11 seconds, including both complete tournament paths, save recovery, tactics, settings, language, menus and input. The new art contract also compares original glyph/header pixels and the large mascot's source palette/mirrored pose. All **eight isolated package checks** and **36 file hashes** passed ([package log](menu-fidelity-package.txt)).

Version **0.52.0** is [published for the usual launcher's next start](menu-fidelity-published.txt), snapshot `3008b99b5ed66219-2b4b128d-0703f4d7-4e7d91ab`. Portable package: `dist/actua96-0.52.0-5c173938ec7a4bb3a1922aeccddc1c95`. Executable SHA-256 `3008B99B5ED66219452FA9D5CA3FAA9EFAF6804B3E28058685B000E000B62D5A` matches the build, package and published copies ([hashes](menu-fidelity-hashes.json)). The [launcher check](menu-fidelity-launcher-check.txt) passed. The user's tournament [save hash](menu-fidelity-save-retention.json) and all 427 tracked [SOURCE files](menu-fidelity-source-retention.json) remain unchanged. No running game was restarted. Validation was local; no new Gaiking or physical-controller session was performed for this presentation batch.

# Previous release - 0.51.2

Half-time and full-time exits now depth-sort the original stadium faces with the players, so the tunnel roof and crowd cover players entering the stand. The original routes, camera, timings and live match state remain intact. The close individual walk-off camera still needs separate occlusion work. [Evidence, visual comparison and scope](TUNNEL-OCCLUSION.md).

All **88 checks now pass** across the full run and corrected-fixture reruns. The [initial full run](tunnel-occlusion-regression.txt) passed 86/88 in 371.59 seconds; the two failures were an off-camera point in the new exit test and a replay scratch-directory collision. The on-pitch probe now lies within the camera, and the replay test atomically claims a fresh directory. Both [final contracts passed](tunnel-occlusion-final-contracts.txt) in 9.82 seconds, including **140 pixel comparisons** across five venues, both halves and both team lanes. The old draw order [fails the interior-pixel check](tunnel-occlusion-red.txt). All **eight isolated package checks** and **36 hashes** passed ([log](tunnel-occlusion-package.txt)).

Version **0.51.2** is [published for the usual launcher's next start](tunnel-occlusion-published.txt), snapshot `291e02b9e9150908-2b4b128d-0703f4d7-4e7d91ab`. Portable package: `dist/actua96-0.51.2-95e90d79b385455bb0c2eb05a851b710`. Executable SHA-256 `291E02B9E915090889FFEA9E025A9BEA6223B580D4E237D3839709761C299CD7`. Preserved SOURCE and the tournament [save hash](tunnel-occlusion-save-retention.json) are unchanged. No running game was restarted. This update was validated locally, not on Gaiking.

# Previous release - 0.51.1

Automatic selection was enabled and operated in two complete human-input match diagnostics, without manual switching. The audit reproduced a controller binding problem: RB handed a human exhibition to CPU control, while LB switched players. RB was also blocked during human tournament play. Both shoulders now switch players in human matches, and pause identifies `AUTO SELECT ON` or `CPU v CPU`. RB cannot give human control away and cannot change selection while paused; takeover of watched exhibitions remains available during play. The original automatic-selection rules are unchanged. This corrects a reproduced mapping problem; it does not prove that RB caused the user's reported experience. [Investigation, controls and evidence](AUTOMATIC-SELECTION.md).

All **88 regression checks passed** in **213.39 seconds** ([log](selection-rb-regression.txt)), including the expanded full-match selection contract, actual SDL controller mapping, tournament routing, head-to-head and saved replays. The [two human-input matches](selection-rb-matches.txt) recorded 166/185 selection changes, including 34/47 defensive and 103/115 loose-ball handoffs. The [31-match tournament](selection-rb-session.txt) retained the prior scores and transition counts with every checkpoint resumed. All **eight isolated package checks** and 36 hashes passed ([log](selection-rb-package.txt)). The new [pause status](selection-rb-paused.png) was visually reviewed. Gaiking's SSH connection [timed out](selection-rb-gaiking.txt); this version has not been tested on that machine.

Version **0.51.1** is [published for the usual launcher's next start](selection-rb-published.txt), snapshot `9e29b813c4d10edc-2b4b128d-0703f4d7-4e7d91ab`. Portable package: `dist/actua96-0.51.1-2da912f78b3b48ad84a3208799b1f1bc`. Executable SHA-256 `9E29B813C4D10EDC6A7837BB22E3809463162EA3DA042C9BC9C332333819DFCF` matches the built, packaged and published copies. SOURCE is unchanged and the tournament [save hash is unchanged](selection-rb-save-retention.json). No running game was restarted.

# Previous release - 0.51.0

Frontend page changes now retain the outgoing screen, fade to black and reveal the destination using the recovered 25-step curve. Buffered keyboard, controller and mouse input cannot activate an unseen destination. Settled text keeps its direct rendering at fractional window scales. [Source boundaries, controls and asset audit](MENU-TRANSITIONS.md).

All **88 regression checks passed** in **199.69 seconds** ([log](menu-transition-regression.txt)). Six existing UI tests now render destination menus between deliberate navigation actions; their original assertions remain in place ([focused checks](menu-transition-navigation-check.txt)). The dedicated transition contract covers outgoing/black/incoming pixels, focus hold, buffered input, actual SDL virtual-controller events, target-content reset and scaled rendering. The [31-match session](menu-transition-session.txt) retained the prior scores and transition counts, with all 31 checkpoints resumed. The transition contract and executable frontend flow passed [on Gaiking](menu-transition-gaiking.txt) using dummy/software rendering.

Version **0.51.0** is [published for the usual launcher's next start](menu-transition-published.txt), snapshot `30fe6bf2e039562a-2b4b128d-0703f4d7-4e7d91ab`. Its portable package passed all **eight isolated checks** and 36 file hashes ([log](menu-transition-package.txt)): `dist/actua96-0.51.0-17d26a16c68a4712b15ce530ca96debc`. Executable SHA-256 `30FE6BF2E039562AE021CFEE50D72ADFD485B34BF6229DDAFA578564F7DD64D4` matches the built, packaged, published and Gaiking-tested copies. Gaiking's separate `menus-0510` folder reuses existing media and a hash-verified SDL DLL. SOURCE and the user's tournament [hash](menu-transition-save-retention.json) remain unchanged. No running game was restarted.

The fade timing remains a native 60 Hz adaptation, not verified DOS interrupt timing or exact palette quantization. The source/imported-CD atlas audit found artwork for shirts 1-15 only; matching 16-20 artwork remains absent. Physical two-controller validation is still pending at the user's convenience. This batch can be reviewed during the next normal session.

# Previous release - 0.50.0

Frontend menus now use the original 25-step fade-up curve, and officials remain visible during period-end player exits with the recovered referee/assistant dispatcher. Source review corrected the earlier assumption of missing referee tunnel walks; unused winner clips still have no verified game-event trigger. [Source evidence, implementation boundaries and screenshots](PRESENTATION-FINISH.md).

All **87 regression checks passed** in **197.27 seconds** ([log](presentation-finish-regression.txt)). The new five-screen rendered test checks intermediate brightness, exact unfaded final pixels, focus hold, navigation and unchanged gameplay; the exit test covers officials in all five venues at both ends. The [31-match session](presentation-finish-session.txt) retained the exact prior scores and transition counts and resumed every checkpoint. [Gaiking](presentation-finish-gaiking.txt) passed the new presentation contract, all five venue exits and the actual executable frontend flow using dummy/software rendering. Physical head-to-head validation remains pending at the user's convenience.

Version **0.50.0** is [published for the usual launcher's next start](presentation-finish-published.txt), snapshot `25f52bed6312ca1a-2b4b128d-0703f4d7-4e7d91ab`. The portable package passed all **eight isolated checks** and 36 file hashes ([log](presentation-finish-package.txt)): `dist/actua96-0.50.0-5a31bca9a8604fe3a54b4aef6702d460`. Executable SHA-256 `25F52BED6312CA1ACF2CF6747AFC320CC287DE5B9CC97756E4C5643B05386F48` matches the published, packaged and Gaiking-tested copies. Gaiking uses a separate small `presentation-0500` folder with existing media. SOURCE remains unchanged, the tournament [hash is unchanged](presentation-finish-save-retention.json), and no running game was restarted.

This batch can be reviewed during the next normal match rather than a separate hands-on test. Menu fade-down and exact DOS menu/timer behavior remain outside this implementation; the documented 60 Hz composed RGB fade is a native adaptation of the recovered curve.

# Previous release - 0.49.0

Local head-to-head exhibitions are ready: connect two controllers and choose **H / West (Xbox X): 2 PLAYERS** on match setup. Each controller has independent movement, selection, passing, shooting, restarts and shootout input around the original engine. Countries and controller assignments persist through changes of ends. Start pauses; a lost controller pauses the match, and a replacement must explicitly join before resuming. [Controls, implementation and scope](HEAD-TO-HEAD.md).

All **86 regression checks passed** in **176.68 seconds** ([log](headtohead-regression.txt)). After isolating automated tests from attached physical controllers, all five affected controller/frontend checks passed again on the final build ([log](headtohead-final-checks.txt)). The [31-match frontend session](headtohead-session.txt) retained the exact 0.48 results and transition counts, including all 31 checkpoint resumes. The engine contract covers both halves, eight original-control pass receptions, independent charges/shots, substitutions, 24 restart cases and a complete two-human-input shootout. Both that contract and the actual two-controller SDL application smoke passed [on Gaiking](headtohead-gaiking.txt), using virtual controllers and dummy/software rendering.

Version **0.49.0** is [published for the usual launcher's next start](headtohead-published.txt), snapshot `05294507bbc51ce9-2b4b128d-0703f4d7-4e7d91ab`. Its portable package passed **eight isolated checks** and all 36 file hashes ([log](headtohead-package.txt)): `dist/actua96-0.49.0-060a9fe37bec4b94bc79970ce6497bca`. Executable SHA-256 `05294507BBC51CE937EF4AFAA92AE974372FD71970FA4B3384D85CF6BAB3DA4F` matches the packaged, published and Gaiking-tested copies. Gaiking's small separate `headtohead-0490` folder reuses existing test media. The user's tournament [hash is unchanged](headtohead-save-retention.json), SOURCE remains unchanged, and no running game was restarted.

The [selection screen](headtohead-selection.png), [lineups](headtohead-lineup.png), [live pitch](headtohead-pitch.png), [disconnect pause](headtohead-disconnect.png) and [P2 power indicator](headtohead-p2-charge.png) were visually reviewed. One combined match with two physical controllers remains the human playtest. Same-team co-op, multiplayer tournaments and network play are not implemented. The remaining original-game parity and hardware validation gaps are listed below.

# Previous release - 0.48.0

Original first-time passing is connected: hold X + Space or controller West + South (Xbox X + A) on an incoming loose ball. The original predictor chooses a reachable chip, standing header or jumping header and an eligible receiver. Three contradictory shot-only conditions are corrected in generated code; the preserved SOURCE files are unchanged. [Controls and precise adaptation](FIRST-TIME-PASSING.md).

All **84 regression checks passed** in **213.93 seconds** ([full log](firstpass-regression.txt)). The [31-match frontend session](firstpass-session.txt) produced the same results and transition counts as 0.47: Spain champion, 59 goals/replays, two extra-time matches, one shootout, and all checkpoints resumed. [Six contact cases](firstpass-contracts.txt) verified the three pass types on both halves, with no prior collection and no shot. Invalid-receiver and pause/re-arm cases also passed. The real SDL keyboard and virtual-controller smoke passed [locally](firstpass-focused.txt) and [on Gaiking](firstpass-gaiking.txt).

Version **0.48.0** is [published for the usual launcher's next start](firstpass-published.txt). The portable package passed **seven isolated checks** and all 36 file hashes ([log](firstpass-package.txt)); it is `dist/actua96-0.48.0-bfc41f8edea442eb9fbf86e282cdf712`. The executable SHA-256 is `B324EC649545CB4D0995CC322E795386636B7EBD6212D8F55B06462287CF7AEF`, matching Gaiking's tested copy. Its separate `firstpass-0480` folder reuses existing test media; no old packages or user files were removed. [Pause help](firstpass-help.png) was visually reviewed. The user's tournament [hash is unchanged](firstpass-save-retention.json). No running game was restarted.

**Single-player is close to feature complete for this restoration.** The next validation is one combined human match/tournament session, not individual tests for each control. Full original-game parity still has separate gaps: local/network multiplayer, matching Euro 96 kits/names/number artwork, exact original menus and remaining presentation. Mid-match saving is also absent; unfinished fixtures resume from kickoff. The detailed remaining-work list below distinguishes these from hardware/audio/controller validation.

# Previous release - 0.47.0

Saved replays now persist the original 15-second visual buffer, with a library at the title and from paused play, isolated playback, existing five cameras/speeds/seek controls, and safe return to the live match. Files and navigation are native additions, not a recovered DOS replay-file feature. [Five-part playtest and boundaries](SAVED-REPLAYS.md).

The new tests also exposed a reserve-number crash: tournament identities 16-20 reached a renderer limited to 1-15 shirt artwork. These valid players now use the original 12-body-face path without a back-number overlay. Matching number artwork remains a completeness gap.

All **84 regression checks passed** in 184.91 seconds ([log](replay-file-regression.txt)), including all 31 watched tournament fixtures and checkpoint resumes ([session](replay-file-session.txt)). The replay file contract confirmed exact render parity across all five cameras and at an interpolated subframe; the executable smoke confirmed real keyboard and virtual-controller dispatch.

Version **0.47.0** is [published for the usual launcher's next start](replay-file-published.txt), snapshot `8b9d1c1679d1d026-2b4b128d-0703f4d7-4e7d91ab`. The launcher verification passed and the executable hash matches the Gaiking-tested and portable builds. No running game was restarted.

The final portable package passed **seven isolated checks**, including the new keyboard/controller replay-file smoke ([log](replay-file-package.txt)). Gaiking passed the five-group file/rendering contract, actual native replay-file input smoke with focus/disconnect, and the existing frontend smoke ([log](replay-file-gaiking.txt)). It used dummy/software rendering and virtual controller events. Reviewed captures: [library](replay-file-library.png), [saved frame](replay-file-frame.png). Physical controller/audio testing remains manual.

Package: `dist/actua96-0.47.0-fe312ce2cf954d0195f77bba1742e93a`. Gaiking's separate `replay-files-0470` executables use its existing `package-0440-final` media. Twenty obsolete pre-match BMPs were losslessly zipped and SHA-256 verified, then the redundant BMPs removed to free test space; evidence and release packages remain. The user's tournament [hash is unchanged](replay-file-save-retention.json).

# Previous release - 0.46.0

Substitutions now show the original outgoing/incoming tunnel route and player on/off camera. Direct and second-yellow dismissals show the outgoing route without a replacement. Referees and assistants move to original restart targets, with the source referee-facing Y-coordinate typo corrected. [Combined five-part playtest and source boundaries](WALK-OFFS.md); [extraction evidence](walkoff-source.json).

The route runs on a presentation copy while the live match remains frozen. Native substitution and discipline results are applied once. Pause, skip, focus/disconnect and action-release protection cover the transition; diagnostic modes without frontend controls omit it. Referee tunnel entrances/exits remain unrestored.

All **82 regression checks passed** in 198.29 seconds ([log](walkoff-regression.txt)). The complete watched tournament naturally completed **226 individual player transitions**, 31 openings and 62 team tunnel exits, and resumed all 31 saved checkpoints ([session](walkoff-session.txt)). Outgoing, incoming and dismissed-player render captures were inspected.

Version **0.46.0** is [published for the usual launcher's next start](walkoff-published.txt), snapshot `978b6e822fe95e8f-2b4b128d-0703f4d7-4e7d91ab`. The published launcher verification passed. No running game was restarted.

The final portable package passed all six isolated checks ([log](walkoff-package.txt)). Gaiking passed the five-group walk-off contract plus the actual native frontend and diagnostic match-depth smoke ([log](walkoff-gaiking.txt)). Gaiking used dummy/software rendering; physical controller and audio review remain manual. Only two inactive portable copies from earlier package audits were removed to free test space, retaining their logs and existing release packages.

Portable package: `dist/actua96-0.46.0-0cc593a4896b482a86f3e83b2a517f62`. Gaiking's separate `walkoff-0460` executables use the existing `package-0440-final` media under the established validation folder. The user's tournament [hash remains unchanged](walkoff-save-retention.json).

# Previous release - 0.45.0

The original pre-match on-pitch lineup, officials and orbiting camera now precede exhibition and watched tournament kickoffs. The native presentation copy preserves live match state, and skip/pause/focus handling protects the transition. [Source boundaries and one combined five-part playtest](PRE-MATCH.md).

All **81 regression checks passed** in 189.44 seconds ([log](prematch-regression.txt)), including all 31 watched tournament fixtures with **31 natural openings and 62 natural tunnel exits** ([session summary](prematch-session.txt)). Visual inspection covered the close, mid-orbit and wide views. The final portable package passed all six isolated checks ([log](prematch-package.txt)). Gaiking passed the five-group opening contract and actual native frontend smoke with a held virtual-controller skip button ([log](prematch-gaiking.txt)); this round uses dummy/software rendering, not a physical-controller or audible playback test.

Version **0.45.0** is [published for the usual launcher's next start](prematch-published.txt), build `bdcef784ce4898ff-2b4b128d-0703f4d7-4e7d91ab`. No running game was restarted. The user's tournament SHA-256 remains `9683D18CECAD6D6C3D57D294CA57CD1B0CFF427C723892502D05958F8F4BC2DF`.

Portable package: `dist/actua96-0.45.0-c813ede96d8b45ae8d9a0f14ee482e7a`. Gaiking's separate `prematch-0450` test executables use the existing `package-0440-final` media under the established validation folder, avoiding another large media copy on its nearly full disk. Its older full packages remain unchanged.

# Previous release - 0.44.0

Human team play now uses the original automatic selection rules, with a native safe-tick handoff and brief manual override. [Five-case playtest and source boundaries](AUTOMATIC-SELECTION.md). Loose balls, defensive retention, receivers/interception, keeper handoff and manual switching passed on both pitch halves, locally and on Gaiking.

The [80-check regression](selection-regression.txt) initially passed 79 checks. The rendered set-piece test waited for open play only after a fixed settling period; a completed throw-in could produce another awarded throw-in during that period, leaving the test waiting for unprovided input. It now records the original restart's exit immediately, retains the settling period and checks the keeper's actual distribution count. All seven rendered cases then [passed](selection-setpiece-recheck.txt), completing coverage of all 80 checks. The full tournament session, match matrix, Back-button protection, human controls and save recovery passed in the full run. No gameplay change was needed to repair that test.

Version **0.44.0** is [published through the usual launcher](selection-published.txt), effective on its next start. The final package passed all six isolated checks [locally](selection-final-package.txt) and [on Gaiking](selection-final-gaiking.txt); the final Gaiking build also passed all seven rendered restart/distribution cases, explicitly observing the subsequent throw-in (mode 11) in the formerly stalled case. The five-group selection test passed separately on Gaiking. The final test-output-only change prints numeric restart IDs rather than character bytes. No running game was restarted, and the user's tournament hash remains `9683D18CECAD6D6C3D57D294CA57CD1B0CFF427C723892502D05958F8F4BC2DF`.

The latest Gaiking package is `package-0440-final` under the existing `Actua96-validation-0431-db8278d27a474896ae7488258a7df14c` temporary directory. Earlier packages are retained; they are not the current release.

## Prior validation checkpoints

The 0.43.2 controller fix removes the old Back shortcut that replaced an exhibition with an action demonstration. Back does nothing on the pitch, live or paused; Start retains pause/resume and Back still closes menus/replays. [Focused regression results](back-button-validation.txt). The user also confirmed F11 fullscreen toggling works; other hands-on checks below remain open. The 0.43.1 validation evidence below is retained with its original version labels.

All eight focused controller/frontend/tournament checks passed in 116.04 seconds. The refreshed 36-file portable package passed all six isolated checks [locally](back-button-package.txt) and [on Gaiking](back-button-gaiking.txt), including virtual-controller Back presses in live and paused play. Version 0.43.2 is [published for the usual launcher's next start](back-button-published.txt); no running game was restarted. Gaiking's separate test copy is under the prior validation directory in `package-0432`; its earlier `package-final` remains 0.43.1.

Updated 2026-10-12. The single-player match and tournament are ready for a combined human validation session. Independent Windows 11 testing on Gaiking passed the package checks, 63 simulated fixtures, save recovery and hidden Direct3D rendering in its active RDP desktop. Physical controller, audio and display checks remain manual. [Gaiking results and scope](GAIKING-VALIDATION.md). This document supersedes the historical remaining-work lists in RELEASE-AUDIT.md.

## Stabilization changes

Tournament Resume now tries `tournament.sav.bak` if the main checkpoint is missing or invalid. The hub explicitly displays **BACKUP LOADED - PREVIOUS CHECKPOINT**, because recovery can restore older progress. Loading does not rewrite either file. S / LB, or the usual subsequent save, writes the recovered progress. A damaged primary is never copied over the valid backup. Valid primary checkpoints still take precedence; invalid primary and backup files report an error without changing already-loaded progress. Older supported save versions and the existing legacy-directory migration remain supported.

The backup-overwrite defect was reproduced with isolated files before the fix: saving over a damaged primary replaced the good backup with that damaged file. [Failing regression](save-recovery-before.txt). The repair follows the validation already used for preference backups. No gameplay source routines, tournament format or save schema changed.

## Complete tournament session

The new session test runs all **31 fixtures through the actual frontend and original CPU engine**, rather than only feeding scores into the tournament table. It enters lineups, advances matches through the presentation/replay scheduler, waits for automatic half-time continuation and natural tunnel completion, opens and returns from the live-match hub, and recreates the frontend to Resume after every fixture. All saved state is serialized again and compared byte-for-byte, including results, squad ordering, fitness, discipline, formations and set-piece takers.

The run completed **62 natural tunnel exits, 37 interval screens, 57 goal replays, 57 match goals, three extra-time matches and two shootouts**. Spain won the final against Russia on penalties after 0:0. These are deterministic automated test outcomes, not historical Euro results or a human playtest. All 31 checkpoints resumed at the correct next fixture, and the completed competition refused further fixtures.

The session advances the real engine at fixed simulation steps without rendering every frame or waiting wall-clock match time. It checks transitions, state and persistence; it does not assess perceived frame pacing, controller feel or real sound-device quality. Existing rendered scenes, input smoke tests and action/physics contracts remain complementary checks. [Session and recovery test output](stabilization-focused.txt).

The recovery tests additionally cover a damaged primary, a missing primary, explicit saving after recovery, both files invalid, preserved in-memory state, and the visible recovery screen. The packaged executable's existing production-directory probe now exercises recovery too, using uniquely named temporary checkpoints beside the real save. It never damages or replaces the user's `tournament.sav`.

## One combined human playtest

Use the usual `Play-Actua96.cmd` for this build when the current game is closed. A new tournament replaces the current competition, so finish existing progress first or copy the save folder if it must be retained.

1. Choose a country and play its tournament matches. Try the restored shot/pass combinations, first touch, aftertouch, substitutions and half-time tactics in normal play. Check responsiveness and ball/animation continuity together.
2. Between completed fixtures, quit and use Resume. Check the next opponent, scores, lineup, injuries, bans and saved options. Unfinished matches restart from kickoff; mid-match saving is not implemented.
3. During a match, pause, switch focus away and back, disconnect/reconnect the controller, and resume. Check that the ball and clock stay stopped while paused and that no held action fires unexpectedly.
4. Fullscreen has been confirmed by the user. Check the display scaling actually used and listen to music, effects, crowd and English/French commentary on the real audio output; try changing the audio output device. Actua films are no longer in normal Euro 96 startup or releases.
5. Finish the tournament through the winner screen. Repeat the portable launch/save/controller/display/audio checks on another Windows 11 PC or account. The portable package's `verify_package.ps1 -Smoke` checks its files and hidden flows, but does not replace that hardware test.

Report any issue with the fixture, match clock, action/control, screen and whether it repeats. Screenshots or a short clip help with animation/audio reports. No separate per-feature extraction or test loop is needed.

## Remaining completeness work (updated for 0.61.0)

- **Original presentation:** all eight retail Euro 96 stadiums, 32 outfield strips, keeper/head art and shirt numbers 1–20 are restored. Normal menus use recovered art/fonts, but some layouts, exact DOS timing and unused retail branches remain adaptations. Animated player/stadium previews, trophy films and remaining video-board dispatch need verified integration. Actua films are excluded from normal Euro 96 startup/releases.
- **Audio and appearance fidelity:** thirty English event categories and 132 score takes now use Euro 96 recordings. Half-time and regulation result calls now select verified retail period recordings. Five source-only categories, exhibition/custom-draw introductions, missing/ambiguous half-time scores, results music and optional French still use compatibility media. Original-draw introductions and country-specific knockout winners are restored through 0.61.0. Retail appearance matches cover 296 preserved roster identities; English names cover 308/320. See the [current media audit](EURO96-TARGET.md) for precise boundaries.
- **Original controls:** the both-button first-time chip/header path is now connected with a documented source correction. Full input parity with every retail version is not claimed.
- **Persistence:** mid-match saves remain absent. Native replay files, completed tournament fixtures and preferences now persist.
- **Multiplayer:** local head-to-head exhibitions now work with two controllers. Same-team co-op, multiplayer tournaments and network play remain separate tasks.

These are distinct from the remaining human and physical-hardware checks needed for a usable single-player release. Further cosmetic additions are not required before doing that validation.

Screens: [recovered checkpoint notice](stabilization-recovery.png), [completed tournament](stabilization-champion.png).

## Final validation and delivery

All **79 CTest checks passed** in 201.15 seconds on the final build: [full regression](stabilization-regression.txt). The [portable package](stabilization-package.txt) verified 36 file hashes and passed all six isolated checks. Its executable explicitly verified damaged-checkpoint recovery and retention of the valid backup. Screenshot flushing and the new production probe's expected checkpoint were corrected during test development; the final complete suite was rerun afterward.

Version **0.43.1** is published through the existing launcher, snapshot `6289c1c8bcd580ca-2b4b128d-0703f4d7-4e7d91ab`. The [published executable probe](stabilization-published.txt) passed; the user's tournament hash and absent preference file remain unchanged. SOURCE remains unchanged. No running game was restarted.

The remaining release validation is the human tournament session and physical controller/audio/fullscreen/display-scaling testing above. Independent-machine automated testing is complete on Gaiking; it does not replace those hands-on checks. The portable package was refreshed with a PowerShell 5.1 verifier fix, and its bundled checker passed all six checks on both PCs. The game executable and launcher snapshot are unchanged.
