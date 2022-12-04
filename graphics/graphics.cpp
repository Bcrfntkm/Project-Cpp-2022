#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cmath>

std::array<SDL_Color, 6> COLORS = {SDL_Color({44, 93, 55}), SDL_Color({227, 197, 21}), SDL_Color({238, 81, 177}), 
                                     SDL_Color({165, 156, 211}), SDL_Color({75, 45, 159}), SDL_Color({192, 168, 183}),
                                    };

DPoint::DPoint(double x, double y):x(x), y(y){}
DPoint::DPoint():x(0), y(0){}

VirtualFrame::VirtualFrame(double x, double y, double w, double h):x(x), y(y), w(w), h(h){}
VirtualFrame::VirtualFrame(SDL_Rect frame){
    x = -1;
    y = static_cast<double>(frame.h) / frame.w / 2;
    w = 2;
    h = static_cast<double>(frame.h) / frame.w;
}

DPoint VirtualFrame::to_virtual(SDL_Point p, SDL_Rect &frame){
    double virt_p_x = static_cast<double>(p.x)/frame.w*w + x;
    double virt_p_y = - static_cast<double>(p.y)/frame.h*h + y;
    return DPoint(virt_p_x, virt_p_y);
}
SDL_Point VirtualFrame::to_SDL(DPoint p, SDL_Rect &frame){
    int SDL_p_x = (p.x - x) / w * frame.w; 
    int SDL_p_y = (y - p.y) / h * frame.h; 
    return SDL_Point({SDL_p_x, SDL_p_y});
}
VirtualFrame VirtualFrame::zoom(SDL_Point start, SDL_Point end, SDL_Rect &frame){
    DPoint v_start =to_virtual(start, frame);
    DPoint v_end = to_virtual(end, frame);
    double n_x = std::min(v_start.x, v_end.x);
    double n_y = std::max(v_start.y, v_end.y);
    double n_w = std::abs(v_start.x -  v_end.x);
    double n_h = (n_w * frame.h) / frame.w;
    return VirtualFrame(n_x, n_y, n_w, n_h);
}
DPoint VirtualFrame::get_top_left(){
    return DPoint(x, y);
}
DPoint VirtualFrame::get_bottom_right(){
    return DPoint(x + w, y - h);
}

