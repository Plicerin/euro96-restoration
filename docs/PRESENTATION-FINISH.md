Historical release record. For current completion, compatibility boundaries and test gates, use [COMPLETION-CHECKLIST.md](COMPLETION-CHECKLIST.md).

# Original presentation audit and finish - 0.50.0

Frontend screens now fade up using the original 25-step linear brightness curve. The referee and assistants remain visible while players leave at half-time/full-time, with original period-end actions. No additional controls or separate playtest are needed; review these during the next normal match.

## What the source establishes

`RULES.CPP::process_ref` explicitly requests the stand animation during `SWAP_ENDS`. `process_lman` continues an assistant's existing target movement, or calls `lman_follow`, which stands whenever match mode is nonzero. `3D_UPD2.CPP` emits all three official render records during this period. The previous restoration hid them; that exclusion is removed. Original motion capture, movement and native interpolation remain in use.

`tools/restore_prematch.py` extracts the referee's period-end branch and the assistant dispatcher. They run on the existing exit presentation copy. `SWAP_ENDS` is scoped to that official update: the native player-exit adapter still calls `tunnel_pos` directly, avoiding the deliberately guarded live tunnel dispatcher. Match players, officials, RNG, statistics and time remain unchanged throughout the presentation.

The earlier remaining-work list incorrectly assumed missing officials' tunnel entrances/exits. The supplied `init_officials` starts them on the pitch, and the period-end referee branch stands. No new tunnel walk is assigned to them. This is evidence about the supplied source revision, not a claim about every retail executable.

`EURO_DEF.H` defines `FADE_COUNTER` as 25. `EURO_GEN.CPP::EUROinterupt` adds each palette channel divided by that counter on each fade-up interrupt, then restores the exact original palette. The native frontend uses that linear progression to fade the entire composed menu, including its text and panels. It finishes at the exact unfaded pixels. The SDL adapter uses 60 presentation ticks per second (about 0.42 seconds); this timer rate, RGB alpha quantization and input policy are native. DOS palette-port writes, fade-down transitions and exact original menus are not claimed restored.

Focus loss freezes a fade. Navigation immediately restores brightness without delaying or consuming an existing menu action. A new screen starts its own fade. Match rendering, movies, pause help and on-pitch presentation overlays do not fade. Named still-image captures omit transitions so they continue to show the requested screen.

The three authored WIN board sequences still have no verified game-event entry point: source references are their definitions and renderer spool continuations. They remain unassigned. BOOST is the previously identified alternate advertisement. [Audit with file hashes and call sites](presentation-finish-audit.json), [generated recovery evidence](presentation-finish-source.json).

## Validation

The new presentation contract renders title, selection, lineup, half-time and results, checking a black start, intermediate image brightness, focus hold, exact final pixels, navigation and unchanged live match state. It independently exercises the referee stand branch, idle assistant stand, an assistant already moving to a target, animation/pause and live-official isolation.

The existing exit test now requires all three official models alongside players, and checks live official bytes throughout exits in all five CD stadiums in both directions. Existing frontend and head-to-head smoke tests continue to exercise application input and transitions. [Focused checks](presentation-finish-focused.txt).

Reviewed screens: [partial fade](presentation-finish-selection-fade.png), [fully visible setup](presentation-finish-selection.png), [result](presentation-finish-result.png), [officials during player exit](presentation-finish-exit.bmp). Final regression, package and independent-machine evidence is recorded in [release status](RELEASE-CANDIDATE.md). Physical two-controller head-to-head validation remains pending at the user's convenience.
