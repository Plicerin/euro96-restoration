param([string]$BuildDirectory = 'D:\Codex\euro96-web\build', [int]$Port = 8786, [switch]$NoOpen)
$ErrorActionPreference = 'Stop'
$taskSite = Join-Path $BuildDirectory 'site'
foreach ($taskFile in @('index.html','euro96.js','euro96.wasm','assets.json')) {
    if (-not (Test-Path -LiteralPath (Join-Path $taskSite $taskFile))) { throw 'Browser build is incomplete. Run tools/build_web.ps1 first.' }
}
$taskUrl = "http://127.0.0.1:$Port/"
$taskRunning = $false
try {
    $taskRemote = Invoke-RestMethod -Uri ($taskUrl + 'assets.json') -TimeoutSec 2
    if ($taskRemote.preview -ne 'browser-proof') { throw 'A different server is using this port.' }
    $taskRunning = $true
} catch {
    if ($_.Exception.Message -eq 'A different server is using this port.') { throw }
}
if (-not $taskRunning) {
    $taskPython = (Get-Command python -ErrorAction Stop).Source
    $taskServer = Start-Process -FilePath $taskPython -WindowStyle Hidden -PassThru -ArgumentList @('-m','http.server',"$Port",'--bind','127.0.0.1','--directory',('"'+$taskSite+'"')) -RedirectStandardOutput (Join-Path $BuildDirectory 'server-output.txt') -RedirectStandardError (Join-Path $BuildDirectory 'server-errors.txt')
    $taskServer.Id | Set-Content -LiteralPath (Join-Path $BuildDirectory 'server.pid')
    for ($taskAttempt=0; $taskAttempt -lt 30; ++$taskAttempt) {
        Start-Sleep -Milliseconds 100
        try { $taskRemote = Invoke-RestMethod -Uri ($taskUrl + 'assets.json') -TimeoutSec 1; if($taskRemote.preview -eq 'browser-proof') {$taskRunning=$true; break} } catch {}
        if ($taskServer.HasExited) { throw 'Browser server did not start. Check server-errors.txt or choose another port.' }
    }
    if (-not $taskRunning) { throw 'Browser server did not respond.' }
}
if (-not $NoOpen) { Start-Process $taskUrl }
Write-Output $taskUrl
