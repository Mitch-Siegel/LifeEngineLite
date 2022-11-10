#include "organismview.h"
#include <SDL2/SDL.h>

#include "imgui.h"
#include "lifeforms.h"
#include "util.h"

#define ORGANISM_VIEWER_SCALE_FACTOR 32.0

OrganismView::OrganismView(Organism *o, SDL_Renderer *r)
{
    this->myOrganism = o;
    int maxX = 1;
    int maxY = 1;
    int organismX = o->x;
    int organismY = o->y;
    for (Cell *c : o->myCells)
    {
        int x_rel = c->x - organismX;
        int y_rel = c->y - organismY;

        if (abs(x_rel) > maxX) maxX = abs(x_rel);
        if (abs(y_rel) > maxY) maxY = abs(y_rel);
    }
    this->dim_x = (maxX * 2) + 1;
    this->dim_y = (maxY * 2) + 1;
    this->t = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET,
                                this->dim_x * ORGANISM_VIEWER_SCALE_FACTOR, this->dim_y * ORGANISM_VIEWER_SCALE_FACTOR);
    printf("texture dimension is %d, %d\n", this->dim_x, this->dim_y);
    SDL_SetRenderTarget(r, this->t);
    if(SDL_RenderSetScale(r, ORGANISM_VIEWER_SCALE_FACTOR, ORGANISM_VIEWER_SCALE_FACTOR))
    {
        printf("Couldn't set render scale!\n");
        exit(1);
    }
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    printf("organism %p with %lu cells\n", o, o->nCells());
    for (Cell *c : o->myCells)
    {
        SetColorForCell(r, c);
        int x_rel = c->x - o->x;
        int y_rel = c->y - o->y;
        SDL_RenderDrawPoint(r, x_rel + maxX, y_rel + maxY);
        printf("rendering at %d, %d\n", x_rel + maxX, y_rel + maxY);
    }
    SDL_RenderSetScale(r, 1.0, 1.0);
    SDL_SetRenderTarget(r, nullptr);
    this->open = true;
    sprintf(this->name, "Organism Viewer (%p)", o);
}

void OrganismView::OnFrame()
{
    ImGui::Begin(this->name);
    if(ImGui::Button("Close", ImVec2(125, 25)))
    {
        printf("window closed!\n");
        this->open = false;
    }
    ImGui::Image(this->t, ImVec2(this->dim_x * ORGANISM_VIEWER_SCALE_FACTOR, this->dim_y * ORGANISM_VIEWER_SCALE_FACTOR));
    // ImGui::Text("Test text");
    ImGui::End();
}

OrganismView::~OrganismView()
{
    SDL_DestroyTexture(this->t);
}
