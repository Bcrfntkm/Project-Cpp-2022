#ifndef graphics
#define graphics
#include "SDL.h"
#include <memory>
#include <array>
#include <list>
#include "../Newton/Newton.cpp"

struct DPoint{
    double x;
    double y;
    DPoint(double x, double y);
    DPoint();
    DPoint(DPoint const &src) = default;
    DPoint(DPoint &&src) = default;
    DPoint& operator=(DPoint const &rhs) = default;
    DPoint& operator=(DPoint &&rhs) = default;
};
struct VirtualFrame{
    double x;    
    double y;    
    double w;    
    double h;    
    VirtualFrame(double x, double y, double w, double h);
    VirtualFrame(SDL_Rect frame);
    VirtualFrame(VirtualFrame const &src) = default;
    VirtualFrame(VirtualFrame &&src) = default;
    VirtualFrame& operator=(VirtualFrame const &src) = default;
    VirtualFrame& operator=(VirtualFrame &&src) = default;

    DPoint to_virtual(SDL_Point p, SDL_Rect &frame);
    SDL_Point to_SDL(DPoint p, SDL_Rect &frame);
    VirtualFrame zoom(SDL_Point start, SDL_Point end, SDL_Rect &frame);
    DPoint get_top_left();
    DPoint get_bottom_right();
};
class App;
class Window{
    SDL_Window *win_ptr;
public:
    Window(SDL_Rect& frame);
    Window(Window const &src) = delete;
    Window(Window &&src);
    Window& operator=(Window const &rhs) = delete;
    Window& operator=(Window &&rhs);
    operator  bool() const;
    SDL_Window* get();
    ~Window();
};
class Renderer{
    SDL_Renderer *render_ptr;
public:
    Renderer(Window& win);
    Renderer(Renderer const &src) = delete;
    Renderer(Renderer &&src);
    Renderer& operator=(Renderer const &src) = delete;
    Renderer& operator=(Renderer &&src);
    operator  bool() const;
    void draw_point(SDL_Color color, SDL_Point p);
    void fill_rect(SDL_Color color, SDL_Rect rect);
    void update();
    SDL_Renderer* get();
    ~Renderer();
};

class SafeTexture{
    SDL_Texture * texture;
    SDL_Surface * surf;
public:
    SafeTexture(SDL_Surface *surf, Renderer& renderer);
    SafeTexture(SDL_Surface *surf, Renderer& renderer, SDL_Color key);
    SafeTexture(SafeTexture const &src) = delete;
    SafeTexture(SafeTexture &&src);
    SafeTexture& operator=(SafeTexture const &rhs) = delete;
    SafeTexture& operator=(SafeTexture &&rhs);
    operator  bool() const;
    SDL_Texture * get_texture();
    SDL_Surface * get_surf();
    void update(Renderer& renderer);
    void draw_point(SDL_Point, SDL_Color); 
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
    Button(Button const &src) = delete;
    Button(Button &&src) = delete;
    Button& operator=(Button const &rhs) = delete;
    Button& operator=(Button &&rhs) = delete;
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
    unsigned int indx;
    int click_timer;
public:
    Root(SDL_Rect off_texture, SDL_Rect hover_texture, SDL_Rect rect, unsigned indx);
    Root(Root const &src) = delete;
    Root(Root &&src) = delete;
    Root& operator=(Root const &rhs) = delete;
    Root& operator=(Root &&rhs) = delete;
    void update();
    void pick();
    void release(SDL_Point p);
    void move(SDL_Point p);
    void hover();
    void off();
    bool is_hidden();
    bool can_be_released();
    void draw(Renderer &renderer, SafeTexture &texture_atlas);
    SDL_Rect const & get_rect();
    unsigned get_indx();
    SDL_Point get_centre();
};

class SelectBox{
    SDL_Color color;
    SDL_Rect rect;
    SDL_Point start;
    SDL_Point end;
    bool active;
public:
    SelectBox(SDL_Color color);
    SelectBox(SelectBox const &src) = delete;
    SelectBox(SelectBox &&src) = delete;
    SelectBox& operator=(SelectBox const &rhs) = delete;
    SelectBox& operator=(SelectBox &&rhs) = delete;
    void start_at(SDL_Point p);
    void end_at(SDL_Point p);
    SDL_Point get_start();
    SDL_Point get_end();
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
    VirtualFrame virtual_frame;
    Mode mode;
    bool running;
    std::unique_ptr<Window> win;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<SafeTexture> texture_atlas;
    std::unique_ptr<SafeTexture> background;
    Newton newton;
    std::array<std::unique_ptr<Button>, 4> buttons; 
    std::list<std::shared_ptr<Root> > roots; 
    std::shared_ptr<Root> moving_root;
    SelectBox select;
    std::vector<char> draw_map;
    std::pair<int, int> draw_map_dims;
public:
    App(SDL_Rect frame, VirtualFrame virt_frame);
    App(App const &src) = delete;
    App(App &&src) = delete;
    App& operator=(App const &src) = delete;
    App& operator=(App &&src) = delete;
    void loop();
    void proccess_events();
    void render();
    void run();
    void add_mode();
    void move_mode();
    void refresh();
    void home();
    void zoom(SDL_Point start, SDL_Point end);
    void create_root(SDL_Point);
    void start_move_root(std::shared_ptr<Root> root);
    void end_move_root(SDL_Point p);
    ~App();
};
#endif

