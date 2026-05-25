#pragma once
#ifndef POINT_H
#define POINT_H
#include<SFML/Graphics.hpp>
#include<iostream>
using namespace std;
using namespace sf;
class Point
{
	int x;
	int y;
	Sprite foodSprite;
	Texture foodTexture;
	
public:
	//Point() {}
	Point();
	void setX(int x);
	void setY(int y);
	int getX();
	int getY();
	void draw(RenderWindow& window);
};

#endif 



