Historical release record. For current completion, compatibility boundaries and test gates, use [COMPLETION-CHECKLIST.md](COMPLETION-CHECKLIST.md).

# Gaiking independent Windows validation

Validated 2026-10-12, version 0.43.1. Access to Gaiking succeeded using its existing SSH access; SMB shares were also readable. All files were copied to a new temporary test directory. No running game was restarted, and no services or firewall settings were changed.

## Machine and results

Gaiking runs Windows 11 Pro x64, build 26200, with a Ryzen 7 5700X3D and an installed RTX 4070 (driver 32.0.16.1047). Graphics tests ran in the existing RDP desktop, which also exposes Microsoft Remote Display Adapter. These checks do not establish which physical GPU handled each operation or validate physical-console display behavior.

| Check | Result |
| --- | --- |
| Portable package inventory | All 36 file hashes passed; no optional media absent |
| Six isolated package checks | Version, archives, frontend, production save/recovery, making-of and intro passed |
| Complete frontend tournament | 31 fixtures; resume after each; 62 natural exits, 37 intervals, 57 replays and 57 goals |
| Match matrix | 32 fixtures; 16 countries, 10 formations, 73 goals, seven shootouts and two extended matches |
| Save recovery | Damaged/missing primary, backup preservation and failure-state checks passed |
| Hidden desktop Direct3D 11 | 3D smoke test, original title and intro captures passed |

The full tournament and matrix produced the same deterministic results as the development PC, including Spain winning the final against Russia on penalties after 0:0. Simulation advances fixed engine steps without waiting for real match time; the 63 fixtures are automated engine runs, not human playtests.

The first Direct3D attempts from SSH session 0 failed to create a swap chain. Retesting with a temporary limited-privilege interactive scheduled task in the existing user desktop (session 2) passed all three graphics checks. The temporary task was removed after completion. No credentials were changed. Audio used the dummy device. The title and intro captures were visually inspected.

Tournament and preference files, including their backups, were absent before and after the remote test suite. Production-directory probes use unique temporary filenames. Test files and logs remain in the dedicated temporary directory for inspection.

## PowerShell verifier repair

Windows PowerShell 5.1 returned a null `ExitCode` for a short-lived process even though `--version` printed the correct version and exited successfully. The verifier now retains the process handle immediately after `Start-Process`, before waiting for exit. This prevents the false package failure.

The refreshed portable package includes the repaired verifier and updated inventory. Its own bundled verifier passed all six smoke checks under Windows PowerShell 5.1 on Gaiking and the development PC. The native executable is byte-for-byte unchanged, so this package-tool repair does not change gameplay or require another engine build. The existing launcher remains on the verified 0.43.1 snapshot.

Remote refreshed package, on Gaiking:

```text
C:\Users\vrock\AppData\Local\Temp\Actua96-validation-0431-db8278d27a474896ae7488258a7df14c\package-final
```

Run `Start-Actua96.cmd` from that directory on Gaiking for hands-on testing. Launching an executable through a network share from another computer would run it on that other computer.

## Evidence and limits

- [Refreshed package verification](gaiking-package.txt)
- [Tournament output](gaiking-tournament.txt), [match matrix](gaiking-matrix.txt), [save recovery](gaiking-save-recovery.txt)
- [SSH test report](gaiking-report.json), including the unsuccessful session-0 graphics attempts
- [Successful desktop graphics report](gaiking-desktop-report.json)
- [Title capture](gaiking-title.png), [intro capture](gaiking-intro.png)

Still pending: a human tournament session, physical controller behavior, audible sound and device switching, fullscreen/resizing and actual display scaling on Gaiking. Remote automation does not validate these experiences.
