param(
    [ValidateSet('opponents','practice','physics','original-3d','actions','cpu-teams','match-setup','front-end')][string]$Mode = 'front-end',
    [switch]$SmokeTest,
    [switch]$VerifyOnly
)
$ErrorActionPreference = 'Stop'
$taskRepo = Split-Path -Parent $PSScriptRoot
$taskRoot = Join-Path (Split-Path -Parent $taskRepo) 'Actua96-Playtest'
try {
    $taskManifest = Get-Content -LiteralPath (Join-Path $taskRoot 'current.json') -Raw -Encoding UTF8 | ConvertFrom-Json
    if ($taskManifest.schema -ne 1) { throw 'Unsupported playtest manifest.' }
    $taskBuilds = [IO.Path]::GetFullPath((Join-Path $taskRoot 'builds')) + [IO.Path]::DirectorySeparatorChar
    $taskExe = [IO.Path]::GetFullPath($taskManifest.executable)
    if (-not $taskExe.StartsWith($taskBuilds,[StringComparison]::OrdinalIgnoreCase) -or
        (Split-Path -Leaf $taskExe) -ne 'actua96.exe') { throw 'Invalid playtest executable path.' }
    $taskDll = Join-Path (Split-Path -Parent $taskExe) 'SDL3.dll'
    if ((Get-FileHash -LiteralPath $taskExe).Hash -ne $taskManifest.exeSha256 -or
        (Get-FileHash -LiteralPath $taskDll).Hash -ne $taskManifest.dllSha256) { throw 'Playtest files failed verification. Rebuild before launching.' }
    foreach ($taskStem in @('EURO','EUROREND','FAP','FAPF')) {
        foreach ($taskExtension in @('DAT','OFF')) {
            if (-not (Test-Path -LiteralPath (Join-Path $taskManifest.data "$taskStem.$taskExtension"))) {
                throw 'Shared game data is missing. Keep the source project beside the launcher.'
            }
        }
    }
    if ($taskManifest.pitchSha256) {
        $taskPitch = Join-Path (Split-Path -Parent $taskExe) 'audio/DIGI/PITCH/PITCH.MAD'
        if ((Get-FileHash -LiteralPath $taskPitch -Algorithm SHA256).Hash -ne $taskManifest.pitchSha256) { throw 'Original pitch audio failed verification.' }
    }
    $taskAllowedPresentation = @('FRONTEND/TITLE.IDX','FRONTEND/TITLE.PAL','DIGI/MUSIC/MENU.WAV','DIGI/MUSIC/WIN.WAV','DIGI/MUSIC/LOSE.WAV','DIGI/MUSIC/DRAW.WAV','COMMENTARY/ENGLISH3.BIN','GRAPHICS/TEAMS.BIN','GRAPHICS/EURO96.BIN','GRAPHICS/EURO_PREVIEWS.BIN','VIDEO/EURO_CUP.BIN','GRAPHICS/NUMBERS.BIN','GRAPHICS/PLAYERS.BIN','COMMENTARY/EURO_ENGLISH.BIN','COMMENTARY/EURO_EVENTS.BIN','COMMENTARY/EURO_PERIODS.BIN','COMMENTARY/EURO_SEQUENCES.BIN','DIGI/PITCH/EURO_PITCH.MAD','DIGI/MUSIC/EURO96.WAV','COMMENTARY/EURO_NAMES.BIN','GRAPHICS/STADIUM.BIN','GRAPHICS/SCOTLAND.BIN','GRAPHICS/SPAIN.BIN','GRAPHICS/ITALY.BIN','GRAPHICS/BRAZIL.BIN','GRAPHICS/EURO_STADIUM0.BIN','GRAPHICS/EURO_STADIUM1.BIN','GRAPHICS/EURO_STADIUM2.BIN','GRAPHICS/EURO_STADIUM3.BIN','GRAPHICS/EURO_STADIUM4.BIN','GRAPHICS/EURO_STADIUM5.BIN','GRAPHICS/EURO_STADIUM6.BIN','GRAPHICS/EURO_STADIUM7.BIN','GRAPHICS/EURO_BOARDS.BIN','COMMENTARY/NAMES.BIN','COMMENTARY/FRENCH.BIN','COMMENTARY/FRENCH_NAMES.BIN','VIDEO/MAKING.AMV','VIDEO/INTRO.AMV')
    foreach ($taskAsset in $taskManifest.presentationAssets) {
        if ($taskAsset.path -notin $taskAllowedPresentation) { throw 'Invalid presentation asset path.' }
        $taskAssetFile = Join-Path (Split-Path -Parent $taskExe) ('audio/' + $taskAsset.path)
        if ((Get-FileHash -LiteralPath $taskAssetFile -Algorithm SHA256).Hash -ne $taskAsset.sha256) { throw "Presentation asset failed verification: $($taskAsset.path)" }
    }
    if ($VerifyOnly) { Write-Output "Ready: $($taskManifest.version) / $($taskManifest.buildId)"; exit 0 }
    $taskArguments = @("--$Mode",'--data',('"' + $taskManifest.data + '"'))
    if ($SmokeTest) {
        $taskArguments[0] = if ($Mode -eq 'front-end') { '--frontend-smoke' } elseif ($Mode -eq 'original-3d') { '--graphics-smoke' } elseif ($Mode -eq 'cpu-teams') { '--cpu-smoke' } elseif ($Mode -eq 'match-setup') { '--setup-smoke' } else { "--$Mode-smoke" }
        $env:SDL_AUDIODRIVER='dummy';$env:SDL_VIDEODRIVER='dummy';$env:SDL_RENDER_DRIVER='software'
        $taskTest = Start-Process -FilePath $taskExe -ArgumentList $taskArguments -WorkingDirectory (Split-Path -Parent $taskExe) -WindowStyle Hidden -PassThru -Wait
        if ($taskTest.ExitCode -ne 0) { throw "Playtest smoke failed ($($taskTest.ExitCode))." }
        Write-Output "Playtest smoke passed: $Mode / $($taskManifest.version)"
        exit 0
    }
    # Only close the specific process previously opened by this launcher. Other
    # extracted/release builds and unrelated applications are never stopped.
    $taskSessionFile = Join-Path $taskRoot 'session.json'
    if (Test-Path -LiteralPath $taskSessionFile) {
        $taskSession = Get-Content -LiteralPath $taskSessionFile -Raw -Encoding UTF8 | ConvertFrom-Json
        $taskPrevious = Get-Process -Id $taskSession.processId -ErrorAction SilentlyContinue
        if ($taskPrevious -and $taskPrevious.Path -eq $taskSession.executable -and
            $taskPrevious.Path.StartsWith($taskBuilds,[StringComparison]::OrdinalIgnoreCase) -and
            $taskPrevious.StartTime.ToUniversalTime().Ticks -eq [long]$taskSession.startedTicks) {
            if (-not $taskPrevious.CloseMainWindow() -or -not $taskPrevious.WaitForExit(4000)) {
                throw 'Close the existing Euro 96 playtest window, then open Play-Euro96 again. It was not force-closed.'
            }
        }
    }
    # This is the visible interactive game requested by the user.
    $taskGame = Start-Process -FilePath $taskExe -ArgumentList $taskArguments -WorkingDirectory (Split-Path -Parent $taskExe) -PassThru
    @{processId=$taskGame.Id;executable=$taskExe;startedTicks=$taskGame.StartTime.ToUniversalTime().Ticks.ToString();buildId=$taskManifest.buildId} |
        ConvertTo-Json | Set-Content -LiteralPath $taskSessionFile -Encoding UTF8
    Write-Output "Opened Euro 96 $($taskManifest.version) / $($taskManifest.buildId)"
} catch {
    if ($SmokeTest -or $VerifyOnly) { throw }
    New-Item -ItemType Directory -Force -Path $taskRoot | Out-Null
    $_ | Out-String | Set-Content -LiteralPath (Join-Path $taskRoot 'launcher-error.txt')
    Add-Type -AssemblyName System.Windows.Forms
    [System.Windows.Forms.MessageBox]::Show($_.Exception.Message,'Euro 96 playtest') | Out-Null
    exit 1
}
