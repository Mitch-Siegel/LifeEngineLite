#include "board.h"
#include "lifeforms.h"
#include "rng.h"


#include <SDL2/SDL.h>
#include <iostream>
#include <algorithm>
#include <chrono>

int directions[8][2] = {{1, 0}, {0, -1}, {-1, 0}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
extern Board board;

Board::Board(const int _dim_x, const int _dim_y)
{
	this->tickCount = 0;
	this->dim_x = _dim_x;
	this->dim_y = _dim_y;
	this->Organisms = std::vector<Organism *>();

	for (int y = 0; y < _dim_y; y++)
	{
		this->cells.push_back(std::vector<Cell *>());
		this->DeltaCells.push_back(std::vector<bool>());
		for (int x = 0; x < _dim_x; x++)
		{
			this->cells[y].push_back(new Cell_Empty());
			this->DeltaCells[y].push_back(false);
			this->cells[y][x]->x = x;
			this->cells[y][x]->y = y;
		}
	}
}

Board::~Board()
{
	for (int y = 0; y < this->dim_y; y++)
	{
		for (int x = 0; x < this->dim_x; x++)
		{
			delete this->cells[y][x];
		}
	}
}

void Board::Tick()
{
	this->tickCount++;
	for (size_t i = 0; i < this->FoodCells.size(); i++)
	{
		this->FoodCells[i]->Tick();
		switch (this->FoodCells[i]->type)
		{
		case cell_biomass:
			if (((Cell_Biomass *)this->FoodCells[i])->ticksUntilSpoil == 0)
			{
				board.replaceCell(this->FoodCells[i], new Cell_Empty());
				i--;
			}
			break;

		case cell_plantmass:
			if (((Cell_Plantmass *)this->FoodCells[i])->ticksUntilSpoil == 0)
			{
				board.replaceCell(this->FoodCells[i], new Cell_Empty());
				i--;
			}
			break;

		case cell_fruit:
			if (((Cell_Fruit *)this->FoodCells[i])->ticksUntilSpoil == 0)
			{
				// if we roll grow percent, create a new random organism
				Organism *grownFruit = this->createOrganism(this->FoodCells[i]->x, this->FoodCells[i]->y);
				if (randPercent(FRUIT_GROW_PERCENT) && randPercent(FRUIT_GROW_PERCENT))
				{
					grownFruit->mutability = ((Cell_Fruit *)this->FoodCells[i])->parentMutability;
					board.replaceCell(this->FoodCells[i], new Cell_Empty());
					grownFruit->AddCell(0, 0, GenerateRandomCell());
					Cell *secondRandomCell = GenerateRandomCell();
					bool couldAddSecond = false;
					int dirIndex = randInt(0, 7);
					for (int j = 0; j < 8; j++)
					{
						int *thisDirection = directions[(j + dirIndex) % 8];
						if (board.isCellOfType(grownFruit->x + thisDirection[0], grownFruit->y + thisDirection[1], cell_empty))
						{
							grownFruit->AddCell(thisDirection[0], thisDirection[1], secondRandomCell);
							couldAddSecond = true;
							break;
						}
					}
					if (!couldAddSecond)
					{
						delete secondRandomCell;
					}

					grownFruit->RecalculateStats();
					grownFruit->lifespan = grownFruit->myCells.size() * LIFESPAN_MULTIPLIER;
					grownFruit->AddEnergy(randInt(grownFruit->GetMaxEnergy() / 2, grownFruit->GetMaxEnergy()));
					grownFruit->Heal(grownFruit->GetMaxHealth());

					int newReproductioncooldown = (grownFruit->myCells.size()) * REPRODUCTION_COOLDOWN_MULTIPLIER;
					grownFruit->reproductionCooldown = newReproductioncooldown + randInt(0, newReproductioncooldown);
				}
				else
				{
					board.replaceCellAt(this->FoodCells[i]->x, this->FoodCells[i]->y, new Cell_Plantmass(FRUIT_SPOIL_TIME));
				}
				i--;
			}
			break;

		default:
			std::cerr << "Impossible case for food cell to be something it shouldn't!" << std::endl;
			exit(1);
		}
	}

	for (size_t i = 0; i < this->Organisms.size(); i++)
	{
		if (!this->Organisms[i]->alive)
		{
			Organism *toRemove = this->Organisms[i];
			this->Organisms.erase(this->Organisms.begin() + i);
			delete toRemove;
			i--;
		}
		else
		{

			// printf("%d %d: %lu/%lu energy, %lu cells (%lu ticks old, %d lifespan), repcd %d\n",
			//  Organisms[i]->x, Organisms[i]->y, Organisms[i]->GetEnergy(), Organisms[i]->GetMaxEnergy(), Organisms[i]->myCells.size(), Organisms[i]->age, Organisms[i]->lifespan, Organisms[i]->reproductionCooldown);
			Organism *replicated = this->Organisms[i]->Tick();

			if (replicated != nullptr)
			{
				Organisms.push_back(replicated);
			}
		}
	}
}

void Board::Stats()
{
	static auto lastFrame = std::chrono::high_resolution_clock::now();

	enum Counts
	{
		count_cells,
		count_energy,
		count_maxenergy,
		count_age,
		count_lifespan,
		count_mutability,
		count_maxconviction,
		count_rotatevschange,
		count_turnwhenrotate,
		count_raw,
		count_null
	};

	double plantStats[count_null] = {0.0};
	double moverStats[count_null] = {0.0};
	double plantCellCounts[cell_null] = {0.0};
	double moverCellCounts[cell_null] = {0.0};
	double moverCellSentiments[cell_null] = {0.0};
	size_t touchSensorHaverCount = 0;
	double touchSensorInterval = 0;
	auto now = std::chrono::high_resolution_clock::now();
	auto diff = now - lastFrame;
	size_t millis = std::chrono::duration_cast<std::chrono::microseconds>(diff).count() / 1000;
	printf("\nTICK %lu (%.2f t/s) (%lu organisms)\n", this->tickCount, (28000.0 / (float)millis), this->Organisms.size());
	lastFrame = now;
	for (Organism *o : this->Organisms)
	{
		if (o->cellCounts[cell_mover] && !o->cellCounts[cell_leaf])
		{
			moverStats[count_cells] += o->myCells.size();
			moverStats[count_energy] += o->GetEnergy();
			moverStats[count_maxenergy] += o->GetMaxEnergy();
			moverStats[count_age] += o->age;
			moverStats[count_lifespan] += o->lifespan;
			moverStats[count_mutability] += o->mutability;
			moverStats[count_maxconviction] += o->brain.maxConviction;
			moverStats[count_rotatevschange] += o->brain.rotatevschange;
			moverStats[count_turnwhenrotate] += o->brain.turnwhenrotate;
			moverStats[count_raw]++;
			for (int i = 0; i < cell_null; i++)
			{
				moverCellCounts[i] += o->cellCounts[i];
			}
			if (o->cellCounts[cell_touch] > 0)
			{
				touchSensorHaverCount++;
				for (int i = 0; i < cell_null; i++)
				{
					moverCellSentiments[i] += o->brain.cellSentiments[i];
				}
				for (Cell *c : o->myCells)
				{
					if (c->type == cell_touch)
					{
						Cell_Touch *t = (Cell_Touch *)c;
						touchSensorInterval += t->senseCooldown;
					}
				}
			}
		}
		else
		{
			plantStats[count_cells] += o->myCells.size();
			plantStats[count_energy] += o->GetEnergy();
			plantStats[count_maxenergy] += o->GetMaxEnergy();
			plantStats[count_age] += o->age;
			plantStats[count_lifespan] += o->lifespan;
			plantStats[count_mutability] += o->mutability;
			plantStats[count_maxconviction] += o->brain.maxConviction;
			plantStats[count_rotatevschange] += o->brain.rotatevschange;
			plantStats[count_turnwhenrotate] += o->brain.turnwhenrotate;
			plantStats[count_raw]++;
			for (int i = 0; i < cell_null; i++)
			{
				plantCellCounts[i] += o->cellCounts[i];
			}
		}
	}

	for (int i = 0; i < count_raw; i++)
	{
		plantStats[i] /= plantStats[count_raw];
		moverStats[i] /= moverStats[count_raw];
	}

	for (int i = 0; i < cell_null; i++)
	{
		plantCellCounts[i] /= plantStats[count_raw];
		moverCellCounts[i] /= moverStats[count_raw];
	}
	if (touchSensorHaverCount > 0)
	{
		for (int i = 0; i < cell_null; i++)
		{
			moverCellSentiments[i] /= touchSensorHaverCount;
		}
		touchSensorInterval /= touchSensorHaverCount;
	}

	printf("%5.0f Plants - avg %2.2f cells, %2.0f%% (%4.2f) energy, %.0f/%.0f lifespan, %.1f%% mutability\n",
		   plantStats[count_raw],
		   plantStats[count_cells],
		   plantStats[count_energy] / plantStats[count_maxenergy] * 100,
		   (plantStats[count_energy] / plantStats[count_maxenergy]) * plantStats[count_energy],
		   plantStats[count_age],
		   plantStats[count_lifespan],
		   plantStats[count_mutability]);

	printf("%5.0f Movers - avg %2.2f cells, %2.0f%% (%4.2f) energy, %.0f/%.0f lifespan, %.1f%% mutability\n\t%.3f max conviction, %.1f rotate vs change, %.1f turn when rotate\n",
		   moverStats[count_raw],
		   moverStats[count_cells],
		   moverStats[count_energy] / moverStats[count_maxenergy] * 100,
		   (moverStats[count_energy] / moverStats[count_maxenergy]) * moverStats[count_energy],
		   moverStats[count_age],
		   moverStats[count_lifespan],
		   moverStats[count_mutability],
		   moverStats[count_maxconviction],
		   moverStats[count_rotatevschange],
		   moverStats[count_turnwhenrotate]);
	printf("Plants/Movers ratio: %2.2f/1\n", plantStats[count_raw] / moverStats[count_raw]);

	printf("%lu (%.2f%%) movers have touch sensors (avg sense interval %2.2f)\n", touchSensorHaverCount, 100 * (float)touchSensorHaverCount / moverStats[count_raw], touchSensorInterval);
	char cellShortNames[cell_null][5] = {"EMPT", "PMAS", "BMAS", "LEAF", "BARK", "FLWR", "FRUT", "HERB", "CARN", "MOVR", "KILR", "ARMR", "TUCH"};
	printf("CELL:APLNTC|AMOVRC|ASSENT\n");
	for (int i = cell_empty; i < cell_null; i++)
	{
		printf("%4s: %2.2f | %2.2f | %02.2f\n",
			   cellShortNames[i],
			   plantCellCounts[i],
			   moverCellCounts[i],
			   moverCellSentiments[i]);
	}
}

// returns true if out of bounds, false otherwise
bool Board::boundCheckPos(int x, int y)
{
	if (0 > x || 0 > y || x >= this->dim_x || y >= this->dim_y)
	{
		printf("out of bounds (dim is %d, %d)\n", this->dim_x, this->dim_y);
		return true;
	}
	return false;
}

// returns true if a cell at given position is of the provided type
bool Board::isCellOfType(int x, int y, enum CellTypes type)
{
	if (this->boundCheckPos(x, y))
	{
		return false;
	}

	return this->cells[y][x]->type == type;
}

// replace the cell at a given position with another cell
// automatically handles adding and removing from food list
void Board::replaceCellAt(const int _x, const int _y, Cell *_cell)
{
	// if out of bounds, bail
	if (this->boundCheckPos(_x, _y))
	{
		std::cerr << "Replacing cell at out-of-bounds position!";
		exit(1);
	}

	switch (this->cells[_y][_x]->type)
	{
	case cell_plantmass:
	case cell_biomass:
	case cell_fruit:
		this->FoodCells.erase(std::find(this->FoodCells.begin(), this->FoodCells.end(), this->cells[_y][_x]));
		break;

	default:
		break;
	}
	delete this->cells[_y][_x];

	_cell->x = _x;
	_cell->y = _y;
	switch (_cell->type)
	{
	case cell_plantmass:
	case cell_biomass:
	case cell_fruit:
		this->FoodCells.push_back(_cell);
		break;

	default:
		break;
	}
	this->cells[_y][_x] = _cell;
	this->DeltaCells[_y][_x] = true;
}

void Board::replaceCell(Cell *_replaced, Cell *_newCell)
{
	this->replaceCellAt(_replaced->x, _replaced->y, _newCell);
}

void Board::swapCellAtIndex(int _x, int _y, Cell *a)
{
	// if out of bounds, bail
	if (this->boundCheckPos(_x, _y))
	{
		std::cerr << "Swapping cell at out-of-bounds position!";
		exit(1);
	}
	Cell *b = this->cells[_y][_x];
	int a_oldx = a->x;
	int a_oldy = a->y;
	b->x = a->x;
	b->y = a->y;
	this->cells[_y][_x] = a;
	this->cells[a_oldy][a_oldx] = b;
	a->x = _x;
	a->y = _y;

	this->DeltaCells[_y][_x] = true;
	this->DeltaCells[a_oldy][a_oldx] = true;
}

Organism *Board::createOrganism(const int _x, const int _y)
{
	Organism *newOrganism = new Organism(_x, _y);
	this->Organisms.push_back(newOrganism);
	return newOrganism;
}

