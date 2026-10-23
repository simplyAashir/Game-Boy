param([switch]$Run,[switch]$Test,[switch]$DebugBuild)
$ErrorActionPreference='Stop'
$projectRoot=Split-Path $PSScriptRoot -Parent
Set-Location -LiteralPath $projectRoot
$compiler=Join-Path $projectRoot '.tools/mingw64/bin/g++.exe'
$sfml=Join-Path $projectRoot '.tools/SFML-2.6.1'
if(!(Test-Path $compiler) -or !(Test-Path $sfml)) { throw 'Run scripts/setup.ps1 first to download the matching compiler and SFML.' }
New-Item -ItemType Directory -Force build | Out-Null
$env:PATH="$(Join-Path $projectRoot '.tools/mingw64/bin');$env:PATH"
$flags=@('-std=c++17','-O2','-Wall','-Wextra','-Wpedantic','-I',"$sfml/include",'-I','src')
if($DebugBuild) { $flags = $flags | Where-Object { $_ -ne '-O2' }; $flags += @('-O0','-g') }
$gameSources=@('src/Main.cpp','src/Model.cpp','src/Frontend.cpp','src/GameViews.cpp','src/Application.cpp')
& $compiler @flags @gameSources '-L' "$sfml/lib" '-lsfml-graphics' '-lsfml-window' '-lsfml-audio' '-lsfml-system' '-o' 'build/FantasyQuest.exe'
if($LASTEXITCODE -ne 0) { throw 'Build failed.' }
foreach($dll in @('sfml-graphics-2.dll','sfml-window-2.dll','sfml-audio-2.dll','sfml-system-2.dll','openal32.dll')) {
    Copy-Item "$sfml/bin/$dll" build -Force
}
foreach($dll in @('libgcc_s_seh-1.dll','libstdc++-6.dll','libwinpthread-1.dll')) { Copy-Item ".tools/mingw64/bin/$dll" build -Force }
Write-Host 'Built build/FantasyQuest.exe'
if($Test) {
    & $compiler @flags 'tests/ModelTests.cpp' 'tests/RenderStubs.cpp' 'src/Model.cpp' '-o' 'build/ModelTests.exe'
    if($LASTEXITCODE -ne 0) { throw 'Test build failed.' }
    & ./build/ModelTests.exe
    if($LASTEXITCODE -ne 0) { throw 'Tests failed.' }
    & $compiler @flags '-DPOCKET_UI_TEST' @gameSources 'tests/UIFlowTests.cpp' '-L' "$sfml/lib" '-lsfml-graphics' '-lsfml-window' '-lsfml-audio' '-lsfml-system' '-o' 'build/UIFlowTests.exe'
    if($LASTEXITCODE -ne 0) { throw 'UI test build failed.' }
    & ./build/UIFlowTests.exe --preview selftest
    Get-Content build/application-test-results.txt
    if($LASTEXITCODE -ne 0) { throw 'UI flow tests failed.' }
}
if($Run) { & ./build/FantasyQuest.exe }
