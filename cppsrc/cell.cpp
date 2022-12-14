#include <vector>

#include "lifeforms.h"
#include "board.h"
#include "rng.h"
#include "util.h"

extern Board *board;
int CellEnergyDensities[cell_null] = {
	0,	// empty
	0,	// pcell_bark
	0,	// biomass
	1,	// leaf
	8,	// bark
	2,	// flower
	0,	// fruit
	24, // herbivore
	60, // carnivore
	30, // mover
	0,	// killer
	-5, // armor
	1,	// touch sensor
	1,	// eye
};

/*
int CellEnergyDensities[cell_null] = {
	0,	 // empty
	0,	 // pcell_bark
	0,	 // biomass
	1,	 // leaf
	8,	 // bark
	2,	 // flower
	0,	 // fruit
	40,	 // herbivore
	100, // carnivore
	50,	 // mover
	0,	 // killer
	-5,	 // armor
	5,	 // touch sensor
	5,	 // eye
};*/

Cell *GenerateRandomCell()
{
	switch (randInt(0, 8))
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

	case 7:
		return new Cell_Touch();
		break;

	case 8:
		return new Cell_Eye();
		break;
	}

	return nullptr;
}

Cell::~Cell()
{
}

const int &Spoilable_Cell::TicksUntilSpoil()
{
	if (this->ticksUntilSpoil_ == nullptr)
	{
		return this->startingTicksUntilSpoil;
	}
	else
	{
		return *this->ticksUntilSpoil_;
	}
}

void Spoilable_Cell::attachTicksUntilSpoil(int *slotValue)
{
	this->ticksUntilSpoil_ = slotValue;
}

