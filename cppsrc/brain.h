#include "config.h"

enum Intent
{
    intent_continue,
    intent_changeDir,
    intent_rotateClockwise,
    intent_rotateCounterClockwise,
};

class Brain
{
public:
    Brain();
    
    enum Intent currentIntent;
    bool justRewarded;
    int moveDirIndex;
    int conviction, maxConviction, rotatevschange, turnwhenrotate;
    
    int cellSentiments[cell_null];
    
    enum Intent Decide();

    void Reward();

    void Punish();

    void Mutate();

    void RotateSuccess(bool clockwise);

    Brain Clone();

};
