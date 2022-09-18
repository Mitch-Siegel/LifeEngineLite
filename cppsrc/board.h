#include <vector>

#include "config.h"
#include "lifeforms.h"

#pragma once
extern int directions[8][2];
class Cell;

class Board
{
public:
    std::size_t tickCount;
    int dim_x, dim_y;
    std::vector<std::vector<Cell *>> cells;
    std::vector<bool> DeltaCells;

    std::vector<Cell *> FoodCells;

    std::vector<Organism *> Organisms;

    Board(const int _dim_x, const int _dim_y);

    ~Board();

    void Tick();

    bool boundCheckPos(const int _x, const int _y);

    bool isCellOfType(const int _x, const int _y, enum CellTypes type);

    void replaceCellAt(const int _x, const int _y, Cell *_cell);

    void replaceCell(Cell *_replaced, Cell *_newCell);

    void swapCellAtIndex(int _x, int _y, Cell *_toSwap);

    Organism *createOrganism(const int _x, const int _y);

};