Spoilable_Cell::Spoilable_Cell(int _startingTicksUntilSpoil)
{
	this->ticksUntilSpoil_ = nullptr;
	this->startingTicksUntilSpoil = _startingTicksUntilSpoil;
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

Cell_Plantmass::Cell_Plantmass() : Spoilable_Cell(Settings.Get(WorldSettings::plantmass_spoil_time))
{
	this->type = cell_plantmass;
	this->myOrganism = nullptr;
}

Cell_Plantmass::Cell_Plantmass(int _ticksUntilSpoil) : Spoilable_Cell(_ticksUntilSpoil)
{
	this->type = cell_plantmass;
	this->myOrganism = nullptr;
}

void Cell_Plantmass::Tick()
{
}

Cell_Plantmass *Cell_Plantmass::Clone()
{
	return new Cell_Plantmass(*this);
}

// biomass
Cell_Biomass::~Cell_Biomass()
{
}

Cell_Biomass::Cell_Biomass() : Spoilable_Cell(Settings.Get(WorldSettings::biomass_spoil_time))
{
	this->type = cell_biomass;
	this->myOrganism = nullptr;
}

Cell_Biomass::Cell_Biomass(int _ticksUntilSpoil) : Spoilable_Cell(_ticksUntilSpoil)
{
	this->type = cell_biomass;
	this->myOrganism = nullptr;
}

void Cell_Biomass::Tick()
{
}

Cell_Biomass *Cell_Biomass::Clone()
{
	return new Cell_Biomass(*this);
}

// leaf cell
Cell_Leaf::~Cell_Leaf()
{
	if (this->associatedFlower != nullptr)
	{
		this->associatedFlower->associatedLeaf = nullptr;
	}
}

Cell_Leaf::Cell_Leaf()
{
	this->type = cell_leaf;
	this->myOrganism = nullptr;
	this->flowering = randPercent(Settings.Get(WorldSettings::leaf_flowering_ability_percent));
	this->flowerCooldown = Settings.Get(WorldSettings::leaf_flowering_cooldown);
	this->associatedFlower = nullptr;
}

Cell_Leaf::Cell_Leaf(int floweringPercent)
{
	this->type = cell_leaf;
	this->myOrganism = nullptr;
	this->flowering = randPercent(floweringPercent);
	this->flowerCooldown = Settings.Get(WorldSettings::leaf_flowering_cooldown);
	this->associatedFlower = nullptr;
}

void Cell_Leaf::Tick()
{
	if (!this->flowering || (this->myOrganism->Energy() < (Settings.Get(WorldSettings::leaf_flowering_cost) + 1)))
	{
		return;
	}

	if (this->flowerCooldown > 0)
	{
		this->flowerCooldown--;
	}
	// can flower
	else
	{
		if (this->associatedFlower == nullptr)
		{
			int checkDirIndex = randInt(0, 3);
			for (int i = 0; i < 4; i++)
			{
				int *thisDirection = directions[(checkDirIndex + i) % 4];
				int x_abs = this->x + thisDirection[0];
				int y_abs = this->y + thisDirection[1];
				if (board->isCellOfType(x_abs, y_abs, cell_empty))
				{
					this->myOrganism->ExpendEnergy(Settings.Get(WorldSettings::leaf_flowering_cost));
					Cell_Flower *grownFlower = new Cell_Flower(this);
					this->associatedFlower = grownFlower;
					this->myOrganism->AddCell(x_abs - this->myOrganism->x, y_abs - this->myOrganism->y, grownFlower);
					this->flowerCooldown = Settings.Get(WorldSettings::leaf_flowering_cooldown);
					return;
				}
			}
		}
	}
}

const bool &Cell_Leaf::CanFlower()
{
	return this->flowering;
}

Cell_Leaf *Cell_Leaf::Clone()
{
	Cell_Leaf *cloned = new Cell_Leaf(*this);
	cloned->associatedFlower = nullptr;
	return cloned;
}

// bark cell
Cell_Bark::~Cell_Bark()
{
}

Cell_Bark::Cell_Bark()
{
	this->type = cell_bark;
	this->myOrganism = nullptr;
	this->actionCooldown = Settings.Get(WorldSettings::bark_grow_cooldown);
	this->integrity = Settings.Get(WorldSettings::bark_max_integrity);
}

void Cell_Bark::Tick()
{
	if (this->actionCooldown > 0)
	{
		this->actionCooldown--;
	}

	if (this->integrity < 1)
	{
		this->myOrganism->RemoveCell(this);
		board->replaceCell(this, new Cell_Empty());
		return;
	}

	int bonusEnergy = 0;
	bool canGrow = this->actionCooldown == 0;
	// even though leaves can exist in a 5x5 around bark, bark will only grow a leaf directly adjacent
	int checkDirIndex = randInt(0, 3);
	for (int i = 0; i < 4; i++)
	{
		int *thisDirection = directions[(checkDirIndex + i) % 4];
		int x_abs = this->x + thisDirection[0];
		int y_abs = this->y + thisDirection[1];
		if (canGrow && board->isCellOfType(x_abs, y_abs, cell_empty) &&
			(this->myOrganism->Energy() > (Settings.Get(WorldSettings::bark_grow_cost) + 1)))
		{
			// high chance to grow plant vs thorn
			if (randPercent(Settings.Get(WorldSettings::bark_plant_vs_thorn)))
			{
				// high chance to grow leaf vs bark
				if (randPercent(Settings.Get(WorldSettings::bark_plant_vs_thorn)))
				{
					this->myOrganism->AddCell(x_abs - this->myOrganism->x, y_abs - this->myOrganism->y, new Cell_Leaf());
				}
				else
				{
					this->myOrganism->AddCell(x_abs - this->myOrganism->x, y_abs - this->myOrganism->y, new Cell_Bark());
				}
			}
			else
			{
				this->myOrganism->AddCell(x_abs - this->myOrganism->x, y_abs - this->myOrganism->y, new Cell_Killer());
			}
			this->myOrganism->ExpendEnergy(Settings.Get(WorldSettings::bark_grow_cost));
			this->actionCooldown = Settings.Get(WorldSettings::bark_grow_cooldown);
			canGrow = false;
		}
		else if (board->isCellOfType(x_abs, y_abs, cell_leaf) && board->cells[y_abs][x_abs]->myOrganism == this->myOrganism)
		{
			bonusEnergy++;
		}
	}
	// any leaves attached to bark generate bonus energy
	// this->myOrganism->AddEnergy((static_cast<double>(bonusEnergy) * PHOTOSYNTHESIS_ENERGY_MULTIPLIER) / BARK_BONUS_ENERGY_DIVIDER);
}

Cell_Bark *Cell_Bark::Clone()
{
	Cell_Bark *cloned = new Cell_Bark(*this);
	cloned->actionCooldown = Settings.Get(WorldSettings::bark_grow_cooldown);
	cloned->integrity = Settings.Get(WorldSettings::bark_max_integrity);
	return cloned;
}

// flower cell
Cell_Flower::~Cell_Flower()
{
	if (this->associatedLeaf != nullptr)
	{
		this->associatedLeaf->associatedFlower = nullptr;
	}
}

Cell_Flower::Cell_Flower(Cell_Leaf *associatedLeaf)
{
	this->type = cell_flower;
	this->myOrganism = nullptr;
	this->bloomCooldown = Settings.Get(WorldSettings::flower_bloom_cooldown);
	this->associatedLeaf = associatedLeaf;
}

void Cell_Flower::Tick()
{
	if (this->bloomCooldown > 0)
	{
		this->bloomCooldown--;
		return;
	}
	else if ((this->myOrganism->reproductionCooldown > 0) || (this->myOrganism->Energy() < (Settings.Get(WorldSettings::flower_bloom_cost) + 1)))
	{
		return;
	}
	else
	{
		bool couldBloom = false;
		int checkDirIndex = randInt(0, 3);
		for (int i = 0; i < 4; i++)
		{
			int *thisDirection = directions[(checkDirIndex + i) % 4];
			int x_abs = this->x + thisDirection[0];
			int y_abs = this->y + thisDirection[1];
			if (board->isCellOfType(x_abs, y_abs, cell_empty))
			{
				board->replaceCellAt(x_abs, y_abs, new Cell_Fruit(this->myOrganism->mutability));
				couldBloom = true;
				break;
				// this->myOrganism->AddCell(x_rel, y_rel, new Cell_Fruit());
			}
		}
		// couldBloom = true;

		if (couldBloom)
		{
			this->myOrganism->ExpendEnergy(Settings.Get(WorldSettings::flower_bloom_cost));
			this->bloomCooldown = Settings.Get(WorldSettings::flower_bloom_cooldown);
			if (randPercent(Settings.Get(WorldSettings::flower_wilt_chance)))
			{
				// if (this->myOrganism->Energy() > (FLOWER_COST + 1) &&
				if (randPercent(Settings.Get(WorldSettings::flower_expand_percent)) /* && (this->myOrganism->Energy() > (FLOWER_COST + 1))*/)
				{
					this->myOrganism->ReplaceCell(this, new Cell_Leaf(100));
				}
				else
				{
					this->myOrganism->RemoveCell(this);
					board->replaceCell(this, new Cell_Empty());
				}
			}
		}
	}
}

Cell_Flower *Cell_Flower::Clone()
{
	printf("Illegal clone of cell_flower!\n");
	exit(1);
	// return new Cell_Flower(*this);
}

// fruit cell
Cell_Fruit::~Cell_Fruit()
{
}

Cell_Fruit::Cell_Fruit() : Spoilable_Cell(Settings.Get(WorldSettings::fruit_spoil_time))
{
	this->type = cell_fruit;
	this->myOrganism = nullptr;
	this->parentMutability = 50;
}

Cell_Fruit::Cell_Fruit(int _parentMutability) : Spoilable_Cell(Settings.Get(WorldSettings::fruit_spoil_time))
{
	this->type = cell_fruit;
	this->myOrganism = nullptr;
	this->parentMutability = _parentMutability;
}

void Cell_Fruit::Tick()
{
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
	this->digestCooldown = 0;
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

	int checkDirIndex = randInt(0, 3);
	for (int i = 0; i < 4 && !couldEat; i++)
	{
		int *thisDirection = directions[(checkDirIndex + i) % 4];
		int x_abs = this->x + thisDirection[0];
		int y_abs = this->y + thisDirection[1];
		if (board->boundCheckPos(x_abs, y_abs))
		{
			continue;
		}
		Cell *potentiallyEaten = board->cells[y_abs][x_abs];
		if (potentiallyEaten->type == cell_leaf ||
			potentiallyEaten->type == cell_flower ||
			potentiallyEaten->type == cell_fruit ||
			potentiallyEaten->type == cell_plantmass ||
			potentiallyEaten->type == cell_bark)
		{
			Organism *eatenParent = potentiallyEaten->myOrganism;
			bool removeEaten = true;
			if (eatenParent != this->myOrganism)
			{
				switch (potentiallyEaten->type)
				{
				case cell_leaf:
					gainedEnergy = Settings.Get(WorldSettings::leaf_food_energy);
					this->digestCooldown = 0;
					break;

				case cell_flower:
					gainedEnergy = Settings.Get(WorldSettings::flower_food_energy);
					this->digestCooldown = 1;
					break;

				case cell_fruit:
					gainedEnergy = Settings.Get(WorldSettings::fruit_food_energy);
					this->digestCooldown = 2;
					break;

				case cell_plantmass:
					gainedEnergy = Settings.Get(WorldSettings::plantmass_food_energy);
					this->digestCooldown = 0;
					break;

				case cell_bark:
				{
					gainedEnergy = 0;
					Cell_Bark *chompedBark = static_cast<Cell_Bark *>(potentiallyEaten);
					if (--chompedBark->integrity > 0)
					{
						removeEaten = false;
					}
					this->digestCooldown = 1;
				}
				break;

				default:
					std::cerr << "Impossible case for herbivore cell to eat something it shouldn't!" << std::endl;
					exit(1);
				}
				if (removeEaten)
				{
					if (potentiallyEaten->myOrganism != nullptr)
					{
						potentiallyEaten->myOrganism->RemoveCell(potentiallyEaten);
						board->replaceCell(potentiallyEaten, new Cell_Empty());
					}
					else
					{
						board->replaceCell(potentiallyEaten, new Cell_Empty());
					}
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

	// if eating something other than a leaf, take time to digest it
	if (couldEat)
	{
		this->myOrganism->AddEnergy(gainedEnergy);
	}

	if (!valid)
	{
		for (int i = 0; i < 8; i++)
		{
			int *thisDirection = directions[i];
			int abs_x = this->x + thisDirection[0];
			int abs_y = this->y + thisDirection[1];
			valid = valid || (!board->boundCheckPos(abs_x, abs_y) && board->cells[abs_y][abs_x]->myOrganism == this->myOrganism);
		}
	}

	if (!valid)
	{
		this->myOrganism->RemoveCell(this);
		board->replaceCell(this, new Cell_Empty());
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

	int checkDirIndex = randInt(0, 3);
	for (int i = 0; i < 4; i++)
	{
		int *thisDirection = directions[(checkDirIndex + i) % 4];
		int x_abs = this->x + thisDirection[0];
		int y_abs = this->y + thisDirection[1];
		if (board->boundCheckPos(x_abs, y_abs))
		{
			continue;
		}
		Cell *potentiallyEaten = board->cells[y_abs][x_abs];
		if (potentiallyEaten->type == cell_biomass)
		{
			gainedEnergy = Settings.Get(WorldSettings::biomass_food_energy);
			// this->myOrganism->AddEnergy(BIOMASS_FOOD_ENERGY);
			board->replaceCell(potentiallyEaten, new Cell_Empty());
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
			valid = valid || (!board->boundCheckPos(abs_x, abs_y) && board->cells[abs_y][abs_x]->myOrganism == this->myOrganism);
		}
	}

	if (couldEat)
	{
		this->myOrganism->AddEnergy(gainedEnergy);
		this->digestCooldown = 6;
	}

	if (!valid)
	{
		this->myOrganism->RemoveCell(this);
		board->replaceCell(this, new Cell_Empty());
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

void Cell_Killer::Tick()
{
	int damageDone = 0;
	for (int i = 0; i < 4; i++)
	{
		int *thisDirection = directions[i];
		int abs_x = this->x + thisDirection[0];
		int abs_y = this->y + thisDirection[1];
		if (!board->boundCheckPos(abs_x, abs_y))
		{
			Cell *adjacent = board->cells[abs_y][abs_x];
			if (adjacent->myOrganism != nullptr)
			{
				if (adjacent->myOrganism != this->myOrganism && adjacent->myOrganism->Identifier().Species() != this->myOrganism->Identifier().Species())
				{
					// still expend the energy to try and hit armor
					// but if directly contacting armor, do no damage
					if (adjacent->type != cell_armor)
					{
						damageDone++;
						adjacent->myOrganism->Damage(1);
					}
				}
			}
		}
	}
	// base cost plus some addl cost per damage done
	this->myOrganism->ExpendEnergy((damageDone * Settings.Get(WorldSettings::killer_damage_cost)) + Settings.Get(WorldSettings::killer_tick_cost));

	int adjacentLeaves = 0;
	int adjacentBark = 0;
	for (int i = 0; i < 4; i++)
	{
		int *thisDirection = directions[i];
		int abs_x = this->x + thisDirection[0];
		int abs_y = this->y + thisDirection[1];
		if (!board->boundCheckPos(abs_x, abs_y))
		{
			Cell *adjacentCell = board->cells[abs_y][abs_x];
			if (adjacentCell->myOrganism == this->myOrganism)
			{
				switch (adjacentCell->type)
				{
				case cell_leaf:
					adjacentLeaves++;
					break;

				case cell_bark:
					adjacentBark++;
					break;

				default:
					break;
				}
			}
		}
	}
	if (adjacentBark || (this->myOrganism->cellCounts[cell_leaf] < this->myOrganism->nCells() * 0.5))
	{
		if ((adjacentBark + adjacentLeaves > 2) || (adjacentBark >= 2))
		{
			this->myOrganism->ReplaceCell(this, new Cell_Bark());
		}
	}
	else
	{
		this->myOrganism->RemoveCell(this);
		board->replaceCell(this, new Cell_Empty());
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

void Cell_Armor::Tick()
{
	bool valid = false;
	this->myOrganism->ExpendEnergy(1);
	for (int i = 0; i < 8 && !valid; i++)
	{
		int *thisDirection = directions[i];
		int abs_x = this->x + thisDirection[0];
		int abs_y = this->y + thisDirection[1];
		valid = valid || (!board->boundCheckPos(abs_x, abs_y) && board->cells[abs_y][abs_x]->myOrganism == this->myOrganism);
	}
	if (!valid)
	{
		this->myOrganism->RemoveCell(this);
		board->replaceCell(this, new Cell_Empty());
	}
}

Cell_Armor *Cell_Armor::Clone()
{
	return new Cell_Armor(*this);
}

// touch sensor cell
Cell_Touch::~Cell_Touch()
{
}

Cell_Touch::Cell_Touch()
{
	this->type = cell_touch;
	this->myOrganism = nullptr;
}

void Cell_Touch::Tick()
{
	nn_num_t cellsTouched[cell_null] = {0.0};
	for (int i = 0; i < 4; i++)
	{
		int *thisDirection = directions[i];
		int x_abs = this->x + thisDirection[0];
		int y_abs = this->y + thisDirection[1];
		if (!board->boundCheckPos(x_abs, y_abs))
		{
			Cell *checked = board->cells[y_abs][x_abs];
			if (checked->myOrganism == this->myOrganism)
			{
				continue;
			}
			cellsTouched[checked->type] = 1.0;
		}
	}
	this->myOrganism->brain->SetSensoryInput(this->BrainInputIndex(), cellsTouched);
}

Cell_Touch *Cell_Touch::Clone()
{
	return new Cell_Touch(*this);
}

// eye cell
Cell_Eye::~Cell_Eye()
{
}

Cell_Eye::Cell_Eye()
{
	this->type = cell_eye;
	this->myOrganism = nullptr;
	this->direction = randInt(0, 3);
}

void Cell_Eye::Tick()
{
	nn_num_t cellsSeen[cell_null] = {0.0};
	int *deltaCoords = directions[this->direction];
	int x_checked = this->x;
	int y_checked = this->y;
	for (int i = 0; i < Settings.Get(WorldSettings::eye_max_seeing_distance); i++)
	{
		x_checked += deltaCoords[0];
		y_checked += deltaCoords[1];
		if (!board->boundCheckPos(x_checked, y_checked))
		{
			Cell *checked = board->cells[y_checked][x_checked];
			if (checked->type != cell_empty)
			{
				if (checked->myOrganism != this->myOrganism)
				{
					cellsSeen[checked->type] = static_cast<nn_num_t>(Settings.Get(WorldSettings::eye_max_seeing_distance) - i) / Settings.Get(WorldSettings::eye_max_seeing_distance);
					// cellsSeen[checked->type] = 1.0;
				}
				break;
			}
		}
		else
		{
			break;
		}
	}
	this->myOrganism->brain->SetSensoryInput(this->BrainInputIndex(), cellsSeen);
}

int Cell_Eye::Direction()
{
	return this->direction;
}

Cell_Eye *Cell_Eye::Clone()
{
	Cell_Eye *cloned = new Cell_Eye(*this);
	cloned->direction = this->direction;
	return cloned;
}
