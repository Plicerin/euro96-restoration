# Original tunnel exits - 0.42.0

At half-time and full-time, active players now leave the pitch through the recovered stadium's tunnel before the team-talk or results screen. Press **Enter / Space / Escape**, controller **South / East / Back**, or click to skip to that screen. **P / Start** pauses or resumes the exit. Focus loss or controller disconnection pauses it; focus return does not resume automatically.

The usual launcher picks up the update on the next launch. A match already running is not restarted.

## Original behavior and native boundaries

`SOURCE/INTELL.CPP::tunnel_pos` supplies the route: each side first lines up at the tunnel's x coordinate, proceeds from the touchline toward the entrance, and clears `draw_me` near the endpoint. Team B's relative coordinates are converted back to world coordinates, as in the original tactics path. The recovered `init_run_act`, `run_action`, `process_dir` and `process_anims` routines supply movement and animation. Motion capture and existing interpolation render the players.

`SOURCE/3DENG.H::stad_info` defines the tunnel x/y/z integers at descriptor bytes 224/228/232. The matched CD stadium descriptors already shipped in all five venue packs contain `(595,31,-1425)`, giving gameplay x/y `(595,1425)`. The unused height expression in the original route helper is omitted. No new stadium geometry or player animation was invented.

`OriginalExit` holds a separate engine copy. It runs at 20 Hz while the live match stays frozen, preserving its player statistics, clock, RNG, ball state, substitutions, score and replay history. Live `get_target` tunnel dispatch remains guarded: the native period scheduler calls the recovered route on the presentation copy instead. The camera framing, skip/pause UI, transition timing and 45-second fallback timeout are native presentation choices, not claims of exact DOS cutscene behavior. Tested exits complete naturally in about 14 seconds without reaching that timeout.

The existing period/results screens remain the next step. Skipping an exit opens its screen; it does not also skip the team talk or start a new fixture. CPU half-time auto-continuation starts its usual countdown after the exit. Extra-time changes of ends do not invent a tunnel break. Tournament results are recorded at full-time using the existing checkpoint path, before the optional exit, and cannot be recorded again by skipping it.

The exit hides match-only ball/selection/radar graphics and officials whose tunnel route has not been recovered. Menu/result music waits until the exit finishes. Original half/full-time stadium-board animations continue during the exit, and pause with it.

## Validation

The tunnel contract exercises all five CD venues in both playing directions, with a sent-off player excluded. Every remaining player reaches the original endpoint: 279 ticks before the end swap and 282 afterward. Every step checks that live player bytes, statistics, RNG and match time remain unchanged. Tests cover pose pause, focus loss/return, keyboard/gamepad skipping, team-talk continuation, full-time/results, rematch and the extra-time interval exclusion. The executable front-end smoke also traverses both exits.

Screens: [players approaching the tunnel](tunnel-100.png), [later exit](tunnel-200.png), [pause/continue controls](tunnel-controls.png). Generated source provenance is recorded under `tunnel_exit_042` in [the recovery report](player-recovery.json).

All **74 regression checks passed** in 212.10 seconds, including the match matrix and tournament continuity: [full results](tunnel-regression.txt). The final follow-up handles focus loss immediately even if focus returns in the same frame, pauses on active-controller disconnection, and keeps named menu captures focused on their requested screen. The final binary passed [all three affected checks](tunnel-final-tests.txt), including actual SDL focus events and virtual-gamepad resume/skip.

The [portable package](tunnel-package.txt) passed all five isolated checks. Published **0.42.0**, snapshot `891d7ee29468dbb1-2b4b128d-0703f4d7-8b1ab7f2`, through the existing launcher. The [published save/preferences probe](tunnel-published.txt) confirms that the user's tournament hash is unchanged and preferences remain absent. SOURCE is unchanged. No interactive game was restarted.

## Remaining presentation work

This restores period-end player exits, not pre-match entrances, referee exits or individual substitution/sending-off walk-offs. The short startup GDV still has decoder errors. Exact DOS menus are not reproduced.

The unused stadium clips were decoded for inspection. `BOOST` is an authored advertisement, commented out as an alternative to the goal clip in `3DENG.C::do_goal_anim`; it is not a missing gameplay event. The three two-part `WIN` clips are authored winner animations with a first section followed by a loop. Their player routines exist, but no verified gameplay trigger was located in the supplied source. They remain unconnected rather than assigning them speculative events.
