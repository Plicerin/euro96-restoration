# Original first-time passing - 0.48.0

On an incoming loose ball, hold **X + Space**, or **West + South (Xbox X + A)**. The original engine chooses a first-time chip, standing header or jumping header toward an eligible teammate. A valid receiver and reachable ball trajectory are required. This does not force a contact or add a ground-pass animation to the source's first-time choices.

Shot alone still requests a first-time strike. Pass alone requests control. With possession, the existing stationary charge/release combinations remain unchanged. Pause clears held input; release both buttons before starting a new action.

## Source correction

`SOURCE/INTELL.CPP` explicitly labels its EURO96 interception variant as supporting first-time passing. `can_i_intercept` sets `pass_option` from both buttons and calls `user_pass_check`, which chooses the receiver and allows chip/standing/jumping-header contact. But its record-writing conditions accept only `strike_option` (shot without pass) or CPU control. Those conditions reject the very both-button request that computed the pass.

`tools/restore_player.py` corrects exactly the three pass-capable record gates (10, 9 and 7) to also accept `pass_option`. Extraction asserts one replacement per record. The foot shot, volley, diving header and overhead gates are untouched. `strike_and_control` already transfers receiver/type into the player; `chip_strike` and `head_strike` already execute `pass_ball` at original animation contact. The supplied SOURCE files remain unchanged.

This is a corrective adaptation of the preserved Euro source, not a claim of byte-identical DOS behavior. No new ball attraction, receiver targeting, animation timing or CPU decision code was introduced.

## Combined validation

The human-control contracts exercise reachable chip, standing-header and jumping-header trajectories on both pitch halves, requiring one pass, no shot, no prior collection, the expected receiver, and outgoing velocity toward that teammate. Invalid-receiver and pause/re-arm checks accompany existing shots, trapping, charges, crossing and aftertouch contracts.

The SDL control smoke sends actual keyboard and virtual-controller combinations through the application event loop and observes completed pass contacts. The complete regression suite also covers CPU matches, tournament progression, saves, graphics, audio and replays. All 84 checks passed in 213.93 seconds. [Full regression](firstpass-regression.txt), [contact contracts](firstpass-contracts.txt), [Gaiking](firstpass-gaiking.txt), [seven package checks](firstpass-package.txt), [delivery status](RELEASE-CANDIDATE.md). The reviewed [pause reference](firstpass-help.png) includes the combination. No separate per-feature playtest is requested.

## Completeness

Single-player match and tournament functionality is close to complete for this restoration. Remaining work includes local/network multiplayer, matching Euro 96 artwork/names, some exact original presentation and mid-match saving. The current save model resumes unfinished fixtures from kickoff. Physical controller feel, audio output and display behavior still benefit from one combined human session.
