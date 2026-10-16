#include "player.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
using actua::PlayerPractice;
static int checks=0;
static void expect(bool v,const char* m){++checks;if(!v)throw std::runtime_error(m);}
int main(int argc,char** argv){try{
    std::ofstream trace;if(argc>1){trace.open(argv[1]);if(!trace)throw std::runtime_error("Cannot write movement trace");trace<<"scenario,tick,x,y,ballx,bally\n";}
    int worst=0;
    for(int scenario=0;scenario<5;++scenario){
        PlayerPractice p;p.opponent_shooting=false;p.set_opponents(true);p.engine.holder_lose_ball();
        auto& r=p.engine.teams[12];
        r.tm_x=scenario==4?640.0f:(scenario%2?1100.0f:180.0f);
        r.tm_y=scenario==4?400.0f:(scenario<2?150.0f:650.0f);
        p.engine.teams[1].tm_x=640;p.engine.teams[1].tm_y=400;
        p.engine.teams[2].tm_x=740;p.engine.teams[2].tm_y=400;
        p.engine.collect_ball(&r);
        p.engine.teams[13].guy_on=0; // Isolate the existing dribble-path contract.
        float heading=std::atan2(r.tm_ydis,r.tm_xdis);int previous_turn=0,reversals=0;
        for(int tick=0;tick<600;++tick){
            p.step();const float now=std::atan2(r.tm_ydis,r.tm_xdis);
            const float delta=std::remainder(now-heading,6.283185307f);heading=now;
            // Count substantial left/right steering reversals, ignoring float noise.
            if(std::abs(delta)>0.04f){const int turn=delta>0?1:-1;if(previous_turn && turn!=previous_turn)++reversals;previous_turn=turn;}
            if(trace)trace<<scenario<<','<<tick<<','<<r.tm_x<<','<<r.tm_y<<','<<p.engine.ballx<<','<<p.engine.bally<<'\n';
            expect(p.engine.ball_poss==13 && !p.restart_ticks && r.tm_x>=12 && r.tm_x<=1268 && r.tm_y>=12 && r.tm_y<=788,"Extended dribble stays in play");
        }
        std::cout<<"Scenario "<<scenario<<": "<<reversals<<" steering reversals in 30 seconds.\n";
        if(reversals>worst)worst=reversals;
    }
    std::cout<<"Worst reversal count: "<<worst<<"\n";
    if(argc==1)expect(worst<=20,"Opponent must not repeatedly flip steering near a boundary");
    // Tiny distance changes must not ping-pong the defensive roles.
    PlayerPractice roles;roles.set_opponents(true);
    roles.engine.teams[1].tm_x=640;roles.engine.teams[1].tm_y=400;
    for(int tick=0;tick<24;++tick){
        roles.engine.teams[12].tm_x=640;roles.engine.teams[12].tm_y=280;
        roles.engine.teams[13].tm_x=640;roles.engine.teams[13].tm_y=520;
        roles.engine.ballx=640;roles.engine.bally=tick%2?401.0f:399.0f;
        roles.step();
        expect(std::abs(roles.engine.teams[12].go_tx-640)<5,"Nearly equal distances retain the current presser");
    }
    roles.engine.teams[12].tm_x=1000;roles.engine.teams[12].tm_y=100;
    roles.engine.teams[13].tm_x=650;roles.engine.teams[13].tm_y=420;
    roles.engine.ballx=640;roles.engine.bally=400;roles.step();
    expect(roles.engine.teams[13].go_tx<650,"Clearly closer defender can take over pressing");
    // Crossing the centre lines must not flip the support target across the holder.
    PlayerPractice support;support.set_opponents(true);support.engine.holder_lose_ball();
    support.engine.teams[12].tm_x=639;support.engine.teams[12].tm_y=399;
    support.engine.collect_ball(&support.engine.teams[12]);float last_heading=0;
    for(int tick=0;tick<24;++tick){
        support.engine.teams[12].tm_x=tick%2?641.0f:639.0f;
        support.engine.teams[12].tm_y=tick%2?401.0f:399.0f;
        support.engine.teams[13].tm_x=900;support.engine.teams[13].tm_y=600;
        support.step();const auto& runner=support.engine.teams[13];
        const float heading=std::atan2(runner.go_ty-600,runner.go_tx-900);
        if(tick)expect(std::abs(std::remainder(heading-last_heading,6.283185307f))<0.06f,
            "Midfield crossings preserve the support runner's target side");
        last_heading=heading;
    }
    std::cout<<"Passed "<<checks<<" opponent motion checks.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
