# Fantasy Quest design implementation

The supplied design guide is the visual reference for every screen. Artwork is separated from live gameplay: illustrated environments contain no baked-in letters, scores, buttons or puzzle results.

## Shared style

- Four painterly environments: golden castle lake, moonlit forest, candlelit library, and ruined courtyard.
- Antique-gold and ivy borders with emerald leather centers; reusable blank parchment for readable content.
- Cinzel Decorative Bold for fantasy titles and Lora for body copy, controls and letters.
- Forest green, moss, antique gold, parchment beige, warm brown and restrained mystical blue.
- Nine-slice panels preserve ornament proportions at different panel dimensions.
- Real SFML buttons, key tiles and hit regions remain separate from the art.
- Quiet fireflies, short page fades and staggered Wordle flips provide movement without obscuring the puzzle.

## Quest identities

Snake uses the forest, green jewel-like segments, red glowing fruit and blue timeweaver crystals. Its direction-facing eyes and tongue follow the logical heading; movement between cells is interpolated.

Wordle uses a library, illuminated spellbook, parchment board and tactile green/yellow/black keys. A submitted row flips tile by tile; input pauses briefly during this feedback. Scores save immediately even when the player leaves before the animation finishes.

Hangman uses the courtyard, torn parchment, a wooden gallows and a textured transparent puppet. Six separately drawn anatomy regions track mistakes. Letter cells and lives update from game state; scenery never substitutes for gameplay.

Main menu, profiles, achievements, score chronicle, settings, guide, sleep and outcome overlays reuse the same assets and typography. Settings offer three audio sliders and mute, and achievements use leaf medallions.

## Boundaries

The illustrations implement the mood and materials of the guide. The code does not show decorative controls that imply unavailable settings, such as a nonfunctional language selector, or a fake Snake lives counter. Fullscreen/language/brightness controls from the concept sheet are not claimed as implemented.

## Gameplay feedback

Snake's one-second impact sequence darkens the whole game area, pulses the crash title and applies a decaying camera shake before the result. A high-score ribbon and golden particles play over ongoing movement. Hangman uses green letter bounces/sparkles and red mistake flashes, figure sway and a lost-heart ring. Both word games expose a compact category dropdown with keyboard navigation. Wordle evaluates every complete guess and reveals all three colours.
