# Restoration closeout — 0.70.0

The Windows restoration has a playable original-engine match and tournament loop, original Euro presentation assets, saved progress/replays, and local two-controller exhibitions. This does not certify exact parity with every DOS retail behavior. Outstanding fidelity differences below remain visible work, rather than being silently declared accepted.

## Latest presentation corrections

The 105-minute break now uses the half-time board, as the preserved match source does. Country-carousel movement uses the source frontend's nominal 30 ms loop cadence instead of the former 60 Hz native pace. Input selection remains immediate; fades retain their separate 63 Hz cadence. [Evidence and limits](PERIOD-PRESENTATION.md).

## Latest audio restoration

The two formerly unmatched live pass events are resolved: short and long passes share the existing Euro CPA bank, and the two special set-piece reception branches are silent, as in the retail executable. The adapter applies this only with the retail English event pack; older English/French behavior and gameplay are preserved. All media bytes are unchanged. [Pass routing evidence](RETAIL-PASS-AUDIO.md).

The earlier GL_POST-to-POS mapping restored eight Euro rebound-goal recordings. Two other formerly listed gaps have no active direct source callers. [Rebound recording and caller evidence](REBOUND-AUDIO.md).

## Presentation fixes from 0.67.0

The fade generator now recovers the frontend's nominal 63 Hz timer registration from `SOURCE/TEST.CPP`, verifies the `AddTimer` macro in `SOURCE/TIMER.H`, and combines it with the existing 25-step fade curve. A fade phase is 25/63 seconds (about 0.397 seconds), independent of monitor refresh. Source files are untouched. The RGB composition, input guard, palette quantization and interrupt jitter remain native adaptations. Country-carousel timing was investigated separately in 0.70.0 and now uses its own source-derived 30 ms main-loop cadence.

Statistics screens now leave a clear gap between the period heading and the score. The right-hand interval panel uses the actual period, including extra-time entry and the extra-time interval, instead of always saying HALF TIME. Scores, match results, timers, player state and gameplay rules are unchanged.

[Recovered fade evidence](closing-fidelity-fade-source.json), [full-time capture](closing-fidelity-full-time.png), [extra-time capture](closing-fidelity-extra-time.png), [focused checks](closing-fidelity-focused.txt).

## Remaining fidelity work

| Item | Established evidence | Current treatment / next requirement |
| --- | --- | --- |
| Retail penalty board | Record 7 contains 20 valid frames; the next frame attempts a literal write through pixel 10442 of a 10240-pixel image. | Safe preserved-source fallback. A verified intact asset or justified reconstruction is needed; no clipped or guessed repair was imported. |
| Winner boards and other board timing | Winner records decode, but all 36,631 LE relocations reveal no handler pointers; direct calls remain confined to their dispatcher. The same audit finds both real gameplay goal-board calls. | Winner behavior is unverified / possibly dormant, not a confirmed missing feature. Keep it unwired without a proven trigger. Computed/aliased accesses remain outside the proof. [Audit](WINNER-BOARD-AUDIT.md). Extra/full-time and scheduling differences remain. |
| Rare English commentary | Eight duplicated HLF filenames contain two unverified score recordings; missing/ambiguous half-time scores retain compatibility handling. Two dormant pass categories remain available in compatibility data. | Identify the half-time recording scores before replacing them. The former live pass-category gaps are resolved in 0.69.0; GL_POST was resolved in 0.68.0. Full retail scheduler parity is not claimed. |
| French commentary | Matching Euro French recordings are absent from the supplied disc. | Optional Actua French compatibility pack; matching media is needed for Euro-specific fidelity. |
| Exact frontend behavior | Original art/fonts and source-derived fade steps/cadence are present; navigation, extension screens and some animation timing are native. | A complete retail interaction comparison remains necessary for exact parity. |

[Reproducible board-data and caller audit](retail-board-limits.json), [media inventory](EURO96-TARGET.md), [audio boundaries](RETAIL-IDENTITY-AUDIO.md). The board audit includes all recorded relocations and a positive goal-caller control; it still does not claim that the winner artwork was definitively unused.

## User validation still pending

The physical two-controller session and audible audio-device switching remain untested. Automated virtual-controller, PCM and dummy-device checks do not substitute for these. They can be checked together during a normal play session using the [combined checklist](COMPLETION-CHECKLIST.md#one-combined-hands-on-check).

Online/LAN play, same-team co-op, keyboard-versus-controller matches, multiplayer championships and mid-match save states remain outside the agreed release scope. Kuznetsov's single Actua name tone remains the previously accepted immaterial omission.

Final build, package and launcher results are recorded in [release status](RELEASE-CANDIDATE.md).
