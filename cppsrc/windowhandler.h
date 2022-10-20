#include <SDL2/SDL.h>
#include <string>
#include <map>

#include "board.h"

class GameWindow
{
    friend class WindowingSystem;
    friend class BoardWindow;

public:
    GameWindow(int width, int height, std::string &title, float scale);
    ~GameWindow();
    void Focus();
    void Render();

    virtual void Tick() = 0;
    virtual void Draw() = 0;

private:
    uint32_t id_;
    std::string title;

    SDL_Window *w;
    SDL_Renderer *r;
    int width, height;
    bool focused;
    virtual void EventHandler(SDL_Event &e) = 0;

public:
    const decltype(id_) &id() const { return id_; }
    const decltype(w) &window() const { return w; }
    const decltype(r) &renderer() const { return r; }
};

class BoardWindow : public GameWindow
{

public:
    BoardWindow(int width, int height, std::string &title, float scale);
    
    void SetBoard(Board *b);
    
    void EventHandler(SDL_Event &e);
    
    void Tick();
    
    void Draw();

private:
    bool autoplay;
    
    size_t autoplaySpeed;
    
    int frameToRender;
    
    Board *myBoard;
};

class WindowingSystem
{
public:
    WindowingSystem();
    ~WindowingSystem();

    GameWindow *Add(GameWindow *w);

    void HandleEvent(SDL_Event &e);

    void Tick();

private:
    std::map<int, GameWindow *> activeWindows;
    // std::vector<GameWindow *> windowList;
};
