// #include "imgui.h"
// #include <SDL2/SDL.h>

#include <cstdint>
#include <vector>
#include <set>
#include <stddef.h>
#include <map>
#include "config.h"
#include "nnet.h"
#include "lifeforms.h"

class SDL_Texture;
class SDL_Renderer;
class Organism;

class OrganismView
{
private:
    SDL_Texture *texture;
    SDL_Texture *scaledTexture;
    Organism *myOrganism;
    OrganismIdentifier myIdentifier;
    bool allowUpdates;
    bool open;
    int dim_x, dim_y;
    char name[64];
    uint32_t cellCounts[cell_null];
    uint32_t nCells;

    void SetUpBrainVisualization();
    void DrawOrganism(SDL_Renderer *r);
    void Update(SDL_Renderer *r);

    std::vector<size_t> inputs;
    std::vector<std::set<size_t>> graph;
    std::vector<size_t> outputs;
    
    // mapping from origin node to {destination node, edge weight}
    std::map<size_t, std::vector<std::pair<size_t, nn_num_t>>> connectionsByPost;
    
    // mapping from unit POST number to activation of that unit
    std::map<size_t, nn_num_t> activationsByPost;
    // mapping from unit POST number to string of activation number
    std::map<size_t, char[6]> labelsByPost;
    
    // mapping from unit ID to unit pointer
    std::map<size_t, SimpleNets::Unit *> units;
    std::map<size_t, size_t> idsByPost;
    std::map<size_t, size_t> postsById;

public:
    OrganismView(Organism *o, SDL_Renderer *r);

    ~OrganismView();

    void DisableUpdates();

    void OnFrame(SDL_Renderer *r);

    bool isOpen() { return this->open; }
};
