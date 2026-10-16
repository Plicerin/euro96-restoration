param([string]$PackageDirectory = $PSScriptRoot, [switch]$Smoke)
$ErrorActionPreference = 'Stop'
$taskRoot = (Resolve-Path -LiteralPath $PackageDirectory).Path.TrimEnd('\','/')
$taskManifest = Get-Content -LiteralPath (Join-Path $taskRoot 'package.json') -Raw | ConvertFrom-Json
if ($taskManifest.schema -ne 1 -or $taskManifest.version -notmatch '^\d+\.\d+\.\d+$') { throw 'Invalid package manifest.' }
if ((Get-Item -LiteralPath $taskRoot).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Package root must not be a link.' }
$taskEntries = @(Get-ChildItem -LiteralPath $taskRoot -Recurse -Force)
foreach ($taskEntry in $taskEntries) {
    if ($taskEntry.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw "Package contains a link: $($taskEntry.FullName)" }
}
$taskKnown = @{}
foreach ($taskFile in $taskManifest.files) {
    $taskRelative = [string]$taskFile.path
    if (-not $taskRelative -or $taskRelative -match '[:\\]' -or $taskRelative.StartsWith('/') -or ($taskRelative.Split('/') | Where-Object { $_ -in @('','..','.') })) { throw "Invalid package path: $taskRelative" }
    if ($taskKnown.ContainsKey($taskRelative) -or $taskRelative -eq 'package.json') { throw "Duplicate or reserved package path: $taskRelative" }
    $taskKnown[$taskRelative] = $true
    $taskPath = [IO.Path]::GetFullPath((Join-Path $taskRoot $taskRelative))
    if (-not $taskPath.StartsWith($taskRoot + [IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) { throw 'Package path escapes root.' }
    if (-not (Test-Path -LiteralPath $taskPath -PathType Leaf)) { throw "Missing package file: $taskRelative" }
    if ((Get-Item -LiteralPath $taskPath).Length -ne $taskFile.bytes -or (Get-FileHash -LiteralPath $taskPath -Algorithm SHA256).Hash -ne $taskFile.sha256) { throw "Package hash/size mismatch: $taskRelative" }
}
$taskRequired = @('actua96.exe','SDL3.dll','SDL-LICENSE.txt','READ-ME.txt','Start-Actua96.cmd','verify_package.ps1')
foreach ($taskStem in @('EURO','EUROREND','FAP','FAPF')) { foreach ($taskExt in @('DAT','OFF')) { $taskRequired += "data/$taskStem.$taskExt" } }
foreach ($taskRelative in $taskRequired) { if (-not $taskKnown.ContainsKey($taskRelative)) { throw "Required file absent from manifest: $taskRelative" } }
foreach ($taskFile in $taskEntries | Where-Object { -not $_.PSIsContainer }) {
    $taskRelative = $taskFile.FullName.Substring($taskRoot.Length+1).Replace('\','/')
    if ($taskRelative -ne 'package.json' -and -not $taskKnown.ContainsKey($taskRelative)) { throw "Unlisted package file: $taskRelative" }
}
Write-Output "Package $($taskManifest.version): $($taskKnown.Count) files hash-checked; $(@($taskManifest.missingOptionalMedia).Count) optional media absent."
if (-not $Smoke) { return }
# Copy outside the repository, including a space in the path. Neither the working
# directory nor executable ancestors can supply missing SOURCE/local-data assets.
$taskScratch = Join-Path ([IO.Path]::GetTempPath()) ('Actua96 package audit ' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $taskScratch | Out-Null
$taskIsolated = Join-Path $taskScratch 'portable game'
Copy-Item -LiteralPath $taskRoot -Destination $taskIsolated -Recurse
$taskSavedEnv = @{}
foreach ($taskName in @('SDL_VIDEODRIVER','SDL_AUDIODRIVER','SDL_RENDER_DRIVER')) { $taskSavedEnv[$taskName]=[Environment]::GetEnvironmentVariable($taskName) }
try {
    $env:SDL_VIDEODRIVER='dummy';$env:SDL_AUDIODRIVER='dummy';$env:SDL_RENDER_DRIVER='software'
    $taskChecks = @(
        @{argument='--version';marker=('(?m)^' + [regex]::Escape($taskManifest.version) + '\s*$')},
        @{argument='--verify';marker='15 backgrounds, 10 tactics tables verified'},
        @{argument='--frontend-smoke';marker='Native render completed:'},
        @{argument='--tournament-save-smoke';marker='Production save directory verified'}
        @{argument='--replay-file-smoke';marker='Replay file keyboard/controller save'}
        @{argument='--head-to-head-smoke';marker='Local head-to-head SDL setup'}
    )
    if ([version]$taskManifest.version -ge [version]'0.56.0') { $taskChecks += @{argument='--startup-smoke';marker='Euro96 direct startup, legacy shortcut isolation and menu navigation passed'} }
    if ($taskKnown.ContainsKey('audio/COMMENTARY/EURO_EVENTS.BIN')) {
        $taskChecks += @{argument='--audio-smoke';marker='Native render completed:'}
        $taskChecks += @{argument='--commentary-smoke';marker='Native render completed:'}
    }
    if ($taskKnown.ContainsKey('audio/COMMENTARY/FRENCH.BIN')) { $taskChecks[2].marker='French/English voice keyboard and gamepad integration passed' }
    if ($taskKnown.ContainsKey('audio/VIDEO/MAKING.AMV')) { $taskChecks += @{argument='--movie-smoke';marker='CD movie title, keyboard, gamepad, focus and return checks passed'} }
    if ($taskKnown.ContainsKey('audio/VIDEO/INTRO.AMV')) { $taskChecks += @{argument='--intro-smoke';marker='Original intro startup, replay, keyboard, gamepad, focus and completion passed'} }
    foreach ($taskCheck in $taskChecks) {
        $taskLog = Join-Path $taskScratch ($taskCheck.argument.TrimStart('-') + '.txt')
        $taskErrorLog = $taskLog + '.err'
        $taskProcess = Start-Process -FilePath (Join-Path $taskIsolated 'actua96.exe') -ArgumentList $taskCheck.argument -WorkingDirectory $taskScratch -WindowStyle Hidden -PassThru -RedirectStandardOutput $taskLog -RedirectStandardError $taskErrorLog
        # Windows PowerShell 5.1 can lose ExitCode if a short-lived process
        # exits before its handle is retained. Keep it before waiting.
        $taskProcessHandle = $taskProcess.Handle
        if (-not $taskProcess.WaitForExit(60000)) { $taskProcess.Kill(); throw "Package check timed out: $($taskCheck.argument)" }
        $taskProcess.WaitForExit()
        if ($taskProcess.ExitCode -ne 0 -or (Get-Content -LiteralPath $taskLog -Raw) -notmatch $taskCheck.marker) { throw "Package check failed: $($taskCheck.argument). Logs: $taskScratch" }
        Write-Output "Isolated package passed $($taskCheck.argument)"
    }
} finally {
    foreach ($taskName in $taskSavedEnv.Keys) { [Environment]::SetEnvironmentVariable($taskName,$taskSavedEnv[$taskName]) }
}
Write-Output "Isolated package and logs retained: $taskScratch"
