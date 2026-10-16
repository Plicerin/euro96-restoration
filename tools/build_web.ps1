param(
    [string]$SdkDirectory = 'D:\Codex\euro96-web\emsdk',
    [string]$SdlSource = 'D:\Codex\euro96-web\SDL3-3.4.18',
    [string]$BuildDirectory = 'D:\Codex\euro96-web\build',
    [int]$Jobs = 4
)
$ErrorActionPreference = 'Stop'
$taskRepo = Split-Path -Parent $PSScriptRoot
$taskPython = (Get-Command python -ErrorAction Stop).Source
$taskCmake = (Get-Command cmake -ErrorAction SilentlyContinue).Source
if (-not $taskCmake) { $taskCmake = 'C:\Program Files\CMake\bin\cmake.exe' }
$taskNinja = (Get-Command ninja -ErrorAction Stop).Source
$taskToolchain = Join-Path $SdkDirectory 'upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake'
if (-not (Test-Path -LiteralPath $taskToolchain)) { throw 'Install and activate Emscripten first; see docs/BROWSER-PREVIEW.md.' }
$env:EMSDK = $SdkDirectory
$env:EM_CONFIG = Join-Path $SdkDirectory '.emscripten'
$env:PATH = (Join-Path $SdkDirectory 'upstream/emscripten') + ';' + $env:PATH
& $taskCmake -S $taskRepo -B $BuildDirectory -G Ninja "-DCMAKE_TOOLCHAIN_FILE=$taskToolchain" "-DCMAKE_MAKE_PROGRAM=$taskNinja" "-DPython3_EXECUTABLE=$taskPython" "-DACTUA_SDL_SOURCE=$SdlSource" -DACTUA_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE) { throw 'Browser configuration failed.' }
& $taskCmake --build $BuildDirectory --target actua96 --parallel $Jobs
if ($LASTEXITCODE) { throw 'Browser compilation failed.' }
& $taskPython (Join-Path $PSScriptRoot 'prepare_web.py') --output (Join-Path $BuildDirectory 'site')
if ($LASTEXITCODE) { throw 'Browser asset staging failed.' }
Write-Output ('Browser preview: ' + (Join-Path $BuildDirectory 'site/index.html'))
