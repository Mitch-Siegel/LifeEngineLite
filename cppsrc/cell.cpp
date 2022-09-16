#include "lifeforms.h"
#include "board.h"
#include <vector>
#include <stdlib.h>
#include <curses.h>

extern std::vector<Cell *> foodCells;

/*
Cell::Cell()
{
	this->x = -1;
	this->y = -1;
	this->myOrganism = nullptr;
	this->type = cell_null;
	std::cout << "Default cell constructor" << std::endl;
}
*/

/*
Cell::Cell(int _x, int _y, enum CellTypes _type, Organism *_myOrganism)
{
	this->x = _x;
	this->y = _y;
	this->type = _type;
	this->myOrganism = _myOrganism;
}

Cell::Cell(enum CellTypes _type, Organism *_myOrganism)
{
	this->x = -1;
	this->y = -1;
	this->type = _type;
	this->myOrganism = _myOrganism;
}*/


/*
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
		*/
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
/*
}
break;
*/
/*
case cell_producer:
*/
/*
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
}*/

void Cell::Tick()
{
	mvprintw(28, 28, "cell::Tick()");
}
/*
Cell *Cell::Clone()
{
	return new Cell(*this);
}*/

Cell *Cell::Clone()
{
	return new Cell(*this);
}

// empty cell
Cell_Empty::Cell_Empty()// : Cell(cell_empty, nullptr)
{
	this->type = cell_empty;
	this->myOrganism = nullptr;
}

void Cell_Empty::Tick()
{
}

Cell_Empty *Cell_Empty::Clone()
{
	return new Cell_Empty(*this);
}

// food cell
Cell_Food::Cell_Food()// : Cell(cell_food, nullptr)
{
	this->type = cell_food;
	this->myOrganism = nullptr;
}

Cell_Food::Cell_Food(/*int _x, int _y, */int _ticksUntilSpoil)// : Cell(/*_x, _y, */cell_food, nullptr)
{
	this->ticksUntilSpoil = _ticksUntilSpoil;
}

void Cell_Food::Tick()
{

}

// leaf cell
Cell_Leaf::Cell_Leaf()// : Cell(cell_leaf, nullptr)
{
	this->type = cell_leaf;
	this->myOrganism = nullptr;
}

Cell_Leaf::Cell_Leaf(/*int _x, int _y, */Organism *_myOrganism)// : Cell(cell_leaf, nullptr)
{
	this->type = cell_leaf;
	this->myOrganism = _myOrganism;
}

Cell_Leaf::Cell_Leaf(const Cell_Leaf &c) : Cell(c)
{
}

void Cell_Leaf::Tick()
{
	mvprintw(27, 0, "leafcell::tick()");
	if (this->photosynthesisCooldown > 0)
	{
		this->photosynthesisCooldown--;
		return;
	}
	this->myOrganism->energy++;
	this->photosynthesisCooldown = 1;
}

Cell_Leaf *Cell_Leaf::Clone()
{
	return new Cell_Leaf(*this);
}

