#include "Point.h"


Point::Point() {
	x = rand() % 650 + 100;
	y = rand() % 400 + 100;

	if (foodTexture.loadFromFile("D:/Semester 03/OOP/OOP_GAMEBOY/Sprites/food1.png")) {
		cout << "Failed to load texture" << std::endl;
	}
	foodSprite.setTexture(foodTexture);
}



void Point::setX(int x) {
	this->x = x;
}

void Point::setY(int y) {
	this->y = y;
}

int Point::getX() {
	return x;
}

int Point::getY() {
	return y;
}


void Point::draw(RenderWindow& window) {
	/*x = rand() % 650 + 100;
	y = rand() % 400 + 100;
	foodSprite.setPosition(x, y);*/
	window.draw(foodSprite);
}