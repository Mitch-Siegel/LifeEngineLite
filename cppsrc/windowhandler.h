#include <SDL2/SDL.h>
#include <string>
#include <map>
#include <chrono>


extern int scalefactor;

#pragma once

class WindowingSystem;

class GameWindow
{
    friend class WindowingSystem;
    friend class BoardWindow;
    friend class OrganismWindow;

public:
    GameWindow();
    virtual ~GameWindow() = 0;
    void Init(int width, int height, std::string &title, float scale);
    void Focus();
    void Render();

    virtual void Tick() = 0;
    virtual void Draw() = 0;

private:   
    bool open;

    uint32_t id_;
    std::string title;

    SDL_Window *w;
    SDL_Renderer *r;
    WindowingSystem *myWS;
    int width, height;
    bool focused;
    virtual void EventHandler(SDL_Event &e) = 0;

    float tick_frequency;
    std::chrono::high_resolution_clock::time_point lastFrame;

    void HandleWindowEvent(SDL_Event &e);

public:
    const decltype(id_) &id() const { return id_; }
    const decltype(w) &window() const { return w; }
    const decltype(r) &renderer() const { return r; }
};


class WindowingSystem
{
    friend class GameWindow;
public:
    WindowingSystem();
    ~WindowingSystem();

    GameWindow *Add(GameWindow *w);

    void HandleEvent(SDL_Event &e);

    void Tick();

private:
    std::map<int, GameWindow *> activeWindows;
    std::map<SDL_Renderer *, int> renderReferenceCounts;
    uint32_t focusedWindow;
    // std::vector<GameWindow *> windowList;
};
