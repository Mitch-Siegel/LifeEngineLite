#include "config.h"
#include "brain.h"

#include <vector>
#include <cstdint>

#pragma once

class Cell;

class Organism
{
private:
	std::size_t currentHealth, maxHealth;
	std::size_t currentEnergy, maxEnergy;

public:
	int x = -1;
	int y = -1;
	std::size_t age;
	int mutability;
	bool alive;
	std::vector<Cell *> myCells;
	int reproductionCooldown;
	std::size_t lifespan;
	Brain brain;
	int cellCounts[cell_null];

	Organism(int center_x, int center_y);

	void Die();

	void Remove();

	Organism *Tick();

	void RecalculateStats();

	bool CheckValidity();

	void Move();

	void Rotate(bool clockwise);

	int AddCell(int x_rel, int y_rel, Cell *_cell);

	void RemoveCell(Cell *_myCell);

	void ReplaceCell(Cell *_myCell, Cell *_newCell);

	std::size_t GetMaxHealth();

	std::size_t GetEnergy();

	std::size_t GetMaxEnergy();

	void Damage(std::size_t n);

	void Heal(std::size_t n);

	void ExpendEnergy(std::size_t n);

	void AddEnergy(std::size_t n);

	bool CanOccupyPosition(int _x_abs, int _y_abs);

	Organism *Reproduce();

	void Mutate();
};

class Organism;

#define DEFAULT_MUTABILITY 15

// as proportion of max energy
#define REPRODUCTION_ENERGY_MULTIPLIER .75
#define REPRODUCTION_COOLDOWN_MULTIPLIER 0.25
// lifespan related to sqare root of energy density
#define LIFESPAN_MULTIPLIER 150
#define ENERGY_DENSITY_MULTIPLIER 5
#define MAX_HEALTH_MULTIPLIER 1

#define HERB_FOOD_MULTIPLIER 8
// #define HERB_FOOD_MULTIPLIER 15
#define HERB_DIGEST_TIME 6
// #define HERB_DIGEST_TIME 0

#define LEAF_FOOD_ENERGY 3 * HERB_FOOD_MULTIPLIER
#define FLOWER_FOOD_ENERGY 4 * HERB_FOOD_MULTIPLIER
#define FRUIT_FOOD_ENERGY 15 * HERB_FOOD_MULTIPLIER

#define FRUIT_SPOIL_TIME 30
// must roll 2x in a row
#define FRUIT_GROW_PERCENT 15
// if the fruit grows, percent probability it will mutate vs just becoming another plant
#define FRUIT_MUTATE_PERCENT 15

#define SPOILTIME_BASE 100
#define PLANTMASS_SPOIL_TIME_MULTIPLIER 3 * SPOILTIME_BASE
// #define BIOMASS_SPOIL_TIME_MULTIPLIER 5 * SPOILTIME_BASE
#define BIOMASS_SPOIL_TIME_MULTIPLIER 0 * SPOILTIME_BASE

#define PLANTMASS_FOOD_ENERGY 4 * HERB_FOOD_MULTIPLIER
#define BIOMASS_FOOD_ENERGY 18 * PLANTMASS_FOOD_ENERGY
#define CARN_DIGEST_TIME 50

#define FLOWER_COST 8 * ENERGY_DENSITY_MULTIPLIER
#define LEAF_COST 2 * ENERGY_DENSITY_MULTIPLIER

// whether or not a leaf is able to flower, rolled at creation
#define LEAF_FLOWERING_ABILITY_PERCENT 60
// must roll this percent 2x to grow a flower on a leaf that can
#define PLANT_GROW_PERCENT 28
// percent for a flower to wilt into another leaf vs just going away
#define FLOWER_EXPAND_PERCENT 100

#define FLOWER_BLOOM_COOLDOWN 150
#define FLOWER_WILT_CHANCE 35
#define FLOWER_BLOOM_COST 5 * ENERGY_DENSITY_MULTIPLIER

#define KILLER_DAMAGE_COST ENERGY_DENSITY_MULTIPLIER

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

class Cell_Empty : public Cell
{
public:
	~Cell_Empty() override;

	Cell_Empty();

	void Tick() override;

	Cell_Empty *Clone() override;
};

class Cell_Plantmass : public Cell
{
public:
	int ticksUntilSpoil;

public:
	~Cell_Plantmass() override;

	Cell_Plantmass();

	explicit Cell_Plantmass(int _ticksUntilSpoil);

	void Tick() override;

	Cell_Plantmass *Clone() override;
};

class Cell_Biomass : public Cell
{
public:
	int ticksUntilSpoil;

public:
	~Cell_Biomass() override;

	Cell_Biomass();

	explicit Cell_Biomass(int _ticksUntilSpoil);

	void Tick() override;

	Cell_Biomass *Clone() override;
};

class Cell_Leaf : public Cell
{
	bool flowering;

public:
	~Cell_Leaf() override;

	Cell_Leaf();

	Cell_Leaf(int floweringPercent);

	explicit Cell_Leaf(Organism *_myOrganism);

	// Cell_Leaf(const Cell_Leaf &c);

	void Tick() override;

	Cell_Leaf *Clone() override;
};

class Cell_Flower : public Cell
{
	int bloomCooldown;

public:
	~Cell_Flower() override;

	Cell_Flower();

	explicit Cell_Flower(Organism *_myOrganism);

	void Tick() override;

	Cell_Flower *Clone() override;
};

class Cell_Fruit : public Cell
{
public:
	int ticksUntilSpoil;
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
	int8_t direction;

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
