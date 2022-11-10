// #include "imgui.h"
// #include <SDL2/SDL.h>
#include "config.h"
#include <cstdint>

class SDL_Texture;
class SDL_Renderer;
class Organism;

class OrganismView
{
    private:
        SDL_Texture *t;
        Organism *myOrganism;
        uint64_t mySpecies;
        bool open;
        int dim_x, dim_y;
        char name[48];
        uint32_t cellCounts[cell_null];
        uint32_t nCells;

    public:
        OrganismView(Organism *o, SDL_Renderer *r);

        ~OrganismView();

        void OnFrame();

        bool isOpen() {return this->open;}
};
