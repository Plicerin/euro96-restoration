# Kickoff selection — 0.61.1

The reported opening failure was reproduced before the fix: player 9 remained selected, 274.925 engine units from the ball, while the original centre pair was players 7 and 10. The test fails before the first simulation tick ([failing test](kickoff-selection-before.txt)).

`prepare_restart()` clears control for all actors after placing the original centre formation. `human_restart()` deliberately excludes centre kicks because the preserved routine releases their pass automatically. Consequently the native selection was never reassigned to either centre player. Live `update_auto_selection()` also excludes active restarts. Existing full-match selection tests exercised live selection but did not assert which player was selected at the first rendered frame.

The centre setup now assigns the original taker to the human attacking side, and the closest active outfield player to the defending human side. Local players use their own end-relative team slots. The native user registry follows that choice; a stale manual-selection hold is cleared and the live selection cadence is ready for the first eligible tick after the pass. Original actor control stays neutral during the scripted restart, preserving its timing, recipient, and animation. The original selection-circle, receiver/interceptor priorities, keeper contact protections, and all SOURCE files remain unchanged.

The new regression covers 32 kickoff combinations: either physical side, all four periods, single-player, local head-to-head, and spectator modes. It checks selection before restart animation, the designated centre pass, then exactly one controlled player per human after release. Existing loose-ball, defense, receiver, keeper and manual-priority tests still run, followed by two full matches without manual switching. Separate presentation tests check both natural and skipped intro completion at tick zero.

After the fix, player 7 is selected at distance 10 from the ball. The rendered retail Wembley scene shows the yellow marker at centre ([capture](kickoff-selected.bmp)). These are automated simulations and rendered captures; a physical-controller session has not been performed.

Release verification is recorded in [current release status](RELEASE-CANDIDATE.md).
