#pragma once
#include "player_original.hpp"
#include "original_presentation.hpp"
#include <memory>
namespace actua {
void step_original_exit_officials(original::PlayerEngine&);
// Presentation copy: original routing/actions never advance the live match.
class OriginalExit {
public:
 void start(const original::PlayerEngine&,float tunnel_x,float tunnel_y);
 void advance(double seconds);
 void finish(){engine.reset();clock_=0;paused=false;}
 bool active()const{return engine!=nullptr;}
 int remaining()const;
 int ticks()const{return ticks_;}
 float alpha()const{return paused?1.0f:float(clock_);}
 bool paused=false;
 std::unique_ptr<original::PlayerEngine> engine;
 OriginalPresentation presentation;
private:
 double clock_=0;
 int ticks_=0;
};
}
