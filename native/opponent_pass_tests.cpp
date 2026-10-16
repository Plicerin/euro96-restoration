#include "player.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using actua::PlayerPractice;
static int checks=0;
static void expect(bool v,const char* m){++checks;if(!v)throw std::runtime_error(m);}
static PlayerPractice setup(){
    PlayerPractice p;p.set_opponents(true);p.engine.holder_lose_ball();
    auto& carrier=p.engine.teams[12];carrier.tm_x=900;carrier.tm_y=400;carrier.tm_xdis=-1;carrier.tm_ydis=0;
    auto& target=p.engine.teams[13];target.tm_x=670;target.tm_y=400;target.tm_xdis=-1;target.tm_ydis=0;
    p.engine.teams[1].tm_x=1000;p.engine.teams[1].tm_y=400;
    p.engine.teams[2].tm_x=1050;p.engine.teams[2].tm_y=650;
    p.engine.collect_ball(&carrier);return p;
}
static void launch(PlayerPractice& p){for(int i=0;i<160 && !p.red_passes;++i)p.step();}
int main(){try{
    auto p=setup();p.set_input(0.2f,0);launch(p);
    expect(p.red_passes==1 && !p.engine.ball_poss,"Red chooses an open forward pass");
    expect(p.selected==1 && p.input_x==0.2f,"Opponent pass retains user selection and movement");
    expect(!p.engine.ball_inair && p.engine.ballz==2,"Opponent uses a real ground pass");
    expect(p.engine.teams[12].tm_pass==1,"Original pass statistic records red attempt");
    for(int i=0;i<160 && !p.engine.ball_poss;++i)p.step();
    expect(p.engine.ball_poss==14 && p.red_completed==1,"Moving red teammate receives the pass");
    expect(p.engine.teams[12].tm_pwon==1 && p.passes_received==0,"Red completion stats are separate from blue catches");
    for(int i=0;i<12;++i)p.step();expect(p.red_passes==1,"Cooldown prevents immediate ping-pong passing");
    p=setup();launch(p);
    p.engine.teams[1].tm_x=p.engine.ballx+p.engine.ballxdis*2;
    p.engine.teams[1].tm_y=p.engine.bally+p.engine.ballydis*2;
    for(int i=0;i<12 && !p.engine.ball_poss;++i)p.step();
    expect(p.engine.ball_poss==2 && p.blue_wins==1 && p.red_completed==0,"Blue can intercept an opponent pass in flight");
    expect(p.selected==1,"Interception leaves control on the blue interceptor");
    p=setup();p.paused=true;for(int i=0;i<100;++i)p.step();
    expect(!p.red_passes && p.ticks==0,"Paused opponents cannot pass");
    p=setup();for(int i=0;i<10;++i)p.step();expect(!p.red_passes,"Fresh possession has time to settle before passing");
    p=setup();
    for(int i=0;i<120;++i){
        auto& a=p.engine.teams[12];auto& b=p.engine.teams[13];
        a.tm_x=900;a.tm_y=400;a.tm_xdis=-1;a.tm_ydis=0;
        b.tm_x=730;b.tm_y=270;b.go_txdis=b.go_tydis=0;
        p.engine.teams[1].tm_x=810;p.engine.teams[1].tm_y=335;
        p.engine.hold_ball(&a);p.step();
    }
    expect(p.red_passes==0,"A blue player blocking the passing lane prevents the pass");
    p=setup();p.engine.teams[12].tm_x=500;p.engine.teams[12].tm_xdis=1;
    p.engine.teams[13].tm_x=740;p.engine.teams[2].tm_x=200;
    p.engine.hold_ball(&p.engine.teams[12]);
    for(int i=0;i<40 && !p.red_passes;++i){
        p.engine.teams[1].tm_x=p.engine.teams[12].tm_x-100;
        p.engine.teams[1].tm_y=p.engine.teams[12].tm_y;p.step();
    }
    expect(p.red_passes==1 && p.engine.ballxdis>0,"Pressure can trigger a safe backward outlet pass");
    p=setup();p.engine.teams[13].guy_on=0;for(int i=0;i<140;++i)p.step();
    expect(!p.red_passes,"No pass to an inactive teammate");
    p=setup();launch(p);p.reset();
    expect(p.red_passes==0 && p.red_completed==0 && p.engine.ball_poss==2,"Reset clears opponent passing state and counters");
    p=setup();launch(p);p.set_opponents(false);for(int i=0;i<120;++i)p.step();
    expect(!p.red_passes && p.engine.ball_poss==2,"Solo mode cancels opponent passes");
    // A real turnover followed by a sustained possession produces visible exchanges.
    p.set_opponents(true);for(int i=0;i<1000;++i)p.step();
    std::cout<<"Live AI: "<<p.red_passes<<" attempts, "<<p.red_completed<<" completions in 50 seconds.\n";
    expect(p.red_passes>=2 && p.red_completed>=2,"Opponents exchange the ball in ordinary play");
    std::cout<<"Passed "<<checks<<" opponent-passing checks.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
