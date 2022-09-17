#include <vector>

#include "config.h"
#include "lifeforms.h"

#pragma once
extern int directions[4][2];
class Cell;

class Board
{
public:
    int dim_x, dim_y;
    std::vector<std::vector<Cell *>> cells;

    std::vector<Organism *> Organisms;

    Board(const int _dim_x, const int _dim_y);

    void Tick();

    bool boundCheckPos(const int _x, const int _y);

    bool isCellOfType(const int _x, const int _y, enum CellTypes type);

    Cell *replaceCellAt(const int _x, const int _y, Cell *_cell);

    Organism *createOrganism(const int _x, const int _y);

};

// #ifndef board
// #include "lifeforms.h"
// class Cell;
// extern Cell *board[BOARD_DIM][BOARD_DIM];
// #endif
