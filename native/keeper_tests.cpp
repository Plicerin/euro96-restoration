#include "player.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using actua::PlayerPractice;
static int checks=0;
static void expect(bool v,const char* m){++checks;if(!v)throw std::runtime_error(m);}
static void ball(PlayerPractice& p,int side,float speed,float y=400,float z=2){
    p.set_opponents(true);p.engine.holder_lose_ball();
    p.engine.ballx=side==0?60.0f:1220.0f;p.engine.bally=y;p.engine.ballz=z;
    p.engine.ballxdis=side==0?-speed:speed;p.engine.ballydis=p.engine.ballzdis=0;
    p.engine.ball_inair=z>2;p.engine.last_touch=side==0?13:2;
}
int main(){try{
    PlayerPractice p;expect(!p.keepers[0].active && !p.keepers[1].active,"Solo drill has no keepers");
    for(int side:{0,1}){
        ball(p,side,12);for(int i=0;i<8 && p.keeper_holding<0;++i)p.step();
        expect(p.keeper_holding==side && p.keepers[std::size_t(side)].saves==1,"Either keeper catches a manageable shot");
        expect(!p.blue_score && !p.red_score && p.selected>=1 && p.selected<=2,"Save prevents goal without selecting a keeper");
        const int remaining=p.keeper_hold_ticks;p.paused=true;p.step();
        expect(p.keeper_hold_ticks==remaining,"Pause freezes keeper hold");p.paused=false;
        expect(!p.challenge() && !p.shoot() && !p.pass(),"Keeper catch cannot be tackled or manually kicked");
        for(int i=0;i<25 && p.keeper_holding>=0;++i)p.step();
        expect(p.keeper_holding<0 && !p.engine.ball_poss && (side==0?p.engine.ballxdis>0:p.engine.ballxdis<0),"Keeper releases a real outlet into the field");
        for(int i=0;i<180 && !p.engine.ball_poss && p.keeper_holding<0 && !p.restart_ticks;++i)p.step();
        expect(p.engine.ball_poss>0 && (p.engine.ball_poss<12)==(side==0),"Keeper outlet reaches an outfield teammate");
        ball(p,side,40);p.step();
        expect(p.keepers[std::size_t(side)].parries==1 && p.keeper_holding<0,"Hard shots produce a loose rebound");
        expect((side==0?p.engine.ballxdis>0:p.engine.ballxdis<0) && !p.blue_score && !p.red_score,"Parry reverses ball into play before scoring");
        const float x=p.engine.ballx;p.step();
        expect((side==0?p.engine.ballx>x:p.engine.ballx<x) && p.keepers[std::size_t(side)].saves==1,"Rebound continues physically without repeated save contacts");
        ball(p,side,150);p.step();
        expect(p.keepers[std::size_t(side)].parries==1 && !p.blue_score && !p.red_score,"Swept keeper contact beats a same-tick whole-ball crossing");
        ball(p,side,150,428);p.keepers[std::size_t(side)].y=364;p.step();
        expect((side==0?p.red_score:p.blue_score)==1,"A shot beyond keeper reach scores");
        ball(p,side,40,436);for(int i=0;i<5 && !p.restart_ticks;++i)p.step();
        expect((side==0?p.red_score:p.blue_score)==1 && !p.keepers[std::size_t(side)].saves,"A close corner shot can beat a centred keeper");
        ball(p,side,12,420);p.step();
        expect(p.keepers[std::size_t(side)].y==400,"Keeper response has a short delay rather than instant tracking");
        ball(p,side,150,400,40);p.step();
        expect(!p.keepers[std::size_t(side)].saves && p.restart_ticks>0,"High balls cannot be magnetically caught");
    }
    ball(p,0,40);p.step();const float x=p.engine.ballx,y=p.engine.bally;
    p.engine.teams[1].tm_x=x+10;p.engine.teams[1].tm_y=y;
    for(int i=0;i<30 && !p.engine.ball_poss;++i)p.step();
    expect(p.engine.ball_poss==2 || p.engine.ball_poss==3,"Blue can recover a parried rebound");
    ball(p,1,100);p.engine.teams[12].tm_x=1235;p.engine.teams[12].tm_y=400;p.step();
    expect(p.engine.ball_poss==13 && !p.keepers[1].saves,"Earlier outfield interception precedes keeper contact");
    ball(p,0,12);for(int i=0;i<8 && p.keeper_holding<0;++i)p.step();
    for(int i=0;i<25 && p.keeper_holding>=0;++i)p.step();
    p.engine.holder_lose_ball();p.engine.ballx=1270;p.engine.bally=400;p.engine.ballz=2;
    p.engine.ballxdis=40;p.engine.ballydis=p.engine.ballzdis=0;p.engine.ball_inair=0;p.step();
    for(int i=0;i<40;++i)p.step();
    expect(p.blue_score==1 && p.keepers[0].saves==1 && p.keepers[0].y==400,"Kickoff resets keeper position but retains save statistics");
    p.reset();expect(p.keeper_holding<0 && !p.keepers[0].saves && !p.keepers[1].saves,"Reset clears keeper state and saves");
    p.set_opponents(false);expect(!p.keepers[0].active && !p.keepers[1].active,"Disabling opponents removes keepers");
    std::cout<<"Passed "<<checks<<" keeper checks.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
