#include "original_walkoff.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace actua {
void OriginalWalkoff::start(const original::PlayerEngine& live,const OriginalActions::WalkoffEvent& event,float x,float y){
 finish();event_=event;ticks_=0;incoming_=timed_out_=false;
 engine=std::make_unique<original::PlayerEngine>(live);auto& e=*engine;
 e.tunnel_x=x;e.tunnel_y=y;e.ball_poss=e.ball_limbo_on=e.ball_in_hands=0;
 e.just_scored=e.shot_pending=0;e.player_num=short(event.index+1);
 e.user_controlled=e.uf1=e.uf2=e.um=e.must_face=e.stop_and_face=e.sender_1_2=0;e.match_mode=0;
 e.player_on_off=short(event.index+1);e.player_being_subbed=event.replacement?e.player_on_off:0;
 auto& p=e.teams.at(event.index);p=event.outgoing;p.guy_on=p.draw_me=1;p.control=0;
 p.tm_limbo=p.tm_strike=p.tm_trap=0;p.tm_z=p.tm_zdis=0;e.reset_ideas(&p);e.init_stand_act(&p);
 e.init_off_int(&p);
 presentation=OriginalPresentation{};presentation.observe(e,1,0);
 previous_=current_=original_camera(17,e,0,0,0,{});
}
void OriginalWalkoff::arrive(){
 auto& e=*engine;auto& p=e.teams[event_.index];
 if(!incoming_ && !p.guy_on){
  if(!event_.replacement){finish();return;}
  // FOOTBALL.CPP process_subs: replace identity only after reaching the tunnel,
  // retain that position, reverse player_on_off and run the same route back.
  const auto x=p.tm_x,y=p.tm_y;p=event_.incoming;p.tm_x=x;p.tm_y=y;p.tm_z=0;
  p.guy_on=p.draw_me=1;p.control=0;p.tm_trap=p.tm_limbo=p.tm_strike=0;
  e.reset_ideas(&p);e.init_stand_act(&p);e.player_being_subbed=0;
  e.player_on_off=short(-event_.index-1);incoming_=true;e.init_off_int(&p);
  presentation=OriginalPresentation{};presentation.observe(e,1,ticks_);
 }else if(incoming_ && !e.player_on_off)finish();
}
void OriginalWalkoff::advance(double seconds){
 if(!active() || paused || !std::isfinite(seconds))return;
 clock_+=std::clamp(seconds,0.0,.1)*20;
 while(clock_>=1 && active()){
  auto& e=*engine;auto& p=e.teams[event_.index];
  if(p.guy_on){
   if(p.tm_act==0)e.init_off_int(&p);
   if(p.guy_on && e.player_on_off){e.process_anims(&p);if(p.tm_act==1)e.run_action(&p);e.process_dir(&p);}
   if(!std::isfinite(p.tm_x) || !std::isfinite(p.tm_y))throw std::runtime_error("Non-finite original walk-off");
  }
  ++ticks_;clock_-=1;arrive();
  if(!active())return;
  presentation.observe(e,1,ticks_);previous_=current_;
  current_=original_camera(17,e,0,0,0,current_);
  if(ticks_>=1200){timed_out_=true;finish();}
 }
}
CameraPose OriginalWalkoff::camera()const{
 const auto lerp=[&](float a,float b){return a+(b-a)*alpha();};
 return {lerp(previous_.x,current_.x),lerp(previous_.y,current_.y),lerp(previous_.z,current_.z),
  lerp(previous_.tx,current_.tx),lerp(previous_.ty,current_.ty),lerp(previous_.tz,current_.tz),current_.scale};
}
}
