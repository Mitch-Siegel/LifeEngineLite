#include <vector>

#include "lifeforms.h"
#include "board.h"
#include "worldsettings.h"
#include "rng.h"
#include "util.h"

extern Board *board;
int CellEnergyDensities[cell_null] = {
	0,	// empty
	0,	// plantmass
	0,	// biomass
	1,	// leaf
	8,	// bark
	2,	// flower
	0,	// fruit
	16, // mouth
	16, // mover
	8,	// killer
	8,	// armor
	1,	// touch sensor
	1,	// eye
};

Cell *GenerateRandomCell()
{
	switch (randInt(0, 7))
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
		return new Cell_Mouth();
		break;

	case 4:
		return new Cell_Killer();
		break;

	case 5:
		return new Cell_Armor();
		break;

	case 6:
		return new Cell_Touch();
		break;

	case 7:
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
}

Cell_Leaf::Cell_Leaf()
{
	this->type = cell_leaf;
	this->myOrganism = nullptr;
	this->flowering = randPercent(Settings.Get(WorldSettings::leaf_flowering_ability_percent));
}

Cell_Leaf::Cell_Leaf(int floweringPercent)
{
	this->type = cell_leaf;
	this->myOrganism = nullptr;
	this->flowering = randPercent(floweringPercent);
	this->photosynthesisCooldown = Settings.Get(WorldSettings::photosynthesis_interval);
}

void Cell_Leaf::CalculatePhotosynthesieEffectiveness()
{
	this->crowding = 0;
	for (int i = 0; i < 8; i++)
	{
		int x_check = this->x + directions[i][0];
		int y_check = this->y + directions[i][1];
		if (!board->boundCheckPos(x_check, y_check))
		{
			Cell *neighbor = board->cells[y_check][x_check];
			if (neighbor->myOrganism == this->myOrganism)
			{
				switch (neighbor->type)
				{
				case cell_bark:
					this->crowding = 0;
					return;
					break;

					// case cell_leaf:
					// break;

				default:
					this->crowding++;
					break;
				}
			}
			else
			{
				switch (neighbor->type)
				{
				case cell_empty:
					break;

				case cell_plantmass:
				case cell_biomass:
				case cell_fruit:
					// this->crowding++;
					break;

				default:
					this->crowding++;
					break;
				}
			}
		}
	}
}

