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
    float conviction;
    int maxConviction, rotatevschange, turnwhenrotate;
    
    float cellSentiments[cell_null];
    
    enum Intent Decide();

    void Reward(float amount);

    void Punish(float amount);

    void ForceRechoose();

    void Mutate();

    void RotateSuccess(bool clockwise);

    Brain Clone();

};
