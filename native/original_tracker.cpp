#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "action_constants.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace actua {
bool OriginalGraphics::tracker_active()const{
 const auto& s=*sequence;const auto& e=s.engine;
 return action_view && !exit.active() && !prematch.active() && !walkoff.active() && !replaying && (s.human_restart() || (s.penalty_human_taker() && s.penalty_aiming())) &&
     e.set_piece_on!=original::action_constants::SETP_THROW && e.dead_ball_cnt<=1 && e.setp_taker>0 &&
     e.ball_poss==e.setp_taker && !e.ball_limbo_on && e.teams[e.setp_taker-1].int_move!=15;
}
void OriginalGraphics::update_tracker(){
 tracker_points_.clear();tracker_markers=0;
 if(!tracker_active()){tracker_tick=-1;return;}
 const auto& s=*sequence;const auto& e=s.engine;
 if(!tracker_engine)tracker_engine=std::make_unique<original::PlayerEngine>();
 if(tracker_tick!=s.ticks || tracker_generation!=s.generation || tracker_engine->setp_power!=e.setp_power || tracker_engine->setp_hgt!=e.setp_hgt || tracker_engine->ballx!=e.ballx || tracker_engine->bally!=e.bally || tracker_engine->ballz!=e.ballz){
     *tracker_engine=e;tracker_tick=s.ticks;tracker_generation=s.generation;
 }
 auto& preview=*tracker_engine;preview.tracker_sprites.clear();
 const bool penalty=e.set_piece_on==original::action_constants::SETP_PENALTY;
 const short minimum=short(penalty?e.setp_power/2:((e.teams[e.setp_taker-1].tm_pow/6)+8)/2);
 // Original tracker advances 0.2 each logic tick, never on paint count. Show the
 // minimum-power trajectory too, so the player can aim before increasing power.
 const float phase=std::fmod((float(s.ticks%5)+float(clock))*.2f,1.0f);
 preview.draw_tracker(minimum,e.setp_kick_x,e.setp_kick_y,phase);
 tracker_points_=preview.tracker_sprites;
}
void OriginalGraphics::add_tracker_sprites(bool shadows){
 std::vector<float> points(1,0);std::vector<legacy::word> refs;
 for(const auto& p:tracker_points_)if((p.ref!=0)==shadows){
     points.insert(points.end(),{float(p.x),float(p.z),-float(p.y)});refs.push_back(legacy::word(p.ref));
 }
 if(refs.empty())return;
 points[0]=float(refs.size());
 if(core->curobj+refs.size()>core->objects.data()+core->objects.size() || core->curpol+refs.size()>core->polygons.data()+core->polygons.size())throw std::runtime_error("Tracker sprite capacity exceeded");
 core->addobjsp(points.data(),refs.data());tracker_markers+=int(refs.size());
}
}
