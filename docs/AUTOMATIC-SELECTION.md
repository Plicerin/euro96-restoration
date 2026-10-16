# Automatic player selection - 0.44.0

Automatic selection is enabled in human full-team play, including exhibitions and tournaments. E / controller East (Xbox B) / LB / RB remains a manual override. The selector never chooses an opposing player, and CPU-versus-CPU mode stays entirely CPU-controlled. There is no new menu setting in this batch.

## Controller audit and correction - 0.51.1

Automatic selection remains enabled by default in human full-team matches. There is no on/off preference. Two complete human-input match diagnostics, without any manual switching, produced 166 and 185 selection changes respectively: 34/47 while defending and 103/115 while the ball was loose. These runs did not reproduce a disabled selector; they do not establish what happened in the user's particular session.

The controller audit did reproduce a conflicting binding in the actual SDL event route: LB switched players, but RB changed a human exhibition into CPU control of both teams. RB was also swallowed during human tournament matches. Both shoulders now switch in human play, including tournament matches. RB cannot give a human team to the CPU or change the selected player while paused. It still takes control of a watched exhibition while playing; watched tournaments retain their existing takeover restriction. Match setup's Watch option and the exhibition keyboard B command still allow deliberate spectator play.

Pause now identifies human team play with `AUTO SELECT ON` and spectator play with `CPU v CPU`. This is a status label, not a new toggle. At match setup, North / Xbox Y selects human play; South / Xbox A selects Watch.

The original selector still checks every 12 logic ticks (about 0.6 seconds), prioritizes intended receivers/interceptors and retains a nearby selected player inside its selection circle. It does not instantly select whichever player is nearest on every frame. Its rules and gameplay source were not changed by this correction.

[Reproduced RB failure](selection-rb-before.txt), [focused controller/frontend/tournament checks](selection-rb-focused.txt), [full-match diagnostic results](selection-rb-matches.txt), [pause status screenshot](selection-rb-paused.png). The existing selection contract now includes both full-match diagnostics after actual kickoff, without forced reselection events or manual-switch calls. The SDL smoke covers RB selection, pause protection and spectator takeover; the tournament contract checks input routing in human and watched fixtures. Final validation and publication are recorded in [release status](RELEASE-CANDIDATE.md).

## One combined playtest

Use the usual launcher on its next start; existing running games keep their current build.

1. **Loose ball:** let a clearance travel away from your selected player. Control should move to a teammate who can reach the ball; possession still requires real contact.
2. **Defending:** follow an opponent. The selector should retain a nearby defender instead of flickering between players, then hand off when play moves away.
3. **Pass receiver:** pass and keep moving. The intended receiver can become selected before receiving the ball. Held direction carries across the handoff; first-touch buttons remain available.
4. **Goalkeeper:** catch and distribute. Keep control during the holding/release animation, then return to an outfield player. Restarts and shootouts retain their existing designated controls.
5. **Manual override:** press LB, RB or B. That choice wins for one selection interval (12 logic ticks, about 0.6 seconds); after that the original selection-circle and receiver rules govern automatic changes.

Automatic selection is deliberately more stable than choosing the mathematically nearest player every frame. An intended receiver, interceptor or predicted-path candidate takes priority over distance; the current player is retained inside the original selection circle unless a receiver or teammate possession calls for a handoff.

## Preserved logic and native integration

`tools/restore_player.py` extracts intact `clear_auto`, `auto_select_a` and `auto_select_b` bodies from the pinned `SOURCE/USER.CPP`. The preserved source remains unchanged. The generator records each extracted routine's source line and hash in `player-recovery.json`.

The original rules exclude fallen/off-field candidates and normally exclude goalkeepers. `BALLINT.CPP::player_distances` supplies the original `sel_circle` flags, and the restored predictor supplies receiver/interceptor/near-path state. Teammate possession retains its immediate handoff in the live match adapter.

The native adapter still owns keyboard/gamepad input and the single human user's selected index. It synchronizes only the `plr` and `chng` fields required by these selectors, not the missing DOS user record's binary layout. It does not enable the DOS multiplayer user registry or unrestored `user_conts` dispatcher. `auto_users_a/b` remain zero in the surrounding legacy AI.

Selection checks use the original `new_users` interval (`ball_travel++ > select_cnt`, with `select_cnt=10` in `FOOTBALL.CPP`): once per 12 ticks. That cadence also polls loose-ball selection in the native scheduler. Original `reselect` notifications request an earlier check at the next safe tick boundary. Manual-switch protection for one interval is a native integration policy; it is not claimed as an original retail feature.

Pending kick/save contact and ball-limbo frames retain control until release. Keeper possession, dead balls, intervals and shootouts use their existing dedicated adapters. Automatic changes reset charge counters and discard a queued discrete action belonging to the previous player, while preserving held direction and first-touch buttons. They do not alter ball position, velocity or ownership.

## Verification

`selection_tests` covers the five groups on both pitch halves, candidate exclusions, circle stability, all three priority levels, possession handoff, CPU mode and an actual live-scheduler switch with held movement. The surrounding team and human-control tests exercise passes, tackles, AI, contact, charge and aftertouch behavior.

[Focused results](selection-focused.txt), [source hashes and integration record](selection-source.json). The same five-group selection executable and all six isolated package checks also passed [on Gaiking](selection-gaiking.txt). Gaiking initially ran out of disk space while making an audit copy; four unused copies created by this task were removed after checking running-game paths. Their logs and all playable versioned packages were retained. The 0.44.0 remote package is in `package-0440` under the existing validation directory.

Further release checks are recorded in [current release status](RELEASE-CANDIDATE.md).

The final package, including the corrected test and numeric diagnostics, is `package-0440-final` on Gaiking. Its six package checks and seven rendered set-piece cases [passed](selection-final-gaiking.txt). The usual local launcher is [published at 0.44.0](selection-published.txt).

The full regression exposed a test scheduling assumption: after a throw-in released, a second throw-in could be awarded before the smoke test's 100-frame settling check. Recording the original release immediately makes the test independent of subsequent play. The corrected test still requires each restart to leave its set-piece mode and a keeper punt to increment the distribution count; all seven cases passed.
