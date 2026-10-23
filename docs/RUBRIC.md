# Rubric mapping

This is an implementation map, not a guaranteed grade. The instructor decides marks and interpretation.

| Rubric area | Implementation/evidence |
|---|---|
| Class design and relationship documentation | `ARCHITECTURE.md` class diagram, ownership notes, invariants, and viva prompts |
| Base Game, inheritance, virtual and abstract methods | Abstract `Game`; final SnakeGame, WordleGame, HangmanGame overrides |
| Player hierarchy | Abstract User, persistent Player, nonpersistent GuestPlayer model; three persistent slots in UI |
| Polymorphic updates/rendering | GameCollection returns Game&; application calls virtual input/update/draw; separate virtual save/load |
| Screen composition | GameBoy owns ScreenDisplay with RenderTexture; fixed logical resolution, letterboxing, screen state |
| Input | InputSystem maps SFML key/text events; single-turn snake guard, duplicate Hangman-letter guard, mouse mapping |
| Sound system | Owned SFML buffers, twelve shared playback voices, two streamed scene channels with crossfades and outcome stream, master/music/effects and mute controls |
| Menu system | MenuSystem page state, fades, library, profile/settings/guide/patch pages, sleep/wake |
| Snake components | Bounded contiguous body segments follow predecessor positions; free-cell food generation, Grid, Collision |
| Wordle components | LetterGrid, WordDictionary, KeyboardDisplay with duplicate-aware feedback |
| Hangman components | WordCategories, LetterTracker, owned HangmanFigure with part tracking and life limit, progressively drawn wooden wanderer |
| Game collection | Three non-owning Game pointers to pre-existing GameBoy-owned cartridges |
| Resource sharing | Shared font Resources/Painter and sound pool; RAII for SFML objects, no manual heap ownership |
| Lifecycle | Single construction, reset, suspend/resume, normal-exit saving; per-profile snapshots |
| Profile management | Three create/load slots, editable sanitized names, settings, scores, achievements |
| Scores | Each game's current score and per-profile best/round/win records |
| Achievements | Five persistent patches, conditions evaluated on first completion of a round |
| Persistence | Versioned profile/game files, validation, temporary writes, retained backups, automatic saves |
| Snake movement/controls | Fixed-step timing, position interpolation, arrow/WASD controls, no 180-degree reversal |
| Snake collisions | Board bounds and self-collision including vacating-tail case; model checks |
| Snake growth/difficulty/power-ups | Bounded growth, food score, increasing pace, three starting paces, slow-motion pickup with expiry |
| Wordle validation | Five-letter alphabetic input; every full guess receives feedback, following the requested rules |
| Wordle feedback/keyboard | Exact matches first, then count-limited present matches; key certainty cannot downgrade |
| Wordle selection/end conditions | Random answer from selected category, six attempts, win/loss and category persistence |
| Hangman categories | Creatures / Objects / Nature / Places / Footballers / Animals, eight clue-paired words each |
| Hangman drawing/input/lives | Six progressively added wooden wanderer components, letter tracking, six-error loss, one hint |
| Graphics/UI | Fantasy environments, gold/ivy frames, parchment, Cinzel/Lora typography, fireflies and staggered tile reveals |
| Code quality | Formatted C++17, private game state, const accessors, explicit ownership, validated saved input, tests |
| Additional sound | Six credited fantasy recordings, 27 event cue files, master/music/effects controls and mute |
| Extended functionality | Pause/resume, settings, guide, achievements, hidden Konami-code library variation |

## Interpretation and limits to explain during grading

- **No linked list:** the rubric's phrase “snake body segments linked correctly” is implemented as correct predecessor-following relationships in a fixed array. There are no linked nodes or next pointers. This follows the user's explicit restriction.
- **Arrays are the bounded storage assumption.** No STL collection containers are used. If the instructor excludes elementary arrays or strings, that is a different restriction requiring clarification.
- **Guess policy:** Per the latest requested rules, Wordle evaluates any five alphabetic letters. The answer bank supplies category words; it does not reject guesses.
- **Attempts:** three new Wordle rounds per 15 minutes, each allowing six valid guesses. Resuming does not consume a new round. This interpretation is documented in the guide.
- **Persistence scope:** local profiles, settings, scores, achievements, cooldown, and all cartridge round states are saved. Interpolation phase and page-fade animation are deliberately transient.
- **Loading/unloading:** cartridges are retained member objects and suspended rather than dynamically allocated/unloaded. Shared fonts/audio are loaded once.
- **GuestPlayer:** included and tested as the second User-derived type. The product's profile screen exposes persistent slots; it does not provide a separate guest-session button.
- **Figure styling:** Hangman uses a six-piece wooden wanderer rather than stock stick-figure artwork. The same progressive-mistake mechanic applies.
- **Power-ups:** one functional pickup type (slow motion plus bonus points). No unsupported claim of multiple power-up classes.
- **Offline integrity:** saves and cooldowns are appropriate for a semester project, not protected against deliberate file editing or wall-clock changes.
- **Runtime verification:** consult `VERIFICATION.md`; do not claim that an automated rule check proves flawless gameplay on every machine.
