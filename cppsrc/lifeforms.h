#include "config.h"
#include "brain.h"

#include <set>
#include <cstdint>

#pragma once

class OrganismIdentifier
{
private:
	uint32_t memberID_;
	uint32_t species_;

public:
	OrganismIdentifier()
	{
		this->species_ = 0;
		this->memberID_ = 0;
	}

	OrganismIdentifier(uint32_t species)
	{
		this->species_ = species;
		this->memberID_ = 0;
	}

	const uint32_t &Species() const { return this->species_; };

	const uint32_t &MemberID() const { return this->memberID_; };

	void SetMemberID(uint32_t newID) { this->memberID_ = newID; };

	bool operator<(const OrganismIdentifier &b) const
	{
		return ((static_cast<uint64_t>(this->species_) << 32) + this->memberID_) <
			   ((static_cast<uint64_t>(b.species_) << 32) + b.memberID_);
	}
};

class Cell;
class OrganismView;
class SDL_Texture;
class SDL_Renderer;

class Organism
{
	friend class Board;
	friend class OrganismView;

private:
	uint64_t currentHealth, maxHealth;
	uint64_t currentEnergy, maxEnergy;
	int64_t vitality_;
	std::set<Cell *> myCells;
	uint64_t nCells_;
	OrganismIdentifier identifier_;

	double fractionalEnergy = 0.0;

	bool requireConnectednessCheck;
	// remove any cells which aren't directly connected to the organism
	void VerifyCellConnectedness();

	// figure out what a killed cell should become based on the parameters of this organism
	void ReplaceKilledCell(Cell *replaced);

public:
	int x = -1;
	int y = -1;
	int direction;

	uint64_t age;
	int mutability;
	bool alive;
	uint64_t lifespan;
	Brain *brain;
	uint64_t cellCounts[cell_null];

	const uint64_t &nCells() const { return this->nCells_; }

	const int64_t &Vitality() const { return this->vitality_; }

	Organism(int center_x, int center_y);

	Organism(int center_x, int center_y, const Brain &baseBrain);

	~Organism();

	void Die();

	void Remove();

	Organism *Tick();

	void RecalculateStats();

	bool CheckValidity();

	void Move(int moveDirection);

	void Rotate(bool clockwise);

	void OnCellAdded(Cell *added);

	void OnCellRemoved(Cell *removed);

	void AddCell(int x_rel, int y_rel, Cell *_cell);

	void RemoveCell(Cell *_myCell, bool doVitalityLoss);

	void ReplaceCell(Cell *_myCell, Cell *_newCell);

	const uint64_t &Health();

	const uint64_t &MaxHealth();

	const uint64_t &Energy();

	const uint64_t &MaxEnergy();

	void Damage(uint64_t n);

	void Heal(uint64_t n);

	void ExpendEnergy(double n);

	void AddEnergy(uint64_t n);

	void ExpendVitality(uint32_t n);

	bool CanOccupyPosition(int _x_abs, int _y_abs);

	bool CanMoveToPosition(int _x_abs, int _y_abs);

	Organism *Reproduce();

	bool Mutate();

	enum OrganismClassifications Classify();

	const OrganismIdentifier &Identifier() { return this->identifier_; };

	SDL_Texture *OneShotRender(SDL_Renderer *r, SDL_Texture *inTex);
};

class Organism;
#define LIFESPAN(maxEnergy, nCells) (Settings.Get(WorldSettings::lifespan_multiplier) * maxEnergy * sqrt(nCells))

class Organism;

class Cell
{
public:
	Organism *myOrganism = nullptr;
	enum CellTypes type = cell_null;
	int x = -1;
	int y = -1;

	virtual ~Cell() = 0;

	virtual void Tick() = 0;

	virtual Cell *Clone() = 0;
};

Cell *GenerateRandomCell();

class Spoilable_Cell : public Cell
{
private:
	int *ticksUntilSpoil_;
	int startingTicksUntilSpoil;

public:
	const int &TicksUntilSpoil();

	void attachTicksUntilSpoil(int *slotValue);

	explicit Spoilable_Cell(int _startingTicksUntilSpoil);
};

class Cell_Empty : public Cell
{
public:
	~Cell_Empty() override;

	Cell_Empty();

	void Tick() override;

	Cell_Empty *Clone() override;
};

class Cell_Plantmass : public Spoilable_Cell
{
public:
	~Cell_Plantmass() override;

