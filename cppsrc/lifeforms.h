#include "config.h"
#include "brain.h"

#include <vector>

#pragma once

// enum CellTypes;

class Cell;

class Organism
{
public:
	int x, y;
	std::size_t age;
	std::size_t currentHealth, maxHealth;
	std::size_t energy;
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

	void ExpendEnergy(int n);

	bool CanOccupyPosition(int _x_abs, int _y_abs);

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

class Cell_Food : public Cell
{
public:
	int ticksUntilSpoil;

public:
	~Cell_Food() override;

	Cell_Food();

	Cell_Food(int _ticksUntilSpoil);

	void Tick() override;

	Cell_Food *Clone() override;
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
