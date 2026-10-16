# Extra-time entry restored — 0.63.0

The period overlay now contains 282 recordings, adding all 26 supplied XTM clips. Tied 90-minute knockout transitions select the verified original score-specific recording. [Evidence and remaining gaps](RETAIL-IDENTITY-AUDIO.md).

The sections below describe the earlier period restoration.

# Euro 96 period commentary — 0.59.0

Update in 0.60.0: [tournament introductions and country-specific knockout winners](EURO96-SEQUENCES.md) are now connected. The compatibility boundaries below describe the initial 0.59.0 period overlay; the current sequence document records the expanded routing.

English half-time announcements now select the retail HLF recording for the actual score. Decisive regulation results select FLL; drawn regulation results use END, as the retail executable does. Team order does not change the spoken score. The existing whistle and match-transition timing remain intact.

## Executable and disc evidence

The importer pins the supplied Euro96 executable SHA-256 and all three MAD hashes against the previously recorded CD inventory. File offsets `0xa71a0`/`0xa71a5` identify the 45-minute call with selector 3; `0xa7327`/`0xa732c` identify the ordinary 90-minute call with selector 2. These call the dispatcher at `0x103eb4`. The seven-pointer table at data offset `0x12d70` assigns 3 to HLF and 2 to FLL.

The full-time branch first tests for a draw. With neither the extra-time nor penalty transition flag set, it calls event 101 at `0x103f6f`/`0x103f74`. The special event table assigns 101 to END with thirteen numbered recordings. Therefore a drawn regulation result must not randomly select one of the FLL score clips merely because the files exist.

The score branch sorts the two scores, accepts a larger digit up to 7 and smaller digit up to 5, and chooses suffix A–D. The importer records and validates these instruction anchors rather than inferring dispatch from filenames. [Import evidence and per-record hashes](euro96-periods-import.json).

`EURO_PERIODS.BIN` contains 112 unambiguous HLF takes, 131 FLL takes and thirteen END phrases. Of these 256 preserved records, 232 are reachable through the restored routes; the 24 drawn-score FLL takes are retained as disc records but deliberately not selected. All WAV payloads remain byte-identical to the CD.

## Compatibility boundaries

- HLF has no unambiguous recordings labelled 1:1, 2:1, 3:1, 3:2 or 3:3. The four 3:2 filenames each occur twice; both copies are excluded. These scores retain the old generic half-time call. Neither duplicate order nor a neighbouring score is treated as evidence of the intended recording.
- FLL is missing the C take for 3:2. Its three available takes rotate without substitution. Out-of-range decisive scores retain the generic full-time call. END remains valid for draws beyond the score-file bounds because that retail branch runs before those bounds.
- Knockout finals, extra-time and shootout endings have additional winner/competition-specific retail branches. They keep the generic compatibility call until those contexts and recordings are connected. First-half HLF works in knockout matches too.
- The supplement is optional and English-only. French and installations without it keep their existing period calls. Corrupt supplements fail explicitly; a failed language switch keeps the previous voice, current speech and volume.
- Source event IDs 42/43, queue priority, focus/pause behaviour and the gameplay random generator are unchanged. Internal supplement bank IDs cannot be requested as gameplay events.

## Introduction correction and remaining work

The earlier audio notes incorrectly described MCX as the introduction bank. Inspection of the retail welcome function at file offset `0x101ba4` shows STT selection, country fragments and a legacy MCA fallback. Tournament-stage selection is still being traced. This release does not substitute MCX for the current welcome ending or claim complete retail introduction parity.

Remaining audio work includes those fixture introductions, ambiguous HLF labels, knockout result sequences, five source-only commentary categories, result music/trophy playback and the optional French compatibility boundary. Difficulty, gameplay, preserved SOURCE and save formats are unchanged.

## Validation

The independent Python test compares every imported WAV hash against the earlier CD inventory, including excluded duplicate labels. Runtime tests compare entire mixed phrases to verified PCM, exercise both score orders and all available takes before repetition, draws including high scores, absent/malformed supplements, compatibility contexts, language changes, priority, pause and mute. Existing full CPU-match commentary tests exercise actual period boundaries. [Release validation](RELEASE-CANDIDATE.md).

Automated audio verification uses dummy devices and exact sample comparisons. It does not constitute listening on physical audio hardware.
