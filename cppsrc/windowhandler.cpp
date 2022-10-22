#include "windowhandler.h"
#include <SDL2/SDL.h>

GameWindow::GameWindow()
{
    this->id_ = 0;
    this->w = nullptr;
    this->r = nullptr;
    this->lastFrame = std::chrono::high_resolution_clock::now();
    this->tick_frequency = 1000.0 / 60.0;
    this->open = true;
}

GameWindow::~GameWindow()
{
    if (this->myWS->renderReferenceCounts[this->r]-- == 0)
    {
        SDL_DestroyRenderer(this->r);
    }
}

void GameWindow::Init(int width, int height, std::string &title, float scale)
{
    // SDL_CreateWindowAndRenderer(width, height, 0, &this->w, &this->r);
    this->w = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    if (w != NULL)
    {
        this->id_ = SDL_GetWindowID(this->w);
        this->r = SDL_CreateRenderer(this->w, -1, SDL_RENDERER_ACCELERATED);
        if (this->r == NULL)
        {
            printf("Error creating renderer for window %s\n", title.c_str());
            exit(1);
        }
        this->focused = true;
        SDL_RenderSetScale(r, scale, scale);
        SDL_SetRenderDrawColor(this->r, 0, 0, 0, 0);
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

WindowingSystem::WindowingSystem()
{
}

WindowingSystem::~WindowingSystem()
{
}

GameWindow *WindowingSystem::Add(GameWindow *w)
{
    this->activeWindows[w->id()] = w;
    w->myWS = this;
    renderReferenceCounts[w->r]++;
    return w;
}

void WindowingSystem::HandleEvent(SDL_Event &e)
{
    for (auto w : this->activeWindows)
    {
        if (w.second->open)
        {
            w.second->EventHandler(e);
        }
    }
}

void WindowingSystem::Tick()
{
    for (auto w = this->activeWindows.begin(), next_w = w; w != this->activeWindows.end(); w = next_w)
    {
        ++next_w;
        if (w->second->open)
        {
            w->second->Tick();
        }
        else
        {
            delete w->second;
            this->activeWindows.erase(w->second->id());
        }
        // w.second->Render();
    }
    // SDL_Delay(1);
}
