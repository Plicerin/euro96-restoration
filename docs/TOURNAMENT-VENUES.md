# Original championship venue schedule — 0.64.0

New championships with all eight Euro stadium packs follow the original 31-fixture venue schedule. The opening England–Switzerland fixture is at Wembley; the next fixture is at Elland Road. The quarter-finals use Wembley, Anfield, Old Trafford and Villa Park; the semi-finals use Old Trafford and Wembley; the final returns to Wembley. Fixture order determines the venue even when the native controller adapter swaps the teams to give the chosen country the human slot.

## Evidence

The 31 entries in `SOURCE/EURO_VAR.CPP`'s `MatchVenues` agree exactly with the supplied retail executable at file offset `0x139a98` (data-relative `0xa244`). The executable's venue labels at `0x13347d` independently establish city/descriptor order. The city defines in the preserved header are commented out; they are not treated as compiled evidence.

At file offsets `0x9904f`–`0x99063`, retail instructions read the current fixture, read its `MatchVenues` entry, and write it to the setup stadium word at data-relative `0x868f4`. Stadium dispatch reads that same word at `0xe8917` and checks the 0–7 range. This establishes match selection in addition to the fixture-page label. The [offline verifier](../tools/verify_euro96_venues.py) checks the whole executable SHA-256, the source table, all venue labels and five instruction anchors. [Verified table and anchors](venue-schedule-retail.json).

The build extracts the schedule from preserved source, mapping retail IDs 0–7 to native IDs 5–12. SOURCE is unchanged. Existing [stadium packs and renderer](EURO96-STADIUMS.md) supply the geometry, palettes and tunnels; this change adds no generated stadium assets or gameplay logic.

## Saves and missing media

- New scheduled championships write save version 4, with a checked venue-policy field covered by the existing checksum and atomic replacement/backup logic.
- Versions 1–3 keep their saved fixed venue. Saving such a tournament still writes version 3. All native stadium IDs 0–12 retain their meanings; loading never silently upgrades an existing championship.
- New championships without the complete eight-pack set retain the fixed-venue behavior. The setup page says **ORIGINAL SCHEDULE** or **SELECTED STADIUM** accordingly, and the group hub names the next venue.
- If a scheduled championship is resumed where its next stadium is missing, progression stops with **SCHEDULED VENUE UNAVAILABLE**. Restoring the pack permits retry; no result or replacement venue is invented.
- These native save files are not DOS saves. Version 4 requires this build or later. Unfinished matches still resume from kickoff.

## Validation scope

The venue contract checks all 31 schedule entries and checkpoints, versions 1–3 across all 13 existing venue IDs, invalid policy data with a valid checksum, away-country control, frontend legacy retention and absent media. The full watched championship contract checks the actual loaded stadium before each opening, reloads every checkpoint, visits all eight venues and requires Wembley for the final. Existing stadium rendering/tunnel contracts complement this state/session coverage.

No physical controller or audible-device result is implied by these automated checks. Use the usual launcher on its next start; an existing championship deliberately keeps its original fixed venue.

All [104 regression checks](venue-schedule-regression.txt) passed. The [Gaiking repeat](venue-schedule-gaiking.json) passed both the venue/save contract and the full watched championship. The [portable package](venue-schedule-package.txt) passed 53 file hashes and nine smoke checks. Reviewed menus: [championship setup](venue-schedule-setup.png), [next fixture venue](venue-schedule-hub.png).
