#include "config.h"
#include "brain.h"

#include <vector>

#pragma once

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
	bool hasFlower;


	Organism(int center_x, int center_y);

	void Die();

	void Remove();

	Organism *Tick();

	bool CheckValidity();

	void Move();

	int AddCell(int x_rel, int y_rel, Cell *_cell);

	void RemoveCell(Cell *_myCell);

	void ReplaceCell(Cell *_myCell, Cell *_newCell);

	std::size_t GetEnergy();
	
	std::size_t GetMaxEnergy();

	void ExpendEnergy(std::size_t n);

	void AddEnergy(std::size_t n);

	void CalculateMaxEnergy();

	bool CanOccupyPosition(int _x_abs, int _y_abs);

	Organism *Reproduce();

	void Mutate();
};

class Organism;

#define REPRODUCTION_ENERGY_MULTIPLIER 1
#define REPRODUCTION_COOLDOWN_MULTIPLIER 15
#define LIFESPAN_MULTIPLIER 250
#define MAX_ENERGY_MULTIPLIER 15

#define BIOMASS_FOOD_ENERGY 1000

#define FLOWER_COST 55
#define FLOWER_PERCENT 2
#define FLOWER_MUTATION_PERCENT 2

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

Cell *GenerateRandomCell();

class Cell_Empty : public Cell
{
public:
	~Cell_Empty() override;

	Cell_Empty();

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
