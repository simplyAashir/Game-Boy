#pragma once
#include "Model.hpp"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
namespace pocket {
namespace palette {
inline const sf::Color ink{16, 30, 22}, panel{12, 32, 22}, raised{28, 56, 36}, line{133, 103, 44};
inline const sf::Color paper{250, 235, 197}, muted{208, 191, 148}, lime{123, 196, 100},
    violet{123, 205, 207}, peach{222, 178, 74}, red{195, 74, 58}, gold{224, 187, 102},
    darkText{47, 37, 22};
} // namespace palette
class Resources {
  public:
    sf::Font regular, bold;
    sf::Texture environments, atlas, puppet;
    bool load();
};
class Painter {
    sf::RenderTarget &target;
    const Resources &resources;
    void image(const sf::Texture &, sf::IntRect, float, float, float, float,
               sf::Color tint = sf::Color::White);

  public:
    float time;
    sf::Vector2f mouse;
    int bestScore = 0, revealRow = -1;
    float revealTime = 10, reactionTime = 0;
    int reactionKind = 0;
    char reactionLetter = 0;
    Painter(sf::RenderTarget &t, const Resources &r, float clock, sf::Vector2f cursor)
        : target(t), resources(r), time(clock), mouse(cursor) {}
    void box(float, float, float, float, sf::Color, float radius = 0,
             sf::Color outline = sf::Color::Transparent, float thickness = 1);
    void circle(float, float, float, sf::Color, sf::Color outline = sf::Color::Transparent,
                float thickness = 1);
    void line(float, float, float, float, sf::Color, float thickness = 2);
    void text(const std::string &, float, float, unsigned size = 18,
              sf::Color color = palette::paper, bool bold = false);
    void center(const std::string &, float, float, unsigned size = 18,
                sf::Color color = palette::paper, bool bold = false);
    void fit(const std::string &, float, float, float, unsigned, sf::Color color = palette::paper,
             bool bold = false);
    void paragraph(const std::string &, float, float, float, unsigned, sf::Color);
    bool hover(float, float, float, float) const;
    void button(const std::string &, float, float, float, float, bool selected = false,
                sf::Color accent = palette::gold);
    void frame(float, float, float, float, bool parchment = false, float border = 28);
    void illustration(int, float, float, float, float, sf::Color tint = sf::Color::White);
    void environment(int);
    void flourish(float, float, float, sf::Color color = palette::gold);
    void diamond(float, float, float, sf::Color);
    void glow(float, float, float, sf::Color);
    void heart(float, float, float, sf::Color);
    void keyboard(const KeyboardDisplay &, float, bool hangman = false,
                  const HangmanGame *game = nullptr);
    void wanderer(float, float, float, int, sf::Color);
};
enum class Cue {
    Move,
    Select,
    Error,
    Eat,
    Back,
    Enter,
    Pause,
    Resume,
    Save,
    Achievement,
    Power,
    Expire,
    Level,
    Collision,
    Letter,
    Delete,
    Submit,
    TileExact,
    TilePresent,
    TileAbsent,
    Correct,
    Wrong,
    Repeat,
    Hint,
    Warning,
    WrongAlt1,
    WrongAlt2,
    Count
};
class InputSystem {
  public:
    int gameKey(const sf::Event &, GameId) const;
    sf::Vector2f toLogical(sf::Vector2i, sf::Vector2u) const;
    sf::Vector2f cursor(const sf::RenderWindow &, const sf::RenderTexture &) const;
};
// Owns the logical display buffer. A fixed canvas is letterboxed into every resolution.
class ScreenDisplay {
    sf::RenderTexture buffer;

  public:
    static constexpr unsigned Width = 1280, Height = 800;
    bool create();
    sf::RenderTexture &canvas() {
        return buffer;
    }
    void present(sf::RenderWindow &, bool scanlines, float fade = 0, sf::Vector2f shake = {0, 0});
};
class SoundSystem {
    sf::SoundBuffer buffers[static_cast<int>(Cue::Count)];
    sf::Sound channels[12];
    float voiceGains[12]{};
    sf::Music streams[2], sting;
    int channel = 0, current = 0, scene = -1;
    float fade = 1, master = 80, musicVolume = 28, effectVolume = 55, stingTime = 0;
    std::string audioRoot = "assets/audio";
    bool available[static_cast<int>(Cue::Count)]{};
    bool ready = false, muted = false, sleeping = false, ducked = false;

