#include "lifeforms.h"
#include "board.h"
#include <vector>
#include <stdlib.h>
#include <curses.h>

extern Board board;
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

/*
Cell *Cell::Clone()
{
	return new Cell(*this);
}*/

Cell::~Cell()
{
}


// empty cell
Cell_Empty::~Cell_Empty()
{
}

Cell_Empty::Cell_Empty()
{
	this->type = cell_empty;
	this->myOrganism = nullptr;
}

// Cell_Empty::~Cell()
// {

// }

void Cell_Empty::Tick()
{
	
}

Cell_Empty *Cell_Empty::Clone()
{
	return new Cell_Empty(*this);
}

// food cell

Cell_Food::~Cell_Food()
{
}

Cell_Food::Cell_Food()
{
	this->type = cell_food;
	this->myOrganism = nullptr;
}

Cell_Food::Cell_Food(int _ticksUntilSpoil) : Cell_Food()
{
	this->ticksUntilSpoil = _ticksUntilSpoil;
}

void Cell_Food::Tick()
{
	mvprintw(27, 0, "food::tick()");
	this->ticksUntilSpoil--;
	if(this->ticksUntilSpoil == 0)
	{
		board.replaceCellAt(this->x, this->y, new Cell_Empty());
	}
}

Cell_Food *Cell_Food::Clone()
{
	return new Cell_Food(*this);
}


// leaf cell
Cell_Leaf::~Cell_Leaf()
{
}

Cell_Leaf::Cell_Leaf()
{
	this->type = cell_leaf;
	this->myOrganism = nullptr;
}

Cell_Leaf::Cell_Leaf(/*int _x, int _y, */Organism *_myOrganism)
{
	this->type = cell_leaf;
	this->myOrganism = _myOrganism;
}

// Cell_Leaf::Cell_Leaf(const Cell_Leaf &c) : Cell(c)
// {
// }

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

