#pragma once
#include "SnakeSegment.h"
#include "Grid.h"
#include "Point.h"
#ifndef SNAKEGAME_H
#define SNAKEGAME_H
	class SnakeGame
	{
		SnakeSegment* snakeBody;
		Point food;
		Grid grid;
	public:
		SnakeGame(Point food, SnakeSegment snakesegment): food(food),snakeBody(&snakesegment){}

		SnakeGame(Point food,Grid grid): food(food),grid(grid),snakeBody(new SnakeSegment){
			
			Texture Texbgd_snake;
			Texbgd_snake.loadFromFile("D:/Semester 03/OOP/OOP_GAMEBOY/Sprites/bgd_snake1.png");
			Sprite sprt_bgd_snake(Texbgd_snake);
		
		}

	};
#endif 



