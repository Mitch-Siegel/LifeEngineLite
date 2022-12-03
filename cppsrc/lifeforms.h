#include "config.h"
#include "brain.h"

#include <vector>
#include <cstdint>

#pragma once

class Cell;
class OrganismView;

class Organism
{
	friend class Board;
	friend class OrganismView;

private:
	uint64_t currentHealth, maxHealth;
	uint64_t currentEnergy, maxEnergy;
	std::vector<Cell *> myCells;
	uint64_t nCells_;

public:
	int x = -1;
	int y = -1;
	int direction;
	unsigned int species;
	uint64_t age;
	int mutability;
	bool alive;
	int reproductionCooldown;
	uint64_t lifespan;
	Brain *brain;
	uint64_t cellCounts[cell_null];

	const uint64_t &nCells() const { return this->nCells_; }

	Organism(int center_x, int center_y);

	Organism(int center_x, int center_y, const Brain &baseBrain);

	~Organism();

	void Die();

	void Remove();

	Organism *Tick();

	void RecalculateStats();

	bool CheckValidity();

	void Move(int moveDirection);

	void Rotate(bool clockwise);

	void AddCell(int x_rel, int y_rel, Cell *_cell);

	void RemoveCell(Cell *_myCell);

	void ReplaceCell(Cell *_myCell, Cell *_newCell);

	uint64_t GetMaxHealth();

	uint64_t GetEnergy();

	uint64_t GetMaxEnergy();

	void Damage(uint64_t n);

	void Heal(uint64_t n);

	void ExpendEnergy(uint64_t n);

	void AddEnergy(uint64_t n);

	bool CanOccupyPosition(int _x_abs, int _y_abs);

	bool CanMoveToPosition(int _x_abs, int _y_abs);

	Organism *Reproduce();

	bool Mutate();

	enum OrganismClassifications Classify();
};

class Organism;

#define DEFAULT_MUTABILITY 15

// as proportion of max energy
#define REPRODUCTION_ENERGY_MULTIPLIER .8

#define REPRODUCTION_COOLDOWN 0

#define LIFESPAN_MULTIPLIER 35
#define ENERGY_DENSITY_MULTIPLIER 8
#define MAX_HEALTH_MULTIPLIER 1

#define FOOD_MULTIPLIER 3.0 * ENERGY_DENSITY_MULTIPLIER

#define LEAF_FOOD_ENERGY 1
#define FLOWER_FOOD_ENERGY 3
#define FRUIT_FOOD_ENERGY 8

#define SPOILTIME_BASE 10
#define PLANTMASS_SPOIL_TIME_MULTIPLIER 50 * SPOILTIME_BASE
#define BIOMASS_SPOIL_TIME_MULTIPLIER 20 * SPOILTIME_BASE

#define FRUIT_SPOIL_TIME 5 * SPOILTIME_BASE

// must roll this 2x
#define FRUIT_GROW_PERCENT 10

#define PLANTMASS_FOOD_ENERGY 2
// #define BIOMASS_FOOD_ENERGY 15 * PLANTMASS_FOOD_ENERGY
#define BIOMASS_FOOD_ENERGY 5 * PLANTMASS_FOOD_ENERGY

#define FLOWER_COST 2 * ENERGY_DENSITY_MULTIPLIER
#define LEAF_FLOWERING_COOLDOWN 50

// whether or not a leaf is able to flower, rolled at creation
#define LEAF_FLOWERING_ABILITY_PERCENT 40

#define PLANT_GROW_PERCENT 50
// percent for a flower to wilt into another leaf vs just going away
#define FLOWER_EXPAND_PERCENT 100

#define BARK_GROW_COOLDOWN 30
#define BARK_PLANT_VS_THORN 95
#define BARK_GROW_COST 2 * ENERGY_DENSITY_MULTIPLIER
#define BARK_MAX_INTEGRITY 3

// max integrity for leaves which are next to a bark

#define FLOWER_BLOOM_COOLDOWN 75
#define FLOWER_WILT_CHANCE 30
#define FLOWER_BLOOM_COST 3.5 * ENERGY_DENSITY_MULTIPLIER

#define TOUCH_SENSE_COOLDOWN 2

#define KILLER_DAMAGE_COST 0 * ENERGY_DENSITY_MULTIPLIER

#define ARMOR_HEALTH_BONUS 4 * MAX_HEALTH_MULTIPLIER

#define MAX_EYE_SEEING_DISTANCE 10

class Organism;

class Cell
{
public:
	Organism *myOrganism = nullptr;
	enum CellTypes type = cell_null;
	int x = -1;
	int y = -1;

	virtual ~Cell() = 0;

