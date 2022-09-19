#include "brain.h"
#include "rng.h"

Brain::Brain()
{
    this->currentIntent = intent_changeDir;
    this->moveDirIndex = -1;
    this->conviction = 0;
    this->maxConviction = 1;
}

void Brain::Reward()
{
    this->conviction++;
    if (this->conviction > maxConviction)
    {
        this->conviction = maxConviction;
    }
}

void Brain::Punish()
{
    this->conviction--;
    if (this->conviction < (-1 * maxConviction))
    {
        this->conviction = (-1 * maxConviction);
    }
}

void Brain::Decide()
{
    if((randInt(0,  2 * maxConviction) - maxConviction) > conviction || randPercent(10))
    {
        this->currentIntent = intent_changeDir;
    }
    switch (this->currentIntent)
    {
    case intent_continue:
        break;

    case intent_changeDir:
        this->moveDirIndex = randInt(0, 3);
        this->currentIntent = intent_continue;
        this->conviction = 0;
        break;

    case intent_rotate:
        break;
    }
}

void Brain::Mutate()
{
    this->maxConviction += (randInt(-1, 1));
    if(this->maxConviction < 1)
    {
        this->maxConviction = 1;
    }
}

Brain Brain::Clone()
{
    return Brain(*this);
}
