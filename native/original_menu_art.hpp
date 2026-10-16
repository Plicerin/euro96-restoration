#pragma once
#include "archive.hpp"
#include "menu_art_data.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
namespace actua {
// EURO_SEL's direction and .2/.4/.8/1 frame acceleration, paced by a
// source frontend loop cadence. Country choice remains immediate for input.
class OriginalMenuCarousel {
public:
 void select(int team,bool reset=false){
  const int aim=menu_art::aim[std::clamp(team,0,15)];
  if(target_<0 || reset){target_=aim;counter_=float(aim*4);speed_=0;clock_=0;return;}
  if(aim==target_)return;
  target_=aim;direction_=menu_art::country_direction(counter_,aim);speed_=.2f;clock_=0;
 }
 void advance(double elapsed){
  if(!std::isfinite(elapsed) || elapsed<=0)return;
  clock_+=std::min(elapsed,.25);
  while(clock_+1e-12>=menu_art::country_step_seconds){clock_-=menu_art::country_step_seconds;
   if(!moving())continue;
   if(int(counter_)!=target_*4){counter_+=direction_*speed_;speed_=std::min(speed_*2,1.0f);}
   else{counter_=float(target_*4);speed_=0;}
   counter_=std::fmod(counter_+64,64.0f);
  }
 }
 int frame()const{return int(counter_)&63;}
 bool moving()const{return speed_>0;}
private:
 int target_=-1,direction_=1;float counter_=0,speed_=0;double clock_=0;
};
// Original frontend artwork composited at its authored 640 x 480 resolution.
class OriginalMenuArt {
public:
 enum Ink {white, dark, blue, cyan};
 explicit OriginalMenuArt(const Archive&);
 OriginalMenuArt(const OriginalMenuArt&)=delete;
 OriginalMenuArt& operator=(const OriginalMenuArt&)=delete;
 ~OriginalMenuArt();
 void begin(int screen,int team=-1);
 void clear();
 void present(SDL_Renderer*,SDL_FRect view={48,8,864,648});
 void image(int id,int x,int y,int w=0,int h=0,int team=-1,bool mirror=false);
 void crop(int record,int sx,int sy,int sw,int sh,int x,int y,int w,int h);
 void rgba(const Bytes&,int sw,int sh,int x,int y,int w,int h);
 void panel(int x,int y,int w,int h);
 void bar(int x,int y,int w);
 void text(int x,int y,const std::string&,Ink ink=white,int size=0,int align=0,int limit=640);
 int width(const std::string&,int size=0)const;
 void button(int x,int y,int w,const std::string&,bool selected=false,double clock=0);
 void country(int team);
 void country_frame(int frame);
 const Bytes& pixels()const{return frame_;}
private:
 std::array<Bytes,111> entries_;
 Bytes frame_,upload_,palette_,font_palette_;
 SDL_Texture* texture_=nullptr;SDL_Renderer* owner_=nullptr;
 void blit(int record,int sx,int sy,int sw,int sh,int x,int y,int w,int h,const Bytes&,int offset=0,bool panel_mask=false,bool mirror=false);
};
}