	virtual void Tick() = 0;

	virtual Cell *Clone() = 0;
};

Cell *GenerateRandomCell();

class Spoilable_Cell : public Cell
{
private:
	int *ticksUntilSpoil_;
	int startingTicksUntilSpoil;

public:

	const int &TicksUntilSpoil();

	void attachTicksUntilSpoil(int *slotValue);

	explicit Spoilable_Cell(int _startingTicksUntilSpoil);
};

class Cell_Empty : public Cell
{
public:
	~Cell_Empty() override;

	Cell_Empty();

	void Tick() override;

	Cell_Empty *Clone() override;
};

class Cell_Plantmass : public Spoilable_Cell
{
public:
	~Cell_Plantmass() override;

	Cell_Plantmass();

	explicit Cell_Plantmass(int _ticksUntilSpoil);

	void Tick() override;

	Cell_Plantmass *Clone() override;
};

class Cell_Biomass : public Spoilable_Cell
{
public:
	~Cell_Biomass() override;

	Cell_Biomass();

	explicit Cell_Biomass(int _ticksUntilSpoil);

	void Tick() override;

	Cell_Biomass *Clone() override;
};

class Cell_Leaf : public Cell
{
	friend class Cell_Herbivore;
	friend class Cell_Bark;
	friend class Board;
	int flowerCooldown;
	bool flowering;

public:
	~Cell_Leaf() override;

	Cell_Leaf();

	explicit Cell_Leaf(int floweringPercent);

	explicit Cell_Leaf(Organism *_myOrganism);

	// Cell_Leaf(const Cell_Leaf &c);

	void Tick() override;

	Cell_Leaf *Clone() override;
};

class Cell_Bark : public Cell
{
	friend class Organism;
	friend class Cell_Herbivore;
	int actionCooldown;
	int integrity;

public:
	~Cell_Bark() override;

	Cell_Bark();

	explicit Cell_Bark(Organism *_myOrganism);

	void Tick() override;

	Cell_Bark *Clone() override;
};

class Cell_Flower : public Cell
{
	friend class Cell_Leaf;
	friend class Board;
	int bloomCooldown;

public:
	~Cell_Flower() override;

	Cell_Flower();

	// explicit Cell_Flower(Organism *_myOrganism);

	void Tick() override;

	Cell_Flower *Clone() override;
};

class Cell_Fruit : public Spoilable_Cell
{
public:
	int parentMutability;

public:
	~Cell_Fruit() override;

	Cell_Fruit();

	explicit Cell_Fruit(int parentMutability);

	explicit Cell_Fruit(Organism *_myOrganism);

	void Tick() override;

	Cell_Fruit *Clone() override;
};

class Cell_Mover : public Cell
{
public:
	~Cell_Mover() override;

	Cell_Mover();

	explicit Cell_Mover(Organism *_myOrganism);

	void Tick() override;

	Cell_Mover *Clone() override;
};

class Cell_Herbivore : public Cell
{
	uint8_t digestCooldown;

public:
	~Cell_Herbivore() override;

	Cell_Herbivore();

	explicit Cell_Herbivore(Organism *_myOrganism);

	void Tick() override;

	Cell_Herbivore *Clone() override;
};

class Cell_Carnivore : public Cell
{
	uint8_t digestCooldown;

public:
	int8_t direction;

	~Cell_Carnivore() override;

	Cell_Carnivore();

	explicit Cell_Carnivore(Organism *_myOrganism);

	void Tick() override;

	Cell_Carnivore *Clone() override;
};

class Cell_Killer : public Cell
{

public:
	~Cell_Killer() override;

	Cell_Killer();

	explicit Cell_Killer(Organism *_myOrganism);

	void Tick() override;

	Cell_Killer *Clone() override;
};

class Cell_Armor : public Cell
{

public:
	~Cell_Armor() override;

	Cell_Armor();

	explicit Cell_Armor(Organism *_myOrganism);

	void Tick() override;

	Cell_Armor *Clone() override;
};

class Cell_Touch : public Cell
{
	friend class Organism;
	friend class Board;
	int senseInterval;
	int senseCooldown;

public:
	~Cell_Touch() override;

	Cell_Touch();

	void Tick() override;

	Cell_Touch *Clone() override;

	int getSenseInterval() { return this->senseInterval; };
};

class Cell_Eye : public Cell
{
	friend class Organism;
	friend class Board;
	int senseInterval;
	int senseCooldown;
	int direction;

public:
	~Cell_Eye() override;

	Cell_Eye();

	void Tick() override;

	Cell_Eye *Clone() override;

	int getSenseInterval() { return this->senseInterval; };
};
