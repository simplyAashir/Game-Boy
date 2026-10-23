#include "Frontend.hpp"
#include <algorithm>
#include <cmath>
namespace pocket {
using namespace palette;
void SnakeGame::draw(Painter &p) const {
    p.frame(64, 240, 720, 480, false, 25);
    constexpr float bx = 88, by = 264, size = 24;
    for (int y = 0; y < Grid::Rows; ++y)
        for (int x = 0; x < Grid::Columns; ++x) {
            p.box(bx + x * size, by + y * size, size, size,
                  ((x + y) % 2) ? sf::Color(7, 28, 21, 210) : sf::Color(13, 37, 27, 200), 0,
                  sf::Color(107, 104, 58, 40), .5f);
            if (x % 4 == 0 && y % 4 == 0)
                p.diamond(bx + x * size, by + y * size, 2, sf::Color(177, 143, 64, 100));
        }
    float alpha = running() && !finished() ? blend() : 1;
    for (int i = body().length() - 1; i >= 0; --i) {
        const auto &s = body().segment(i);
        float x = (s.previous.x + (s.cell.x - s.previous.x) * alpha) * size + bx,
              y = (s.previous.y + (s.cell.y - s.previous.y) * alpha) * size + by;
        if (i == 0)
            p.glow(x + 12, y + 12, 9, sf::Color(77, 236, 92, 95));
        p.box(x + 1, y + 1, 22, 22, i == 0 ? sf::Color(86, 208, 86) : sf::Color(37, 142, 64), 4,
              sf::Color(164, 234, 104), 1);
        p.box(x + 4, y + 4, 16, 16, sf::Color::Transparent, 3, sf::Color(193, 246, 119, 110));
        p.line(x + 4, y + 4, x + 18, y + 18, sf::Color(181, 236, 109, 70), 1);
        p.line(x + 18, y + 4, x + 4, y + 18, sf::Color(181, 236, 109, 70), 1);
        if (i == 0) {
            Cell next = body().next(), head = s.cell;
            int dx = next.x - head.x, dy = next.y - head.y;
            float ex = x + 12 + dx * 4, ey = y + 12 + dy * 4;
            p.circle(ex - dy * 5, ey + dx * 5, 2.5f, ink);
            p.circle(ex + dy * 5, ey - dx * 5, 2.5f, ink);
            p.line(x + 12 + dx * 10, y + 12 + dy * 10, x + 12 + dx * 17, y + 12 + dy * 17, paper,
                   1);
        }
    }
    Cell f = meal();
    if (f.x >= 0) {
        float fx = bx + f.x * size + 12, fy = by + f.y * size + 12;
        p.glow(fx, fy, 9, sf::Color(249, 76, 29, 170));
        p.circle(fx - 2, fy, 7, sf::Color(206, 32, 25), gold, 1);
        p.circle(fx + 3, fy, 6, sf::Color(239, 58, 36));
        p.circle(fx - 3, fy - 3, 2, sf::Color(255, 191, 112));
        p.line(fx, fy - 6, fx + 4, fy - 12, lime, 2);
    }
    if (bonus().x >= 0) {
        Cell b = bonus();
        float x = bx + b.x * size + 12, y = by + b.y * size + 12;
        p.glow(x, y, 10, sf::Color(112, 219, 235, 150));
        p.diamond(x, y, 8, violet);
        p.diamond(x, y, 4, paper);
    }
    p.frame(810, 237, 405, 482, true, 28);
    p.center("Snake", 1012, 280, 22, darkText, true);
    p.flourish(1012, 311, 285, line);
    p.center("Score", 917, 342, 18, darkText);
    p.center(std::to_string(score()), 917, 380, 34, darkText, true);
    p.center("Best", 1106, 342, 18, darkText);
    p.center(std::to_string(std::max(p.bestScore, score())), 1106, 380, 34, darkText, true);
    p.flourish(1012, 418, 285, line);
    p.center("Level " + std::to_string(level()) + "  /  Length " + std::to_string(body().length()),
             1012, 448, 18, darkText);
    p.center("Arrows / WASD to move.", 1012, 493, 21, darkText);
    p.center("Collect fruit to grow.", 1012, 527, 21, darkText);
    p.center("Avoid walls and your tail.", 1012, 561, 21, darkText);
    p.flourish(1012, 600, 285, line);
    p.center(slowed() > 0 ? "Slow Motion" : "Blue Crystal", 1012, 633, 18, darkText, true);
    p.center(slowed() > 0
                 ? std::to_string(static_cast<int>(std::ceil(slowed()))) + " seconds of slowed time"
                 : "Find a crystal to slow time.",
             1012, 665, 17, darkText);
    if (!running() && !finished()) {
        p.frame(222, 423, 404, 108);
        p.center("Ready?", 424, 460, 20, gold, true);
        p.center("Arrow keys / WASD / ENTER to begin", 424, 498, 16, paper);
    }
}
void WordleGame::draw(Painter &p) const {
    p.illustration(1, 74, 332, 272, 225);

    p.frame(444, 199, 378, 380, true, 22);
    for (int row = 0; row < 6; ++row)
        for (int col = 0; col < 5; ++col) {
            int state = board().colors[row][col];
            bool active = row == board().used && !finished();
            float phase = 1;
            if (row == p.revealRow)
                phase = std::clamp((p.revealTime - col * .13f) / .28f, 0.f, 1.f);
            int shown = phase < .5f ? 0 : state;
            sf::Color fill = shown == 3   ? sf::Color(38, 103, 44)
                             : shown == 2 ? sf::Color(220, 174, 35)
                             : shown == 1 ? sf::Color(6, 8, 7)
                                          : sf::Color(205, 181, 133, 120);
            float height =
                51 * (row == p.revealRow ? std::max(.06f, std::abs(std::cos(phase * 3.14159265f)))
                                         : 1.f);
            float x = 480 + col * 65.f, y = 220 + row * 59.f + (51 - height) / 2;
            p.box(x, y, 56, height, fill, 2, active ? gold : line, 1.5f);
            char letter = ' ';
            if (row < board().used)
                letter = board().rows[row][col];
            else if (active && col < static_cast<int>(entry().size()))
                letter = entry()[col];
            if (height > 23)
                p.center(std::string(1, letter), x + 28, y + height / 2, 28,
                         shown ? paper : darkText);
            if (active && col == static_cast<int>(entry().size()) && std::fmod(p.time, 1.f) < .55f)
                p.box(x + 18, y + 40, 20, 2, line);
        }
    p.frame(863, 246, 342, 287, true, 25);
    p.center("Find the Word", 1033, 282, 18, darkText, true);
    p.flourish(1033, 314, 246, line);
    const sf::Color swatches[] = {sf::Color(38, 103, 44), sf::Color(220, 174, 35),
                                  sf::Color(6, 8, 7)};
    const char *labels[] = {"Right letter, right place", "Right letter, wrong place",
                            "Letter not in word"};
    for (int i = 0; i < 3; ++i) {
        p.box(893, 341 + i * 45.f, 20, 20, swatches[i], 1, line);
        p.text(labels[i], 925, 339 + i * 45.f, 16, darkText);
    }
    p.center("Guess " + std::to_string(std::min(6, board().used + 1)) + " of 6", 1033, 498, 18,
             darkText, true);
    p.frame(65, 251, 270, 70, false, 12);
    bool error = notice() == "Enter five letters first.";
    if (error)
        p.box(70, 256, 260, 60, sf::Color::Transparent, 2, gold, 2);
    p.paragraph(notice(), 88, 268, 225, 15, error ? gold : paper);
    p.keyboard(keys(), 600);
}
void HangmanFigure::draw(Painter &p) const {
    float strength = p.reactionTime / .8f;
    float shake = p.reactionKind < 0 ? std::sin((.8f - p.reactionTime) * 60) * 10 * strength : 0;
    p.wanderer(270 + shake, 371, .88f, count(), gold);
}
void HangmanGame::draw(Painter &p) const {
    p.frame(64, 238, 423, 326, false, 23);
    p.box(89, 261, 374, 278, sf::Color(7, 24, 18, 90));
    figure.draw(p);
    float pulse = p.reactionTime / .8f, progress = 1 - pulse;
    if (pulse > 0) {
        sf::Color tint = p.reactionKind > 0 ? lime : red;
        tint.a = static_cast<sf::Uint8>(180 * pulse);
        p.box(70, 244, 411, 314, sf::Color::Transparent, 4, tint, 2 + pulse * 3);
        if (p.reactionKind < 0) {
            p.circle(276, 410, 25 + progress * 90, sf::Color::Transparent, tint, 3);
            p.line(246, 385, 306, 445, tint, 3 * pulse);
            p.line(306, 385, 246, 445, tint, 3 * pulse);
        }
    }
    p.frame(66, 563, 420, 133);
    p.center("Lives", 276, 594, 18, gold, true);
    for (int i = 0; i < 6; ++i) {
        if (i < 6 - wrong())
            p.glow(151 + i * 49.f, 635, 8, sf::Color(253, 177, 43, 110));
        p.heart(151 + i * 49.f, 635, 13, i < 6 - wrong() ? peach : sf::Color(38, 45, 31));
    }
    p.center("Incorrect guesses: " + std::to_string(wrong()) + " / 6", 276, 673, 15, paper);
    p.frame(514, 237, 701, 331, true, 28);
    p.center("Guess the word", 863, 277, 21, darkText);
    p.flourish(863, 310, 530, line);
    const auto &word = solution();
    float width = std::min(62.f, 626.f / word.size()), start = 863 - word.size() * width / 2;
    for (int i = 0; i < static_cast<int>(word.size()); ++i) {
        float x = start + i * width;
        if (used(word[i]) || finished())
            p.center(std::string(1, word[i]), x + width / 2,
                     372 -
                         ((p.reactionKind > 0 && (p.reactionLetter == word[i] || !p.reactionLetter))
                              ? std::sin(progress * 3.14159265f) * 13 * pulse
                              : 0),
                     37, used(word[i]) ? darkText : red, true);
        if (pulse > 0 && p.reactionKind > 0 && used(word[i]) &&
            (p.reactionLetter == word[i] || !p.reactionLetter)) {
            for (int j = 0; j < 6; ++j) {
                float angle = j * 3.14159265f / 3;
                p.glow(x + width / 2 + std::cos(angle) * progress * 38,
                       372 + std::sin(angle) * progress * 38, 2 + pulse * 2,
                       sf::Color(109, 224, 92, static_cast<sf::Uint8>(230 * pulse)));
            }
        }
        p.line(x + 6, 404, x + width - 6, 404, line, 2);
    }

    if (pulse > 0) {
        sf::Color color = p.reactionKind > 0 ? sf::Color(34, 102, 43) : red;
        std::string label = p.reactionKind > 0 ? "Correct!" : "Wrong guess";
        if (p.reactionLetter)
            label = std::string(1, p.reactionLetter) + "  -  " + label;
        p.center(label, 863, 457 - std::sin(progress * 3.14159265f) * 7, 23, color, true);
        if (p.reactionKind < 0 && wrong() > 0) {
            float x = 151 + (6 - wrong()) * 49.f;
            p.circle(x, 635, 14 + progress * 21, sf::Color::Transparent,
                     sf::Color(223, 76, 54, static_cast<sf::Uint8>(220 * pulse)), 2);
        }
    }
    p.button(hasHint() ? "Hint Used" : "Hint [F1]", 544, 518, 245, 40, hasHint(), gold);
    if (hasHint())
        p.fit(hint(), 805, 529, 369, 15, darkText);
    else
        p.text("Hint costs 75 points.", 805, 529, 16, darkText);
    KeyboardDisplay blank;
    p.keyboard(blank, 589, true, this);
}
} // namespace pocket
