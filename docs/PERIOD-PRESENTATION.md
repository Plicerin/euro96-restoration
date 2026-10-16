# Period boards and country-selection timing — 0.70.0

The break at 105 minutes now displays the half-time stadium animation, including the retail Euro branding when available. Previously the observer used `interval_minute >= 90`, incorrectly replaying the extra-time-entry animation at that break. `SOURCE/FOOTBALL.CPP::watch_match_time` calls `do_half_time_anim(0)` at both 45 and 105 minutes; only the tied knockout break at 90 calls `do_extra_time_anim(0)`. Final-whistle priority and the existing extra/full-time fallback artwork are retained. Repeated observations of the same break do not restart the board.

Country selection now advances through its original poses at the preserved frontend's nominal 30 ms loop cadence. `DEFINES.H` supplies a 200 Hz timer (`10 * 20`); `TEST.CPP` registers `nethandler`, waits for `count >= TIMER_SPEED/30`, clears the counter, and calls `DoTeamSelection` once. The integer division yields six interrupts, or 30 ms. `NETWORK.C` increments the counter unconditionally. `TIMER.H` passes the rate through to the timer registration. The earlier Windows adapter used 60 steps per second, almost twice this nominal rate.

The generator now checks that evidence and emits the cadence instead of hard-coding a display rate. The original direction, acceleration and 64 authored poses remain. Team choices still respond immediately; only their visible carousel movement changes. Fade timing continues to use its separate 63 Hz interrupt. DOS execution delays and interrupt jitter are not reproduced, and this is source evidence rather than a measured retail-executable cadence.

Tests cover the 45/90/105-minute board selection with and without the retail pack, full-time priority, repeated observations, gameplay-state isolation, all 256 country transitions, the expected halfway pose at 30/60/144 Hz, fractional time accumulation, rapid retargeting and focus hold. The provenance check rejects changed timer registration, counter increment, loop divisor and timer macro evidence.

## Remaining limits

The supplied retail board handlers use IDs 1 and 3 for red/yellow cards, whereas the preserved source uses them for extra/full time. Reusing those IDs for period graphics would display cards. The existing relocation/caller audit still provides no verified retail period/winner trigger; the source-derived period routing remains a compatibility treatment. Board playback stays at its existing native 20 Hz. No damaged penalty frames, speculative winner triggers or new artwork are imported.

Exact retail menu navigation, selection-button animation timing, board scheduling, and ambiguous half-time commentary remain open comparisons. This batch does not change gameplay, imported media, audio routing or save formats. Physical two-controller testing is deferred at the user's request.
