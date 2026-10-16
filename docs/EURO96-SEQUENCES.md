Current update (0.66.0): exhibitions and custom draws now follow the verified retail early-return gates, without an Actua welcome. French/absent supplements retain compatibility. [Evidence](FIDELITY-BATCH.md#welcome-commentary). The following sections retain the earlier restoration record.

# Euro 96 tournament speech — 0.61.0

Original-draw group matches now use the retail STT introduction selected by the original fixture lookup. Quarter-finals play their complete STT25–28 recordings. Semi-finals and the final play STT followed by the home country's TTZ fragment and away country's TTY fragment. Championship winners, penalty-shootout winners and extra-time winners use the corresponding original country recordings. A decisive regulation knockout result uses the already recovered FLL score announcement.

## Evidence

The importer pins the executable, compressed English script and every MAD bank to the earlier disc inventory. The fixture-to-speech table is at executable file offset `0x139ab8`; the launch code at `0x98126` through `0x98133` indexes it using the current fixture and writes the selected speech number. The sequence is not consecutive: fixtures 9/10 select STT10/STT09, for example.

The group fixture slots in the retail table at `0x139910` match the preserved source's first 24 fixtures. The retail English script supplies the default country-to-group-slot draw. The native audio route checks both the original-draw flag and the countries before using a fixed group recording. Reversing the native home slot to give a tournament user control of their chosen country does not change the scheduled pairing or its recording.

The welcome function at `0x101ba4` checks the championship game type and original-draw flag. Speech numbers 29–31 preload TTY and TTZ country fragments, then load STT. The playback routine at `0x105193` takes the top entry and decrements the stack at `0x1051d1`, establishing the audible order STT → home TTZ → away TTY. The importer verifies the suffix strings and instruction anchors. These are concatenated original samples, with no generated speech.

The retail full-time dispatcher first chooses a winner. Speech number 31 selects that country's TTC clip before considering other result types. Otherwise the penalty flag selects TTG, and the extra-time flag selects TTV. Those flags are set by the corresponding period-transition branches; golden-goal completion during extra time uses the same extra-time result route. The native adapter uses its authoritative penalty scores for shootouts and match goals otherwise, so a tied match followed by penalties announces the shootout winner.

[Import evidence](euro96-sequences-import.json) records the 31 fixture mappings, suffixes, instruction bytes and original WAV hashes. `EURO_SEQUENCES.BIN` contains 30 STT records and 64 country fragments/result calls. All 94 imported records are now selected in supported contexts.

## Quarter-final bracket evidence

The retail qualifier table at file offset `0x139ad8` has group positions `[0,1,4,5,8,9,12,13]`: winner/runner-up of groups A through D. Code at `0x8089f`–`0x808a9` reads each six-byte row and finds the country at that league position; `0x80939` adds 16 to the row index and `0x80940` writes that new code into the country’s draw entry. The fixture resolver at `0x7eed5` compares those draw entries. Thus codes 16–23 are qualifier identities, not country IDs.

| Native quarter | Retail codes | Resolved retail pairing | Native pairing | Recording |
| --- | --- | --- | --- | --- |
| 1 | 19, 16 | Runner-up B / Winner A | Winner A / Runner-up B | STT25 |
| 2 | 18, 17 | Winner B / Runner-up A | Winner B / Runner-up A | STT26 |
| 3 | 20, 23 | Winner C / Runner-up D | Winner C / Runner-up D | STT27 |
| 4 | 21, 22 | Runner-up C / Winner D | Winner D / Runner-up C | STT28 |

The importer now validates every resolved bracket pairing against preserved source, pins the qualifier-assignment instruction bytes, and records both encodings. The existing supplement bytes remain identical; this release enables the four already recovered WAVs. Quarter-final speech is a complete STT clip, so no country fragments or compatibility ending are appended. No fixture or home/away gameplay order changes are needed.

## Boundaries

- STT20 is absent on this disc. The executable attempts an MCA fallback, but that bank is also absent. It then queues the two country X/Y fragments. This fixture now plays those fragments without appending an Actua ending.
- Quarter-final introductions require the original-draw championship context, just like the other STT recordings. Their participants are resolved by the existing tournament engine.
- Exhibition and custom-draw introductions remain the existing native compatibility sequence. The verified retail welcome routine excludes those contexts; no unsupported claim of exact retail exhibition audio is made.
- French keeps its existing voice and sequence behaviour. Missing supplements preserve older installations; malformed media is rejected and language changes remain transactional.
- Extra-time entry announcements are restored in 0.63.0 using the verified XTM branch; see [evidence](RETAIL-IDENTITY-AUDIO.md). Full retail sequencing of every transition remains under review. Winner names are never spoken for tied or invalid result scores.

Tournament fixture context is presentation-only. Preparation supplies it, launch consumes it, and a reset clears it. It is not added to saves or replay files and does not alter fixtures, scores, difficulty, gameplay RNG, input or simulation. Existing tournament checkpoints derive the context again on their next launch.

## Validation

Independent inventory tests check every original WAV and the non-consecutive fixture lookup. Runtime checks compare complete mixed PCM, including all fragment boundaries, for 224 introductions and 160 country/side/result combinations. The quarter-final cases cover every possible cross-group qualifier pairing on both native sides, plus eight actual saved/resumed tournament kickoffs. They cover swapped human teams, live kickoff, final-over-penalty/extra-time precedence, golden goals, regulation knockout results, tied shootouts, language changes, absent/malformed media, mute/pause and context cleanup. Existing full tournament and frontend tests cover actual competition progression and saves. [Release status](RELEASE-CANDIDATE.md).

Automated validation uses dummy audio devices and exact sample comparison. Physical listening remains a separate check.
