#include <vector>

#include "lifeforms.h"
#include "board.h"
#include "rng.h"

extern Board board;
int CellEnergyDensities[cell_null] = {
	0,	// empty
	0,	// plantmass
	0,	// biomass
	1,	// leaf
	20, // bark
	2,	// flower
	0,	// fruit
	15, // herbivore
	45, // carnivore
	20, // mover
	7,	// killer
	5,	// armor
};

Cell *GenerateRandomCell()
{

	switch (randInt(0, 6))
	{
	case 0:
		return new Cell_Mover();
		break;

	case 1:
		return new Cell_Leaf();
		break;

	case 2:
		return new Cell_Bark();
		break;

	case 3:
		return new Cell_Herbivore();
		break;

	case 4:
		return new Cell_Carnivore();
		break;

	case 5:
		return new Cell_Killer();
		break;

	case 6:
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
	this->flowersRemaining = randInt(0, randPercent(LEAF_FLOWERING_ABILITY_PERCENT) * (randInt(1, LEAF_MAX_FLOWERS)));
}

Cell_Leaf::Cell_Leaf(int floweringPercent)
{
	this->type = cell_leaf;
	this->myFlower = nullptr;
	this->myOrganism = nullptr;
	this->flowersRemaining = randInt(0, randPercent(floweringPercent) * LEAF_MAX_FLOWERS);
}

Cell_Leaf::Cell_Leaf(Organism *_myOrganism)
{
	this->type = cell_leaf;
	this->myFlower = nullptr;
	this->myOrganism = _myOrganism;
	this->flowersRemaining = randInt(0, randPercent(LEAF_FLOWERING_ABILITY_PERCENT) * LEAF_MAX_FLOWERS);
}

void Cell_Leaf::Tick()
{
	if ((this->flowersRemaining > 0) &&
		this->myFlower == nullptr &&
		this->myOrganism->GetEnergy() > (FLOWER_COST + 3) &&
		randPercent(PLANT_GROW_PERCENT))
	{
		// don't grow a flower if this leaf is already adjacent to a flower
		/*for (int i = 0; i < 4; i++)
		{
			int *thisDirection = directions[i];
			int x_abs = this->x + thisDirection[0];
			int y_abs = this->y + thisDirection[1];
			if (board.isCellOfType(x_abs, y_abs, cell_flower) && board.cells[y_abs][x_abs]->myOrganism == this->myOrganism)
			{
				return;
			}
		}*/

		int checkDirIndex = randInt(0, 3);
		for (int i = 0; i < 4; i++)
		{
			int *thisDirection = directions[(checkDirIndex + i) % 4];
			int x_abs = this->x + thisDirection[0];
			int y_abs = this->y + thisDirection[1];
			if (board.isCellOfType(x_abs, y_abs, cell_empty))
			{
				this->myOrganism->ExpendEnergy(FLOWER_COST);
				Cell_Flower *newFlower = new Cell_Flower();
				this->myFlower = newFlower;
				newFlower->myLeaf = this;
				this->myOrganism->AddCell(x_abs - this->myOrganism->x, y_abs - this->myOrganism->y, newFlower);
				this->flowersRemaining--;
				return;
			}
		}
	}

	if (this->myOrganism->age % 5 == 0 || this->myOrganism->age % 6 == 0 /* || this->myOrganism->age % 6 == 0 || this->myOrganism->age % 7 == 0*/)
	{
		int energyGained = 1;
		// each bark in a plant adds a 2% chance for every leaf to generate an extra energy every tick
		if (randPercent(2 * this->myOrganism->cellCounts[cell_bark]))
		{
			energyGained++;
		}
		this->myOrganism->AddEnergy(energyGained);
	}
}

Cell_Leaf *Cell_Leaf::Clone()
{
	return new Cell_Leaf(*this);
}

// bark cell
Cell_Bark::~Cell_Bark()
{
}

Cell_Bark::Cell_Bark()
{
	this->type = cell_bark;
	this->myOrganism = nullptr;
	this->leafGrowCooldown = BARK_GROW_COOLDOWN;
}

Cell_Bark::Cell_Bark(Organism *_myOrganism)
{
	this->type = cell_bark;
	this->myOrganism = _myOrganism;
	this->leafGrowCooldown = BARK_GROW_COOLDOWN;
}

void Cell_Bark::Tick()
{
	if (this->leafGrowCooldown > 0)
	{
		this->leafGrowCooldown--;
		return;
	}
	else
	{
		if (this->myOrganism->GetEnergy() > BARK_GROW_COST)
		{
			int checkDirIndex = randInt(0, 3);
			for (int i = 0; i < 4; i++)
			{
				int *thisDirection = directions[(checkDirIndex + i) % 4];
				int x_abs = this->x + thisDirection[0];
				int y_abs = this->y + thisDirection[1];
				if (board.isCellOfType(x_abs, y_abs, cell_empty))
				{
					this->myOrganism->AddCell(x_abs - this->x, y_abs - this->y, new Cell_Leaf());
					this->myOrganism->ExpendEnergy(BARK_GROW_COST);
					this->leafGrowCooldown = BARK_GROW_COOLDOWN;
					return;
				}
			}
		}
	}
}

Cell_Bark *Cell_Bark::Clone()
{
	return new Cell_Bark(*this);
}

// flower cell
Cell_Flower::~Cell_Flower()
{
}

Cell_Flower::Cell_Flower()
{
	this->type = cell_flower;
	this->myLeaf = nullptr;
	this->myOrganism = nullptr;
	this->bloomCooldown = FLOWER_BLOOM_COOLDOWN;
}

Cell_Flower::Cell_Flower(Cell_Leaf *_myLeaf)
{
	this->type = cell_flower;
	this->myLeaf = _myLeaf;
	this->myOrganism = nullptr;
	this->bloomCooldown = FLOWER_BLOOM_COOLDOWN;
}

/*
Cell_Flower::Cell_Flower(Organism *_myOrganism)
{
	this->type = cell_flower;
	this->myLeaf = nullptr;
	this->myOrganism = _myOrganism;
	this->bloomCooldown = FLOWER_BLOOM_COOLDOWN;
}
*/

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
				if (this->myLeaf != nullptr)
				{
					this->myLeaf->myFlower = nullptr;
				}
				// new leaf cell should be able to flower too
				this->myOrganism->ReplaceCell(this, new Cell_Leaf(100));
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
	int gainedEnergy = 0;
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
					gainedEnergy = LEAF_FOOD_ENERGY;
					break;

				case cell_flower:
					gainedEnergy = FLOWER_FOOD_ENERGY;
					break;

				case cell_fruit:
					gainedEnergy = FRUIT_FOOD_ENERGY;
					break;

				case cell_plantmass:
					gainedEnergy = PLANTMASS_FOOD_ENERGY;
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
		this->myOrganism->AddEnergy(gainedEnergy);
		this->digestCooldown = ceil((gainedEnergy * gainedEnergy) * HERB_DIGEST_TIME_MULTIPLIER);
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
	this->digestCooldown = 0;
}

Cell_Carnivore::Cell_Carnivore(Organism *_myOrganism)
{
	this->type = cell_carnivore_mouth;
	this->myOrganism = _myOrganism;
	this->digestCooldown = 0;
}

void Cell_Carnivore::Tick()
{
	if (this->digestCooldown > 0)
	{
		this->digestCooldown--;
		return;
	}
	bool couldEat = false;
	int gainedEnergy = 0;
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
			gainedEnergy = BIOMASS_FOOD_ENERGY;
			// this->myOrganism->AddEnergy(BIOMASS_FOOD_ENERGY);
			board.replaceCell(potentiallyEaten, new Cell_Empty());
			couldEat = true;
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

	if (couldEat)
	{
		this->myOrganism->brain.Reward();
		this->myOrganism->AddEnergy(gainedEnergy);
		this->digestCooldown = ceil((gainedEnergy * gainedEnergy) * CARN_DIGEST_TIME_MULTIPLIER);
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
