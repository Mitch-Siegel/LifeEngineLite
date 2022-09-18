#include <vector>

#include "lifeforms.h"
#include "board.h"
#include "rng.h"

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
	this->ticksUntilSpoil--;
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

Cell_Leaf::Cell_Leaf(Organism *_myOrganism)
{
	this->type = cell_leaf;
	this->myOrganism = _myOrganism;
}

void Cell_Leaf::Tick()
{
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


// flower cell
Cell_Flower::~Cell_Flower()
{
}

Cell_Flower::Cell_Flower()
{
	this->type = cell_flower;
	this->myOrganism = nullptr;
}

Cell_Flower::Cell_Flower(Organism *_myOrganism)
{
	this->type = cell_flower;
	this->myOrganism = _myOrganism;
}

void Cell_Flower::Tick()
{
	if (this->bloomCooldown > 0)
	{
		this->bloomCooldown--;
		return;
	}
}

Cell_Flower *Cell_Flower::Clone()
{
	return new Cell_Flower(*this);
}


// mover cell
Cell_Mover::~Cell_Mover()
{
}

Cell_Mover::Cell_Mover()
{
	this->type = cell_mover;
	this->myOrganism = nullptr;
}

Cell_Mover::Cell_Mover(Organism *_myOrganism)
{
	this->type = cell_mover;
	this->myOrganism = _myOrganism;
}

void Cell_Mover::Tick()
{
	
}

Cell_Mover *Cell_Mover::Clone()
{
	return new Cell_Mover(*this);
}

// herbivore mouth cell
Cell_Herbivore::~Cell_Herbivore()
{
}

Cell_Herbivore::Cell_Herbivore()
{
	this->type = cell_herbivore_mouth;
	this->myOrganism = nullptr;
}

Cell_Herbivore::Cell_Herbivore(Organism *_myOrganism)
{
	this->type = cell_herbivore_mouth;
	this->myOrganism = _myOrganism;
}

void Cell_Herbivore::Tick()
{
	int checkDirIndex = randInt(0, 3);
	for(int i = 0; i < 4; i++)
	{
		int *thisDirection = directions[checkDirIndex + i % 4];
		int x_abs = this->x + thisDirection[0];
		int y_abs = this->y + thisDirection[1];
		if(board.isCellOfType(x_abs, y_abs, cell_leaf))
		{
			Cell *eatenLeaf = board.cells[y_abs][x_abs];
			Organism *leafParent = eatenLeaf->myOrganism;
			std::vector<Cell *>::iterator foundLeaf = std::find(leafParent->myCells.begin(), leafParent->myCells.end(), eatenLeaf);
			leafParent->myCells.erase(foundLeaf);
			board.replaceCell(eatenLeaf, new Cell_Empty());
			this->myOrganism->energy += 10;
		}
	}
}

Cell_Herbivore *Cell_Herbivore::Clone()
{
	return new Cell_Herbivore(*this);
}