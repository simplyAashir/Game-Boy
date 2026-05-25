#pragma once

#ifndef SNAKESEGMENT_H
#define SNAKESEGMENT_H

#include <SFML/Graphics.hpp>
#include <iostream>
using namespace std;
using namespace sf;

class SnakeSegment {
    int x;
    int y;
    char dir;
    Sprite snakeSprite;
    Texture snakeTexture;

public:
    SnakeSegment(int x = 0, int y = 0, char dir = 0); // Constructor
    void draw(RenderWindow& window);                 // Draw function
    void moveSnake(sf::RenderWindow& window, float speed, sf::Clock& moveClock);                            // Snake movement
    void updateDirection(Event& event);             // Handle direction change
};

#endif
