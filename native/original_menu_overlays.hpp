#pragma once
#include "original_graphics.hpp"
namespace actua {
void draw_menu_match_panels(SDL_Renderer*,const OriginalGraphics&,bool pad);
void draw_menu_pause(SDL_Renderer*,const OriginalGraphics&,bool pad);
void draw_menu_power(SDL_Renderer*,const OriginalGraphics&,float charge);
void draw_menu_hint(SDL_Renderer*,const OriginalGraphics&,const std::string&);
void draw_menu_setup(SDL_Renderer*,const OriginalGraphics&,int row);
void draw_menu_replay(SDL_Renderer*,const OriginalGraphics&,bool pad);
void draw_menu_squad(SDL_Renderer*,const OriginalGraphics&,bool away,int row,int slot,int shirt,const std::string& message);
}
