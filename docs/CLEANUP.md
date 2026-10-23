# Folder cleanup

Removed the unused Visual Studio prototype (OOP_GAMEBOY, old solution, root include/lib and Sprites), old Pocket package/executables, unused fonts and four superseded sounds, extracted source packs, downloaded archives, audio-preparation runtimes, and temporary editing scripts.

Preserved current C++ sources, tests, SFML assets/licenses, documentation, VS Code settings, compiler/SFML/formatter, current build/package, Git history and all active data folders.

Before removing the old live-QA and Pocket packages, their save files were copied to data/archived-saves/live-qa and data/archived-saves/old-pocket.

Removed 629 files; reclaimed 379.3 MiB. Required build tools remain in .tools.

Additional stale preview files and unused runtime DLLs are removed after verification. A backup executable still in use by an open game is retained until that game exits.

Final cleanup also removed 110 generated previews, scratch/save fixtures and helper files (57.8 MiB), plus unused debug/network DLLs and stale packaged fonts/sounds. Selected current screenshots remain under docs/screenshots.

One backup executable is held open by the running game and remains until it exits. The newly packaged FantasyQuest.exe is ready for the next launch.
