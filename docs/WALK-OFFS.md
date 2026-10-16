# Individual walk-offs and referee transitions - 0.46.0

Exhibitions and watched tournament matches now show the original player on/off route. Substitutes run first to the tunnel-side touchline, then into the tunnel; the incoming player returns from there to the touchline. Red cards show only the outgoing route. Referees and assistants move toward the restart targets already supplied by the original rules.

## Combined five-part batch

1. Outgoing substitutes keep their own identity, kit and motion capture until they reach the tunnel.
2. Incoming substitutes use their own shirt and ratings, enter from the tunnel and return to the touchline; goalkeeper slots retain keeper materials.
3. Both direct reds and second yellows show the dismissed player leaving. The team remains a player short; no substitute is introduced.
4. Enter / Space / Escape, South / East / Back, or a left click skips the current transition. P / Start pauses. Focus loss or controller disconnect pauses; focus return does not resume. Held action buttons must be released before play resumes. Queued changes play in order.
5. Referee and assistant restart targets now use the original jogging routine. Referee facing uses the player's actual Y coordinate; coincident targets remain finite.

Use F3 to choose and queue a substitution, then close the squad panel and resume. It takes effect at the next stoppage. The normal launcher receives this batch on its next start; no running match needs to be interrupted.

## Source and adaptation boundaries

- INTELL.CPP `init_off_int` and `remove_player` are extracted. CD tunnel coordinates replace the original stadium descriptor access; native camera ownership replaces `camera=sub_camera`. Source touchline routing, arrival thresholds, trap flag, player_on_off direction and removal logic are preserved.
- ACTIONS.CPP `init_run_act`, `run_action`, `process_anims` and `process_dir` drive movement and motion capture, including original disappearance near the tunnel.
- FOOTBALL.CPP `process_subs` supplies the sequence: finish the outgoing route, replace the identity at the tunnel, reverse player_on_off, then run the incoming route. The existing native squad/rating swap supplies the new record. It is not a second gameplay substitution.
- 3D_UPD2.CPP camera 17 follows the actor using the original near/far touchline expressions.
- RULES.CPP `goto_target` moves officials at the original 2.6 units per tick. A native zero-distance guard prevents division by zero. `at_target` corrects the source's tm_x-used-for-Y typo, and considers a coincident target complete. These corrections are explicit deviations from the preserved body.

The live substitution or discipline result is committed once by the existing match adapter. A separate presentation copy shows the route while the live players, ball, officials, clock, RNG and statistics remain frozen. Completion or skip returns to the already prepared restart. A 1,200-tick safety timeout prevents a presentation from trapping the match; tests require natural completion before it. End-of-period transitions take priority. Diagnostic modes without frontend controls omit these sequences, while retaining actual substitutions/discipline.

The native event queue, pause/skip text, stats/squad mapping and presentation scheduling are not DOS UI. Officials leaving or entering through the tunnel and the complete DOS await_kicker state machine remain unrestored. This batch restores referee movement around restarts, not a fabricated referee tunnel sequence.

## Validation

The five-group contract tests original routes in all five venues and both halves, including keeper substitution; direct and second-yellow reds; correct identities and counts; immutable live state; queue order; pause/skip/focus/disconnect; new-match reset; and referee target movement/facing. Rendered outgoing, incoming and dismissed-player views are inspected. A full watched tournament requires natural individual-transition completion in addition to its existing match, replay, interval and checkpoint checks.

[Focused results](walkoff-focused.txt), [regression](walkoff-regression.txt), [Gaiking](walkoff-gaiking.txt), [portable package](walkoff-package.txt), [source extraction](walkoff-source.json). Physical controller/audio review remains part of the user's combined playtest.
