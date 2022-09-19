#include "curses.h"
#include <stdlib.h>
#include <vector>
#include <cmath>

#include "lifeforms.h"
#include "board.h"
#include "rng.h"

extern Board board;
Organism::Organism(int center_x, int center_y)
{
	this->x = center_x;
	this->y = center_y;
	// this->myCells = std::vector<Cell &>();
	this->maxHealth = 0;
	this->currentHealth = this->maxHealth;
	this->currentEnergy = 0;
	this->maxEnergy = 0;
	this->age = 0;
	this->alive = 1;
	this->reproductionCooldown = 0;
	this->canMove = false;
	this->hasFlower = false;
}

void Organism::Die()
{
	for (size_t i = 0; i < this->myCells.size(); i++)
	{
		Cell *thisCell = this->myCells[i];
		board.replaceCell(thisCell, new Cell_Biomass(10));
	}
	this->myCells.clear();
	this->alive = false;
}

void Organism::Remove()
{

	for (size_t i = 0; i < this->myCells.size(); i++)
	{
		Cell *thisCell = this->myCells[i];
		board.replaceCellAt(thisCell->x, thisCell->y, new Cell_Empty());
	}
	this->alive = false;
}

Organism *Organism::Tick()
{
	/*if (this->CheckValidity())
	{
		return nullptr;
	}*/
	/*if (this->myCells.size() > 2)
	{
		this->ExpendEnergy(this->myCells.size() - 1);
	}
	else
	{*/
	this->ExpendEnergy(1);
	//}
	// if (this->myCells.size() > 1)
	// {
	// this->ExpendEnergy(this->myCells.size() - 1);
	// }
	// }
	if (this->currentEnergy == 0 /*|| this->currentHealth == 0*/ || this->lifespan == 0 || this->myCells.size() == 0)
	{
		this->Die();
		return nullptr;
	}
	this->lifespan--;

	for (size_t i = 0; i < this->myCells.size(); i++)
	{
		this->myCells[i]->Tick();
	}
	this->age++;

	if (this->canMove)
	{
		this->brain.Decide();
		this->Move();
	}

	// don't allow organisms of size 1 to reproduce
	if (this->reproductionCooldown == 0 /* && (this->myCells.size() > 1 || board.Organisms.size() < 3)*/)
	{
		if (this->currentEnergy > ((this->myCells.size() + 1) * REPRODUCTION_ENERGY_MULTIPLIER))
		{
			return this->Reproduce();
		}
	}
	else
	{
		if (this->reproductionCooldown > 0)
		{
			this->reproductionCooldown--;
		}
	}
	return nullptr;
}

void Organism::RecalculateStats()
{
	this->canMove = false;
	for (size_t i = 0; i < this->myCells.size(); i++)
	{
		if (this->myCells[i]->type == cell_mover)
		{
			this->canMove = true;
			break;
		}
	}

	this->maxHealth = this->myCells.size() * MAX_HEALTH_MULTIPLIER;
	this->currentEnergy = randInt(1, (this->myCells.size() * MAX_ENERGY_MULTIPLIER) / 3);
	this->maxEnergy = this->myCells.size() * MAX_ENERGY_MULTIPLIER;
}

// disallow specific types of organisms from existing
// return true if invalid

bool Organism::CheckValidity()
{
	bool allMouths = true;
	for (size_t i = 0; i < this->myCells.size(); i++)
	{
		allMouths &= (this->myCells[0]->type == cell_herbivore_mouth);
	}
	return allMouths;
}

void Organism::Move()
{
	int *moveDir = directions[this->brain.moveDirIndex];
	if (this->CanOccupyPosition(this->x + moveDir[0], this->y + moveDir[1]))
	{
		for (size_t i = 0; i < this->myCells.size(); i++)
		{
			Cell *movedCell = this->myCells[i];
			int newX = movedCell->x + moveDir[0];
			int newY = movedCell->y + moveDir[1];
			board.swapCellAtIndex(newX, newY, movedCell);
		}
	}
	else
	{
		this->brain.Punish();
	}
}

void Organism::ExpendEnergy(size_t n)
{
	if (n > this->currentEnergy)
	{
		this->currentEnergy = 0;
		return;
	}

	this->currentEnergy -= n;
}

void Organism::AddEnergy(size_t n)
{
	this->currentEnergy += n;
	if (this->currentEnergy > this->maxEnergy)
	{
		this->currentEnergy = this->maxEnergy;
	}
}

void Organism::CalculateMaxEnergy()
{
	this->maxEnergy = this->myCells.size() * MAX_ENERGY_MULTIPLIER;
}

std::size_t Organism::GetEnergy()
{
	return this->currentEnergy;
}

std::size_t Organism::GetMaxEnergy()
{
	return this->maxEnergy;
}

bool Organism::CanOccupyPosition(int _x_abs, int _y_abs)
{
	for (size_t i = 0; i < this->myCells.size(); i++)
	{
		Cell *thisCell = this->myCells[i];

		int newCellX_rel = thisCell->x - this->x;
		int newCellY_rel = thisCell->y - this->y;
		if (!board.isCellOfType(_x_abs + newCellX_rel, _y_abs + newCellY_rel, cell_empty))
		{
			return false;
		}
	}
	return true;
}

