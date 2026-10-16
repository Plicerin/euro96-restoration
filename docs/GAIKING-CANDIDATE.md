# Gaiking validation — 0.63.0 RC1

Windows 11 Pro build 26200. The candidate passed nine isolated package checks, 31 full frontend tournament matches with all checkpoints resumed, 32 matrix fixtures, save recovery, player-name PCM and period/extra-time PCM checks. The same binary was tested locally and remotely. User save/preferences files were absent before and after remote testing; the checks use isolated scratch files.

Four hidden Direct3D 11 checks ran in the existing RDP desktop (session 2): gameplay rendering, title, selection and result. All passed. Captures were visually inspected for complete artwork, readable controls and layout. The results capture is a synthetic fixture, not a recorded played match. The temporary limited-privilege task was removed. These checks do not establish physical-console GPU choice, audible output, controller feel or high-DPI behavior.

Reports: [engine/package](final-candidate-gaiking-report.json), [desktop](final-candidate-gaiking-desktop.json), [package log](final-candidate-gaiking-package.txt), [tournament](final-candidate-gaiking-tournament.txt), [matrix](final-candidate-gaiking-matrix.txt), [final hash check/task cleanup](final-candidate-gaiking-final.txt).

Captures: [title](final-candidate-gaiking-title.bmp), [selection](final-candidate-gaiking-selection.bmp), [result](final-candidate-gaiking-result.bmp).

The final quick-start/manifest refresh was hash-verified remotely after testing; executable and media bytes did not change. Candidate package remains at:

`C:/Users/vrock/AppData/Local/Temp/Euro96-candidate-e9971331a96e4d35b1268ed49164fda5/package`

Use Start-Euro96.cmd there for hands-on testing when convenient. No running game was restarted. [Current release gates](COMPLETION-CHECKLIST.md).
