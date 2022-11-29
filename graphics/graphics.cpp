#include "graphics.h"
#include <unistd.h>

Window::Window(SDL_Rect& frame):win_ptr(nullptr), frame(frame){}
bool Window::init(){
    win_ptr = SDL_CreateWindow("", frame.x, frame.y, frame.w, frame.h, SDL_WINDOW_SHOWN);
    return win_ptr != nullptr;
}
Window::operator  bool() const {
    return win_ptr != nullptr;
}
SDL_Window* Window::get(){
    return win_ptr;
}
Window::~Window(){
    SDL_DestroyWindow(win_ptr);
}

Renderer::Renderer(SDL_Rect& frame, Window& win): render_ptr(nullptr), frame(frame), win(win){}
bool Renderer::init(){
    render_ptr = SDL_CreateRenderer(win.get(), -1, SDL_RENDERER_SOFTWARE);
    SDL_SetRenderDrawBlendMode(render_ptr, SDL_BLENDMODE_BLEND);
    return render_ptr != nullptr;
}
Renderer::operator bool() const{
    return render_ptr != nullptr;
}
void Renderer::draw_point(SDL_Color color, SDL_Point p){
    SDL_SetRenderDrawColor(render_ptr, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(render_ptr, p.x, p.y);
}
void Renderer::update(){
    SDL_RenderPresent(render_ptr);
}
SDL_Renderer* Renderer::get(){
    return render_ptr;
}
Renderer::~Renderer(){
    SDL_DestroyRenderer(render_ptr);
}

App::App():frame({100,100,1000,600}), running(false), win(frame), 
    renderer(frame, win){
    if(SDL_Init(SDL_INIT_VIDEO) == 0){
        if (win.init()){
            if (renderer.init()){
                running = true;
            }
        }
    }
}
void App::proccess_events(){
    SDL_Event event; 
    while(SDL_PollEvent(&event)){
        if (event.type == SDL_QUIT)
            running = false;
    }
}
void App::loop(){
}
void App::render(){
    for (unsigned i = 0; i < frame.h; ++i){
        for (unsigned j = 0; j < frame.w; ++j){
            renderer.draw_point(SDL_Color({0,0,0,255}), SDL_Point({j, i}));
        }
    }
    renderer.update();
}
void App::run(){
    while(running){
        proccess_events();
        loop();
        render();
        sleep(0.03);
    }
}
App::~App(){
    SDL_Quit();
} 
