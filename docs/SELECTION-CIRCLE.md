# Selection circle smoothing — 0.65.3

The menu selection circle is a 43 × 37 indexed sprite with twelve original animation frames. Smooth menu scaling alone still showed stair steps in its rim and rotating details. A light, one-pixel filter now smooths those sprites before composition; the original frames, colours, animation order and 12-frame-per-second timing remain in use. The same filter applies to the inactive blue circles.

The separable `[1, 4, 1]` filter stays inside each sprite rectangle to avoid sampling neighbouring atlas frames. Transparent samples contribute coverage rather than black colour, and the filtered result is alpha-composited over the menu. This is a display adaptation of the original art. Archived assets, text rendering, menu hit targets and other graphics are unchanged.

[Title capture](selection-circle-title.png) and [all twelve frames](selection-circle-frames.png). Each pair in the frame sheet shows the original on the left and smoothed version on the right.

Validation uses the existing menu-art, transition and frontend checks, plus hidden Direct3D title rendering and comparison with the preceding build. [Checks](selection-circle-checks.txt), [release evidence](selection-circle-release.json). The usual launcher uses the update on its next start without interrupting an existing match.

All four checks passed in 8.64 seconds. All twelve original/filtered frame pairs and the Direct3D title capture were visually reviewed. The title capture has changes only within the circle column; cyan and white menu labels are pixel-identical to 0.65.2. [Pixel comparison](selection-circle-pixel-check.json).
