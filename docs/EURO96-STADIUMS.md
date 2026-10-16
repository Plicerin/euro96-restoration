# Retail Euro 96 stadiums — assets 0.57.0, schedule 0.64.0

The ordinary venue selector now offers Wembley, Villa Park, Elland Road, St James Park, Old Trafford, Anfield, Hillsborough and City Ground. Wembley is the default for a new session. These are playable retail stadium models, not the animated stadium previews.

## Recovery evidence

[The importer](../tools/import_euro96_stadiums.py) pins the supplied Euro96 executable and graphics archive hashes before extracting anything. The executable contains eight 244-byte stadium descriptors at file offset `0x13b06c`. Each selects two texture pages, four stand meshes, material coordinates, a pitch bitmap/palette, dimensions and tunnel positions. The contiguous venue labels at `0x13347d` follow the same order as the stadium names in decoded SCRIPT96.ENG. The importer checks the label bytes and four instruction anchors in the retail loader; [the report](euro96-stadium-import.json) records descriptor, component and output hashes.

The retail descriptor adds a stadium-palette record at offset 16 compared with the older 240-byte Actua layout. Record 60 supplies colours 192–255. Retail pitch palettes occupy 128–143; the high-resolution sky uses 144–159. Importing only the texture images into the older palette would produce incorrect colours.

The adapter preserves the retail stadium palette for textured and flat stand faces. Its existing sky renderer uses 208–223, so the sky pixels are reindexed by 64 while retaining every original colour. Ground rendering uses the first 64 rows of the retail pitch page, with the original 256-byte stride. The first supported sky variant is deterministic and does not consume gameplay RNG.

## Compatibility and boundaries

- Retail venues use IDs 5–12. Legacy Actua IDs 0–4 retain their original meanings, files and render paths for existing tournaments and saved replays. No save-format migration is required. Without the optional retail packs, the selector retains its previous five-venue fallback.
- The retail assets are separate `EURO_STADIUM0.BIN` through `EURO_STADIUM7.BIN` files. The importer leaves SOURCE and older imports untouched and refuses to overwrite differing output files.
- The existing original-engine match logic, camera, flag geometry/wind animation, net geometry and native presentation routing remain. Corner flags keep the preserved renderer's verified atlas: retail ground pages no longer contain those flag coordinates. This is not a claim that every peripheral retail renderer asset has been replaced.
- Wembley puts the video board in texture page 8. The other seven descriptors point their video scratch area at page 15, which their stand material coordinates do not reference. The adapter does not overwrite its preserved goal-net page with that unused scratch image. Full retail video dispatch remains separate work.
- Exhibition venue selection is native. New championships now use the verified [original 31-fixture venue schedule](TOURNAMENT-VENUES.md); existing saves retain their selected stadium. Exact retail display modes are not reproduced.

## Validation

The stadium contract renders all eight venues through three match cameras and an overview, exercises tunnel exits in both halves, checks frozen-frame determinism and preservation of live player/RNG state, round-trips new venue IDs through tournament saves and replay files, retains all five legacy IDs, and rejects malformed media. The existing presentation test also checks animated corner flags.

[Rendered overview and tunnel captures](euro-stadium-review.png) were reviewed together. All [95 regression checks](euro-stadium-regression.txt) passed in 445.94 seconds. All [47 packaged files and seven isolated smoke checks](euro-stadium-package.txt) passed. The [published release](euro-stadium-release.json) retains the user save and all 427 tracked SOURCE files; no running game was restarted.
