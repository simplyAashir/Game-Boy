param()
$ErrorActionPreference='Stop'
$projectRoot=Split-Path $PSScriptRoot -Parent
Set-Location -LiteralPath $projectRoot
if(!(Test-Path build/FantasyQuest.exe)){throw 'Build the game first.'}
$destination=Join-Path $projectRoot 'dist/FantasyQuest'
New-Item -ItemType Directory -Force $destination,"$destination/licenses" | Out-Null
function Copy-ChangedFile([string]$source,[string]$target) {
    $sourceItem=Get-Item -LiteralPath $source
    if(Test-Path -LiteralPath $target) {
        if((Get-FileHash -LiteralPath $sourceItem.FullName).Hash -eq (Get-FileHash -LiteralPath $target).Hash){return}
    }
    $parent=Split-Path -Parent $target
    New-Item -ItemType Directory -Force $parent | Out-Null
    try {Copy-Item -LiteralPath $sourceItem.FullName -Destination $target -Force -ErrorAction Stop}
    catch {
        if([IO.Path]::GetExtension($target) -ne '.exe'){throw}
        # A running Windows executable can be renamed; keep it recoverable until restart.
        $backup=Join-Path $parent ('FantasyQuest.previous-'+[guid]::NewGuid().ToString('N')+'.exe')
        Move-Item -LiteralPath $target -Destination $backup -ErrorAction Stop
        Copy-Item -LiteralPath $sourceItem.FullName -Destination $target -Force -ErrorAction Stop
        Write-Host 'Updated the executable. Restart the open game to use this version.'
    }
}
Copy-ChangedFile 'build/FantasyQuest.exe' "$destination/FantasyQuest.exe"
Get-ChildItem build -Filter '*.dll' -File | ForEach-Object {Copy-ChangedFile $_.FullName (Join-Path $destination $_.Name)}
$assetRoot=(Resolve-Path assets).Path
Get-ChildItem -LiteralPath $assetRoot -Recurse -File | ForEach-Object {
    $relative=$_.FullName.Substring($assetRoot.Length+1)
    Copy-ChangedFile $_.FullName (Join-Path "$destination/assets" $relative)
}
Copy-ChangedFile 'docs/ASSETS.md' "$destination/licenses/ASSETS.md"
Copy-ChangedFile 'docs/AUDIO_CREDITS.md' "$destination/licenses/AUDIO_CREDITS.md"
Copy-ChangedFile '.tools/SFML-2.6.1/license.md' "$destination/licenses/SFML.md"
$toolchainLicense=Get-ChildItem .tools/mingw64 -File -Filter '*license*' -ErrorAction SilentlyContinue
foreach($license in $toolchainLicense){Copy-ChangedFile $license.FullName "$destination/licenses/$($license.Name)"}
@(
'GAMEBOX: FANTASY QUEST',
'',
'Double-click FantasyQuest.exe. Your existing data/ saves are preserved when updating.',
'Use Help for controls. Settings > Test Sound plays a short chime.',
'Wordle: type five letters and press Enter. Every complete guess gets colour feedback.',
'Fonts and sounds have licenses alongside their assets; see licenses/ASSETS.md.',
'Copyright 2026 Aashir Sheraz. All rights reserved.'
) | Set-Content "$destination/START-HERE.txt"
Write-Host "Portable game prepared at $destination"