Window::Window(SDL_Rect& frame){
    win_ptr = SDL_CreateWindow("", frame.x, frame.y, frame.w, frame.h, SDL_WINDOW_SHOWN);
}
Window::Window(Window &&src):win_ptr(src.win_ptr){
    src.win_ptr = nullptr;
}
Window& Window::operator=(Window &&rhs){
    Window tmp(std::move(rhs));
    std::swap(tmp.win_ptr, win_ptr);
    return *this;
}
Window::operator bool() const {
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
Renderer::Renderer(Renderer &&src):render_ptr(src.render_ptr){
    render_ptr = nullptr;
}
Renderer& Renderer::operator=(Renderer &&rhs){
    Renderer tmp(std::move(rhs));
    std::swap(tmp.render_ptr, render_ptr);
    return *this;
}
Renderer::operator bool() const{
    return render_ptr != nullptr;
}
void Renderer::draw_point(SDL_Color color, SDL_Point p){
    SDL_SetRenderDrawColor(render_ptr, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(render_ptr, p.x, p.y);
}
void Renderer::fill_rect(SDL_Color color, SDL_Rect rect){
    SDL_SetRenderDrawColor(render_ptr, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(render_ptr, &rect);
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

SafeTexture::SafeTexture(SDL_Surface *surf, Renderer &renderer):texture(nullptr), surf(surf){
    if (surf != nullptr){
        texture = SDL_CreateTextureFromSurface(renderer.get(), surf);
    }
}
SafeTexture::SafeTexture(SDL_Surface *surf, Renderer &renderer, SDL_Color key):texture(nullptr){
    if (surf != nullptr){
        SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format, key.r, key.g, key.b));
        texture = SDL_CreateTextureFromSurface(renderer.get(), surf);
    }
}
SafeTexture::SafeTexture(SafeTexture &&src): surf(src.surf), texture(src.texture){
    src.surf = nullptr;
    src.texture = nullptr;
}
SafeTexture& SafeTexture::operator=(SafeTexture &&rhs){
    SafeTexture tmp(std::move(rhs));
    std::swap(rhs.surf, surf);
    std::swap(rhs.texture, texture);
    return *this;
}
SafeTexture::operator bool() const {
    return texture != nullptr;
}
SDL_Texture* SafeTexture::get_texture(){
    return texture;
}
SDL_Surface* SafeTexture::get_surf(){
    return surf;
}
void SafeTexture::draw_point(SDL_Point p, SDL_Color color){
    SDL_Rect fake_rect = SDL_Rect({p.x, p.y, 1, 1});
    SDL_FillRect(surf, &fake_rect, 
                 SDL_MapRGB(surf->format, color.r, color.g, color.b));

}
void SafeTexture::update(Renderer &renderer){
     if(texture != nullptr)
        SDL_DestroyTexture(texture);
     texture = SDL_CreateTextureFromSurface(renderer.get(), surf);
}

SafeTexture::~SafeTexture(){
    if(texture != nullptr)
        SDL_DestroyTexture(texture);
    if(surf != nullptr)
        SDL_FreeSurface(surf);
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
        SDL_RenderCopy(renderer.get(), texture_atlas.get_texture(), &off_texture, &rect);
    }else if (state == HOVER){
        SDL_RenderCopy(renderer.get(), texture_atlas.get_texture(), &hover_texture, &rect);
    }else if (state == ON){
        SDL_RenderCopy(renderer.get(), texture_atlas.get_texture(), &on_texture, &rect);
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


Root::Root(SDL_Rect off_texture, SDL_Rect hover_texture, SDL_Rect rect, unsigned indx): 
    off_texture(off_texture), hover_texture(hover_texture), rect(rect), state(RootState::SHOWN), click_timer(0), indx(indx){}
unsigned Root::get_indx(){return indx;}
SDL_Point Root::get_centre(){return SDL_Point({rect.x + 10, rect.y + 10});}
void Root::pick(){state = RootState::HIDDEN; click_timer = 3;}
void Root::release(SDL_Point p){state = RootState::SHOWN; move(p);}
void Root::move(SDL_Point p){rect.x = p.x - 10; rect.y = p.y - 10;}
void Root::hover(){state = RootState::HOVER;}
void Root::off(){state = RootState::SHOWN;}
bool Root::is_hidden(){return state == RootState::HIDDEN;}
bool Root::can_be_released(){return click_timer == 0;}
void Root::draw(Renderer &renderer, SafeTexture &texture_atlas){
    if (state == SHOWN){
        SDL_RenderCopy(renderer.get(), texture_atlas.get_texture(), &off_texture, &rect);
    }else if (state == HOVER){
        SDL_RenderCopy(renderer.get(), texture_atlas.get_texture(), &hover_texture, &rect);
    }
}
void Root::update(){
    if (click_timer > 0){
        click_timer --;
    }
}
SDL_Rect const & Root::get_rect(){return rect;}

SelectBox::SelectBox(SDL_Color color): color(color), rect({0,0,0,0}), start({0,0}), end({0,0}),
                                            active(false){}
void SelectBox::start_at(SDL_Point p){
    start = p;
    active = true;
}
void SelectBox::end_at(SDL_Point p){
    end = p;
    rect.x = std::min(start.x, end.x); 
    rect.y = std::min(start.y, end.y);
    rect.w = std::abs(start.x - end.x);
    rect.h = std::abs(start.y - end.y);
}
SDL_Point SelectBox::get_start(){
    return start;
}
SDL_Point SelectBox::get_end(){
    return end;
}
void SelectBox::draw(Renderer &renderer){
    renderer.fill_rect(color, rect);
}
void SelectBox::release(){
    rect = {0,0,0,0};
    start = {0,0};
    end = {0,0};
    active = false;
}
SDL_Rect const & SelectBox::get_rect(){return rect;}

App::App(SDL_Rect frame, VirtualFrame virt_frame):frame(frame), virtual_frame(virt_frame), 
         newton(std::pair<double, double>(virt_frame.get_top_left().x, virt_frame.get_top_left().y), 
                 std::pair<double, double>(virt_frame.get_bottom_right().x, virt_frame.get_bottom_right().y)),
         running(false), mode(NORMAL), select(SDL_Color({164, 197, 250, 200})) 
    {
    if(SDL_Init(SDL_INIT_VIDEO) == 0){
        win.reset(new Window(frame));
        if (win){
            renderer.reset(new Renderer(*win.get()));
            if (renderer){
                SDL_Surface* loaded_bmp = SDL_LoadBMP("resources/texture_atlas.bmp");
                texture_atlas.reset(new SafeTexture(loaded_bmp, *renderer,SDL_Color({255, 0, 255, 255})));
                if (texture_atlas){

                    buttons[0].reset(new Button(
                            SDL_Rect({0,0,100,100}), SDL_Rect({100,0,100,100}), SDL_Rect({200,0,100,100}), 
                            SDL_Rect({10,10,60,60}), *this, &App::home));
                    buttons[1].reset(new Button(
                            SDL_Rect({0,100,100,100}), SDL_Rect({100,100,100,100}), SDL_Rect({200,100,100,100}), 
                            SDL_Rect({10,75,60,60}), *this, &App::refresh));
                    buttons[2].reset(new Button(
                            SDL_Rect({0,200,100,100}), SDL_Rect({100,200,100,100}), SDL_Rect({200,200,100,100}), 
                            SDL_Rect({10,140,60,60}), *this, &App::add_mode));
                    buttons[3].reset(new Button(
                            SDL_Rect({0,300,100,100}), SDL_Rect({100,300,100,100}), SDL_Rect({200,300,100,100}), 
                            SDL_Rect({10,205,60,60}), *this, &App::move_mode));
                    draw_map_dims = newton.get_dimensions();
                    background.reset(new SafeTexture(
                                         SDL_CreateRGBSurface(0, draw_map_dims.first, draw_map_dims.second, 32, 0, 0, 0, 0), *renderer));
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
            bool button_pressed = false;
            for (auto it = buttons.begin(); it != buttons.end(); ++it){
                if (SDL_PointInRect(&mouse_pos, &((*it)->get_rect()))){
                    (*it)->press();
                    button_pressed = true;
                }
            }
            if (!button_pressed){
                if (mode == Mode::NORMAL){
                    if (!select.is_active()){
                        select.start_at(mouse_pos);
                    }
                }else if (mode == Mode::ADD){
                    create_root(mouse_pos);
                }

            }


        }else if(event.type == SDL_MOUSEBUTTONUP){
            SDL_Point mouse_pos({event.button.x, event.button.y});

            for (auto it = buttons.begin(); it != buttons.end(); ++it){
                if (SDL_PointInRect(&mouse_pos, &((*it)->get_rect())))
                (*it)->release();
            }

            if (mode == Mode::NORMAL){
                if (select.is_active()){
                    zoom(select.get_start(), select.get_end());
                    select.release();

                }
            }
            else if (mode == Mode::MOVE){
                if (moving_root){
                    if (moving_root->can_be_released()){
                        end_move_root(mouse_pos);

                    }            
                }else{
                    for (auto it = roots.begin(); it != roots.end(); ++it){
                        if (SDL_PointInRect(&mouse_pos, &((*it)->get_rect())))
                            start_move_root(*it);
                    }
                }
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
            if (mode == Mode::NORMAL){
                if (select.is_active()){
                    select.end_at(mouse_pos);
                }
            }
            else if (mode == Mode::MOVE){
                for (auto it = roots.begin(); it != roots.end(); ++it){
                    if ((*it)->is_hidden())
                        continue;

                    if (SDL_PointInRect(&mouse_pos, &((*it)->get_rect())))
                        (*it)->hover();
                    else
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
    for (auto it = roots.begin(); it != roots.end(); ++it){
        (*it)->update();
    }
    if (moving_root && mode != Mode::MOVE){
        moving_root->off();
        moving_root.reset();
    }
}
void App::render(){
    SDL_RenderCopy(renderer->get(), background->get_texture(), NULL, NULL);

    for (auto it = roots.begin(); it != roots.end(); ++it)
        (*it)->draw(*renderer, *texture_atlas);
    for (auto it = buttons.begin(); it != buttons.end(); ++it){
        (*it)->draw(*renderer, *texture_atlas);
    }
    select.draw(*renderer);
    
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
    if (!draw_map.empty()){
        draw_map.clear();
    }
    newton.method(draw_map);
    for (int i = 0; i < draw_map_dims.first; ++i){
        for (int j = 0; j < draw_map_dims.second; ++j){
            int color_key = draw_map[ i * draw_map_dims.second + draw_map_dims.second - 1 - j];
             background->draw_point(SDL_Point({i, j}), COLORS[color_key]);
        }
    }
    background->update(*renderer);
    mode = Mode::NORMAL;
}
void App::home(){
    mode = Mode::NORMAL;
    VirtualFrame new_virt_frame(-1,1,2,2);
    for (auto it = roots.begin(); it != roots.end(); ++it){
        (*it)->move(new_virt_frame.to_SDL(virtual_frame.to_virtual((*it)->get_centre(), frame),frame));
    }
    virtual_frame = new_virt_frame;
    newton.zoom(std::make_pair(-1,1), std::make_pair(1,-1));
    refresh();
}
void App::move_mode(){
    mode = Mode::MOVE;
}
void App::start_move_root(std::shared_ptr<Root> root){
    moving_root = root;
    root->pick();
}
void App::end_move_root(SDL_Point p){
    DPoint virt_new_root = virtual_frame.to_virtual(p, frame);
    newton.move_root(moving_root->get_indx(),
                     std::make_pair(virt_new_root.x, virt_new_root.y));
    moving_root->release(p);
    moving_root.reset();
}
void App::create_root(SDL_Point p){
    if (roots.size() < COLORS.size()){
        DPoint virt_root = virtual_frame.to_virtual(SDL_Point({p.x - 10, p.y - 10}), frame);
            newton.get_root(virt_root.x, virt_root.y, roots.size());
            roots.push_back(std::shared_ptr<Root>(new Root(SDL_Rect({0,400,20,20}), 
                                         SDL_Rect({20,400,20,20}), SDL_Rect({p.x - 10, p.y - 10, 20, 20}),roots.size()))); 

    }
}
void App::zoom(SDL_Point start, SDL_Point end){
    VirtualFrame new_virt_frame = virtual_frame.zoom(start, end, frame);
    for (auto it = roots.begin(); it != roots.end(); ++it){
        (*it)->move(new_virt_frame.to_SDL(virtual_frame.to_virtual((*it)->get_centre(), frame),frame));
    }
    virtual_frame = new_virt_frame;
    DPoint v_top_left = virtual_frame.get_top_left();
    DPoint v_bottom_right = virtual_frame.get_bottom_right();
    newton.zoom(std::make_pair(v_top_left.x, v_top_left.y), std::make_pair(v_bottom_right.x, v_bottom_right.y));
    refresh();
}
App::~App(){
    SDL_Quit();
} 
