param([string]$Configuration = 'Release', [switch]$Package)
$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath $PSScriptRoot
$taskVersion = '3.4.18'
$taskHash = '78D84602AE616CFE26B33A73B7D3B9B6B56E8A6650E53D92C9C6CA938F747ACD'
$taskDeps = Join-Path $PSScriptRoot '_deps'
$taskArchive = Join-Path $taskDeps 'SDL3-devel.zip'
$taskSdl = Join-Path $taskDeps ('SDL3-' + $taskVersion)
New-Item -ItemType Directory -Force -Path $taskDeps | Out-Null
if (-not (Test-Path -LiteralPath $taskArchive)) {
    Invoke-WebRequest "https://github.com/libsdl-org/SDL/releases/download/release-$taskVersion/SDL3-devel-$taskVersion-VC.zip" -OutFile $taskArchive
}
if ((Get-FileHash -LiteralPath $taskArchive -Algorithm SHA256).Hash -ne $taskHash) {
    throw 'SDL download checksum mismatch. Remove _deps/SDL3-devel.zip and retry.'
}
if (-not (Test-Path -LiteralPath (Join-Path $taskSdl 'cmake/SDL3Config.cmake'))) {
    Expand-Archive -LiteralPath $taskArchive -DestinationPath $taskDeps -Force
}
$taskPython = (Get-Command python -ErrorAction Stop).Source
& cmake -S . -B build -G 'Visual Studio 17 2022' -A x64 "-DSDL3_DIR=$taskSdl/cmake" "-DPython3_EXECUTABLE=$taskPython" -DCMAKE_SUPPRESS_REGENERATION=ON
if ($LASTEXITCODE) { throw 'CMake configuration failed.' }
& cmake --build build --config $Configuration --parallel
if ($LASTEXITCODE) { throw 'Compilation failed.' }
& ctest --test-dir build -C $Configuration --output-on-failure
if ($LASTEXITCODE) { throw 'Verification failed.' }
& $taskPython tools/audit_source.py
if ($LASTEXITCODE) { throw 'Source audit failed.' }
Copy-Item -LiteralPath 'build/generated/engine-recovery.json' -Destination 'docs/engine-recovery.json'
Copy-Item -LiteralPath 'build/generated/player-recovery.json' -Destination 'docs/player-recovery.json'
Copy-Item -LiteralPath 'build/generated/graphics-recovery.json' -Destination 'docs/graphics-recovery.json'
Copy-Item -LiteralPath 'build/generated/roster-camera-recovery.json' -Destination 'docs/roster-camera-recovery.json'
& (Join-Path $PSScriptRoot 'tools/publish_playtest.ps1') -Configuration $Configuration
if ($Package) { & (Join-Path $PSScriptRoot 'package-windows.ps1') -Configuration $Configuration }

