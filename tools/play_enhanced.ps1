param(
    [ValidateSet('720','1080','classic')][string]$Picture = '720',
    [string]$Executable = ''
)
$ErrorActionPreference = 'Stop'
$taskRepo = Split-Path -Parent $PSScriptRoot
if (-not $Executable) { $Executable = Join-Path (Split-Path -Parent $taskRepo) 'Euro96-Enhanced/actua96.exe' }
if (-not (Test-Path -LiteralPath $Executable)) { throw 'Enhanced preview is not built. See docs/ENHANCED-PRESENTATION.md.' }
$taskArguments = @('--data', ('"' + (Join-Path $taskRepo 'SOURCE') + '"'), '--audio-data', ('"' + (Join-Path $taskRepo 'local-data') + '"'), '--front-end')
if ($Picture -ne 'classic') { $taskArguments += '--wide' + $Picture }
Start-Process -FilePath $Executable -ArgumentList $taskArguments -WorkingDirectory $taskRepo
