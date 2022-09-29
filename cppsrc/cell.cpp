#include <vector>

#include "lifeforms.h"
#include "board.h"
#include "rng.h"

extern Board board;
int CellEnergyDensities[cell_null] = {
	0,	// empty
	0,	// plantmass
	0,	// biomass
	4,	// leaf
	5,	// flower
	0,	// fruit
	20, // herbivore
	0,	// carnivore
	25, // mover
	15, // killer
	15, // armor
};

Cell *GenerateRandomCell()
{

	switch (randInt(0, 5))
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

	case 4:
		return new Cell_Killer();
		break;

	case 5:
		return new Cell_Armor();
		break;
	}

	return nullptr;
}

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
	this->flowering = randPercent(LEAF_FLOWERING_ABILITY_PERCENT);
}

Cell_Leaf::Cell_Leaf(int floweringPercent)
{
	this->type = cell_leaf;
	this->myOrganism = nullptr;
	this->flowering = randPercent(floweringPercent);
}

Cell_Leaf::Cell_Leaf(Organism *_myOrganism)
{
	this->type = cell_leaf;
	this->myOrganism = _myOrganism;
	this->flowering = randPercent(LEAF_FLOWERING_ABILITY_PERCENT);
}

void Cell_Leaf::Tick()
{
	if (this->flowering &&
		(this->myOrganism->myCells.size() > 2) &&
		this->myOrganism->GetEnergy() > FLOWER_COST &&
		randPercent(PLANT_GROW_PERCENT))
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
		if (this->myOrganism->age % 4 == 0 || this->myOrganism->age % 5 == 0 || this->myOrganism->age % 6 == 0 || this->myOrganism->age % 7 == 0)
		{
			this->myOrganism->AddEnergy(1);
		}
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
				// if (randPercent(FLOWER_EXPAND_PERCENT))
				// {
				this->myOrganism->ReplaceCell(this, new Cell_Leaf());
				// }
				// else
				// {
				// this->myOrganism->RemoveCell(this);
				// board.replaceCell(this, new Cell_Empty());
				// }
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
	this->myOrganism->ExpendEnergy(1);
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
	this->digestCooldown = 1;
	this->direction = randInt(0, 3);
}

Cell_Herbivore::Cell_Herbivore(Organism *_myOrganism)
{
	this->type = cell_herbivore_mouth;
	this->myOrganism = _myOrganism;
	this->digestCooldown = 1;
	this->direction = randInt(0, 3);
}

void Cell_Herbivore::Tick()
{
	if (this->digestCooldown > 0)
	{
		this->digestCooldown--;
		return;
	}
	bool couldEat = false;
	bool valid = false;
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
		if (board.boundCheckPos(x_abs, y_abs))
		{
			continue;
		}
		Cell *potentiallyEaten = board.cells[y_abs][x_abs];
		if (potentiallyEaten->type == cell_leaf || potentiallyEaten->type == cell_flower || potentiallyEaten->type == cell_fruit || potentiallyEaten->type == cell_plantmass)
		{
			Organism *eatenParent = potentiallyEaten->myOrganism;
			if (eatenParent != this->myOrganism)
			{
				switch (potentiallyEaten->type)
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
				if (potentiallyEaten->myOrganism != nullptr)
				{
					potentiallyEaten->myOrganism->RemoveCell(potentiallyEaten);
					board.replaceCell(potentiallyEaten, new Cell_Empty());
				}
				else
				{
					board.replaceCell(potentiallyEaten, new Cell_Empty());
				}
				couldEat = true;
			}
			// set valid just so we have a chance of being able to skip the check below
			else
			{
				valid = true;
			}
		}
	}

	if (couldEat)
	{
		this->myOrganism->brain.Reward();
		this->digestCooldown = HERB_DIGEST_TIME;
	}

	if (!valid)
	{
		for (int i = 0; i < 8; i++)
		{
			int *thisDirection = directions[i];
			int abs_x = this->x + thisDirection[0];
			int abs_y = this->y + thisDirection[1];
			valid = valid || (!board.boundCheckPos(abs_x, abs_y) && board.cells[abs_y][abs_x]->myOrganism == this->myOrganism);
		}
	}

	if (!valid)
	{
		this->myOrganism->RemoveCell(this);
		board.replaceCell(this, new Cell_Empty());
	}
}

Cell_Herbivore *Cell_Herbivore::Clone()
{
	return new Cell_Herbivore(*this);
}

// carnivore mouth cell
Cell_Carnivore::~Cell_Carnivore()
{
}

Cell_Carnivore::Cell_Carnivore()
{
	this->type = cell_carnivore_mouth;
	this->myOrganism = nullptr;
	this->digestCooldown = CARN_DIGEST_TIME;
}

Cell_Carnivore::Cell_Carnivore(Organism *_myOrganism)
{
	this->type = cell_carnivore_mouth;
	this->myOrganism = _myOrganism;
	this->digestCooldown = CARN_DIGEST_TIME;
}

