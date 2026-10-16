param([string]$Configuration = 'Release', [string]$BuildDirectory = '')
$ErrorActionPreference = 'Stop'
if (-not $BuildDirectory) { $BuildDirectory = Join-Path $PSScriptRoot 'build' }
$taskBinary = Join-Path $BuildDirectory "$Configuration/actua96.exe"
$taskDll = Join-Path $BuildDirectory "$Configuration/SDL3.dll"
foreach ($taskRequired in @($taskBinary,$taskDll)) {
    if (-not (Test-Path -LiteralPath $taskRequired -PathType Leaf)) { throw "Missing build file: $taskRequired" }
}
$taskVersion = [regex]::Match((Get-Content -LiteralPath (Join-Path $PSScriptRoot 'CMakeLists.txt') -Raw),'project\(Actua96Windows VERSION ([0-9.]+)').Groups[1].Value
$taskBinaryVersion = (& $taskBinary --version --verify | Out-String).Trim()
if ($LASTEXITCODE -ne 0 -or $taskBinaryVersion -ne $taskVersion) { throw "Build version '$taskBinaryVersion' does not match source $taskVersion. Rebuild first." }
$taskLicense = Get-ChildItem -Path (Join-Path $PSScriptRoot '_deps'),(Join-Path $PSScriptRoot '../../work/deps/sdl3') -Filter LICENSE.txt -Recurse -ErrorAction SilentlyContinue |
    Where-Object { $_.FullName -match 'SDL3-3\.4\.18' } | Select-Object -First 1
if (-not $taskLicense) { throw 'SDL 3.4.18 license missing; cannot produce a complete package.' }
# Never overlay a previous release: stale optional media must not survive packaging.
$taskPackage = Join-Path $PSScriptRoot ('dist/actua96-' + $taskVersion + '-' + [guid]::NewGuid().ToString('N'))
$taskData = Join-Path $taskPackage 'data'
New-Item -ItemType Directory -Path $taskData | Out-Null
Copy-Item -LiteralPath $taskBinary,$taskDll -Destination $taskPackage
foreach ($taskStem in @('EURO','EUROREND','FAP','FAPF')) {
    foreach ($taskExt in @('DAT','OFF')) {
        Copy-Item -LiteralPath (Join-Path $PSScriptRoot "SOURCE/$taskStem.$taskExt") -Destination $taskData
    }
}
$taskMissingMedia = @()
foreach ($taskRelative in @('DIGI/PITCH/PITCH.MAD','DIGI/MUSIC/MENU.WAV','DIGI/MUSIC/WIN.WAV','DIGI/MUSIC/LOSE.WAV','DIGI/MUSIC/DRAW.WAV','COMMENTARY/ENGLISH3.BIN','GRAPHICS/TEAMS.BIN','GRAPHICS/EURO96.BIN','GRAPHICS/EURO_PREVIEWS.BIN','VIDEO/EURO_CUP.BIN','GRAPHICS/NUMBERS.BIN','GRAPHICS/PLAYERS.BIN','COMMENTARY/EURO_ENGLISH.BIN','COMMENTARY/EURO_EVENTS.BIN','COMMENTARY/EURO_PERIODS.BIN','COMMENTARY/EURO_SEQUENCES.BIN','DIGI/PITCH/EURO_PITCH.MAD','DIGI/MUSIC/EURO96.WAV','COMMENTARY/EURO_NAMES.BIN','GRAPHICS/STADIUM.BIN','GRAPHICS/SCOTLAND.BIN','GRAPHICS/SPAIN.BIN','GRAPHICS/ITALY.BIN','GRAPHICS/BRAZIL.BIN','GRAPHICS/EURO_STADIUM0.BIN','GRAPHICS/EURO_STADIUM1.BIN','GRAPHICS/EURO_STADIUM2.BIN','GRAPHICS/EURO_STADIUM3.BIN','GRAPHICS/EURO_STADIUM4.BIN','GRAPHICS/EURO_STADIUM5.BIN','GRAPHICS/EURO_STADIUM6.BIN','GRAPHICS/EURO_STADIUM7.BIN','GRAPHICS/EURO_BOARDS.BIN','COMMENTARY/NAMES.BIN','COMMENTARY/FRENCH.BIN','COMMENTARY/FRENCH_NAMES.BIN')) {
    $taskMedia = Join-Path $PSScriptRoot ('local-data/' + $taskRelative)
    if (Test-Path -LiteralPath $taskMedia -PathType Leaf) {
        $taskTarget = Join-Path $taskPackage ('audio/' + $taskRelative)
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $taskTarget) | Out-Null
        Copy-Item -LiteralPath $taskMedia -Destination $taskTarget
    } else { $taskMissingMedia += $taskRelative }
}
@('@echo off','cd /d "%~dp0"','start "" "%~dp0actua96.exe" --front-end') | Set-Content -LiteralPath (Join-Path $taskPackage 'Start-Actua96.cmd') -Encoding ascii
Copy-Item -LiteralPath (Join-Path $taskPackage 'Start-Actua96.cmd') -Destination (Join-Path $taskPackage 'Start-Euro96.cmd')
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'docs/QUICKSTART.txt') -Destination (Join-Path $taskPackage 'READ-ME.txt')
Copy-Item -LiteralPath $taskLicense.FullName -Destination (Join-Path $taskPackage 'SDL-LICENSE.txt')
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'tools/verify_package.ps1') -Destination $taskPackage
foreach ($taskLaunch in @(@('Ball-Physics','--physics'),@('Player-Practice','--practice'),@('Opponent-Challenge','--opponents'))) {
    @('@echo off','cd /d "%~dp0"',('start "" "%~dp0actua96.exe" ' + $taskLaunch[1])) | Set-Content -LiteralPath (Join-Path $taskPackage ('Start-' + $taskLaunch[0] + '.cmd')) -Encoding ascii
}
$taskFiles = @(Get-ChildItem -LiteralPath $taskPackage -Recurse -File | Sort-Object FullName | ForEach-Object {
    [ordered]@{path=$_.FullName.Substring($taskPackage.Length+1).Replace('\','/');bytes=$_.Length;sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash}
})
[ordered]@{schema=1;version=$taskVersion;createdUtc=[DateTime]::UtcNow.ToString('o');missingOptionalMedia=$taskMissingMedia;files=$taskFiles} |
    ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $taskPackage 'package.json') -Encoding UTF8
& (Join-Path $PSScriptRoot 'tools/verify_package.ps1') -PackageDirectory $taskPackage
if ($taskMissingMedia.Count) { Write-Warning ('Optional CD media absent: ' + ($taskMissingMedia -join ', ')) }
Write-Output "Portable preview: $taskPackage"
