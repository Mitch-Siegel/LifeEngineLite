#include <vector>

#include "lifeforms.h"
#include "board.h"
#include "rng.h"

extern Board board;
int CellEnergyDensities[cell_null] = {
	0,
	0,
	4,
	5,
	0,
	30,
	30,
};

Cell *GenerateRandomCell()
{
	/*
	switch (randInt(0, 1))
	{
	case 0:
		return new Cell_Leaf();

		break;

	case 1:
		return new Cell_Leaf();

		break;
	}*/

	switch (randInt(0, 2))
	{
	case 0:
		return new Cell_Mover();
		break;

	case 1:
		return new Cell_Leaf();
		break;

	case 2:
		return new Cell_Herbivore();
		break;
	}

	return nullptr;
}
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
	if (isCellOfType(x_abs, y_abs, Cell_Biomass))
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
			  // couldPlace = this->myOrganism->AddCell(x_rel, y_rel, Cell_Biomass);
			  delete board[y_abs][x_abs];
			  Cell *droppedFood = new Cell(x_abs, y_abs, Cell_Biomass, this->myOrganism);
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

case Cell_Biomass:
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
Cell_Biomass::~Cell_Biomass()
{
}

Cell_Biomass::Cell_Biomass()
{
	this->type = cell_biomass;
	this->myOrganism = nullptr;
	this->ticksUntilSpoil = BIOMASS_SPOIL_TIME;
}

Cell_Biomass::Cell_Biomass(int _ticksUntilSpoil) : Cell_Biomass()
{
	this->ticksUntilSpoil = BIOMASS_SPOIL_TIME;
}

void Cell_Biomass::Tick()
{
	this->ticksUntilSpoil--;
	if (this->ticksUntilSpoil < 0)
	{
		this->ticksUntilSpoil = 0;
	}
}

Cell_Biomass *Cell_Biomass::Clone()
{
	return new Cell_Biomass(*this);
}

// leaf cell
Cell_Leaf::~Cell_Leaf()
{
}

Cell_Leaf::Cell_Leaf()
{
	this->type = cell_leaf;
	this->myOrganism = nullptr;
	this->photosynthesisCooldown = 0;
}

Cell_Leaf::Cell_Leaf(Organism *_myOrganism)
{
	this->type = cell_leaf;
	this->myOrganism = _myOrganism;
	this->photosynthesisCooldown = 0;
}

void Cell_Leaf::Tick()
{
	if (this->photosynthesisCooldown > 0)
	{
		this->photosynthesisCooldown--;
		return;
	}
	if (this->myOrganism->GetEnergy() > FLOWER_COST && randPercent(FLOWER_PERCENT) && randPercent(FLOWER_PERCENT))
	{
		int checkDirIndex = randInt(0, 3);
		for (int i = 0; i < 4; i++)
		{
			int *thisDirection = directions[(checkDirIndex + i) % 4];
			int x_abs = this->x + thisDirection[0];
			int y_abs = this->y + thisDirection[1];
			if (board.isCellOfType(x_abs, y_abs, cell_empty))
			{
				// int x_rel = x_abs - this->myOrganism->x;
				// int y_rel = y_abs - this->myOrganism->y;
				this->myOrganism->ExpendEnergy(FLOWER_COST);
				this->myOrganism->ReplaceCell(this, new Cell_Flower());
				return;
			}
		}
	}
	else
	{
		this->myOrganism->AddEnergy(1);
		// this->myOrganism->lifespan++;
		this->photosynthesisCooldown = 0;
	}
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
	this->bloomCooldown = FLOWER_BLOOM_COOLDOWN;
}

Cell_Flower::Cell_Flower(Organism *_myOrganism)
{
	this->type = cell_flower;
	this->myOrganism = _myOrganism;
	this->bloomCooldown = FLOWER_BLOOM_COOLDOWN;
}

