#include <stdlib.h>
#include <iostream>

#include "cell.h"

#define BOARD_DIM 20
Cell board[BOARD_DIM][BOARD_DIM];

int main(int argc, char *argv[])
{
	//board = new Cell *[BOARD_DIM];
	for(int y = 0; y < BOARD_DIM; y++)
	{
		for(int x = 0; x < BOARD_DIM; x++)
		{
			board[y][x] = Cell(x, y, cell_food, NULL);
		}
	}
	std::cout << "Hello, World!" << std::endl;
}
