# Winner-board caller audit after 0.68.0

The supplied retail executable contains the winner-board handlers and the disc contains their artwork, but this audit still finds no match-end path that starts them. The result is evidence for a potentially dormant retail feature, not justification for inventing a new victory trigger. The game remains at 0.68.0; this investigation changes audit tooling and documentation only.

## What is now checked

The audit locates the embedded DOS image, validates its LE header, all three objects and all 209 contiguous data pages, then parses all **36,631 relocation records**. Every record uses one of four supported internal-fixup forms; unexpected forms or inconsistent source operands fail the audit. No relocation points to any of the eight stadium-board handlers. Fourteen relocations reference the board event-state variable, matching the identified renderer/handler/initialization accesses.

The executable initializes the board event state to zero. The renderer skips its animation dispatch when that state is zero. The identified setters for winner states 7 and 8 are inside the corresponding winner handlers. The only direct call candidates for those handlers come from the renderer's switch; no direct tail-jump candidates were found.

As a positive control, the same scan finds **two real gameplay calls to the goal-board handler**, at file offsets `0xb7543` and `0xb75c4`. They match the own-goal and normal-goal paths and pass zero to begin loading. Thus the search is capable of finding a genuine match event outside the board renderer; the winner paths differ from that working goal path.

This remains static evidence. A computed address, an aliased write, an external patch or a different retail revision could escape this analysis. We therefore keep the winner artwork unwired and label its original use **unverified / possibly dormant**, rather than calling it either a confirmed missing feature or definitively unused content.

## Reproduction and validation

Run `tools/verify_retail_board_limits.py` with the verified retail inspection folder and a report destination. It pins the executable and archive hashes, rechecks all 16 animation records, reproduces the unsafe penalty-frame boundary, and emits the full relocation/caller report. [Machine-readable evidence](retail-board-limits.json).

`tools/test_retail_board_limits.py` takes the pinned retail executable path. Its six checks include the known live goal callers and rejection of a damaged header, changed page map, unsupported relocation form, mismatched relocation operand and removed goal call. [Test results](winner-board-audit-checks.txt).

The LE layout and flag definitions were checked against [Open Watcom's executable-format definitions](https://github.com/open-watcom/open-watcom-v2/blob/master/bld/watcom/h/exeflat.h). No retail program or imported media is modified by the audit. Existing match-board event observation remains the documented native adaptation.
