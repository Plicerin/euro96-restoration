Historical release record. For current completion, compatibility boundaries and test gates, use [COMPLETION-CHECKLIST.md](COMPLETION-CHECKLIST.md).

# Release audit - 0.37.1 (historical)

For the current 0.43.1 status, use [RELEASE-CANDIDATE.md](RELEASE-CANDIDATE.md). The finish lists below describe earlier builds and are retained as historical evidence.

**0.43.0 presentation update:** [the original startup intro](CD-INTRO.md) is restored with all 399 frames and original PCM. It runs before the title and can be replayed with I / North. Startup-intro decoder errors are resolved; historical claims below that it remains blocked are superseded. High-colour movie variants, pre-match/referee transitions, individual walk-offs, replay files and unmatched assets remain separate work.

**0.42.0 presentation update:** [original period-end player tunnel exits](TUNNEL-EXITS.md) now precede half-time/full-time screens. Pre-match/referee transitions and individual walk-offs are still open. `BOOST` is a disabled advertisement alternative, not an absent gameplay event; the authored winner-board clips still lack a verified source trigger. Earlier "tunnel unconnected" statements below describe the historical audit.

**Current update (0.41.0):** the human-control batch, replay controls and complete making-of film below are implemented. Original French events, introductions, scores and matched player names are now selectable and saved: [French commentary](FRENCH-COMMENTARY.md). The numbered finish list and validation totals below describe the historical 0.37.1 audit. Remaining release validation is a combined human tournament session and independent Windows 11 hardware/account testing; the short intro, tunnel/menu presentation, replay files, asset mismatches and multiplayer are still open.

Audit date: 2026-10-11. Scope: a usable Windows single-player restoration using the preserved Euro 96 source and the supplied Actua Soccer CD. This audit does not treat the earlier practice demonstrations as the finished match engine, and does not equate a routine count with a completion percentage.

## Assessment

The project has a working single-player match and tournament loop. The most substantial outstanding gameplay question is **original human-control coverage**: live input currently exposes ground pass, loft, a driven shot and tackle, while the original source contains additional input combinations and ball-control behavior. CPU coverage does not establish that those human actions are available. Presentation completeness and testing beyond this development PC remain open too.

For a first single-player release candidate, plan one focused human-control batch followed by a stabilization/hardware-validation pass. Full retail parity has a separate, open-ended list below, including assets that this particular CD does not supply. The earlier estimate of 2-4 batches was a planning range, not a measured completion percentage or a promise of full parity.

## Five audit areas

| Area | Evidence and result | Limit |
| --- | --- | --- |
| Original action dispatcher | All 16 active cases in ACTIONS.CPP do_action call corresponding restored handlers. TURN_ACT is commented out in the original too. [Machine-readable comparison](release-action-dispatch.json). | Handler dispatch is a narrow structural check; it does not prove every branch, input combination or original system is present. |
| Broad match simulation | 32 additional CPU fixtures cover all 16 countries, all 10 formation tables, all four formats, all wind strengths and varied rule/referee settings. Includes complete 10- and 90-minute-length matches. All finish; each tick checks ball/player states, identities, animation samples and restart progress. [Matrix output](release-match-matrix.txt). | Sampled combinations, not every matchup/configuration. Fifteen action IDs and 99 animation IDs were observed; rare actions still depend on targeted contracts. Human input is not exercised by this matrix. |
| Portable packaging | New folders for every build, executable/source version agreement, mandatory SDL license, required archives and an explicit SHA-256/size inventory. Missing optional CD media is listed. | Checks integrity rather than authenticity; optional media being present does not mean every original asset is available or wired up. |
| Isolation and negative checks | Hidden version, archive, front-end and save probes run from a copied package outside repository ancestors, with a different working directory and spaces in its path. No data/media overrides. Valid fixture accepted; modified, missing, stale/unlisted, incomplete, duplicate and escaping paths rejected. [Package smoke](release-package-smoke.txt), [negative checks](release-package-contracts.txt). | Same Windows PC, dummy video/audio for automated checks. Real display/audio-device compatibility still needs separate review. |
| Usable instructions and finish list | Portable quickstart rewritten for current controls, tournament saves, options and supported limits. This document supersedes historical checkpoint notes as the current finish list. | Older batch documents describe their release at the time; their pending-work lists are historical. |

The matrix produced 73 match goals, seven shootouts, and two fixtures reaching extra time. These are deterministic test outcomes, not historical scores. It observed 15 action IDs; the sixteenth supported dispatcher case is STEAL_ACT. Source trees remain unchanged.

## What is already connected

- Original AI, physics, actions, collisions, tackles and ball interaction within native match scheduling; regulation, restarts, fouls, offside, cards, goalkeeper handling, substitutions, extra time, golden goal and penalties.
- Original 16-country rosters and attributes, ten formation tables, persistent tournament squads, fitness/injury/discipline continuity, preferred takers and half-time tactics.
- All 31 tournament fixtures, original standings/pairings, real-engine simulation of skipped fixtures, saving and resuming completed-fixture checkpoints.
- Recovered textured players, keepers and officials, kit-clash selection, five CD venue sets, sky/pitch/stadium, original motion capture with native interpolation, cameras, stadium event screens, crowd/music/effects/English commentary and matched names.
- Keyboard or one active gamepad, contextual HUD/set-piece indicators, pause/focus-loss handling, replays, configurable match/audio settings, protected saves/preferences with backups and a stable playtest launcher.

