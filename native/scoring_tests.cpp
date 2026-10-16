#include "player.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using actua::PlayerPractice;
static int checks=0;
static void expect(bool v,const char* m){++checks;if(!v)throw std::runtime_error(m);}
static void ball(PlayerPractice& p,float x,float y,float z,float vx,float vy=0,float vz=0){
    p.engine.holder_lose_ball();p.engine.ballx=x;p.engine.bally=y;p.engine.ballz=z;
    p.engine.ballxdis=vx;p.engine.ballydis=vy;p.engine.ballzdis=vz;p.engine.ball_inair=z>2 || vz!=0;
}
int main(){try{
    PlayerPractice p;p.set_opponents(true);
    ball(p,1270,400,2,30);p.step();
    expect(p.blue_score==1 && p.red_score==0 && p.restart_ticks==40,"Right goal scores for blue");
    expect(p.restart_red && !p.outside,"Conceding red team receives kickoff");
    const float scored_x=p.engine.ballx;
    expect(!p.pass() && !p.shoot() && !p.challenge(),"Actions cannot disturb a goal celebration");
    for(int i=0;i<10;++i)p.step();expect(p.blue_score==1 && p.engine.ballx==scored_x,"Goal counts once and freezes play");
    p.paused=true;p.step();expect(p.restart_ticks==30,"Pause freezes restart countdown");p.paused=false;
    for(int i=0;i<30;++i)p.step();
    expect(p.engine.ball_poss==13 && p.restart_ticks==0 && p.blue_score==1,"Automatic red kickoff retains score");
    expect(p.engine.ballx==640 && p.engine.bally==400,"Kickoff places the ball at centre");
    ball(p,10,400,2,-30);p.step();expect(p.red_score==1 && !p.restart_red,"Left goal scores for red including blue own goals");
    for(int i=0;i<40;++i)p.step();expect(p.engine.ball_poss==2 && p.blue_score==1 && p.red_score==1,"Blue kickoff retains both scores");
    p.set_input(0,0);p.set_input(1,0);p.step();expect(p.engine.teams[1].tm_x>630,"Kickoff is playable after direction release");
    p.reset();ball(p,1281,400,2,0.5f);p.step();expect(p.blue_score==0 && !p.restart_ticks,"Partially crossed ball is still in play");
    p.engine.ballxdis=2;p.step();expect(p.blue_score==1,"Whole-ball crossing completes the goal");
    for(float y:{355.0f,360.0f,440.0f,445.0f}) {
        p.reset();ball(p,1270,y,2,40);p.step();expect(!p.blue_score && p.restart_ticks==20,"Wide/post-overlapping balls are not goals");
    }
    for(float z:{32.0f,45.0f}) {
        p.reset();ball(p,1270,400,z,40);p.step();expect(!p.blue_score && p.restart_ticks==20,"Bar-overlapping or high shots are not goals");
    }
    p.reset();ball(p,1270,400,14,40);p.step();expect(p.blue_score==1,"Low airborne shot scores");
    p.reset();ball(p,1200,400,80,400,0,-240);p.step();expect(p.blue_score==0,"Height is tested at crossing rather than low end-of-tick position");
    p.reset();ball(p,1200,400,2,400,400);p.step();expect(p.blue_score==0,"Crossing position rejects diagonal shot wide of goal");
    p.reset();ball(p,1200,780,2,400,200);p.step();expect(p.blue_score==0 && p.restart_ticks==20,"First boundary crossing wins on diagonal exits");
    p.reset();p.blue_score=2;p.red_score=3;p.engine.last_touch=2;ball(p,600,795,2,0,20);p.step();
    expect(p.restart_red && !p.restart_goal && p.restart_ticks==20,"Out ball awarded to opposite last-touch team");
    for(int i=0;i<20;++i)p.step();expect(p.engine.ball_poss==13 && p.blue_score==2 && p.red_score==3 && !p.outside,"Out restart preserves score and resumes play");
    p.engine.last_touch=13;ball(p,600,5,2,0,-20);p.step();expect(!p.restart_red,"Red last touch awards blue restart");
    for(int i=0;i<20;++i)p.step();expect(p.engine.ball_poss==2,"Blue takes out restart");
    p.reset();p.engine.teams[1].tm_shots=3;p.engine.teams[1].tm_pass=5;ball(p,1270,400,2,40);p.step();
    for(int i=0;i<40;++i)p.step();expect(p.engine.teams[1].tm_shots==3 && p.engine.teams[1].tm_pass==5,"Restarts preserve player statistics");
    p.reset();p.engine.teams[12].tm_x=1238;p.engine.teams[12].tm_y=400;
    ball(p,1230,400,2,100);p.step();
    expect(!p.blue_score && p.engine.ball_poss==13 && !p.restart_ticks,"Interception before the line beats a same-tick goal crossing");
    p.reset();p.engine.last_touch=2;ball(p,10,400,2,-30);p.step();
    expect(p.red_score==1 && !p.restart_red,"Blue own goal awards red and blue kickoff");
    p.reset();ball(p,1270,400,2,40);p.step();for(int i=0;i<40;++i)p.step();
    p.set_tackle_held(true);
    for(int i=0;i<400 && (p.engine.ball_poss==0 || p.engine.ball_poss>=12);++i) {
        const auto& blue=p.engine.teams[std::size_t(p.selected)];
        const float dx=p.engine.ballx-blue.tm_x,dy=p.engine.bally-blue.tm_y,d=std::hypot(dx,dy);
        if(d>0)p.set_input(dx/d,dy/d);p.step();
    }
    expect(p.engine.ball_poss<12 && p.blue_wins>0 && p.blue_score==1,"User can regain possession after red kickoff without losing score");
    p.reset();expect(p.blue_score==0 && p.red_score==0 && !p.restart_ticks,"R starts a new scoreline");
    p.set_opponents(false);ball(p,1270,400,2,40);p.step();expect(p.outside && !p.blue_score,"Approved solo drill stays unchanged");
    std::cout<<"Passed "<<checks<<" scoring and restart checks.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
