#include "Frontend.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
namespace pocket {
using namespace palette;
namespace {
const char *titles[] = {"SNAKE", "WORDLE", "HANGMAN"};
bool inside(sf::Vector2f p, float x, float y, float w, float h) {
    return sf::FloatRect(x, y, w, h).contains(p);
}
} // namespace
void GameBoy::notify(const std::string &message) {
    toast = message;
    toastSeconds = 4;
}
void GameBoy::saveAll() {
    bool ok = store.save(profileSlot, player, settings);
    for (int i = 0; i < 3; ++i)
        if (initialized[i])
            ok = store.saveGame(profileSlot, games.at(i)) && ok;
    if (!ok)
        notify("Couldn't save. Check the data folder is writable.");
}
void GameBoy::loadProfile(int slot) {
    profileSlot = slot;
    player = Player{};
    settings = Settings{};
    player.rename("PLAYER 0" + std::to_string(slot + 1));
    store.load(slot, player, settings);
    for (int i = 0; i < 3; ++i) {
        games.at(i).reset();
        initialized[i] = store.loadGame(slot, games.at(i));
        recorded[i] = initialized[i] && games.at(i).finished();
    }
    snake.setDifficulty(settings.difficulty);
    active = -1;
    editing = false;
    sound.volumes(settings);
#ifndef POCKET_UI_TEST
    std::ofstream activeProfile("data/active.txt");
    if (activeProfile)
        activeProfile << slot;
#endif
}
void GameBoy::launch(int id, bool fresh) {
    if (id < 0 || id >= 3)
        return;
    Game &game = games.at(id);
    if (fresh || !initialized[id] || game.finished()) {
        if (id == 1 && !player.beginWordle(std::time(nullptr))) {
            int seconds = static_cast<int>(
                std::max<std::time_t>(0, 900 - (std::time(nullptr) - player.wordleWindow)));
            notify("More Wordle games in " + std::to_string((seconds + 59) / 60) + " min.");
            sound.play(2);
            return;
        }
        game.reset();
        initialized[id] = true;
        recorded[id] = false;
        if (id == 0)
            snake.setDifficulty(settings.difficulty);
    }
    outcomePending = false;
    achievementPending = false;
    revealedRow = -1;
    revealElapsed = 10;
    nextTileCue = 0;
    active = id;
    effects.reset(player.records[id].best, game.score());
    categoryOpen = false;
    menu.paused = false;
    menu.go(MenuSystem::Page::Playing);
    sound.play(1);
    saveAll();
}
bool GameBoy::endingAnimation() const {
    return (active == 0 && effects.crashing()) || (active == 1 && revealElapsed < 1.05f) ||
           (active == 2 && effects.reactionTime() > 0);
}
void GameBoy::chooseCategory(int category) {
    if (active == 1)
        wordle.setCategory(category);
    else if (active == 2)
        hangman.setCategory(category);
    else
        return;
    initialized[active] = true;
    recorded[active] = false;
    categoryOpen = false;
    outcomePending = achievementPending = false;
    revealedRow = -1;
    revealElapsed = 10;
    nextTileCue = 0;
    effects.reset(player.records[active].best, 0);
    toastSeconds = 0;
    sound.play(Cue::Enter);
    saveAll();
}
void GameBoy::finishRound() {
    if (active < 0)
        return;
    Game &game = games.at(active);
    if (game.finished() && !recorded[active]) {
        unsigned previous = player.achievements.value();
        player.record(game);
        recorded[active] = true;
        bool achieved = previous != player.achievements.value();
        if (endingAnimation()) {
            outcomePending = true;
            pendingWin = game.won();
            achievementPending = achieved;
        } else {
            sound.result(game.won());
            if (achieved)
                sound.play(Cue::Achievement);
        }
        if (achieved)
            notify("Achievement unlocked!");
        saveAll();
    }
}
void GameBoy::dispatch(int key) {
    if (active < 0 || menu.paused || menu.page != MenuSystem::Page::Playing)
        return;
    if ((active == 1 && revealElapsed < 1.05f) || categoryOpen ||
        (active == 0 && effects.crashing()))
        return;
    Game &game = games.at(active);
    if (active == 1) {
        int rows = wordle.board().used;
        std::string entry = wordle.entry();
        game.input(key);
        if (wordle.board().used > rows) {
            revealedRow = rows;
            revealElapsed = 0;
            nextTileCue = 0;
            sound.play(Cue::Submit);
        } else if (key == 13) {
            sound.play(Cue::Error);
            toastSeconds = 0; // The persistent Wordle notice shows validation beside the board.
        } else if (wordle.entry() != entry)
            sound.play(key == 8 ? Cue::Delete : Cue::Letter);
    } else if (active == 2) {
        int wrong = hangman.wrong();
        bool hint = hangman.hasHint();
        char letter = static_cast<char>(key >= 'a' && key <= 'z' ? key - 32 : key);
        bool repeat = key >= 0 && key < 128 && hangman.used(letter);
        game.input(key);
        if (!hint && hangman.hasHint()) {
            effects.react(0, true);
            sound.play(Cue::Hint);
        } else if (repeat)
            sound.play(Cue::Repeat);
        else if (hangman.wrong() > wrong) {
            effects.react(letter, false);
            sound.play(Cue::Wrong);
            if (hangman.wrong() == 5)
                sound.play(Cue::Warning);
        } else if (!repeat && letter >= 'A' && letter <= 'Z') {
            effects.react(letter, true);
            sound.play(Cue::Correct);
        }
    } else
        game.input(key);
    finishRound();
    if (active != 0)
        saveAll();
}
void GameBoy::update(float dt) {
    elapsed += dt;
    menu.update(dt);
    toastSeconds = std::max(0.f, toastSeconds - dt);
    if (menu.page == MenuSystem::Page::Playing && !menu.paused) {
        effects.update(dt);
        int before = games.at(active).score(), level = snake.level();
        float slow = snake.slowed();
        revealElapsed += dt;
        games.at(active).update(dt);
        if (active == 0) {
            if (snake.score() > before)
                sound.play(snake.slowed() > slow ? Cue::Power : Cue::Eat);
            if (snake.level() > level)
                sound.play(Cue::Level);
            if (slow > 0 && snake.slowed() == 0)
                sound.play(Cue::Expire);
            if (snake.score() > before && effects.checkRecord(snake.score()))
                sound.play(Cue::Achievement);
            if (snake.score() > player.records[0].best) {
                player.records[0].best = snake.score();
                saveAll(); // Preserve a live record even if the player restarts before a collision.
            }
            if (snake.finished() && !recorded[0] && !snake.won()) {
                effects.crash();
                sound.play(Cue::Collision);
            }
        }
        if (active == 1 && revealedRow >= 0) {
            while (nextTileCue < 5 && revealElapsed >= nextTileCue * .13f + .14f) {
                int state = wordle.board().colors[revealedRow][nextTileCue++];
                sound.play(state == 3   ? Cue::TileExact
                           : state == 2 ? Cue::TilePresent
                                        : Cue::TileAbsent);
            }
        }
        if (outcomePending && !endingAnimation()) {
            sound.result(pendingWin);
            if (achievementPending)
                sound.play(Cue::Achievement);
            outcomePending = achievementPending = false;
        }
        finishRound();
    }
    sound.setScene(menu.page == MenuSystem::Page::Playing ? active + 1 : 0);
    bool duck = menu.page == MenuSystem::Page::Playing && (menu.paused || effects.crashing());
    if (duck != audioPaused && menu.page == MenuSystem::Page::Playing)
        sound.play(duck ? Cue::Pause : Cue::Resume);
    audioPaused = duck;
    sound.duck(duck);
    sound.update(dt);
    autosave += dt;
    if (autosave >= 3) {
        autosave = 0;
        saveAll();
    }
}
void GameBoy::event(const sf::Event &e) {
    if (e.type == sf::Event::Closed) {
        saveAll();
        window.close();
        return;
    }
    if (e.type == sf::Event::LostFocus && menu.page == MenuSystem::Page::Playing &&
        !games.at(active).finished()) {
        menu.paused = true;
        categoryOpen = false;
    }
    if (e.type == sf::Event::Resized) {
        if (e.size.width < 640 || e.size.height < 400)
            window.setSize({std::max(640u, e.size.width), std::max(400u, e.size.height)});
    }
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        draggingSlider = -1;
        click(input.toLogical({e.mouseButton.x, e.mouseButton.y}, window.getSize()));
        return;
    }
    if (e.type == sf::Event::MouseMoved && draggingSlider >= 0 &&
        menu.page == MenuSystem::Page::Settings) {
        auto point = input.toLogical({e.mouseMove.x, e.mouseMove.y}, window.getSize());
        float volume = std::clamp((point.x - 548) / 486 * 100, 0.f, 100.f);
        if (draggingSlider == 0)
            settings.music = volume;
        else if (draggingSlider == 1)
            settings.effects = volume;
        else
            settings.master = volume;
        sound.volumes(settings);
        return;
    }
    if (e.type == sf::Event::MouseMoved && draggingSlider < 0) {
        auto point = input.toLogical({e.mouseMove.x, e.mouseMove.y}, window.getSize());
        int previous = menu.selected;
        if (menu.page == MenuSystem::Page::Home)
            for (int i = 0; i < 7; ++i)
                if (inside(point, 125, 254 + i * 55.f, 278, 42))
                    menu.selected = i;
        if (menu.page == MenuSystem::Page::Library)
            for (int i = 0; i < 3; ++i)
                if (inside(point, 64 + i * 392.f, 302, 368, 400))
                    menu.selected = i;
        if (previous != menu.selected)
            sound.play(Cue::Move);
    }
    if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left &&
        draggingSlider >= 0) {
        draggingSlider = -1;
        saveAll();
        return;
    }
    if (categoryOpen && menu.page == MenuSystem::Page::Playing) {
        if (e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::Up)
                categoryChoice = (categoryChoice + 5) % 6;
            else if (e.key.code == sf::Keyboard::Down)
                categoryChoice = (categoryChoice + 1) % 6;
            else if (e.key.code == sf::Keyboard::Enter)
                chooseCategory(categoryChoice);
            else if (e.key.code == sf::Keyboard::Escape || e.key.code == sf::Keyboard::Tab)
                categoryOpen = false;
        }
        return;
    }
    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Tab &&
        menu.page == MenuSystem::Page::Playing && active > 0 && !menu.paused &&
        !games.at(active).finished() && !endingAnimation()) {
        categoryOpen = true;
        categoryChoice = active == 1 ? wordle.theme() : hangman.theme();
        return;
    }
    if (editing) {
        if (e.type == sf::Event::TextEntered && e.text.unicode >= 32 && e.text.unicode < 127 &&
            editName.size() < 16)
            editName += static_cast<char>(e.text.unicode);
        if (e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::Backspace && !editName.empty())
                editName.pop_back();
            if (e.key.code == sf::Keyboard::Enter) {
                player.rename(editName);
                editing = false;
                saveAll();
                notify("Name saved.");
            }
            if (e.key.code == sf::Keyboard::Escape)
                editing = false;
        }
        return;
    }
    if (e.type == sf::Event::KeyPressed && menu.page == MenuSystem::Page::Home) {
        if (e.key.code == sf::Keyboard::Up || e.key.code == sf::Keyboard::Down)
            sound.play(Cue::Move);
        if (e.key.code == sf::Keyboard::Up)
            menu.selected = (menu.selected + 6) % 7;
        else if (e.key.code == sf::Keyboard::Down)
            menu.selected = (menu.selected + 1) % 7;
        else if (e.key.code == sf::Keyboard::Enter) {
            click({260, 270 + menu.selected * 55.f});
            return;
        } else if (e.key.code >= sf::Keyboard::Num1 && e.key.code <= sf::Keyboard::Num3) {
            launch(e.key.code - sf::Keyboard::Num1);
            return;
        }
    }
    if (e.type == sf::Event::KeyPressed) {
        static const sf::Keyboard::Key secretKeys[] = {
            sf::Keyboard::Up,   sf::Keyboard::Up,    sf::Keyboard::Down, sf::Keyboard::Down,
            sf::Keyboard::Left, sf::Keyboard::Right, sf::Keyboard::Left, sf::Keyboard::Right,
            sf::Keyboard::B,    sf::Keyboard::A};
        if (menu.page == MenuSystem::Page::Library) {
            if (e.key.code == secretKeys[secretIndex]) {
                if (++secretIndex == 10) {
                    secret = !secret;
                    secretIndex = 0;
                    notify("MIDNIGHT MODE. A little secret between us.");
                    sound.play(1);
                }
            } else
                secretIndex = e.key.code == secretKeys[0] ? 1 : 0;
        }
        if (menu.page == MenuSystem::Page::Sleep) {
            menu.go(MenuSystem::Page::Home);
            menu.selected = 0;
            sound.pause(false);
            sound.play(Cue::Resume);
            return;
        }
        if (e.key.code == sf::Keyboard::Escape) {
            if (menu.page == MenuSystem::Page::Playing) {
                if (menu.paused || games.at(active).finished()) {
                    saveAll();
                    menu.go(MenuSystem::Page::Library);
                    menu.paused = false;
                } else
                    menu.paused = true;
            } else if (menu.page == MenuSystem::Page::Help && helpReturnPlay) {
                menu.go(MenuSystem::Page::Playing);
                menu.paused = true;
                helpReturnPlay = false;
            } else if (menu.page == MenuSystem::Page::Library) {
                menu.selected = 0;
                menu.go(MenuSystem::Page::Home);
            } else
                menu.go(MenuSystem::Page::Library);
            return;
        }
        if (menu.page == MenuSystem::Page::Library) {
            if (e.key.code == sf::Keyboard::Right || e.key.code == sf::Keyboard::D) {
                menu.selected = (menu.selected + 1) % 3;
                sound.play(0);
            }
            if (e.key.code == sf::Keyboard::Left || e.key.code == sf::Keyboard::A) {
                menu.selected = (menu.selected + 2) % 3;
                sound.play(0);
            }
            if (e.key.code == sf::Keyboard::Enter)
                launch(menu.selected);
            if (e.key.code >= sf::Keyboard::Num1 && e.key.code <= sf::Keyboard::Num3)
                launch(e.key.code - sf::Keyboard::Num1);
        } else if (menu.page == MenuSystem::Page::Playing) {
            if (e.key.code == sf::Keyboard::F2) {
                helpReturnPlay = true;
                menu.paused = true;
                menu.go(MenuSystem::Page::Help);
                return;
            }
            if (e.key.code == sf::Keyboard::P && active == 0 && !games.at(active).finished()) {
                menu.paused = !menu.paused;
                saveAll();
                return;
            }
            if (menu.paused) {
                if (e.key.code == sf::Keyboard::Enter)
                    menu.paused = false;
                if (e.key.code == sf::Keyboard::R)
                    launch(active, true);
                return;
            }
            if (games.at(active).finished()) {
                if (endingAnimation())
                    return;
                if (e.key.code == sf::Keyboard::Enter)
                    launch(active, true);
                return;
            }
        }
    }
    if (menu.page == MenuSystem::Page::Playing && !menu.paused && !games.at(active).finished()) {
        int key = input.gameKey(e, games.at(active).id());
        if (key >= 0)
            dispatch(key);
    }
}
void GameBoy::click(sf::Vector2f pos) {
    using Page = MenuSystem::Page;
    if (menu.page == Page::Sleep) {
        menu.go(Page::Home);
        menu.selected = 0;
        sound.pause(false);
        sound.play(Cue::Resume);
        return;
    }
    if (inside(pos, 28, 27, 360, 69)) {
        saveAll();
        menu.selected = 0;
        menu.go(Page::Home);
        sound.play(Cue::Back);
        return;
    }
    if (inside(pos, 550, 47, 90, 36)) {
        saveAll();
        menu.selected = std::clamp(menu.selected, 0, 2);
        menu.go(Page::Library);
        menu.paused = false;
        sound.play(Cue::Select);
        return;
    }
    if (inside(pos, 650, 47, 90, 36)) {
        saveAll();
        menu.go(Page::Achievements);
        sound.play(Cue::Select);
        return;
    }
    if (inside(pos, 750, 47, 90, 36)) {
        saveAll();
        menu.go(Page::Settings);
        sound.play(Cue::Select);
        return;
    }
    if (inside(pos, 850, 47, 90, 36)) {
        helpReturnPlay = menu.page == Page::Playing;
        menu.go(Page::Help);
        sound.play(Cue::Enter);
        return;
    }
    if (inside(pos, 970, 42, 230, 46)) {
        saveAll();
        menu.go(Page::Profiles);
        sound.play(Cue::Select);
        return;
    }
    if (inside(pos, 1150, 739, 65, 30)) {
        saveAll();
        menu.go(Page::Sleep);
        sound.pause(true);
        return;
    }
    if (menu.page == Page::Home) {
        for (int i = 0; i < 7; ++i)
            if (inside(pos, 125, 254 + i * 55, 278, 42)) {
                sound.play(Cue::Select);
                switch (i) {
                case 0:
                    menu.selected = 0;
                    menu.go(Page::Library);
                    break;
                case 1:
                    menu.go(Page::Profiles);
                    break;
                case 2:
                    menu.go(Page::Scores);
                    break;
                case 3:
                    menu.go(Page::Achievements);
                    break;
                case 4:
                    menu.go(Page::Settings);
                    break;
                case 5:
                    helpReturnPlay = false;
                    menu.go(Page::Help);
                    break;
                case 6:
                    saveAll();
                    window.close();
                    break;
                }
                return;
            }
    } else if (menu.page == Page::Library) {
        for (int i = 0; i < 3; ++i)
            if (inside(pos, 64 + i * 392, 302, 368, 405)) {
                menu.selected = i;
                launch(i);
                return;
            }
        if (inside(pos, 1002, 687, 196, 34)) {
            menu.go(Page::Help);
            helpReturnPlay = false;
            sound.play(Cue::Enter);
        }
    } else if (menu.page == Page::Playing) {
        Game &game = games.at(active);
        if (inside(pos, 1080, 150, 120, 40)) {
            saveAll();
            menu.go(Page::Library);
            sound.play(Cue::Back);
            return;
        }
        if (menu.paused) {
            if (inside(pos, 462, 435, 356, 42))
                menu.paused = false;
            else if (inside(pos, 462, 493, 356, 42))
                launch(active, true);
            else if (inside(pos, 462, 551, 356, 42)) {
                saveAll();
                menu.go(Page::Library);
                menu.paused = false;
            }
            return;
        }
        if (active > 0 && !game.finished() && !endingAnimation()) {
            float cx = active == 1 ? 65.f : 943.f, cy = active == 1 ? 199.f : 197.f;
            float cw = active == 1 ? 270.f : 257.f, ch = active == 1 ? 42.f : 30.f;
            if (categoryOpen) {
                for (int i = 0; i < 6; ++i)
                    if (inside(pos, cx, cy + ch + 4 + i * 36, cw, 34)) {
                        chooseCategory(i);
                        return;
                    }
                categoryOpen = false;
                return;
            }
            if (inside(pos, cx, cy, cw, ch)) {
                categoryOpen = true;
                categoryChoice = active == 1 ? wordle.theme() : hangman.theme();
                return;
            }
        }
        if (game.finished()) {
            if (endingAnimation())
                return;
            if (inside(pos, 462, 466, 356, 42))
                launch(active, true);
            else if (inside(pos, 462, 522, 356, 42))
                menu.go(Page::Library);
            return;
        }
        if (inside(pos, 943, 150, 120, 40)) {
            menu.paused = true;
            saveAll();
            return;
        }
        if (active == 2) {
            if (inside(pos, 544, 518, 245, 40)) {
                dispatch(1004);
                return;
            }
        }
        if (active == 1 || active == 2) {
            float y = active == 1 ? 600.f : 589.f, cx = active == 2 ? 863.f : 640.f;
            const char *rows[] = {"QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};
            for (int row = 0; row < 3; ++row) {
                int count = static_cast<int>(std::char_traits<char>::length(rows[row]));
                float start = cx - count * 60 / 2.f;
                for (int col = 0; col < count; ++col)
                    if (inside(pos, start + col * 60, y + row * 44, 54, 37)) {
                        dispatch(rows[row][col]);
                        return;
                    }
            }
            if (active == 1 && inside(pos, 880, 557, 140, 38))
                dispatch(13);
            if (active == 1 && inside(pos, 1030, 557, 164, 38))
                dispatch(8);
        }
    } else if (menu.page == Page::Settings) {
        if (inside(pos, 944, 166, 240, 40)) {
            if (settings.muted || settings.master == 0 || settings.effects == 0)
                notify("Turn sound on and raise Effects and Master Volume.");
            else {
                sound.play(Cue::Power);
                notify("Sound test: a short chime.");
            }
            return;
        }
        if (inside(pos, 538, 286, 506, 28)) {
            draggingSlider = 0;
            settings.music = std::clamp((pos.x - 548) / 486 * 100, 0.f, 100.f);
        }
        if (inside(pos, 538, 376, 506, 28)) {
            draggingSlider = 1;
            settings.effects = std::clamp((pos.x - 548) / 486 * 100, 0.f, 100.f);
        }
        if (inside(pos, 538, 466, 506, 28)) {
            draggingSlider = 2;
            settings.master = std::clamp((pos.x - 548) / 486 * 100, 0.f, 100.f);
        }
        for (int i = 0; i < 3; ++i)
            if (inside(pos, 548 + i * 165, 547, 153, 38)) {
                settings.difficulty = i;
                notify("Speed applies to your next Snake game.");
            }
        if (inside(pos, 192, 623, 200, 40))
            settings.muted = !settings.muted;
        if (inside(pos, 548, 623, 205, 40))
            settings.scanlines = !settings.scanlines;
        if (inside(pos, 825, 623, 205, 40))
            settings = Settings{};
        sound.volumes(settings);
        sound.play(Cue::Move);
        saveAll();
    } else if (menu.page == Page::Profiles) {
        for (int i = 0; i < 3; ++i)
            if (inside(pos, 72 + i * 390, 260, 366, 220)) {
                saveAll();
                loadProfile(i);
                saveAll();
                notify("Welcome, " + player.name() + ".");
                sound.play(Cue::Select);
                return;
            }
        if (inside(pos, 540, 537, 420, 53)) {
            editing = true;
            editName = player.name();
        }
    }
}
void GameBoy::chrome(Painter &p) {
    p.box(0, 0, 1280, 112, sf::Color(4, 20, 13, 210));
    p.frame(28, 27, 360, 69, false, 20);
    p.center("Fantasy Quest", 208, 61, 23, gold, true);
    const char *nav[] = {"Games", "Awards", "Settings", "Help"};
    for (int i = 0; i < 4; ++i) {
        bool current = (i == 0 && menu.page == MenuSystem::Page::Library) ||
                       (i == 1 && menu.page == MenuSystem::Page::Achievements) ||
                       (i == 2 && menu.page == MenuSystem::Page::Settings) ||
                       (i == 3 && menu.page == MenuSystem::Page::Help);
        p.button(nav[i], 550 + i * 100, 47, 90, 36, current);
    }
    p.button(player.name(), 970, 42, 230, 46, menu.page == MenuSystem::Page::Profiles);
    p.flourish(640, 108, 1168);
    p.box(0, 732, 1280, 68, sf::Color(4, 20, 13, 210));
    p.flourish(640, 731, 1168);
    p.text("Copyright 2026 Aashir Sheraz. All rights reserved.", 63, 750, 15, muted);

    p.button("Rest", 1150, 739, 65, 30);
    p.glow(1133, 754, 3, lime);
}
void GameBoy::home(Painter &p) {
    p.frame(467, 130, 730, 112, false, 29);
    p.center("GameBox", 832, 184, 49, gold, true);

    p.frame(87, 217, 354, 468, false, 31);
    const char *options[] = {"Play",     "Profiles", "Scores", "Achievements",
                             "Settings", "Help",     "Exit"};
    for (int i = 0; i < 7; ++i)
        p.button(options[i], 125, 254 + i * 55.f, 278, 42, menu.selected == i);
    p.frame(814, 591, 380, 102, true, 24);
    p.center("Welcome, " + player.name(), 1004, 641, 21, darkText);

    p.center("Arrows to choose. Enter to select.", 692, 699, 16, paper);
}
void GameBoy::library(Painter &p) {
    p.frame(359, 129, 562, 96, false, 27);
    p.center("Choose a Game", 640, 174, 34, gold, true);

    for (int i = 0; i < 3; ++i) {
        float x = 64 + i * 392.f;
        bool hot = menu.selected == i;
        p.frame(x, 302, 368, 400, false, 26);
        p.illustration(i, x + 26, 327, 316, 222);
        p.box(x + 26, 513, 316, 46, sf::Color(6, 26, 17, 225));
        p.center(titles[i], x + 184, 531, 23, gold, true);

        p.button(initialized[i] && !games.at(i).finished() ? "Continue" : "Play", x + 83, 632, 202,
                 39, hot);
        if (hot) {
            p.box(x + 4, 306, 360, 392, sf::Color::Transparent, 5, paper, 3);
            p.box(x + 9, 311, 350, 382, sf::Color::Transparent, 3, gold, 2);
            p.glow(x + 184, 313, 8, sf::Color(244, 200, 80, 120));
            p.diamond(x + 184, 313, 5, gold);
        }
    }
    p.text("Left / Right to choose. Enter to play.", 65, 708, 13, paper);
}
void GameBoy::playing(Painter &p) {
    Game &game = games.at(active);
    p.bestScore = player.records[active].best;
    p.revealRow = revealedRow;
    p.revealTime = revealElapsed;
    p.reactionTime = effects.reactionTime();
    p.reactionKind = effects.reactionKind();
    p.reactionLetter = effects.reactionLetter();
    const char *names[] = {"Snake", "Wordle", "Hangman"};
    p.frame(350, 126, 575, active == 1 ? 73 : 103, false, active == 1 ? 18 : 24);
    p.center(names[active], 637, 165, active == 2 ? 31 : 42, gold, true);

    p.button("Pause", 943, 150, 120, 40);
    p.button("< Games", 1080, 150, 120, 40);
    game.draw(p);
    if (active == 1) {
        p.button("Enter", 880, 557, 140, 38);
        p.button("Backspace", 1030, 557, 164, 38);
    }
    if (active > 0 && !menu.paused && !game.finished())
        categories(p);
    if (active == 0 && effects.recordTime() > 0 && !snake.finished()) {
        float age = 2.4f - effects.recordTime();
        float bounce = std::sin(std::min(1.f, age * 4) * 3.14159265f) * 9;
        p.frame(821, 245 - bounce, 383, 70, false, 18);
        p.center("NEW HIGH SCORE!", 1012, 278 - bounce, 23, gold, true);
        for (int i = 0; i < 28; ++i) {
            float phase = std::fmod(age * .65f + i * .137f, 1.f);
            float x = 100 + std::fmod(i * 97.f, 650.f);
            p.diamond(x + std::sin(i + age * 3) * 18, 272 + phase * 400, 2 + i % 3,
                      i % 2 ? gold : lime);
        }
    }
    if (active == 0 && effects.crashing()) {
        float t = effects.crashTime();
        int darkness = t > .78f ? 255 : static_cast<int>(110 + 100 * std::abs(std::sin(t * 24)));
        p.box(0, 0, 1280, 732, sf::Color(0, 0, 0, static_cast<sf::Uint8>(darkness)));
        p.box(0, 112, 1280, 100 * t, sf::Color::Black);
        p.box(0, 732 - 100 * t, 1280, 100 * t, sf::Color::Black);
        if (t < .78f) {
            p.center("CRASH!", 640, 382, 52, t < .12f ? red : gold, true);
            p.line(440 + t * 220, 435, 840 - t * 220, 435, gold, 3);
        }
    }
    bool resultReady = game.finished() && !endingAnimation();
    if (menu.paused || resultReady) {
        p.box(0, 112, 1280, 620, sf::Color(3, 14, 9, 180));
        p.frame(410, 240, 460, 385, false, 32);
        p.flourish(640, 302, 304);
        if (menu.paused) {
            p.center("Game Paused", 640, 340, 27, gold, true);

            p.button("Resume  /  Enter", 462, 435, 356, 42, true);
            p.button("Restart  /  R", 462, 493, 356, 42);
            p.button("Back to Games", 462, 551, 356, 42);
        } else {
            p.center(game.won() ? "You Win!" : "Game Over", 640, 338, 25, gold, true);
            p.center("Score: " + std::to_string(game.score()), 640, 382, 24, paper);
            if (active == 1)
                p.center("The word was " + wordle.solution(), 640, 422, 18, muted);
            else if (active == 2)
                p.center("The word was " + hangman.solution(), 640, 422, 18, muted);

            p.button("Play Again  /  Enter", 462, 466, 356, 42, true);
            p.button("Back to Games", 462, 522, 356, 42);
            if (game.won())
                for (int i = 0; i < 12; ++i)
                    p.glow(435 + std::fmod(i * 79.f + elapsed * 17, 410.f),
                           267 + std::fmod(i * 53.f - elapsed * 20 + 1200, 310.f), 3,
                           sf::Color(248, 206, 89, 120));
        }
    }
}
void GameBoy::categories(Painter &p) {
    bool word = active == 1;
    float x = word ? 65.f : 943.f, y = word ? 199.f : 197.f;
    float w = word ? 270.f : 257.f, h = word ? 42.f : 30.f;
    int selected = word ? wordle.theme() : hangman.theme();
    std::string label = word ? WordleCategories::name(selected) : WordCategories::name(selected);
    p.button("Category: " + label + "  v", x, y, w, h, categoryOpen);
    if (!categoryOpen)
        return;
    p.box(x - 5, y + h, w + 10, 224, ink, 4, gold, 2);
    for (int i = 0; i < 6; ++i) {
        std::string name = word ? WordleCategories::name(i) : WordCategories::name(i);
        p.button(name, x, y + h + 4 + i * 36, w, 34, categoryChoice == i);
    }
}
void GameBoy::settingsPage(Painter &p) {
    p.frame(375, 127, 530, 92);
    p.center("Settings", 640, 168, 34, gold, true);
    p.button("Test Sound", 944, 166, 240, 40);
    p.frame(140, 235, 1000, 468, true, 31);
    const char *labels[] = {"Music", "Sound Effects", "Master Volume"};
    float values[] = {settings.music, settings.effects, settings.master};
    for (int i = 0; i < 3; ++i) {
        float y = 280 + i * 90.f;
        p.text(labels[i], 192, y, 22, darkText);
        p.box(548, y + 12, 486, 6, sf::Color(119, 103, 61), 1);
        p.box(548, y + 12, std::max(1.f, 486 * values[i] / 100), 6, sf::Color(50, 89, 48), 1);
        p.circle(548 + 486 * values[i] / 100, y + 15, 8, gold, line, 1);
        p.text(std::to_string(static_cast<int>(values[i])) + "%", 1060, y, 19, darkText);
    }
    p.text("Snake Speed", 192, 550, 20, darkText);
    const char *pace[] = {"Easy", "Normal", "Hard"};
    for (int i = 0; i < 3; ++i)
        p.button(pace[i], 548 + i * 165, 547, 153, 38, settings.difficulty == i);
    p.button(settings.muted ? "Sound: Muted" : "Sound: On", 192, 623, 200, 40, settings.muted);
    p.button(settings.scanlines ? "Film Grain: On" : "Film Grain: Off", 548, 623, 205, 40,
             settings.scanlines);
    p.button("Reset Defaults", 825, 623, 205, 40);
}
void GameBoy::profilesPage(Painter &p) {
    p.frame(310, 128, 660, 94);
    p.center("Player Profiles", 640, 171, 29, gold, true);
    for (int i = 0; i < 3; ++i) {
        Player other;
        Settings options;
        other.rename("PLAYER 0" + std::to_string(i + 1));
        store.load(i, other, options);
        if (i == profileSlot)
            other = player;
        float x = 72 + i * 390.f;
        p.frame(x, 260, 366, 220, true, 27);
        p.center("Profile " + std::to_string(i + 1), x + 183, 301, 18, darkText, true);
        p.center(other.name(), x + 183, 360, 25, darkText);
        int rounds = 0;
        for (auto &r : other.records)
            rounds += r.rounds;
        p.center(std::to_string(rounds) + " games played", x + 183, 410, 18, darkText);
        if (i == profileSlot)
            p.center("Selected", x + 183, 448, 14, line);
    }
    p.frame(140, 517, 1000, 144, true, 26);
    p.text("Player Name", 188, 546, 20, darkText);
    p.box(540, 537, 420, 53, panel, 2, editing ? gold : line, 2);
    p.fit(editing ? editName + (std::fmod(elapsed, 1.f) < .5f ? "|" : "") : player.name(), 558, 549,
          380, 22, paper);
    p.center(editing ? "Enter to save / Escape to cancel" : "Click your name to edit.", 640, 623,
             17, darkText);
}
void GameBoy::helpPage(Painter &p) {
    p.frame(350, 128, 580, 94);
    p.center("How to Play", 640, 171, 27, gold, true);
    const char *text[3][5] = {{"Snake", "Arrows or WASD to steer.", "Collect fruit to grow.",
                               "Blue crystals slow the snake.", "P or Escape to pause."},
                              {"Wordle", "Type any five letters, then Enter.",
                               "Green: exact. Gold: elsewhere.", "Black: letter not in the word.",
                               "Tab opens word categories."},
                              {"Hangman", "Type or click a letter.", "You have six lives.",
                               "F1 reveals one letter and clue.", "Tab opens word categories."}};
    for (int i = 0; i < 3; ++i) {
        float x = 64 + i * 392.f;
        p.frame(x, 261, 368, 365, true, 28);
        p.center(titles[i], x + 184, 306, 25, darkText, true);
        p.center(text[i][0], x + 184, 353, 17, darkText);
        p.flourish(x + 184, 386, 270, line);
        for (int j = 1; j < 5; ++j)
            p.fit(text[i][j], x + 34, 405 + (j - 1) * 43.f, 300, 17, darkText);
    }
    p.frame(197, 653, 886, 65);
    p.center("Escape pauses. Escape again returns. F2 opens this guide. Your progress is saved.",
             640, 686, 16, paper);
}
void GameBoy::achievementsPage(Painter &p) {
    p.frame(350, 128, 580, 94);
    p.center("Achievements", 640, 171, 28, gold, true);
    for (int i = 0; i < 5; ++i) {
        float x = 64 + (i % 3) * 392.f, y = i < 3 ? 259.f : 462.f;
        bool earned = player.achievements.has(i);
        p.frame(x, y, 368, 175, false, 24);
        p.illustration(3, x + 24, y + 30, 80, 84,
                       earned ? sf::Color::White : sf::Color(105, 105, 92));
        p.fit(Achievement::title(i), x + 129, y + 34, 207, 16, earned ? gold : muted, true);
        p.text(earned ? "Unlocked" : "Locked", x + 129, y + 69, 14, muted);
        p.fit(Achievement::description(i), x + 32, y + 119, 305, 16, paper);
    }
    p.frame(848, 462, 368, 175, true, 24);
    p.center("Best Scores", 1032, 499, 20, darkText, true);
    for (int i = 0; i < 3; ++i) {
        p.text(titles[i], 879, 526 + i * 27.f, 14, darkText);
        p.text(std::to_string(player.records[i].best), 1128, 526 + i * 27.f, 17, darkText);
    }
}
void GameBoy::scoresPage(Painter &p) {
    p.frame(350, 128, 580, 94);
    p.center("Scores", 640, 171, 27, gold, true);
    p.frame(145, 250, 990, 414, true, 30);
    p.center(player.name() + "'s Scores", 640, 294, 26, darkText);
    const char *headers[] = {"Game", "Best Score", "Played", "Wins"};
    const float xs[] = {219, 576, 800, 1001};
    for (int i = 0; i < 4; ++i)
        p.text(headers[i], xs[i], 350, 18, darkText, true);
    p.flourish(640, 394, 800, line);
    for (int i = 0; i < 3; ++i) {
        p.text(titles[i], 219, 427 + i * 62.f, 22, darkText, true);
        p.text(std::to_string(player.records[i].best), 576, 427 + i * 62.f, 23, darkText);
        p.text(std::to_string(player.records[i].rounds), 800, 427 + i * 62.f, 23, darkText);
        p.text(std::to_string(player.records[i].wins), 1001, 427 + i * 62.f, 23, darkText);
    }
}
void GameBoy::draw(Painter &p) {
    using Page = MenuSystem::Page;
    int scene = menu.page == Page::Playing ? active + 1 : menu.page == Page::Help ? 2 : 0;
    p.environment(scene);
    chrome(p);
    switch (menu.page) {
    case Page::Home:
        home(p);
        break;
    case Page::Library:
        library(p);
        break;
    case Page::Playing:
        playing(p);
        break;
    case Page::Settings:
        settingsPage(p);
        break;
    case Page::Profiles:
        profilesPage(p);
        break;
    case Page::Help:
        helpPage(p);
        break;
    case Page::Achievements:
        achievementsPage(p);
        break;
    case Page::Scores:
        scoresPage(p);
        break;
    case Page::Sleep:
        p.box(0, 112, 1280, 620, sf::Color(1, 12, 7, 190));
        p.frame(350, 275, 580, 250);
        p.center("Sleeping", 640, 360, 31, gold, true);
        p.center("Press any key or click to wake.", 640, 436, 18, paper);
        break;
    }
    if (secret)
        for (int i = 0; i < 6; ++i)
            p.glow(120 + i * 207.f, 143 + std::sin(elapsed + i) * 10, 5, violet);
    if (toastSeconds > 0) {
        if (menu.page == Page::Playing && active == 1) {
            if (toast != wordle.notice()) {
                p.frame(65, 251, 270, 70, false, 12);
                p.paragraph(toast, 88, 268, 225, 15, gold);
            }
        } else {
            p.frame(300, 678, 680, 46, false, 12);
            p.center(toast, 640, 701, 14, gold);
        }
    }
}
int GameBoy::run(const std::string &preview) {
    if (!resources.load()) {
        std::cerr << "Missing fantasy assets. Restore the assets folder beside the game.\n";
        return 1;
    }
    auto desktop = sf::VideoMode::getDesktopMode();
    unsigned w = std::min(1280u, desktop.width - 100), h = std::min(800u, desktop.height - 100);
    window.create(sf::VideoMode(w, h), "GameBox | Fantasy Quest", sf::Style::Default);
    window.setVerticalSyncEnabled(true);
    window.setKeyRepeatEnabled(false);
    if (!screen.create()) {
        std::cerr << "Could not create the display buffer.\n";
        return 1;
    }
    std::filesystem::create_directories("data");
    int slot = 0;
    std::ifstream previous("data/active.txt");
    if (previous)
        previous >> slot;
    if (preview.empty()) {
        if (!sound.load())
            notify("Some sound effects are unavailable.");
    }
#ifdef POCKET_UI_TEST
    if (preview == "selftest")
        return testFlows();
#endif
    loadProfile(slot >= 0 && slot < 3 ? slot : 0);
    sound.volumes(settings);
    if (preview.empty())
        sound.play(Cue::Enter);
    if (!preview.empty()) {
        if (preview == "snake" || preview == "wordle" || preview == "hangman" ||
            preview == "pause" || preview == "result") {
            active = preview == "wordle" ? 1 : preview == "hangman" ? 2 : 0;
            games.at(active).reset();
            menu.page = MenuSystem::Page::Playing;
            if (preview == "snake") {
                snake.input(13);
                for (int i = 0; i < 5; ++i)
                    snake.update(.16f);
            }
            if (preview == "wordle") {
                for (char c : std::string("CRANE"))
                    wordle.input(c);
                wordle.input(13);
                for (char c : std::string("SPO"))
                    wordle.input(c);
            }
            if (preview == "hangman") {
                hangman.input('Z');
                hangman.input('X');
                hangman.input('Q');
                hangman.input(1004);
            }
            if (preview == "pause")
                menu.paused = true;
            if (preview == "result") {
                snake.input(13);
                for (int i = 0; i < 100; ++i)
                    snake.update(.2f);
            }
        } else if (preview == "library")
            menu.page = MenuSystem::Page::Library;
        else if (preview == "home")
            menu.page = MenuSystem::Page::Home;
        else if (preview == "scores")
            menu.page = MenuSystem::Page::Scores;
        else if (preview == "sleep")
            menu.page = MenuSystem::Page::Sleep;
        else if (preview == "settings")
            menu.page = MenuSystem::Page::Settings;
        else if (preview == "profiles")
            menu.page = MenuSystem::Page::Profiles;
        else if (preview == "help")
            menu.page = MenuSystem::Page::Help;
        else if (preview == "patches")
            menu.page = MenuSystem::Page::Achievements;
        Painter p(screen.canvas(), resources, 1.5f, {-1, -1});
        draw(p);
        screen.present(window, settings.scanlines);
        std::filesystem::create_directories("build");
        const bool saved = screen.canvas().getTexture().copyToImage().saveToFile("build/preview-" +
                                                                                 preview + ".png");
        window.close();
        return saved ? 0 : 1;
    }
    sf::Clock clock;
    while (window.isOpen()) {
        float dt = std::min(.1f, clock.restart().asSeconds());

        sf::Event e;
        while (window.pollEvent(e))
            event(e);
        if (!window.isOpen())
            break;
        update(dt);
        Painter painter(screen.canvas(), resources, elapsed, input.cursor(window, screen.canvas()));
        draw(painter);
        screen.present(window, settings.scanlines, menu.transition,
                       menu.page == MenuSystem::Page::Playing && active == 0 ? effects.shake()
                                                                             : sf::Vector2f{});
    }
    return 0;
}
} // namespace pocket
