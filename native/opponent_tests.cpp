#include "player.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using actua::PlayerPractice;
static int checks=0;
static void expect(bool v,const char* m){++checks;if(!v)throw std::runtime_error(m);}
static void red_holds(PlayerPractice& p) {
    p.set_opponents(true);p.engine.holder_lose_ball();
    auto& red=p.engine.teams[12];red.tm_x=520;red.tm_y=400;red.tm_xdis=-1;red.tm_ydis=0;
    p.engine.collect_ball(&red);
}
int main() {try {
    PlayerPractice p;p.opponent_shooting=false;expect(!p.opponents,"Approved solo drill remains default");
    p.set_opponents(true);expect(p.opponents && p.engine.teams[12].guy_on && p.engine.teams[13].guy_on,"Two active opponents");
    const float start=p.engine.teams[12].tm_y;p.step();
    expect(p.engine.teams[12].tm_y>start,"Opponent closes down the carrier");
    for(int i=0;i<200 && p.engine.ball_poss<12;++i)p.step();
    expect(p.engine.ball_poss>=12 && p.red_wins==1,"Idle carrier loses possession to pressure");
    expect(p.selected==1 || p.selected==2,"Red possession never selects an opponent");
    const auto red_x=p.engine.teams[std::size_t(p.engine.ball_poss-1)].tm_x;p.step();
    expect(p.engine.teams[std::size_t(p.engine.ball_poss-1)].tm_x!=red_x,"Opponent dribbles after winning possession");
    for(int i=0;i<2000;++i)p.step();
    expect(!p.outside && std::isfinite(p.engine.ballx),"Opponent keeps possession in bounds during extended play");
    // Recover through actual steering and tackles, without relocating either player.
    for(int i=0;i<400 && (p.engine.ball_poss==0 || p.engine.ball_poss>=12);++i) {
        const auto& blue=p.engine.teams[std::size_t(p.selected)];
        const float dx=p.engine.ballx-blue.tm_x,dy=p.engine.bally-blue.tm_y,d=std::hypot(dx,dy);
        if(d>0)p.set_input(dx/d,dy/d);
        p.step();if(d<26)p.challenge();
    }
    expect(p.engine.ball_poss==p.selected+1 && p.blue_wins>0,"Player can chase and tackle a moving opponent without scripted placement");
    p.reset();expect(p.opponents && p.red_wins==0 && p.blue_wins==0 && p.engine.ball_poss==2,"Reset preserves mode and resets possession/counters");
    p.engine.teams[12].tm_x=620;p.engine.teams[12].tm_y=400;
    p.engine.teams[13].tm_x=1000;p.engine.teams[13].tm_y=700;
    p.pass();for(int i=0;i<60 && !p.engine.ball_poss;++i)p.step();
    expect(p.engine.ball_poss>=12 && p.red_wins==1,"Opponent can intercept a ground pass");
    expect(p.passes_received==0 && p.selected==2,"Interception does not count as teammate reception or change selection");
    p.reset();p.engine.holder_lose_ball();p.engine.ballx=610;p.engine.bally=400;p.engine.ballz=30;
    p.engine.ball_inair=1;p.engine.ballxdis=10;p.engine.teams[12].tm_x=620;p.engine.teams[12].tm_y=400;p.step();
    expect(!p.engine.ball_poss,"Opponents cannot attract high balls");
    p.reset();p.engine.holder_lose_ball();p.engine.teams[2].tm_x=600;p.engine.teams[12].tm_x=700;p.engine.teams[12].tm_y=400;
    p.engine.ballx=550;p.engine.bally=400;p.engine.ballxdis=200;p.step();
    expect(p.engine.ball_poss==3,"First contact wins when a ball crosses both teams in one tick");
    red_holds(p);expect(p.action() && p.engine.ball_poss==2 && p.blue_wins==1,"X action wins an in-range tackle");
    expect(p.engine.teams[12].tm_poss==0,"Previous holder releases the ball cleanly");
    expect(p.selected==1,"Tackle keeps player control on blue");
    red_holds(p);p.engine.teams[1].tm_x=450;expect(!p.challenge() && p.engine.ball_poss==13,"Out-of-reach tackle cannot steal possession");
    p.engine.teams[1].tm_x=500;expect(!p.challenge(),"Missed tackle has a recovery period");
    red_holds(p);p.engine.teams[1].tm_xdis=-1;expect(!p.challenge(),"Tackle requires facing the ball");
    red_holds(p);p.paused=true;const auto tick=p.ticks;const auto rx=p.engine.teams[12].tm_x;
    p.step();expect(!p.challenge() && p.ticks==tick && p.engine.teams[12].tm_x==rx,"Pause freezes AI and blocks tackling");
    red_holds(p);p.outside=true;expect(!p.challenge(),"Outside-pitch state blocks tackles");
    p.reset();p.engine.teams[12].tm_x=534;p.engine.teams[12].tm_y=400;
    p.engine.teams[13].tm_x=1000;p.engine.teams[13].tm_y=700;
    for(int i=0;i<7;++i)p.step();
    expect(p.engine.ball_poss==2 && p.opponent_windup[0]==7,"AI tackle has a visible reaction window");
    p.step();expect(p.engine.ball_poss==13,"AI completes a tackle after the reaction window");
    expect(!p.challenge(),"New holder is protected against an immediate steal-back");
    p.reset();p.engine.teams[12].tm_x=534;p.engine.teams[12].tm_y=400;p.step();
    expect(p.opponent_windup[0]>0,"Close opponent prepares a tackle");
    p.pass();p.step();expect(p.opponent_windup[0]==0,"Passing cancels an incoming tackle");
    p.set_opponents(false);expect(!p.engine.teams[12].guy_on && p.engine.ball_poss==2,"Disabling opponents restores the approved drill");
    p.pass();for(int i=0;i<60 && !p.engine.ball_poss;++i)p.step();
    expect(p.engine.ball_poss==3,"Solo pass reception remains unchanged after toggling");
    // A press just before contact must remain active as the player closes in.
    red_holds(p);p.engine.teams[1].tm_x=462;p.set_input(1,0);p.challenge();
    for(int i=0;i<8 && (p.engine.ball_poss==0 || p.engine.ball_poss>=12);++i)p.step();
    expect(p.engine.ball_poss==2,"A tackle pressed just before contact connects during the approach");
    // Chase with eight keyboard directions, sampled at human reaction intervals.
    // Holding tackle must recover from both near and distant real AI turnovers.
    int slowest_chase=0;
    for(int delay:{0,40,200,600}) for(int reaction:{3,6}) for(int player:{1,2}) {
        p.set_opponents(true);for(int i=0;i<200 && p.engine.ball_poss<12;++i)p.step();
        for(int i=0;i<delay;++i)p.step();
        if((p.engine.ball_poss==0 || p.engine.ball_poss>=12) && p.selected!=player)p.switch_player();
        p.set_tackle_held(true);
        int chase_ticks=0;
        for(;chase_ticks<800 && (p.engine.ball_poss==0 || p.engine.ball_poss>=12);++chase_ticks) {
            if(chase_ticks%reaction==0) {
                const auto& blue=p.engine.teams[std::size_t(p.selected)];
                const float angle=std::atan2(p.engine.bally-blue.tm_y,p.engine.ballx-blue.tm_x);
                const float direction=std::round(angle/(3.1415926536f/4))*(3.1415926536f/4);
                p.set_input(std::round(std::cos(direction)),std::round(std::sin(direction)));
            }
            p.step();
        }
        if(chase_ticks>slowest_chase)slowest_chase=chase_ticks;
        if(!(p.engine.ball_poss>0 && p.engine.ball_poss<12 && p.selected+1==p.engine.ball_poss)) std::cerr<<"Failed keyboard chase delay="<<delay<<" reaction="<<reaction<<" player="<<player<<'\n';
        expect(p.engine.ball_poss>0 && p.engine.ball_poss<12 && p.selected+1==p.engine.ball_poss && p.blue_wins>0,"Pressing and teammate defence recover possession with eight-direction steering");
        p.set_input(0,0);p.step();
        expect(p.engine.ball_poss==p.selected+1 && p.engine.teams[std::size_t(p.selected)].tm_shots==0,"Holding tackle after a regain never fires a shot");
    }
    std::cout<<"Slowest of 16 keyboard defence scenarios: "<<float(slowest_chase)/20<<" seconds.\n";
    // A movement key held through an intercepted pass must immediately work on defence.
    p.set_opponents(true);p.engine.teams[12].tm_x=620;p.engine.teams[12].tm_y=400;
    p.engine.teams[13].tm_x=1000;p.engine.teams[13].tm_y=700;
    p.set_input(0,1);p.pass();
    for(int i=0;i<80 && !p.engine.ball_poss;++i){p.set_input(0,1);p.step();}
    expect(p.engine.ball_poss>=12,"Movement-guard regression setup reaches an interception");
    p.set_input(0,1);expect(p.input_y==1,"Interception releases the passer-input guard for defensive movement");
    red_holds(p);p.engine.teams[1].tm_x=300;p.set_tackle_held(true);
    for(int i=0;i<40;++i)p.step();expect(p.engine.teams[1].tm_twon==0 && p.engine.teams[1].tm_x==300,"Held tackle never steals a distant ball");
    red_holds(p);p.set_tackle_held(true);p.paused=true;for(int i=0;i<20;++i)p.step();
    expect(p.engine.ball_poss==13 && p.ticks==0,"Held tackle respects pause");
    std::cout<<"Passed "<<checks<<" opponent and possession checks.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
