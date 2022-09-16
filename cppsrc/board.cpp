#include "board.h"
#include "lifeforms.h"

#include <curses.h>
#include <iostream>

int directions[4][2] = {{1, 0}, {0, -1}, {-1, 0}, {0, 1}};
extern Board board;

Board::Board(const int _dim_x, const int _dim_y)
{
	this->dim_x = _dim_x;
	this->dim_y = _dim_y;
	// this->cells = new Cell *[_dim_y];

	for (int y = 0; y < _dim_y; y++)
	{
		this->cells.push_back(std::vector<Cell>());
		for(int x = 0; x < _dim_x; x++)
		{
			this->cells[y].push_back(Cell_Empty());

		}
		// this->cells[y] = new Cell[_dim_x];
	}
	for (int y = 0; y < _dim_y; y++)
	{
		for (int x = 0; x < _dim_x; x++)
		{
			this->cells[y][x].x = x;
			this->cells[y][x].y = y;

		}
	}
}

void Board::Tick()
{
	for (size_t i = 0; i < this->Organisms.size(); i++)
	{
		// mvprintw(BOARD_DIM + i, 0, "%lu", i);
		if (!this->Organisms[i]->alive)
		{
			this->Organisms.erase(this->Organisms.begin() + i);
			i--;
		}
		else
		{
			mvprintw(this->dim_y + i + 1, 4, "%d %d: %lu energy, %lu cells (%lu ticks old, %d lifespan)",
					 Organisms[i]->x, Organisms[i]->y, Organisms[i]->energy, Organisms[i]->myCells.size(), Organisms[i]->age, Organisms[i]->lifespan);
			Organism *replicated = this->Organisms[i]->Tick();

			if (replicated != nullptr)
			{
				Organism replicatedObj = *replicated;
				// Organisms.push_back(replicatedObj);
			}
		}
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

	return this->cells[y][x].type == type;
}

Cell *Board::replaceCellAt(const int _x, const int _y, const Cell &_cell)
{
	// if out of bounds, return a cell with null type
	if (this->boundCheckPos(_x, _y))
	{
		std::cerr << "Replacing cell at out-of-bounds position!";
		exit(1);
		return nullptr;
	}
	// otherwise return the cell we just replaced
	// _cell.x = _x;
	// _cell.y = _y;
	this->cells[_y][_x] = _cell;
	return &this->cells[_y][_x];
}

Organism *Board::createOrganism(const int _x, const int _y)
{
	Organism *newOrganism = new Organism(_x, _y);
	// Cell *coreCell;
	// if((coreCell = this->replaceCellAt(_x, _y, Cell(_x, _y, _coreCellType, newOrganism))) == nullptr)
	// {
	// std::cerr << "Error creating organism at " << _x << " " << _y << ": Cell was occupied!";
	// exit(1);
	// }
	this->Organisms.push_back(newOrganism);
	return newOrganism;
}
