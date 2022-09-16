#include "config.h"

#include <vector>

#pragma once

enum CellTypes;

class Cell;

class Organism
{
public:
	int x, y;
	std::size_t age;
	std::size_t currentHealth, maxHealth;
	std::size_t energy;
	bool alive;
	std::vector<Cell *> myCells;
	int reproductionCooldown;
	int lifespan;

	Organism(int center_x, int center_y);

	void Die();

	Organism *Tick();

	int AddCell(int x_rel, int y_rel, Cell _cell);

	void ExpendEnergy(int n);

	Organism *Reproduce();

	void Mutate();
};

class Organism;

#define REPRODUCTION_MULTIPLIER 11
#define LIFESPAN_MULTIPLIER 75

#define FLOWER_COOLDOWN 15
#define FOOD_SPOILTIME 10
#define FRUIT_SPOILTIME 20

#define MUTATE_PROBABILITY 0.99


class Cell
{
public:
	Organism *myOrganism;
	enum CellTypes type;
	int x, y;

	//Cell();
	// Cell(int _x, int _y, enum CellTypes _type, Organism *_myOrganism);

	// Cell(enum CellTypes _type, Organism *_myOrganism);

	virtual void Tick();

	virtual Cell *Clone();
};


// template <class Cell>

class Cell_Empty : public virtual Cell
{
public:
	Cell_Empty();
	// Cell_Empty(/*int _x, int _y*/);

	virtual void Tick();

	Cell_Empty *Clone();
};

class Cell_Food : public virtual Cell
{
	int ticksUntilSpoil;

public:
	Cell_Food();

	Cell_Food(/*int _x, int _y, */int _ticksUntilSpoil);

	virtual void Tick();
};

class Cell_Leaf : public virtual Cell
{
	int photosynthesisCooldown;

public:
	Cell_Leaf();

	Cell_Leaf(/*int _x, int _y, */Organism *_myOrganism);

	Cell_Leaf(const Cell_Leaf &c);

	virtual void Tick();

	Cell_Leaf *Clone();
};
