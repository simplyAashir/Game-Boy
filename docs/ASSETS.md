# Visual and software assets

Fantasy Quest follows the user's four supplied design references: forest/castle environments, ivy and antique-gold borders, parchment puzzles, emerald game pieces, and serif typography.

## Active visual assets

- `assets/art/environments.png`: generated 2 x 2 environment atlas containing the castle-lake menu, moonlit forest, candlelit library, and ruined courtyard. AI-generated specifically for this project using the supplied design references.
- `assets/art/fantasy-atlas.png`: generated ornamental frame, parchment, serpent, spellbook, wooden wanderer, and leaf medallion atlas. Shared frames use nine-slice rendering in SFML so borders retain their proportions.
- `assets/art/wanderer.png`: generated transparent carved-wood puppet, divided into six anatomy regions in SFML for progressive Hangman drawing. No static puzzle letters or gameplay outcomes are embedded in the environment artwork.
- **Cinzel Decorative Bold** headings: Google Fonts distribution, SIL Open Font License. [Source](https://github.com/google/fonts/tree/main/ofl/cinzeldecorative). License in `assets/fonts/Cinzel-OFL.txt`.
- **Lora** body and keyboard typography: Google Fonts distribution, SIL Open Font License. [Source](https://github.com/google/fonts/tree/main/ofl/lora). License in `assets/fonts/Lora-OFL.txt`.

All text, score values, menu controls, keyboard feedback, Snake pieces, input hitboxes, tile flips, fireflies, and game-state changes are rendered live with C++ / SFML. Unused earlier fonts have been removed.

## Audio

See [AUDIO_CREDITS.md](AUDIO_CREDITS.md) and the identical credits shipped beside the audio assets. The six music recordings and fantasy effects follow the user's sound plan.

## Software

SFML 2.6.1 uses its official GCC 13.1 x64 build and matching WinLibs compiler. Dependency licenses accompany the portable build. Python/FFmpeg were used only as development tools to prepare audio; the game executable uses C++17, SFML, and standard-library facilities.

The unused historical prototype and its Sprites directory have been removed.
