#include "player.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using actua::PlayerPractice;
static int checks=0;
static void expect(bool v,const char* m){++checks;if(!v)throw std::runtime_error(m);}
static void clear_defenders(PlayerPractice& p){
    p.engine.teams[12].tm_x=1200;p.engine.teams[12].tm_y=50;
    p.engine.teams[13].tm_x=1200;p.engine.teams[13].tm_y=750;
}
static void receive(PlayerPractice& p,float x,float y){
    for(int i=0;i<200 && !p.engine.ball_poss && !p.outside;++i){
        clear_defenders(p);p.set_input(x,y);p.step();
    }
}
int main(){try{
    // Keep the stick/key held from the pass through the catch. Both modes,
    // pass types, analog strengths and eight directions must flow into a dribble.
    for(bool opponents:{false,true}) for(bool loft:{false,true})
    for(float strength:{0.35f,1.0f}) for(int direction=0;direction<8;++direction){
        PlayerPractice p;p.set_opponents(opponents);
        const float angle=float(direction)*3.1415926536f/4;
        const float x=std::cos(angle)*strength,y=std::sin(angle)*strength;
        p.set_input(x,y);for(int i=0;i<5;++i){clear_defenders(p);p.step();}
        expect(p.pass(loft),"Moving pass starts");receive(p,x,y);
        expect(p.engine.ball_poss==3,"Held movement preserves reception");
        const auto before=p.engine.teams[2];
        // Compare with the normal manual release/repress path at the same pose.
        // The restored engine legitimately slows a player turning sharply.
        PlayerPractice manual=p;manual.set_input(0,0);manual.set_input(x,y);
        clear_defenders(manual);manual.step();const auto& expected=manual.engine.teams[2];
        // No extra input poll: catch must already have handed held movement back.
        clear_defenders(p);p.step();const auto& after=p.engine.teams[2];
        expect(std::hypot(after.tm_x-before.tm_x,after.tm_y-before.tm_y)>0.01f &&
            std::hypot(after.tm_x-expected.tm_x,after.tm_y-expected.tm_y)<0.001f,
            "First tick after possession moves exactly like fresh manual input");
        expect(p.engine.ball_poss==3,"First dribble retains possession");
    }
    // A one-touch return before the next input poll must guard the next receiver.
    PlayerPractice p;p.set_input(0,1);p.step();
    for(int exchange=0;exchange<6;++exchange){
        const int receiver=p.selected==1?3:2;
        expect(p.pass(exchange%2!=0),"Immediate return pass starts");
        receive(p,0,1);expect(p.engine.ball_poss==receiver,"One-touch returns retain in-flight protection");
    }
    // Releasing during flight means stop after the catch, with no stale carry.
    p.reset();p.set_input(0,1);p.step();p.pass();receive(p,0,0);
    expect(p.engine.ball_poss==3,"Neutral stick still permits reception");
    const auto neutral=p.engine.teams[2];p.step();
    expect(p.engine.teams[2].tm_x==neutral.tm_x && p.engine.teams[2].tm_y==neutral.tm_y,
        "Neutral input does not resurrect pre-pass movement");
    // Steering without passing through neutral queues the latest direction.
    p.reset();p.set_input(0,1);p.step();p.pass();receive(p,-0.5f,0);
    expect(p.engine.ball_poss==3,"Changing held direction during flight preserves reception");
    expect(p.input_x==-0.5f && p.input_y==0,"Catch uses latest held direction and analog strength");
    p.reset();p.step();expect(p.engine.teams[1].tm_x==500,"Reset clears queued movement");
    std::cout<<"Passed "<<checks<<" receiver-flow checks.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
