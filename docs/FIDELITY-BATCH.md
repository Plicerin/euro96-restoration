# Presentation, stadium boards and welcome audio — 0.66.0

This batch addresses the three requested fidelity areas. It does not claim exact DOS menu behavior or complete retail media parity.

## Menus and match overlays

The live power indicator and the slim match-help line now use the recovered Euro menu font; the power indicator uses the recovered navy panel and cyan meter. The indicator's gameplay meaning, charge range, visibility conditions and input behavior are unchanged. Its layout remains a native aid, not a claim that the original game used that exact meter.

The menu audit covers title, country selection, lineup, tactics/player attributes, match/sound options, championship entry, group/knockout pages, champion, half/full-time statistics, pause, substitutions, replay controls and replay library. These use recovered artwork and fonts. Navigation, extension pages, help text and timing still include native adaptations. Diagnostic physics/practice pages remain developer tools. [Contact sheet](fidelity-menu-audit.png).

The menu capture fixture now loads the same retail media as the application. Previously it exercised recovered menu art over the source-only stadium/kit fallback, which was unsuitable for judging current retail presentation.

Final review also covers the actual application's [power meter](fidelity-app-charge.png), [pause menu](fidelity-app-paused.png) and [spectator view](fidelity-app-watch.png), captured at 1200 × 830 using the software renderer. Component-only pause/replay fixtures on the contact sheet do not provide a full match background. The updated retail-menu fixture passed its final recheck; these captures do not certify physical GPU, controller or audio-device behavior.

## Retail video boards

The supplied 16-record archive is now decoded with the match renderer's actual sequential format. Its initial frame starts after the 1024-byte size table and uses literal/repeat runs. Following frames use tag 1 for literal/skip or tag 2 for literal/repeat. Terminated streams, not the table's frame lengths, control the match decoder.

This distinction recovers all 73 frames of the sponsored goal animation (record 1); its table declares only 67. The sequence begins with GOAL and continues into the original Vauxhall sponsor artwork. Fifteen records decode within pixel bounds. Record 7 fails after 20 frames and is excluded in full, rather than playing a partial animation or reading outside the frame.

The imported `EURO_BOARDS.BIN` contains ten streams: idle, two foul variants, two half-time Euro branding variants, the sponsored goal, two yellow-card variants and two red-card variants. Palette bytes come from the verified Euro rendering archive, independently of kit/sky palette remaps. Native event observation connects bookings/send-offs to the matching card colors. Retail handler IDs differ from the preserved source: handlers 1/3 select red/yellow cards, not extra/full-time graphics.

The board uses private variant rotation without consuming gameplay randomness; 20 Hz playback remains a native timing adaptation. Copies retained for replay preserve frame and variant state. Legacy venues retain their original board. Missing packs retain the existing behavior; malformed packs fail validation without partially replacing loaded data.

Only the stadium descriptor's existing visible board rectangle is updated. Seven retail venues place their scratch buffer on page 15, which their stand UVs do not reference; no new screen is invented in those stands.

Still open: the damaged penalty record, extra/full-time parity, and exact original scheduling. Championship/ordinary winners (records 2, 12, 13) are decoded for inspection but not connected speculatively. The post-0.68.0 audit checks all 36,631 relocations and finds no extra winner-handler references, while recovering the real goal-board gameplay calls. Winner behavior is unverified / possibly dormant, not a confirmed missing feature. [Winner caller audit](WINNER-BOARD-AUDIT.md). Existing sky/flag/net adaptations remain. [Pinned input hashes, instruction anchors and per-record results](euro96-boards-import.json), [retail artwork](retail-board-records.png).

## Welcome commentary

The verified retail welcome routine returns early at file offsets `0x101bfe` and `0x101c0e` unless this is an original-draw championship. Both branches reach `0x102274`, the routine's exit. The importer now pins these branches as well as the previously verified gates and tournament paths.

With the retail sequence pack installed, exhibitions and custom draws no longer append an Actua welcome phrase. Tournament introductions remain unchanged. French/older installations retain their compatible linked welcomes. Invalid retail fixture indices cannot invent an introduction; a mismatched original-draw fixture retains its documented compatibility fallback.

The sequence pack's WAV bytes are unchanged. Tests cover the actual match reset from championship to exhibition, all 256 exhibition country pairings, exact tournament PCM, missing supplements, French and mute/pause behavior.

At 0.66.0, five source-only categories remained unverified. **Updated through 0.69.0:** both retail goal callers establish GL_POST → POS, and all eight recordings are restored. FP_SHORT and FP_LOOSE have no active direct source callers; their compatibility clips remain available. CP_SHORT now shares the retail CPA bank and CP_SETPIECE follows the verified silent branches. Missing/ambiguous half-time recordings and Euro-specific French media remain. The eight duplicated HLF labels still contain only two distinct WAVs of unverified score identity. [Rebound evidence](REBOUND-AUDIO.md), [pass routing](RETAIL-PASS-AUDIO.md).

Validation and publication results are recorded in [release status](RELEASE-CANDIDATE.md).
