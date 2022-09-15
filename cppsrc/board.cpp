#include "board.h"
#include "lifeforms.h"

int directions[4][2] = {{1, 0}, {0, -1}, {-1, 0}, {0, 1}};
Cell *board[BOARD_DIM][BOARD_DIM];

int boundCheckPos(int x, int y)
{
	if (0 > x || 0 > y || x >= BOARD_DIM || y >= BOARD_DIM)
	{
		return 1;
	}
	return 0;
}

int isCellOfType(int x, int y, enum CellTypes type)
{
	if (boundCheckPos(x, y))
	{
		return 0;
	}

	return board[y][x]->type == type;
}