void Cell_Flower::Tick()
{
	if (this->bloomCooldown > 0)
	{
		// this->myOrganism->lifespan += 2;
		this->bloomCooldown--;
		return;
	}
	else
	{
		if (this->myOrganism->GetEnergy() > FLOWER_BLOOM_COST)
		{
			int checkDirIndex = randInt(0, 3);
			for (int i = 0; i < 4; i++)
			{
				int *thisDirection = directions[(checkDirIndex + i) % 4];
				int x_abs = this->x + thisDirection[0];
				int y_abs = this->y + thisDirection[1];
				if (board.isCellOfType(x_abs, y_abs, cell_empty))
				{
					board.replaceCellAt(x_abs, y_abs, new Cell_Fruit(this->myOrganism->mutability));
					break;
					// this->myOrganism->AddCell(x_rel, y_rel, new Cell_Fruit());
				}
			}
			this->myOrganism->ExpendEnergy(FLOWER_BLOOM_COST);
		}
		this->bloomCooldown = FLOWER_BLOOM_COOLDOWN;

		/*if (randPercent(FLOWER_PERCENT))
		{
			Organism *hatched = board.createOrganism(this->x, this->y);
			hatched->AddCell(0, 0, GenerateRandomCell());
			int checkedDirection = randInt(0, 7);
			for (int i = 0; i < 7; i++)
			{
				int *thisDirection = directions[(checkedDirection + i) & 7];
				if (board.isCellOfType(this->x + thisDirection[0], this->y + thisDirection[1], cell_empty))
				{
					hatched->AddCell(thisDirection[0], thisDirection[1], GenerateRandomCell());
					break;
				}
			}
			this->myOrganism->RemoveCell(this);
		}
		else
		{
			this->myOrganism->ReplaceCell(this, new Cell_Leaf());
		}*/
	}
}

Cell_Flower *Cell_Flower::Clone()
{
	return new Cell_Flower(*this);
}

// fruit cell
Cell_Fruit::~Cell_Fruit()
{
}

Cell_Fruit::Cell_Fruit()
{
	this->type = cell_fruit;
	this->myOrganism = nullptr;
	this->ticksUntilSpoil = FRUIT_SPOIL_TIME;
	this->parentMutability = 50;
}

Cell_Fruit::Cell_Fruit(int _parentMutability)
{
	this->type = cell_fruit;
	this->myOrganism = nullptr;
	this->ticksUntilSpoil = FRUIT_SPOIL_TIME;
	this->parentMutability = _parentMutability;
}

Cell_Fruit::Cell_Fruit(Organism *_myOrganism)
{
	this->type = cell_fruit;
	this->myOrganism = _myOrganism;
	this->ticksUntilSpoil = FRUIT_SPOIL_TIME;
	this->parentMutability = 50;
}

void Cell_Fruit::Tick()
{
	this->ticksUntilSpoil--;
	if (this->ticksUntilSpoil < 0)
	{
		this->ticksUntilSpoil = 0;
	}
}

Cell_Fruit *Cell_Fruit::Clone()
{
	return new Cell_Fruit(*this);
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
	int moveCost = 0; // this->myOrganism->myCells.size() / 3;
	moveCost = (moveCost > 0) ? moveCost : 1;
	this->myOrganism->ExpendEnergy(moveCost);
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
	this->myOrganism->ExpendEnergy(1);
	int checkDirIndex = randInt(0, 3);
	for (int i = 0; i < 4; i++)
	{
		int *thisDirection = directions[(checkDirIndex + i) % 4];
		int x_abs = this->x + thisDirection[0];
		int y_abs = this->y + thisDirection[1];
		if (board.isCellOfType(x_abs, y_abs, cell_leaf) || board.isCellOfType(x_abs, y_abs, cell_flower) || board.isCellOfType(x_abs, y_abs, cell_fruit) || board.isCellOfType(x_abs, y_abs, cell_biomass))
		{
			Cell *eaten = board.cells[y_abs][x_abs];
			Organism *eatenParent = eaten->myOrganism;
			if (eatenParent != this->myOrganism || eaten->type != cell_leaf)
			{
				/*if (eatenParent != nullptr)
				{
					std::vector<Cell *>::iterator foundLeaf = std::find(eatenParent->myCells.begin(), eatenParent->myCells.end(), eaten);
					eatenParent->myCells.erase(foundLeaf);
				}*/
				switch (eaten->type)
				{
				case cell_leaf:
					this->myOrganism->AddEnergy(LEAF_FOOD_ENERGY);
					break;

				case cell_flower:
					this->myOrganism->AddEnergy(FLOWER_FOOD_ENERGY);
					break;

				case cell_fruit:
					this->myOrganism->AddEnergy(FRUIT_FOOD_ENERGY);
					break;

				case cell_biomass:
					this->myOrganism->AddEnergy(BIOMASS_FOOD_ENERGY);
					break;

				default:
					std::cerr << "Impossible case for herbivore cell to eat something it shouldn't!" << std::endl;
					exit(1);
				}
				if (eaten->myOrganism != nullptr)
				{
					eaten->myOrganism->RemoveCell(eaten);
					board.replaceCell(eaten, new Cell_Empty());
				}
				else
				{
					board.replaceCell(eaten, new Cell_Empty());
				}
				this->myOrganism->brain.Reward();
			}
		}
	}
}

Cell_Herbivore *Cell_Herbivore::Clone()
{
	return new Cell_Herbivore(*this);
}