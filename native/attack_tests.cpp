#include "player.hpp"
#include <iostream>
#include <stdexcept>
using actua::PlayerPractice;
static int checks=0;
static void expect(bool v,const char* m){++checks;if(!v)throw std::runtime_error(m);}
static PlayerPractice attack(float x=220,float y=400){
    PlayerPractice p;p.set_opponents(true);p.engine.holder_lose_ball();
    auto& r=p.engine.teams[12];r.tm_x=x;r.tm_y=y;r.tm_xdis=-1;r.tm_ydis=0;r.tm_poss=20;
    p.engine.teams[1].tm_x=800;p.engine.teams[1].tm_y=150;
    p.engine.teams[2].tm_x=950;p.engine.teams[2].tm_y=650;
    p.engine.collect_ball(&r);r.tm_poss=20;p.keepers[0].y=364;return p;
}
int main(){try{
    auto p=attack();p.step();
    expect(p.engine.teams[12].tm_shots==1 && !p.engine.ball_poss && p.engine.ballxdis<0,"Red shoots toward left goal from a clear chance");
    expect(p.selected==1 && !p.red_passes,"Shot does not select red or also pass");
    p=attack(100);p.keepers[0].y=364;p.step();
    for(int i=0;i<30 && !p.red_score && p.keeper_holding<0;++i)p.step();
    expect(p.red_score==1 && !p.restart_red,"Red can score a deliberate shot past a misplaced keeper");
    for(int i=0;i<40;++i)p.step();expect(p.engine.ball_poss==2,"Blue gets the kickoff after a red goal");
    p=attack(700);p.step();expect(!p.engine.teams[12].tm_shots,"Red does not shoot from long range");
    p=attack(200,650);p.step();expect(!p.engine.teams[12].tm_shots,"Red avoids an unsuitable wide shooting angle");
    p=attack();p.engine.teams[1].tm_x=120;p.engine.teams[1].tm_y=414;p.step();
    expect(!p.engine.teams[12].tm_shots,"A defender in the shooting lane blocks the decision");
    p=attack();p.paused=true;p.step();expect(!p.engine.teams[12].tm_shots,"Paused opponent cannot shoot");
    p=attack();p.restart_ticks=5;p.step();expect(!p.engine.teams[12].tm_shots,"No shot during restart countdown");
    p=attack(500);p.engine.teams[13].guy_on=0;
    for(int i=0;i<400 && !p.engine.teams[12].tm_shots;++i)p.step();
    expect(p.engine.teams[12].tm_shots>0,"Unpressured red advances into shooting range without scripted repositioning");
    PlayerPractice live;live.set_opponents(true);
    for(int i=0;i<2400;++i)live.step();
    const int shots=live.engine.teams[12].tm_shots+live.engine.teams[13].tm_shots;
    std::cout<<"Two-minute live scenario: "<<shots<<" red shots, "<<live.red_score<<" red goals, "<<live.keepers[0].saves<<" blue saves.\n";
    expect(shots>0 && live.red_score>0,"Normal two-versus-two play produces deliberate red shots and goals");
    std::cout<<"Passed "<<checks<<" attacking checks.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
