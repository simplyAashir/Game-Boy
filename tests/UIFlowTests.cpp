#include "Frontend.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
namespace pocket {
int GameBoy::testFlows() {
    int checks = 0, failures = 0;
    std::ofstream report("build/application-test-results.txt");
    auto expect = [&](bool ok, const char *label) {
        ++checks;
        if (!ok) {
            ++failures;
            report << "UI FAIL: " << label << '\n';
        }
    };
    using Page = MenuSystem::Page;
    store = ProfileStore("build/ui-test-data");
    player = Player{};
    settings = Settings{};
    for (int i = 0; i < 3; ++i) {
        initialized[i] = recorded[i] = false;
        games.at(i).reset();
    }
    auto key = [&](sf::Keyboard::Key code) {
        sf::Event e{};
        e.type = sf::Event::KeyPressed;
        e.key.code = code;
        event(e);
    };
    auto letter = [&](char c) {
        sf::Event e{};
        e.type = sf::Event::TextEntered;
        e.text.unicode = static_cast<unsigned char>(c);
        event(e);
    };
    auto pointer = [&](sf::Event::EventType type, float x, float y) {
        auto size = window.getSize();
        float scale = std::min(size.x / 1280.f, size.y / 800.f);
        int px = static_cast<int>((size.x - 1280 * scale) / 2 + x * scale),
            py = static_cast<int>((size.y - 800 * scale) / 2 + y * scale);
        sf::Event e{};
        e.type = type;
        if (type == sf::Event::MouseMoved) {
            e.mouseMove.x = px;
            e.mouseMove.y = py;
        } else {
            e.mouseButton.button = sf::Mouse::Left;
            e.mouseButton.x = px;
            e.mouseButton.y = py;
        }
        event(e);
    };
    auto render = [&](const std::string &name) {
        Painter p(screen.canvas(), resources, 1.5f, {-1, -1});
        draw(p);
        screen.present(window, false);
        return screen.canvas().getTexture().copyToImage().saveToFile("build/flow-" + name + ".png");
    };
    expect(menu.page == Page::Home, "application starts at the fantasy main menu");
    pointer(sf::Event::MouseMoved, 250, 325);
    expect(menu.selected == 1, "hover updates the same home selection used by arrows");
    key(sf::Keyboard::Up);
    expect(menu.selected == 0, "arrow navigation continues from hovered selection");

    key(sf::Keyboard::Down);
    key(sf::Keyboard::Down);
    key(sf::Keyboard::Enter);
    expect(menu.page == Page::Scores, "main-menu keyboard navigation opens scores");
    expect(render("scores"), "score chronicle renders");
    click({200, 60});
    expect(menu.page == Page::Home, "brand returns to main menu");
    key(sf::Keyboard::Enter);
    expect(menu.page == Page::Library, "play opens quest selection");
    pointer(sf::Event::MouseMoved, 1010, 420);
    expect(menu.selected == 2, "hover highlights the full Hangman card");
    key(sf::Keyboard::Left);
    expect(menu.selected == 1, "card arrow selection follows pointer selection");

    key(sf::Keyboard::Num1);
    expect(active == 0 && menu.page == Page::Playing, "number shortcut launches snake");
    expect(!snake.running(), "snake ready state");
    key(sf::Keyboard::W);
    expect(snake.running(), "WASD mapping starts snake");
    snake.update(.2f);
    key(sf::Keyboard::P);
    expect(menu.paused, "P pauses snake");
    Cell head = snake.body().segment(0).cell;
    dispatch(1003);
    expect(snake.body().segment(0).cell == head, "paused input suppressed");
    key(sf::Keyboard::Enter);
    expect(!menu.paused, "enter resumes");
    sf::Event focus{};
    focus.type = sf::Event::LostFocus;
    event(focus);
    expect(menu.paused, "focus loss pauses");
    key(sf::Keyboard::Escape);
    expect(menu.page == Page::Library, "escape from pause returns library");
    click({600, 420});
    expect(active == 1 && menu.page == Page::Playing && player.wordleRounds == 1,
           "card click launches wordle and consumes one round");
    key(sf::Keyboard::Enter);
    expect(wordle.board().used == 0 && wordle.notice() == "Enter five letters first.",
           "empty Enter shows clear feedback and preserves attempts");
    expect(render("wordle-error"), "incomplete-word message renders without hiding keyboard");
    for (char c : std::string("SHSKS"))
        letter(c);
    std::string firstAnswer = wordle.solution();
    key(sf::Keyboard::Enter);
    expect(wordle.board().used == 1 && wordle.entry().empty(),
           "unknown five-letter guess is evaluated instead of rejected");
    int expected[5];
    LetterGrid::evaluate(firstAnswer, "SHSKS", expected);
    bool feedbackMatches = true;
    for (int i = 0; i < 5; ++i)
        feedbackMatches &= wordle.board().colors[0][i] == expected[i];
    expect(feedbackMatches, "arbitrary guess receives exact present and absent tile feedback");
    for (int i = 0; i < 12; ++i)
        update(.1f);
    chooseCategory(0);
    for (char c : std::string("crane"))
        letter(c);
    key(sf::Keyboard::Enter);
    expect(wordle.board().used == 1, "typed lowercase guess submits");
    expect(revealElapsed == 0 && revealedRow == 0, "submitting starts a staggered row reveal");
    letter('A');
    expect(wordle.entry().empty(), "input is blocked during tile flips");
    update(.1f);
    expect(nextTileCue == 0, "tile sound waits for the reveal midpoint");
    update(.1f);
    expect(nextTileCue == 1, "first tile cue is staggered");
    expect(render("wordle-flip"), "tile flip frame renders");
    for (int i = 0; i < 11; ++i)
        update(.1f);
    expect(nextTileCue == 5, "all five tile cues complete in sequence");
    if (wordle.finished())
        launch(1, true);
    letter('A');
    letter('B');
    expect(wordle.entry() == "AB", "text input is not duplicated");
    key(sf::Keyboard::Backspace);
    expect(wordle.entry() == "A", "backspace maps correctly");
    click({367, 617});
    expect(wordle.entry() == "AQ", "wordle on-screen keyboard matches visual keys");
    click({1120, 577});
    expect(wordle.entry() == "A", "wordle backspace button matches visual control");
    key(sf::Keyboard::Backspace);
    for (char c : std::string("ADIEU"))
        letter(c);
    int previousRows = wordle.board().used;
    click({940, 577});
    expect(wordle.board().used == previousRows + 1 && wordle.entry().empty(),
           "on-screen Enter accepts a common first guess from the expanded bank");
    for (int i = 0; i < 12; ++i)
        update(.1f);
    letter('A');
    click({1130, 170});
    expect(menu.page == Page::Library, "game library button");
    int quota = player.wordleRounds;
    click({600, 420});
    expect(wordle.entry() == "A" && player.wordleRounds == quota,
           "resume preserves entry and quota");
    expect(render("wordle"), "wordle screen renders");
    key(sf::Keyboard::Escape);
    expect(menu.paused, "escape pauses word game");
    click({640, 455});
    expect(!menu.paused, "resume button");
    click({1130, 170});
    click({1030, 420});
    expect(active == 2 && menu.page == Page::Playing, "hangman card click");
    int before = hangman.wrong();
    letter('Z');
    expect(hangman.wrong() == before + 1, "hangman letter input");
    letter('Z');
    expect(hangman.wrong() == before + 1, "repeat letter protected through input system");
    key(sf::Keyboard::F1);
    expect(hangman.hasHint(), "F1 hint input");
    update(.9f);
    click({1080, 210});
    expect(categoryOpen, "Hangman category menu opens");
    click({1050, 320});
    expect(hangman.theme() == 2 && !hangman.hasHint() && hangman.wrong() == 0,
           "category button resets correct cartridge");
    expect(render("hangman"), "hangman screen renders");
    key(sf::Keyboard::F2);
    expect(menu.page == Page::Help, "contextual help opens");
    key(sf::Keyboard::Escape);
    expect(menu.page == Page::Playing && menu.paused, "contextual guide returns paused");
    click({790, 65});
    expect(menu.page == Page::Settings, "settings navigation");
    click({791, 295});
    expect(settings.music > 49 && settings.music < 51, "music slider");
    pointer(sf::Event::MouseButtonPressed, 548, 295);
    expect(settings.music < 1, "mouse event uses reported button position");
    pointer(sf::Event::MouseMoved, 1200, 295);
    expect(settings.music == 100, "dragged slider clamps at its maximum");
    pointer(sf::Event::MouseButtonReleased, 1200, 295);
    expect(draggingSlider == -1, "releasing mouse stops slider drag");
    pointer(sf::Event::MouseMoved, 600, 295);
    expect(settings.music == 100, "released slider ignores later motion");
    click({791, 295});

    click({548, 390});
    expect(settings.effects == 0, "effects mute");
    click({791, 480});
    expect(settings.master > 49 && settings.master < 51, "master volume slider");
    click({280, 643});
    expect(settings.muted, "master mute toggle");
    click({1050, 186});
    expect(toast == "Turn sound on and raise Effects and Master Volume.",
           "sound test clearly explains muted settings");
    click({280, 643});
    expect(!settings.muted, "master unmute toggle");
    settings.effects = 55;
    click({1050, 186});
    expect(toast == "Sound test: a short chime.", "sound test button dispatches an effect");
    click({1000, 567});
    expect(settings.difficulty == 2, "difficulty button");
    bool scanlines = settings.scanlines;
    click({640, 643});
    expect(settings.scanlines != scanlines, "scanline toggle");
    expect(render("settings"), "settings screen renders");
    click({1080, 65});
    expect(menu.page == Page::Profiles, "profile page");
    click({650, 560});
    expect(editing, "name edit begins");
    editName.clear();
    for (char c : std::string("TEST PILOT"))
        letter(c);
    key(sf::Keyboard::Enter);
    expect(player.name() == "TEST PILOT" && !editing, "name editing and commit");
    saveAll();
    Player saved;
    Settings options;
    expect(store.load(0, saved, options) && saved.name() == "TEST PILOT" && options.difficulty == 2,
           "UI profile saves settings and identity");
    // The store is isolated, and no real profile is overwritten by these checks.
    click({670, 360});
    expect(profileSlot == 1 && player.name() == "PLAYER 02", "profile switching");
    click({590, 65});
    expect(menu.page == Page::Library, "library navigation");
    key(sf::Keyboard::Num3);
    expect(active == 2, "new profile launches its own game");
    key(sf::Keyboard::Escape);
    click({640, 570});
    expect(menu.page == Page::Library, "pause save and exit button");
    click({1184, 751});
    expect(menu.page == Page::Sleep, "power sleeps");
    key(sf::Keyboard::Space);
    expect(menu.page == Page::Home, "any key wakes to main menu");
    key(sf::Keyboard::Enter);
    sf::Keyboard::Key sequence[] = {sf::Keyboard::Up,   sf::Keyboard::Up,    sf::Keyboard::Down,
                                    sf::Keyboard::Down, sf::Keyboard::Left,  sf::Keyboard::Right,
                                    sf::Keyboard::Left, sf::Keyboard::Right, sf::Keyboard::B,
                                    sf::Keyboard::A};
    for (auto code : sequence)
        key(code);
    expect(secret, "secret sequence unlocks theme");
    auto logical = input.toLogical({500, 350}, {1000, 700});
    expect(std::abs(logical.x - 640) < .01f && std::abs(logical.y - 400) < .01f,
           "letterboxed mouse center maps correctly");
    expect(input.toLogical({0, 0}, {0, 0}) == sf::Vector2f(-1, -1),
           "zero-size input mapping is guarded");
    window.setSize({1000, 700});
    expect(render("resized"), "letterboxed resolution renders");
    expect(window.getView().getSize() == sf::Vector2f(1000, 700),
           "presentation view follows resized client dimensions");
    sf::Vector2f center = window.mapPixelToCoords({500, 350});
    expect(std::abs(center.x - 500) < .01f && std::abs(center.y - 350) < .01f,
           "resized presentation uses pixel coordinates");
    player.wordleWindow = 0;
    launch(1, true);
    std::string winningWord = wordle.solution();
    for (char c : winningWord)
        letter(c);
    key(sf::Keyboard::Enter);
    expect(wordle.won() && recorded[1], "wordle win is recorded before tile animation finishes");
    int winsBefore = player.records[1].wins;
    click({1130, 170});
    Player afterFastExit;
    Settings afterFastExitOptions;
    expect(store.load(profileSlot, afterFastExit, afterFastExitOptions) &&
               afterFastExit.records[1].wins == winsBefore,
           "leaving immediately after victory preserves the score");
    update(.2f);
    expect(player.records[1].wins == winsBefore, "completed round is never recorded twice");
    // Verify Wordle category menus, keyboard selection, persistence, and all three colours.
    player.wordleWindow = 0;
    launch(1, true);
    click({190, 219});
    expect(categoryOpen, "Wordle category menu opens");
    expect(render("wordle-categories"), "category list renders above the illustration");
    click({190, 299}); // Places: second entry.
    expect(!categoryOpen && wordle.theme() == 1 && WordleCategories::contains(1, wordle.solution()),
           "mouse selects Places answers");
    key(sf::Keyboard::Tab);
    key(sf::Keyboard::Down);
    key(sf::Keyboard::Enter);
    expect(!categoryOpen && wordle.theme() == 2, "keyboard selects Footballers category");
    for (int c = 0; c < WordleCategories::Count; ++c) {
        chooseCategory(c);
        expect(wordle.theme() == c && WordleCategories::contains(c, wordle.solution()),
               "every Wordle category can start a fresh puzzle");
    }
    chooseCategory(0);
    std::stringstream apple("APPLE\n\"\"\n0\n");
    expect(wordle.load(apple), "known answer fixture loads");
    for (char c : std::string("ALERT"))
        letter(c);
    key(sf::Keyboard::Enter);
    update(1.1f);
    expect(wordle.board().colors[0][0] == 3 && wordle.board().colors[0][1] == 2 &&
               wordle.board().colors[0][2] == 2 && wordle.board().colors[0][3] == 1 &&
               wordle.board().colors[0][4] == 1,
           "submitted row contains green yellow and black states");
    expect(render("wordle-colors"), "all three Wordle colours render on a submitted row");
    auto pixels = screen.canvas().getTexture().copyToImage();
    expect(pixels.getPixel(484, 225) == sf::Color(38, 103, 44) &&
               pixels.getPixel(549, 225) == sf::Color(220, 174, 35) &&
               pixels.getPixel(679, 225) == sf::Color(6, 8, 7),
           "actual row pixels are green yellow and black");
    // A record celebration never interrupts movement, and a collision waits one second.
    launch(0, true);
    player.records[0].best = 10;
    effects.reset(10, 0);
    std::stringstream foodFixture("0 0 0 0 0 1 0 0\n9 8 -1 -1\n4 1 1 0\n8 8\n7 8\n6 8\n5 8\n");
    expect(snake.load(foodFixture), "high-score pickup fixture loads");
    snake.input(13);
    update(.2f);
    expect(snake.score() == 20 && effects.recordTime() > 0 && !snake.finished(),
           "crossing previous best starts celebration during play");
    expect(render("snake-record"), "high-score ribbon and particles render");
    Player liveRecord;
    Settings liveOptions;
    expect(store.load(profileSlot, liveRecord, liveOptions) && liveRecord.records[0].best == 20,
           "new high score persists during the live round before a collision");
    expect(!effects.checkRecord(40), "record celebration fires once per round");
    std::stringstream wallFixture("20 0 0 1 0 1 0 0\n0 0 -1 -1\n4 1 1 0\n27 8\n26 8\n25 8\n24 8\n");
    expect(snake.load(wallFixture), "collision fixture loads");
    recorded[0] = false;
    int roundsBeforeCrash = player.records[0].rounds;
    update(.2f);
    expect(snake.finished() && effects.crashing() && endingAnimation() && outcomePending,
           "collision starts timed ending before the result screen");
    expect(recorded[0] && player.records[0].rounds == roundsBeforeCrash + 1,
           "collision score persists before animation finishes");
    key(sf::Keyboard::Enter);
    click({640, 486});
    expect(snake.finished() && effects.crashing(),
           "early restart input cannot skip the impact pause");
    update(.18f);
    expect(effects.shake() != sf::Vector2f(0, 0), "impact produces camera vibration");
    expect(render("snake-impact"), "impact title and blackout pulse render");
    update(.68f);
    expect(render("snake-blackout"), "episode ending reaches blackout");
    expect(screen.canvas().getTexture().copyToImage().getPixel(640, 400) == sf::Color::Black,
           "ending actually blacks out the game area");
    update(.2f);
    expect(!endingAnimation() && !outcomePending &&
               player.records[0].rounds == roundsBeforeCrash + 1,
           "one second later the result is ready and score is not counted twice");
    expect(render("snake-result"), "result appears after the ending");
    key(sf::Keyboard::Enter);
    expect(!snake.finished() && !effects.crashing(), "restarting clears the previous crash effect");
    // Both Hangman reactions are distinct from adding body parts.
    launch(2, true);
    for (int c = 0; c < WordCategories::Count; ++c) {
        chooseCategory(c);
        expect(hangman.theme() == c, "every Hangman category can be selected");
    }
    chooseCategory(3);
    char correct = hangman.solution()[0];
    letter(correct);
    expect(effects.reactionKind() == 1 && effects.reactionTime() > 0 && hangman.wrong() == 0,
           "correct letter starts green sparkle and letter bounce");
    update(.16f);
    expect(render("hangman-correct"), "correct reaction renders");
    float reactionBeforeRepeat = effects.reactionTime();
    letter(correct);
    expect(effects.reactionTime() == reactionBeforeRepeat && hangman.wrong() == 0,
           "repeated letter does not restart reaction or cost a life");
    char wrongLetter = 'A';
    while (hangman.solution().find(wrongLetter) != std::string::npos)
        ++wrongLetter;
    letter(wrongLetter);
    expect(effects.reactionKind() == -1 && effects.reactionTime() > 0 && hangman.wrong() == 1,
           "incorrect letter starts red flash sway and life pulse");
    update(.16f);
    expect(render("hangman-wrong"), "wrong reaction renders");
    for (char c = 'A'; c <= 'Z' && !hangman.finished(); ++c)
        if (hangman.solution().find(c) == std::string::npos)
            letter(c);
    expect(hangman.finished() && endingAnimation() && recorded[2],
           "last incorrect guess still gets a reaction before the result");
    update(.9f);
    expect(!endingAnimation() && !outcomePending, "Hangman outcome follows the final reaction");
    Settings silent;
    silent.music = silent.effects = 0;
    sound.volumes(silent);
    expect(sound.load(), "included audio assets decode and original loop initializes");
    sound.setScene(1);
    sound.update(.5f);
    expect(sound.currentScene() == 1, "forest ambience scene selected");
    sound.setScene(2);
    sound.update(.5f);
    expect(sound.currentScene() == 2, "library music crossfade completes");
    sound.setScene(3);
    sound.update(.5f);
    expect(sound.currentScene() == 3, "wanderer music scene selected");
    Settings mix;
    mix.master = 50;
    mix.music = 40;
    mix.effects = 0;
    sound.volumes(mix);
    expect(std::abs(sound.mixVolume() - 20) < .01f, "master scales music volume");
    mix.muted = true;
    sound.volumes(mix);
    expect(sound.mixVolume() == 0, "mute silences scene music");
    mix.muted = false;
    sound.volumes(mix);
    sound.pause(true);
    expect(sound.mixVolume() == 0, "sleep silences audio");
    sound.pause(false);
    expect(std::abs(sound.mixVolume() - 20) < .01f, "wake restores configured audio gain");
    sound.pause(true);

    report << checks - failures << " / " << checks << " application-flow checks passed\n";
    report.flush();
    window.close();
    return failures ? 1 : 0;
}
} // namespace pocket
