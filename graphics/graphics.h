#ifndef graphics
#define graphics
#include "SDL.h"

class Window{
    SDL_Window *win_ptr;
    SDL_Rect const & frame;
public:
    Window(SDL_Rect& frame);
    bool init();
    operator  bool() const;
    SDL_Window* get();
    ~Window();
};
class Renderer{
    SDL_Renderer *render_ptr;
    Window &win;
    SDL_Rect const &  frame;
public:
    Renderer(SDL_Rect& frame, Window& win);
    bool init();
    operator  bool() const;
    void draw_point(SDL_Color color, SDL_Point p);
    void update();
    SDL_Renderer* get();
    ~Renderer();
};
class App{
private:
    SDL_Rect frame;
    bool running;
    Window win;
    Renderer renderer;
public:
    App();
    void loop();
    void proccess_events();
    void render();
    void run();
    ~App();
};
#endif