These do not imply binary compatibility with DOS saves, exact DOS renderer/menu reproduction, or identical native scheduling/audio mixing. Relevant boundaries are described in [match depth](DEPTH-RECOVERY.md), [presentation](PRESENTATION-BATCH-32.md), [names/venues](PRESENTATION-BATCH-33.md), [tournaments](TOURNAMENT-RECOVERY.md), [continuity](TOURNAMENT-CONTINUITY.md), [tactics](TACTICS-MANAGEMENT.md), and [options](MATCH-SETTINGS.md).

## Update after this audit

Version 0.38.0 implements the next human-input batch: original held-button shot/pass combinations, first-touch requests, crossing, aftertouch and help. See [the implementation, validation and remaining source-branch limits](HUMAN-CONTROLS.md). The 0.37.1 results below remain historical evidence; this does not turn sampled coverage into complete retail parity. Combined human playtesting and independent Windows validation remain open.

Version 0.39.0 extends [replay controls](REPLAY-CONTROLS.md) with original speed levels and manual ending behavior, exact frame stepping, seeking and optional looping. Replay-file persistence and exact DOS menu reproduction remain open. The historical replay gap below describes 0.37.1.

## Concrete finish list at 0.37.1

Subsequent presentation update: 0.40.0 adds the [complete CD making-of film](CD-MOVIE.md), with native playback and original audio. The startup intro still produces decoder errors; full movie parity is not claimed.

### Before calling single-player release-candidate ready

1. **Original human inputs.** Trace USER.CPP, CONTROL.CPP, BALLINT.CPP and the manual against native/original_team.cpp and the live input path. Verify additional shot/pass combinations, first-touch/aerial decisions and aftertouch. Restore only evidenced behavior. Acceptance: an explicit mapping of original controls to keyboard/gamepad, meaningful action/release tests, and one combined human playtest. Existing driven-shot input is a known simplification; this audit does not claim the other branches are complete.
2. **Windows session validation.** A portable launch on a separate Windows 11 machine/account; real fullscreen/windowed resizing and high-DPI display; controller unplug/replug and focus changes; audio output changes; save/backup/restore from the installed location. Acceptance: no missing runtime dependencies, unreadable controls, unintended input after focus loss, lost settings or lost tournament progress. Existing virtual-controller and hidden SDL checks remain useful but do not replace this.
3. **Final stabilization.** One normal human tournament session, the full automated suite, the extended matrix and the isolated package checks on the candidate. Review any crash/audio/visual reports together. No new cosmetic feature is required merely to fill a batch.

### Remaining original-game completeness, separate from a usable first release

- **Matching Euro assets:** 14 matched CD country kits; Croatia/Turkey still labelled substitutes. Exact CD appearance/name matches cover 181/320 Euro roster players. A matching Euro 96 asset set is needed to remove those gaps faithfully. The supplied Actua Soccer CD is a different roster/version; inventing textures or names would not restore them.
- **Presentation:** four GDV files are inventoried but have no full-screen movie player. The original tunnel presentation is unconnected; native period transitions handle half-time. Stadium event video already works and is distinct from the GDV intro files. Original menu transitions and some stadium board clips remain incomplete.
- **Replay depth:** current manual/goal replay uses a native snapshot buffer sized from the original 15-second/20-Hz design. Full DOS replay controls, scrubbing and replay-file persistence are absent.
- **Audio parity:** French CD audio is not exposed as a native language option. Some DOS scheduling/panning behavior is replaced by the native mixer. English event commentary and matched player names are already wired.
- **Multiplayer:** local two-player and network play are not implemented. Multiple connected pads can select the active device, but cannot control both teams independently. This is a separate scope, not a small final polish item.

## Guard review

Generated player_original.cpp still has guards for unsupported actions, the old collect_controlled_ball diagnostic helper, and the tunnel target. The two-player fixture helper rejects upward-header control, but the full match uses original ball_interact rather than that restricted collection helper. Therefore that fixture guard is not evidence that full-match CPU headers are missing. The dispatcher default was not reached by this matrix; that is evidence for the sampled matches only.

## Reproducing this audit

- Run the normal CMake build and CTest suite; release_match_matrix is included.
- Run `python tools/audit_release.py SOURCE <build>/generated docs/release-action-dispatch.json`.
- Run `./tools/test_package.ps1` for corruption/path/completeness fixtures; these contain placeholder bytes and are never executed.
- Run `./package-windows.ps1 -BuildDirectory <build>`; use its freshly returned package path.
- Run `./tools/verify_package.ps1 -PackageDirectory <package> -Smoke`. It retains its isolated copy/logs in Temp and uses unique save-probe files, not the user's competition/preferences.

The package is available for an independent-machine test. The regular Play-Actua96 launcher remains the day-to-day path; no extraction or per-fix manual test is required for this audit batch.

## Final validation and published build

All **67 CTest checks passed** in 205.83 seconds. [Full run](release-regression.txt). The final follow-up changed only version-probe argument handling so packaging can safely reject an older executable without an interactive error dialog; the final binary then passed all four isolated package checks again. [Final package check](release-package-final.txt).

Published playtest snapshot: `a8e6679cbe579e6b-2b4b128d-0703f4d7-65e2336c`, version 0.37.1. Launcher and media hashes passed. Its exact executable passed the production-directory save/preferences probe; the user's existing competition hash was unchanged and preferences remained absent. [Published check](release-published-check.txt). The portable package contains 32 inventoried files plus its manifest, with no optional media missing from this installation. No interactive game was restarted.
