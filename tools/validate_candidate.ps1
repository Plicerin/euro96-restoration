param([string]$Root = $PSScriptRoot)
$ErrorActionPreference='Stop'
$ProgressPreference='SilentlyContinue'
$taskRoot=(Resolve-Path -LiteralPath $Root).Path
$taskPackage=Join-Path $taskRoot 'package'
$taskManifest=Get-Content -LiteralPath (Join-Path $taskPackage 'package.json') -Raw | ConvertFrom-Json
$taskResults=@()
function Get-CandidateUserState {
    $taskFolder=Join-Path $env:APPDATA 'Actua96 Restoration/Actua96'
    foreach($taskName in @('tournament.sav','tournament.sav.bak','tournament.prefs','tournament.prefs.bak')) {
        $taskPath=Join-Path $taskFolder $taskName
        [ordered]@{name=$taskName;sha256=$(if(Test-Path -LiteralPath $taskPath -PathType Leaf){(Get-FileHash -LiteralPath $taskPath).Hash}else{'ABSENT'})}
    }
}
function Invoke-CandidateCheck([string]$Name,[string]$Executable,[string[]]$Arguments,[string]$Marker) {
    $taskOut=Join-Path $taskRoot ($Name+'.out');$taskErr=Join-Path $taskRoot ($Name+'.err')
    $taskClock=[Diagnostics.Stopwatch]::StartNew()
    $taskProcess=Start-Process -FilePath (Join-Path $taskRoot $Executable) -ArgumentList $Arguments -WorkingDirectory $taskRoot -WindowStyle Hidden -PassThru -RedirectStandardOutput $taskOut -RedirectStandardError $taskErr
    $taskHandle=$taskProcess.Handle
    $taskDone=$taskProcess.WaitForExit(180000)
    if(-not $taskDone){$taskProcess.Kill()}else{$taskProcess.WaitForExit()}
    $taskPassed=$taskDone -and $taskProcess.ExitCode -eq 0 -and (Get-Content -LiteralPath $taskOut -Raw) -match $Marker
    $script:taskResults += [ordered]@{name=$Name;passed=$taskPassed;exit=$taskProcess.ExitCode;seconds=[math]::Round($taskClock.Elapsed.TotalSeconds,2)}
    Write-Output "$Name : $taskPassed"
    if(-not $taskPassed){throw "Candidate check failed: $Name"}
}
$taskBefore=@(Get-CandidateUserState);$taskFailure=$null
try {
    & (Join-Path $taskPackage 'verify_package.ps1') -PackageDirectory $taskPackage -Smoke *> (Join-Path $taskRoot 'package-checks.log')
    $taskLog=Get-Content -LiteralPath (Join-Path $taskRoot 'package-checks.log') -Raw
    if(([regex]::Matches($taskLog,'Isolated package passed ')).Count -ne 9){throw 'Expected all nine isolated package checks'}
    $taskResults += [ordered]@{name='nine_isolated_package_checks';passed=$true}
    $env:SDL_VIDEODRIVER='dummy';$env:SDL_AUDIODRIVER='dummy';$env:SDL_RENDER_DRIVER='software'
    $taskData='"'+(Join-Path $taskPackage 'data')+'"';$taskMedia='"'+(Join-Path $taskPackage 'audio')+'"'
    Invoke-CandidateCheck 'full_tournament' 'tournament_session_tests.exe' @($taskData,$taskMedia,('"'+(Join-Path $taskRoot 'session')+'"')) 'Full frontend session: 31 matches'
    Invoke-CandidateCheck 'match_matrix' 'release_audit_tests.exe' @($taskData) 'Release matrix: 32 completed fixtures'
    Invoke-CandidateCheck 'save_recovery' 'save_recovery_tests.exe' @($taskData,$taskMedia,('"'+(Join-Path $taskRoot 'recovery')+'"')) 'Damaged/missing primary recovery'
    Invoke-CandidateCheck 'retail_names' 'euro96_media_tests.exe' @($taskData,$taskMedia,('"'+(Join-Path $taskRoot 'names')+'"')) '1920 complete name PCM/callback cases'
    Invoke-CandidateCheck 'retail_periods' 'euro_period_tests.exe' @($taskData,$taskMedia,('"'+(Join-Path $taskRoot 'periods')+'"')) 'Retail periods:'
}catch{$taskFailure=$_.Exception.Message;Write-Output $taskFailure}
finally {
    $taskAfter=@(Get-CandidateUserState)
    $taskUnchanged=($taskBefore|ConvertTo-Json -Compress) -eq ($taskAfter|ConvertTo-Json -Compress)
    $taskOs=Get-CimInstance Win32_OperatingSystem
    [ordered]@{host=$env:COMPUTERNAME;os=$taskOs.Caption;build=$taskOs.BuildNumber;version=$taskManifest.version;exeSha256=(Get-FileHash -LiteralPath (Join-Path $taskPackage 'actua96.exe')).Hash;results=$taskResults;error=$taskFailure;userStateUnchanged=$taskUnchanged;userStateBefore=$taskBefore;userStateAfter=$taskAfter;scope='Automated hidden checks with dummy video/audio. Physical controls, sound, device switching and display behavior remain separate.'} | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath (Join-Path $taskRoot 'report.json') -Encoding UTF8
}
if($taskFailure -or -not $taskUnchanged){exit 1}
