# Restoration target: UEFA Euro 96

Agreed target: the PC/DOS UEFA Euro 96 game. The GitHub repository's `actuasoccer96` name does not establish the identity of its preserved source. The working restoration uses the Euro 96 gameplay code, squads, tournament structure and menu archives. Windows rendering, audio output, input, storage and portions of frontend navigation are native adapters.

Version 0.55.1 corrects the window title, diagnostic header, error dialog and launch instructions. `Play-Euro96.cmd` is the preferred development launcher; `Start-Euro96.cmd` is the portable launcher. Existing Actua-named shortcuts remain compatible. The executable, repository/build identifiers, save directory and replay format retain their old internal names to avoid breaking existing installations. This is a naming and audit update; it does not claim the remaining Actua assets have already been replaced.

Current release scope and open gates: [completion checklist](COMPLETION-CHECKLIST.md). Kuznetsov's single fallback tone is accepted and closed.

## Current provenance

| Area | What runs now | Remaining Euro 96 work |
| --- | --- | --- |
| Gameplay, squads, tournament | Preserved Euro 96 revision with documented native adaptations | Retail revision differences require explicit evidence; do not silently replace saved squad identities |
| Normal menus, fonts, country carousel, HUD | Preserved Euro 96 archives | All previews restored; source-derived 63 Hz fades and 30 ms carousel cadence. Exact retail navigation, other animation timing and event behavior remain adaptations |
| Outfield kits, player/keeper textures, shirt numbers | Retail Euro 96 overlays | The older Actua kit pack still supplies base/fallback data, including crowd palettes; all 320 player appearance mappings are now verified |
| Menu music | Retail Euro 96 track | None for the menu track; old Actua MENU.WAV remains fallback |
| Startup | Direct entry to Euro 96 frontend; Actua films excluded from normal menus/releases since 0.56.0 | [Source-based startup decision](EURO96-STARTUP.md); exact retail executable startup not fully traced |
| Playable stadiums | Eight retail Euro 96 venues; new championships use the original 31-fixture schedule (0.64.0) | Old championships retain their selected venue; five Actua venues retain existing save/replay IDs and media fallback. [Schedule evidence](TOURNAMENT-VENUES.md), [rendering boundaries](EURO96-STADIUMS.md) |
| Pitch effects/crowd audio | Retail Euro 96 bank and verified event mappings since 0.58.0 | Samples match the older bank; the retail foul reaction is slightly quieter. [Evidence](EURO96-AUDIO.md) |
| English commentary | Euro 96 country intros, 31 imported event banks, 132 score takes, 282 period recordings and tournament/winner sequences; verified shared short/long-pass and silent special set-piece routes | Missing/ambiguous half-time scores retain compatibility handling; two dormant pass categories remain addressable. Exhibitions/custom draws follow the retail welcome restriction. [Pass routing](RETAIL-PASS-AUDIO.md), [rebound mapping](REBOUND-AUDIO.md), [sequence routing](EURO96-SEQUENCES.md) |
| English names | All 320 players voiced: 959 retail Euro tones and one Actua tone | Kuznetsov normal tone fallback accepted by the user; no outstanding name-coverage blocker |
| French commentary/names | Actua Soccer | Supplied Euro 96 image has no French banks; a matching language edition or deliberate optional-compatibility policy is needed |
| Stadium video boards | Ten verified retail streams for idle, fouls, goals, half-time branding and cards (0.66.0) | Damaged penalty record, extra/full-time parity and exact scheduling remain. Winner handlers have no established gameplay trigger after full relocation analysis; possibly dormant, not a confirmed missing feature. [Winner audit](WINNER-BOARD-AUDIT.md), [other evidence](FIDELITY-BATCH.md) |
| Result music | Retail Euro theme resumes after matches (0.65.0) | Actua win/lose/draw stings are fallback only when Euro music is absent. [Verified return path](RESULT-MUSIC.md) |
| Animated previews/trophy films | All 320 player previews (0.62.1), eight venue previews and all 32 trophy/credits films (0.62.0) | All 23 previously unresolved identities are verified; preview pacing and menu placement use native adaptation. [Evidence](EURO96-PREVIEWS.md) |
| Old title files | Actua TITLE.IDX/PAL excluded from releases since 0.56.0 | Originals retained locally for preservation; normal frontend uses Euro 96 artwork |

[Per-file inventory and SHA-256 hashes](euro96-target-asset-audit.json) distinguishes active, mixed and fallback media. It covers 34 current optional media files plus four retired local imports, not every texture inside the preserved archives. Shared bytes do not by themselves establish the same complete retail revision.

## Replacement order

1. Completed in 0.56.0: remove the unsupported Actua film startup and enter the Euro 96 frontend.
2. Completed in 0.57.0: restore all eight retail Euro 96 playable stadium asset sets.
3. Pitch audio, 30 mapped English event categories and score variants restored in 0.58.0. Half-time and regulation result dispatch added in 0.59.0. Original-draw group/semi/final introductions and knockout winners added in 0.60.0. Quarter-final introductions added in 0.61.0. Result-to-frontend music routing restored in 0.65.0. Exhibition/custom-draw welcome restriction restored in 0.66.0. Rebound-goal recordings bring the imported total to 31 in 0.68.0. Short-pass/set-piece routes are resolved in 0.69.0 without new media. Remaining recording gap: missing/ambiguous half-time score labels; two other pass categories have no active direct source callers. Exact retail scheduling is not claimed.
4. Completed in 0.62.0: verified player/stadium previews and championship trophy/credits sequences.
5. Remove superseded Actua fallbacks once their remaining uses are covered; document the French-language boundary explicitly.

Replacements should use original verified media and the original engine wherever possible. No generated substitute artwork, second gameplay engine or implicit roster/save migration is part of this target change.

## Evidence

- [Initial Actua disc identity and compatibility limits](CD-INSPECTION.md)
- [Euro 96 disc identity and inventory](EURO96-CD-INSPECTION.md)
- [Euro 96 menu archive restoration](MENU-FIDELITY.md)
- [Retail kits and verified-identity speech mapping](EURO96-IMPORT.md)
- [Retail heads, keeper art, country intros and menu music](EURO96-PRESENTATION.md)
- [Retail Euro 96 stadium recovery](EURO96-STADIUMS.md)
- [Actua stadium provenance](STADIUM-RECOVERY.md) and [five-venue import hashes](cd-venues-recovery.json)
- [Actua intro provenance](CD-INTRO.md)

Earlier release notes describe historical states. This document is the current target and remaining-media audit.

Historical 0.55.1 validation: all three focused checks and eight isolated package checks passed; 43 package hashes verified. Both launcher aliases match. The user save and 427 tracked SOURCE files are unchanged. [Release evidence](euro-target-release.json). No running game was restarted.
