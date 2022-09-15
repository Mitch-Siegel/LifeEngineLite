#include "config.h"

#pragma once

enum CellTypes;

class Cell;

class Organism
{
	public:
	int x, y;
	int nCells;
	int age;
	int currentHealth, maxHealth;
	int energy;
	bool alive;
	Cell **myCells;
	int reproductionCooldown;
	int lifespan;
	
	Organism(int center_x, int center_y);

	void Die();

	Organism* Tick();

	int AddCell(int x_rel, int y_rel, enum CellTypes type);

	void ExpendEnergy(int n);

	Organism* Reproduce();

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
	enum CellTypes type;
	Organism *myOrganism;
	int x, y;
	int actionCooldown;

	Cell();

	Cell(int x, int y, enum CellTypes type, Organism *myOrganism);

	void Tick();
};




