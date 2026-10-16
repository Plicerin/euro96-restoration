param([string]$SdkDirectory='D:\Codex\euro96-web\emsdk', [string]$BuildDirectory='D:\Codex\euro96-web\build')
$ErrorActionPreference='Stop'
$taskRepo=Split-Path -Parent $PSScriptRoot
$env:EM_CONFIG=Join-Path $SdkDirectory '.emscripten'
$taskNode=(Get-ChildItem -LiteralPath (Join-Path $SdkDirectory 'node') -Filter node.exe -Recurse | Select-Object -First 1).FullName
if (-not $taskNode) { throw 'Emscripten Node runtime is missing.' }
$taskCases=@('ball','selection','regulation','penalty_control')
$taskTargets=@($taskCases | ForEach-Object { 'web_'+$_+'_tests' })
& cmake --build $BuildDirectory --target @taskTargets --parallel 4
if ($LASTEXITCODE) { throw 'WebAssembly test compilation failed.' }
foreach($taskCase in $taskCases) {
    $taskLog=Join-Path $BuildDirectory ('wasm-'+$taskCase+'-tests.txt')
    & $taskNode (Join-Path $BuildDirectory ('web_'+$taskCase+'_tests.js')) (Join-Path $taskRepo 'SOURCE') *> $taskLog
    $taskExit=$LASTEXITCODE
    Get-Content -LiteralPath $taskLog -Tail 4
    if ($taskExit) { throw "$taskCase failed; see $taskLog" }
}
Write-Output 'All four WebAssembly gameplay contract suites passed.'
