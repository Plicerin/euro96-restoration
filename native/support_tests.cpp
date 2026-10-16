#include "player.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
using actua::PlayerPractice;
static int checks=0;
static void expect(bool v,const char* m){++checks;if(!v)throw std::runtime_error(m);}
static void clear_defenders(PlayerPractice& p) {
    p.engine.teams[12].tm_x=1200;p.engine.teams[12].tm_y=50;
    p.engine.teams[13].tm_x=1200;p.engine.teams[13].tm_y=750;
}
static float lane_distance(const PlayerPractice& p,int index) {
    const auto& a=p.engine.teams[1];const auto& b=p.engine.teams[2];const auto& d=p.engine.teams[std::size_t(index)];
    const float dx=b.tm_x-a.tm_x,dy=b.tm_y-a.tm_y,len=dx*dx+dy*dy;
    const float t=std::clamp(((d.tm_x-a.tm_x)*dx+(d.tm_y-a.tm_y)*dy)/len,0.0f,1.0f);
    return std::hypot(d.tm_x-a.tm_x-t*dx,d.tm_y-a.tm_y-t*dy);
}
int main(){try{
    PlayerPractice p;p.set_opponents(true);
    for(int i=0;i<8;++i)p.step();
    expect(std::hypot(p.engine.teams[2].tm_x-740,p.engine.teams[2].tm_y-400)>8,"Off-ball teammate offers a moving support option");
    expect(p.engine.teams[1].tm_x==500 && p.engine.teams[1].tm_y==400,"Support AI does not move the controlled carrier");
    p.reset();p.engine.teams[12].tm_x=620;p.engine.teams[12].tm_y=400;p.engine.teams[13].tm_x=760;p.engine.teams[13].tm_y=400;
    for(int i=0;i<12;++i)p.step();
    expect(lane_distance(p,12)>12,"Teammate moves out of a blocked passing lane");
    expect(std::hypot(p.engine.teams[2].tm_x-p.engine.teams[13].tm_x,p.engine.teams[2].tm_y-p.engine.teams[13].tm_y)>30,"Teammate separates from a close marker");
    p.reset();p.switch_player();p.set_input(0,-1);p.step();
    expect(p.engine.teams[2].tm_y<400,"Manual selection and movement override teammate AI");
    expect(p.engine.teams[1].tm_x==500,"Unselected ball holder is not made to run by support AI");
    p.reset();p.paused=true;p.step();expect(p.engine.teams[2].tm_x==740 && p.ticks==0,"Pause freezes support runs");
    // Isolate reception by keeping defenders away; both pass types must find a moving teammate.
    for(bool loft:{false,true}) for(int run_ticks:{4,10,20}) {
        p.reset();
        for(int i=0;i<run_ticks;++i)p.step();
        const float running=std::hypot(p.engine.teams[2].go_txdis,p.engine.teams[2].go_tydis);
        clear_defenders(p);
        const auto target_x=p.engine.teams[2].tm_x,target_y=p.engine.teams[2].tm_y;
        expect(p.pass(loft),"Pass can target a moving support runner");
        for(int i=0;i<150 && !p.engine.ball_poss && !p.outside;++i){clear_defenders(p);p.set_input(0,0);p.step();}
        if(p.engine.ball_poss!=3)std::cerr<<"Moving receiver miss loft="<<loft<<" run="<<run_ticks<<'\n';
        expect(p.engine.ball_poss==3,"Moving support runner meets the incoming pass");
        expect(running<0.1f || std::hypot(p.engine.teams[2].tm_x-target_x,p.engine.teams[2].tm_y-target_y)>1,"A running receiver continues to meet the pass");
    }
    p.reset();clear_defenders(p);for(int i=0;i<10;++i)p.step();p.pass();
    const auto y=p.engine.teams[2].tm_y;p.set_input(0,1);for(int i=0;i<12;++i){clear_defenders(p);p.step();}
    expect(p.engine.teams[2].tm_y>y+20,"Fresh manual input overrides the auto receiving run");
    p.reset();clear_defenders(p);p.set_input(0,1);for(int i=0;i<6;++i)p.step();p.pass();
    for(int i=0;i<150 && !p.engine.ball_poss;++i){clear_defenders(p);p.set_input(0,1);p.step();}
    expect(p.engine.ball_poss==3,"Held passer direction cannot derail the receiving run");
    // Keep opponents remote to exercise spacing and touchline behavior over a prolonged carry.
    p.reset();p.engine.teams[1].tm_x=100;p.engine.teams[1].tm_y=80;
    p.engine.teams[2].tm_x=280;p.engine.teams[2].tm_y=80;
    for(int i=0;i<240;++i){clear_defenders(p);p.set_input(i<120?1.0f:-1.0f,0);p.step();}
    const auto& teammate=p.engine.teams[2];
    expect(teammate.tm_x>=12 && teammate.tm_x<=1268 && teammate.tm_y>=12 && teammate.tm_y<=788,"Support stays inside the pitch at the touchline");
    expect(std::hypot(teammate.tm_x-p.engine.teams[1].tm_x,teammate.tm_y-p.engine.teams[1].tm_y)>100,"Support avoids crowding the carrier during a long run");
    int completed=0;
    for(bool loft:{false,true}) for(int delay:{4,8,12,16,20,24}) {
        p.reset();for(int i=0;i<delay;++i)p.step();
        if(p.pass(loft)) for(int i=0;i<120 && !p.engine.ball_poss && !p.outside;++i)p.step();
        if(p.engine.ball_poss==3) ++completed;
    }
    std::cout<<"Opening support passes completed against active opponents: "<<completed<<"/12\n";
    expect(completed>=4,"Support offers usable passes against active opponents");
    p.set_opponents(false);for(int i=0;i<30;++i)p.step();
    expect(p.engine.teams[2].tm_x==740 && p.engine.teams[2].tm_y==400,"Solo drill keeps its approved stationary-receiver behavior");
    std::cout<<"Passed "<<checks<<" teammate support checks.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
