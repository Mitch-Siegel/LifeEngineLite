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
    int moveDirIndex;
    int conviction, maxConviction;

    enum Intent Decide();

    void Reward();

    void Punish();

    void Mutate();

    Brain Clone();

};
