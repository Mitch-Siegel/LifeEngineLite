#include "config.h"

enum Intent
{
    intent_continue,
    intent_changeDir,
    intent_rotate,
};

class Brain
{
public:
    Brain();
    
    enum Intent currentIntent;
    int moveDirIndex;
    int conviction;

    void Decide();

    void Reward();

    void Punish();
};
