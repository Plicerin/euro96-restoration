param([string]$ScratchDirectory = (Join-Path $PSScriptRoot '../../../work'))
$ErrorActionPreference='Stop'
$taskVerifier=Join-Path $PSScriptRoot 'verify_package.ps1'
$taskScratch=Join-Path $ScratchDirectory ('package-contracts-'+[guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $taskScratch | Out-Null
function New-PackageFixture([string]$Name) {
    $taskFixture=Join-Path $taskScratch $Name
    New-Item -ItemType Directory -Path (Join-Path $taskFixture 'data') | Out-Null
    $taskPaths=@('actua96.exe','SDL3.dll','SDL-LICENSE.txt','READ-ME.txt','Start-Actua96.cmd','verify_package.ps1')
    foreach($taskStem in @('EURO','EUROREND','FAP','FAPF')){foreach($taskExt in @('DAT','OFF')){$taskPaths+="data/$taskStem.$taskExt"}}
    $taskRecords=@(foreach($taskRelative in $taskPaths){
        $taskFile=Join-Path $taskFixture $taskRelative
        'fixture bytes only - never executed' | Set-Content -LiteralPath $taskFile -Encoding ascii
        [ordered]@{path=$taskRelative;bytes=(Get-Item -LiteralPath $taskFile).Length;sha256=(Get-FileHash -LiteralPath $taskFile).Hash}
    })
    [ordered]@{schema=1;version='0.37.1';missingOptionalMedia=@();files=$taskRecords} | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $taskFixture 'package.json') -Encoding UTF8
    return $taskFixture
}
$taskValid=New-PackageFixture 'valid'
& $taskVerifier -PackageDirectory $taskValid | Out-Null
Write-Output 'Valid fixture accepted (no executable run).'
foreach($taskCase in @('modified','missing','unlisted','required','duplicate','traversal','absolute')){
    $taskFixture=New-PackageFixture $taskCase
    $taskManifestPath=Join-Path $taskFixture 'package.json'
    $taskManifest=Get-Content -LiteralPath $taskManifestPath -Raw | ConvertFrom-Json
    $taskExpected=''
    switch($taskCase){
        'modified' { 'changed' | Add-Content -LiteralPath (Join-Path $taskFixture 'data/EURO.DAT');$taskExpected='hash/size mismatch' }
        'missing' { Remove-Item -LiteralPath (Join-Path $taskFixture 'data/EURO.DAT');$taskExpected='Missing package file' }
        'unlisted' { 'stale old asset' | Set-Content -LiteralPath (Join-Path $taskFixture 'stale.bin');$taskExpected='Unlisted package file' }
        'required' { $taskManifest.files=@($taskManifest.files | Where-Object {$_.path -ne 'SDL-LICENSE.txt'});$taskExpected='Required file absent from manifest' }
        'duplicate' { $taskManifest.files+= $taskManifest.files[0];$taskExpected='Duplicate or reserved package path' }
        'traversal' { $taskManifest.files[0].path='../escape.exe';$taskExpected='Invalid package path' }
        'absolute' { $taskManifest.files[0].path='C:/escape.exe';$taskExpected='Invalid package path' }
    }
    $taskManifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $taskManifestPath -Encoding UTF8
    $taskRejected=$false
    try { & $taskVerifier -PackageDirectory $taskFixture | Out-Null }
    catch { if($_.Exception.Message -notlike "*$taskExpected*"){throw}; $taskRejected=$true }
    if(-not $taskRejected){throw "Verifier accepted $taskCase fixture"}
    Write-Output "Rejected $taskCase fixture as expected."
}
Write-Output "Package contracts passed: 1 valid and 7 invalid fixtures. Scratch: $taskScratch"
