#pragma once
#define BOARD_DIM 20

class Cell;

enum CellTypes
{
	cell_empty,
	cell_food,
	cell_leaf,
	cell_flower,
	cell_mouth,	
};

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
	bool justBorn;
	
	Organism(int center_x, int center_y);

	void Die();

	Organism* Tick();

	int AddCell(int x_rel, int y_rel, enum CellTypes type);

	void ExpendEnergy(int n);

	Organism* Reproduce();

};





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

