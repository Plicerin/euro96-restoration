# Replay controls - 0.39.0

Updated in 0.47.0: replay files and a persistent library are now implemented. See [Saved Replays](SAVED-REPLAYS.md) for save/library controls and limitations.

F4 opens the last recorded passage of a match. On a gamepad, pause with Start and press West. The existing buffer retains up to 15 seconds at 20 recorded frames per second. One combined review now covers five additions:

1. **Slow motion:** 20%, 40%, 60%, 80% and normal speed, following the original replay menu's `speed/5` timing.
2. **Frame stepping:** move forward or backward by exactly one recorded frame; the replay pauses at the selected frame.
3. **Seeking:** jump one second, return to the first/latest frame, or click the timeline. Bounds stop at the available recording, including short recordings.
4. **Replay endings:** manual playback returns to its beginning and pauses, following the original menu behavior. Optional continuous looping is a native addition. Automatic goal replays still return to the live match unless you take over their playback controls.
5. **Replay control panel:** current frame, speed, pause/loop status and camera name are visible together, with keyboard/gamepad prompts. The existing five original camera calculations remain available while paused or playing. Seeking resets visual interpolation and camera tracking to prevent blending unrelated moments.

| Action | Keyboard | Gamepad |
| --- | --- | --- |
| Open replay | F4 | Start, then West |
| Pause/resume | P | Start |
| Previous/next frame | Left / Right | D-pad Left / Right |
| Slower/faster | Down / Up | D-pad Down / Up |
| Back/forward one second | Page Up / Page Down | LB / RB |
| Beginning/latest frame | Home / End | North goes to beginning |
| Loop on/off | L | West |
| Change camera | V | R3 |
| Return to match | Space, Escape, Q or F4 | South or Back |

Click the progress bar to seek with the mouse. Seeking always pauses. Speed changes preserve whether playback is paused. To continue a loop from a paused position, press P / Start. Leaving replay restores the live camera and whether the match was paused before entering; replay time never consumes match time.

## Original source and native boundaries

`SOURCE/3DENG.C`, function `replay`, initializes speed to 5, limits it to 0-5 and advances by `repstep*speed/5`. Its zero-speed action steps by elapsed presentation time. At the end it sets `reppos=0, play=0`. The native viewer provides five moving speeds and explicit pause/exact 20 Hz frame stepping; it does not claim identical DOS button selection or zero-speed step timing.

`SOURCE/3D_UPD2.CPP` contains `setup_replay`, `reset_replay`, `do_replay_render3d` and the circular recorded-coordinate buffer. `SOURCE/FOOTBALL.CPP` initializes the 15-second buffer. Native owned snapshots, interpolation and SDL input already replace that memory/rendering interface. This batch extends that adapter; it does not add replacement football physics or resimulate the recording.

The timeline, one-second jumps, backward stepping and continuous loop switch are native usability additions. The recorded states, motion captures and camera calculations come from the restored game. Original source files remain unchanged. Scrubbing audio and the exact DOS replay-menu artwork remain unimplemented. Replays are silent. Native replay files were added in 0.47.0.

## Verification

The focused contracts cover bounded history, five playback rates, both frame directions, short/long seek bounds, paused-frame stability, manual and automatic endings, looping, all five cameras, reproducible rendering after nonsequential seeks, restoration of live camera/pause, and rejecting stale recordings after a new match. They compare the live engine's object bytes while manipulating/rendering its separate replay copy. Existing SDL smoke checks also send real keyboard and virtual-gamepad events through the replay menu.

[Rendered control panel](replay-controls.png). The final SDL smoke also checks a timeline click after converting logical rendering coordinates to window coordinates and back.

The full 69-check regression run passed 68 checks and exposed a missing window ID in the new synthetic mouse event. SDL therefore skipped coordinate conversion for that test event. The follow-up fixes only the event fixture and reruns replay, front-end and live-input checks; the timeline implementation is unchanged. [Full run](replay-controls-regression.txt), [final affected checks](replay-controls-final-tests.txt).

All four final affected checks passed, completing validation across the 69 checks. Published version 0.39.0 is snapshot `e4555f0de4217008-2b4b128d-0703f4d7-65e2336c`. Launcher/media verification and the published executable's production-directory save/preferences probe passed. The existing tournament hash was unchanged; preferences remained absent. [Published verification](replay-controls-published.txt). No interactive game was started or restarted.
