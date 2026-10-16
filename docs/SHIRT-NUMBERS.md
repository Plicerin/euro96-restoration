# Retail shirt numbers - 0.54.1

The restored renderer now uses the Euro96 CD's complete precomposed shirt numbers **1-20**, including both light and dark variants. Reserve players retain their squad number when substituted into the match. The same binding follows team identity after the ends change and applies to goalkeeper and mirrored animation materials.

## Original assets and native adapter

`tools/import_euro96_numbers.py` verifies the known EUROREND.DAT/OFF hashes, reads the forty number rectangles from retail texture coordinates 533-572 and extracts their pixels from record 38. Their original palette comes from record 0. It preserves the authored shapes, spacing and colours, including transparency; it does not assemble digits or redraw the font. [Per-glyph provenance](euro96-numbers-import.json) and [discovery](EURO96-NUMBER-DISCOVERY.md).

The optional `GRAPHICS/NUMBERS.BIN` pack has bounded size, palette, pixel indices and rectangle validation. Original index zero becomes alpha zero. The retail atlas uses raw indices, unlike the earlier number atlas's +1 encoding, so it is decoded separately. Its native texture page and appended UV entries leave the existing flags, markers, heads, keeper clothing and stadium pages intact. Stadium changes rebuild the appended number bindings without duplicating them.

Light/dark selection is a **native contrast adaptation** based on the current rendered torso's brightness; it is not claimed to reproduce an independently recovered DOS colour-choice table. The existing dark goalkeeper kit uses the light number variant. No gameplay, roster, substitution or save-format changes are required. Without the optional pack, the earlier 1-15 rendering and safe unnumbered 16-20 fallback remain available.

[Rendered England and France shirts 16–20](shirt-number-render-review.png).

## Validation

The dedicated rendered contract covers all twenty numbers for both teams, keepers, mirrored poses and both halves; verifies distinct glyphs and transparent backgrounds; rejects malformed packs; exercises a real number-18 substitution; and checks that all five venue changes and repeated media loading retain the number binding. Existing replay, material, team, stadium and full-game tests provide regression coverage. [Release results](RELEASE-CANDIDATE.md).

During the next normal session, a close rear view of a player numbered 16-20 is enough to inspect the new artwork. No separate test installation is required.
