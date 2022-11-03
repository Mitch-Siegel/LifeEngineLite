#include "curses.h"
#include <stdlib.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cmath>

#include "lifeforms.h"
#include "board.h"
#include "rng.h"

extern Board *board;
Organism::Organism(int center_x, int center_y)
{
	this->x = center_x;
	this->y = center_y;
	this->species = 0;
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
	board->RemoveSpeciesMember(this->species);
	for (size_t i = 0; i < this->myCells.size(); i++)
	{
		Cell *thisCell = this->myCells[i];
		Cell *replacedWith = nullptr;
		switch (thisCell->type)
		{
		case cell_null:
		case cell_empty:
		case cell_biomass:
		case cell_plantmass:
		case cell_fruit:
			std::cerr << "Wrong cell type " << thisCell->type << " contained within organism!" << std::endl;
			exit(1);

		case cell_leaf:
		case cell_flower:
		case cell_bark:
			replacedWith = new Cell_Plantmass(ceil(sqrt(this->myCells.size())) * PLANTMASS_SPOIL_TIME_MULTIPLIER);
			break;

		case cell_armor:
		case cell_killer:
			if ((this->cellCounts[cell_leaf] + this->cellCounts[cell_bark]) >= this->myCells.size() * 0.25)
			{
				replacedWith = new Cell_Plantmass(ceil(sqrt(this->myCells.size())) * PLANTMASS_SPOIL_TIME_MULTIPLIER);
			}
			else
			{
				replacedWith = new Cell_Biomass(ceil(sqrt(this->myCells.size())) * BIOMASS_SPOIL_TIME_MULTIPLIER);
			}

			break;

		case cell_mover:
		case cell_herbivore_mouth:
		case cell_carnivore_mouth:
		case cell_touch:
			replacedWith = new Cell_Biomass(ceil(sqrt(this->myCells.size())) * BIOMASS_SPOIL_TIME_MULTIPLIER);
			break;
		}
		board->replaceCell(thisCell, replacedWith);
	}
	this->myCells.clear();
	this->alive = false;
}

void Organism::Remove()
{
	for (size_t i = 0; i < this->myCells.size(); i++)
	{
		Cell *thisCell = this->myCells[i];
		board->replaceCellAt(thisCell->x, thisCell->y, new Cell_Empty());
	}
	this->alive = false;
}

