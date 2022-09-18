#include "brain.h"
#include "rng.h"

Brain::Brain()
{
    this->currentIntent = intent_changeDir;
    this->moveDirIndex = -1;
}

void Brain::Decide()
{
    switch(this->currentIntent)
    {
        case intent_continue:
            break;

        case intent_changeDir:
            this->moveDirIndex = randInt(0, 3);
            break;

        case intent_rotate:
            break;
    }
}
