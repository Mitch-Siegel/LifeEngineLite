// #include "imgui.h"
// #include <SDL2/SDL.h>

class SDL_Texture;
class SDL_Renderer;
class Organism;

class OrganismView
{
    private:
        SDL_Texture *t;
        Organism *myOrganism;
        bool open;
        int dim_x, dim_y;
        char name[48];

    public:
        OrganismView(Organism *o, SDL_Renderer *r);

        ~OrganismView();

        void OnFrame();

        bool isOpen() {return this->open;}
};
