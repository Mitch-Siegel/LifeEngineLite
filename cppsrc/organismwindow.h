#include "windowhandler.h"
#include "lifeforms.h"


class OrganismWindow : public GameWindow
{

public:
    OrganismWindow(Organism *o);
    
    void EventHandler(SDL_Event &e);
    
    void Tick();
    
    void Draw();

private:
    Organism *myOrganism;
    int width, height;
};