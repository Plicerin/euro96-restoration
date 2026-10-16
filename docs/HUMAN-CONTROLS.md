# Original human ball controls - 0.38.0

This batch connects four more preserved routines to live full-team keyboard/gamepad input: `user_got_ball`, `user_opp_has_ball`, `user_spec_kick` and `rotate_around_ball`. Original counters, decisions, animations and ball-contact routines determine the action. The native input adapter supplies held buttons and direction at the existing 20 Hz simulation rate.

## One combined playtest

Use the usual Play-Actua96 launcher, choose Y / North to play a match, and review these together. P / Start opens the reference panel. West is Xbox X; South is Xbox A.

| Batch item | Controls and behavior |
| --- | --- |
| Shot variations | Move and press X / West for the original normal-shot decision. Stand still, hold X / West and release without direction for a powered drive. Driven shots have no aftertouch. Starting with both buttons and a direction selects the original drive branch. |
| Passing and crossing | Move and press Space / South for original teammate selection; the original wing-area decision can select a cross. Stand still and hold/release Space / South for a powered ground pass. Add direction at release to disguise its direction. While still, hold both buttons, then release X / West for a chip or release Space / South for a high pass. Release the remaining button afterward. C / North retains the existing loft shortcut. |
| First touch and aerial contact | With a loose incoming ball, hold X / West to request a first-time strike, or Space / South to request control. The original trajectory predictor chooses reachable foot/head strikes or trap/chest/head control. Timing, reach and trajectory matter; a button does not force an arbitrary animation. |
| Aftertouch | After a normal shot, use direction to bend or adjust height. Relative to the flight, sideways bends, forward lowers and backward raises. The original player-flair limits and ball-spin formulas apply. |
| Input and help | Keyboard and gamepad feed the same held-button path. The power bar reflects the original counter, capped at 30 simulation ticks. Pause, focus loss and explicit player changes clear the charge; release the buttons before starting another action. Space / South also invokes the original close steal when an opponent has the ball; X / West tackles. |

Stationary combinations must begin without movement. Changing direction during a charge may select the original tee-up or disguised-pass branch. Keeper distributions and set pieces retain their separate existing controls and indicators.

## Provenance and compatibility changes

- `SOURCE/INTELL.CPP`: possession/defense decisions, shot release and interception planner.
- `SOURCE/ACTIONS.CPP`: stationary combinations, turn around the ball and animation/contact decisions.
- `SOURCE/BALL.CPP`: aftertouch and spin physics.
- The supplied CD's `MANUAL.PDF`, English controls on PDF pages 10-11, was cross-checked. Its button descriptions are internally inconsistent and some combinations differ from this preserved Euro source. The mappings above follow the executable source branches, rather than combining contradictory descriptions.
- `tools/restore_player.py` removes comments, preserving line positions, before extracting the four new routines. Commented-out branches contain unmatched braces. Verified special-action values 7/8 come from the existing set-piece recovery.
- **Aftertouch correction:** the original `aim_shot_at_goal` captures its direction basis before `shoot_ball` assigns the shot velocity. A stationary starting velocity could leave a zero basis and suppress bend. The generated integration refreshes that basis after assigning the final shot velocity when aftertouch is enabled. Spin formulas and limits are unchanged. This is an explicit corrective adaptation, not a claim of byte-identical DOS behavior.
- The native charge bar and pause reference are new presentation of restored controls, not reproductions of the original DOS HUD artwork.

The source files under `SOURCE` remain unchanged. The generated recovery report records these additions and adaptations.

## Limits

The original multi-user registry and player-reselection scheduler remain native adapters. This restores additional single-player controls, not local multiplayer. Historical diagnostic fixtures can still use the immediate `request_play_kick` API; interactive full-team X/Space and West/South now use held buttons.

Version 0.48 corrects the three contradictory gates in the preserved EURO96 interception variant. Both buttons now request its original first-time chip or header pass; see [first-time passing](FIRST-TIME-PASSING.md). This does not claim unrestricted bicycle kicks or complete human-input parity with every retail release.

## Automated coverage

`original_human_controls_contracts` checks stationary shot/pass charge and contact, normal shot, chip/high/disguised passes, moving-pass fallback, a running wing cross, close steal, pause re-arming and lateral/vertical aftertouch. Eighteen incoming-ball cases reached contact and exercised trap, chest, head control, foot strike and standing-header selections. Two explicit reachable trajectories verify first-time foot/header shot release. A seeded original volley selection verifies animation-driven contact through the live scheduler; this is not evidence that the predictor naturally chooses a volley for every incoming ball.

`native_original_control_smoke` additionally sends actual SDL keyboard events and virtual-gamepad held/release input, including reconnection. The existing CPU matrix covers 32 fixtures across all countries and formations. These checks support the batch; the combined human playtest remains useful for feel and usability.

All **68 CTest checks passed** in 229.58 seconds. [Full regression](human-controls-regression.txt), [focused contracts and CPU matrix](human-controls-contracts.txt). The 65 CPU matrix case/result lines match the 0.37.1 audit, including winners, completion ticks and restart counts. The final follow-up only stages stationary possession for the charge-bar screenshot fixture; gameplay behavior is unchanged by that follow-up.

[Pause reference screenshot](human-controls-help.png) and [charge cue screenshot](human-controls-charge.png).

The final executable passed the three focused input/front-end checks again. [Final smoke run](human-controls-final-smoke.txt). Published snapshot: `69cc53340891c360-2b4b128d-0703f4d7-65e2336c`, version 0.38.0. Launcher/media hashes and the exact published executable's production-directory save/preferences probe passed; the existing tournament hash was unchanged and preferences remained absent. [Published verification](human-controls-published.txt). No interactive game was started or restarted.
