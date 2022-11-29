#ifndef graphics
#define graphics
#include "SDL.h"
#include <memory>
#include <array>
#include <list>

class App;
class Window{
    SDL_Window *win_ptr;
public:
    Window(SDL_Rect& frame);
    operator  bool() const;
    SDL_Window* get();
    ~Window();
};
class Renderer{
    SDL_Renderer *render_ptr;
public:
    Renderer(Window& win);
    operator  bool() const;
    void draw_point(SDL_Color color, SDL_Point p);
    void fill_rect(SDL_Color color, SDL_Rect rect);
    void update();
    SDL_Renderer* get();
    ~Renderer();
};

class SafeTexture{
    SDL_Texture * texture;
public:
    SafeTexture(SDL_Surface *surf, Renderer& renderer);
    SafeTexture(SDL_Surface *surf, Renderer& renderer, SDL_Color key);
    operator  bool() const;
    SDL_Texture * get();
    ~SafeTexture();
};

class Button{
public:
    enum ButtonState {OFF, HOVER, ON};
private:
    ButtonState state;
    SDL_Rect off_texture;
    SDL_Rect hover_texture;
    SDL_Rect on_texture;
    App & app;
    void (App::*trigger)();
    SDL_Rect rect;
    int click_timer;
public:
    Button(SDL_Rect off_texture, SDL_Rect hover_texture, SDL_Rect on_texture, SDL_Rect rect, App & app, void (App::*trigger)());
    void press();
    void release();
    void hover();
    void off();
    void draw(Renderer &renderer, SafeTexture &texture_atlas);
    void update();
    SDL_Rect const & get_rect();
};

class Root{
public:
    enum RootState {HIDDEN, SHOWN, HOVER};
private:
    RootState state;
    SDL_Rect off_texture;
    SDL_Rect hover_texture;
    SDL_Rect rect;
    int click_timer;
public:
    Root(SDL_Rect off_texture, SDL_Rect hover_texture, SDL_Rect rect);
    void update();
    void pick();
    void release(SDL_Point p);
    void hover();
    void off();
    bool is_hidden();
    bool can_be_released();
    void draw(Renderer &renderer, SafeTexture &texture_atlas);
    SDL_Rect const & get_rect();
};

class SelectBox{
    SDL_Color color;
    SDL_Rect rect;
    SDL_Point start;
    SDL_Point end;
    bool active;
public:
    SelectBox(SDL_Color color);
    void start_at(SDL_Point p);
    void end_at(SDL_Point p);
    void release();
    bool is_active(){
        return active;
    }
    void draw(Renderer &renderer);
    SDL_Rect const & get_rect();
};

class App{
public:
    enum Mode {NORMAL, ADD, MOVE};
private:
    SDL_Rect frame;
    Mode mode;
    bool running;
    std::unique_ptr<Window> win;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<SafeTexture> texture_atlas;
    std::array<std::unique_ptr<Button>, 4> buttons; 
    std::list<std::shared_ptr<Root> > roots; 
    std::shared_ptr<Root> moving_root;
    SelectBox select;
public:
    App();
    void loop();
    void proccess_events();
    void render();
    void run();
    void add_mode();
    void move_mode();
    void refresh();
    void home();
    void create_root(SDL_Point);
    void move_root(std::shared_ptr<Root> root);
    ~App();
};
#endif

