#pragma once
#include "original_actions.hpp"
#include "original_camera.hpp"
#include "original_presentation.hpp"
#include <memory>
namespace actua {
class OriginalWalkoff {
public:
 void start(const original::PlayerEngine&,const OriginalActions::WalkoffEvent&,float tunnel_x,float tunnel_y);
 void advance(double seconds);
 void finish(){engine.reset();clock_=0;paused=false;}
 bool active()const{return engine!=nullptr;}
 bool incoming()const{return incoming_;}
 bool replacement()const{return event_.replacement;}
 int ticks()const{return ticks_;}
 bool timed_out()const{return timed_out_;}
 float alpha()const{return paused?1.0f:float(clock_);}
 const std::string& label()const{return event_.label;}
 CameraPose camera()const;
 bool paused=false;
 std::unique_ptr<original::PlayerEngine> engine;
 OriginalPresentation presentation;
private:
 void arrive();
 OriginalActions::WalkoffEvent event_{};
 double clock_=0;
 int ticks_=0;
 bool incoming_=false,timed_out_=false;
 CameraPose previous_{},current_{};
};
}
