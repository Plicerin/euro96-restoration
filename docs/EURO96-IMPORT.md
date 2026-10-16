# Current coverage — 0.63.0

All 320 players now have verified appearance mappings and spoken-name coverage (959 Euro tones plus one existing Actua fallback). Original score-specific extra-time announcements now play at the actual 90-minute knockout transition. [Evidence](RETAIL-IDENTITY-AUDIO.md).

The remaining sections describe the historical 0.54.0 release.

# Euro96 retail media integration - 0.54.0

**Correction after release:** retail numbers 1-20 have now been found in a different atlas, including light/dark variants. The 0.54.0 runtime still lacks their bindings; the earlier claim that the artwork itself was absent was incorrect. [Direct atlas/UV evidence](EURO96-NUMBER-DISCOVERY.md).

The supplied PC/DOS Euro96 disc now provides all 32 outfield strips (16 primary and 16 alternate), including Croatia and Turkey. The renderer uses the retail script's 16-by-16 kit table to choose the opponent's country-specific alternate. The earlier Actua pack remains available as fallback when this optional overlay is absent.

English player-name coverage increases from 181 to **308 of the existing 320 players**. Exactly matched main-roster and reserve entries supply 296 sets of three retail recordings; 12 other players retain their previously verified Actua recordings. The remaining 12 retain no name speech. Matching requires a unique normalized full name within the same country, never the same slot in a different revision. Source rosters, shirt identities, statistics and saves are unchanged. French commentary remains the existing independently imported pack.

The same 296 exact matches supply appearance flags. This batch uses the existing default appearance for WHT/MED and the existing alternate appearance for BLK. It does not introduce the retail game's expanded head or skin system.

## Evidence and rendering boundary

`tools/import_euro96.py` pins the supplied archive/script hashes, decodes the script using bounded SOURCE/UNMANGLE.C token operations, verifies record bounds and palettes, and validates **412 torso/limb UV entries byte for byte** against the preserved revision. The retail extended UV records moved nine entries; their actual coordinates remain identical. Head and goalkeeper UV layouts differ and are deliberately not transplanted.

Uniform images come from retail records 3-34, with their 32-colour palettes in records 61-92 and the common limb atlas in record 35. Colours are decoded per texture so the expanded retail palette cannot overwrite the stadium or keeper palette. Retail indices 32-63 retain the existing exposed-skin pixels; heads, goalkeeper clothing and officials remain the previously restored assets. This is a compatible outfield-art adapter, not a wholesale port of the retail renderer. [Input, output and clip provenance](euro96-import.json).

The neutral number atlas is shared: retail record 107 equals preserved record 109. It replaces the older Actua country-coloured digits on these new kits. It still contains **1-15 only**; the 0.54.0 adapter does not yet bind the separate retail 1-20 atlas. [Added in 0.54.1](SHIRT-NUMBERS.md). Retail country introduction phrases, new head variants, keeper art, frontend animations and music are not part of this import.

Optional files are `GRAPHICS/EURO96.BIN` and `COMMENTARY/EURO_NAMES.BIN`. Existing media packs remain intact, and both new files are included in portable-package and playtest-snapshot hash verification. Importing requires the staged, validated `work/euro96-inspection/EURO96.iso`; rerunning the importer is deterministic. Unknown revisions and conflicting overlays are rejected.

## Combined review

The new contract renders every strip from front and back, checks all kit choices are reachable, exercises Croatia/Turkey name callbacks and all three intonations across both halves, verifies appearance identities and rejects malformed/truncated packs. Decoder tests cover literals, overlapping copies, byte/word repeats and differences, invalid backreferences, input truncation and output limits. Imported name hashes are cross-checked against the independently captured CD inventory. Existing full-roster checks cover keepers and every starter in both halves.

[Rendered strip contact sheet](euro96-kit-render-review.png). Final regression and publication results are recorded in [release status](RELEASE-CANDIDATE.md).

During the next ordinary session, review Croatia/Turkey strips, an alternate-strip matchup such as England/Germany, close player views, name speech and a half-time/end change together. No separate test launch or ZIP extraction is required.
