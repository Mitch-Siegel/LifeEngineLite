#include "config.h"
#pragma once
extern int directions[4][2];
#define BOARD_DIM 20
class Cell;
extern Cell *board[BOARD_DIM][BOARD_DIM];




// #ifndef board
// #include "lifeforms.h"
// class Cell;
// extern Cell *board[BOARD_DIM][BOARD_DIM];
// #endif

int boundCheckPos(int x, int y);

int isCellOfType(int x, int y, enum CellTypes type);