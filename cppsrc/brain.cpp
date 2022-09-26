#include "brain.h"
#include "rng.h"

Brain::Brain()
{
    this->currentIntent = intent_changeDir;
    this->moveDirIndex = -1;
    this->conviction = 0;
    this->maxConviction = 2;
    // chance to rotate instead of changing direction
    this->rotatevschange = 50;
}

void Brain::Reward()
{
    if (++this->conviction > maxConviction)
    {
        this->conviction = maxConviction;
    }
    this->justRewarded = true;
}

void Brain::Punish()
{
    if (--this->conviction < (-1 * maxConviction))
    {
        this->conviction = (-1 * maxConviction);
    }
}

enum Intent Brain::Decide()
{
    if(justRewarded)
    {
        justRewarded = false;
        return currentIntent;
    }
    if ((randInt(0, 2 * maxConviction) - maxConviction) > conviction || randPercent(1))
    {
        if (!randPercent(rotatevschange))
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
        this->conviction = ceil(this->maxConviction / 2.0);
        return intent_continue;

    case intent_rotateClockwise:
        this->currentIntent = intent_continue;
        this->conviction = ceil(this->maxConviction / 2.0);
        return intent_rotateClockwise;

    case intent_rotateCounterClockwise:
        this->currentIntent = intent_continue;
        this->conviction = ceil(this->maxConviction / 2.0);
        return intent_rotateCounterClockwise;
    }

    // really, G++ can't figure out that this is unreachable?!
    return this->currentIntent;
}

// if the call to rotate() succeeds, this is called
// make the organism's movement direction turn correspondingly
void Brain::RotateSuccess(bool clockwise)
{
    if (clockwise)
    {
        ++this->moveDirIndex %= 4;
    }
    else
    {
        if (--this->moveDirIndex < 0)
        {
            this->moveDirIndex = 3;
        }
    }
}

void Brain::Mutate()
{
    this->maxConviction += (randInt(-1, 1) * (randPercent(50) * randInt(1, 4)));
    if (this->maxConviction < 1)
    {
        this->maxConviction = 1;
    }
    this->rotatevschange += (randInt(-1, 1) * (randPercent(50) * randInt(1, 12)));
    if (this->rotatevschange < 1)
    {
        this->rotatevschange = 1;
    }
    else if (this->rotatevschange > 100)
    {
        this->rotatevschange = 100;
    }
}

Brain Brain::Clone()
{
    return Brain(*this);
}
