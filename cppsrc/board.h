#include <vector>
#include <unordered_map>

#include "config.h"
#include "lifeforms.h"

class GameWindow;

#pragma once
extern int directions[8][2];
class Cell;

class Board
{
private:
    unsigned int nextSpecies;
    std::unordered_map<unsigned int, unsigned int> speciesCounts;
public:
    std::unordered_map<unsigned int, unsigned int> evolvedFrom;
    std::unordered_map<unsigned int, unsigned int> peakSpeciesCounts;

    std::vector<unsigned int> activeSpecies;

    std::size_t tickCount;
    int dim_x, dim_y;
    std::vector<std::vector<Cell *>> cells;
    std::vector<std::vector<bool>> DeltaCells;

    std::vector<Cell *> FoodCells;

    std::vector<Organism *> Organisms;

    Board(const int _dim_x, const int _dim_y);

    ~Board();

    void Tick();

    void Stats();

    bool boundCheckPos(const int _x, const int _y);

    bool isCellOfType(const int _x, const int _y, enum CellTypes type);

    void replaceCellAt(const int _x, const int _y, Cell *_cell);

    void replaceCell(Cell *_replaced, Cell *_newCell);

    void swapCellAtIndex(int _x, int _y, Cell *_toSwap);

    Organism *createOrganism(const int _x, const int _y);

    unsigned int GetNextSpecies();

    void AddSpeciesMember(unsigned int species);

    void RemoveSpeciesMember(unsigned int species);

};
