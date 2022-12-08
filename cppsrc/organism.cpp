#include "curses.h"
#include <stdlib.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cmath>

#include "lifeforms.h"
#include "board.h"
#include "rng.h"

#define moveCost(nCells) floor(sqrt(2.5 * (nCells - 1)))

extern Board *board;
Organism::Organism(int center_x, int center_y)
{
	this->x = center_x;
	this->y = center_y;
	this->species = 0;
	this->myCells = std::set<Cell *>();
	this->maxHealth = 0;
	this->currentHealth = this->maxHealth;
	this->currentEnergy = 0;
	this->lifespan = 0;
	this->maxEnergy = 0;
	this->age = 0;
	this->nCells_ = 0;
	this->alive = true;
	this->reproductionCooldown = 0;
	this->mutability = DEFAULT_MUTABILITY;
	this->brain = new Brain();
	this->direction = randInt(0, 3);
	for (int i = 0; i < cell_null; i++)
	{
		this->cellCounts[i] = 0;
	}
}

Organism::Organism(int center_x, int center_y, const Brain &baseBrain)
{
	this->x = center_x;
	this->y = center_y;
	this->species = 0;
	this->myCells = std::set<Cell *>();
	this->maxHealth = 0;
	this->currentHealth = this->maxHealth;
	this->currentEnergy = 0;
	this->lifespan = 0;
	this->maxEnergy = 0;
	this->age = 0;
	this->nCells_ = 0;
	this->alive = true;
	this->reproductionCooldown = 0;
	this->mutability = DEFAULT_MUTABILITY;
	this->brain = new Brain(baseBrain);
	this->direction = randInt(0, 3);
	for (int i = 0; i < cell_null; i++)
	{
		this->cellCounts[i] = 0;
	}
}

Organism::~Organism()
{
	delete this->brain;
}

void Organism::Die()
{
	board->RemoveSpeciesMember(this->species);
	for (auto celli = this->myCells.begin(); celli != this->myCells.end(); ++celli)
	{
		Cell *thisCell = *celli;
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
			replacedWith = new Cell_Plantmass();
			break;

		case cell_armor:
		case cell_killer:
			if ((this->cellCounts[cell_leaf] + this->cellCounts[cell_bark]) >= this->nCells() * 0.25)
			{
				replacedWith = new Cell_Plantmass();
			}
			else
			{
				replacedWith = new Cell_Biomass();
			}

			break;

		case cell_mover:
		case cell_herbivore_mouth:
		case cell_carnivore_mouth:
		case cell_touch:
		case cell_eye:
			replacedWith = new Cell_Biomass();
			break;
		}
		board->replaceCell(thisCell, replacedWith);
	}
	this->myCells.clear();
	this->alive = false;
}

void Organism::Remove()
{
	for (auto celli = this->myCells.begin(); celli != this->myCells.end(); ++celli)
	{
		Cell *thisCell = *celli;
		board->replaceCellAt(thisCell->x, thisCell->y, new Cell_Empty());
	}
	this->alive = false;
}

