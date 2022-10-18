#include "brain.h"
#include "rng.h"

Brain::Brain()
{
    this->currentIntent = intent_changeDir;
    this->moveDirIndex = -1;
    this->conviction = 0;
    this->maxConviction = 2;
    // chance to rotate instead of changing direction
    this->rotatevschange = 25;
    this->turnwhenrotate = 50;
    this->justRewarded = false;

    for (int i = 0; i < cell_null; i++)
    {
        this->cellSentiments[i] = 0;
    }

    int startingSentimentIndex = randInt(cell_empty, cell_null - 1);
    this->cellSentiments[startingSentimentIndex] = randPercent(50) ? 1 : -1;
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

void Brain::ForceRechoose()
{
    this->conviction = -1 * this->maxConviction;
}

enum Intent Brain::Decide()
{
    if (justRewarded)
    {
        justRewarded = false;
        return currentIntent;
    }
    if ((this->currentIntent == intent_continue) &&
        ((randInt(0, 2 * maxConviction) - maxConviction) > conviction /* ||
          (randPercent(3) && randPercent(3))*/
         ))
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
        this->moveDirIndex = randInt(0, 3);
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
    if (randPercent(turnwhenrotate))
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
    else
    {
        this->currentIntent = intent_changeDir;
    }
}

void Brain::Mutate()
{
    // slightly bias towards lowering conviction so it doesn't just always shoot up
    this->maxConviction += (randInt(-3, 2));
    if (this->maxConviction < 1)
    {
        this->maxConviction = 1;
    }
    this->rotatevschange += (randInt(-1, 1) * (1 + (randPercent(50) * randInt(0, 2))));
    if (this->rotatevschange < 0)
    {
        this->rotatevschange = 0;
    }
    else if (this->rotatevschange > 100)
    {
        this->rotatevschange = 100;
    }

    this->turnwhenrotate += (randInt(-1, 1) * (1 + (randPercent(50) * randInt(0, 2))));
    if (this->turnwhenrotate < 0)
    {
        this->turnwhenrotate = 0;
    }
    else if (this->turnwhenrotate > 100)
    {
        this->turnwhenrotate = 100;
    }

    if (randPercent(10))
    {
        this->cellSentiments[randInt(cell_empty + 1, cell_null - 1)] += (randPercent(50) ? 1 : -1);
        // for (int i = cell_empty; i < cell_null; i++)
        // {
        // if (randPercent(10))
        // {
        // this->cellSentiments[i] += randInt(-1, 1);
        // }
        // }
    }
}

Brain Brain::Clone()
{
    return Brain(*this);
}
