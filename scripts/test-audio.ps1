param()
$ErrorActionPreference='Stop'
$projectRoot=Split-Path $PSScriptRoot -Parent
Set-Location -LiteralPath $projectRoot
$compiler=Join-Path $projectRoot '.tools/mingw64/bin/g++.exe'
$sfml=Join-Path $projectRoot '.tools/SFML-2.6.1'
$env:PATH="$(Join-Path $projectRoot '.tools/mingw64/bin');$env:PATH"
New-Item -ItemType Directory -Force build | Out-Null
$flags=@('-std=c++17','-O2','-Wall','-Wextra','-Wpedantic','-I',"$sfml/include",'-I','src')
& $compiler @flags 'tests/AudioPlaybackTests.cpp' 'src/Frontend.cpp' 'src/Model.cpp' 'tests/RenderStubs.cpp' '-L' "$sfml/lib" '-lsfml-graphics' '-lsfml-window' '-lsfml-audio' '-lsfml-system' '-lole32' '-luuid' '-o' 'build/AudioPlaybackTests.exe'
if($LASTEXITCODE -ne 0){throw 'Audio test build failed.'}
foreach($dll in @('sfml-graphics-2.dll','sfml-window-2.dll','sfml-audio-2.dll','sfml-system-2.dll','openal32.dll')) {
    Copy-Item "$sfml/bin/$dll" build -Force
}
foreach($dll in @('libgcc_s_seh-1.dll','libstdc++-6.dll','libwinpthread-1.dll')) { Copy-Item ".tools/mingw64/bin/$dll" build -Force }
Write-Host 'Testing music and all effects through the Windows output meter and loopback. This plays sounds.'
& ./build/AudioPlaybackTests.exe
$testExit=$LASTEXITCODE
Get-Content build/audio-playback-results.txt
if($testExit -ne 0){throw "Audio playback checks failed (exit $testExit)."}
