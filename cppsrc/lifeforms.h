#include "config.h"
#include "brain.h"

#include <vector>

#pragma once

// enum CellTypes;

class Cell;

class Organism
{
private:
	std::size_t currentHealth, maxHealth;
	std::size_t currentEnergy, maxEnergy;

public:
	int x, y;
	std::size_t age;

	bool alive;
	bool canMove;
	std::vector<Cell *> myCells;
	int reproductionCooldown;
	int lifespan;
	Brain brain;

	Organism(int center_x, int center_y);

	void Die();

	void Remove();

	Organism *Tick();

	void Move();

	int AddCell(int x_rel, int y_rel, Cell *_cell);

	void ReplaceCell(Cell *_myCell, Cell *_newCell);
	
	std::size_t GetEnergy();

	void ExpendEnergy(std::size_t n);

	void AddEnergy(std::size_t n);

	bool CanOccupyPosition(int _x_abs, int _y_abs);

	Organism *Reproduce();

	void Mutate();
};

class Organism;

#define REPRODUCTION_MULTIPLIER 7
#define LIFESPAN_MULTIPLIER 75
#define MAX_ENERGY_MULTIPLIER 15

#define FLOWER_COOLDOWN 15
#define FOOD_SPOILTIME 10
#define FRUIT_SPOILTIME 20

#define MUTATE_PROBABILITY 0.99

#include <curses.h>

class Cell
{
public:
	Organism *myOrganism;
	enum CellTypes type;
	int x, y;

	virtual ~Cell() = 0;

	virtual void Tick() = 0;

	virtual Cell *Clone() = 0;
};

// template <class Cell>
class Cell_Empty : public Cell
{
public:
	~Cell_Empty() override;

	Cell_Empty();
	// Cell_Empty();

	void Tick() override;

	Cell_Empty *Clone() override;
};

class Cell_Biomass : public Cell
{
public:
	int ticksUntilSpoil;

public:
	~Cell_Biomass() override;

	Cell_Biomass();

	Cell_Biomass(int _ticksUntilSpoil);

	void Tick() override;

	Cell_Biomass *Clone() override;
};

class Cell_Leaf : public Cell
{
	int photosynthesisCooldown;

public:
	~Cell_Leaf() override;

	Cell_Leaf();

	Cell_Leaf(Organism *_myOrganism);

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

	Cell_Flower(Organism *_myOrganism);

	void Tick() override;

	Cell_Flower *Clone() override;
};

class Cell_Mover : public Cell
{
	// int bloomCooldown;

public:
	~Cell_Mover() override;

	Cell_Mover();

	Cell_Mover(Organism *_myOrganism);

	void Tick() override;

	Cell_Mover *Clone() override;
};

class Cell_Herbivore : public Cell
{

public:
	~Cell_Herbivore() override;

	Cell_Herbivore();

	Cell_Herbivore(Organism *_myOrganism);

	void Tick() override;

	Cell_Herbivore *Clone() override;
};