Organism *Organism::Tick()
{
	this->age++;

	this->ExpendEnergy(1);

	if (this->currentEnergy == 0 || this->currentHealth == 0 || (this->age >= this->lifespan) || this->nCells() == 0)
	{
		this->Die();
		return nullptr;
	}

	for (auto celli = this->myCells.begin(); celli != this->myCells.end();)
	{
		// drive the iterator like this to prevent it from breaking if the cell's Tick() results in it being removed
		auto next = celli;
		next++;
		Cell *toTick = *celli;
		switch (toTick->type)
		{
		case cell_leaf:
		{
			Cell_Leaf *leafToTick = static_cast<Cell_Leaf *>(toTick);
			if (leafToTick->CanFlower())
			{
				leafToTick->Tick();
			}
		}
		default:
			toTick->Tick();
			break;
		}
		celli = next;
	}

	if (this->cellCounts[cell_mover])
	{
		this->brain->SetBaselineInput(static_cast<nn_num_t>(this->currentEnergy) / static_cast<nn_num_t>(this->maxEnergy),
									  static_cast<nn_num_t>(this->currentHealth) / static_cast<nn_num_t>(this->maxHealth));
		switch (this->brain->Decide())
		{
		case intent_idle:
		{
		}
		break;
		case intent_forward:
		{
			this->Move(1);
		}
		break;
		case intent_back:
		{
			this->Move(3);
		}
		break;
		case intent_left:
		{
			this->Move(2);
		}
		break;
		case intent_right:
		{
			this->Move(0);
		}
		break;
		case intent_rotate_clockwise:
		{
			this->Rotate(true);
		}
		break;
		case intent_rotate_counterclockwise:
		{
			this->Rotate(false);
		}
		break;
		}
	}

	this->AddEnergy(((this->age % PHOTOSYNTHESIS_INTERVAL == 0) ? this->cellCounts[cell_leaf] : 0) +
					(this->cellCounts[cell_leaf] > 0));

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

	for (auto celli = this->myCells.begin(); celli != this->myCells.end(); ++celli)
	{
		Cell *thisCell = *celli;
		cellCounts[thisCell->type]++;
		calculatedMaxEnergy += CellEnergyDensities[thisCell->type];
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

	this->maxHealth = this->nCells() * MAX_HEALTH_MULTIPLIER + (this->cellCounts[cell_armor] * ARMOR_HEALTH_BONUS);
	if (this->currentHealth > this->maxHealth)
	{
		this->currentHealth = this->maxHealth;
	}

	if (this->lifespan > sqrt(this->maxEnergy) * ceil(sqrt(this->nCells())) * LIFESPAN_MULTIPLIER)
	{
		this->lifespan = sqrt(this->maxEnergy) * ceil(sqrt(this->nCells())) * LIFESPAN_MULTIPLIER;
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

	if (this->cellCounts[cell_mover] == 0)
	{
		if (this->cellCounts[cell_touch] || this->cellCounts[cell_eye])
		{
			return true;
		}
	}
	else
	{
		if (this->cellCounts[cell_leaf] > 0.5 * this->nCells_)
		{
			return true;
		}
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

	for (std::set<Cell *>::iterator c = this->myCells.begin(); c != this->myCells.end();)
	{
		if (cellValidity[*c] == true)
		{
			++c;
		}
		else
		{
			board->replaceCell(*c, new Cell_Empty());
			c = this->myCells.erase(c);
			this->nCells_--;
		}
	}

	bool invalid = false;

	// disallow organisms that are all mouths
	// invalid |= (this->cellCounts[cell_herbivore_mouth] == this->nCells());

	// disallow herbivores that have leaves on them
	invalid |= (this->cellCounts[cell_herbivore_mouth] > 0 && this->cellCounts[cell_leaf] > 0);

	// must have a mover to have a touch sensor
	invalid |= (this->cellCounts[cell_touch] > 0 && this->cellCounts[cell_mover] == 0);

	// must have a mover to have an eye
	invalid |= (this->cellCounts[cell_eye] > 0 && this->cellCounts[cell_mover] == 0);

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

void Organism::Move(int moveDirection)
{
	int *moveDir = directions[(moveDirection + this->direction) % 4];
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

		for (auto celli = this->myCells.begin(); celli != this->myCells.end(); ++celli)
		{
			Cell *movedCell = *celli;

			// put an empty in its position
			Cell *filler = new Cell_Empty();
			filler->x = movedCell->x;
			filler->y = movedCell->y;
			board->cells[movedCell->y][movedCell->x] = filler;

			// calculate the new x and y position we are moving to
			int newX = movedCell->x + moveDir[0];
			int newY = movedCell->y + moveDir[1];
			board->DeltaCells.insert(std::pair<int, int>(movedCell->x, movedCell->y));
			board->DeltaCells.insert(std::pair<int, int>(newX, newY));

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
		this->ExpendEnergy(moveCost(this->nCells_));
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
		board->DeltaCells.insert(std::pair<int, int>(a->x, a->y));

		b->x = oldX;
		b->y = oldY;
		board->cells[b->y][b->x] = b;
		board->DeltaCells.insert(std::pair<int, int>(b->x, b->y));
	}

	for (Cell *c : this->myCells)
	{
		switch (c->type)
		{
		case cell_eye:
		{
			Cell_Eye *rotatedEye = static_cast<Cell_Eye *>(c);
			rotatedEye->direction += (clockwise ? -1 : 1);
			if (rotatedEye->direction < 0)
			{
				rotatedEye->direction = 3;
			}
			else if (rotatedEye->direction > 3)
			{
				rotatedEye->direction %= 4;
			}
		}
		break;

		default:
			break;
		}
	}

	this->ExpendEnergy(moveCost(this->nCells_));
	this->direction += (clockwise ? -1 : 1);
	if (this->direction < 0)
	{
		this->direction = 3;
	}
	else if (this->direction > 4)
	{
		this->direction %= 4;
	}
}

void Organism::Damage(uint64_t n)
{
	if (n > this->currentHealth)
	{
		this->currentHealth = 0;
		return;
	}

	this->currentHealth -= n;
}

void Organism::Heal(uint64_t n)
{
	this->currentHealth += n;
	if (this->currentHealth > this->maxHealth)
	{
		this->currentHealth = this->maxHealth;
	}
}

void Organism::ExpendEnergy(uint64_t n)
{
	if (n > this->currentEnergy)
	{
		this->currentEnergy = 0;
		return;
	}

	this->currentEnergy -= n;
}

void Organism::AddEnergy(uint64_t n)
{
	this->currentEnergy += n;
	if (this->currentEnergy > this->maxEnergy)
	{
		this->currentEnergy = this->maxEnergy;
	}
}

uint64_t Organism::GetMaxHealth()
{
	return this->maxHealth;
}

uint64_t Organism::GetEnergy()
{
	return this->currentEnergy;
}

uint64_t Organism::GetMaxEnergy()
{
	return this->maxEnergy;
}

// returns true if possible, false if not
bool Organism::CanOccupyPosition(int _x_abs, int _y_abs)
{
	for (auto celli = this->myCells.begin(); celli != this->myCells.end(); ++celli)
	{
		Cell *thisCell = *celli;

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
	for (auto celli = this->myCells.begin(); celli != this->myCells.end(); ++celli)
	{
		Cell *thisCell = *celli;

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
	// this->reproductionCooldown = REPRODUCTION_COOLDOWN;

	int dirIndex = randInt(0, 7);
	for (int i = 0; i < 8; i++)
	{
		int *thisDir = directions[(dirIndex + i) % 8];
		for (int j = 1; j < ceil(sqrt(this->nCells())) + 1; j++)
		{
			int dir_x = thisDir[0] * j;
			int dir_y = thisDir[1] * j;

			if (this->CanOccupyPosition(this->x + dir_x, this->y + dir_y))
			{
				int dir_x_extra = 0;
				int dir_y_extra = 0;
				if (randPercent(40))
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

				Organism *replicated = new Organism(this->x + dir_x + dir_x_extra, this->y + dir_y + dir_y_extra, *this->brain);
				replicated->direction = this->direction;
				replicated->mutability = this->mutability;
				for (auto celli = this->myCells.begin(); celli != this->myCells.end(); ++celli)
				{
					Cell *thisCell = *celli;
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
					size_t parentLeafCount = this->cellCounts[cell_leaf];
					size_t parentBarkCount = this->cellCounts[cell_bark];
					size_t childLeafCount = replicated->cellCounts[cell_leaf];
					size_t childBarkCount = replicated->cellCounts[cell_bark];
					if (parentLeafCount && childLeafCount)
					{
						if ((childLeafCount == (parentLeafCount + 1)) ||
							(childLeafCount == (parentLeafCount - 1)))
						{
							newSpecies = false;
						}
					}
					if (parentBarkCount && childBarkCount)
					{
						if ((childBarkCount == (parentBarkCount + 1)) ||
							(childBarkCount == (parentBarkCount - 1)))
						{
							newSpecies = false;
						}
					}
				}

				if (replicated->CheckValidity() || replicated->maxEnergy == 0)
				{
					replicated->Remove();
					return replicated;
				}

				if (newSpecies)
				{
					replicated->species = board->GetNextSpecies();
					board->AddSpeciesMember(replicated);
					board->RecordEvolvedFrom(this, replicated);
				}
				else
				{
					replicated->species = this->species;
					board->AddSpeciesMember(replicated);
				}

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
				}
				if (randPercent(this->mutability))
				{
					replicated->Rotate(randPercent(50));
				}

				if (this->cellCounts[cell_mover] && randPercent(this->mutability))
				{
					replicated->brain->Mutate();
				}

				replicated->reproductionCooldown = 15 * replicated->nCells_; // + randInt(0, REPRODUCTION_COOLDOWN);
				replicated->RecalculateStats();
				replicated->Heal(replicated->GetMaxHealth());
				// if (replicated->cellCounts[cell_mover])
				// {
				// replicated->brain->Mutate();
				// }
				replicated->currentEnergy = randInt(replicated->maxEnergy / 2, replicated->maxEnergy);
				replicated->lifespan = sqrt(replicated->maxEnergy) * sqrt(replicated->nCells()) * LIFESPAN_MULTIPLIER;
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
	if (this->nCells() > 1 && randPercent(30))
	{
		int switchedIndex = randInt(0, this->nCells() - 2);
		auto switchedIterator = this->myCells.begin();
		for(int i = 0; i < switchedIndex; i++)
		{
			++switchedIterator;
		}
		Cell *toReplace = *switchedIterator;
		Cell *replacedWith = GenerateRandomCell();
		while (replacedWith->type == toReplace->type)
		{
			delete replacedWith;
			replacedWith = GenerateRandomCell();
		}
		this->ReplaceCell(toReplace, replacedWith);
		return true;
	}
	else
	{
		bool allLeaves = this->cellCounts[cell_leaf] == this->nCells();
		// remove a cell
		if (this->nCells() > 2 && randPercent(50) && !allLeaves)
		{
			int removedIndex = randInt(0, this->nCells() - 2);
			auto removedIterator = this->myCells.begin();
			for(int i = 0; i < removedIndex; i++)
			{
				++removedIterator;
			}
			Cell *toRemove = *removedIterator;
			this->RemoveCell(toRemove);
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

void Organism::OnCellAdded(Cell *added)
{
	switch (added->type)
	{
	case cell_touch:
	case cell_eye:
	{
		Sensor_Cell *c = static_cast<Sensor_Cell *>(added);
		if (c->BrainInputIndex() == -1)
		{
			c->SetBrainInputIndex(this->brain->GetNewSensorIndex());
		}
	}
	break;

	default:
		break;
	}
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
	this->myCells.insert(_cell);
	this->nCells_++;

	this->OnCellAdded(_cell);

	this->RecalculateStats();
}

void Organism::RemoveCell(Cell *_myCell)
{
	if (this->myCells.count(_myCell) == 0)
	{
		std::cerr << "Bad call to remove cell with _myCell not from this organism!" << std::endl;
		exit(1);
	}
	this->myCells.erase(_myCell);
	this->RecalculateStats();
	this->nCells_--;
}

void Organism::ReplaceCell(Cell *_myCell, Cell *_newCell)
{
	this->RemoveCell(_myCell);
	_newCell->myOrganism = this;
	this->myCells.insert(_newCell);
	this->nCells_++;
	// int x_rel = _myCell->x - this->x;
	// int y_rel = _myCell->y - this->y;
	board->replaceCell(_myCell, _newCell);

	this->OnCellAdded(_newCell);

	this->RecalculateStats();
	// this->AddCell(x_rel, y_rel, _newCell);
}

enum OrganismClassifications Organism::Classify()
{
	int plantCells = this->cellCounts[cell_leaf] +
					 this->cellCounts[cell_bark] +
					 this->cellCounts[cell_flower];
	// if at least 1/3 plant or can't move, it's a plant
	if (static_cast<uint64_t>(plantCells * 3) >= this->nCells() || !this->cellCounts[cell_mover])
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
