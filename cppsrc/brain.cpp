#include "brain.h"
#include "rng.h"

Brain::Brain()
{
    this->currentIntent = intent_changeDir;
    this->moveDirIndex = -1;
    this->conviction = 0;
    this->maxConviction = 2;
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

enum Intent Brain::Decide()
{
    if ((randInt(0, 2 * maxConviction) - maxConviction) > conviction || randPercent(10))
    {
        if (randPercent(50))
        {
            this->currentIntent = intent_changeDir;
        }
        else
        {
            if (randPercent(50))
            {
                this->currentIntent = intent_rotateClockwise;
            }
            else
            {
                this->currentIntent = intent_rotateCounterClockwise;
            }
        }
    }
    switch (this->currentIntent)
    {
    case intent_continue:
        return intent_continue;

    case intent_changeDir:
        this->moveDirIndex = randInt(0, 3999) % 4;
        this->currentIntent = intent_continue;
        this->conviction = 1;
        return intent_continue;

    case intent_rotateClockwise:
        this->currentIntent = intent_continue;
        ++this->moveDirIndex %= 4;
        this->conviction = 1;
        return intent_rotateClockwise;

    case intent_rotateCounterClockwise:
        this->currentIntent = intent_continue;
        this->conviction = 1;
        if(--this->moveDirIndex < 0)
        {
            this->moveDirIndex = 3;
        }
        return intent_rotateCounterClockwise;
    }

    // really, G++ can't figure out that this is unreachable?!
    return this->currentIntent;
}

void Brain::Mutate()
{
    this->maxConviction += (randInt(-1, 1));
    if (this->maxConviction < 1)
    {
        this->maxConviction = 1;
    }
}

Brain Brain::Clone()
{
    return Brain(*this);
}
