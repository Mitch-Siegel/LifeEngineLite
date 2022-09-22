#include <vector>

#include "lifeforms.h"
#include "board.h"
#include "rng.h"

extern Board board;
int CellEnergyDensities[cell_null] = {
	0,
	0,
	0,
	4,
	5,
	0,
	-5,
	300,
	55,
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

	switch (randInt(0, 3))
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

	case 3:
		return new Cell_Carnivore();
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
	if (isCellOfType(x_abs, y_abs, cell_plantmass))
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
			  // couldPlace = this->myOrganism->AddCell(x_rel, y_rel, cell_plantmass);
			  delete board[y_abs][x_abs];
			  Cell *droppedFood = new Cell(x_abs, y_abs, cell_plantmass, this->myOrganism);
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

case cell_plantmass:
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

// plantmass
Cell_Plantmass::~Cell_Plantmass()
{
}

Cell_Plantmass::Cell_Plantmass()
{
	this->type = cell_plantmass;
	this->myOrganism = nullptr;
	this->ticksUntilSpoil = PLANTMASS_SPOIL_TIME_MULTIPLIER;
}

Cell_Plantmass::Cell_Plantmass(int _ticksUntilSpoil) : Cell_Plantmass()
{
	this->ticksUntilSpoil = PLANTMASS_SPOIL_TIME_MULTIPLIER;
}

void Cell_Plantmass::Tick()
{
	this->ticksUntilSpoil--;
	if (this->ticksUntilSpoil < 0)
	{
		this->ticksUntilSpoil = 0;
	}
}

Cell_Plantmass *Cell_Plantmass::Clone()
{
	return new Cell_Plantmass(*this);
}

// biomass
Cell_Biomass::~Cell_Biomass()
{
}

Cell_Biomass::Cell_Biomass()
{
	this->type = cell_biomass;
	this->myOrganism = nullptr;
	this->ticksUntilSpoil = BIOMASS_SPOIL_TIME_MULTIPLIER;
}

Cell_Biomass::Cell_Biomass(int _ticksUntilSpoil) : Cell_Biomass()
{
	this->ticksUntilSpoil = BIOMASS_SPOIL_TIME_MULTIPLIER;
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
}

Cell_Leaf::Cell_Leaf(Organism *_myOrganism)
{
	this->type = cell_leaf;
	this->myOrganism = _myOrganism;
}

void Cell_Leaf::Tick()
{
	if ((this->myOrganism->myCells.size() > 2) && 
		this->myOrganism->GetEnergy() > FLOWER_COST && 
		randPercent(this->myOrganism->mutability) && 
		randPercent(FLOWER_PERCENT))
	{
		int checkDirIndex = randInt(0, 3);
		for (int i = 0; i < 4; i++)
		{
			int *thisDirection = directions[(checkDirIndex + i) % 4];
			int x_abs = this->x + thisDirection[0];
			int y_abs = this->y + thisDirection[1];
			if (board.isCellOfType(x_abs, y_abs, cell_empty))
			{
				this->myOrganism->ExpendEnergy(FLOWER_COST);
				this->myOrganism->AddCell(x_abs - this->myOrganism->x, y_abs - this->myOrganism->y, new Cell_Flower());
				return;
			}
		}
	}
	else
	{
		this->myOrganism->AddEnergy(1);
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
		this->bloomCooldown--;
		return;
	}
	else
	{
		bool couldBloom = false;
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
			couldBloom = true;
			this->myOrganism->ExpendEnergy(FLOWER_BLOOM_COST);
		}
		this->bloomCooldown = FLOWER_BLOOM_COOLDOWN;

		if (couldBloom)
		{
			if (randPercent(FLOWER_WILT_CHANCE))
			{
				this->myOrganism->ReplaceCell(this, new Cell_Leaf());
			}
		}
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
	int moveCost = (this->myOrganism->myCells.size() - 2) / 2;
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
	if (this->myOrganism->cellCounts[cell_mover] == 0 && this->myOrganism->myCells.size() > 1)
	{
		this->myOrganism->ExpendEnergy(randInt(1, 2));
	}
	int checkDirIndex = randInt(0, 3);
	for (int i = 0; i < 4; i++)
	{
		int *thisDirection = directions[(checkDirIndex + i) % 4];
		int x_abs = this->x + thisDirection[0];
		int y_abs = this->y + thisDirection[1];
		if (board.isCellOfType(x_abs, y_abs, cell_leaf) || board.isCellOfType(x_abs, y_abs, cell_flower) || board.isCellOfType(x_abs, y_abs, cell_fruit) || board.isCellOfType(x_abs, y_abs, cell_plantmass))
		{
			Cell *eaten = board.cells[y_abs][x_abs];
			Organism *eatenParent = eaten->myOrganism;
			if (eatenParent != this->myOrganism /* &&
				 this->myOrganism->canMove*/
												/*((!this->myOrganism->hasLeaf && eaten->type == cell_leaf) ||
												 (!this->myOrganism->hasFlower && eaten->type == cell_flower))*/
			)
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

				case cell_plantmass:
					this->myOrganism->AddEnergy(PLANTMASS_FOOD_ENERGY);
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


// herbivore mouth cell
Cell_Carnivore::~Cell_Carnivore()
{
}

Cell_Carnivore::Cell_Carnivore()
{
	this->type = cell_carnivore_mouth;
	this->myOrganism = nullptr;
}

Cell_Carnivore::Cell_Carnivore(Organism *_myOrganism)
{
	this->type = cell_carnivore_mouth;
	this->myOrganism = _myOrganism;
}

void Cell_Carnivore::Tick()
{
	if (this->myOrganism->cellCounts[cell_mover] == 0 && this->myOrganism->myCells.size() > 1)
	{
		this->myOrganism->ExpendEnergy(randInt(1, 2));
	}
	int checkDirIndex = randInt(0, 3);
	for (int i = 0; i < 4; i++)
	{
		int *thisDirection = directions[(checkDirIndex + i) % 4];
		int x_abs = this->x + thisDirection[0];
		int y_abs = this->y + thisDirection[1];
		if (board.isCellOfType(x_abs, y_abs, cell_biomass))
		{
			Cell *eaten = board.cells[y_abs][x_abs];
			Organism *eatenParent = eaten->myOrganism;
			if (eatenParent != this->myOrganism /* &&
				 this->myOrganism->canMove*/
												/*((!this->myOrganism->hasLeaf && eaten->type == cell_leaf) ||
												 (!this->myOrganism->hasFlower && eaten->type == cell_flower))*/
			)
			{
				/*if (eatenParent != nullptr)
				{
					std::vector<Cell *>::iterator foundLeaf = std::find(eatenParent->myCells.begin(), eatenParent->myCells.end(), eaten);
					eatenParent->myCells.erase(foundLeaf);
				}*/
				switch (eaten->type)
				{
				case cell_biomass:
					this->myOrganism->AddEnergy(BIOMASS_FOOD_ENERGY);
					break;

				default:
					std::cerr << "Impossible case for carnivore cell to eat something it shouldn't!" << std::endl;
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

Cell_Carnivore *Cell_Carnivore::Clone()
{
	return new Cell_Carnivore(*this);
}