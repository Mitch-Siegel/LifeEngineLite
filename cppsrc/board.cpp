#include "board.h"
#include "lifeforms.h"

#include <curses.h>
#include <iostream>
#include <algorithm>

int directions[4][2] = {{1, 0}, {0, -1}, {-1, 0}, {0, 1}};
extern Board board;

Board::Board(const int _dim_x, const int _dim_y)
{
	this->dim_x = _dim_x;
	this->dim_y = _dim_y;

	for (int y = 0; y < _dim_y; y++)
	{
		this->cells.push_back(std::vector<Cell *>());
		for (int x = 0; x < _dim_x; x++)
		{
			this->cells[y].push_back(new Cell_Empty());
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
	size_t organismCellsCount = 0;
	size_t organismEnergyCount = 0;
	for (size_t i = 0; i < this->FoodCells.size(); i++)
	{
		this->FoodCells[i]->Tick();
		if (((Cell_Biomass *)this->FoodCells[i])->ticksUntilSpoil == 0)
		{
			board.replaceCellAt(this->FoodCells[i]->x, this->FoodCells[i]->y, new Cell_Empty());
			i--;
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
			// mvprintw(this->dim_y + i + 1, 4, "%d %d: %lu energy, %lu cells (%lu ticks old, %d lifespan)",
					//  Organisms[i]->x, Organisms[i]->y, Organisms[i]->energy, Organisms[i]->myCells.size(), Organisms[i]->age, Organisms[i]->lifespan);
			Organism *replicated = this->Organisms[i]->Tick();

			if (replicated != nullptr)
			{
				Organisms.push_back(replicated);
			}
		}
	}

	printf("%lu organisms, average size %.3f cells, %.3f energy\n", this->Organisms.size(), organismCellsCount / (float)(this->Organisms.size()), organismEnergyCount / (float)(this->Organisms.size()));
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
	
	if (this->cells[_y][_x]->type == cell_biomass)
	{
		this->FoodCells.erase(std::find(this->FoodCells.begin(), this->FoodCells.end(), this->cells[_y][_x]));
	}
	delete this->cells[_y][_x];

	_cell->x = _x;
	_cell->y = _y;
	if (_cell->type == cell_biomass)
	{
		this->FoodCells.push_back(_cell);
	}
	this->cells[_y][_x] = _cell;
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
}

Organism *Board::createOrganism(const int _x, const int _y)
{
	Organism *newOrganism = new Organism(_x, _y);
	this->Organisms.push_back(newOrganism);
	return newOrganism;
}
