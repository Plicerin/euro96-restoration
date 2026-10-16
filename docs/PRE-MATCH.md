# Original pre-match presentation - 0.45.0

After squad confirmation, exhibition and watched tournament fixtures open with the original on-pitch lineup. Two team lines face one another beside halfway, with the referee at the centre spot and assistants at the touchlines. The original camera starts close to the ball, rotates and pulls away, then holds before the prepared match begins.

## One combined playtest

1. Start any exhibition: check the team lines, selected kits and two goalkeepers.
2. Watch the camera complete its nine-second sequence and reach kickoff at 0:00.
3. Start another match and pause with P / Start. Changing focus or disconnecting the controller also pauses; returning focus does not resume automatically.
4. Skip with Enter / Space / Escape, South / East / Back, or a left click. Hold South briefly: it must not become a pass. Release it before playing. F11 and audio shortcuts remain available.
5. Watch a tournament fixture or rematch: each new match gets the opening. Half-time still uses the tunnel exit and team talk, without replaying the opening.

## Preserved source and executable evidence

- ACTIONS.CPP `init_team`: extracted presentation assignments for position, facing, stand animation, contact and control. Selected squad identities, shirts, ratings and availability are retained.
- ACTIONS.CPP `init_officials`: extracted body sets the three officials' locations, facing and animation. FOOTBALL.CPP `animate_players` advances their and the players' frames without football AI or physics.
- FOOTBALL.CPP `init_line_up` and original main-loop countdown: FOOTBALL.OBJ's instruction at 0x947 writes 180 (LINE_UP_DELAY + 40). At the native 20 Hz rate this gives seven seconds of camera movement and two seconds holding its final pose.
- 3D_UPD2.CPP pre-match block: extracted orbit expressions, including the original integer division for the radius. The original scale of 440 is retained. Native rendering interpolates between those discrete poses.
- ACTIONS.OBJ at 0x48d supplies the referee animation step 1/64. The source and object revisions differ in the assistants' animation ID: this restoration follows source MC_STAND for assistants and MC_PLEAD for the referee, with recovered timing.

`tools/restore_prematch.py` validates pinned object hashes and records source hashes, recovered constants and adaptations in [prematch-source.json](prematch-source.json). Preserved SOURCE files are unchanged.

## Native boundaries

The presentation runs on a copy. It never advances the live players, ball, officials, statistics, match clock or RNG. Completion and skip return to the existing prepared original-engine kickoff; they do not create a second match or reseed gameplay. Menu text, pause/skip controls, focus handling and release-before-action protection are native. The opening's animation and camera are original; this is not an invented walk out of the tunnel. Exact DOS display timing and camera buffering are not claimed.

This does not restore individual substitute/send-off walks or officials leaving the pitch. The on-pitch lineup is distinct from those remaining presentation tasks.

## Validation

The five-group test covers original positions/animation, camera path/hold, all five imported stadiums, immutable live state, pause/skip/focus/disconnect, rematch and absent-media fallback. It saves twenty rendered views. Visual review confirms the close, mid-orbit and wide views. The native frontend smoke also holds and releases a virtual-controller skip button. The complete tournament test now requires 31 natural openings as well as 62 natural tunnel exits.

[Focused tests](prematch-focused.txt), [full regression](prematch-regression.txt), [Gaiking opening and native frontend checks](prematch-gaiking.txt). Gaiking uses dummy/software rendering here; physical gamepad, audible playback and a human review remain part of the combined playtest.

All 81 regression checks passed in 189.44 seconds. The portable package passed its six isolated checks. Version 0.45.0 is published for the normal launcher's next start; no running game was restarted.
