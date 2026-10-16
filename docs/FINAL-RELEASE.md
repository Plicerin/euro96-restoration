# UEFA Euro 96 — final restoration 0.70.0

Finalized on October 16, 2026, at the user's request. Git tag: `v0.70.0-final`.

This release closes the agreed Windows restoration scope: original-engine matches and championships, recovered Euro presentation and audio, keyboard/gamepad controls, local two-controller exhibitions, saved championships and replays. Future graphics upgrades and browser portability research are separate from this baseline.

## Preserved build

- Version: **0.70.0**
- Build ID: `a54dbd48ccbc4175-2b4b128d-0703f4d7-479329d4`
- Executable SHA-256: `a54dbd48ccbc417547f68ceb44ac3d4c73beb2eb5d2b6abaceec5c89880dad90`
- Full regression: **109/109 passed**.
- Portable package: **54 file hashes and nine isolated smoke checks passed**.
- The built executable, portable package and published launcher executable match. The 427 preserved source files, imported media and user saves/preferences were unchanged by the final presentation batch.

[Release identity](period-presentation-release.json), [full regression](period-presentation-regression.txt), [portable checks](period-presentation-package-smoke.txt), [launcher verification](period-presentation-launcher-verify.txt).

## Archive boundaries

Git preserves the restoration source, build/import tools, documentation and validation evidence alongside the upstream preserved source. Imported disc media (`local-data`), build dependencies/output, user data and portable packages remain excluded by `.gitignore`. The verified complete portable package remains in the local `dist` directory identified in the release record; the existing Play-Euro96 launcher continues to use the published build.

Final means the agreed restoration work is closed, not exact parity with every retail behavior. Known limits include malformed retail penalty-board data, unverified period/winner-board scheduling, some native frontend behavior, rare ambiguous half-time commentary and the optional Actua French pack. Physical two-controller validation is deferred; audible audio-device switching has not been certified. These are recorded in the [closeout](RESTORATION-CLOSEOUT.md) and [completion checklist](COMPLETION-CHECKLIST.md).

Do not move or replace the final tag for later enhancements. Preserve this baseline and develop any graphics or web work separately.
