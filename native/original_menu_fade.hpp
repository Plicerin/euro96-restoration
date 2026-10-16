#pragma once
#include "menu_fade_data.hpp"
#include <algorithm>
#include <cmath>
namespace actua {
// EURO_GEN's linear palette fade-down/up, applied to composed menu textures.
// TEST.CPP registers EUROinterupt at 63 Hz, independent of display refresh.
class OriginalMenuFade {
public:
 bool page(int value,bool previous_frame=true){if(value==page_)return false;clock_=page_>=0 && previous_frame?0:original_menu_fade_steps;page_=value;return true;}
 void reset(){page_=-1;finish();}
 void finish(){clock_=original_menu_fade_steps*2;}
 void advance(double seconds){if(page_>=0 && std::isfinite(seconds))clock_=std::min(double(original_menu_fade_steps*2),clock_+std::clamp(seconds,0.0,.1)*original_menu_fade_hz);}
 int step()const{return std::clamp(int(clock_+1e-8),0,original_menu_fade_steps*2);}
 bool outgoing()const{return page_>=0 && step()<original_menu_fade_steps;}
 int page_id()const{return page_;}
 unsigned char shade()const{const int phase=outgoing()?step():original_menu_fade_steps*2-step();return static_cast<unsigned char>(255*phase/original_menu_fade_steps);}
private:
 int page_=-1;
 double clock_=original_menu_fade_steps*2;
};
}
