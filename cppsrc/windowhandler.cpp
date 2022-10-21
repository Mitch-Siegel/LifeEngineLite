#include "windowhandler.h"
#include <SDL2/SDL.h>

GameWindow::GameWindow()
{
    this->id_ = 0;
    this->r = nullptr;
    this->w = nullptr;
    this->focused = false;
    this->myWS = nullptr;
    this->lastFrame = std::chrono::high_resolution_clock::now();
    this->tick_frequency = 1000.0 / 60.0;
}

GameWindow::~GameWindow()
{
}

void GameWindow::Init(int width, int height, std::string &title, float scale)
{
    SDL_CreateWindowAndRenderer(width, height, 0, &this->w, &this->r);
    this->id_ = SDL_GetWindowID(this->w);
    if (w != NULL && r != NULL)
    {
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
        // w.second->Render();
    }
}