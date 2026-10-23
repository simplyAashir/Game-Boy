#include "Model.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace pocket;
int checks = 0, failures = 0;
void expect(bool condition, const char *label) {
    ++checks;
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << label << '\n';
    }
}
void type(Game &g, const std::string &s) {
    for (char c : s)
        g.input(c);
}
int main() {
    std::srand(42);
    Snake s;
    s.reset();
    expect(s.length() == 4, "snake initial length");
    expect(s.next() == Cell{9, 8}, "snake initial direction");
    s.turn(Direction::Left);
    expect(s.next() == Cell{9, 8}, "reject reversal");
    s.turn(Direction::Up);
    s.turn(Direction::Left);
    expect(s.next() == Cell{8, 7}, "one direction change per tick");
    s.advance(true);
    expect(s.length() == 5 && s.segment(0).cell == Cell{8, 7}, "growth advances head");
    expect(s.segment(4).cell == Cell{5, 8}, "growth preserves tail");
    s.turn(Direction::Left);
    s.advance(false);
    expect(s.segment(0).cell == Cell{7, 7}, "turn after tick");
    Food f;
    Grid grid;
    for (int i = 0; i < 1000; ++i) {
        f.spawn(s);
        expect(grid.contains(f.cell) && !s.occupies(f.cell), "food valid and unoccupied");
    }
    std::stringstream loop("4 0 0 0\n1 1\n2 1\n2 2\n1 2\n");
    Snake tail;
    expect(tail.load(loop), "valid bent snake loads");
    tail.turn(Direction::Left); // head at (1,1), left is (0,1)
    Collision collision;
    expect(!collision.hits(grid, tail, Cell{1, 2}, false), "vacating tail is legal");
    expect(collision.hits(grid, tail, Cell{1, 2}, true), "tail occupied when growing");
    expect(collision.hits(grid, tail, Cell{-1, 0}, false), "wall collision");
    expect(collision.hits(grid, tail, Cell{2, 1}, false), "body collision");
    std::stringstream corrupt("99999 0 0 0");
    expect(!s.load(corrupt) && s.length() == 5, "corrupt snake rejected without mutation");
    SnakeGame sg;
    sg.update(10);
    expect(!sg.running() && !sg.finished(), "snake waits to start");
    sg.input(13);
    for (int i = 0; i < 100; ++i)
        sg.update(.2f);
    expect(sg.finished() && !sg.won(), "snake wall ends run");
    SnakeGame eater;
    std::stringstream meal("0 0 0 0 0 1 0 0\n9 8 -1 -1\n4 1 1 0\n8 8\n7 8\n6 8\n5 8\n");
    expect(eater.load(meal), "snake fixture loads");
    eater.input(13);
    eater.update(.2f);
    expect(eater.score() == 20 && eater.body().length() == 5, "eating scores and grows");
    std::stringstream snakeSaved;
    eater.save(snakeSaved);
    SnakeGame restored;
    expect(restored.load(snakeSaved) && restored.score() == 20 && restored.body().length() == 5,
           "snake state roundtrip");
    SnakeGame powered;
    std::stringstream powerSave("0 0 0 0 0 1 0 8\n20 12 9 8\n4 1 1 0\n8 8\n7 8\n6 8\n5 8\n");
    expect(powered.load(powerSave), "power-up fixture loads");
    powered.input(13);
    powered.update(.2f);
    expect(powered.score() == 25 && powered.slowed() > 0 && powered.bonus().x == -1,
           "power-up grants points and slow motion then disappears");
    float slowBefore = powered.slowed();
    powered.update(.1f);
    expect(powered.slowed() < slowBefore, "slow-motion duration decreases");
    Snake full;
    std::stringstream fullSave;
    fullSave << Grid::Capacity << " 1 1 0\n";
    for (int y = 0; y < Grid::Rows; ++y)
        for (int x = 0; x < Grid::Columns; ++x)
            fullSave << (y % 2 ? Grid::Columns - 1 - x : x) << ' ' << y << '\n';
    expect(full.load(fullSave), "maximum-capacity snake loads");
    f.spawn(full);
    expect(f.cell == Cell{-1, -1}, "full board food generation terminates");
    int colors[5];
    LetterGrid::evaluate("APPLE", "ALERT", colors);
    expect(colors[0] == 3 && colors[1] == 2 && colors[2] == 2 && colors[3] == 1 && colors[4] == 1,
           "wordle exact present absent");
    LetterGrid::evaluate("APPLE", "PUPPY", colors);
    expect(colors[0] == 2 && colors[1] == 1 && colors[2] == 3 && colors[3] == 1 && colors[4] == 1,
           "wordle duplicate allocation");
    LetterGrid::evaluate("LEVEL", "HELLO", colors);
    expect(colors[0] == 1 && colors[1] == 3 && colors[2] == 2 && colors[3] == 2 && colors[4] == 1,
           "wordle two repeated letters");
    KeyboardDisplay keyboard;
    int green[5] = {3, 1, 1, 1, 1}, gray[5] = {1, 1, 1, 1, 1};
    keyboard.record("APPLE", green);
    keyboard.record("APPLE", gray);
    expect(keyboard.states[0] == 3, "keyboard never downgrades green");
    for (const char *guess : {"ADIEU", "ABOUT", "ARISE", "STARE", "PUPPY", "HELLO"}) {
        WordleGame firstRow;
        type(firstRow, guess);
        firstRow.input(13);
        expect(firstRow.board().used == 1 && firstRow.entry().empty(),
               "common valid first guess submits and clears entry");
        std::stringstream stored;
        firstRow.save(stored);
        WordleGame reloaded;
        expect(reloaded.load(stored) && reloaded.board().rows[0] == guess,
               "expanded dictionary guesses survive save and load");
    }
    WordleGame wg;
    wg.input('A');
    wg.input(13);
    expect(wg.board().used == 0, "short word rejected");
    wg.input(8);
    type(wg, "ZZZZZ");
    wg.input(13);
    expect(wg.board().used == 1 && wg.board().rows[0] == "ZZZZZ",
           "every complete five-letter guess receives feedback");
    for (int c = 0; c < WordleCategories::Count; ++c) {
        for (int i = 0; i < WordleCategories::size(c); ++i) {
            std::string word = WordleCategories::word(c, i);
            expect(word.size() == 5 && WordleCategories::contains(c, word),
                   "category answer has exactly five letters");
        }
        WordleGame themed;
        themed.setCategory(c);
        type(themed, "SHSKS");
        themed.input(13);
        std::stringstream snapshot;
        themed.save(snapshot);
        WordleGame resumed;
        expect(resumed.load(snapshot) && resumed.theme() == c &&
                   resumed.solution() == themed.solution() && resumed.board().rows[0] == "SHSKS",
               "category, arbitrary guess and feedback persist");
    }
    for (int c = 0; c < WordCategories::Count; ++c) {
        HangmanGame themed;
        themed.setCategory(c);
        themed.input(1004);
        std::stringstream snapshot;
        themed.save(snapshot);
        HangmanGame resumed;
        expect(resumed.load(snapshot) && resumed.theme() == c && resumed.hasHint() &&
                   resumed.solution() == themed.solution(),
               "every Hangman category persists");
        for (int i = 0; i < WordCategories::Words; ++i) {
            std::string word = WordCategories::word(c, i);
            bool alphabetic = !word.empty();
            for (char letter : word)
                alphabetic = alphabetic && letter >= 'A' && letter <= 'Z';
            expect(alphabetic && std::string(WordCategories::clue(c, i)).size() > 0,
                   "category entry has a guessable name and clue");
        }
    }
    std::stringstream legacyWordle("APPLE\n\"AB\"\n1\nALERT\n");
    WordleGame legacyRound;
    expect(legacyRound.load(legacyWordle) && legacyRound.theme() == 0 &&
               legacyRound.entry() == "AB" && legacyRound.board().used == 1,
           "legacy Wordle save remains readable");
    expect(!WordleCategories::contains(-1, "APPLE") && !WordleCategories::contains(6, "APPLE"),
           "out of range category is rejected");
    wg.reset();
    std::string answer = wg.solution();
    type(wg, answer);
    wg.input(13);
    expect(wg.finished() && wg.won() && wg.score() == 600, "wordle victory and score");
    int used = wg.board().used;
    wg.input('A');
    wg.input(13);
    expect(wg.board().used == used, "finished wordle immutable");
    WordleGame loss;
    std::string wrong = loss.solution() == "CRANE" ? "APPLE" : "CRANE";
    for (int i = 0; i < 6; ++i) {
        type(loss, wrong);
        loss.input(13);
    }
    expect(loss.finished() && !loss.won() && loss.board().used == 6, "six attempts ends wordle");
    WordleGame partial;
    type(partial, "AB");
    std::stringstream partialSave;
    partial.save(partialSave);
    WordleGame loadedWord;
    expect(loadedWord.load(partialSave) && loadedWord.entry() == "AB" &&
               loadedWord.solution() == partial.solution(),
           "wordle pending and answer persist");
    std::stringstream invalidWord("ZZZZZ\n\"\"\n0\n");
    expect(!loadedWord.load(invalidWord), "unknown saved answer rejected");
    HangmanGame hg;
    char missing = 'A';
    while (hg.solution().find(missing) != std::string::npos)
        ++missing;
    hg.input(missing);
    hg.input(missing);
    expect(hg.wrong() == 1, "duplicate wrong letters don't cost lives");
    hg.input(1004);
    int guesses = 0;
    for (char c = 'A'; c <= 'Z'; ++c)
        if (hg.used(c))
            ++guesses;
    hg.input(1004);
    int guessesAfter = 0;
    for (char c = 'A'; c <= 'Z'; ++c)
        if (hg.used(c))
            ++guessesAfter;
    expect(hg.hasHint() && guesses == guessesAfter, "hint usable once");
    std::string solution = hg.solution();
    type(hg, solution);
    expect(hg.won() && hg.finished() && hg.score() == 425, "hangman win with hint penalty");
    hg.reset();
    for (char c = 'A'; c <= 'Z' && !hg.finished(); ++c)
        if (hg.solution().find(c) == std::string::npos)
            hg.input(c);
    expect(hg.finished() && !hg.won() && hg.wrong() == 6, "hangman six errors loss");
    hg.setCategory(2);
    expect(hg.theme() == 2 && !hg.finished() && hg.wrong() == 0, "category resets round");
    hg.input(1004);
    std::stringstream savedHangman;
    hg.save(savedHangman);
    HangmanGame recovered;
    expect(recovered.load(savedHangman) && recovered.solution() == hg.solution() &&
               recovered.hasHint(),
           "hangman state roundtrip");
    Player player;
    const std::time_t now = 100000;
    expect(player.remainingWordle(now) == 3, "initial wordle quota");
    expect(player.beginWordle(now) && player.beginWordle(now) && player.beginWordle(now),
           "three rounds allowed");
    expect(!player.beginWordle(now + 899), "fourth round blocked");
    expect(player.beginWordle(now + 900) && player.wordleRounds == 1,
           "quota renews after 15 minutes");
    player.record(wg);
    expect(player.records[1].best == 600 && player.records[1].wins == 1 &&
               player.achievements.has(2),
           "scores and achievement");
    player.rename("Test/../Player");
    expect(player.name() == "TestPlayer", "profile sanitization");
    ProfileStore store("build/test-data");
    Settings settings;
    settings.music = 31;
    expect(store.save(0, player, settings), "profile save");
    Player other;
    Settings options;
    expect(store.load(0, other, options) && other.name() == player.name() &&
               other.records[1].best == 600 && options.music == 31,
           "profile roundtrip");
    expect(store.save(0, player, settings), "profile replacement");
    {
        std::ofstream damage("build/test-data/profile0.sav");
        damage << "broken";
    }
    expect(store.load(0, other, options) && other.records[1].best == 600,
           "profile backup recovery");
    expect(store.saveGame(0, partial), "save cartridge");
    WordleGame cartridge;
    expect(store.loadGame(0, cartridge) && cartridge.entry() == "AB", "load cartridge");
    expect(!store.loadGame(0, hg), "missing cartridge rejected");
    expect(!store.save(-1, player, settings), "invalid slot rejected");
    Settings fantasyOptions;
    fantasyOptions.master = 47;
    fantasyOptions.music = 32;
    fantasyOptions.effects = 21;
    fantasyOptions.muted = true;
    expect(store.save(2, player, fantasyOptions), "fantasy audio settings save");
    Settings reloadedOptions;
    Player reloadedPlayer;
    expect(store.load(2, reloadedPlayer, reloadedOptions) && reloadedOptions.master == 47 &&
               reloadedOptions.muted,
           "master volume and mute persist");
    {
        std::ofstream legacy("build/test-data/profile1.sav");
        legacy << "POCKET_PROFILE_1\n\"OLD HERO\"\n0 0 0\n0 0 0\n0 0 0\n0 0 0\n22 65 1 1\n";
    }
    expect(store.load(1, reloadedPlayer, reloadedOptions) && reloadedPlayer.name() == "OLD HERO" &&
               !reloadedOptions.scanlines,
           "old profile loads with fantasy display defaults");
    GameCollection collection(sg, wg, hg);
    Game &polymorphic = collection.at(1);
    expect(polymorphic.id() == GameId::Wordle && polymorphic.won(), "polymorphic game access");
    GuestPlayer guest;
    User &user = guest;
    expect(!user.persistent(), "user hierarchy dispatch");
    std::cout << (checks - failures) << " / " << checks << " checks passed\n";
    return failures ? 1 : 0;
}
