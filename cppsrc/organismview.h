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
    std::vector<size_t> inputs;
    std::vector<std::set<size_t>> graph;
    std::vector<size_t> outputs;
    // mapping from origin node to {destination node, edge weight}
    std::map<size_t, std::vector<std::pair<size_t, nn_num_t>>> connectionsByPost;
    std::map<size_t, char[6]> labelsByPost;
    std::map<size_t, SimpleNets::Unit *> units;
    std::map<size_t, size_t> idsByPost;
    std::map<size_t, size_t> postsById;

public:
    OrganismView(Organism *o, SDL_Renderer *r);

    ~OrganismView();

    void OnFrame();

    bool isOpen() { return this->open; }
};
