#include "windowhandler.h"
#include <SDL2/SDL.h>

GameWindow::GameWindow(int width, int height, std::string &title, float scale)
{
    SDL_CreateWindowAndRenderer(width, height, 0, &w, &r);
    this->id_ = SDL_GetWindowID(w);
    if (w != NULL && r != NULL)
    {
        focused = true;
        // SDL_RenderSetScale(r, scale, scale);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    }
    else
    {
        printf("Error creating window %s\n", title.c_str());
        exit(1);
    }
}

void GameWindow::Focus()
{
    SDL_ShowWindow(w);
    SDL_RaiseWindow(w);
}

void GameWindow::Render()
{
    SDL_RenderPresent(r);
}

BoardWindow::BoardWindow(int width, int height, std::string &title, float scale) : GameWindow(width, height, title, scale)
{
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
                autoplaySpeed = 1000;
                autoplay = true;
            }
            else
            {
                if (autoplaySpeed > 1)
                {
                    autoplaySpeed /= 2;
                    if (autoplaySpeed < 1)
                    {
                        autoplaySpeed = 1;
                    }
                }
                else
                {
                    autoplaySpeed = 0;
                }
            }
            printf("Delay %lums between frames\n", autoplaySpeed);
            break;

        case SDLK_DOWN:
            if (!autoplay)
            {
                autoplaySpeed = 1000;
                autoplay = true;
            }
            else
            {
                if (autoplaySpeed == 0)
                {
                    autoplaySpeed = 1;
                }
                else
                {
                    autoplaySpeed *= 2;
                    if (autoplaySpeed > 1000)
                    {
                        autoplaySpeed = 1000;
                    }
                }
            }
            printf("Delay %lums between frames\n", autoplaySpeed);
            break;

        case SDLK_RIGHT:
            frameToRender *= 2;
            printf("Rendering every %d tick(s)\n", frameToRender);
            break;

        case SDLK_LEFT:
            frameToRender /= 2;
            if (frameToRender < 1)
            {
                frameToRender = 1;
            }
            printf("Rendering every %d tick(s)\n", frameToRender);
            break;

        case SDLK_SPACE:
            // if not autoplaying, set max autoplay speed
            if (!autoplay)
            {
                autoplay = true;
                autoplaySpeed = 0;
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

        printf("%d, %d\n", x, y);
    }
}

void BoardWindow::Tick()
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

WindowingSystem::WindowingSystem()
{
}

WindowingSystem::~WindowingSystem()
{
}

GameWindow *WindowingSystem::Add(GameWindow *w)
{
    this->activeWindows[w->id()] = w;
    return w;
}

void WindowingSystem::HandleEvent(SDL_Event &e)
{
    for (auto w : this->activeWindows)
    {
        if (w.second->focused)
        {
            w.second->EventHandler(e);
        }
    }
}

void WindowingSystem::Tick()
{
    for (auto w : this->activeWindows)
    {
        w.second->Tick();
        w.second->Render();
    }
}