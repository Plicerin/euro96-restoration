Historical release record. For current completion, compatibility boundaries and test gates, use [COMPLETION-CHECKLIST.md](COMPLETION-CHECKLIST.md).

# Presentation fidelity and release validation - 0.53.0

The close substitution/sending-off view now clips actor surfaces and ground shadows against opaque stadium surfaces. Team selection now moves through the original 64 country-carousel poses instead of jumping straight to the selected country.

## Close tunnel view

The original renderer's screen vertices are retained. The Windows compositor additionally keeps reciprocal camera depth for actor vertices and clips their triangles against projected CD stadium faces. This resolves intersecting faces that cannot be ordered by average polygon depth. Texture coordinates are interpolated along clipped edges; degenerate fragments are discarded. Transparent/cutout stadium faces are conservatively excluded from solid occlusion. Only individual walk-off presentation uses this treatment; live match and period-end rendering retain their existing paths.

The close camera looks into the actual opening. A player can therefore legitimately remain visible inside the tunnel. The correction hides surfaces behind the roof/crowd, rather than hiding actors at an arbitrary distance or changing their original route/camera.

The regression exercises 60 natural interior observations across five venues and both halves, on outgoing and incoming substitution legs. Those observations require visible pixels through the entrance. Twenty-five additional probes place the same actor in the opening and behind each adjoining section of the stand: opening visible, flanking crowd opaque. Mathematical cases cover fully hidden, fully visible and intersecting-depth triangles plus texture-coordinate continuity. Existing route, red-card, goalkeeper replacement, input, focus and live-state isolation checks remain active. With clipping temporarily disabled, the crowd probe leaked 80 player/shadow pixels and failed; the enabled renderer contributes zero pixels there. [Old-order failure](presentation-close-tunnel-red.txt).

## Country selection

`EURO_SEL.CPP::CalculateDir` is extracted into the generated asset header. The generator checks the source's initial speed .2, doubling acceleration, maximum speed 1 and 64-pose wrap. The native animator follows those steps at 60 display ticks per second; the exact DOS frontend cadence is not claimed. All original country sprite positions and palettes are retained.

Choosing a team remains immediate for keyboard/controller/mouse actions; animation does not delay kickoff or modify match state. A new choice can retarget motion, focus hold stops the display clock, and page entry initializes the selected pose. Tests cover all 256 country-to-country transitions, 30/60/144 Hz pacing, a rapid retarget, intermediate frontend pixels and original settled poses.

## Fidelity audit and remaining scope

The extra WIN stadium sequences still have no verified match-event caller in the supplied source. Their functions and spool continuations are present, but the dispatcher is disabled with `if (0)` in `3DENG.C`. They remain unassigned; inventing a trigger would not establish original fidelity. See the earlier [source audit](PRESENTATION-FINISH.md).

Missing matching Euro96 assets remain: Croatia/Turkey substitute kits, incomplete exact roster appearance coverage and back-number artwork 16-20. Windows extension pages retain original-art styling; unused retail/modem branches and exact DOS timing are not a full frontend port. Completed fixtures, preferences and replays persist; mid-match saves, co-op, multiplayer tournaments and network play remain separate optional work.

One normal play session can review this batch: change both teams, navigate settings/tactics, play through a substitution or dismissal, cross half-time, and continue a saved championship. Physical two-controller head-to-head, controller reconnect and actual audio-device behavior still need hands-on verification. Fullscreen has already been confirmed by the user. Automated tests use isolated fixtures and cannot replace those hardware checks.

Final regression, package, save/source-retention and publication evidence is recorded in [release status](RELEASE-CANDIDATE.md). Publication does not restart an existing game.
