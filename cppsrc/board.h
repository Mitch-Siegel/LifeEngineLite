#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <boost/thread/mutex.hpp>

#include "config.h"
#include "lifeforms.h"
#include "worldstats.h"

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

struct SDL_Texture;

class SpeciesInfo
{
public:
    // which species is this
    uint32_t number;
    // the unique ID which will be assigned to the next member
    uint32_t nextID;
    // what species this species evolved from
    uint32_t evolvedFrom;
    // how many concurrently living members this species had at its peak
    uint32_t peakCount;
    // how many currently living members this species has
    uint32_t count;

    // list of species which this species has evolved into
    std::vector<uint32_t> evolvedInto;

    enum OrganismClassifications classification;

    SDL_Texture *example;

    SpeciesInfo()
    {
        this->number = 0;
        this->nextID = 0;
        this->evolvedFrom = 0;
        this->peakCount = 0;
        this->count = 0;
        this->classification = class_null;
        this->example = nullptr;
    };

    uint32_t GetNextId() { return this->nextID++; };
};

class Board
{
private:
    unsigned int nextSpecies;

    class Food_Slot
    {
    private:
        std::set<Spoilable_Cell *> cells;

    public:
        int ticksUntilSpoil;

        explicit Food_Slot(int _ticksUntilSpoil) { this->ticksUntilSpoil = _ticksUntilSpoil; }

        void insert(Spoilable_Cell *c) { this->cells.insert(c); };

        std::set<Spoilable_Cell *>::iterator erase(std::set<Spoilable_Cell *>::iterator element) { return this->cells.erase(element); }

        std::set<Spoilable_Cell *>::iterator begin() { return this->cells.begin(); }

        std::set<Spoilable_Cell *>::iterator end() { return this->cells.end(); }
    };

    std::map<uint64_t, Food_Slot *> FoodCells;

    std::map<uint32_t, SpeciesInfo> species;

    std::set<uint32_t> activeSpecies_;

public:
    boost::mutex mutex;

    uint64_t tickCount;
    int dim_x, dim_y;
    std::vector<std::vector<Cell *>> cells;

    std::set<std::pair<int, int>> DeltaCells;

    std::set<Organism *> Organisms;

    WorldStats stats;

    Board(const int _dim_x, const int _dim_y);

    ~Board();

    inline void GetMutex() { this->mutex.lock(); }

    inline bool TryGetMutex() { return this->mutex.try_lock(); }

    inline void ReleaseMutex() { this->mutex.unlock(); }

    bool Tick();

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

    const SpeciesInfo &GetSpeciesInfo(uint32_t species);

    const std::set<uint32_t> &activeSpecies() { return this->activeSpecies_; };

    void RecordEvolvedFrom(Organism *evolvedFrom, Organism *evolvedTo);
};
