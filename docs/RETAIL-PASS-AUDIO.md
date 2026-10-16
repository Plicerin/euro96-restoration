# Retail pass-commentary routing — 0.69.0

The two remaining live source-only pass events now follow verified Euro 96 behavior. Ordinary short and long passes use the same existing retail CPA bank (17 recordings). The two special set-piece reception branches clear their flag and bypass commentary entirely. No replacement audio or guessed bank is required.

## Evidence

The retail reception routine begins at file offset `0xba34f`. Its pass/random/side gates and branch structure correspond to `SOURCE/BALLINT.CPP::has_ball`. The ordinary-pass branch computes the distance, stores the result, then unconditionally selects retail event 12 at `0xba44b`. It jumps to the commentary call at `0xba479`. Descriptor 12 names CPA and provides 17 takes; there is no intervening long-versus-short comparison.

For pass kinds 1 and 2 with the set-piece flag set, the retail routine clears that flag at `0xba3b6` / `0xba3dd`. The jumps at `0xba3bd` / `0xba3e4` both reach `0xba47e`, after the commentary call. This establishes silence for these specific branches, not a blanket rule that all set pieces are silent. The preserved source emits CP_SETPIECE in those two cases.

[Pinned executable hash, instructions, descriptor and decoded jump destinations](retail-pass-audio-evidence.json). Reproduce with `tools/verify_retail_pass_audio.py` and the verified retail executable.

## Implementation boundary

When the active English voice pack is EURO_EVENTS.BIN, the audio adapter maps source event 14 to existing event/bank 13 and suppresses source event 18. Short and long reactions share pending-event identity, recent-clip history and cooldown. Switching languages atomically changes this policy; invalid language switches preserve the current one.

Older English packs and the optional French pack retain their distinct short-pass and set-piece recordings. The imported packs remain byte-identical, including retained compatibility clips. The preserved gameplay routine, its event IDs, random gate, reception/possession state and flag clearing are untouched. Other differences between the source revision and retail reception routine remain outside this change; this is not a claim of complete retail commentary-scheduler parity.

The former live pass-category gaps are resolved. FP_SHORT and FP_LOOSE remain dormant compatibility entries with no active direct symbolic source calls. Remaining audio fidelity work includes ambiguous/missing half-time score recordings and the absent Euro French recordings.

## Validation

Native checks compare complete PCM (apart from the existing edge fades) for all 17 CPA takes reached through a short-pass request. Twelve actual reception cases cover both teams, short/long distance and both special set-piece branches, checking unchanged source events, cleared flags, possession/ball state and gameplay RNG. Tests cover shared pending/cooldown behavior, repeated English/French switches, failed switching and older English fallback. Existing commentary tests verify the appropriate route for each active language.

These use dummy audio devices. Full-suite, portable-package and publication results are recorded in [release status](RELEASE-CANDIDATE.md); physical audio validation remains separate.