  public:
    bool load(const std::string &root = "assets/audio");
    void volumes(const Settings &);
    void play(int cue);
    void play(Cue cue) {
        play(static_cast<int>(cue));
    }
    void setScene(int);
    void update(float);
    void result(bool);
    void pause(bool);
    void duck(bool value) {
        ducked = value;
    }
    int currentScene() const {
        return scene;
    }
    float mixVolume() const {
        return muted || sleeping ? 0 : master * .01f * musicVolume;
    }
};
class MenuSystem {
  public:
    enum class Page {
        Home,
        Library,
        Playing,
        Settings,
        Profiles,
        Help,
        Achievements,
        Scores,
        Sleep
    };
    Page page = Page::Home;
    int selected = 0;
    bool paused = false;
    float transition = 0;
    void go(Page next) {
        page = next;
        transition = 0.32f;
    }
    void update(float dt) {
        transition = transition > dt ? transition - dt : 0;
    }
};
// Presentation effects advance with frame time; gameplay never blocks on a sleep call.
class GameEffects {
    float impact = 2, record = 0, reaction = 0;
    int recordTarget = 0, reactionType = 0;
    char lastLetter = 0;
    bool recordShown = false;

  public:
    void reset(int best, int score) {
        impact = 2;
        record = reaction = 0;
        recordTarget = best;
        recordShown = score > best;
        reactionType = 0;
        lastLetter = 0;
    }
    void update(float dt);
    void crash() {
        impact = 0;
    }
    bool crashing() const {
        return impact < 1;
    }
    float crashTime() const {
        return impact;
    }
    float recordTime() const {
        return record;
    }
    bool checkRecord(int score) {
        if (recordShown || score <= recordTarget)
            return false;
        recordShown = true;
        record = 2.4f;
        return true;
    }
    void react(char letter, bool correct) {
        lastLetter = letter;
        reactionType = correct ? 1 : -1;
        reaction = .8f;
    }
    float reactionTime() const {
        return reaction;
    }
    int reactionKind() const {
        return reactionType;
    }
    char reactionLetter() const {
        return lastLetter;
    }
    sf::Vector2f shake() const;
};
class GameBoy {
    sf::RenderWindow window;
    ScreenDisplay screen;
    Resources resources;
    InputSystem input;
    SoundSystem sound;
    MenuSystem menu;
    GameEffects effects;
    bool categoryOpen = false;
    int categoryChoice = 0;
    SnakeGame snake;
    WordleGame wordle;
    HangmanGame hangman;
    GameCollection games;
    Player player;
    Settings settings;
    ProfileStore store;
    int profileSlot = 0, active = -1, draggingSlider = -1;
    bool initialized[3]{}, recorded[3]{};
    bool helpReturnPlay = false, editing = false, secret = false, powered = true;
    std::string editName, toast;
    float toastSeconds = 0, elapsed = 0, autosave = 0;
    int secretIndex = 0;
    void event(const sf::Event &);
    void click(sf::Vector2f);
    void dispatch(int);
    void launch(int, bool fresh = false);
    void saveAll();
    void loadProfile(int);
    void finishRound();
    bool endingAnimation() const;
    void chooseCategory(int);
    void categories(Painter &);
    void notify(const std::string &);
    void draw(Painter &);
    void chrome(Painter &);
    void home(Painter &);
    void scoresPage(Painter &);
    void update(float);
    float revealElapsed = 10;
    int revealedRow = -1, nextTileCue = 0;
    bool audioPaused = false, outcomePending = false, pendingWin = false,
         achievementPending = false;
    void library(Painter &);
    void playing(Painter &);
    void settingsPage(Painter &);
    void profilesPage(Painter &);
    void helpPage(Painter &);
    void achievementsPage(Painter &);

  public:
    GameBoy() : games(snake, wordle, hangman) {}
#ifdef POCKET_UI_TEST
    int testFlows();
#endif
    int run(const std::string &preview = "");
};
} // namespace pocket
