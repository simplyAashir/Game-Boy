#include "Frontend.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
namespace pocket {
using namespace palette;
bool Resources::load() {
    bool ok = regular.loadFromFile("assets/fonts/Lora.ttf") &&
              bold.loadFromFile("assets/fonts/CinzelDecorative-Bold.ttf") &&
              environments.loadFromFile("assets/art/environments.png") &&
              atlas.loadFromFile("assets/art/fantasy-atlas.png") &&
              puppet.loadFromFile("assets/art/wanderer.png");
    environments.setSmooth(true);
    atlas.setSmooth(true);
    puppet.setSmooth(true);
    return ok;
}
void Painter::box(float x, float y, float w, float h, sf::Color fill, float radius,
                  sf::Color outline, float thickness) {
    if (w <= 0 || h <= 0)
        return;
    radius = std::min(radius, std::min(w, h) / 2);
    sf::ConvexShape shape;
    constexpr int steps = 6;
    shape.setPointCount(4 * (steps + 1));
    const float cx[4] = {x + w - radius, x + w - radius, x + radius, x + radius},
                cy[4] = {y + radius, y + h - radius, y + h - radius, y + radius};
    for (int corner = 0; corner < 4; ++corner)
        for (int j = 0; j <= steps; ++j) {
            float angle = (-90 + corner * 90 + j * 90.f / steps) * 3.14159265f / 180;
            shape.setPoint(corner * (steps + 1) + j, {cx[corner] + std::cos(angle) * radius,
                                                      cy[corner] + std::sin(angle) * radius});
        }
    shape.setFillColor(fill);
    shape.setOutlineColor(outline);
    shape.setOutlineThickness(outline.a ? thickness : 0);
    target.draw(shape);
}
void Painter::circle(float x, float y, float radius, sf::Color fill, sf::Color outline,
                     float thickness) {
    sf::CircleShape shape(radius, 40);
    shape.setPosition(x - radius, y - radius);
    shape.setFillColor(fill);
    shape.setOutlineColor(outline);
    shape.setOutlineThickness(outline.a ? thickness : 0);
    target.draw(shape);
}
void Painter::line(float x, float y, float x2, float y2, sf::Color color, float thickness) {
    sf::RectangleShape shape({std::hypot(x2 - x, y2 - y), thickness});
    shape.setOrigin(0, thickness / 2);
    shape.setPosition(x, y);
    shape.setRotation(std::atan2(y2 - y, x2 - x) * 180 / 3.14159265f);
    shape.setFillColor(color);
    target.draw(shape);
}
void Painter::text(const std::string &value, float x, float y, unsigned size, sf::Color color,
                   bool weight) {
    sf::Text text(value, weight ? resources.bold : resources.regular, size);
    text.setPosition(x, y);
    text.setFillColor(color);
    target.draw(text);
}
void Painter::center(const std::string &value, float x, float y, unsigned size, sf::Color color,
                     bool weight) {
    sf::Text text(value, weight ? resources.bold : resources.regular, size);
    auto bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
    text.setPosition(x, y);
    text.setFillColor(color);
    target.draw(text);
}
void Painter::fit(const std::string &value, float x, float y, float width, unsigned size,
                  sf::Color color, bool weight) {
    sf::Text text(value, weight ? resources.bold : resources.regular, size);
    while (size > 10 && text.getLocalBounds().width > width)
        text.setCharacterSize(--size);
    text.setPosition(x, y);
    text.setFillColor(color);
    target.draw(text);
}
void Painter::image(const sf::Texture &texture, sf::IntRect source, float x, float y, float w,
                    float h, sf::Color tint) {
    if (w <= 0 || h <= 0)
        return;
    sf::Sprite sprite(texture, source);
    sprite.setPosition(x, y);
    sprite.setScale(w / source.width, h / source.height);
    sprite.setColor(tint);
    target.draw(sprite);
}
void Painter::frame(float x, float y, float w, float h, bool parchment, float border) {
    const int sx = parchment ? 512 : 0, sy = 0, sw = 512, sh = 490, pad = 64;
    float b = std::min(border, std::min(w, h) / 2);
    const int xs[3] = {sx, sx + pad, sx + sw - pad}, ys[3] = {sy, sy + pad, sy + sh - pad},
              ws[3] = {pad, sw - 2 * pad, pad}, hs[3] = {pad, sh - 2 * pad, pad};
    const float dx[3] = {x, x + b, x + w - b}, dy[3] = {y, y + b, y + h - b},
                dw[3] = {b, w - 2 * b, b}, dh[3] = {b, h - 2 * b, b};
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
            image(resources.atlas, {xs[col], ys[row], ws[col], hs[row]}, dx[col], dy[row], dw[col],
                  dh[row]);
}
void Painter::illustration(int id, float x, float y, float w, float h, sf::Color tint) {
    const sf::IntRect sources[] = {
        {1024, 0, 512, 490}, {0, 490, 512, 534}, {512, 490, 512, 534}, {1024, 490, 512, 534}};
    image(resources.atlas, sources[id >= 0 && id < 4 ? id : 0], x, y, w, h, tint);
}
void Painter::environment(int scene) {
    const auto size = resources.environments.getSize();
    int w = static_cast<int>(size.x) / 2, h = static_cast<int>(size.y) / 2;
    sf::IntRect rect((scene % 2) * w, (scene / 2) * h, w, h);
    const int crop = static_cast<int>(w / (1280.f / 800));
    rect.top += (h - crop) / 2;
    rect.height = crop;
    image(resources.environments, rect, 0, 0, 1280, 800);
    box(0, 0, 1280, 800, sf::Color(4, 14, 11, 35));
    // Deterministic looping fireflies are visual effects, not gameplay state.
    for (int i = 0; i < 23; ++i) {
        float x = std::fmod(i * 153.7f + std::sin(time * .23f + i) * 19 + 1280, 1280.f),
              y = std::fmod(i * 83.3f - time * (2 + i % 4) + 8000, 800.f);
        float pulse = (1 + std::sin(time * 1.1f + i)) * .5f;
        glow(x, y, 2 + pulse * 2,
             sf::Color(238, 198, 78, static_cast<sf::Uint8>(90 + pulse * 100)));
    }
}
void Painter::diamond(float x, float y, float size, sf::Color color) {
    sf::CircleShape shape(size, 4);
    shape.setOrigin(size, size);
    shape.setPosition(x, y);
    shape.setFillColor(color);
    target.draw(shape);
}
void Painter::glow(float x, float y, float radius, sf::Color color) {
    for (int i = 4; i > 0; --i) {
        sf::Color c = color;
        c.a = static_cast<sf::Uint8>(color.a / (i * 4));
        circle(x, y, radius * i, c);
    }
    circle(x, y, radius * .3f, color);
}
void Painter::flourish(float x, float y, float width, sf::Color color) {
    line(x - width / 2, y, x - 12, y, color, 1);
    line(x + 12, y, x + width / 2, y, color, 1);
    diamond(x, y, 6, color);
    diamond(x - width / 2, y, 3, color);
    diamond(x + width / 2, y, 3, color);
}
void Painter::heart(float x, float y, float s, sf::Color color) {
    circle(x - s * .4f, y - s * .2f, s * .55f, color);
    circle(x + s * .4f, y - s * .2f, s * .55f, color);
    sf::ConvexShape shape(3);
    shape.setPoint(0, {x - s * .9f, y});
    shape.setPoint(1, {x + s * .9f, y});
    shape.setPoint(2, {x, y + s});
    shape.setFillColor(color);
    target.draw(shape);
}
void Painter::paragraph(const std::string &value, float x, float y, float width, unsigned size,
                        sf::Color color) {
    std::string line, word;
    for (std::size_t i = 0; i <= value.size(); ++i) {
        char c = i < value.size() ? value[i] : ' ';
        if (c != ' ') {
            word += c;
            continue;
        }
        if (word.empty())
            continue;
        sf::Text measure(line + (line.empty() ? "" : " ") + word, resources.regular, size);
        if (!line.empty() && measure.getLocalBounds().width > width) {
            text(line, x, y, size, color);
            y += size * 1.4f;
            line = word;
        } else
            line += (line.empty() ? "" : " ") + word;
        word.clear();
    }
    if (!line.empty())
        text(line, x, y, size, color);
}
bool Painter::hover(float x, float y, float w, float h) const {
    return sf::FloatRect(x, y, w, h).contains(mouse);
}
void Painter::button(const std::string &value, float x, float y, float w, float h, bool selected,
                     sf::Color accent) {
    bool hot = hover(x, y, w, h), active = selected || hot;
    sf::Color fill = active ? sf::Color(237, 196, 101) : sf::Color(11, 30, 22, 245);
    if (active)
        box(x - 4, y - 4, w + 8, h + 8, sf::Color(245, 202, 93, 48), 6);
    box(x, y, w, h, fill, 3, active ? paper : palette::line, active ? 3 : 1);
    box(x + 3, y + 3, w - 6, h - 6, sf::Color::Transparent, 1,
        active ? sf::Color(100, 70, 22) : sf::Color(116, 101, 58, 120));
    const float l = std::min(9.f, h * .2f);
    line(x, y + l, x + l, y, gold, 1);
    line(x + w - l, y, x + w, y + l, gold, 1);
    line(x, y + h - l, x + l, y + h, gold, 1);
    line(x + w - l, y + h, x + w, y + h - l, gold, 1);
    unsigned size = h < 32 ? 12 : h < 40 ? 15 : 18;
    sf::Text measure(value, resources.regular, size);
    while (size > 10 && measure.getLocalBounds().width > w - 14)
        measure.setCharacterSize(--size);
    center(value, x + w / 2, y + h / 2, size, active ? darkText : muted, false);
    (void)accent;
}
void Painter::wanderer(float x, float y, float scale, int mistakes, sf::Color color) {
    const sf::Color wood{140, 91, 43}, light{192, 143, 77};
    line(x - 118 * scale, y + 158 * scale, x + 105 * scale, y + 158 * scale, wood, 10 * scale);
    line(x - 100 * scale, y + 158 * scale, x - 100 * scale, y - 106 * scale, wood, 14 * scale);
    line(x - 100 * scale, y - 106 * scale, x + 22 * scale, y - 106 * scale, wood, 14 * scale);
    line(x - 96 * scale, y - 62 * scale, x - 55 * scale, y - 104 * scale, light, 6 * scale);
    line(x + 20 * scale, y - 106 * scale, x + 20 * scale, y - 75 * scale, gold, 3 * scale);
    const sf::FloatRect parts[] = {{.385f, 0, .22f, .25f},      {.385f, .245f, .22f, .32f},
                                   {.1f, .25f, .285f, .38f},    {.605f, .25f, .295f, .38f},
                                   {.255f, .535f, .24f, .445f}, {.505f, .535f, .235f, .445f}};
    auto size = resources.puppet.getSize();
    float width = 240 * scale, height = 245 * scale;
    float left = x + 20 * scale - .49f * width, top = y - 75 * scale;
    for (int i = 0; i < std::min(6, mistakes); ++i) {
        auto r = parts[i];
        sf::IntRect crop(static_cast<int>(r.left * size.x), static_cast<int>(r.top * size.y),
                         static_cast<int>(r.width * size.x), static_cast<int>(r.height * size.y));
        if (i == 2 || i == 3) {
            const sf::Vector2f leftArm[] = {
                {.10f, .25f}, {.385f, .25f}, {.385f, .48f}, {.25f, .64f}, {.10f, .64f}};
            const sf::Vector2f rightArm[] = {
                {.605f, .25f}, {.90f, .25f}, {.90f, .64f}, {.75f, .64f}, {.605f, .48f}};
            sf::Vertex vertices[5];
            for (int j = 0; j < 5; ++j) {
                auto q = i == 2 ? leftArm[j] : rightArm[j];
                vertices[j].position = {left + q.x * width, top + q.y * height};
                vertices[j].texCoords = {q.x * size.x, q.y * size.y};
            }
            sf::RenderStates states;
            states.texture = &resources.puppet;
            target.draw(vertices, 5, sf::TriangleFan, states);
        } else
            image(resources.puppet, crop, left + r.left * width, top + r.top * height,
                  r.width * width, r.height * height);
    }
    (void)color;
}
void Painter::keyboard(const KeyboardDisplay &keyboard, float y, bool hangman,
                       const HangmanGame *game) {
    float cx = hangman ? 863.f : 640.f;
    frame(cx - 320, y - 10, 640, 139, false, 15);
    const char *rows[] = {"QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};
    for (int row = 0; row < 3; ++row) {
        int n = static_cast<int>(std::char_traits<char>::length(rows[row]));
        float start = cx - n * 60 / 2.f;
        for (int col = 0; col < n; ++col) {
            char c = rows[row][col];
            int state = keyboard.states[c - 'A'];
            if (hangman && game && game->used(c))
                state = game->solution().find(c) == std::string::npos ? 1 : 3;
            sf::Color fill = state == 3   ? sf::Color(37, 103, 43)
                             : state == 2 ? sf::Color(169, 124, 26)
                             : state == 1 ? sf::Color(7, 9, 8)
                                          : sf::Color(197, 175, 126);
            float x = start + col * 60, top = y + row * 44;
            box(x, top, 54, 37, fill, 2, hover(x, top, 54, 37) ? paper : gold, 1);
            box(x + 3, top + 3, 48, 31, sf::Color::Transparent, 1, sf::Color(92, 69, 33, 140));
            center(std::string(1, c), x + 27, top + 18, 21, state ? paper : darkText);
        }
    }
}
int InputSystem::gameKey(const sf::Event &e, GameId id) const {
    if (e.type == sf::Event::TextEntered && e.text.unicode >= 32 && e.text.unicode < 127)
        return static_cast<int>(e.text.unicode);
    if (e.type != sf::Event::KeyPressed)
        return -1;
    if (e.key.code == sf::Keyboard::Enter)
        return 13;
    if (e.key.code == sf::Keyboard::Backspace)
        return 8;
    if (id == GameId::Snake) {
        if (e.key.code == sf::Keyboard::Up || e.key.code == sf::Keyboard::W)
            return 1000;
        if (e.key.code == sf::Keyboard::Right || e.key.code == sf::Keyboard::D)
            return 1001;
        if (e.key.code == sf::Keyboard::Down || e.key.code == sf::Keyboard::S)
            return 1002;
        if (e.key.code == sf::Keyboard::Left || e.key.code == sf::Keyboard::A)
            return 1003;
    }
    if (e.key.code == sf::Keyboard::F1)
        return 1004;
    return -1;
}
sf::Vector2f InputSystem::toLogical(sf::Vector2i pixel, sf::Vector2u size) const {
    if (size.x == 0 || size.y == 0)
        return {-1, -1};
    float scale = std::min(size.x / 1280.f, size.y / 800.f);
    return {(pixel.x - (size.x - 1280 * scale) / 2) / scale,
            (pixel.y - (size.y - 800 * scale) / 2) / scale};
}
sf::Vector2f InputSystem::cursor(const sf::RenderWindow &window, const sf::RenderTexture &) const {
    return toLogical(sf::Mouse::getPosition(window), window.getSize());
}
bool ScreenDisplay::create() {
    return buffer.create(Width, Height);
}
void ScreenDisplay::present(sf::RenderWindow &window, bool scanlines, float fade,
                            sf::Vector2f shake) {
    if (scanlines) {
        sf::RectangleShape line({1280, 1});
        line.setFillColor(sf::Color(0, 0, 0, 13));
        for (int y = 0; y < 800; y += 4) {
            line.setPosition(0, static_cast<float>(y));
            buffer.draw(line);
        }
    }
    if (fade > 0) {
        sf::RectangleShape overlay({1280, 800});
        overlay.setFillColor(
            sf::Color(15, 20, 23, static_cast<sf::Uint8>(std::min(180.f, fade * 450))));
        buffer.draw(overlay);
    }
    buffer.display();
    auto size = window.getSize();
    float scale = std::min(size.x / 1280.f, size.y / 800.f);
    sf::Sprite output(buffer.getTexture());
    output.setScale(scale, scale);
    output.setPosition((size.x - 1280 * scale) / 2 + shake.x * scale,
                       (size.y - 800 * scale) / 2 + shake.y * scale);
    // The SFML default view retains its initial size; presentation needs current pixels.
    window.setView(
        sf::View(sf::FloatRect(0, 0, static_cast<float>(size.x), static_cast<float>(size.y))));
    window.clear(ink);
    window.draw(output);
    window.display();
}
void GameEffects::update(float dt) {
    dt = std::max(0.f, dt);
    impact = std::min(2.f, impact + dt);
    record = std::max(0.f, record - dt);
    reaction = std::max(0.f, reaction - dt);
}
sf::Vector2f GameEffects::shake() const {
    if (!crashing())
        return {0, 0};
    float strength = 11 * (1 - impact);
    return {std::sin(impact * 113) * strength, std::cos(impact * 137) * strength * .6f};
}
bool SoundSystem::load(const std::string &root) {
    audioRoot = root;
    scene = -1;
    sleeping = false;
    bool complete = true;
    ready = true;
    sf::Listener::setGlobalVolume(100);
    for (int i = 0; i < static_cast<int>(Cue::Count); ++i) {
        std::string number = (i < 10 ? "0" : "") + std::to_string(i);
        available[i] = buffers[i].loadFromFile(audioRoot + "/effects/" + number + ".ogg");
        complete = available[i] && complete;
    }
    setScene(0);
    return complete && scene == 0;
}

void SoundSystem::volumes(const Settings &s) {
    master = s.master;
    musicVolume = s.music;
    effectVolume = s.effects;
    muted = s.muted;
    update(0);
}
void SoundSystem::play(int cue) {
    if (!ready || cue < 0 || cue >= static_cast<int>(Cue::Count) || sleeping)
        return;
    if (cue == static_cast<int>(Cue::Wrong)) {
        int variant = std::rand() % 3;
        if (variant)
            cue =
                variant == 1 ? static_cast<int>(Cue::WrongAlt1) : static_cast<int>(Cue::WrongAlt2);
    }
    if (!available[cue])
        cue = static_cast<int>(Cue::Select);
    if (!available[cue])
        return;
    int index = channel++ % 12;
    sf::Sound &voice = channels[index];
    voice.stop();
    voice.setBuffer(buffers[cue]);
    voice.setRelativeToListener(true);
    voice.setAttenuation(0);
    float gain = (cue == static_cast<int>(Cue::Letter) || cue == static_cast<int>(Cue::Move) ||
                  cue == static_cast<int>(Cue::TileAbsent))
                     ? .45f
                     : 1.f;
    voiceGains[index] = gain;
    voice.setVolume(muted ? 0 : effectVolume * master * .01f * gain);
    voice.setPitch(cue == static_cast<int>(Cue::Delete)   ? .82f
                   : cue == static_cast<int>(Cue::Expire) ? .72f
                                                          : 1.f);
    voice.play();
}
void SoundSystem::setScene(int value) {
    if (!ready || value < 0 || value > 3 || value == scene)
        return;
    const char *names[] = {"menu", "snake", "wordle", "hangman"};
    int next = 1 - current;
    streams[next].stop();
    if (!streams[next].openFromFile((audioRoot + "/music/") + names[value] + ".ogg"))
        return;
    streams[next].setLoop(true);
    streams[next].setRelativeToListener(true);
    streams[next].setAttenuation(0);
    streams[next].setVolume(scene < 0 ? mixVolume() : 0);
    streams[next].play();
    if (sleeping)
        streams[next].pause();
    fade = scene < 0 ? 1.f : 0.f;
    current = next;
    scene = value;
}
void SoundSystem::update(float dt) {
    fade = std::min(1.f, fade + dt / .45f);
    float gain = mixVolume() * (ducked ? .4f : 1.f) * (stingTime > 0 ? .35f : 1.f);
    streams[current].setVolume(gain * fade);
    streams[1 - current].setVolume(gain * (1 - fade));
    if (fade >= 1 && streams[1 - current].getStatus() == sf::SoundSource::Playing)
        streams[1 - current].stop();
    for (int i = 0; i < 12; ++i)
        channels[i].setVolume(muted || sleeping ? 0 : effectVolume * master * .01f * voiceGains[i]);
    if (stingTime > 0) {
        stingTime = std::max(0.f, stingTime - dt);
        sting.setVolume(
            muted || sleeping ? 0 : master * .01f * effectVolume * std::min(1.f, stingTime));
        if (stingTime == 0)
            sting.stop();
    }
}
void SoundSystem::result(bool won) {
    if (!ready)
        return;
    sting.stop();
    if (sting.openFromFile((audioRoot + "/music/") + (won ? "victory" : "defeat") + ".ogg")) {
        stingTime = won ? 4.f : 3.f;
        sting.setRelativeToListener(true);
        sting.setAttenuation(0);
        sting.setVolume(muted ? 0 : master * .01f * effectVolume);
        sting.play();
    }
}
void SoundSystem::pause(bool paused) {
    sleeping = paused;
    for (auto &stream : streams) {
        if (paused)
            stream.pause();
        else if (stream.getStatus() == sf::SoundSource::Paused)
            stream.play();
    }
    if (paused) {
        sting.pause();
        for (auto &voice : channels)
            voice.stop();
    } else if (sting.getStatus() == sf::SoundSource::Paused)
        sting.play();
}
} // namespace pocket
