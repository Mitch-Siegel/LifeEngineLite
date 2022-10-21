#include "boardwindow.h"
#include "organismwindow.h"

BoardWindow::BoardWindow(int width, int height, std::string &title, float scale) : GameWindow()
{
    this->autoplay = false;
    this->autoplaySpeed = 1000;
    this->Init(width, height, title, scale);
}

void BoardWindow::SetBoard(Board *b)
{
    this->myBoard = b;
}

void BoardWindow::EventHandler(SDL_Event &e)
{
    if (e.type == SDL_QUIT)
    {
    }
    if (e.type == SDL_KEYDOWN)
    {

        switch (e.key.keysym.sym)
        {
        case SDLK_RETURN:
            autoplay = false;
            frameToRender = 1;
            this->myBoard->Tick();
            this->myBoard->Stats();
            break;

        case SDLK_UP:
            if (!autoplay)
            {
                autoplay = true;
                tick_frequency = 1000 / 60;
            }
            else
            {
                tick_frequency /= 2;
                if (tick_frequency < 1)
                {
                    tick_frequency = 0;
                }
            }
            printf("Tick every %.2f ms\n", tick_frequency);
            break;

        case SDLK_DOWN:
            if (!autoplay)
            {
                autoplay = true;
                tick_frequency = 1000 / 60;
            }
            else
            {
                if (tick_frequency == 0)
                {
                    tick_frequency = 1.041666627;
                }
                else
                {
                    tick_frequency *= 2;
                }
            }
            printf("Tick every %.2f ms\n", tick_frequency);
            break;

        case SDLK_SPACE:
            // if not autoplaying, set max autoplay speed
            if (!autoplay)
            {
                autoplay = true;
                tick_frequency = 0;
                break;
            }

            // if autoplaying, fall through and stop the autoplay

        default:
            autoplay = false;
            frameToRender = 1;
            break;
        }
    }
    else if (e.type == SDL_MOUSEBUTTONDOWN)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);
        x /= scalefactor;
        y /= scalefactor;
        Organism *clickedOrganism = this->myBoard->cells[y][x]->myOrganism;
        if (clickedOrganism != nullptr)
        {
            this->myWS->Add(new OrganismWindow(clickedOrganism));
        }
    }
}

void BoardWindow::Tick()
{
    size_t micros = 0;
    if (this->tick_frequency > 0)
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto diff = now - lastFrame;
        micros = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
    }
    if (this->tick_frequency == 0 || micros > (this->tick_frequency * 1000))
    {
        if (this->autoplay)
        {
            this->myBoard->Tick();
            if (this->myBoard->tickCount % 50 == 0)
            {
                this->myBoard->Stats();
            }
        }
        this->Draw();
        this->Render();
        this->lastFrame = std::chrono::high_resolution_clock::now();
    }
}

void BoardWindow::Draw()
{
    // we'll use this texture as our own backbuffer
    static SDL_Texture *boardBuf = SDL_CreateTexture(this->r, SDL_PIXELFORMAT_RGB888,
                                                     SDL_TEXTUREACCESS_TARGET, this->myBoard->dim_x, this->myBoard->dim_y);

    // draw to board buffer instead of backbuffer
    SDL_SetRenderTarget(r, boardBuf);

    for (int y = 0; y < this->myBoard->dim_y; y++)
    {
        for (int x = 0; x < this->myBoard->dim_x; x++)
        {
            if (this->myBoard->DeltaCells[y][x])
            {
                this->myBoard->DeltaCells[y][x] = false;
                Cell *thisCell = this->myBoard->cells[y][x];
                switch (thisCell->type)
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
                SDL_RenderDrawPoint(r, x, y);
            }
        }
    }
    // reset render target, copy board buf, and spit it out to the screen
    SDL_SetRenderTarget(r, NULL);
    SDL_RenderCopy(r, boardBuf, NULL, NULL);
}
