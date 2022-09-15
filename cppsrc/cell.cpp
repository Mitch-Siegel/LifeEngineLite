#include "lifeforms.h"
#include "board.h"
#include <vector>
#include <stdlib.h>

extern std::vector<Cell *> foodCells;

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

	if (this->actionCooldown > 0)
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
		// TODO: check if food is fruit attached to an organism
		// if so, need to detach it form that organism's cell list
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
	*/
	case cell_flower:

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
					// couldPlace = this->myOrganism->AddCell(x_rel, y_rel, cell_food);
					delete board[y_abs][x_abs];
					Cell *droppedFood = new Cell(x_abs, y_abs, cell_food, this->myOrganism);
					droppedFood->actionCooldown = FRUIT_SPOILTIME;
					board[y_abs][x_abs] = droppedFood;
					foodCells.push_back(droppedFood);
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
				this->actionCooldown = FLOWER_COOLDOWN;
				this->myOrganism->ExpendEnergy(15);
			}
			else
			{
				this->actionCooldown = FLOWER_COOLDOWN * 4;
			}
		}
		break;

	case cell_leaf:
		this->myOrganism->energy++;
		this->actionCooldown = 1;
		break;

	case cell_food:
		break;
	}
}
