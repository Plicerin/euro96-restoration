# Match HUD (0.25.2)

The live match sidebar now prioritizes score, clock, selected player and the
controls relevant to possession. Carrying the ball shows pass, loft and shoot;
defending shows tackle, switch and movement. Keeper possession shows pass, loft
and punt. The match mode label makes the change explicit. Original set-piece
aiming dots and power/height bars remain in place.

Watching CPU play shows the ball carrier and passes, shots and saves, with each
pair ordered like the teams in the score. These are the existing original-engine
counters, not newly calculated completion rates. Pause also shows those stats.
P / Start opens the pause reference: squad, recent replay, setup and match reset.
Keyboard/gamepad bindings have not changed. Labels are reference instructions,
not clickable buttons. F5 practice and restart controls reset the match.

Score, phase and clock share one group. Important actions use larger text and
separate key labels. Long names/status lines are fitted within their available
width. The existing 960 x 664 logical layout scales with the SDL window. The
full-time panel and shootout history remain, with new-match/setup prompts when
the match ends. Replays show skip and pause controls.

All changes are native presentation in native/main.cpp; no original gameplay,
physics, AI, or SOURCE files changed. This is an accessibility/readability update
to the restoration's HUD, not a claim that these panels reproduce the DOS UI.
No external design assets or component inspiration were used.

## Reproducible visual review

The capture-only --hud-capture option accepts attack, defend, keeper, watch or
paused and requires --capture PATH. It launches hidden, advances original CPU
play to actual possession (or uses the existing keeper practice fixture), then
captures the HUD with deterministic keyboard prompts. It cannot be selected
without an output capture path and does not affect an existing game process.

Example: actua96 --data SOURCE --hud-capture paused --capture hud-paused.bmp

The usual launcher loads this version on its next launch. Publishing does not
restart the running game.

As of 0.26.0 the larger match view is the default. F6 / live R3 restores these
panels, while pause and manual set pieces display them automatically. See
[PRESENTATION-RECOVERY.md](PRESENTATION-RECOVERY.md) for the original HUD restoration.
