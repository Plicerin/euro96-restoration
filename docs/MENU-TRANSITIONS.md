Current update (0.67.0): the nominal fade cadence is now recovered as 63 Hz from TEST.CPP's timer registration, with the existing 25 source steps per phase. See [closeout evidence and remaining boundaries](RESTORATION-CLOSEOUT.md). The following is the historical 0.51.0 record.

# Original menu fade-down/up - 0.51.0

Frontend page changes now retain the outgoing menu, fade it to black, then reveal the destination with a fade-up. Title, setup, lineup, tactics, options, tournament, replay-library and results pages use this common path. The first menu after match/presentation playback fades in; gameplay, films and on-pitch overlays retain their existing timing.

## Source and native boundaries

`EURO_GEN.CPP::EUROinterupt` subtracts each current palette channel divided by `FADE_COUNTER` during fade-down, then adds the destination palette channels divided by the same counter during fade-up. `EURO_DEF.H` defines 25 steps. The recovery generator checks both arithmetic branches and records their source hashes. SDL render-target textures retain the composed outgoing and incoming pages; a black overlay applies the recovered linear curve. No replacement artwork is created.

Each phase lasts about 0.42 seconds at the native 60 Hz presentation rate. This clock, RGB compositing, the native menu layout and navigation policy are adaptations; exact DOS palette quantization and interrupt timing are not claimed. Settled screens draw directly so fractional window/DPI scaling does not permanently resample their text through the intermediate texture. Match state remains frozen as it did in the existing frontend.

While the outgoing page is visible, the first navigation press reveals the destination and is consumed. Additional input queued before that destination is drawn is also consumed. This prevents a double click, multiple buffered button presses or key repeat from activating an unseen kickoff or option. After the destination is visible, another deliberate press performs the action. During fade-in, input finishes the fade and proceeds normally. F8/F9/F11 remain available throughout. Focus loss holds the transition; normal menu animation resumes when focused. A render-target content reset discards the retained page and rebuilds the current screen with a fade-in.

Named/static captures omit transitions. Existing fixed-frame executable gameplay/movie smoke scripts use immediate menu navigation; the dedicated rendered transition contract exercises the actual transition and input guard, including SDL virtual-controller events.

## Combined automated coverage

- Outgoing image retained exactly at normal resolution, intermediate dimming, fully black handoff, destination brightening and exact settled pixels.
- Focus hold/return, held-key repeats, global display shortcuts and unchanged live players/RNG/ticks.
- Keyboard and buffered-confirm protection before and after the destination is drawn.
- Actual SDL virtual-controller button/release events, pointer double clicks and render-target reset recovery.
- Scaled/letterboxed rendering, exact settled text and restoration of the caller's render target and logical presentation.

The six existing navigation tests now draw destination menus between deliberate actions; their replay, tournament, venue, tactics, settings and save-recovery assertions are retained. [Navigation regression checks](menu-transition-navigation-check.txt).

[Focused tests](menu-transition-focused.txt). Reviewed frames: [outgoing](menu-transition-outgoing.png), [black handoff](menu-transition-black.png), [incoming](menu-transition-incoming.png), [settled scaled menu](menu-transition-scaled.png). [Generated source evidence](menu-transition-source.json). Full regression, packaging and second-PC results are recorded in [release status](RELEASE-CANDIDATE.md). Review can wait until the next normal play session; the physical two-controller head-to-head check remains pending.

## Asset audit

The preserved number atlas was decoded again: its first row contains 1-11 and the second row contains 12-15, with the rest blank. The CD importer already verifies these shared assets byte-for-byte. Numbers 16-20 therefore still require matching artwork; the renderer retains the existing safe body/kit fallback without a back-number overlay. This audit did not generate replacement numbers or change kits/rosters. [Decoded atlas](menu-transition-number-atlas.png), [all source/imported-kit number-region checks](menu-transition-number-audit.json).
