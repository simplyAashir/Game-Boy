#include "Model.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
namespace pocket {
namespace {
const char *words[] = {
    "APPLE", "ALERT", "ALIEN", "ANGLE", "AUDIO", "BEACH", "BEAST", "BERRY", "BIRCH", "BLACK",
    "BLADE", "BLEND", "BLINK", "BLOCK", "BLOOM", "BOARD", "BRAIN", "BRAVE", "BREAD", "BRICK",
    "BRING", "BRUSH", "CABIN", "CANDY", "CARRY", "CHARM", "CHASE", "CHEST", "CHILL", "CIVIC",
    "CLEAN", "CLEAR", "CLIMB", "CLOCK", "CLOUD", "COAST", "COMET", "CORAL", "CRANE", "CRATE",
    "CRISP", "CROWN", "DANCE", "DEALT", "DREAM", "DRIFT", "DRINK", "DRIVE", "EARTH", "EIGHT",
    "EMBER", "EMPTY", "ENJOY", "ENTER", "EQUAL", "EVENT", "EVERY", "EXTRA", "FAITH", "FEAST",
    "FIELD", "FLAME", "FLASH", "FLOAT", "FLOOR", "FOCUS", "FORCE", "FRAME", "FRESH", "FRONT",
    "FROST", "FRUIT", "GHOST", "GIANT", "GLASS", "GLOBE", "GLORY", "GRACE", "GRAPE", "GRASS",
    "GREAT", "GREEN", "GREET", "GROVE", "GUARD", "GUESS", "GUIDE", "HAPPY", "HEART", "HELLO",
    "HONEY", "HORSE", "HOTEL", "HOUSE", "HUMAN", "HUMOR", "IDEAL", "IMAGE", "INDEX", "INPUT",
    "IVORY", "JELLY", "JOINT", "JUDGE", "JUICE", "KNIFE", "KNOCK", "LABEL", "LASER", "LATER",
    "LAUGH", "LAYER", "LEARN", "LEMON", "LEVEL", "LIGHT", "LIMIT", "LINEN", "LOCAL", "LOGIC",
    "LUCKY", "LUNAR", "MAGIC", "MAJOR", "MANGO", "MAPLE", "MARCH", "MATCH", "MAYBE", "METAL",
    "MIGHT", "MODEL", "MONEY", "MONTH", "MOTOR", "MOUSE", "MOVIE", "MUSIC", "NIGHT", "NOBLE",
    "NOISE", "NORTH", "NOVEL", "NURSE", "OCEAN", "OLIVE", "ORBIT", "OTHER", "OTTER", "OUTER",
    "PAINT", "PANEL", "PAPER", "PARTY", "PAUSE", "PEACH", "PEARL", "PHASE", "PHONE", "PIANO",
    "PILOT", "PIXEL", "PIZZA", "PLACE", "PLAIN", "PLANE", "PLANT", "PLATE", "PLAZA", "POINT",
    "POWER", "PRESS", "PRICE", "PRIDE", "PRIME", "PRINT", "PRISM", "PROUD", "PULSE", "QUEEN",
    "QUEST", "QUICK", "QUIET", "QUILT", "RADAR", "RADIO", "RAISE", "RANGE", "REACH", "READY",
    "RELAY", "RESET", "RIDER", "RIVER", "ROBIN", "ROBOT", "ROUND", "ROYAL", "RUGBY", "SAFER",
    "SALAD", "SCALE", "SCENE", "SCORE", "SCOUT", "SEVEN", "SHADE", "SHAKE", "SHAPE", "SHARE",
    "SHARK", "SHEEP", "SHEET", "SHELL", "SHIFT", "SHINE", "SHIRT", "SHOCK", "SHORE", "SHORT",
    "SIGHT", "SINCE", "SKILL", "SLATE", "SLEEP", "SLICE", "SLIDE", "SMALL", "SMART", "SMILE",
    "SMOKE", "SNAKE", "SOLAR", "SOLID", "SOLVE", "SOUND", "SOUTH", "SPACE", "SPARK", "SPEAK",
    "SPEED", "SPELL", "SPICE", "SPIKE", "SPOON", "SPORT", "SPRAY", "SQUAD", "STACK", "STAGE",
    "STAIR", "STAMP", "STAND", "START", "STATE", "STEAM", "STEEL", "STILL", "STONE", "STORM",
    "STORY", "STRAW", "STUDY", "STYLE", "SUGAR", "SUNNY", "SUPER", "SWEET", "SWIFT", "TABLE",
    "TEACH", "THANK", "THEIR", "THEME", "THERE", "THICK", "THING", "THINK", "THIRD", "THORN",
    "THREE", "THROW", "TIGER", "TIMER", "TITLE", "TOAST", "TODAY", "TOKEN", "TOUCH", "TOWER",
    "TRACE", "TRACK", "TRAIL", "TRAIN", "TREAT", "TREND", "TRIAL", "TRICK", "TRULY", "TRUST",
    "TULIP", "ULTRA", "UNDER", "UNION", "UNITY", "UNTIL", "URBAN", "USAGE", "USUAL", "VALID",
    "VALUE", "VIDEO", "VINYL", "VISIT", "VITAL", "VOICE", "WATER", "WHALE", "WHEAT", "WHEEL",
    "WHERE", "WHICH", "WHITE", "WHOLE", "WIDER", "WITCH", "WOMAN", "WORLD", "WORRY", "WORTH",
    "WOULD", "WRITE", "WRONG", "YACHT", "YEARN", "YIELD", "YOUNG", "YOUTH", "ZEBRA", "ZESTY"};
const char *categoryWords[6][8] = {
    {"DRAGON", "PHOENIX", "UNICORN", "GRIFFIN", "WYVERN", "SPRITE", "KRAKEN", "PEGASUS"},
    {"AMULET", "GRIMOIRE", "CHALICE", "LANTERN", "SCEPTER", "CRYSTAL", "COMPASS", "TALISMAN"},
    {"WILLOW", "THISTLE", "CLOVER", "ROWAN", "FOREST", "MEADOW", "ORCHID", "JASMINE"},
    {"PARIS", "TOKYO", "DUBAI", "LONDON", "CAIRO", "BERLIN", "SYDNEY", "ISTANBUL"},
    {"MESSI", "RONALDO", "NEYMAR", "SALAH", "MBAPPE", "HAALAND", "ROONEY", "HENRY"},
    {"TIGER", "ELEPHANT", "LION", "GIRAFFE", "PENGUIN", "DOLPHIN", "ZEBRA", "KANGAROO"}};
const char *clues[6][8] = {
    {"A winged creature that breathes fire", "A mythical bird reborn from ashes",
     "A horse with a single magical horn", "A guardian with eagle and lion features",
     "A winged dragon with two legs", "A tiny playful spirit of the forest",
     "A legendary sea creature with tentacles", "A mythical horse with feathered wings"},
    {"A small charm worn for protection", "An ancient book of magical spells",
     "A ceremonial cup for a royal feast", "A traveler carries this to light the way",
     "A royal staff and symbol of authority", "A gem that may hold enchanted power",
     "A traveler's guide to the cardinal directions", "An object believed to bring good fortune"},
    {"A graceful tree beside quiet waters", "A prickly plant with purple flowers",
     "A three-leaf plant associated with luck", "A tree with clusters of red berries",
     "A realm sheltered by countless trees", "An open field of flowers and grasses",
     "An elegant flower of the wild realm", "A fragrant white blossom at twilight"},
    {"French capital with the Eiffel Tower", "Capital of Japan", "UAE city with the Burj Khalifa",
     "UK capital on the Thames", "Egyptian capital near the pyramids", "Capital of Germany",
     "Australian city with a famous opera house", "Turkish city on two continents"},
    {"Argentine forward who won the 2022 World Cup", "Portuguese forward famous for number seven",
     "Brazilian forward known for skillful dribbling",
     "Egyptian forward nicknamed the Egyptian King",
     "French forward who scored a hat trick in the 2022 final",
     "Norwegian striker known for his goals", "Former England and Manchester United forward",
     "French Arsenal legend nicknamed King Thierry"},
    {"A big striped cat", "A huge animal with a trunk", "A big cat with a mane",
     "A tall animal with a very long neck", "A flightless bird that swims",
     "An intelligent marine mammal", "A black and white striped animal",
     "An Australian animal that carries a joey"}};
const char *wordleCategoryWords[5][8] = {
    {"PARIS", "TOKYO", "DUBAI", "DELHI", "SEOUL", "MIAMI", "PERTH", "CAIRO"},
    {"MESSI", "SALAH", "HENRY", "SILVA", "COSTA", "ALABA", "PEDRI", "DEPAY"},
    {"TIGER", "ZEBRA", "HORSE", "PANDA", "OTTER", "WHALE", "SHEEP", "MOOSE"},
    {"MAPLE", "CEDAR", "BIRCH", "TULIP", "DAISY", "GRASS", "RIVER", "OCEAN"},
    {"CHARM", "MAGIC", "SPELL", "QUEST", "WITCH", "GHOST", "GIANT", "ELVEN"}};
bool validLetters(const std::string &s, int max = 24) {
    if (s.empty() || static_cast<int>(s.size()) > max)
        return false;
    for (char c : s)
        if (c < 'A' || c > 'Z')
            return false;
    return true;
}
bool atomicFinish(const std::string &temp, const std::string &target) {
    std::error_code ec;
    const std::string backup = target + ".bak";
    std::filesystem::remove(backup, ec);
    ec.clear();
    if (std::filesystem::exists(target)) {
        std::filesystem::rename(target, backup, ec);
        if (ec)
            return false;
    }
    std::filesystem::rename(temp, target, ec);
    if (ec) {
        std::error_code ignored;
        if (std::filesystem::exists(backup))
            std::filesystem::rename(backup, target, ignored);
        return false;
    }
    return true;
}
} // namespace
void Snake::reset() {
    count = 4;
    heading = requested = Direction::Right;
    turned = false;
    for (int i = 0; i < count; ++i)
        segments[i] = {{8 - i, 8}, {8 - i, 8}};
}
void Snake::turn(Direction d) {
    if (!turned && d != heading && (static_cast<int>(d) + 2) % 4 != static_cast<int>(heading)) {
        requested = d;
        turned = true;
    }
}
Cell Snake::next() const {
    Cell c = segments[0].cell;
    switch (requested) {
    case Direction::Up:
        --c.y;
        break;
    case Direction::Down:
        ++c.y;
        break;
    case Direction::Left:
        --c.x;
        break;
    case Direction::Right:
        ++c.x;
        break;
    }
    return c;
}
bool Snake::occupies(Cell c, bool includeTail) const {
    for (int i = 0; i < count - (includeTail ? 0 : 1); ++i)
        if (segments[i].cell == c)
            return true;
    return false;
}
void Snake::advance(bool grow) {
    Cell destination = next();
    if (grow && count < Grid::Capacity) {
        segments[count] = segments[count - 1];
        ++count;
    }
    for (int i = 0; i < count; ++i)
        segments[i].previous = segments[i].cell;
    for (int i = count - 1; i > 0; --i)
        segments[i].cell = segments[i - 1].cell;
    segments[0].cell = destination;
    heading = requested;
    turned = false;
}
bool Snake::save(std::ostream &out) const {
    out << count << ' ' << static_cast<int>(heading) << ' ' << static_cast<int>(requested) << ' '
        << turned << '\n';
    for (int i = 0; i < count; ++i)
        out << segments[i].cell.x << ' ' << segments[i].cell.y << '\n';
    return static_cast<bool>(out);
}
bool Snake::load(std::istream &in) {
    int n, h, r, t;
    if (!(in >> n >> h >> r >> t) || n < 4 || n > Grid::Capacity || h < 0 || h > 3 || r < 0 ||
        r > 3 || (t != 0 && t != 1) || (h + 2) % 4 == r)
        return false;
    Grid grid;
    Snake candidate;
    candidate.count = n;
    for (int i = 0; i < n; ++i) {
        Cell c;
        if (!(in >> c.x >> c.y) || !grid.contains(c))
            return false;
        for (int j = 0; j < i; ++j)
            if (candidate.segments[j].cell == c)
                return false;
        if (i > 0) {
            Cell p = candidate.segments[i - 1].cell;
            if (std::abs(p.x - c.x) + std::abs(p.y - c.y) != 1)
                return false;
        }
        candidate.segments[i] = {c, c};
    }
    candidate.heading = static_cast<Direction>(h);
    candidate.requested = static_cast<Direction>(r);
    candidate.turned = t != 0;
    *this = candidate;
    return true;
}
void Food::spawn(const Snake &snake) {
    int free = Grid::Capacity - snake.length();
    if (free <= 0) {
        cell = {-1, -1};
        return;
    }
    int pick = std::rand() % free;
    for (int y = 0; y < Grid::Rows; ++y)
        for (int x = 0; x < Grid::Columns; ++x)
            if (!snake.occupies({x, y}) && pick-- == 0) {
                cell = {x, y};
                return;
            }
}
int WordDictionary::count() {
    return sizeof(words) / sizeof(words[0]);
}
const char *WordDictionary::word(int index) {
    return words[index % count()];
}
bool WordDictionary::accepts(const std::string &value) {
    if (value.size() != 5 || !validLetters(value, 5))
        return false;
    for (const char *word : words)
        if (value == word)
            return true;
    return false;
}
void LetterGrid::clear() {
    used = 0;
    for (auto &row : rows)
        row.clear();
    for (auto &row : colors)
        for (int &c : row)
            c = 0;
}
void LetterGrid::evaluate(const std::string &answer, const std::string &guess, int result[5]) {
    int remaining[26]{};
    for (int i = 0; i < 5; ++i) {
        result[i] = guess[i] == answer[i] ? 3 : 1;
        if (result[i] != 3)
            ++remaining[answer[i] - 'A'];
    }
    for (int i = 0; i < 5; ++i)
        if (result[i] != 3 && remaining[guess[i] - 'A'] > 0) {
            result[i] = 2;
            --remaining[guess[i] - 'A'];
        }
}
void KeyboardDisplay::reset() {
    for (int &s : states)
        s = 0;
}
void KeyboardDisplay::record(const std::string &guess, const int colors[5]) {
    for (int i = 0; i < 5; ++i)
        states[guess[i] - 'A'] = std::max(states[guess[i] - 'A'], colors[i]);
}
const char *WordCategories::name(int c) {
    static const char *n[] = {"Creatures", "Objects", "Nature", "Places", "Footballers", "Animals"};
    return n[c >= 0 && c < Count ? c : 0];
}
const char *WordCategories::word(int c, int i) {
    return categoryWords[c >= 0 && c < Count ? c : 0][i >= 0 && i < Words ? i : 0];
}
const char *WordCategories::clue(int c, int i) {
    return clues[c >= 0 && c < Count ? c : 0][i >= 0 && i < Words ? i : 0];
}
const char *WordleCategories::name(int c) {
    const char *names[] = {"General", "Places", "Footballers", "Animals", "Nature", "Fantasy"};
    return names[c >= 0 && c < Count ? c : 0];
}
int WordleCategories::size(int c) {
    return c == 0 ? WordDictionary::count() : 8;
}
const char *WordleCategories::word(int c, int i) {
    if (c <= 0 || c >= Count)
        return WordDictionary::word(i);
    return wordleCategoryWords[c - 1][i >= 0 && i < 8 ? i : 0];
}
bool WordleCategories::contains(int c, const std::string &word) {
    if (c < 0 || c >= Count)
        return false;
    for (int i = 0; i < size(c); ++i)
        if (word == WordleCategories::word(c, i))
            return true;
    return false;
}
void LetterTracker::reset() {
    for (bool &b : used)
        b = false;
}
bool LetterTracker::contains(char c) const {
    return c >= 'A' && c <= 'Z' && used[c - 'A'];
}
bool LetterTracker::mark(char c) {
    if (c < 'A' || c > 'Z' || contains(c))
        return false;
    used[c - 'A'] = true;
    return true;
}
void LetterTracker::save(std::ostream &out) const {
    for (bool b : used)
        out << (b ? '1' : '0');
    out << '\n';
}
bool LetterTracker::load(std::istream &in) {
    std::string s;
    if (!(in >> s) || s.size() != 26)
        return false;
    for (char c : s)
        if (c != '0' && c != '1')
            return false;
    for (int i = 0; i < 26; ++i)
        used[i] = s[i] == '1';
    return true;
}
void SnakeGame::reset() {
    snake.reset();
    food.spawn(snake);
    points = 0;
    ended = victory = started = false;
    accumulator = slowSeconds = powerSeconds = 0;
    eaten = 0;
    power = {-1, -1};
}
void SnakeGame::input(int key) {
    if (ended)
        return;
    if (key >= 1000 && key <= 1003) {
        snake.turn(static_cast<Direction>(key - 1000));
        started = true;
    }
    if (key == 13 || key == ' ')
        started = true;
}
float SnakeGame::blend() const {
    float interval = std::max(0.065f, 0.19f - difficulty * 0.025f - eaten * 0.003f) *
                     (slowSeconds > 0 ? 1.65f : 1.f);
    return std::min(1.f, accumulator / interval);
}
void SnakeGame::update(float dt) {
    if (ended || !started)
        return;
    slowSeconds = std::max(0.f, slowSeconds - dt);
    powerSeconds = std::max(0.f, powerSeconds - dt);
    if (powerSeconds == 0)
        power = {-1, -1};
    accumulator += std::min(dt, 0.25f);
    float interval = std::max(0.065f, 0.19f - difficulty * 0.025f - eaten * 0.003f) *
                     (slowSeconds > 0 ? 1.65f : 1.f);
    while (accumulator >= interval && !ended) {
        accumulator -= interval;
        step();
    }
}
void SnakeGame::step() {
    Cell destination = snake.next();
    bool grow = destination == food.cell;
    if (collision.hits(grid, snake, destination, grow)) {
        ended = true;
        return;
    }
    snake.advance(grow);
    if (destination == power) {
        slowSeconds = 7;
        powerSeconds = 0;
        power = {-1, -1};
        points += 25;
    }
    if (grow) {
        points += 10 * (1 + difficulty);
        ++eaten;
        if (snake.length() == Grid::Capacity) {
            ended = victory = true;
            return;
        }
        food.spawn(snake);
        if (eaten % 3 == 0) {
            Food bonus;
            bonus.spawn(snake);
            power = bonus.cell;
            if (power == food.cell)
                power = {-1, -1};
            powerSeconds = 8;
        } else if (power == food.cell)
            power = {-1, -1};
    }
}
bool SnakeGame::save(std::ostream &out) const {
    out << points << ' ' << ended << ' ' << victory << ' ' << started << ' ' << eaten << ' '
        << difficulty << ' ' << slowSeconds << ' ' << powerSeconds << '\n'
        << food.cell.x << ' ' << food.cell.y << ' ' << power.x << ' ' << power.y << '\n';
    return snake.save(out);
}
bool SnakeGame::load(std::istream &in) {
    SnakeGame c;
    if (!(in >> c.points >> c.ended >> c.victory >> c.started >> c.eaten >> c.difficulty >>
          c.slowSeconds >> c.powerSeconds >> c.food.cell.x >> c.food.cell.y >> c.power.x >>
          c.power.y))
        return false;
    if (c.points < 0 || c.points > 1000000 || c.eaten < 0 || c.eaten > Grid::Capacity - 4 ||
        c.difficulty < 0 || c.difficulty > 2 || !std::isfinite(c.slowSeconds) ||
        !std::isfinite(c.powerSeconds) || c.slowSeconds < 0 || c.slowSeconds > 7 ||
        c.powerSeconds < 0 || c.powerSeconds > 8 || !c.snake.load(in))
        return false;
    if (c.snake.length() != c.eaten + 4)
        return false;
    if (c.snake.length() < Grid::Capacity &&
        (!c.grid.contains(c.food.cell) || c.snake.occupies(c.food.cell)))
        return false;
    if (!(c.power == Cell{-1, -1}) &&
        (!c.grid.contains(c.power) || c.snake.occupies(c.power) || c.power == c.food.cell))
        return false;
    *this = c;
    return true;
}
void WordleGame::reset() {
    answer = WordleCategories::word(category, std::rand() % WordleCategories::size(category));
    pending.clear();
    letters.clear();
    keyboard.reset();
    points = 0;
    ended = victory = false;
    message = "Type a five-letter word.";
}
void WordleGame::input(int key) {
    if (ended)
        return;
    if (key >= 'a' && key <= 'z')
        key -= 32;
    if (key >= 'A' && key <= 'Z' && pending.size() < 5) {
        pending += static_cast<char>(key);
        message = "Enter to submit. Backspace to edit.";
    } else if (key == 8 && !pending.empty()) {
        pending.pop_back();
        message = "Enter to submit. Backspace to edit.";
    } else if (key == 13) {
        if (pending.size() != 5) {
            message = "Enter five letters first.";
            return;
        }
        int *colors = letters.colors[letters.used];
        LetterGrid::evaluate(answer, pending, colors);
        keyboard.record(pending, colors);
        letters.rows[letters.used++] = pending;
        victory = pending == answer;
        ended = victory || letters.used == 6;
        if (victory) {
            points = (7 - letters.used) * 100;
            message = "You found the word!";
        } else if (ended)
            message = "The word was " + answer + ".";
        else
            message = "Type your next guess.";
        pending.clear();
    }
}
bool WordleGame::save(std::ostream &out) const {
    out << "FANTASY_WORDLE_2 " << category << '\n'
        << answer << '\n'
        << std::quoted(pending) << '\n'
        << letters.used << '\n';
    for (int i = 0; i < letters.used; ++i)
        out << letters.rows[i] << '\n';
    return static_cast<bool>(out);
}
bool WordleGame::load(std::istream &in) {
    WordleGame c;
    int used;
    std::string entry;
    std::string header;
    if (!(in >> header))
        return false;
    if (header == "FANTASY_WORDLE_2") {
        if (!(in >> c.category >> c.answer))
            return false;
    } else {
        c.category = 0;
        c.answer = header; // Original five-letter saves remain readable.
    }
    if (!(in >> std::quoted(entry) >> used) || !WordleCategories::contains(c.category, c.answer) ||
        used < 0 || used > 6 || entry.size() > 5 || (!entry.empty() && !validLetters(entry, 5)))
        return false;
    for (int i = 0; i < used; ++i) {
        std::string guess;
        if (!(in >> guess) || guess.size() != 5 || !validLetters(guess, 5) || c.ended)
            return false;
        c.pending = guess;
        c.input(13);
    }
    if (c.ended && !entry.empty())
        return false;
    c.pending = entry;
    *this = c;
    return true;
}
void HangmanGame::reset() {
    chosen = std::rand() % WordCategories::Words;
    answer = WordCategories::word(category, chosen);
    figure.reset();
    points = 0;
    ended = victory = hinted = false;
    tracker.reset();
}
void HangmanGame::check() {
    victory = true;
    for (char c : answer)
        if (!tracker.contains(c))
            victory = false;
    ended = victory || wrong() >= HangmanFigure::MaxMistakes;
    if (victory)
        points = std::max(25, (HangmanFigure::MaxMistakes - wrong()) * 100 - (hinted ? 75 : 0));
}
void HangmanGame::input(int key) {
    if (ended)
        return;
    if (key == '?' || key == 1004) {
        if (!hinted) {
            hinted = true;
            for (char c : answer)
                if (!tracker.contains(c)) {
                    tracker.mark(c);
                    break;
                }
            check();
        }
        return;
    }
    if (key >= 'a' && key <= 'z')
        key -= 32;
    if (key < 'A' || key > 'Z' || !tracker.mark(static_cast<char>(key)))
        return;
    if (answer.find(static_cast<char>(key)) == std::string::npos)
        figure.addPart();
    check();
}
bool HangmanGame::save(std::ostream &out) const {
    out << "FANTASY_HANGMAN_2 " << category << ' ' << chosen << ' ' << hinted << '\n';
    tracker.save(out);
    return static_cast<bool>(out);
}
bool HangmanGame::load(std::istream &in) {
    HangmanGame c;
    std::string version;
    if (!(in >> version) || version != "FANTASY_HANGMAN_2")
        return false;
    if (!(in >> c.category >> c.chosen >> c.hinted) || c.category < 0 ||
        c.category >= WordCategories::Count || c.chosen < 0 || c.chosen >= WordCategories::Words ||
        !c.tracker.load(in))
        return false;
    c.answer = WordCategories::word(c.category, c.chosen);
    c.figure.reset();
    for (char ch = 'A'; ch <= 'Z'; ++ch)
        if (c.tracker.contains(ch) && c.answer.find(ch) == std::string::npos)
            if (!c.figure.addPart())
                return false;
    c.check();
    *this = c;
    return true;
}
void User::rename(const std::string &value) {
    std::string safe;
    for (char c : value)
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
            c == ' ' || c == '_') {
            if (safe.size() < 16)
                safe += c;
        }
    if (!safe.empty() && safe.find_first_not_of(' ') != std::string::npos)
        username = safe;
}
const char *Achievement::title(int b) {
    static const char *t[] = {"FIRST CONTACT", "SNAKE CHARMER", "WORD PERFECT", "GREAT ESCAPE",
                              "TRIPLE THREAT"};
    return t[b % 5];
}
const char *Achievement::description(int b) {
    static const char *t[] = {"Finish your first round", "Score 100 points in Snake",
                              "Decode a Wordle puzzle", "Win a round of Hangman",
                              "Play all three cartridges"};
    return t[b % 5];
}
void Player::record(const Game &g) {
    ScoreRecord &r = records[static_cast<int>(g.id())];
    r.best = std::max(r.best, g.score());
    ++r.rounds;
    if (g.won())
        ++r.wins;
    achievements.unlock(0);
    if (g.id() == GameId::Snake && g.score() >= 100)
        achievements.unlock(1);
    if (g.id() == GameId::Wordle && g.won())
        achievements.unlock(2);
    if (g.id() == GameId::Hangman && g.won())
        achievements.unlock(3);
    if (records[0].rounds && records[1].rounds && records[2].rounds)
        achievements.unlock(4);
}
int Player::remainingWordle(std::time_t now) {
    if (wordleWindow <= 0 || now - wordleWindow >= 900) {
        wordleWindow = now;
        wordleRounds = 0;
    }
    return std::max(0, 3 - wordleRounds);
}
bool Player::beginWordle(std::time_t now) {
    if (!remainingWordle(now))
        return false;
    ++wordleRounds;
    return true;
}
bool ProfileStore::save(int slot, const Player &p, const Settings &s) const {
    if (slot < 0 || slot >= 3)
        return false;
    std::error_code ec;
    std::filesystem::create_directories(directory, ec);
    if (ec)
        return false;
    std::string path = directory + "/profile" + std::to_string(slot) + ".sav", temp = path + ".tmp";
    std::ofstream out(temp);
    if (!out)
        return false;
    out << "POCKET_PROFILE_2\n" << std::quoted(p.name()) << '\n';
    for (const auto &r : p.records)
        out << r.best << ' ' << r.rounds << ' ' << r.wins << '\n';
    out << p.achievements.value() << ' ' << p.wordleWindow << ' ' << p.wordleRounds << '\n'
        << s.music << ' ' << s.effects << ' ' << s.difficulty << ' ' << s.scanlines << ' '
        << s.master << ' ' << s.muted << '\n';
    out.close();
    return static_cast<bool>(out) && atomicFinish(temp, path);
}
bool ProfileStore::load(int slot, Player &p, Settings &s) const {
    if (slot < 0 || slot >= 3)
        return false;
    auto read = [&](const std::string &path) {
        std::ifstream in(path);
        std::string magic, name;
        Player c;
        Settings options;
        unsigned flags;
        if (!(in >> magic >> std::quoted(name)) ||
            (magic != "POCKET_PROFILE_1" && magic != "POCKET_PROFILE_2") || name.size() > 16 ||
            name.empty())
            return false;
        c.rename(name);
        for (auto &r : c.records)
            if (!(in >> r.best >> r.rounds >> r.wins) || r.best < 0 || r.best > 1000000 ||
                r.rounds < 0 || r.rounds > 1000000 || r.wins < 0 || r.wins > r.rounds)
                return false;
        if (!(in >> flags >> c.wordleWindow >> c.wordleRounds >> options.music >> options.effects >>
              options.difficulty >> options.scanlines))
            return false;
        if (magic == "POCKET_PROFILE_2" && !(in >> options.master >> options.muted))
            return false;
        if (magic == "POCKET_PROFILE_1")
            options.scanlines = false;
        if (!std::isfinite(options.master) || options.master < 0 || options.master > 100)
            return false;
        if (flags > 31 || c.wordleRounds < 0 || c.wordleRounds > 3 || c.wordleWindow < 0 ||
            !std::isfinite(options.music) || !std::isfinite(options.effects) || options.music < 0 ||
            options.music > 100 || options.effects < 0 || options.effects > 100 ||
            options.difficulty < 0 || options.difficulty > 2)
            return false;
        c.achievements.restore(flags);
        p = c;
        s = options;
        return true;
    };
    std::string path = directory + "/profile" + std::to_string(slot) + ".sav";
    return read(path) || read(path + ".bak");
}
bool ProfileStore::saveGame(int slot, const Game &g) const {
    if (slot < 0 || slot >= 3)
        return false;
    std::error_code ec;
    std::filesystem::create_directories(directory, ec);
    if (ec)
        return false;
    std::string path = directory + "/game" + std::to_string(slot) + "_" +
                       std::to_string(static_cast<int>(g.id())) + ".sav",
                temp = path + ".tmp";
    std::ofstream out(temp);
    if (!out)
        return false;
    out << "POCKET_GAME_1 " << static_cast<int>(g.id()) << '\n';
    bool ok = g.save(out);
    out.close();
    return ok && static_cast<bool>(out) && atomicFinish(temp, path);
}
bool ProfileStore::loadGame(int slot, Game &g) const {
    if (slot < 0 || slot >= 3)
        return false;
    std::ifstream in(directory + "/game" + std::to_string(slot) + "_" +
                     std::to_string(static_cast<int>(g.id())) + ".sav");
    std::string magic;
    int id;
    if (!(in >> magic >> id) || magic != "POCKET_GAME_1" || id != static_cast<int>(g.id()))
        return false;
    return g.load(in);
}
} // namespace pocket
