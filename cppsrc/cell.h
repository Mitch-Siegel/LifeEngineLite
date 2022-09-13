#pragma once
enum CellTypes
{
	cell_empty,
	cell_food,
	cell_producer,
	cell_mouth,	

};

// forward-declare to avoid include nastiness
class Organism;

class Cell
{
	public:
	enum CellTypes type;
	Organism *myOrganism;
	int x, y;

	Cell();

	Cell(int x, int y, enum CellTypes type, Organism *myOrganism);

	void Tick();
};





