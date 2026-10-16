# Original CD inspection - 2026-10-10

The user-provided `actua-soccer.iso` supplies the missing original sound assets.
Inspection used 7-Zip read-only listing and extraction to stdout. No installer or
DOS executable was run, no image was mounted, and no interactive game was
restarted. The published game remains version 0.26.0 without audio playback.

## Identity and compatibility

The image is 283,176,960 bytes with volume label OEMSA. READ.ME identifies it as
Actua Soccer. Its SHA-256 is
`1632fa60cb97280379193b439b071dd8312dcd9f111abec9293bba40b4dbea35`.
This does not establish an exact release/build or make all assets interchangeable
with the preserved Euro 96 snapshot. The disc ACTREND.OFF has 439 entries, whereas
the preserved EUROREND.OFF has 229. Do not replace the preserved graphics archives.

All 20 names recovered from SampleBank in SOURCE/AUDIO.OBJ match WAV entries in
the disc's DIGI/PITCH/PITCH.MAD. Recovery follows the original OMF pointer
relocations and 16-byte memsam records, with the count documented in AUDIO.H.
This establishes sample availability and PCM readability, not completed runtime
integration or identical versions of the entire audio engine.

## Validated audio

| Content | Banks / files |
| --- | ---: |
| Pitch effects and crowd | 1 MAD bank, 26 samples |
| English commentary phrases | 46 MAD banks |
| French commentary phrases | 46 MAD banks |
| English team speech | 45 MAD banks |
| French team speech | 45 MAD banks |
| Music | 8 WAV files |

All 183 MAD banks passed directory, offset, size, contiguity, complete PCM-frame
read and end-of-file checks. Their 7,400 sample records are mono, 11,025 Hz,
8-bit uncompressed PCM. Each MAD has 24-byte directory records containing a
16-byte filename, little-endian 32-bit offset and size. The first payload offset
gives the directory size. Each payload is a complete RIFF WAVE file.

The pitch bank includes four ambience loops, goal/miss/save/foul/reaction samples,
whistles, kicks, pass, bounce, fall and six chants. Twenty entries match the
original active SampleBank; the six chants follow them on disc.

Music files DRAW, EDITSCRN, FINAL, FOOTBALL, FOOTY, LOSE, MENU and WIN also passed
PCM-frame checks. They are 22,050 Hz, 8-bit PCM; FOOTY is mono and the others stereo.

English E_GLA.MAD and French F_GLA.MAD each contain a duplicate filename ending
in gla30.wav. Future loading must preserve record order and handle duplicate
names explicitly rather than assuming a unique filename for every record.

The disc also contains MANUAL.PDF and four VIDEO/*.GDV files. They were inventoried,
not decoded or reviewed in this audio inspection.

## Reproduction and next implementation

With Python 3.11+ and 7-Zip on PATH, run from the repository root:

```powershell
python tools/inspect_cd_audio.py "C:\Users\vrock\Downloads\actua-soccer.iso" --report docs/cd-audio-inspection.json
```

The report records the complete disc file inventory, ISO and bank hashes, every
sample's metadata/hash, duplicate names, and the source pitch-table comparison.
The tool writes only its requested JSON report; it does not install or copy game
assets into SOURCE. The initial six extracted inspection files remain in the
workspace's work/cd-inspection directory, separate from game data.

Next: recover the original PitchBank/CrowdBank event mappings and connect the
engine's existing sound events to an SDL audio backend using the original WAVs.
The current native bridge only counts play_fx calls and remembers the last sound;
that is insufficient for faithfully queuing multiple events in one simulation
step. Commentary requires additional phrase/team mapping checks. Music and video
playback are separate integration work. No substitute sound effects are needed.
