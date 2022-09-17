#include "lifeforms.h"
#include "board.h"
#include "curses.h"
#include <stdlib.h>
#include <vector>

extern Board board;
Organism::Organism(int center_x, int center_y)
{
	this->x = center_x;
	this->y = center_y;
	// this->myCells = std::vector<Cell &>();
	this->maxHealth = 0;
	this->currentHealth = this->maxHealth;
	this->energy = 0;
	this->age = 0;
	this->alive = 1;
	this->reproductionCooldown = 0;
}


void Organism::Die()
{
	for (size_t i = 0; i < this->myCells.size(); i++)
	{
		Cell *thisCell = this->myCells[i];
		board.replaceCellAt(thisCell->x, thisCell->y, new Cell_Food(10));
	}
	this->alive = 0;
}

Organism *Organism::Tick()
{
	if (/*this->energy == 0 || this->currentHealth == 0 || */ this->lifespan == 0)
	{
		this->Die();
		return nullptr;
	}
	this->lifespan--;

	for (size_t i = 0; i < this->myCells.size(); i++)
	{
		// mvprintw(25 + i, 0, "Ticking cell %lu (type %d)", i, this->myCells[i]->type);
		this->myCells[i]->Tick();
	}
	this->age++;
	
	if (this->reproductionCooldown == 0)
	{
		if (this->energy > ((this->myCells.size() + 1) * REPRODUCTION_MULTIPLIER))
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
	for (size_t i = 0; i < this->myCells.size(); i++)
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
	dir_x += ((rand() >> 5) % 3 - 1) * ((rand() >> 5) % 2 == 0);
	dir_y += ((rand() >> 5) % 3 - 1) * ((rand() >> 5) % 2 == 0);
	// int baby_offset_x = (((rand() >> 5) % 4 == 0) + ((rand() >> 5) % 8 == 0) + ((rand() >> 5) % 16 == 0)) * directions[index][0];
	// int baby_offset_y = (((rand() >> 5) % 4 == 0) + ((rand() >> 5) % 8 == 0) + ((rand() >> 5) % 16 == 0)) * directions[index][1];
	int baby_offset_x = 0;
	int baby_offset_y = 0;
	bool canReproduceHere = true;
	for (size_t i = 0; i < this->myCells.size(); i++)
	{
		Cell *thisCell = this->myCells[i];
		canReproduceHere &= board.isCellOfType(thisCell->x + dir_x + baby_offset_x, thisCell->y + dir_y + baby_offset_y, cell_empty);
	}

	if (canReproduceHere)
	{
		Organism *replicated = new Organism(this->x + dir_x + baby_offset_x, this->y + dir_y + baby_offset_y);
		for (size_t i = 0; i < this->myCells.size(); i++)
		{
			Cell *thisCell = this->myCells[i];
			int this_rel_x = thisCell->x - this->x;
			int this_rel_y = thisCell->y - this->y;
			Cell *replicatedCell =thisCell->Clone();
			replicatedCell->myOrganism = replicated;
			replicated->AddCell(this_rel_x + baby_offset_x, this_rel_y + baby_offset_y, replicatedCell);
		}
		this->ExpendEnergy(this->myCells.size() * REPRODUCTION_MULTIPLIER);
		replicated->energy = replicated->myCells.size() * 5;
		this->reproductionCooldown = this->myCells.size() * 3;
		replicated->reproductionCooldown = replicated->myCells.size() * 5;
		replicated->lifespan = replicated->myCells.size() * LIFESPAN_MULTIPLIER;

		// mutate with 50% probability for testing
		if (rand() % 2 == 0)
		{
			replicated->Mutate();
		}

		return replicated;
	}
	return nullptr;
}

// random generation using this method is super janky: 
// TODO: improve this
void Organism::Mutate()
{
	// change existing cell
	if (rand() % 2 == 0 && this->myCells.size() > 1)
	{
		// int cellIndex = (rand() >> 5) % this->myCells.size();
		// this->myCells[cellIndex].type = (enum CellTypes)((rand() >> 5) % (int)cell_mouth);
	}
	else
	{
		// remove a cell
		if (rand() % 2 == 0 && this->myCells.size() > 1)
		{
		}
		// add a cell
		else
		{
			/*
			char couldAdd = 0;
			int cellIndex = (rand() >> 5) % this->nCells;
			while(!couldAdd)
			{
				int dirIndex = rand() >> 4;
				for (int i = 0; i < 4; i++)
				{
					int x_abs = this->x + directions[dirIndex][0];
					int y_abs = this->y + directions[dirIndex][1];
					if (isCellOfType(x_abs, y_abs, cell_empty))
					{
						delete board[y_abs][x_abs];

						board[y_abs][x_abs] = new Cell(x_abs, y_abs, cell_empty, nullptr);
						couldAdd = 1;
						break;
					}
					++dirIndex %= 4;
				}
				++cellIndex %= this->nCells;
			}
			*/
		}
	}
}

// return 1 if cell is occupied, else 0
int Organism::AddCell(int x_rel, int y_rel, Cell *_cell)
{
	int x_abs = this->x + x_rel;
	int y_abs = this->y + y_rel;
	if (!board.isCellOfType(x_abs, y_abs, cell_empty))
	{
		return 1;
	}

	_cell->x = x_abs;
	_cell->y = y_abs;
	_cell->myOrganism = this;

	this->myCells.push_back(board.replaceCellAt(x_abs, y_abs, _cell));

	return 0;
}
