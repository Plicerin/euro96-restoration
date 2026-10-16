# Original match presentation (0.26.0)

## Five-part batch

1. Original vertical pitch scanner in the lower left.
2. Original team markers and caller-colored ball sprite, with original palette decoding.
3. Original halftime team-identity mapping: the scanner stays fixed to pitch coordinates;
   colors follow the home/away roster as the teams change ends. It does not rotate with the camera.
4. Original high-resolution proportional font, clock at the top, team-colored score at
   the bottom, and pending-substitution icons.
5. A larger 4:3 match view, with restoration tabs and help panels tucked away during play.

## Controls

- P / Start: pause and show help.
- F6: toggle restoration panels. Right-stick click toggles them during live play;
  its existing paused/full-time reset binding remains available.
- Tab / F7: toggle radar. Tab retains its prior navigation behavior outside the match.
- V: existing camera selection.

Set-piece controls and manual shootout goalkeeper controls remain visible. The
set-piece bars were shifted to leave the original scanner and score unobscured.
Keeper delivery hints remain available in the larger view. Hidden restoration
tabs do not consume mouse clicks on the enlarged match. Setup and squad screens
continue to pause play and use the existing native panels.

## Evidence and boundary

FGFX.C contains three font descriptors and 129 sprite descriptors. The build
compares their serialized values against initialized data in 3DENG.OBJ. The
original loader instructions verify EUROREND records 36 (BM_EXTRA1), 37
(BM_EXTRA3) and 121 (BM_EXTRA2 at row 180), rather than trusting older FILES.C IDs.
The overlay uses font 2, radar sprites 88-92 and substitution icons 86-87.

3DENG.C supplies the scanner's high-resolution coordinates and half-dependent
marker selection, font character remapping/spacing, top clock, bottom score,
and palette indices 31/32/56. EURO_GRF.C GetTEAMname supplies the 11-character
team-name limit and final-period truncation. Sprite bytes use the original rule:
zero is transparent, one is the caller color, and other bytes select palette
index byte minus one. Font artwork and marker shapes are original assets.

SDL handles compositing and nearest-neighbor resizing. A half-opacity black
rectangle replaces the original scanner palette darkening filter; it does not
reproduce palette quantization. The doubled scanner background uses regular
nearest scaling. Coordinates outside the pitch are bounded safely for scanner
rendering; bitmap output is clipped. Unknown font characters become spaces.
The 864 x 648 viewport and help footer are native presentation changes.

The existing interpolated actor and ball presentation supplies radar positions.
Replays use replay snapshots for the radar and show REPLAY instead of a live
clock. The score uses the current match totals, not a reconstructed historical
replay score. Original DOS menus have not been recovered in this batch.
The power/height bars and pause/help panels remain native UI, not original art.
No original gameplay routines or SOURCE files are modified.

## Verification

The overlay contract checks original byte decoding, proportional font metrics,
pitch endpoints, halftime marker identity, atlas availability, safe clipping,
all three cameras, sent-off player exclusion, replay rendering and byte-for-byte
live engine isolation during drawing. The existing CPU input smoke test also
checks keyboard and gamepad panel toggles, radar toggling and hidden-toolbar
click handling. Captures cover the larger view, all cameras, pause and set pieces.
Detailed build evidence is in hud-recovery.json and presentation-test-results.txt.
