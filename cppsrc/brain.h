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

    void Decide();
};
