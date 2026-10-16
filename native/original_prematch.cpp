#include "original_prematch.hpp"
#include <algorithm>
#include <cmath>
namespace actua {
void OriginalPrematch::start(const original::PlayerEngine& source){
 finish();engine=std::make_unique<original::PlayerEngine>(source);auto& e=*engine;
 init_original_lineup(e);
 e.ballx=float(e.cntspot_x);e.bally=float(e.cntspot_y);e.ballz=e.ball_diam/2;
 e.ball_poss=e.ball_limbo_on=e.ball_in_hands=0;
 ticks_=0;presentation=OriginalPresentation{};presentation.observe(e,1,0);
 previous_=current_=original_lineup_camera(e,180);
}
void OriginalPrematch::advance(double seconds){
 if(!active() || paused || !std::isfinite(seconds))return;
 clock_+=std::clamp(seconds,0.0,.1)*20;
 while(clock_>=1 && active()){
  animate_original_lineup(*engine);++ticks_;clock_-=1;
  for(auto& p:engine->teams)p.tm_newanim=0; // Allow native subframe sampling of the unchanged stand clip.
  presentation.observe(*engine,1,ticks_);
  previous_=current_;current_=original_lineup_camera(*engine,180-ticks_);
  if(ticks_==180)finish();
 }
}
CameraPose OriginalPrematch::camera()const{
 const auto lerp=[&](float a,float b){return a+(b-a)*alpha();};
 return {lerp(previous_.x,current_.x),lerp(previous_.y,current_.y),lerp(previous_.z,current_.z),
  current_.tx,current_.ty,current_.tz,current_.scale};
}
}
