// #include "imgui.h"
// #include <SDL2/SDL.h>

#include <cstdint>
#include <vector>
#include <set>
#include <stddef.h>
#include <map>
#include "config.h"
#include "nnet.h"

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

    void VisualizeBrain();
    std::vector<std::set<size_t>> graph;
    // mapping from origin node to {destination node, edge weight}
    std::map<size_t, std::vector<std::pair<size_t, nn_num_t>>> connectionsByPost;

public:
    OrganismView(Organism *o, SDL_Renderer *r);

    ~OrganismView();

    void OnFrame();

    bool isOpen() { return this->open; }
};
