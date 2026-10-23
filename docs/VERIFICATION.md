# Verification record - current build

## Results

- **1,513 / 1,513 model checks passed.** This includes 1,000 food-placement samples and repeated category-bank checks, not 1,513 separate features. Tests cover Snake movement/growth/collisions/power-ups, duplicate-aware Wordle colours, arbitrary complete guesses, all category answer lengths, category/save round trips, legacy saves, Hangman lives/repeats/hints, profiles and persistence.
- **133 / 133 application-flow checks passed.** Actual SFML event handlers and rendering verify keyboard/mouse input, category menus, category changes, Enter submission, tile timing, all three tile-colour pixels, immediate score saving, pause/focus/resize/profile controls, and animation timing.
- Snake checks cross a previous best during play, verify immediate persisted high score and a single celebration, trigger collision, reject an early restart, measure camera displacement, inspect an actual black pixel during blackout, wait one second, and verify one recorded result.
- Hangman checks correct sparkle/bounce and wrong flash/sway/lost-life reactions, repeat protection, and delaying the final result until the last reaction finishes.
- Existing Windows audio test covers **41 playback checks**: four music scenes, all 27 effects, two outcomes, mute/unmute, sleep/wake, and partial-resource recovery. It reads the app's own output meter plus 48 kHz stereo output loopback. Results are in build/audio-playback-results.txt. The test only reads OS volume settings.
- Rendered and inspected Wordle's green/yellow/black row, category dropdown, Snake impact/blackout/high-score/result frames, and both Hangman reactions. Evidence images are under docs/screenshots.
- Builds use C++17, matching GCC 13.1/SFML 2.6.1 and Wall/Wextra/Wpedantic. The game uses C++/SFML; the Windows COM playback probe is a separate test executable.

## Repeat

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1 -Test
powershell -ExecutionPolicy Bypass -File scripts/test-audio.ps1
```

The audio command plays sounds. Its deliberately missing-cue test prints one expected missing-file message and then verifies continued music/fallback output. Tests use build/test-data and build/ui-test-data rather than normal profiles.

## Saves and cleanup

Current development and portable-game data folders are preserved. Saves from removed old packages were copied to data/archived-saves. Wordle format 2 saves categories and reads original snapshots. The original Hangman category indices and words remain stable. Animation timers are intentionally transient.

See CLEANUP.md for removed files. Required compiler/SFML/formatter tools remain local. Old prototype source is also recoverable from Git history.

## Limits

Automated event, timing, pixel and output checks are evidence for these behaviours; a complete manual play-through and subjective speaker listening are not claimed. Other operating systems and the optional CMake route have not been exercised here.
