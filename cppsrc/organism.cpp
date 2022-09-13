#include "organism.h"
#include "cell.h"

Organism::Organism(int newCellCount, Cell *newCells)
{
	this->nCells = newCellCount;
	this->myCells = newCells;
	this->maxHealth = nCells;
	this->currentHealth = this->maxHealth;
	this->energy = 5 * newCellCount;
	this->age = 0;
}

void Organism::Die()
{
	delete this->myCells;
}

void Organism::Tick()
{
	this->energy--;
	if(this->energy == 0 || this->currentHealth == 0)
	{
		this->Die();
	}

	for(int i = 0; i < this->nCells; i++)
	{
		this->myCells[i].Tick();
	}

}