void Cell_Carnivore::Tick()
{
	if (this->digestCooldown > 0)
	{
		this->digestCooldown--;
		return;
	}
	bool couldEat = false;
	bool valid = false;
	if (this->myOrganism->cellCounts[cell_mover] == 0 && this->myOrganism->myCells.size() > 1)
	{
		this->myOrganism->ExpendEnergy(randInt(2 * ENERGY_DENSITY_MULTIPLIER, 4 * ENERGY_DENSITY_MULTIPLIER));
	}
	int checkDirIndex = randInt(0, 3);
	for (int i = 0; i < 4; i++)
	{
		int *thisDirection = directions[(checkDirIndex + i) % 4];
		int x_abs = this->x + thisDirection[0];
		int y_abs = this->y + thisDirection[1];
		if (board.boundCheckPos(x_abs, y_abs))
		{
			continue;
		}
		Cell *potentiallyEaten = board.cells[y_abs][x_abs];
		if (potentiallyEaten->type == cell_biomass)
		{
			this->myOrganism->AddEnergy(BIOMASS_FOOD_ENERGY);
			board.replaceCell(potentiallyEaten, new Cell_Empty());
			couldEat = true;
		}
		this->digestCooldown = CARN_DIGEST_TIME;
	}

	if (!valid)
	{
		for (int i = 0; i < 8; i++)
		{
			int *thisDirection = directions[i];
			int abs_x = this->x + thisDirection[0];
			int abs_y = this->y + thisDirection[1];
			valid = valid || (!board.boundCheckPos(abs_x, abs_y) && board.cells[abs_y][abs_x]->myOrganism == this->myOrganism);
		}
	}

	if (couldEat)
	{
		this->myOrganism->brain.Reward();
		this->digestCooldown = HERB_DIGEST_TIME;
	}

	if (!valid)
	{
		this->myOrganism->RemoveCell(this);
		board.replaceCell(this, new Cell_Empty());
	}
}

Cell_Carnivore *Cell_Carnivore::Clone()
{
	return new Cell_Carnivore(*this);
}

// killer cell
Cell_Killer::~Cell_Killer()
{
}

Cell_Killer::Cell_Killer()
{
	this->type = cell_killer;
	this->myOrganism = nullptr;
}

Cell_Killer::Cell_Killer(Organism *_myOrganism)
{
	this->type = cell_killer;
	this->myOrganism = _myOrganism;
}

void Cell_Killer::Tick()
{
	int damageDone = 0;
	bool valid = false;
	for (int i = 0; i < 4; i++)
	{
		int *thisDirection = directions[i];
		int abs_x = this->x + thisDirection[0];
		int abs_y = this->y + thisDirection[1];
		if (!board.boundCheckPos(abs_x, abs_y))
		{
			Cell *adjacent = board.cells[abs_y][abs_x];
			if (adjacent->myOrganism != nullptr)
			{
				if (adjacent->myOrganism != this->myOrganism)
				{
					damageDone++;
					adjacent->myOrganism->Damage(1);
				}
				else
				{
					valid = true;
				}
			}
		}
	}

	if (!valid)
	{
		for (int i = 0; i < 8; i++)
		{
			int *thisDirection = directions[i];
			int abs_x = this->x + thisDirection[0];
			int abs_y = this->y + thisDirection[1];
			valid = valid || (!board.boundCheckPos(abs_x, abs_y) && board.cells[abs_y][abs_x]->myOrganism == this->myOrganism);
		}
	}
	this->myOrganism->ExpendEnergy((damageDone * KILLER_DAMAGE_COST));
	if (!valid)
	{
		this->myOrganism->RemoveCell(this);
		board.replaceCell(this, new Cell_Empty());
	}
}

Cell_Killer *Cell_Killer::Clone()
{
	return new Cell_Killer(*this);
}

// armor cell
Cell_Armor::~Cell_Armor()
{
}

Cell_Armor::Cell_Armor()
{
	this->type = cell_armor;
	this->myOrganism = nullptr;
}

Cell_Armor::Cell_Armor(Organism *_myOrganism)
{
	this->type = cell_armor;
	this->myOrganism = _myOrganism;
}

void Cell_Armor::Tick()
{
	bool valid = false;
	this->myOrganism->ExpendEnergy(1);
	for (int i = 0; i < 8 && !valid; i++)
	{
		int *thisDirection = directions[i];
		int abs_x = this->x + thisDirection[0];
		int abs_y = this->y + thisDirection[1];
		valid = valid || (!board.boundCheckPos(abs_x, abs_y) && board.cells[abs_y][abs_x]->myOrganism == this->myOrganism);
	}
	if (!valid)
	{
		this->myOrganism->RemoveCell(this);
		board.replaceCell(this, new Cell_Empty());
	}
}

Cell_Armor *Cell_Armor::Clone()
{
	return new Cell_Armor(*this);
}
