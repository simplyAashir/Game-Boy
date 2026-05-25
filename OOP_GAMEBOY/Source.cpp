#include <SFML/Graphics.hpp>
#include "SnakeGame.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;
using namespace sf;

int main() {
    srand(time(0));
    sf::RenderWindow window(sf::VideoMode(850, 600), "Snake Game");
    window.setVerticalSyncEnabled(true);

    // Load background
    sf::Texture Texbgd_snake;
    if (!Texbgd_snake.loadFromFile("D:/Semester 03/OOP/OOP_GAMEBOY/Sprites/bgd_snake1.png")) {
        std::cout << "Failed to load background texture!" << std::endl;
        return -1;
    }
    sf::Sprite sprt_bgd_snake(Texbgd_snake);

    // Initialize snake
    SnakeSegment snakeSegment(100, 100, 'r'); // Starting position and direction
    sf::Clock moveClock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Smooth movement using the updated moveSnake function
        snakeSegment.moveSnake(window, 5.0f, moveClock);

        // Render everything
        window.clear();
        window.draw(sprt_bgd_snake);
        snakeSegment.draw(window);
        window.display();
    }

    return 0;
}
