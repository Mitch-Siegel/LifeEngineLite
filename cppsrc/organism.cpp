#include "organism.h"
#include "curses.h"
#include <stdlib.h>
#include <vector>

int directions[4][2] = {{1, 0}, {0, -1}, {-1, 0}, {0, 1}};
extern Cell *board[BOARD_DIM][BOARD_DIM];
extern std::vector<Organism *> Organisms;

int boundCheckPos(int x, int y)
{
	if (0 > x || 0 > y || x >= BOARD_DIM || y >= BOARD_DIM)
	{
		return 1;
	}
	return 0;
}

int isCellOfType(int x, int y, enum CellTypes type)
{
	if (boundCheckPos(x, y))
	{
		return 0;
	}

	return board[y][x]->type == type;
}

Organism::Organism(int center_x, int center_y)
{
	this->x = center_x;
	this->y = center_y;
	this->nCells = 0;
	this->myCells = nullptr;
	this->maxHealth = nCells;
	this->currentHealth = this->maxHealth;
	this->energy = 0;
	this->age = 0;
	this->alive = 1;
	this->reproductionCooldown = 0;
	this->justBorn = true;
}

void Organism::Die()
{
	for (int i = 0; i < this->nCells; i++)
	{
		board[this->myCells[i]->y][this->myCells[i]->x] = new Cell(this->myCells[i]->x, this->myCells[i]->y, cell_food, nullptr);
		delete this->myCells[i];
	}
	this->alive = 0;
}

Organism *Organism::Tick()
{
	if(this->justBorn)
	{
		this->justBorn = false;
		return nullptr;
	}

	// if (this->energy == 0 || this->currentHealth == 0)
	// {
	// this->Die();
	// return;
	// }

	for (int i = 0; i < this->nCells; i++)
	{
		this->myCells[i]->Tick();
	}
	this->age++;

	if (this->reproductionCooldown == 0)
	{
		if (this->energy >= this->nCells * 10)
		{
			return this->Reproduce();
		}
	}
	else
	{
		this->reproductionCooldown--;
	}
	return nullptr;
}

void Organism::ExpendEnergy(int n)
{
	this->energy -= n;
}

Organism *Organism::Reproduce()
{
	int max_rel_x = 1;
	int max_rel_y = 1;
	for (int i = 0; i < this->nCells; i++)
	{
		Cell *thisCell = this->myCells[i];
		int this_rel_x = this->x - thisCell->x;
		int this_rel_y = this->y - thisCell->y;
		max_rel_x = (this_rel_x > max_rel_x) ? this_rel_x : max_rel_x;
		max_rel_y = (this_rel_x > max_rel_y) ? this_rel_y : max_rel_y;
	}

	int index = (rand() >> 5) % 4;
	int dir_x = directions[index][0] * max_rel_x;
	int dir_y = directions[index][1] * max_rel_y;
	// int baby_offset_x = (((rand() >> 5) % 4 == 0) + ((rand() >> 5) % 8 == 0) + ((rand() >> 5) % 16 == 0)) * directions[index][0];
	// int baby_offset_y = (((rand() >> 5) % 4 == 0) + ((rand() >> 5) % 8 == 0) + ((rand() >> 5) % 16 == 0)) * directions[index][1];
	int baby_offset_x = 0;
	int baby_offset_y = 0;
	char canReproduceHere = 1;
	for (int i = 0; i < this->nCells; i++)
	{
		Cell *thisCell = this->myCells[i];
		canReproduceHere &= isCellOfType(thisCell->x + dir_x + baby_offset_x, thisCell->y + dir_y + baby_offset_y, cell_empty);
	}

	if (canReproduceHere)
	{
		Organism *replicated = new Organism(this->x + dir_x + baby_offset_x, this->y + dir_y + baby_offset_y);
		for (int i = 0; i < this->nCells; i++)
		{
			Cell *thisCell = this->myCells[i];
			int this_rel_x = thisCell->x - this->x;
			int this_rel_y = thisCell->y - this->y;
			replicated->AddCell(this_rel_x + baby_offset_x, this_rel_y + baby_offset_y, thisCell->type);
		}
		this->ExpendEnergy(this->nCells * 5);
		replicated->energy = 0;
		this->reproductionCooldown = this->nCells * 5;
		replicated->reproductionCooldown = replicated->nCells * 10;
		return replicated;
	}
	return nullptr;
	// Organism *new = new Organism(this-)
}

// return 1 if cell is occupied, else 0
int Organism::AddCell(int x_rel, int y_rel, enum CellTypes type)
{
	int x_abs = this->x + x_rel;
	int y_abs = this->y + y_rel;
	if (!isCellOfType(x_abs, y_abs, cell_empty))
	{
		return 1;
	}

	Cell **newMyCells = new Cell *[this->nCells + 1];
	for (int i = 0; i < this->nCells; i++)
	{
		newMyCells[i] = this->myCells[i];
	}

	delete board[y_abs][x_abs];
	board[y_abs][x_abs] = new Cell(x_abs, y_abs, type, this);
	newMyCells[this->nCells++] = board[y_abs][x_abs];
	if (this->myCells != nullptr)
	{
		delete[] this->myCells;
	}
	this->myCells = newMyCells;

	return 0;
}

Cell::Cell()
{
	this->actionCooldown = 0;
}

Cell::Cell(int x, int y, enum CellTypes type, Organism *myOrganism)
{
	this->x = x;
	this->y = y;
	this->type = type;
	this->myOrganism = myOrganism;
	this->actionCooldown = 0;
}

void Cell::Tick()
{
	
	if(this->actionCooldown > 0)
	{
		this->actionCooldown--;
		return;
	}

	switch (this->type)
	{
	case cell_empty:
		break;

	case cell_mouth:
	{
		/*
		char couldEat = 0;
		for (int i = 0; i < 4; i++)
		{
			int x_abs = this->x + directions[i][0];
			int y_abs = this->y + directions[i][1];
			if (isCellOfType(x_abs, y_abs, cell_food))
			{
				delete board[y_abs][x_abs];

				board[y_abs][x_abs] = new Cell(x_abs, y_abs, cell_empty, nullptr);
				couldEat = 1;
				break;
			}
		}
		if (couldEat)
		{
			this->actionCooldown = 2;
			this->myOrganism->energy += 20;
		}*/
	}
	break;

	/*
	case cell_producer:

		if (this->myOrganism->energy > 15)
		{
			char couldPlace = 0;
			int index = (rand() >> 5) % 4;
			int dir = ((rand() >> 6) % 2 == 0) ? 1 : -1;
			for (int i = 0; i < 4; i++)
			{
				int x_abs = this->x + directions[index][0];
				int y_abs = this->y + directions[index][1];
				if (isCellOfType(x_abs, y_abs, cell_empty))
				{
					delete board[y_abs][x_abs];
					board[y_abs][x_abs] = new Cell(x_abs, y_abs, cell_food, nullptr);
					couldPlace = 1;
					break;
				}
				index += dir;
				index %= 4;
				if (index < 0)
				{
					index += 4;
				}
			}
			if (couldPlace)
			{
				this->actionCooldown = 10;
				this->myOrganism->ExpendEnergy(15);
			}
		}
		break;
		*/
	case cell_leaf:
		this->myOrganism->energy++;
		this->actionCooldown = 1;
		break;

	case cell_flower:
		break;

	case cell_food:
		break;
	}
}
