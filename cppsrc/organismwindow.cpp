#include "organismwindow.h"
#include "boardwindow.h"

OrganismWindow::OrganismWindow(Organism *o) : GameWindow()
{
    this->myOrganism = o;
    int x_min = 0;
    int x_max = 0;
    int y_min = 0;
    int y_max = 0;
    for (Cell *c : o->myCells)
    {
        int x_rel = c->x - o->x;
        int y_rel = c->y - o->y;
        if (x_rel < x_min)
        {
            x_min = x_rel;
        }
        else if (x_rel > x_max)
        {
            x_max = x_rel;
        }

        if (y_rel < y_min)
        {
            y_min = y_rel;
        }
        else if (y_rel > y_max)
        {
            y_max = y_rel;
        }
    }
    std::string name("Organism View");
    this->width = x_max - x_min + 1;
    this->height = y_max - y_min + 1;
    this->Init(this->width * 32, this->height * 32, name, 32);
}

OrganismWindow::~OrganismWindow()
{
    printf("~OrganismWindow()\n");
    SDL_DestroyWindow(this->w);
    // SDL_DestroyRenderer(this->r);
}

void OrganismWindow::EventHandler(SDL_Event &e)
{
    if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)
    {
        this->open = false;
        // this->myWS->CloseWindow(this);
        return;
    }
    if (e.type == SDL_KEYDOWN && this->focused)
    {
        switch (e.key.keysym.sym)
        {
        default:
            break;
        }
    }
    else if (e.type == SDL_MOUSEBUTTONDOWN && this->focused)
    {
    }
}

void OrganismWindow::Tick()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto diff = now - lastFrame;
    size_t micros = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
    if (micros > this->tick_frequency)
    {
        this->Draw();
        this->Render();
    }
    this->lastFrame = std::chrono::high_resolution_clock::now();
}

void OrganismWindow::Draw()
{
    // we'll use this texture as our own backbuffer
    static SDL_Texture *winBuf = SDL_CreateTexture(this->r, SDL_PIXELFORMAT_RGB888,
                                                   SDL_TEXTUREACCESS_TARGET, this->width, this->width);

    // draw to board buffer instead of backbuffer
    SDL_SetRenderTarget(r, winBuf);

    for (Cell *c : this->myOrganism->myCells)
    {
        int x_rel = c->x - this->myOrganism->x;
        int y_rel = c->y - this->myOrganism->y;
        switch (c->type)
        {
        case cell_empty:
            SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
            break;

        case cell_plantmass:
            SDL_SetRenderDrawColor(r, 10, 40, 10, 255);
            break;

        case cell_biomass:
            SDL_SetRenderDrawColor(r, 150, 60, 60, 255);
            break;

        case cell_leaf:
            SDL_SetRenderDrawColor(r, 30, 120, 30, 255);
            break;

        case cell_bark:
            SDL_SetRenderDrawColor(r, 75, 25, 25, 255);
            break;

        case cell_mover:
            SDL_SetRenderDrawColor(r, 50, 120, 255, 255);
            break;

        case cell_herbivore_mouth:
            SDL_SetRenderDrawColor(r, 255, 150, 0, 255);
            break;

        case cell_carnivore_mouth:
            SDL_SetRenderDrawColor(r, 255, 100, 150, 255);
            break;

        case cell_flower:
            SDL_SetRenderDrawColor(r, 50, 250, 150, 255);
            break;

        case cell_fruit:
            SDL_SetRenderDrawColor(r, 200, 200, 0, 255);
            break;

        case cell_killer:
            SDL_SetRenderDrawColor(r, 255, 0, 0, 255);
            break;

        case cell_armor:
            SDL_SetRenderDrawColor(r, 175, 0, 255, 255);
            break;

        case cell_touch:
            SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
            break;

        case cell_null:
            break;
        }
        SDL_RenderDrawPoint(r, (width / 2) + x_rel, (height / 2) + y_rel);
    }

    // reset render target, copy board buf, and spit it out to the screen
    SDL_SetRenderTarget(r, NULL);
    SDL_RenderCopy(r, winBuf, NULL, NULL);
}
