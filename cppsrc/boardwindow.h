#include "windowhandler.h"
#include "board.h"

class BoardWindow : public GameWindow
{

public:
    BoardWindow(int width, int height, std::string &title, float scale);
    ~BoardWindow();
    
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