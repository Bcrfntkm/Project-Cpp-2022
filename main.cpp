#include "graphics/graphics.h"
int main(){
   App app(SDL_Rect({100,100, 1000, 800}), VirtualFrame(-1,0.8,2,1.6));
   app.run();
   return 0;
}

