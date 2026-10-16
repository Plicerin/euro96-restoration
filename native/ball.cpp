#include "ball.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace actua::original {
BallEngine::BallEngine() {
    // FOOTBALL.OBJ startup routine: integer division precedes conversion to float.
    zone_wid=float(pitch_len/7);
    zone_hgt=float(pitch_wid/3);
    // FOOTBALL.CPP match() overrides glob_var.h's 0.5 default.
    gravity=0.60f;
    ballx=float(cntspot_x); bally=float(cntspot_y); ballz=ball_diam/2;
    ball_still=1;
    get_ball_zone();
}
}
namespace actua {
BallSandbox::BallSandbox() { reset(); }
void BallSandbox::reset() {
    engine=original::BallEngine{};
    ticks=0; outside=false; trail_size=0;
    trail[trail_size++]={engine.ballx,engine.bally,engine.ballz};
}
const char* BallSandbox::preset_name() const {
    static constexpr const char* names[]{"Ground pass","Lofted shot","High lob"};
    return names[std::clamp(preset,0,2)];
}
void BallSandbox::launch() {
    const bool wind=engine.wind_on!=0;
    reset();
    if(wind) toggle_wind();
    const float elevation=preset==0?0.0f:(preset==1?25.0f:55.0f);
    const float radians=elevation*3.1415926536f/180.0f;
    engine.get_xydis_from_ang(aim,power*std::cos(radians));
    engine.get_zdis_from_ang(elevation,power);
    engine.ball_inair=preset!=0;
    engine.spin_cnt=0;
    engine.full_xyspin=0.055f; // Sandbox input, not a recovered player kick strength.
    engine.full_zspin=0.015f;
    engine.swerve=preset?std::clamp(curve,-1,1):0;
    engine.shot_pending=1;
    engine.shoot_x=std::cos(aim*3.1415926536f/180.0f);
    engine.shoot_y=-std::sin(aim*3.1415926536f/180.0f);
    engine.launch_ball(1);
    engine.ball_still=0;
}
void BallSandbox::step() {
    if(paused || outside) return;
    engine.ball_trajectory();
    ++ticks;
    if(!std::isfinite(engine.ballx) || !std::isfinite(engine.bally) || !std::isfinite(engine.ballz))
        throw std::runtime_error("Non-finite state in restored ball engine");
    outside=engine.ballx<0 || engine.ballx>float(engine.pitch_len) || engine.bally<0 || engine.bally>float(engine.pitch_wid);
    // Match boundary and restart rules are not yet restored. Preserve the last valid zone.
    if(!outside) engine.get_ball_zone();
    if(trail_size<trail.size()) trail[trail_size++]={engine.ballx,engine.bally,engine.ballz};
    else {
        std::move(trail.begin()+1,trail.end(),trail.begin());
        trail.back()={engine.ballx,engine.bally,engine.ballz};
    }
}
void BallSandbox::turn(float degrees) { aim=std::fmod(aim+degrees+360.0f,360.0f); }
void BallSandbox::change_power(float amount) { power=std::clamp(power+amount,3.0f,24.0f); }
void BallSandbox::aim_at(float x,float y) {
    const float dx=x-float(engine.cntspot_x),dy=y-float(engine.cntspot_y);
    if(dx*dx+dy*dy>1) aim=std::atan2(-dy,dx)*180.0f/3.1415926536f;
}
void BallSandbox::set_aftertouch(float x,float y) {
    // Caller inputs are confined to the first of the preserved 20 user slots.
    const float length=std::sqrt(x*x+y*y);
    if(length>1) { x/=length; y/=length; }
    engine.users_dir[0].x=x; engine.users_dir[0].y=y;
    engine.after_touch_on=(engine.ball_inair && (x!=0 || y!=0))?1:0;
}
void BallSandbox::toggle_wind() {
    engine.wind_on=engine.wind_on?0:1;
    engine.wind_speed=45; engine.wind_x=0; engine.wind_y=-1;
}
int FixedStep::advance(double seconds,bool active) {
    if(!active) { remainder=0; return 0; }
    if(!std::isfinite(seconds) || seconds<0) return 0;
    remainder+=std::min(seconds,0.25);
    const int count=std::min(5,int((remainder+1e-10)/0.05));
    remainder-=double(count)*0.05;
    return count;
}
}
