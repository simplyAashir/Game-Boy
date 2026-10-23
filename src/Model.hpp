#pragma once
#include <ctime>
#include <iosfwd>
#include <string>
namespace pocket {
class Painter;
enum class GameId { Snake, Wordle, Hangman };
enum class Direction { Up, Right, Down, Left };
struct Cell {
    int x = 0, y = 0;
    bool operator==(const Cell &c) const {
        return x == c.x && y == c.y;
    }
};
class Grid {
  public:
    static constexpr int Columns = 28, Rows = 18, Capacity = Columns * Rows;
    bool contains(Cell c) const {
        return c.x >= 0 && c.y >= 0 && c.x < Columns && c.y < Rows;
    }
};
struct SnakeSegment {
    Cell cell, previous;
};
class Snake {
    SnakeSegment segments[Grid::Capacity];
    int count = 4;
    Direction heading = Direction::Right, requested = Direction::Right;
    bool turned = false;

  public:
    void reset();
    void turn(Direction);
    Cell next() const;
    bool occupies(Cell, bool includeTail = true) const;
    void advance(bool grow);
    int length() const {
        return count;
    }
    const SnakeSegment &segment(int i) const {
        return segments[i];
    }
    bool save(std::ostream &) const;
    bool load(std::istream &);
};
class Food {
  public:
    Cell cell;
    void spawn(const Snake &);
};
class Collision {
  public:
    bool hits(const Grid &g, const Snake &s, Cell n, bool grow) const {
        return !g.contains(n) || s.occupies(n, grow);
    }
};
class WordDictionary {
  public:
    static int count();
    static const char *word(int);
    static bool accepts(const std::string &);
};
class LetterGrid {
  public:
    std::string rows[6];
    int colors[6][5]{};
    int used = 0;
    void clear();
    static void evaluate(const std::string &, const std::string &, int result[5]);
};
class KeyboardDisplay {
  public:
    int states[26]{};
    void reset();
    void record(const std::string &, const int colors[5]);
};
class WordCategories {
  public:
    static constexpr int Count = 6, Words = 8;
    static const char *name(int);
    static const char *word(int, int);
    static const char *clue(int, int);
};
class WordleCategories {
  public:
    static constexpr int Count = 6;
    static const char *name(int);
    static int size(int);
    static const char *word(int, int);
    static bool contains(int, const std::string &);
};
class LetterTracker {
    bool used[26]{};

  public:
    void reset();
    bool contains(char) const;
    bool mark(char);
    void save(std::ostream &) const;
    bool load(std::istream &);
};
class HangmanFigure {
    int parts = 0;

  public:
    static constexpr int MaxMistakes = 6;
    void reset() {
        parts = 0;
    }
    int count() const {
        return parts;
    }
    bool addPart() {
        if (parts >= MaxMistakes)
            return false;
        ++parts;
        return true;
    }
    void draw(Painter &) const;
};
// Cartridges share one abstract interface. Rendering and updates dispatch virtually.
class Game {
  protected:
    int points = 0;
    bool ended = false, victory = false;

  public:
    virtual ~Game() = default;
    virtual GameId id() const = 0;
    virtual void reset() = 0;
    virtual void update(float) = 0;
    virtual void input(int) = 0;
    virtual void draw(Painter &) const = 0;
    virtual bool save(std::ostream &) const = 0;
    virtual bool load(std::istream &) = 0;
    int score() const {
        return points;
    }
    bool finished() const {
        return ended;
    }
    bool won() const {
        return victory;
    }
};
class SnakeGame final : public Game {
    Grid grid;
    Snake snake;
    Food food;
    Collision collision;
    float accumulator = 0, slowSeconds = 0, powerSeconds = 0;
    int eaten = 0, difficulty = 1;
    Cell power{-1, -1};
    bool started = false;
    void step();

