#include "graphics.h"
#include <unistd.h>
#include <iostream>

Window::Window(SDL_Rect& frame){
    win_ptr = SDL_CreateWindow("", frame.x, frame.y, frame.w, frame.h, SDL_WINDOW_SHOWN);
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

Renderer::Renderer(Window& win){
    render_ptr = SDL_CreateRenderer(win.get(), -1, SDL_RENDERER_SOFTWARE);
    SDL_SetRenderDrawBlendMode(render_ptr, SDL_BLENDMODE_BLEND);
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

SafeTexture::SafeTexture(SDL_Surface *surf, Renderer &renderer):texture(nullptr){
    if (surf != nullptr){
        texture = SDL_CreateTextureFromSurface(renderer.get(), surf);
        SDL_FreeSurface(surf);
    }
}
SafeTexture::SafeTexture(SDL_Surface *surf, Renderer &renderer, SDL_Color key):texture(nullptr){
    if (surf != nullptr){
        SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format, key.r, key.g, key.b));
        texture = SDL_CreateTextureFromSurface(renderer.get(), surf);
        SDL_FreeSurface(surf);

    }
}
SafeTexture::operator  bool() const {
    return texture != nullptr;
}
SDL_Texture* SafeTexture::get(){
    return texture;
}

SafeTexture::~SafeTexture(){
    if(texture != nullptr)
        SDL_DestroyTexture(texture);
}


Button::Button(SDL_Rect off_texture, SDL_Rect hover_texture, SDL_Rect on_texture, SDL_Rect rect, App& app, void (App::*trigger)()): 
    off_texture(off_texture), hover_texture(hover_texture), on_texture(on_texture), rect(rect), app(app), trigger(trigger), state(ButtonState::OFF), click_timer(0){}
void Button::press(){
                    if(click_timer == 0){
                        state = ButtonState::ON; 
                    }
}
void Button::hover(){if(click_timer == 0) state = ButtonState::HOVER;}
void Button::off(){if(click_timer == 0) state = ButtonState::OFF;}
void Button::release(){if(click_timer == 0) {click_timer = 3; (app.*trigger)();}}
void Button::draw(Renderer &renderer, SafeTexture &texture_atlas){
    if (state == OFF){
        SDL_RenderCopy(renderer.get(), texture_atlas.get(), &off_texture, &rect);
    }else if (state == HOVER){
        SDL_RenderCopy(renderer.get(), texture_atlas.get(), &hover_texture, &rect);
    }else if (state == ON){
        SDL_RenderCopy(renderer.get(), texture_atlas.get(), &on_texture, &rect);
    }
}
void Button::update(){
    if (click_timer > 0){
        click_timer --;
        if (click_timer == 0)
            hover();
    }
}
SDL_Rect const & Button::get_rect(){return rect;}


App::App():frame({100,100,1000,600}), running(false), mode(NORMAL) 
    {
    if(SDL_Init(SDL_INIT_VIDEO) == 0){
        win.reset(new Window(frame));
        if (win){
            renderer.reset(new Renderer(*win.get()));
            if (renderer){
                texture_atlas.reset(new SafeTexture(SDL_LoadBMP("resources/texture_atlas.bmp"), *renderer,SDL_Color({255, 0, 255, 255})));
                if (texture_atlas){

                    buttons[0].reset(new Button(
                            SDL_Rect({0,0,100,100}), SDL_Rect({100,0,100,100}), SDL_Rect({200,0,100,100}), 
                            SDL_Rect({10,10,100,100}), *this, &App::home));
                    buttons[1].reset(new Button(
                            SDL_Rect({0,100,100,100}), SDL_Rect({100,100,100,100}), SDL_Rect({200,100,100,100}), 
                            SDL_Rect({10,115,100,100}), *this, &App::refresh));
                    buttons[2].reset(new Button(
                            SDL_Rect({0,200,100,100}), SDL_Rect({100,200,100,100}), SDL_Rect({200,200,100,100}), 
                            SDL_Rect({10,220,100,100}), *this, &App::add_mode));
                    buttons[3].reset(new Button(
                            SDL_Rect({0,300,100,100}), SDL_Rect({100,300,100,100}), SDL_Rect({200,300,100,100}), 
                            SDL_Rect({10,325,100,100}), *this, &App::move_mode));
                    running = true;
                }
            }
        }
    }
}
void App::proccess_events(){
    SDL_Event event; 
    while(SDL_PollEvent(&event)){
        if (event.type == SDL_QUIT){
            running = false;
        }else if(event.type == SDL_MOUSEBUTTONDOWN){
            SDL_Point mouse_pos({event.button.x, event.button.y});
            for (auto it = buttons.begin(); it != buttons.end(); ++it){
                if (SDL_PointInRect(&mouse_pos, &((*it)->get_rect())))
                (*it)->press();
            }

        }else if(event.type == SDL_MOUSEBUTTONUP){
            SDL_Point mouse_pos({event.button.x, event.button.y});

            for (auto it = buttons.begin(); it != buttons.end(); ++it){
                if (SDL_PointInRect(&mouse_pos, &((*it)->get_rect())))
                (*it)->release();
            }

        }else if(event.type == SDL_MOUSEMOTION){
            SDL_Point mouse_pos({event.motion.x, event.motion.y});
            for (auto it = buttons.begin(); it != buttons.end(); ++it){
                if (SDL_PointInRect(&mouse_pos, &((*it)->get_rect())))
                    (*it)->hover();
                else{
                    (*it)->off();
                }
            }
        }
    }
}
void App::loop(){
    for (auto it = buttons.begin(); it != buttons.end(); ++it){
        (*it)->update();
    }

}
void App::render(){
    for (unsigned i = 0; i < frame.h; ++i){
        for (unsigned j = 0; j < frame.w; ++j){
            renderer->draw_point(SDL_Color({0,0,0,255}), SDL_Point({j, i}));
        }
    }
    for (auto it = buttons.begin(); it != buttons.end(); ++it){
        (*it)->draw(*renderer, *texture_atlas);
    }
    
    renderer->update();
}
void App::run(){
    while(running){
        proccess_events();
        loop();
        render();
        sleep(0.03);
    }
}
void App::add_mode(){
    mode = Mode::ADD;
}
void App::refresh(){
    mode = Mode::NORMAL;
}
void App::home(){
    mode = Mode::NORMAL;
}
void App::move_mode(){
    mode = Mode::MOVE;
}

App::~App(){
    SDL_Quit();
} 
