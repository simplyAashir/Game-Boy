# GameBox: Fantasy Quest — Audio Credits

All recordings below are used under the CC0 designation on their source pages. The files are included locally; the game never downloads audio while running.

| In-game file | Original recording | Creator | Source |
|---|---|---|---|
| `music/menu.ogg` | Loop: The Old Tower Inn | RandomMind | [Source](https://opengameart.org/content/medieval-the-old-tower-inn) |
| `music/snake.ogg` | Forest Ambience | TinyWorlds | [Source](https://opengameart.org/content/forest-ambience) |
| `music/wordle.ogg` | Town Theme RPG | cynicmusic / pixelsphere.org | [Source](https://opengameart.org/content/town-theme-rpg) |
| `music/hangman.ogg` | Rising Moon | RandomMind | [Source](https://opengameart.org/content/fantasy-rising-moon) |
| `music/victory.ogg` | Medieval: Victory Theme | RandomMind | [Source](https://opengameart.org/content/medieval-victory-theme) |
| `music/defeat.ogg` | Medieval: Defeat Theme | RandomMind | [Source](https://opengameart.org/content/medieval-defeat-theme) |

Music was converted to OGG Vorbis and level matched (scene tracks target -23 LUFS, forest ambience -27 LUFS, outcome excerpts -20 LUFS, all with a -3 dBTP target). The menu uses the supplied loop recording. Other scene music loops by returning to the beginning; recording boundaries are not all guaranteed seamless. Music transitions use a 0.45-second crossfade. Victory/defeat recordings are streamed as short, fading excerpts rather than played in full.

## Effects

- [80 CC0 RPG SFX](https://opengameart.org/content/80-cc0-rpg-sfx), **rubberduck**: gems, coins, books, spells, wood and stone.
- [Fantasy Sound Effects / Tinysized SFX](https://opengameart.org/content/fantasy-sound-effects-tinysized-sfx), **Vehicle**: page rustles, wooden taps, creaks and soft whooshes.
- [Interface Sounds](https://kenney.nl/assets/interface-sounds), **Kenney**: short UI clicks and bell cues.

`effects/manifest.json` lists all 27 prepared cue files, their original filenames, source packs, and trim durations. Effects were trimmed, converted to OGG Vorbis, normalized with a -23 LUFS target / -3 dBTP ceiling, and given short fade-outs. Very short clips may not yield stable integrated loudness measurements, so the normalization target should not be interpreted as an exact perceived loudness guarantee. Three distinct wood/creak recordings are varied for wrong Hangman guesses.

## Mixing and playback

The shared C++ SoundSystem owns 27 buffers, 12 reusable sound voices, two music streams, and one outcome stream. Settings provide master, music, effects, and mute. Master scales both categories. Pausing ducks the scene music; sleep pauses it. Wordle tile cues occur at staggered reveal midpoints; ordinary Snake movement is silent.

Audio decoding, track opening, scene switching and volume calculations are covered by the application checks. Subjective listening and playback on the presentation computer are still recommended.