	Cell_Plantmass();

	explicit Cell_Plantmass(int _ticksUntilSpoil);

	void Tick() override;

	Cell_Plantmass *Clone() override;
};

class Cell_Biomass : public Spoilable_Cell
{
public:
	~Cell_Biomass() override;

	Cell_Biomass();

	explicit Cell_Biomass(int _ticksUntilSpoil);

	void Tick() override;

	Cell_Biomass *Clone() override;
};

class Cell_Flower;
class Cell_Leaf : public Cell
{
	friend class Cell_Herbivore;
	friend class Cell_Bark;
	friend class Cell_Flower;
	friend class Organism;
	friend class Board;

private:
	int photosynthesisCooldown;
	int flowerCooldown;
	bool flowering;
	int crowding;

public:
	~Cell_Leaf() override;

	Cell_Leaf();

	explicit Cell_Leaf(int floweringPercent);

	explicit Cell_Leaf(Organism *_myOrganism);

	// Cell_Leaf(const Cell_Leaf &c);

	void CalculatePhotosynthesieEffectiveness();

	void Tick() override;

	const bool &CanFlower();

	Cell_Leaf *Clone() override;
};

class Cell_Bark : public Cell
{
	friend class Organism;
	friend class Cell_Herbivore;
	int actionCooldown;
	int integrity;

public:
	~Cell_Bark() override;

	Cell_Bark();

	explicit Cell_Bark(Organism *_myOrganism);

	void Tick() override;

	Cell_Bark *Clone() override;
};

class Cell_Flower : public Cell
{
	friend class Cell_Leaf;
	friend class Board;
	int bloomCooldown;

public:
	~Cell_Flower() override;

	Cell_Flower();

	// explicit Cell_Flower(Organism *_myOrganism);

	void Tick() override;

	Cell_Flower *Clone() override;
};

class Cell_Fruit : public Spoilable_Cell
{
public:
	int parentMutability;

public:
	~Cell_Fruit() override;

	Cell_Fruit();

	explicit Cell_Fruit(int parentMutability);

	explicit Cell_Fruit(Organism *_myOrganism);

	void Tick() override;

	Cell_Fruit *Clone() override;
};

class Cell_Mover : public Cell
{
public:
	~Cell_Mover() override;

	Cell_Mover();

	explicit Cell_Mover(Organism *_myOrganism);

	void Tick() override;

	Cell_Mover *Clone() override;
};

class Cell_Herbivore : public Cell
{
	uint8_t digestCooldown;

public:
	~Cell_Herbivore() override;

	Cell_Herbivore();

	explicit Cell_Herbivore(Organism *_myOrganism);

	void Tick() override;

	Cell_Herbivore *Clone() override;
};

class Cell_Carnivore : public Cell
{
	uint8_t digestCooldown;

public:
	~Cell_Carnivore() override;

	Cell_Carnivore();

	explicit Cell_Carnivore(Organism *_myOrganism);

	void Tick() override;

	Cell_Carnivore *Clone() override;
};

class Cell_Killer : public Cell
{

public:
	~Cell_Killer() override;

	Cell_Killer();

	explicit Cell_Killer(Organism *_myOrganism);

	void Tick() override;

	Cell_Killer *Clone() override;
};

class Cell_Armor : public Cell
{

public:
	~Cell_Armor() override;

	Cell_Armor();

	explicit Cell_Armor(Organism *_myOrganism);

	void Tick() override;

	Cell_Armor *Clone() override;
};

class Sensor_Cell : public Cell
{
protected:
	int brainInputIndex_;

public:
	void SetBrainInputIndex(int index) { this->brainInputIndex_ = index; };

	const int &BrainInputIndex() { return this->brainInputIndex_; };

	Sensor_Cell() { this->brainInputIndex_ = -1; };

	Sensor_Cell(const Sensor_Cell &c)
	{
		*this = c;
		this->brainInputIndex_ = c.brainInputIndex_;
	}
};

class Cell_Touch : public Sensor_Cell
{
	friend class Organism;
	friend class Board;

public:
	~Cell_Touch() override;

	Cell_Touch();

	void Tick() override;

	Cell_Touch *Clone() override;
};

class Cell_Eye : public Sensor_Cell
{
	friend class Organism;
	friend class Board;

private:
	int direction;

public:
	~Cell_Eye() override;

	Cell_Eye();

	void Tick() override;

	int Direction();

	Cell_Eye *Clone() override;
};
