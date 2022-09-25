#include "curses.h"
#include <stdlib.h>
#include <vector>
#include <unordered_map>
#include <cmath>

#include "lifeforms.h"
#include "board.h"
#include "rng.h"

extern Board board;
Organism::Organism(int center_x, int center_y)
{
	this->x = center_x;
	this->y = center_y;
	this->myCells = std::vector<Cell *>();
	this->maxHealth = 0;
	this->currentHealth = this->maxHealth;
	this->currentEnergy = 0;
	this->lifespan = 0;
	this->maxEnergy = 0;
	this->age = 0;
	this->alive = true;
	this->reproductionCooldown = 0;
	this->mutability = DEFAULT_MUTABILITY;
	for (int i = 0; i < cell_null; i++)
	{
		this->cellCounts[i] = 0;
	}
}

void Organism::Die()
{
	for (size_t i = 0; i < this->myCells.size(); i++)
	{
		Cell *thisCell = this->myCells[i];
		Cell *replacedWith;
		switch (thisCell->type)
		{
		case cell_null:
		case cell_empty:
		case cell_biomass:
		case cell_plantmass:
		case cell_fruit:
			std::cerr << "Wrong cell type contained within organism!" << std::endl;
			exit(1);

		case cell_leaf:
		case cell_flower:
			replacedWith = new Cell_Plantmass(this->myCells.size() * PLANTMASS_SPOIL_TIME_MULTIPLIER);
			break;

		case cell_mover:
		case cell_herbivore_mouth:
		case cell_carnivore_mouth:
			replacedWith = new Cell_Biomass(this->myCells.size() * BIOMASS_SPOIL_TIME_MULTIPLIER);
		}
		board.replaceCell(thisCell, replacedWith);
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
	if (this->myCells.size() > 1 || ((this->myCells.size() == 1) && (this->myCells[0]->type != cell_leaf)))
	{
		this->ExpendEnergy(1);
	}

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

	if (this->cellCounts[cell_mover])
	{
		if (this->currentEnergy > 2)
		{
			switch (this->brain.Decide())
			{
			case intent_changeDir:
			case intent_continue:
				this->Move();
				break;

			case intent_rotateClockwise:
				this->Rotate(true);
				break;

			case intent_rotateCounterClockwise:
				this->Rotate(false);
				break;
			}
		}
	}

	// don't allow organisms of size 1 to reproduce
	if (this->reproductionCooldown == 0 && this->myCells.size() > 1)
	{
		if (this->currentEnergy > ((this->maxEnergy * REPRODUCTION_ENERGY_MULTIPLIER) * 1.25))
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
	this->maxEnergy = 10;
	int calculatedMaxEnergy = 10;
	for (int i = 0; i < cell_null; i++)
	{
		cellCounts[i] = 0;
	}
	for (size_t i = 0; i < this->myCells.size(); i++)
	{
		cellCounts[this->myCells[i]->type]++;
		calculatedMaxEnergy += CellEnergyDensities[this->myCells[i]->type];
	}
	calculatedMaxEnergy *= ENERGY_DENSITY_MULTIPLIER;
	if (calculatedMaxEnergy < 0)
	{
		this->maxEnergy = 0;
	}
	else
	{
		this->maxEnergy = calculatedMaxEnergy;
	}
	this->maxHealth = this->myCells.size() * MAX_HEALTH_MULTIPLIER;
	if (this->lifespan > this->myCells.size() * LIFESPAN_MULTIPLIER)
	{
		this->lifespan = this->myCells.size() * LIFESPAN_MULTIPLIER;
	}
	if (this->currentEnergy > this->maxEnergy)
	{
		this->currentEnergy = this->maxEnergy;
	}
}

// disallow specific types of organisms from existing
// return true if invalid
bool Organism::CheckValidity()
{
	bool invalid = false;
	invalid |= (this->cellCounts[cell_herbivore_mouth] / (float)this->myCells.size()) > 0.667;
	invalid |= (this->cellCounts[cell_herbivore_mouth] > 0 && this->cellCounts[cell_leaf] > 0);
	return invalid;
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
		this->x += moveDir[0];
		this->y += moveDir[1];
	}
	else
	{
		this->brain.Punish();
	}
}

void Organism::Rotate(bool clockwise)
{
	// .first is this organism's cell
	// .second is the cell being swapped with
	std::vector<std::pair<Cell *, Cell *>> swaps;

	// track which cells we have looked at
	// prevents double-swapping if 2 cells from this organism just trade places
	std::unordered_map<Cell *, bool> swappedMap;

	for (Cell *c : this->myCells)
	{
		if (!swappedMap[c])
		{
			int x_rel = c->x - this->x;
			int y_rel = c->y - this->y;
			int new_x, new_y;

			if (clockwise)
			{
				new_x = this->x + (y_rel * -1);
				new_y = this->y + x_rel;
			}
			else
			{
				new_x = this->x + y_rel;
				new_y = this->y + (x_rel * -1);
			}

			if(board.boundCheckPos(new_x, new_y))
			{
				this->brain.Punish();
				return;
			}

			Cell *swappedWith = board.cells[new_y][new_x];
			// can't rotate!
			if (swappedWith->type != cell_empty && swappedWith->myOrganism != this)
			{
				this->brain.Punish();
				return;
			}

			swappedMap[c] = true;
			// redundant check but doesn't hurt
			if (!swappedMap[swappedWith])
			{
				swappedMap[swappedWith] = true;
				swaps.push_back(std::pair<Cell *, Cell *>(c, swappedWith));
			}
		}
	}

	for (std::pair<Cell *, Cell *> thisSwap : swaps)
	{
		Cell *a = thisSwap.first;
		Cell *b = thisSwap.second;
		int oldX = a->x;
		int oldY = a->y;
		a->x = b->x;
		a->y = b->y;
		board.cells[a->y][a->x] = a;

		b->x = oldX;
		b->y = oldY;
		board.cells[b->y][b->x] = b;
	}
	this->brain.RotateSuccess(clockwise);
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
	this->reproductionCooldown = (this->GetMaxEnergy() / ENERGY_DENSITY_MULTIPLIER) * REPRODUCTION_COOLDOWN_MULTIPLIER;

	int dirIndex = randInt(0, 7);
	for (int i = 0; i < 8; i++)
	{
		int *thisDir = directions[(dirIndex + i) % 8];
		for (int j = 1; j < ceil(sqrt(this->myCells.size())) + 1; j++)
		{
			int dir_x = thisDir[0] * j;
			int dir_y = thisDir[1] * j;

			if (this->CanOccupyPosition(this->x + dir_x, this->y + dir_y))
			{
				this->ExpendEnergy(this->maxEnergy * REPRODUCTION_ENERGY_MULTIPLIER);

				Organism *replicated = new Organism(this->x + dir_x, this->y + dir_y);
				replicated->mutability = this->mutability;
				for (size_t k = 0; k < this->myCells.size(); k++)
				{
					Cell *thisCell = this->myCells[k];
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
						replicated->AddCell(this_rel_x, this_rel_y, replicatedCell);
						break;
					}
				}

				if (randPercent(this->mutability))
				{
					replicated->Mutate();
				}
				replicated->mutability += randInt(-1, 1);
				if (replicated->mutability < 0)
				{
					replicated->mutability = 0;
				}
				else
				{
					if (replicated->mutability > 100)
					{
						replicated->mutability = 100;
					}
				}

				if (replicated->CheckValidity())
				{

					replicated->Remove();
					return replicated;
				}

				int newReproductioncooldown = (this->GetMaxEnergy() / ENERGY_DENSITY_MULTIPLIER) * REPRODUCTION_COOLDOWN_MULTIPLIER;
				replicated->reproductionCooldown = newReproductioncooldown + randInt(0, newReproductioncooldown);
				replicated->RecalculateStats();
				replicated->brain = this->brain.Clone();
				if (replicated->cellCounts[cell_mover])
				{
					replicated->brain.Mutate();
				}
				replicated->currentEnergy = randInt(1, replicated->maxEnergy / 3);
				replicated->lifespan = replicated->myCells.size() * LIFESPAN_MULTIPLIER;
				return replicated;
			}

			if (randPercent(33))
			{
				continue;
			}
		}
		if (randPercent(25))
		{
			continue;
		}
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
			bool couldAdd = false;

			int numCells = this->myCells.size();
			int cellIndex = 0;
			if (numCells > 1)
			{
				cellIndex = randInt(0, numCells - 1);
			}
			for (int i = 0; (i < numCells) && !couldAdd; i++)
			{
				Cell *thisAttempt = this->myCells[(cellIndex + randInt(0, numCells - 1)) % numCells];
				int thisDirectionIndex = randInt(0, 7);
				for (int j = 0; j < 8; j++)
				{
					int *thisDirection = directions[(thisDirectionIndex + randInt(0, 7)) % 8];
					int x_abs = thisAttempt->x + thisDirection[0];
					int y_abs = thisAttempt->y + thisDirection[1];
					if (board.isCellOfType(x_abs, y_abs, cell_empty))
					{
						x_rel = x_abs - this->x;
						y_rel = y_abs - this->y;
						couldAdd = true;
						break;
					}
				}
			}

			if (couldAdd)
			{
				this->AddCell(x_rel, y_rel, GenerateRandomCell());
			}
		}
	}
	this->RecalculateStats();
}

// return 1 if cell is occupied, else 0
int Organism::AddCell(int x_rel, int y_rel, Cell *_cell)
{
	int x_abs = this->x + x_rel;
	int y_abs = this->y + y_rel;
	if (!board.isCellOfType(x_abs, y_abs, cell_empty))
	{
		return true;
	}

	_cell->x = x_abs;
	_cell->y = y_abs;
	_cell->myOrganism = this;
	board.replaceCellAt(x_abs, y_abs, _cell);
	this->myCells.push_back(_cell);
	this->RecalculateStats();

	return false;
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
	this->RecalculateStats();
}

void Organism::ReplaceCell(Cell *_myCell, Cell *_newCell)
{
	this->RemoveCell(_myCell);
	_newCell->myOrganism = this;
	this->myCells.push_back(_newCell);
	// int x_rel = _myCell->x - this->x;
	// int y_rel = _myCell->y - this->y;
	board.replaceCell(_myCell, _newCell);
	this->RecalculateStats();
	// this->AddCell(x_rel, y_rel, _newCell);
}
