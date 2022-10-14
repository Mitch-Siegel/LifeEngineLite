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
	std::size_t cellCounts[cell_null];

	Organism(int center_x, int center_y);

	void Die();

	void Remove();

	Organism *Tick();

	void RecalculateStats();

	bool CheckValidity();

	void Move();

	void Rotate(bool clockwise);

	void AddCell(int x_rel, int y_rel, Cell *_cell);

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
	
	bool CanMoveToPosition(int _x_abs, int _y_abs);

	Organism *Reproduce();

	void Mutate();
};

class Organism;

#define DEFAULT_MUTABILITY 15

// as proportion of max energy
#define REPRODUCTION_ENERGY_MULTIPLIER .8
// #define REPRODUCTION_COOLDOWN_MULTIPLIER 3.5
#define REPRODUCTION_COOLDOWN_MULTIPLIER 0
// #define REPRODUCTION_COOLDOWN_MULTIPLIER 1.5
// lifespan related to sqare root of energy density
#define LIFESPAN_MULTIPLIER 200
#define ENERGY_DENSITY_MULTIPLIER 8
#define MAX_HEALTH_MULTIPLIER 1

// #define HERB_FOOD_MULTIPLIER 2.25 * ENERGY_DENSITY_MULTIPLIER
#define HERB_FOOD_MULTIPLIER 1.667 * ENERGY_DENSITY_MULTIPLIER
#define CARN_FOOD_MULTIPLIER 10 * ENERGY_DENSITY_MULTIPLIER

#define LEAF_FOOD_ENERGY 1
#define FLOWER_FOOD_ENERGY 2
#define FRUIT_FOOD_ENERGY 10

#define SPOILTIME_BASE 100
#define PLANTMASS_SPOIL_TIME_MULTIPLIER 2 * SPOILTIME_BASE
// #define BIOMASS_SPOIL_TIME_MULTIPLIER 5 * SPOILTIME_BASE
#define BIOMASS_SPOIL_TIME_MULTIPLIER 0 * SPOILTIME_BASE

#define FRUIT_SPOIL_TIME 0.5 * SPOILTIME_BASE

#define FRUIT_GROW_PERCENT 20
// if the fruit grows, percent probability it will mutate vs just becoming another plant
#define FRUIT_MUTATE_PERCENT 25

#define PLANTMASS_FOOD_ENERGY 2
#define BIOMASS_FOOD_ENERGY 0 * PLANTMASS_FOOD_ENERGY

#define FLOWER_COST 2 * ENERGY_DENSITY_MULTIPLIER

// whether or not a leaf is able to flower, rolled at creation
#define LEAF_FLOWERING_ABILITY_PERCENT 20

#define PLANT_GROW_PERCENT 100
// percent for a flower to wilt into another leaf vs just going away
#define FLOWER_EXPAND_PERCENT 100

#define BARK_GROW_COOLDOWN 30
#define BARK_GROW_COST 2 * ENERGY_DENSITY_MULTIPLIER
#define BARK_MAX_INTEGRITY 10
#define BARK_REGENERATE_INTEGRITY_COST 1 * ENERGY_DENSITY_MULTIPLIER

#define FLOWER_BLOOM_COOLDOWN 55
#define FLOWER_WILT_CHANCE 30
#define FLOWER_BLOOM_COST 1.5 * ENERGY_DENSITY_MULTIPLIER


#define TOUCH_SENSE_COOLDOWN 2

#define ARMOR_HEALTH_BONUS 4 * MAX_HEALTH_MULTIPLIER

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

class Cell_Flower;

class Cell_Leaf : public Cell
{
	friend class Cell_Flower;
	friend class Board;
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

class Cell_Bark : public Cell
{
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
	friend class Board;
	int senseInterval;
	int senseCooldown;

public:
	~Cell_Touch() override;

	Cell_Touch();

	void Tick() override;

	Cell_Touch *Clone() override;
};
