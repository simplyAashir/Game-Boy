param()
$ErrorActionPreference='Stop'
$projectRoot=Split-Path $PSScriptRoot -Parent
Set-Location -LiteralPath $projectRoot
New-Item -ItemType Directory -Force .tools,assets/fonts,assets/audio | Out-Null
$compilerRelease='https://github.com/brechtsanders/winlibs_mingw/releases/download/13.1.0-16.0.5-11.0.0-msvcrt-r5/'
$compilerArchive='winlibs-x86_64-posix-seh-gcc-13.1.0-mingw-w64msvcrt-11.0.0-r5'
if(!(Test-Path '.tools/mingw64/bin/g++.exe')) {
    $sevenZip=Get-Command 7z -ErrorAction SilentlyContinue
    $sevenZipPath=if($sevenZip){$sevenZip.Source}else{'C:\Program Files\7-Zip\7z.exe'}
    if(Test-Path $sevenZipPath) {
        Invoke-WebRequest ($compilerRelease+$compilerArchive+'.7z') -OutFile .tools/compiler.7z
        & $sevenZipPath x .tools/compiler.7z '-o.tools' -y
        if($LASTEXITCODE -ne 0){throw 'Compiler extraction failed.'}
    } else {
        Invoke-WebRequest ($compilerRelease+$compilerArchive+'.zip') -OutFile .tools/compiler.zip
        Expand-Archive .tools/compiler.zip .tools -Force
    }
}
if(!(Test-Path '.tools/SFML-2.6.1/include')) {
    Invoke-WebRequest 'https://www.sfml-dev.org/files/SFML-2.6.1-windows-gcc-13.1.0-mingw-64-bit.zip' -OutFile .tools/sfml.zip
    Expand-Archive .tools/sfml.zip .tools -Force
}
Write-Host 'Portable GCC 13.1 and matching SFML 2.6.1 are ready. Run scripts/build.ps1 -Run.'
