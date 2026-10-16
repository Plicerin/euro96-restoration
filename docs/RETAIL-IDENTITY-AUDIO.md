# Retail names, appearances and extra-time speech — 0.63.0

All 320 preserved players now have verified appearance mappings and all three spoken-name tones. Of the 960 tones, 959 are original Euro 96 recordings. The supplied E_RUS.MAD omits e_rus17x.wav, so Kuznetsov's normal tone retains its previously verified Actua recording; his other two tones use Euro 96. No player remains unvoiced.

The importer now decodes both scripts as DOS CP850 and reuses the conservative identity resolver already verified for menu previews. Where names differ, the digital identity, date of birth, club, position and eight ratings must all agree. This also distinguishes Yuri Kovtun's main-roster entry from the reserve entry. All 24 previously unresolved appearance mappings are filled, including Helder and Neno. All 888 previously imported retail name tones remain byte-identical. Kit image/palette/clash bytes remain identical; only the final 320-byte appearance table changes. Source roster order, attributes and saves are untouched.

Each country bank is pinned against the independently captured CD inventory. Imports replace only the exact known old overlay or identical new output; other overlays remain protected. [Identity and clip evidence](euro96-import.json), [preview identities](PREVIEW-IDENTITIES.md).

## Extra-time transition

The retail executable's 90-minute tied-knockout path sets its extra-time flag at file offset 0xa7269 and calls the full-time dispatcher at 0xa7291. The dispatcher checks that flag at 0x103f24. For tied scores from 0–0 through 8–8, it adds one to the score and formats a two-digit XTM filename. For larger tied scores it selects from numbers 9–32. These are verified instruction bytes, not guesses from filenames.

The native match scheduler now sends that boundary event once when it actually enters the 90-minute interval. The audio adapter uses the 26 available original XTM recordings, private bank 104; missing high-score clips are excluded from selection. It retains the original whistle, crucial-shot gate, score, rules, interval and change of ends. Speech randomness is isolated from gameplay randomness. Missing XTM or French audio retains the previous whistle-only transition, rather than incorrectly announcing a final result. Extra-time winner announcements remain the existing verified TTV route.

[Import instructions and WAV hashes](euro96-periods-import.json). The earlier 256 period records remain byte-identical.

## Investigated but unresolved

Eight HLF records have duplicated 3–2 filenames. Four contain one WAV and four contain another; neither WAV hash matches the older disc inventory. This does not establish the intended score of either recording. Missing/ambiguous half-time scores therefore retain the existing generic interval call.

The initial 0.63.0 music trace found the EURO96 loader and match-launch stop but did not establish post-match continuation. **Resolved in 0.65.0:** the main frontend loop also calls the theme handler, and the completed match returns to that loop. Results now resume the Euro theme; Actua outcome stings remain absent-theme fallback only. [Complete return-path evidence](RESULT-MUSIC.md). The [earlier partial trace and duplicate comparison](identity-audio-unresolved.json) are retained as historical evidence; the HLF ambiguity above remains unresolved.

## Validation

The player test exercises all 960 tones through the original callback in both halves: 1,920 cases comparing every mixed PCM sample and the exact clip ending. It also checks all 320 appearance mappings and renders all 32 kits from both sides. Independent Python checks compare the imported WAV hashes with the original CD inventory and the identities with the separate preview audit.

Period tests drive real 90-minute transitions under ordinary and golden-goal knockout rules, cover all nine score-specific XTM clips and every available high-score take, compare complete PCM, and check single dispatch, language/absent-media behavior and unchanged gameplay RNG. The batch also includes the full 31-match watched tournament, save/reload continuity, penalty outcomes and trophy film checks. Final results and publication are recorded in [release status](RELEASE-CANDIDATE.md). These are automated dummy-device checks; no physical audio/controller session is claimed.