  public:
    SnakeGame() {
        reset();
    }
    GameId id() const override {
        return GameId::Snake;
    }
    void reset() override;
    void update(float) override;
    void input(int) override;
    void draw(Painter &) const override;
    bool save(std::ostream &) const override;
    bool load(std::istream &) override;
    const Snake &body() const {
        return snake;
    }
    Cell meal() const {
        return food.cell;
    }
    Cell bonus() const {
        return power;
    }
    float blend() const;
    bool running() const {
        return started;
    }
    int level() const {
        return 1 + eaten / 4;
    }
    float slowed() const {
        return slowSeconds;
    }
    void setDifficulty(int d) {
        difficulty = d;
    }
};
class WordleGame final : public Game {
    int category = 0;
    std::string answer, pending, message;
    LetterGrid letters;
    KeyboardDisplay keyboard;

  public:
    WordleGame() {
        reset();
    }
    GameId id() const override {
        return GameId::Wordle;
    }
    void reset() override;
    void update(float) override {}
    void input(int) override;
    void draw(Painter &) const override;
    bool save(std::ostream &) const override;
    bool load(std::istream &) override;
    void setCategory(int c) {
        category = c >= 0 && c < WordleCategories::Count ? c : 0;
        reset();
    }
    int theme() const {
        return category;
    }
    const LetterGrid &board() const {
        return letters;
    }
    const KeyboardDisplay &keys() const {
        return keyboard;
    }
    const std::string &entry() const {
        return pending;
    }
    const std::string &notice() const {
        return message;
    }
    const std::string &solution() const {
        return answer;
    }
};
class HangmanGame final : public Game {
    int category = 0, chosen = 0;
    HangmanFigure figure;
    bool hinted = false;
    LetterTracker tracker;
    std::string answer;
    void check();

  public:
    HangmanGame() {
        reset();
    }
    GameId id() const override {
        return GameId::Hangman;
    }
    void reset() override;
    void update(float) override {}
    void input(int) override;
    void draw(Painter &) const override;
    bool save(std::ostream &) const override;
    bool load(std::istream &) override;
    void setCategory(int c) {
        category = c >= 0 && c < WordCategories::Count ? c : 0;
        reset();
    }
    int theme() const {
        return category;
    }
    int wrong() const {
        return figure.count();
    }
    bool used(char c) const {
        return tracker.contains(c);
    }
    bool hasHint() const {
        return hinted;
    }
    const char *hint() const {
        return WordCategories::clue(category, chosen);
    }
    const std::string &solution() const {
        return answer;
    }
};
class User {
  protected:
    std::string username = "PLAYER 01";

  public:
    virtual ~User() = default;
    virtual bool persistent() const = 0;
    const std::string &name() const {
        return username;
    }
    void rename(const std::string &);
};
class GuestPlayer final : public User {
  public:
    bool persistent() const override {
        return false;
    }
};
struct ScoreRecord {
    int best = 0, rounds = 0, wins = 0;
};
class Achievement {
    unsigned flags = 0;

  public:
    bool has(int b) const {
        return (flags & (1u << b)) != 0;
    }
    void unlock(int b) {
        flags |= 1u << b;
    }
    unsigned value() const {
        return flags;
    }
    void restore(unsigned v) {
        flags = v & 31u;
    }
    static const char *title(int);
    static const char *description(int);
};
class Player final : public User {
  public:
    ScoreRecord records[3];
    Achievement achievements;
    std::time_t wordleWindow = 0;
    int wordleRounds = 0;
    bool persistent() const override {
        return true;
    }
    void record(const Game &);
    int remainingWordle(std::time_t);
    bool beginWordle(std::time_t);
};
struct Settings {
    float music = 28, effects = 55, master = 80;
    bool muted = false;
    int difficulty = 1;
    bool scanlines = false;
};
class ProfileStore {
    std::string directory;

  public:
    explicit ProfileStore(std::string p = "data") : directory(p) {}
    bool save(int, const Player &, const Settings &) const;
    bool load(int, Player &, Settings &) const;
    bool saveGame(int, const Game &) const;
    bool loadGame(int, Game &) const;
};
// Non-owning aggregation: GameBoy owns all three cartridge objects directly.
class GameCollection {
    Game *cartridges[3];

  public:
    GameCollection(SnakeGame &s, WordleGame &w, HangmanGame &h) : cartridges{&s, &w, &h} {}
    Game &at(int i) {
        return *cartridges[i >= 0 && i < 3 ? i : 0];
    }
};
} // namespace pocket