Organism *Organism::Reproduce()
{
	this->reproductionCooldown = this->myCells.size() * REPRODUCTION_COOLDOWN_MULTIPLIER;

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

	int index = randInt(0, 3);
	int dir_x = directions[index][0] * max_rel_x * 2;
	int dir_y = directions[index][1] * max_rel_y * 2;
	dir_x += (randInt(-1, 1)) * (randPercent(50));
	dir_y += (randInt(-1, 1)) * (randPercent(50));
	int baby_offset_x = 0;
	int baby_offset_y = 0;

	if (this->CanOccupyPosition(this->x + dir_x, this->y + dir_y))
	{
		this->ExpendEnergy(this->myCells.size() * REPRODUCTION_ENERGY_MULTIPLIER);

		Organism *replicated = new Organism(this->x + dir_x + baby_offset_x, this->y + dir_y + baby_offset_y);
		for (size_t i = 0; i < this->myCells.size(); i++)
		{
			Cell *thisCell = this->myCells[i];
			switch (thisCell->type)
			{
			case cell_flower:
			case cell_fruit:
				break;

			default:
				int this_rel_x = thisCell->x - this->x;
				int this_rel_y = thisCell->y - this->y;
				Cell *replicatedCell = thisCell->Clone();
				replicatedCell->myOrganism = replicated;
				replicated->AddCell(this_rel_x + baby_offset_x, this_rel_y + baby_offset_y, replicatedCell);
			}
		}

		if (randPercent(20))
		{
			replicated->Mutate();
		}

		if (replicated->CheckValidity())
		{
			replicated->Remove();
			delete replicated;
			return nullptr;
		}

		replicated->RecalculateStats();
		replicated->reproductionCooldown = replicated->myCells.size() * REPRODUCTION_COOLDOWN_MULTIPLIER * 2;
		replicated->lifespan = replicated->myCells.size() * LIFESPAN_MULTIPLIER;

		return replicated;
	}
	return nullptr;
}

// random generation using this method is super janky:
// TODO: improve this
void Organism::Mutate()
{
	// change existing cell
	if (randPercent(30) && this->myCells.size() > 1)
	{
		int switchedIndex = randInt(0, this->myCells.size() - 1);

		Cell *toReplace = this->myCells[switchedIndex];
		this->myCells.erase(std::find(this->myCells.begin(), this->myCells.end(), toReplace));

		Cell *replacedWith = GenerateRandomCell();
		replacedWith->myOrganism = this;

		board.replaceCell(toReplace, replacedWith);
		this->myCells.push_back(replacedWith);
		// int cellIndex = (rand() >> 5) % this->myCells.size();
		// this->myCells[cellIndex].type = (enum CellTypes)((rand() >> 5) % (int)cell_mouth);
	}
	else
	{
		// remove a cell
		if (randPercent(50) && this->myCells.size() > 2)
		{
			Cell *toRemove = this->myCells[randInt(0, this->myCells.size() - 1)];
			this->myCells.erase(std::find(this->myCells.begin(), this->myCells.end(), toRemove));
			board.replaceCell(toRemove, new Cell_Empty());
		}
		// add a cell
		else
		{
			int x_rel = 0;
			int y_rel = 0;
			bool couldAdd = 0;
			size_t nTries = 0;
			int prevDirectionIndex = -1;

			// this is wildly inefficient but it's an easy way to choose a random position and ensure it stays in bounds
			while (!couldAdd && nTries < this->myCells.size())
			{
				int thisDirectionIndex = randInt(0, 3);
				// make sure we don't immediately choose an opposite direction
				while (thisDirectionIndex + 2 % 4 == prevDirectionIndex)
				{
					thisDirectionIndex = randInt(0, 3);
				}
				nTries++;

				prevDirectionIndex = thisDirectionIndex;
				int *thisDirection = directions[thisDirectionIndex];
				x_rel += thisDirection[0];
				y_rel += thisDirection[1];
				int x_abs = this->x + x_rel;
				int y_abs = this->y + y_rel;
				if (board.boundCheckPos(x_abs, y_abs))
				{
					x_rel -= thisDirection[0];
					y_rel -= thisDirection[1];
					continue;
				}
				else
				{
					if (board.isCellOfType(x_abs, y_abs, cell_empty))
					{
						couldAdd = true;
					}
				}
			}

			if (couldAdd)
			{
				this->AddCell(x_rel, y_rel, GenerateRandomCell());
			}
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
	board.replaceCellAt(x_abs, y_abs, _cell);
	this->myCells.push_back(_cell);
	this->canMove |= (_cell->type == cell_mover);

	return 0;
}

void Organism::RemoveCell(Cell *_myCell)
{
	std::vector<Cell *>::iterator cellIterator = std::find(this->myCells.begin(), this->myCells.end(), _myCell);
	if (cellIterator == this->myCells.end())
	{
		std::cerr << "Bad call to remove cell with _myCell not from this organism!" << std::endl;
		exit(1);
	}
	this->myCells.erase(cellIterator);
}

void Organism::ReplaceCell(Cell *_myCell, Cell *_newCell)
{
	this->RemoveCell(_myCell);
	_newCell->myOrganism = this;
	this->myCells.push_back(_newCell);
	// int x_rel = _myCell->x - this->x;
	// int y_rel = _myCell->y - this->y;
	board.replaceCell(_myCell, _newCell);
	// this->AddCell(x_rel, y_rel, _newCell);
}
