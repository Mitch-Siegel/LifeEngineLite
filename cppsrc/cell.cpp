#include "cell.h"

extern Cell** board;

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

Cell::Tick()
{
    switch(this->type)
    {
        case cell_empty:
            break;

        case cell_mouth:
            break;

        case cell_producer;

    }
}
