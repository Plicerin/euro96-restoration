param([string]$Configuration = 'Release', [string]$BuildDirectory = '')
$ErrorActionPreference = 'Stop'
$taskRepo = Split-Path -Parent $PSScriptRoot
$taskOutput = Split-Path -Parent $taskRepo
$taskRoot = Join-Path $taskOutput 'Actua96-Playtest'
$taskBuilds = Join-Path $taskRoot 'builds'
if (-not $BuildDirectory) { $BuildDirectory = Join-Path $taskRepo 'build' }
$taskBinary = Join-Path $BuildDirectory "$Configuration/actua96.exe"
$taskDll = Join-Path $BuildDirectory "$Configuration/SDL3.dll"
$taskExeHash = (Get-FileHash -LiteralPath $taskBinary -Algorithm SHA256).Hash
$taskDllHash = (Get-FileHash -LiteralPath $taskDll -Algorithm SHA256).Hash
$taskId = $taskExeHash.Substring(0,16).ToLowerInvariant() + '-' + $taskDllHash.Substring(0,8).ToLowerInvariant()
$taskPitchSource = Join-Path $taskRepo 'local-data/DIGI/PITCH/PITCH.MAD'
$taskPitchHash = ''
if (Test-Path -LiteralPath $taskPitchSource) {
    $taskPitchHash = (Get-FileHash -LiteralPath $taskPitchSource -Algorithm SHA256).Hash
    $taskId += '-' + $taskPitchHash.Substring(0,8).ToLowerInvariant()
}
$taskPresentation = @()
foreach ($taskRelative in @('DIGI/MUSIC/MENU.WAV','DIGI/MUSIC/WIN.WAV','DIGI/MUSIC/LOSE.WAV','DIGI/MUSIC/DRAW.WAV','COMMENTARY/ENGLISH3.BIN','GRAPHICS/TEAMS.BIN','GRAPHICS/EURO96.BIN','GRAPHICS/EURO_PREVIEWS.BIN','VIDEO/EURO_CUP.BIN','GRAPHICS/NUMBERS.BIN','GRAPHICS/PLAYERS.BIN','COMMENTARY/EURO_ENGLISH.BIN','COMMENTARY/EURO_EVENTS.BIN','COMMENTARY/EURO_PERIODS.BIN','COMMENTARY/EURO_SEQUENCES.BIN','DIGI/PITCH/EURO_PITCH.MAD','DIGI/MUSIC/EURO96.WAV','COMMENTARY/EURO_NAMES.BIN','GRAPHICS/STADIUM.BIN','GRAPHICS/SCOTLAND.BIN','GRAPHICS/SPAIN.BIN','GRAPHICS/ITALY.BIN','GRAPHICS/BRAZIL.BIN','GRAPHICS/EURO_STADIUM0.BIN','GRAPHICS/EURO_STADIUM1.BIN','GRAPHICS/EURO_STADIUM2.BIN','GRAPHICS/EURO_STADIUM3.BIN','GRAPHICS/EURO_STADIUM4.BIN','GRAPHICS/EURO_STADIUM5.BIN','GRAPHICS/EURO_STADIUM6.BIN','GRAPHICS/EURO_STADIUM7.BIN','GRAPHICS/EURO_BOARDS.BIN','COMMENTARY/NAMES.BIN','COMMENTARY/FRENCH.BIN','COMMENTARY/FRENCH_NAMES.BIN')) {
    $taskAssetSource = Join-Path $taskRepo ('local-data/' + $taskRelative)
    if (Test-Path -LiteralPath $taskAssetSource) {
        $taskPresentation += [ordered]@{path=$taskRelative;sha256=(Get-FileHash -LiteralPath $taskAssetSource -Algorithm SHA256).Hash}
    }
}
if ($taskPresentation.Count) {
    $taskHasher = [Security.Cryptography.SHA256]::Create()
    try {
        $taskMediaBytes = [Text.Encoding]::UTF8.GetBytes(($taskPresentation | ConvertTo-Json -Compress))
        $taskMediaHash = [BitConverter]::ToString($taskHasher.ComputeHash($taskMediaBytes)).Replace('-','').ToLowerInvariant()
    } finally { $taskHasher.Dispose() }
    $taskId += '-' + $taskMediaHash.Substring(0,8)
}
$taskSnapshot = Join-Path $taskBuilds $taskId
New-Item -ItemType Directory -Force -Path $taskSnapshot | Out-Null
foreach ($taskSource in @($taskBinary,$taskDll)) {
    $taskTarget = Join-Path $taskSnapshot (Split-Path -Leaf $taskSource)
    if (-not (Test-Path -LiteralPath $taskTarget)) { Copy-Item -LiteralPath $taskSource -Destination $taskTarget }
    if ((Get-FileHash -LiteralPath $taskSource).Hash -ne (Get-FileHash -LiteralPath $taskTarget).Hash) {
        throw "Playtest snapshot mismatch; existing snapshot was not overwritten: $taskTarget"
    }
}
if ($taskPitchHash) {
    $taskPitchFolder = Join-Path $taskSnapshot 'audio/DIGI/PITCH'
    New-Item -ItemType Directory -Force -Path $taskPitchFolder | Out-Null
    $taskPitchTarget = Join-Path $taskPitchFolder 'PITCH.MAD'
    if (-not (Test-Path -LiteralPath $taskPitchTarget)) { Copy-Item -LiteralPath $taskPitchSource -Destination $taskPitchTarget }
    if ((Get-FileHash -LiteralPath $taskPitchTarget -Algorithm SHA256).Hash -ne $taskPitchHash) { throw 'Pitch audio snapshot mismatch.' }
}
foreach ($taskAsset in $taskPresentation) {
    $taskAssetSource = Join-Path $taskRepo ('local-data/' + $taskAsset.path)
    $taskAssetTarget = Join-Path $taskSnapshot ('audio/' + $taskAsset.path)
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $taskAssetTarget) | Out-Null
    if (-not (Test-Path -LiteralPath $taskAssetTarget)) { Copy-Item -LiteralPath $taskAssetSource -Destination $taskAssetTarget }
    if ((Get-FileHash -LiteralPath $taskAssetTarget -Algorithm SHA256).Hash -ne $taskAsset.sha256) { throw "Presentation snapshot mismatch: $($taskAsset.path)" }
}
$taskCmake = Get-Content -LiteralPath (Join-Path $taskRepo 'CMakeLists.txt') -Raw
$taskVersion = [regex]::Match($taskCmake,'project\(Actua96Windows VERSION ([0-9.]+)').Groups[1].Value
$taskManifest = [ordered]@{
    schema = 1
    version = $taskVersion
    buildId = $taskId
    executable = Join-Path $taskSnapshot 'actua96.exe'
    data = Join-Path $taskRepo 'SOURCE'
    exeSha256 = $taskExeHash
    dllSha256 = $taskDllHash
    pitchSha256 = $taskPitchHash
    presentationAssets = $taskPresentation
    publishedUtc = [DateTime]::UtcNow.ToString('o')
}
$taskCurrent = Join-Path $taskRoot 'current.json'
$taskTemporary = Join-Path $taskRoot ('current-' + [guid]::NewGuid().ToString('N') + '.tmp')
$taskManifest | ConvertTo-Json | Set-Content -LiteralPath $taskTemporary -Encoding UTF8
# Publish the pointer only after the executable, library and media have been hash-checked.
if (Test-Path -LiteralPath $taskCurrent) { [IO.File]::Replace($taskTemporary,$taskCurrent,(Join-Path $taskRoot 'previous.json')) }
else { [IO.File]::Move($taskTemporary,$taskCurrent) }
$taskLauncher = Join-Path $taskOutput 'Play-Euro96.cmd'
@('@echo off','powershell.exe -NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File "%~dp0actuasoccer96-windows\tools\playtest.ps1"') |
    Set-Content -LiteralPath $taskLauncher -Encoding ASCII
Copy-Item -LiteralPath $taskLauncher -Destination (Join-Path $taskOutput 'Play-Actua96.cmd') -Force
Write-Output "Playtest ready: version $taskVersion ($taskId)"
Write-Output "Open: $taskLauncher"

$taskGraphicsLauncher = Join-Path $taskOutput 'View-Original-Actua96.cmd'
@('@echo off','powershell.exe -NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File "%~dp0actuasoccer96-windows\tools\playtest.ps1" -Mode front-end') |
    Set-Content -LiteralPath $taskGraphicsLauncher -Encoding ASCII
