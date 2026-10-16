# Original match audio - 0.27.0

Five connected features are ready for one listening session:

1. Original ball-contact, pass/kick, bounce and sliding-tackle samples.
2. Referee whistles, including distinct half-time and full-time recordings.
3. Original low crowd ambience during matches.
4. Original goal cheers, save applause, near-miss and card reactions.
5. F8 mute, automatic silence on pause/focus loss, and flushing stale audio when
   leaving a match, opening setup/squad panels or entering replay.

Open the existing View-Original-Actua96.cmd launcher when ready. No installation,
ISO mounting or manual file copying is needed for this local playtest. Publishing
this batch does not restart an existing game. F8 toggles all match sound; pause
and setup display its status. Commentary, menu music and video playback are not
connected in this batch. Replays are silent rather than replaying live events.

## Original evidence

The pitch bank comes from the user-provided Actua Soccer CD, imported into
local-data/DIGI/PITCH/PITCH.MAD, outside the unchanged SOURCE tree. Its SHA-256 is
0703f4d751ac3af7c61a5fa611dbe4873f92b337f04ec8a7068e4f45d68a6e20.
All 20 samples in the original SampleBank match this bank. Six additional chants
are present but not used. See CD-INSPECTION.md for the full disc inventory.

The build recovers 19 PitchBank and 10 CrowdBank entries from pinned AUDIO.OBJ,
including their SampleBank pointer relocations, amplitude, frequency and flags.
Instruction checks establish one-based event indexing (16-byte stride minus 16).
The recovered mapping and source hash are recorded in audio-recovery.json.

Existing original play_fx calls supply contact/bounce/tackle events. The rules
adapter now retains foul/offside whistle calls. Native centre restarts and period
transitions supply whistle events at their integration boundaries. The source's
nothing_happening gate contains an unconditional full-time whistle before it
checks pending shots; this remains suppressed, and the actual period transition
emits one appropriate whistle after the gate passes. Golden-goal termination
uses the full-time sample.

## Native boundaries

SDL replaces the unavailable DOS audio driver. WAV data is decoded at its recorded
11,025 Hz rate into a 48 kHz float stereo mixer. Table frequency 0x8000 is treated
as nominal WAV speed; the original approximate-frequency variation of -512 through
+511 is applied relative to that nominal value, using a separate audio PRNG.
The DOS driver's exact frequency-divider implementation is not recovered.
The original table amplitudes and FOOT.CPP default crowd/pitch volume ratio are
retained. Mono samples receive native camera-relative panning and a 600-unit
rolloff distance. This is a native spatial mix, not bit-identical DOS playback;
it also avoids the source's y-minus-camera_x coordinate typo.

The mixer uses up to 32 overlapping voices, 2 ms sample-edge ramps, bounded output
and approximately 40 ms of queued audio. Version 0.32 selects original crowd ambience banks from the recovered mood decisions.
Goal/save/card cues observe existing engine events. Near-miss cues count original
shot_passed classifications without changing its legacy timer. Original mood decisions use separate presentation state; native crossfades replace the unrecovered DOS scheduler. See [the presentation batch](PRESENTATION-BATCH-32.md) for fidelity limits. Crowd samples
are quieter than pitch effects, matching the original default volume ratio.

The engine owns a bounded 128-event buffer so multiple events between rendered
frames survive. Prediction suppresses audio, copied simulation states own separate
buffers, and overflow is counted. Playback drains only that buffer, leaving ball,
player and gameplay RNG state alone. No callback reads mutable engine state.

Missing assets or an unavailable audio device leave the game playable silently.
The local importer validates MAD/WAV structure and all required sample names.
Publication copies the pitch bank into the immutable executable snapshot, adds
its hash to the build identity/manifest, and verifies it before launching.

## Reproduction

```powershell
python tools/import_cd_audio.py "C:\Users\vrock\Downloads\actua-soccer.iso"
```

Configure/build normally after import. Audio tests are enabled when the local
pitch bank is available. original_audio_contracts checks mapping, all 19 effects,
finite PCM, stereo orientation, overlapping events, gameplay RNG isolation,
prediction, buffer bounds, one-shot reactions, mute, pause, restart and SDL's
silent dummy output. It also runs a complete original CPU match and checks actual
contact/bounce and half/full-time whistle events without overflowing the queue.
native_audio_smoke exercises the real application's F8 and pause event handling.
Both use SDL's dummy audio device; these tests do not replace listening on the
user's actual speakers. Full-suite results are in audio-test-results.txt.
