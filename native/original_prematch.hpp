#pragma once
#include "original_camera.hpp"
#include "original_presentation.hpp"
#include <memory>
namespace actua {
void init_original_lineup(original::PlayerEngine&);
void animate_original_lineup(original::PlayerEngine&);
CameraPose original_lineup_camera(const original::PlayerEngine&,int remaining);
class OriginalPrematch {
public:
 void start(const original::PlayerEngine&);
 void advance(double seconds);
 void finish(){engine.reset();clock_=0;paused=false;}
 bool active()const{return engine!=nullptr;}
 int ticks()const{return ticks_;}
 float alpha()const{return paused?1.0f:float(clock_);}
 CameraPose camera()const;
 bool paused=false;
 std::unique_ptr<original::PlayerEngine> engine;
 OriginalPresentation presentation;
private:
 double clock_=0;
 int ticks_=0;
 CameraPose previous_{},current_{};
};
}
