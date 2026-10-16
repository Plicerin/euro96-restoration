# Period-end tunnel occlusion — 0.51.2

Update: the remaining individual walk-off camera now has a separate depth-clipping correction in [0.53.0](PRESENTATION-RELEASE.md). The historical scope below describes the original 0.51.2 change.

Players at half-time/full-time followed the original tunnel route, but remained visible over the crowd after entering the stand. The native renderer flushed every stadium face before drawing any player, so the roof could never cover a player.

During a period-end exit, the stadium faces now share the depth-sorted actor pass. Ground shadows precede the stand floors. The existing CD roof and crowd geometry cover players entering the tunnel, without hiding them by an arbitrary timer or changing their route.

## Source and scope

`SOURCE/INTELL.CPP::tunnel_pos` supplies the two team lanes and the endpoint dismissal. All five imported stadiums contain the same tunnel mesh: its floor extends from approximately gameplay y=1117 to 1425; its roof starts near y=1202 at height 61. The tunnel endpoint is x=595, y=1425. The native period-end camera remains unchanged.

This is a correction to the Windows presentation compositor, not a claim of identical retail rendering. No original movement, animation, match logic, source files or CD artwork changed. Live-match rendering retains its existing passes.

The closer individual substitution/sending-off camera is outside this correction. An exploratory shared-pass check still showed partial actor overdraw in that camera; polygon-average sorting alone is insufficient there. That experimental change was not retained. A later fix should use a more precise occlusion treatment for that view.

## Regression evidence

- The exit contract compares rendered pixels with and without one actor at seven positions: on the pitch, at the touchline, approaching the entrance and three positions inside. All combinations of five venues, both halves and both team lanes are exercised: 140 comparisons.
- Approaching players must contribute visible pixels. Interior players must contribute zero pixels over the stadium. With the old draw order, the first interior probe leaks 46 player pixels and fails ([red check](tunnel-occlusion-red.txt)).
- Natural exits still exercise the original route, pause/focus, skip, period transitions and live-state immutability. Late-frame captures now include ticks 230, 250 and 270 as well as 100 and 200.
- Visually reviewed the same natural exit at ticks 200, 230 and 250. At tick 250, the [old frame](tunnel-occlusion-before-250.png) shows players on the crowd; the [corrected frame](tunnel-occlusion-after-250.png) covers them while the player entering the opening remains partially visible below the roof.
- The existing substitution/sending-off regression continues to check original routes, identities and immutable match state; it does not establish correct occlusion in that closer camera.
- Final pixel probes use gameplay y=600, 800, 1040, 1140, 1280, 1340 and 1380. An initial y=400 probe was outside the exit camera's viewport and was corrected. The unrelated replay test now claims a fresh scratch directory atomically: process-relative SDL startup ticks had [collided with a previous run](tunnel-occlusion-replay-fixture.json), leaving seven clips where the fixture expected three. Neither fixture correction changes the game executable.

See [release status](RELEASE-CANDIDATE.md) for final suite/package results and the published snapshot. A running game is not restarted by publication.
