#include "cell.h"

class Organism
{
	int nCells;
	int age;
	int currentHealth, maxHealth;
	int energy;
	Cell *myCells;
	
	Organism(int newCellCount, Cell *newCells);

	void Die();

	void Tick();

};
