#include "player.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using actua::PlayerPractice;
static int checks=0;
static void expect(bool v,const char* m){++checks;if(!v)throw std::runtime_error(m);}
static PlayerPractice defense(int selected=1){
    PlayerPractice p;p.set_opponents(true);if(selected==2)p.switch_player();
    p.engine.holder_lose_ball();
    auto& r=p.engine.teams[12];r.tm_x=900;r.tm_y=300;r.tm_xdis=-1;r.tm_ydis=0;
    p.engine.teams[13].tm_x=720;p.engine.teams[13].tm_y=550;
    p.engine.collect_ball(&r);return p;
}
int main(){try{
    for(int selected:{1,2}){
        auto p=defense(selected);const int other=selected==1?2:1;
        const auto before=p.engine.teams[std::size_t(other)],manual=p.engine.teams[std::size_t(selected)];
        for(int i=0;i<12;++i)p.step();const auto& after=p.engine.teams[std::size_t(other)];
        expect(std::hypot(after.tm_x-before.tm_x,after.tm_y-before.tm_y)>12,"Unselected blue moves during red possession");
        expect(p.engine.teams[std::size_t(selected)].tm_x==manual.tm_x && p.engine.teams[std::size_t(selected)].tm_y==manual.tm_y,
            "Defensive AI never steers the selected player");
        p=defense(selected);p.paused=true;const auto stopped=p.engine.teams[std::size_t(other)];
        for(int i=0;i<12;++i)p.step();
        expect(p.engine.teams[std::size_t(other)].tm_x==stopped.tm_x && p.engine.teams[std::size_t(other)].tm_y==stopped.tm_y,"Pause freezes defensive support");
    }
    auto cover=defense();
    const float tx=cover.engine.ballx+(720-cover.engine.ballx)*0.65f,ty=cover.engine.bally+(550-cover.engine.bally)*0.65f;
    const float start=std::hypot(cover.engine.teams[2].tm_x-tx,cover.engine.teams[2].tm_y-ty);
    for(int i=0;i<30;++i){
        auto& red=cover.engine.teams[12];red.tm_x=900;red.tm_y=300;red.tm_xdis=-1;red.tm_ydis=0;red.tm_poss=1;
        cover.engine.teams[13].tm_x=720;cover.engine.teams[13].tm_y=550;
        cover.engine.hold_ball(&red);cover.step();
    }
    expect(std::hypot(cover.engine.teams[2].tm_x-tx,cover.engine.teams[2].tm_y-ty)<start-30,"Defender closes the red passing lane");
    expect(cover.engine.ball_poss==13,"Lane coverage does not automatically steal a held ball");
    auto p=defense();p.engine.holder_lose_ball();p.engine.last_touch=13;
    p.engine.ballx=820;p.engine.bally=400;p.engine.ballz=2;p.engine.ballxdis=p.engine.ballydis=p.engine.ballzdis=0;
    p.engine.teams[12].tm_x=1200;p.engine.teams[12].tm_y=50;
    p.engine.teams[13].tm_x=1200;p.engine.teams[13].tm_y=750;
    for(int i=0;i<80 && !p.engine.ball_poss;++i)p.step();
    expect(p.engine.ball_poss==3 && p.blue_wins==1,"Unselected teammate pursues and collects a loose opponent ball");
    expect(p.selected==2,"Recovered possession selects the teammate who collected it");
    p=defense();p.step();p.switch_player();const auto stopped=p.engine.teams[2];
    for(int i=0;i<8;++i)p.step();
    expect(p.engine.teams[2].tm_x==stopped.tm_x && p.engine.teams[2].tm_y==stopped.tm_y,"Selecting the defender immediately cancels AI movement");
    p.set_input(0,1);p.step();expect(p.engine.teams[2].tm_y!=stopped.tm_y,"Manual steering works after switching to defender");
    p=defense();p.restart_ticks=10;const auto waiting=p.engine.teams[2];
    for(int i=0;i<5;++i)p.step();
    expect(p.engine.teams[2].tm_x==waiting.tm_x && p.engine.teams[2].tm_y==waiting.tm_y,"Restart countdown freezes defensive movement");
    p.set_opponents(false);for(int i=0;i<30;++i)p.step();
    expect(p.engine.teams[2].tm_x==740 && p.engine.teams[2].tm_y==400,"Solo passing practice remains stationary");
    std::cout<<"Passed "<<checks<<" teammate defence checks.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
