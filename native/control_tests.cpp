#include "original_actions.hpp"
#include "original_graphics.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <limits>
using namespace actua;
using Kick=OriginalActions::PlayKick;
static void require(bool condition,const char* why){if(!condition)throw std::runtime_error(why);}
static void ticks(OriginalActions& s,int count){for(int i=0;i<count;++i)s.step();}
static void directions(OriginalActions& s){
    for(int x=-1;x<=1;++x)for(int y=-1;y<=1;++y){
        if(!x&&!y)continue;s.start_play();const float d=std::hypot(float(x),float(y));
        s.set_play_input(float(x),float(y));ticks(s,24);const auto& p=s.engine.teams[1];
        require(((p.tm_x-900)*x+(p.tm_y-400)*y)/d>40,"Every direction must move along the requested axis after turning");
        require((p.tm_xdis*x+p.tm_ydis*y)/d>.98f,"Original turning must eventually face the movement direction");
        require(s.engine.ball_poss==2 && std::hypot(s.engine.ballx-p.tm_x,s.engine.bally-p.tm_y)<19,"Original dribble must stay at the runner's feet");
        s.set_play_input(0,0);s.step();const auto stopped=s.engine.teams[1];ticks(s,20);
        require(s.engine.teams[1].tm_x==stopped.tm_x && s.engine.teams[1].tm_y==stopped.tm_y,"Stopping must not slide on any axis");
    }
    s.start_play();s.set_play_input(std::numeric_limits<float>::quiet_NaN(),1);ticks(s,10);
    require(s.engine.teams[1].tm_x==900 && s.engine.teams[1].tm_y==400,"Invalid input is neutral");
    s.start_play();s.switch_play_player();s.set_play_input(1,0);ticks(s,16);
    require(s.engine.teams[2].tm_x>1060 && s.engine.teams[1].tm_x==900 && s.engine.ball_poss==2,"Switching permits off-ball movement without dragging the ball");
}
int main(int argc,char** argv){try{
    OriginalGraphics g(argv[1]);auto& s=*g.sequence;directions(s);s.start_play();
    s.set_play_input(1,0);for(int i=0;i<12;++i)s.step();
    require(s.engine.teams[1].tm_x>940 && s.engine.ball_poss==2,"Original user running must dribble forward");
    s.set_play_input(0,0);s.step();const float stopped=s.engine.teams[1].tm_x;
    for(int i=0;i<30;++i)s.step();require(s.engine.teams[1].tm_x==stopped,"Neutral input must stop drift");
    for(Kick kick:{Kick::ground,Kick::loft}){
        s.start_play();s.request_play_kick(kick);
        for(int i=0;i<180 && !s.receptions;++i){s.step();if(argc>2)std::cout<<s.ticks<<" ball "<<s.engine.ballx<<","<<s.engine.bally<<","<<s.engine.ballz<<" owner "<<s.engine.ball_poss<<" recv "<<s.receptions<<" act "<<s.engine.teams[2].tm_act<<" cnt "<<s.engine.teams[2].int_cnt<<" strike "<<s.engine.teams[2].tm_strike<<" prep "<<s.pass_target<<'\n';}
        require(s.releases==1 && s.receptions==1 && s.selected==2 && s.engine.ball_poss==3,"Original pass must be received and switch control");
        // Hold fresh input through completion of the actual reception animation.
        const float received_x=s.engine.teams[2].tm_x;s.set_play_input(1,0);ticks(s,25);
        require(s.engine.teams[2].tm_x>received_x+15 && s.engine.ball_poss==3,"Held input must move the receiver when original control animation completes");
    }
    s.start_play();
    for(int pass=0;pass<6;++pass){
        const int owner=s.engine.ball_poss;s.request_play_kick(Kick::ground);
        const int expected=s.receptions+1;
        for(int i=0;i<140 && s.receptions<expected;++i)s.step();
        require(s.receptions==expected && s.engine.ball_poss!=owner,"Repeated original passes must transfer possession");
        ticks(s,20);
    }
    s.start_play();s.request_play_kick(Kick::ground);s.clear_play_input();s.step();
    require(s.releases==0 && s.engine.teams[1].tm_act==0,"Focus/pause cancellation must discard pending kicks");
    s.start_play();s.engine.holder_lose_ball();s.engine.ballx=906;s.engine.bally=400;s.engine.ballz=2;
    s.engine.ballxdis=s.engine.ballydis=s.engine.ballzdis=0;s.engine.ball_inair=0;ticks(s,2);
    require(s.engine.ball_poss==2,"Original foot-contact control must allow loose-ball recovery");
    s.start_play();s.request_play_kick(Kick::shot);for(int i=0;i<60;++i)s.step();
    require(s.shots==1 && s.engine.teams[1].tm_shots==1 && !s.engine.ball_poss,"Original driven shot must release and count once");
    s.start_play();s.engine.teams[1].tm_x=200;s.engine.hold_ball(&s.engine.teams[1]);s.request_play_kick(Kick::shot);ticks(s,20);
    require(!s.shots && s.engine.ball_poss==2,"Original shot range must reject an out-of-range request");
    s.start_play();s.engine.teams[1].tm_xdis=0;s.engine.teams[1].tm_ydis=-1;s.engine.hold_ball(&s.engine.teams[1]);s.request_play_kick(Kick::shot);
    for(int i=0;i<30 && !s.shots;++i)s.step();
    require(s.shots==1 && s.engine.ballydis<0 && std::abs(s.engine.ballxdis)<.1f,"User shot must follow facing, not auto-aim at goal");
    s.start_play();s.engine.ballx=float(s.engine.pitch_len)+1;s.engine.ball_poss=0;ticks(s,100);
    require(!s.running,"Out-of-play preview must settle and stop");s.start_play();require(s.running && !s.shots && !s.receptions && s.engine.ball_poss==2,"Reset must restore a usable possession");
    std::cout<<"Original running, neutral stop, ground/loft reception and shot passed.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
