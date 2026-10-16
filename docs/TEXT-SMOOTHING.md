# Selected text contrast — 0.65.2

The original cyan font ramp has pale edges around a darker blue centre, giving selected labels and cyan headings an outlined appearance even after filtering. These letters now use the white font's dark-to-light shading, tinted bright cyan. This is a deliberate readability adjustment to the in-memory font palette; archived artwork is unchanged. White/dark/blue text ramps, glyph shapes, spacing, menu backgrounds and controls retain their existing rendering.

[Updated title](selected-text-title.png), [updated pause menu](selected-text-paused.png), [menu checks](selected-text-checks.txt), [release evidence](selected-text-release.json).

Four existing menu/art/transition/frontend checks passed in 7.55 seconds. Hidden Direct3D 11 title and pause captures were visually reviewed; the unselected white title-menu labels are [pixel-identical to 0.65.1](selected-text-pixel-check.json). This update changes cyan lettering throughout the shared menu renderer, including selected entries, focus rows and headings, while retaining the previous smoothing.

# Text smoothing — 0.65.1

The recovered 640 × 480 menu artwork was enlarged with nearest-pixel sampling. At fractional window sizes this produced irregular strokes and rough edges, especially on the small pause-menu font. Menus and the match HUD now use bilinear filtering. The shared menu path covers pause, setup, substitutions, replay controls, title/options, teams/tactics, championship tables and result screens. Retained frames during menu fades also use smooth scaling.

Original glyphs, colour gradients, text measurements and hit targets are preserved. Filtering applies to the composed menu/HUD images, so their panel borders and artwork also soften slightly. Pitch, player and stadium texture sampling is unchanged. This is display smoothing of the original bitmap fonts, not a replacement vector font or newly recovered retail feature; at large scales their original low resolution remains visible.

Uploads use premultiplied alpha before filtering, avoiding dark fringes around transparent lettering while preserving translucent HUD panels. The original CPU pixel buffers remain unchanged, and the upload buffers are reused.

Twelve existing menu, overlay, frontend and presentation checks passed in 18.33 seconds, including authored font/header pixels, keyboard/controller/pointer targets, transition timing, scaled/letterboxed composition, renderer reset and gameplay-state isolation. [Test log](text-smoothing-checks.txt). Hidden Direct3D 11 captures at 1200 × 830 were reviewed for the pause menu, title and match HUD; software captures also cover the controller pause menu at 960 × 664.

[Pause before](text-smoothing-pause-before.png), [pause after](text-smoothing-pause-after.png), [title](text-smoothing-title.png), [match HUD](text-smoothing-hud.png).

The usual launcher picks up this update on its next start. A running match is not restarted. See [release verification](text-smoothing-release.json) and the [completion checklist](COMPLETION-CHECKLIST.md) for the remaining restoration work.
