# Euro 96 match audio — 0.58.0

Later additions: [period score calls in 0.59.0](EURO96-PERIODS.md) and [tournament introductions/winner sequences in 0.60.0](EURO96-SEQUENCES.md). See the [current asset audit](EURO96-TARGET.md) for the remaining compatibility recordings.

Thirty existing match-commentary categories now use the retail Euro 96 banks and their verified numbered ranges. This covers set pieces, fouls/cards, passing, goals, saves and misses. Score announcements use all four original A–D takes for each of the 33 supported scorelines. Pitch and crowd samples now come directly from the Euro 96 disc, with its slightly quieter foul reaction.

## Evidence and mapping

`tools/import_euro96_audio.py` verifies the executable and every extracted bank against the pinned disc inventory. It reads the 46 six-byte category descriptors at executable file offset `0x1424b0`. Instruction anchors at `0x102e6c`, `0x102e97`, `0x102e9e` and `0x102f15` establish one-based category indexing, six-byte descriptor stride, the recorded count and one-based numbered selection.

Retail category numbers differ from this preserved source revision. The adapter retains SOURCE event IDs and maps the intersection by the original three-letter bank identity; it never substitutes the retail numeric IDs. Thirty categories match. All clips beyond the executable's declared range are excluded, even if the disc includes additional takes. Ambiguous duplicate filenames (`GLA30`, `GLH04`) are excluded entirely. Missing numbered clips are not fabricated.

The score dispatcher sorts the two score digits, bounds them at 7 and 5, and chooses an A–D suffix. Its bank-pointer table starts at data offset `0x12d70`, with SCO as the first entry; file-offset anchors `0x104067` through `0x1040c3` record the bounds, four-way selection and bank lookup. The native queue still uses the original supported home/away bounds and original goal-to-restart timing. Only the recording choice changes; speech selection does not consume gameplay RNG.

The twenty Euro 96 pitch/crowd WAV records are byte-identical to their counterparts in the existing Actua bank. The complete 19-entry pitch table at `0x1422e0` matches the preserved `AUDIO.OBJ` mappings. The ten-entry crowd table at `0x142410` also matches, except that the foul amplitude at `0x1424a4` is 25600 instead of 26624. No other crowd scheduling or mix levels change.

[Import provenance](euro96-audio-import.json) records mappings, original filenames, exclusions, instruction bytes and hashes. `tools/test_euro96_audio.py` independently checks the imported records against the earlier disc inventory and verifies that retained compatibility clips are unchanged.

## Compatibility and remaining work

`EURO_EVENTS.BIN` takes priority over the previous English pack. Its `AC96COM2` format encodes score variants as `score + 100*variant`; other categories retain their existing encoding. The loader validates the variant separately and retains the original public score identity. Legacy `AC96COM1` packs still work when the new overlay is absent. Duplicate encoded records, illegal scores and variant letters beyond D are rejected. French and player-name packs keep their existing formats and selection.

`EURO_PITCH.MAD` takes priority over the older pitch bank. The older imports remain intact for explicit compatibility. The portable package and usual launcher include and verify both new overlays.

Current remaining fidelity work (0.65.0):

- Source-only CPB, CPI, FPB, FPC and GLF events retain compatibility recordings until retail callers are verified.
- Period, tournament introduction and winner mappings are restored; missing/ambiguous half-time scores and exhibition/custom-draw introductions remain compatibility routes. [Current sequence evidence](EURO96-SEQUENCES.md).
- Result music now resumes the Euro theme through the verified retail frontend route. Actua outcome stings remain fallback only without that theme. [Trace and compatibility](RESULT-MUSIC.md). Trophy/credits films and their stereo audio are restored.
- French remains an optional Actua voice pack because this Euro disc has no French banks.
- All 320 players are voiced. Kuznetsov's one Actua tone is accepted by the user and is not an open blocker.

[Completion checklist](COMPLETION-CHECKLIST.md) is the authoritative current backlog.

Difficulty, gameplay, squad identities, save formats and the preserved SOURCE files are unchanged. No running game is restarted by publication.

## Validation

The new runtime contract compares live mixed PCM against independently provenance-checked records for the highest numbered recording in each of the 35 source event categories and all 132 score variants. It tests score non-repetition, all pitch/crowd mappings, the retail foul gain, legacy fallback and malformed packs. Language-switch tests use score hashes independently extracted from the retail MAD. Existing full-match audio/commentary checks cover real engine callbacks and match completion.

Final regression, portable-package and publication results are recorded in [release status](RELEASE-CANDIDATE.md). Automated dummy-device checks do not replace listening through the user's actual audio hardware.
