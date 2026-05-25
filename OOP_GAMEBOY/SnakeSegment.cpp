#include "SnakeSegment.h"

// Constructor (already defined as before)
SnakeSegment::SnakeSegment(int x, int y, char dir) : x(x), y(y), dir(dir) {
    if (!snakeTexture.loadFromFile("D:/Semester 03/OOP/OOP_GAMEBOY/Sprites/Snake1.png")) {
        cout << "Failed to load snake texture!" << endl;
    }
    snakeSprite.setTexture(snakeTexture);
}

const int boxPixelsX = 32; // Horizontal step size
const int boxPixelsY = 32; // Vertical step size

void SnakeSegment::moveSnake(sf::RenderWindow& window, float speed, sf::Clock& moveClock) {
    int X = snakeSprite.getPosition().x;
    int Y = snakeSprite.getPosition().y;

    // Add a longer wait threshold
    if (moveClock.getElapsedTime().asMilliseconds() < 200) {
        return; // Wait for the threshold to allow smoother movement
    }

    // Only restart the clock after valid movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && Y - boxPixelsY >= 0) {
        snakeSprite.move(0, -boxPixelsY);
        std::cout << "Moved Up to Y = " << snakeSprite.getPosition().y << std::endl;
        moveClock.restart(); // Restart clock
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && Y + boxPixelsY <= window.getSize().y) {
        snakeSprite.move(0, boxPixelsY);
        std::cout << "Moved Down to Y = " << snakeSprite.getPosition().y << std::endl;
        moveClock.restart(); // Restart clock
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && X - boxPixelsX >= 0) {
        snakeSprite.move(-boxPixelsX, 0);
        std::cout << "Moved Left to X = " << snakeSprite.getPosition().x << std::endl;
        moveClock.restart(); // Restart clock
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && X + boxPixelsX <= window.getSize().x) {
        snakeSprite.move(boxPixelsX, 0);
        std::cout << "Moved Right to X = " << snakeSprite.getPosition().x << std::endl;
        moveClock.restart(); // Restart clock
    }
}


// Draw function (unchanged)
void SnakeSegment::draw(sf::RenderWindow& window) {
    snakeSprite.setPosition(x, y);
    window.draw(snakeSprite);
}