void Cell_Leaf::Tick()
{
	if (this->photosynthesisCooldown > 0)
	{
		this->photosynthesisCooldown--;
	}
	else
	{
		this->myOrganism->AddEnergy(1.0);
		this->photosynthesisCooldown = Settings.Get(WorldSettings::photosynthesis_interval) + this->crowding;
	}

	if (!this->flowering)
	{
		return;
	}

	if (this->myOrganism->Vitality() > 0)
	{
		{
			int checkDirIndex = randInt(0, 3);
			for (int i = 0; i < 4; i++)
			{
				int *thisDirection = directions[(checkDirIndex + i) % 4];
				int x_abs = this->x + thisDirection[0];
				int y_abs = this->y + thisDirection[1];
				if (board->isCellOfType(x_abs, y_abs, cell_empty))
				{

					this->myOrganism->ExpendVitality(1);
					this->myOrganism->AddCell(x_abs - this->myOrganism->x, y_abs - this->myOrganism->y, new Cell_Flower());
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
	this->actionCooldown = 0;
	this->integrity = Settings.Get(WorldSettings::bark_max_integrity);
}

void Cell_Bark::Tick()
{
	// tick cost applied in organism::tick()

	if (this->integrity < 1)
	{
		this->myOrganism->RemoveCell(this, true);
		board->replaceCell(this, new Cell_Empty());
		return;
	}

	/*
	if (this->myOrganism->Vitality() > 0)
	{
		// bark will only grow a leaf directly adjacent
		int checkDirIndex = randInt(0, 3);
		for (int i = 0; i < 4; i++)
		{
			int *thisDirection = directions[(checkDirIndex + i) % 4];
			int x_abs = this->x + thisDirection[0];
			int y_abs = this->y + thisDirection[1];
			if (board->isCellOfType(x_abs, y_abs, cell_empty))
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
				this->myOrganism->ExpendVitality(1);
				return;
			}
		}
	}
	*/
}

Cell_Bark *Cell_Bark::Clone()
{
	Cell_Bark *cloned = new Cell_Bark(*this);
	cloned->integrity = Settings.Get(WorldSettings::bark_max_integrity);
	return cloned;
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

void Cell_Flower::Tick()
{
	if (this->myOrganism->Vitality() > 0)
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
			this->myOrganism->ExpendVitality(1);
			if (randPercent(Settings.Get(WorldSettings::flower_wilt_chance)))
			{
				if (randPercent(Settings.Get(WorldSettings::flower_expand_percent)))
				{
					this->myOrganism->ReplaceCell(this, new Cell_Leaf(100));
				}
				else
				{
					this->myOrganism->RemoveCell(this, false);
					board->replaceCell(this, new Cell_Empty());
				}
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
Cell_Mouth::~Cell_Mouth()
{
}

Cell_Mouth::Cell_Mouth()
{
	this->type = cell_mouth;
	this->myOrganism = nullptr;
	this->digestCooldown = 0;
}

void Cell_Mouth::Tick()
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
			potentiallyEaten->type == cell_bark || 
			potentiallyEaten->type == cell_biomass)
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
					this->digestCooldown = 0;
					break;

				case cell_fruit:
					gainedEnergy = Settings.Get(WorldSettings::fruit_food_energy);
					this->digestCooldown = 1;
					break;

				case cell_plantmass:
					gainedEnergy = Settings.Get(WorldSettings::plantmass_food_energy);
					this->digestCooldown = 0;
					break;

				case cell_biomass:
					gainedEnergy = Settings.Get(WorldSettings::biomass_food_energy);
					this->digestCooldown = 4;
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
						potentiallyEaten->myOrganism->RemoveCell(potentiallyEaten, true);
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
		this->myOrganism->RemoveCell(this, true);
		board->replaceCell(this, new Cell_Empty());
	}
}

Cell_Mouth *Cell_Mouth::Clone()
{
	return new Cell_Mouth(*this);
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
	this->myOrganism->ExpendEnergy((damageDone * Settings.Get(WorldSettings::killer_damage_cost)) + (this->myOrganism->age % (Settings.Get(WorldSettings::killer_cost_interval) + 1) == 0));
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
	std::vector<nn_num_t> sense(cell_null);
	for (int i = 0; i < 4; i++)
	{
		int *thisDirection = directions[i];
		int x_abs = this->x + thisDirection[0];
		int y_abs = this->y + thisDirection[1];
		if (!board->boundCheckPos(x_abs, y_abs))
		{
			Cell *checked = board->cells[y_abs][x_abs];
			if (checked->myOrganism != this->myOrganism)
			{
				sense[checked->type] = 1.0;
			}
		}
	}
	this->myOrganism->brain->SetSensoryInput(this->BrainInputIndex(), sense);
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
	std::vector<nn_num_t> sense(cell_null);
	int *deltaCoords = directions[this->direction];
	int x_checked = this->x;
	int y_checked = this->y;
	uint64_t maxSeeingDistance = Settings.Get(WorldSettings::eye_max_seeing_distance);
	for (uint64_t i = 0; i < maxSeeingDistance; i++)
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
					sense[checked->type] = static_cast<nn_num_t>(Settings.Get(WorldSettings::eye_max_seeing_distance) - i) / Settings.Get(WorldSettings::eye_max_seeing_distance);
				}
				break;
			}
		}
		else
		{
			break;
		}
	}
	this->myOrganism->brain->SetSensoryInput(this->BrainInputIndex(), sense);
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
