#include "cell.h"

Cell::Cell()
{

}

Cell::Cell(int x, int y, enum CellTypes type, Organism *myOrganism)
{
	this->x = x;
	this->y = y;
	this->type = type;
	this->myOrganism = myOrganism;
}
