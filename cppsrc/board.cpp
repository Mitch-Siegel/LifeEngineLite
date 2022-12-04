#include "board.h"
#include "lifeforms.h"
#include "rng.h"

#include <SDL2/SDL.h>
#include <iostream>
#include <algorithm>
#include <chrono>

int directions[8][2] = {{1, 0}, {0, -1}, {-1, 0}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
extern Board *board;

Board::Board(const int _dim_x, const int _dim_y)
{
	this->tickCount = 0;
	this->nextSpecies = 1;
	this->dim_x = _dim_x;
	this->dim_y = _dim_y;
	this->Organisms = std::vector<Organism *>();

	for (int y = 0; y < _dim_y; y++)
	{
		this->cells.push_back(std::vector<Cell *>());
		for (int x = 0; x < _dim_x; x++)
		{
			this->cells[y].push_back(new Cell_Empty());
			this->cells[y][x]->x = x;
			this->cells[y][x]->y = y;
		}
	}
}

Board::~Board()
{
	for (int y = 0; y < this->dim_y; y++)
	{
		for (int x = 0; x < this->dim_x; x++)
		{
			delete this->cells[y][x];
		}
	}
}

// returns false if did tick, true if couldn't acquire mutex
bool Board::Tick()
{
	if(!this->TryGetMutex())
	{
		return true;
	}
	this->tickCount++;
	std::map<uint64_t, Board::Food_Slot *> newFoodCells;
	for (std::map<uint64_t, Board::Food_Slot *>::iterator sloti = this->FoodCells.begin(); sloti != this->FoodCells.end(); ++sloti)
	{
		if (sloti->first == 0)
		{
			Food_Slot *thisSlot = sloti->second;
			// don't drive the iterator because the replacecell() calls for each food cell will delete the elements
			// an overridden version of the replacecell specifying whether or not to do the delete would save the std::find's worth of time
			for (std::vector<Spoilable_Cell *>::iterator c = thisSlot->begin(); c != thisSlot->end(); ++c)
			{
				Spoilable_Cell *expiringFood = *c;
				switch (expiringFood->type)
				{
				case cell_biomass:
				case cell_plantmass:
					this->replaceCell_NoTrackReplacedFood(expiringFood, new Cell_Empty());
					break;

				case cell_fruit:
					// if we roll grow percent, create a new random organism
					if (randPercent(FRUIT_GROW_PERCENT) && randPercent(FRUIT_GROW_PERCENT))
					{
						Organism *grownFruit = this->createOrganism(expiringFood->x, expiringFood->y);
						grownFruit->mutability = 10;
						this->replaceCell_NoTrackReplacedFood(expiringFood, new Cell_Empty());
						grownFruit->AddCell(0, 0, GenerateRandomCell());
						Cell *secondRandomCell = GenerateRandomCell();
						bool couldAddSecond = false;
						int dirIndex = randInt(0, 7);
						for (int j = 0; j < 8; j++)
						{
							int *thisDirection = directions[(j + dirIndex) % 8];
							if (this->isCellOfType(grownFruit->x + thisDirection[0], grownFruit->y + thisDirection[1], cell_empty))
							{
								grownFruit->AddCell(thisDirection[0], thisDirection[1], secondRandomCell);
								couldAddSecond = true;
								break;
							}
						}
						if (!couldAddSecond)
						{
							delete secondRandomCell;
						}

						if (!grownFruit->CheckValidity())
						{
							grownFruit->species = this->GetNextSpecies();
							this->AddSpeciesMember(grownFruit);
							grownFruit->RecalculateStats();
							grownFruit->lifespan = grownFruit->nCells() * LIFESPAN_MULTIPLIER;
							grownFruit->AddEnergy(randInt(grownFruit->GetMaxEnergy() / 2, grownFruit->GetMaxEnergy()));
							grownFruit->Heal(grownFruit->GetMaxHealth());

							grownFruit->reproductionCooldown = 0;
						}
						else
						{
							grownFruit->Remove();
						}
					}
					else
					{
						this->replaceCell_NoTrackReplacedFood(expiringFood, new Cell_Plantmass(FRUIT_SPOIL_TIME));
					}
					break;

				default:
					printf("Impossible case for food cell to be something it shouldn't (cell_types enum %d)!\n", expiringFood->type);
					exit(1);
				}
			}
			delete sloti->second;
		}
		else
		{
			sloti->second->ticksUntilSpoil--;
			newFoodCells[sloti->first - 1] = sloti->second;
		}
		// this->FoodCells[foodVectorI.first] = nullptr;

		// printf("%lu\n", foodVectorI.first);
	}
	this->FoodCells = newFoodCells;

	for (uint64_t i = 0; i < this->Organisms.size(); i++)
	{
		if (!this->Organisms[i]->alive)
		{
			Organism *toRemove = this->Organisms[i];
			this->Organisms.erase(this->Organisms.begin() + i);
			delete toRemove;
			i--;
		}
		else
		{
			Organism *replicated = this->Organisms[i]->Tick();

			if (replicated != nullptr)
			{
				Organisms.push_back(replicated);
			}
		}
	}

	this->ReleaseMutex();
	return false;
}

// returns true if out of bounds, false otherwise
bool Board::boundCheckPos(int x, int y)
{
	if (0 > x || 0 > y || x >= this->dim_x || y >= this->dim_y)
	{
		return true;
	}
	return false;
}

// returns true if a cell at given position is of the provided type
bool Board::isCellOfType(int x, int y, enum CellTypes type)
{
	if (this->boundCheckPos(x, y))
	{
		return false;
	}

	return this->cells[y][x]->type == type;
}

// replace the cell at a given position with another cell
// automatically handles adding and removing from food list
void Board::replaceCellAt(const int _x, const int _y, Cell *_cell)
{
	// if out of bounds, bail
	if (this->boundCheckPos(_x, _y))
	{
		std::cerr << "Replacing cell at out-of-bounds position!";
		exit(1);
	}

	Cell *erased = this->cells[_y][_x];
	switch (erased->type)
	{
	case cell_plantmass:
	case cell_biomass:
	case cell_fruit:
	{
		Spoilable_Cell *s = static_cast<Spoilable_Cell *>(erased);
		int ticksUntilSpoil = s->TicksUntilSpoil();
		Board::Food_Slot *thisSlot = this->FoodCells[ticksUntilSpoil];
		thisSlot->erase(std::find(thisSlot->begin(), thisSlot->end(), s));
	}
	break;

	default:
		break;
	}
	delete erased;

	_cell->x = _x;
	_cell->y = _y;
	switch (_cell->type)
	{
	case cell_plantmass:
	case cell_biomass:
	case cell_fruit:
	{
		Spoilable_Cell *s = static_cast<Spoilable_Cell *>(_cell);
		Board::Food_Slot *thisFoodSlot;
		if (!this->FoodCells.count(s->TicksUntilSpoil()))
		{
			thisFoodSlot = new Board::Food_Slot(s->TicksUntilSpoil());
			this->FoodCells[s->TicksUntilSpoil()] = thisFoodSlot;
		}
		else
		{
			thisFoodSlot = this->FoodCells[s->TicksUntilSpoil()];
		}
		s->attachTicksUntilSpoil(&thisFoodSlot->ticksUntilSpoil);
		thisFoodSlot->push_back(s);
	}

	break;

	default:
		break;
	}
	this->cells[_y][_x] = _cell;
	this->DeltaCells.insert(std::pair<int, int>(_x, _y));
}

// replace the cell at a given position with another cell
// DOES NOT handle removing food from the food list
// assumed to only be used by Board::Tick() for food slots with 0 remaining time
void Board::replaceCellAt_NoTrackReplacedFood(const int _x, const int _y, Cell *_cell)
{
	// if out of bounds, bail
	if (this->boundCheckPos(_x, _y))
	{
		std::cerr << "Replacing cell at out-of-bounds position!";
		exit(1);
	}

	Cell *erased = this->cells[_y][_x];
	delete erased;

	_cell->x = _x;
	_cell->y = _y;
	switch (_cell->type)
	{
	case cell_plantmass:
	case cell_biomass:
	case cell_fruit:
	{
		Spoilable_Cell *s = static_cast<Spoilable_Cell *>(_cell);
		Board::Food_Slot *thisFoodSlot;
		if (!this->FoodCells.count(s->TicksUntilSpoil()))
		{
			thisFoodSlot = new Board::Food_Slot(s->TicksUntilSpoil());
			this->FoodCells[s->TicksUntilSpoil()] = thisFoodSlot;
		}
		else
		{
			thisFoodSlot = this->FoodCells[s->TicksUntilSpoil()];
		}
		s->attachTicksUntilSpoil(&thisFoodSlot->ticksUntilSpoil);
		thisFoodSlot->push_back(s);
	}

	break;

	default:
		break;
	}
	this->cells[_y][_x] = _cell;
	this->DeltaCells.insert(std::pair<int, int>(_x, _y));
}

void Board::swapCellAtIndex(int _x, int _y, Cell *a)
{
	// if out of bounds, bail
	if (this->boundCheckPos(_x, _y))
	{
		std::cerr << "Swapping cell at out-of-bounds position!";
		exit(1);
	}
	Cell *b = this->cells[_y][_x];
	int a_oldx = a->x;
	int a_oldy = a->y;
	b->x = a->x;
	b->y = a->y;
	this->cells[_y][_x] = a;
	this->cells[a_oldy][a_oldx] = b;
	a->x = _x;
	a->y = _y;

	this->DeltaCells.insert(std::pair<int, int>(_x, _y));
	this->DeltaCells.insert(std::pair<int, int>(a_oldx, a_oldy));
}

Organism *Board::createOrganism(const int _x, const int _y)
{
	Organism *newOrganism = new Organism(_x, _y);
	this->Organisms.push_back(newOrganism);
	return newOrganism;
}

unsigned int Board::GetNextSpecies()
{
	return this->nextSpecies++;
}

void Board::AddSpeciesMember(Organism *o)
{
	int s = o->species;
	if (this->species[s].count == 0)
	{
		this->activeSpecies_.push_back(s);
		this->species[s].classification = o->Classify();
	}
	this->species[s].count++;
	if (this->species[s].count > this->species[s].peakCount)
	{
		this->species[s].peakCount = this->species[s].count;
	}
}

void Board::RemoveSpeciesMember(unsigned int species)
{
	this->species[species].count--;
	if (this->species[species].count == 0)
	{
		auto i = this->activeSpecies_.begin();
		while (*i != species)
		{
			i++;
		}
		this->activeSpecies_.erase(i);
	}
}

const SpeciesInfo &Board::GetSpeciesInfo(uint32_t species)
{
	return this->species[species];
}

void Board::RecordEvolvedFrom(Organism* evolvedFrom, Organism* evolvedTo)
{
	this->species[evolvedFrom->species].evolvedInto.push_back(evolvedTo->species);
	this->species[evolvedTo->species].evolvedFrom = evolvedFrom->species;
}