Organism *Organism::Tick()
{
	this->age++;

	// if (this->myCells.size() > 1 || this->age % 2 == 0)
	// {
	this->ExpendEnergy(1);
	// }

	if (this->currentEnergy == 0 || this->currentHealth == 0 || (this->age >= this->lifespan) || this->myCells.size() == 0)
	{
		this->Die();
		return nullptr;
	}

	for (size_t i = 0; i < this->myCells.size(); i++)
	{
		this->myCells[i]->Tick();
	}

	if (this->cellCounts[cell_mover])
	{
		if (this->currentEnergy > 2)
		{
			switch (this->brain.Decide())
			{
			case intent_changeDir:
			case intent_continue:
				for (size_t i = 0; i < this->cellCounts[cell_mover]; i++)
				{
					this->Move();
				}
				break;

			case intent_rotateClockwise:
				for (size_t i = 0; i < this->cellCounts[cell_mover]; i++)
				{
					this->Rotate(true);
				}
				break;

			case intent_rotateCounterClockwise:
				for (size_t i = 0; i < this->cellCounts[cell_mover]; i++)
				{
					this->Rotate(false);
				}
				break;
			}
		}
	}

	this->AddEnergy(((this->age % 5 == 0) ? this->cellCounts[cell_leaf] : 0) +
					(this->cellCounts[cell_leaf] > 0));
	// (((this->age % 3) == 0) ? this->cellCounts[cell_bark] * ceil(sqrt(this->cellCounts[cell_leaf])) : 0));
	/*
	if (this->myOrganism->age % 3 == 0 )
	{
		int energyGained = 1;
		// small chance for each leaf to generate an extra energy
		// chance is boosted by plants with bark on board
		if (this->myOrganism->age % (200 - (10 * this->myOrganism->cellCounts[cell_bark])) == 0)
		{
			energyGained++;
		} andPercent(2 * (this->myOrganism->myCells.size() - 4));
		// if(this->myOrganism->myCells.size() < 5 && randPercent(15))
		// {
		// energyGained--;
		// }
		this->myOrganism->AddEnergy(energyGained);
	}
	*/

	if (this->reproductionCooldown == 0)
	{
		if (this->currentEnergy > ((this->maxEnergy * REPRODUCTION_ENERGY_MULTIPLIER) * 1.2))
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
	this->maxEnergy = 0;
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
	int nCells = this->myCells.size();
	this->maxHealth = nCells * MAX_HEALTH_MULTIPLIER + (this->cellCounts[cell_armor] * ARMOR_HEALTH_BONUS);
	if (this->currentHealth > this->maxHealth)
	{
		this->currentHealth = this->maxHealth;
	}

	if (this->lifespan > sqrt(this->maxEnergy) * ceil(sqrt(this->myCells.size())) * LIFESPAN_MULTIPLIER)
	{
		this->lifespan = sqrt(this->maxEnergy) * ceil(sqrt(this->myCells.size())) * LIFESPAN_MULTIPLIER;
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
	if (board->boundCheckPos(this->x, this->y))
	{
		return true;
	}
	if (board->cells[this->y][this->x]->myOrganism != this)
	{
		return true;
	}

	std::unordered_map<Cell *, bool> cellValidity;
	// conduct a search on cells, only keep ones directly attached to the organism
	std::vector<Cell *> searchQueue;
	Cell *start = board->cells[this->y][this->x];
	searchQueue.push_back(start);
	while (searchQueue.size() > 0)
	{
		Cell *examined = searchQueue.back();
		searchQueue.pop_back();
		cellValidity[examined] = true;
		for (int i = 0; i < 8; i++)
		{
			int x_abs = examined->x + directions[i][0];
			int y_abs = examined->y + directions[i][1];
			if (!board->boundCheckPos(x_abs, y_abs))
			{
				Cell *neighbor = board->cells[y_abs][x_abs];
				if (neighbor->myOrganism == this && cellValidity[neighbor] == false)
				{
					searchQueue.push_back(neighbor);
				}
			}
		}
	}

	for (std::vector<Cell *>::iterator c = this->myCells.begin(); c != this->myCells.end();)
	{
		if (cellValidity[*c] == true)
		{
			c++;
		}
		else
		{
			board->replaceCell(*c, new Cell_Empty());
			c = this->myCells.erase(c);
		}
	}

	bool invalid = false;

	// disallow organisms that are all mouths
	// invalid |= (this->cellCounts[cell_herbivore_mouth] == this->myCells.size());

	// disallow herbivores that have leaves on them
	invalid |= (this->cellCounts[cell_herbivore_mouth] > 0 && this->cellCounts[cell_leaf] > 0);

	// must have a mover to have a touch sensor
	invalid |= (this->cellCounts[cell_touch] > 0 && this->cellCounts[cell_mover] == 0);

	bool hasCenterCell = false;

	// plants must have a killer cell next to bark
	if (!invalid && this->cellCounts[cell_mover] == 0 && this->cellCounts[cell_killer])
	{
		for (Cell *c : this->myCells)
		{
			if (c->x == this->x && c->y == this->y)
			{
				hasCenterCell = true;
			}
			if (c->type == cell_killer)
			{
				bool killerValid = false;
				// killers on plants must have a bark directly adjacent (no diagonals)
				for (int i = 0; i < 4; i++)
				{
					int *thisDirection = directions[i];
					int x_abs = c->x + thisDirection[0];
					int y_abs = c->y + thisDirection[1];
					if (!board->boundCheckPos(x_abs, y_abs))
					{
						Cell *neighborCell = board->cells[y_abs][x_abs];
						killerValid |= ((neighborCell->myOrganism == this) &&
										(neighborCell->type == cell_bark));
					}
				}
				if (!killerValid)
				{
					invalid = true;
					break;
				}
			}
		}
	}

	if (!invalid)
	{
		if (!hasCenterCell)
		{
			for (Cell *c : this->myCells)
			{
				if (c->x == this->x && c->y == this->y)
				{
					hasCenterCell = true;
					break;
				}
			}
		}
		if (!hasCenterCell)
		{
			invalid = true;
		}
	}
	return invalid;
}

void Organism::Move()
{
	int *moveDir = directions[this->brain.moveDirIndex];
	if (this->CanMoveToPosition(this->x + moveDir[0], this->y + moveDir[1]))
	{
		class MovedCell
		{
		public:
			MovedCell(Cell *_c, int _newX, int _newY)
			{
				this->c = _c;
				this->newX = _newX;
				this->newY = _newY;
			}
			Cell *c;
			int newX, newY;
		};
		std::vector<MovedCell> moves;

		// first pass - pick up all cells and replace with empties
		for (size_t i = 0; i < this->myCells.size(); i++)
		{
			// pick up the cell we are moving
			Cell *movedCell = this->myCells[i];
			// put an empty in its position
			Cell *filler = new Cell_Empty();
			filler->x = movedCell->x;
			filler->y = movedCell->y;
			board->cells[movedCell->y][movedCell->x] = filler;

			// calculate the new x and y position we are moving to
			int newX = movedCell->x + moveDir[0];
			int newY = movedCell->y + moveDir[1];
			board->DeltaCells[movedCell->y][movedCell->x] = true;
			board->DeltaCells[newY][newX] = true;

			movedCell->x = newX;
			movedCell->y = newY;
			moves.push_back(MovedCell(movedCell, newX, newY));
		}

		// second pass - delete empties and place cells back down at delta pos
		for (MovedCell m : moves)
		{
			delete board->cells[m.newY][m.newX];
			board->cells[m.newY][m.newX] = m.c;
		}
		this->x += moveDir[0];
		this->y += moveDir[1];

		// only expend energy if can move
		/*
		\operatorname{ceil}\left(\sqrt{\left(2^{.3x\ }+1.5\right)}\right)-2
		*/
		int moveCost = floor(this->myCells.size() * 0.5) * (this->cellCounts[cell_leaf] + 1);
		// int moveCost = ceil(sqrt(pow(2, .3 * this->myCells.size()) + 1.5)) - 2;
		this->ExpendEnergy(moveCost);
	}
	else
	{
		// this->brain.ForceRechoose();
		if (this->cellCounts[cell_touch])
		{
			this->brain.Punish();
		}
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

			if (board->boundCheckPos(new_x, new_y))
			{
				// this->brain.Punish();
				return;
			}

			Cell *swappedWith = board->cells[new_y][new_x];
			// can't rotate!
			if (swappedWith->type != cell_empty && swappedWith->myOrganism != this)
			{
				// this->brain.Punish();
				return;
			}

			swappedMap[c] = true;
			// redundant check but doesn't hurt
			if (!swappedMap.count(swappedWith))
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
		board->cells[a->y][a->x] = a;
		board->DeltaCells[a->y][a->x] = true;

		b->x = oldX;
		b->y = oldY;
		board->cells[b->y][b->x] = b;
		board->DeltaCells[b->y][b->x] = true;
	}

	int rotateCost = floor(this->myCells.size() * 0.5) * (this->cellCounts[cell_leaf] + 1);

	this->ExpendEnergy(rotateCost);
	this->brain.RotateSuccess(clockwise);
}

void Organism::Damage(size_t n)
{
	if (n > this->currentHealth)
	{
		this->currentHealth = 0;
		return;
	}

	this->currentHealth -= n;
	this->brain.Punish();
}

void Organism::Heal(size_t n)
{
	this->currentHealth += n;
	if (this->currentHealth > this->maxHealth)
	{
		this->currentHealth = this->maxHealth;
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

std::size_t Organism::GetMaxHealth()
{
	return this->maxHealth;
}

std::size_t Organism::GetEnergy()
{
	return this->currentEnergy;
}

std::size_t Organism::GetMaxEnergy()
{
	return this->maxEnergy;
}

// returns true if possible, false if not
bool Organism::CanOccupyPosition(int _x_abs, int _y_abs)
{
	for (size_t i = 0; i < this->myCells.size(); i++)
	{
		Cell *thisCell = this->myCells[i];

		int newCellX_rel = thisCell->x - this->x;
		int newCellY_rel = thisCell->y - this->y;
		int new_x_abs = _x_abs + newCellX_rel;
		int new_y_abs = _y_abs + newCellY_rel;
		bool boundCheckResult = board->boundCheckPos(new_x_abs, new_y_abs);
		// if location out of bounds or not empty
		if (boundCheckResult || board->cells[new_y_abs][new_x_abs]->type != cell_empty)
		{
			// fail
			return false;
		}
	}
	return true;
}

bool Organism::CanMoveToPosition(int _x_abs, int _y_abs)
{
	for (size_t i = 0; i < this->myCells.size(); i++)
	{
		Cell *thisCell = this->myCells[i];

		int newCellX_rel = thisCell->x - this->x;
		int newCellY_rel = thisCell->y - this->y;
		int new_x_abs = _x_abs + newCellX_rel;
		int new_y_abs = _y_abs + newCellY_rel;
		bool boundCheckResult = board->boundCheckPos(new_x_abs, new_y_abs);
		if (boundCheckResult ||																														// if out of bounds
			(!boundCheckResult && board->cells[new_y_abs][new_x_abs]->type != cell_empty && board->cells[new_y_abs][new_x_abs]->myOrganism != this) // or in-bounds and this position is occupied by something else
		)
		{
			// fail
			return false;
		}
	}
	return true;
}

Organism *Organism::Reproduce()
{
	// this->reproductionCooldown = ceil(sqrt(this->myCells.size()) * REPRODUCTION_COOLDOWN_MULTIPLIER);
	this->reproductionCooldown = REPRODUCTION_COOLDOWN;

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
				int dir_x_extra = 0;
				int dir_y_extra = 0;
				if (randPercent(20))
				{
					for (int k = 0; k < 16; k++)
					{
						dir_x_extra = randInt(-3, 3);
						dir_y_extra = randInt(-3, 3);
						if (this->CanOccupyPosition(this->x + dir_x + dir_x_extra, this->y + dir_y + dir_y_extra))
						{
							break;
						}
						dir_x_extra = 0;
						dir_y_extra = 0;
					}
				}

				this->ExpendEnergy(this->maxEnergy * REPRODUCTION_ENERGY_MULTIPLIER);

				Organism *replicated = new Organism(this->x + dir_x + dir_x_extra, this->y + dir_y + dir_y_extra);
				replicated->mutability = this->mutability;
				for (size_t k = 0; k < this->myCells.size(); k++)
				{
					Cell *thisCell = this->myCells[k];
					switch (thisCell->type)
					{
					case cell_flower:
					case cell_fruit:
						break;

					case cell_bark:
					{
						int this_rel_x = thisCell->x - this->x;
						int this_rel_y = thisCell->y - this->y;
						Cell_Bark *replicatedCell = static_cast<Cell_Bark *>(thisCell->Clone());
						replicatedCell->myOrganism = replicated;
						replicatedCell->integrity = BARK_MAX_INTEGRITY;
						replicated->AddCell(this_rel_x, this_rel_y, replicatedCell);
					}
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

				bool newSpecies = false;
				if (randPercent(this->mutability))
				{
					newSpecies = replicated->Mutate();
				}

				if (replicated->CheckValidity() || replicated->maxEnergy == 0)
				{
					replicated->Remove();
					return replicated;
				}

				if (newSpecies)
				{
					replicated->species = board->GetNextSpecies();
					board->evolvedFrom[replicated->species] = this->species;
				}
				else
				{
					replicated->species = this->species;
				}
				board->AddSpeciesMember(replicated);

				if (randPercent(this->mutability))
				{
					replicated->mutability += randInt(-2, 3);
					if (replicated->mutability < 1)
					{
						replicated->mutability = 1;
					}
					else if (replicated->mutability > 100)
					{
						replicated->mutability = 100;
					}

					for (Cell *c : this->myCells)
					{
						if (c->type == cell_touch)
						{
							static_cast<Cell_Touch *>(c)->senseInterval += randInt(-1, 1);
						}
					}
				}
				if (randPercent(this->mutability))
				{
					replicated->Rotate(randPercent(50));
				}

				// this->brain.Reward();
				// int newReproductioncooldown = ceil(sqrt(replicated->myCells.size()) * REPRODUCTION_COOLDOWN_MULTIPLIER);
				replicated->reproductionCooldown = REPRODUCTION_COOLDOWN; // + randInt(0, REPRODUCTION_COOLDOWN);
				replicated->RecalculateStats();
				replicated->Heal(replicated->GetMaxHealth());
				replicated->brain = this->brain.Clone();
				if (replicated->cellCounts[cell_mover])
				{
					replicated->brain.Mutate();
				}
				replicated->currentEnergy = randInt(replicated->maxEnergy / 2, replicated->maxEnergy);
				replicated->lifespan = sqrt(replicated->maxEnergy) * sqrt(replicated->myCells.size()) * LIFESPAN_MULTIPLIER;
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
	// this->brain.Punish();
	return nullptr;
}

bool Organism::Mutate()
{
	// change existing cell
	if (this->myCells.size() > 1 && randPercent(30))
	{
		int switchedIndex = randInt(0, this->myCells.size() - 1);

		Cell *toReplace = this->myCells[switchedIndex];
		this->myCells.erase(std::find(this->myCells.begin(), this->myCells.end(), toReplace));

		Cell *replacedWith = GenerateRandomCell();
		replacedWith->myOrganism = this;

		bool actuallyMutated = (replacedWith->type != toReplace->type);

		board->replaceCell(toReplace, replacedWith);
		this->myCells.push_back(replacedWith);
		return actuallyMutated;
		// int cellIndex = (rand() >> 5) % this->myCells.size();
		// this->myCells[cellIndex].type = (enum CellTypes)((rand() >> 5) % (int)cell_mouth);
	}
	else
	{
		bool allLeaves = this->cellCounts[cell_leaf] == this->myCells.size();
		// remove a cell
		if (this->myCells.size() > 2 && randPercent(50) && !allLeaves)
		{
			Cell *toRemove = this->myCells[randInt(0, this->myCells.size() - 1)];
			this->myCells.erase(std::find(this->myCells.begin(), this->myCells.end(), toRemove));
			board->replaceCell(toRemove, new Cell_Empty());
			return true;
		}
		// add a cell
		else
		{
			int x_rel;
			int y_rel;
			bool canAdd = false;

			int dirIndex = randInt(0, 7);
			for (int i = 0; i < 8 && !canAdd; i++)
			{
				int *thisDirection = directions[(dirIndex + i) % 8];
				y_rel = 0;
				x_rel = 0;
				bool looking = true;
				while (looking)
				{
					if (board->boundCheckPos(this->x + x_rel, this->y + y_rel))
					{
						looking = false;
						break;
					}

					if (board->isCellOfType(this->x + x_rel, this->y + y_rel, cell_empty))
					{
						looking = false;
						canAdd = true;
						break;
					}
					else
					{
						if (randPercent(50))
						{
							y_rel += thisDirection[1];
						}
						else
						{
							x_rel += thisDirection[0];
						}
					}
				}
			}

			if (canAdd)
			{
				this->AddCell(x_rel, y_rel, allLeaves ? new Cell_Leaf : GenerateRandomCell());
				return true;
			}
		}
	}
	return false;
}

void Organism::AddCell(int x_rel, int y_rel, Cell *_cell)
{
	int x_abs = this->x + x_rel;
	int y_abs = this->y + y_rel;
	if (!board->isCellOfType(x_abs, y_abs, cell_empty))
	{
		printf("Can't add cell at %d, %d (organism at %d, %d)\n", x_abs, y_abs, this->x, this->y);
		exit(1);
	}

	_cell->x = x_abs;
	_cell->y = y_abs;
	_cell->myOrganism = this;
	board->replaceCellAt(x_abs, y_abs, _cell);
	this->myCells.push_back(_cell);
	this->RecalculateStats();
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
	board->replaceCell(_myCell, _newCell);
	this->RecalculateStats();
	// this->AddCell(x_rel, y_rel, _newCell);
}

enum OrganismClassifications Organism::Classify()
{
	int plantCells = this->cellCounts[cell_leaf] +
					 this->cellCounts[cell_bark] +
					 this->cellCounts[cell_flower];
	// if at least 1/3 plant or can't move, it's a plant
	if (static_cast<size_t>(plantCells * 3) >= this->myCells.size() || !this->cellCounts[cell_mover])
	{
		return class_plant;
	}

	// otherwise classify by mouth
	if (this->cellCounts[cell_herbivore_mouth] && this->cellCounts[cell_carnivore_mouth])
	{
		return class_omnivore;
	}
	else if (this->cellCounts[cell_herbivore_mouth])
	{
		return class_herbivore;
	}
	else if (this->cellCounts[cell_carnivore_mouth])
	{
		return class_carnivore;
	}

	// if no mouths but not substantially plant, just classify as plant anyways
	return class_plant;
}
