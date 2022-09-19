#include "board.h"
#include "lifeforms.h"
#include "rng.h"

#include <curses.h>
#include <iostream>
#include <algorithm>

int directions[8][2] = {{1, 0}, {0, -1}, {-1, 0}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
extern Board board;

Board::Board(const int _dim_x, const int _dim_y)
{
	this->tickCount = 0;
	this->dim_x = _dim_x;
	this->dim_y = _dim_y;
	this->Organisms = std::vector<Organism *>();

	for (int y = 0; y < _dim_y; y++)
	{
		this->cells.push_back(std::vector<Cell *>());
		for (int x = 0; x < _dim_x; x++)
		{
			this->cells[y].push_back(new Cell_Empty());
			this->DeltaCells.push_back(false);
		}
	}
	for (int y = 0; y < _dim_y; y++)
	{
		for (int x = 0; x < _dim_x; x++)
		{
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

void Board::Tick()
{
	this->tickCount++;
	size_t organismCellsCount = 0;
	size_t organismEnergyCount = 0;
	size_t organismLifespan = 0;
	size_t mutabilityTotal = 0;
	for (size_t i = 0; i < this->FoodCells.size(); i++)
	{
		this->FoodCells[i]->Tick();
		switch (this->FoodCells[i]->type)
		{
		case cell_plantmass:
			if (((Cell_Plantmass *)this->FoodCells[i])->ticksUntilSpoil == 0)
			{
				board.replaceCell(this->FoodCells[i], new Cell_Empty());
				i--;
			}
			break;

		case cell_fruit:
			if (((Cell_Fruit *)this->FoodCells[i])->ticksUntilSpoil == 0)
			{
				if (randPercent(FRUIT_GROW_PERCENT) && randPercent(FRUIT_GROW_PERCENT))
				{
					Organism *grownFruit = this->createOrganism(this->FoodCells[i]->x, this->FoodCells[i]->y);
					grownFruit->mutability = ((Cell_Fruit *)this->FoodCells[i])->parentMutability;
					board.replaceCell(this->FoodCells[i], new Cell_Empty());
					grownFruit->AddCell(0, 0, GenerateRandomCell());
					Cell *secondRandomCell = GenerateRandomCell();
					bool couldAddSecond = false;
					int dirIndex = randInt(0, 7);
					for (int i = 0; i < 8; i++)
					{
						int *thisDirection = directions[(i + dirIndex) % 8];
						if (grownFruit->AddCell(thisDirection[0], thisDirection[1], secondRandomCell) == 0)
						{
							couldAddSecond = true;
							break;
						}
					}
					if(!couldAddSecond)
					{
						delete secondRandomCell;
					}

					grownFruit->RecalculateStats();
					grownFruit->lifespan = grownFruit->myCells.size() * LIFESPAN_MULTIPLIER;
					// grownFruit->AddEnergy(randInt(grownFruit->GetMaxEnergy() / 2, grownFruit->GetMaxEnergy()));
					grownFruit->AddEnergy(grownFruit->GetMaxEnergy());
				}
				else
				{
					board.replaceCellAt(this->FoodCells[i]->x, this->FoodCells[i]->y, new Cell_Plantmass(FRUIT_SPOIL_TIME));
				}
				i--;
			}
			break;

		default:
			std::cerr << "Impossible case for food cell to be something it shouldn't!" << std::endl;
			exit(1);
		}
	}

	for (size_t i = 0; i < this->Organisms.size(); i++)
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
			organismCellsCount += this->Organisms[i]->myCells.size();
			organismEnergyCount += this->Organisms[i]->GetEnergy();
			organismLifespan += this->Organisms[i]->lifespan;
			mutabilityTotal += this->Organisms[i]->mutability;
			// printf("%d %d: %lu/%lu energy, %lu cells (%lu ticks old, %d lifespan), repcd %d\n",
			//  Organisms[i]->x, Organisms[i]->y, Organisms[i]->GetEnergy(), Organisms[i]->GetMaxEnergy(), Organisms[i]->myCells.size(), Organisms[i]->age, Organisms[i]->lifespan, Organisms[i]->reproductionCooldown);
			Organism *replicated = this->Organisms[i]->Tick();

			if (replicated != nullptr)
			{
				Organisms.push_back(replicated);
			}
		}
	}
	if (this->tickCount % 100 == 0)
	{

		printf("%lu organisms, average size %.1f cells, %.1f energy, %.0f lifespan, %.1f%% mutability\n\n",
			   this->Organisms.size(),
			   organismCellsCount / (float)(this->Organisms.size()),
			   organismEnergyCount / (float)(this->Organisms.size()),
			   organismLifespan / (float)(this->Organisms.size()),
			   mutabilityTotal / (float)(this->Organisms.size()));
	}
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

	switch (this->cells[_y][_x]->type)
	{
	case cell_plantmass:
	case cell_fruit:
		this->FoodCells.erase(std::find(this->FoodCells.begin(), this->FoodCells.end(), this->cells[_y][_x]));
		break;

	default:
		break;
	}
	delete this->cells[_y][_x];

	_cell->x = _x;
	_cell->y = _y;
	switch (_cell->type)
	{
	case cell_plantmass:
	case cell_fruit:
		this->FoodCells.push_back(_cell);
		break;

	default:
		break;
	}
	this->cells[_y][_x] = _cell;
	this->DeltaCells[(_y * this->dim_y) + _x] = true;
}

void Board::replaceCell(Cell *_replaced, Cell *_newCell)
{
	this->replaceCellAt(_replaced->x, _replaced->y, _newCell);
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

	this->DeltaCells[(_y * this->dim_y) + _x] = true;
	this->DeltaCells[(a_oldx * this->dim_y) + a_oldx] = true;
}

Organism *Board::createOrganism(const int _x, const int _y)
{
	Organism *newOrganism = new Organism(_x, _y);
	this->Organisms.push_back(newOrganism);
	return newOrganism;
}
