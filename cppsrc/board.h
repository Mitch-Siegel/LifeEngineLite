#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <boost/thread/mutex.hpp>

#include "config.h"
#include "lifeforms.h"

class GameWindow;

#pragma once
extern int directions[8][2];
class Cell;

namespace std
{
    template <>
    struct hash<std::pair<int, int>>
    {
        auto operator()(const std::pair<int, int> &p) const -> uint64_t
        {
            return static_cast<uint64_t>(p.first) + (static_cast<uint64_t>(p.second) << 32);
        }
    };
} // namespace std

class Board
{
private:
    boost::mutex mutex;
    unsigned int nextSpecies;
    std::unordered_map<unsigned int, unsigned int> speciesCounts;

    class Food_Slot
    {
    private:
        std::vector<Spoilable_Cell *> cells;

    public:
        int ticksUntilSpoil;

        explicit Food_Slot(int _ticksUntilSpoil) { this->ticksUntilSpoil = _ticksUntilSpoil; }

        void push_back(Spoilable_Cell *c) { this->cells.push_back(c); };

        std::vector<Spoilable_Cell *>::iterator erase(std::vector<Spoilable_Cell *>::iterator element) { return this->cells.erase(element); }

        std::vector<Spoilable_Cell *>::iterator begin() { return this->cells.begin(); }

        std::vector<Spoilable_Cell *>::iterator end() { return this->cells.end(); }
    };

    std::map<uint64_t, Food_Slot *> FoodCells;

public:
    std::unordered_map<unsigned int, enum OrganismClassifications>
        speciesClassifications;

    std::unordered_map<unsigned int, unsigned int> evolvedFrom;
    std::unordered_map<unsigned int, unsigned int> peakSpeciesCounts;

    std::vector<unsigned int> activeSpecies;

    uint64_t tickCount;
    int dim_x, dim_y;
    std::vector<std::vector<Cell *>> cells;

    std::unordered_set<std::pair<int, int>> DeltaCells;

    std::vector<Organism *> Organisms;

    Board(const int _dim_x, const int _dim_y);

    ~Board();

    inline void GetMutex() { this->mutex.lock(); }

    inline bool TryGetMutex() { return this->mutex.try_lock(); }

    inline void ReleaseMutex() { this->mutex.unlock(); }

    void Tick();

    void Stats();

    bool boundCheckPos(const int _x, const int _y);

    bool isCellOfType(const int _x, const int _y, enum CellTypes type);

    void replaceCellAt(const int _x, const int _y, Cell *_cell);

    void replaceCell(Cell *_replaced, Cell *_newCell) { this->replaceCellAt(_replaced->x, _replaced->y, _newCell); }

private:
    void replaceCellAt_NoTrackReplacedFood(const int _x, const int _y, Cell *_cell);

    void replaceCell_NoTrackReplacedFood(Cell *_replaced, Cell *_newCell) { this->replaceCellAt_NoTrackReplacedFood(_replaced->x, _replaced->y, _newCell); }

public:
    void swapCellAtIndex(int _x, int _y, Cell *_toSwap);

    Organism *createOrganism(const int _x, const int _y);

    unsigned int GetNextSpecies();

    void AddSpeciesMember(Organism *o);

    void RemoveSpeciesMember(unsigned int species);
};
