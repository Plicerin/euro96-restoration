# Completion checklist — 0.70.0

Scope: a playable Windows 11 restoration of PC/DOS UEFA Euro 96, using its preserved gameplay engine, with single-player championships and local two-controller head-to-head exhibitions. This is a release candidate, not a claim of exact parity with every retail revision.

## Implemented and covered by automated checks

- Original-engine full matches, AI, goalkeeper actions, set pieces, automatic selection, half-time, extra time, golden goals and penalties.
- Keyboard/gamepad human controls; two-controller opposing teams in exhibition; disconnect/focus protection.
- All 16 countries and 320 preserved squad identities; retail kits, keeper/player artwork, numbers 1–20 and eight playable Euro venues.
- Original-art menus with smooth font scaling, team/tactics/substitution screens, match HUD, animated player/venue previews, tunnel exits, championship trophy and credits films.
- A complete 31-match championship, original per-fixture venue schedule for new competitions, fixture results, player condition/discipline, save/resume and validated backup recovery. Existing saves retain their selected venue.
- Goal/manual replays and saved replay files; preferences and fullscreen support.
- Retail pitch/crowd effects, menu/result theme routing, mapped commentary, tournament introductions, score/period/extra-time/winner calls and complete player-name coverage.

## Release gates

| Gate | Status | Evidence |
| --- | --- | --- |
| Period boards and carousel | Passed: five focused checks; 45/90/105-minute routing, full-time priority, all 256 country transitions, 30/60/144 Hz pose checks and timer-evidence rejection | [Scope](PERIOD-PRESENTATION.md), [focused checks](period-presentation-focused.txt), [source cadence](period-menu-timing.json) |
| Retail pass audio | Passed: 17 CPA takes, twelve real reception cases, shared pending/cooldown behavior, language switching and older-media fallback; six focused checks | [Scope](RETAIL-PASS-AUDIO.md), [focused checks](pass-audio-focused.txt), [retail branches](retail-pass-audio-evidence.json) |
| Rebound-goal audio | Passed: eight complete retail takes, both actual goal triggers, single dispatch and unchanged gameplay RNG; four focused checks | [Evidence](REBOUND-AUDIO.md), [focused checks](rebound-audio-focused.txt), [eight-record delta](rebound-audio-import-delta.json) |
| Final presentation cleanup | Passed: source 63 Hz fade cadence, 25-step handoff, refresh independence, statistics layout and extra-time labels; actual application captures reviewed | [Closeout](RESTORATION-CLOSEOUT.md), [focused checks](closing-fidelity-focused.txt), [capture report](closing-app-captures.json) |
| Presentation fidelity batch | Passed: six focused checks, retail menu fixture recheck, 18-page menu review and actual-app power/pause/spectator captures | [Scope](FIDELITY-BATCH.md), [focused checks](fidelity-batch-focused.txt), [menu recheck](fidelity-menu-final-check.txt), [captures](fidelity-app-captures.json) |
| Duplicate card-banner cleanup | Passed: five presentation/frontend/discipline checks; restored event panel retained in both match views | [Log](card-banner-checks.txt) |
| Fresh match randomness | Passed: fresh starts/rematches, fixed-seed trace/capture reproduction, argument validation and original card thresholds | [Scope](MATCH-RANDOMNESS.md), [application report](match-randomness-cli-report.json) |
| Selection-circle smoothing | Passed: four menu/frontend checks on 0.65.3; all 12 frames reviewed and title text pixel-identical | [Log](selection-circle-checks.txt), [visual review](SELECTION-CIRCLE.md) |
| Selected text contrast | Passed: four menu/frontend checks on 0.65.2; Direct3D title/pause reviewed and white labels pixel-identical | [Log](selected-text-checks.txt), [pixel comparison](selected-text-pixel-check.json) |
| Menu/HUD smoothing update | Passed: 12 relevant checks on 0.65.1; Direct3D pause/title/HUD captures reviewed | [Log](text-smoothing-checks.txt), [visuals and scope](TEXT-SMOOTHING.md) |
| Full local suite | Passed: 109/109 on 0.70.0 in 577.30 seconds | [Full log](period-presentation-regression.txt) |
| Original action dispatch | All 16 active source cases call restored handlers; narrow structural check only | [Audit](final-candidate-action-audit.json) |
| Package verifier protections | Valid fixture accepted; seven corruption/path cases rejected | [Log](final-candidate-package-contracts.txt) |
| Portable candidate | Passed: 54 hashes and nine isolated smoke checks on 0.70.0 | [Package log](period-presentation-package-smoke.txt), [launcher](period-presentation-launcher-verify.txt), [identity](period-presentation-release.json) |
| Gaiking Windows 11 music update (0.65.0) | Passed: full-track result PCM/looping, compatibility music, frontend contracts and native frontend smoke | [Gaiking report](result-music-gaiking.json) |
| Gaiking venue baseline (0.64.0) | Passed: 31 scheduled fixtures/checkpoints, 39 legacy format/venue combinations and a complete watched championship through all eight venues | [Prior venue report](venue-schedule-gaiking.json) |
| Gaiking broader baseline (0.63.0 RC1) | Passed: nine package checks, 31-match tournament, 32-match matrix, save/name/period checks and four desktop Direct3D checks | [Prior Gaiking report](final-candidate-gaiking-report.json) |
| Physical two-controller session | Deferred: user cannot test head-to-head presently | [Combined checklist below](#one-combined-hands-on-check) |
| Audible output and device switching | Pending user hands-on validation | Automated audio checks compare PCM through dummy devices |
| Fullscreen | Previously confirmed working by the user | Current Gaiking physical display/scaling session remains unverified |

## Accepted omission

Kuznetsov's normal-tone name recording uses the verified Actua sample because the Euro disc lacks that WAV. The user explicitly accepted this on 2026-10-14 as immaterial. His other two tones use Euro audio. This is closed, not an open restoration blocker.

## Remaining fidelity differences

These are documented gaps in exact retail parity, not absent basic match/tournament functions. They have not all been declared acceptable by the user.

| Area | Current behavior | What exact parity would require |
| --- | --- | --- |
| Rare commentary | Ambiguous/missing half-time scores retain compatibility handling; two dormant pass categories remain in the packs. Live short-pass/set-piece routes and rebound-goal recordings are resolved. | Eight duplicated HLF labels remain unresolved; do not guess their scores. Full retail commentary scheduling remains an adaptation. [Pass routing](RETAIL-PASS-AUDIO.md), [rebound mapping](REBOUND-AUDIO.md). |
| French language | Optional, functional Actua voice pack | Matching Euro French media, absent from the supplied disc. English remains the verified primary restoration. |
| Frontend behavior | Original artwork/fonts, 25-step fades at nominal 63 Hz and country carousel at the source's nominal 30 ms cadence; native navigation, extension pages, selection-button timing and RGB composition | A complete retail frontend-event/timing comparison. There is no evidence of a missing standalone Euro intro film. |
| Peripheral presentation | Retail idle, foul, goal, half-time branding and card boards; existing flag/sky/net adapters | Penalty record 7 is malformed; extra/full-time parity and exact scheduling remain. Winner paths are unverified / possibly dormant after the full relocation audit, not a confirmed missing feature. [Winner audit](WINNER-BOARD-AUDIT.md), [other boundaries](FIDELITY-BATCH.md). |

[Current media provenance](EURO96-TARGET.md), [audio evidence](RETAIL-IDENTITY-AUDIO.md), [venue boundaries](EURO96-STADIUMS.md), [menu boundaries](MENU-FIDELITY.md). Old release notes describe earlier builds and are not the current backlog.

## Outside the agreed release scope

LAN/online play, same-team co-op, keyboard-versus-controller head-to-head, multiplayer championships and mid-match save states. These are possible future projects, not implied completed features.

## One combined hands-on check

Use the usual Play-Euro96.cmd launcher when convenient; no per-feature extraction is needed.

1. Start a two-controller exhibition. Confirm that each pad controls its own country and retains it after half-time.
2. Disconnect one pad, reconnect/rejoin with Start, then resume. Confirm the other pad cannot take over the vacant side inadvertently.
3. Listen to commentary/effects/music, change Windows audio output, and confirm whether sound follows the new output or needs a restart.
4. Toggle fullscreen, resize, Alt-Tab away and return. Confirm input is neutral on resume and menus remain readable.
5. Resume a championship checkpoint, complete a fixture, close normally, and confirm the next fixture is available on reopening.

Record any failures together. Automated completion does not mark these physical checks passed. Do not restart an existing game to apply this audit.
