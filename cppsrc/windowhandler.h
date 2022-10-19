#include <SDL2/SDL.h>
#include <string>
#include <map>

class GameWindow
{
    friend class WindowingSystem;

public:
    GameWindow(int width, int height, std::string &title);
    ~GameWindow();
    void HandleEvent(SDL_Event &e);
    void Focus();
    void Render();

    void BoardInputHandler(SDL_Event &e);
    void OrganismViewInputHandler(SDL_Event &e);

    enum HandlerTypes
    {
        handler_board,
        handler_organismview,
        max_handlers,
    };

    void SetInputHandler(HandlerTypes t);
    // typedef B* (A::*BMemFun)(void);
    // BMemFun funcs_[2];
    void (GameWindow::*handlerFunctions[max_handlers])(SDL_Event &e) = {GameWindow::BoardInputHandler, GameWindow::OrganismViewInputHandler};

private:
    uint32_t id_;
    std::string title;

    SDL_Window *w;
    SDL_Renderer *r;
    int width, height;
    bool focused;
    void (*EventHandlerFunction)(SDL_Event &e);

public:
    const decltype(id_) &id() const { return id_; }
    const decltype(w) &window() const { return w; }
    const decltype(r) &renderer() const { return r; }
};

class WindowingSystem
{
public:
    WindowingSystem();
    ~WindowingSystem();

    GameWindow *Create(int width, int height, std::string &title);

    GameWindow *Create(int width, int height, float scale, std::string &title);

    void HandleEvent(SDL_Event &e);

private:
    std::map<int, GameWindow *> activeWindows;
    // std::vector<GameWindow *> windowList;
};
