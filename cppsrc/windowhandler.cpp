#include "windowhandler.h"
#include <SDL2/SDL.h>



GameWindow::GameWindow(int width, int height, std::string &title)
{
    SDL_CreateWindowAndRenderer(width, height, 0, &w, &r);
    this->id_ = SDL_GetWindowID(w);
    if (w != NULL && r != NULL)
    {
        focused = true;
        EventHandlerFunction = nullptr;
        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    }
    else
    {
        printf("Error creating window %s\n", title.c_str());
        exit(1);
    }
}

void GameWindow::HandleEvent(SDL_Event &e)
{
    printf("Gamewindow::handleevent\n");
    this->BoardInputHandler(e);
    /*
    if (e.type == SDL_WINDOWEVENT && e.window.windowID == this->id())
    {
        switch (e.window.event)
        {
        case SDL_WINDOWEVENT_EXPOSED:
            SDL_RenderPresent(r);
            break;

        case SDL_WINDOWEVENT_ENTER:
            this->focused = true;
            break;

        case SDL_WINDOWEVENT_LEAVE:
            this->focused = false;
            break;

        case SDL_WINDOWEVENT_FOCUS_GAINED:
            this->focused = true;
            break;

        case SDL_WINDOWEVENT_FOCUS_LOST:
            this->focused = false;
            break;

        case SDL_WINDOWEVENT_CLOSE:
            SDL_HideWindow(w);
            break;
        }
    }*/
}

void GameWindow::Focus()
{
    SDL_ShowWindow(w);
    SDL_RaiseWindow(w);
}

void GameWindow::Render()
{
    SDL_RenderClear(r);
    SDL_RenderPresent(r);
}

void GameWindow::SetInputHandler(HandlerTypes t)
{
    printf("set input handler for %p\n", this);
    this->EventHandlerFunction = handlerFunctions[t];
}

WindowingSystem::WindowingSystem()
{
}

WindowingSystem::~WindowingSystem()
{
}

GameWindow *WindowingSystem::Create(int width, int height, std::string &title)
{
    GameWindow *created = new GameWindow(width, height, title);
    this->activeWindows[created->id()] = created;
    return created;
}

GameWindow *WindowingSystem::Create(int width, int height, float scale, std::string &title)

{
    GameWindow *created = this->Create(width * scale, height * scale, title);
    SDL_RenderSetScale(created->renderer(), scale, scale);
    return created;
}

void WindowingSystem::HandleEvent(SDL_Event &e)
{
    for(auto w : this->activeWindows)
    {
        if(w.second->focused)
        {
            w.second->EventHandlerFunction(e);
        }
    }
}