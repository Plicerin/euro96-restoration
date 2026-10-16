#include "player.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using actua::PlayerPractice;
static int checks=0;
static void expect(bool v,const char* m){++checks;if(!v)throw std::runtime_error(m);}
static PlayerPractice fresh(){PlayerPractice p;p.set_opponents(true);return p;}
static void last_tick_goal(PlayerPractice& p,bool blue){
    p.engine.holder_lose_ball();p.keepers[blue?1:0].active=false;
    p.engine.ballx=blue?1279.0f:1.0f;p.engine.bally=400;p.engine.ballz=2;
    p.engine.ballxdis=blue?20.0f:-20.0f;p.engine.ballydis=p.engine.ballzdis=0;
    p.engine.ball_inair=0;p.engine.ball_still=0;p.engine.last_touch=blue?2:13;
    p.match_elapsed_ticks=PlayerPractice::match_duration_ticks-1;p.step();
}
int main(){try{
    auto p=fresh();expect(p.match_seconds_left()==180 && !p.match_finished,"New match starts at three minutes");
    for(int i=0;i<19;++i)p.step();expect(p.match_seconds_left()==180,"Partial second rounds up on display");
    p.step();expect(p.match_seconds_left()==179 && p.match_elapsed_ticks==20,"Clock advances at twenty ticks per second");
    p.toggle_pause();const auto paused_ticks=p.ticks;
    for(int i=0;i<100;++i)p.step();expect(p.match_elapsed_ticks==20 && p.ticks==paused_ticks,"Pause freezes clock and simulation");
    p.toggle_pause();p.step();expect(p.match_elapsed_ticks==21,"Resuming continues same match");
    p=fresh();last_tick_goal(p,true);
    expect(p.match_finished && p.blue_score==1 && p.match_result()=="BLUE WINS","Final-tick blue goal counts before full time");
    expect(p.match_seconds_left()==0 && !p.restart_ticks,"Full time takes priority over goal restart");
    auto ended=p;const auto result=p.status;
    expect(!p.pass() && !p.pass(true) && !p.shoot() && !p.challenge() && !p.action(),"Full time rejects ball actions");
    p.switch_player();p.aim_at(0,0);p.set_input(1,1);p.set_tackle_held(true);p.toggle_pause();
    for(int i=0;i<100;++i)p.step();
    expect(p.status==result && p.ticks==ended.ticks && p.selected==ended.selected && !p.paused,"Full time cannot be resumed or overwritten by actions");
    expect(p.engine.ballx==ended.engine.ballx && p.engine.teams[1].tm_xdis==ended.engine.teams[1].tm_xdis && !p.tackling(),"Result screen freezes ball, aim and tackles");
    p.reset();expect(p.opponents && !p.match_finished && !p.blue_score && !p.red_score && p.match_elapsed_ticks==0 && !p.keepers[0].saves,"Rematch clears score, clock and statistics");
    p=fresh();last_tick_goal(p,false);expect(p.red_score==1 && p.match_result()=="RED WINS","Final-tick red goal counts");
    p=fresh();p.blue_score=1;last_tick_goal(p,false);expect(p.match_result()=="DRAW","Final-tick equalizer produces draw");
    p=fresh();p.match_elapsed_ticks=10;
    p.engine.holder_lose_ball();p.engine.ballx=1279;p.engine.bally=400;p.engine.ballz=2;
    p.engine.ballxdis=20;p.engine.ballydis=p.engine.ballzdis=0;p.engine.ball_inair=0;p.engine.ball_still=0;p.keepers[1].active=false;p.step();
    expect(p.restart_ticks==40 && p.match_elapsed_ticks==11,"Scoring tick consumes clock");
    for(int i=0;i<40;++i)p.step();expect(!p.restart_ticks && p.match_elapsed_ticks==11,"Goal celebration and kickoff stop clock");
    p.step();expect(p.match_elapsed_ticks==12,"Clock resumes after kickoff");
    p=fresh();p.engine.holder_lose_ball();p.engine.ballx=600;p.engine.bally=1;p.engine.ballz=2;
    p.engine.ballxdis=0;p.engine.ballydis=-20;p.engine.ballzdis=0;p.engine.ball_inair=0;p.engine.ball_still=0;p.step();
    expect(p.restart_ticks==20,"Out-of-play fixture starts restart");
    for(int i=0;i<20;++i)p.step();expect(p.match_elapsed_ticks==1,"Out-of-play restart stops clock");
    p=fresh();p.engine.holder_lose_ball();p.keeper_holding=0;p.keeper_hold_ticks=20;p.step();
    expect(p.match_elapsed_ticks==1 && p.keeper_hold_ticks==19,"Keeper possession consumes active match time");
    p.match_elapsed_ticks=PlayerPractice::match_duration_ticks-1;p.step();
    const int held=p.keeper_hold_ticks;for(int i=0;i<100;++i)p.step();
    expect(p.match_finished && p.keeper_hold_ticks==held,"Full time freezes an in-progress keeper hold");
    p.set_opponents(false);for(int i=0;i<4000;++i)p.step();
    expect(!p.match_finished && !p.match_elapsed_ticks,"Solo practice remains untimed");
    p.set_opponents(true);expect(p.match_seconds_left()==180 && !p.match_finished,"Returning to opponents starts fresh match");

    // Six complete games: idle human, chase/tackle bot, alternating ground and
    // loft passing, attacking/shooting, switching, and repeated pause/resume.
    // These use public controls and normal AI, with no scripted ball ownership.
    int goals=0,saves=0,red_passes=0,regains=0,total_steps=0;
    for(int style=0;style<6;++style){
        p=fresh();int consecutive_hold=0,consecutive_restart=0,steps=0;
        for(;steps<8000 && !p.match_finished;++steps){
            if(style){
                const auto& b=p.engine.teams[std::size_t(p.selected)];
                const bool owns=p.engine.ball_poss==p.selected+1;
                float dx=owns?1280-b.tm_x:p.engine.ballx-b.tm_x;
                float dy=owns?(style%2?430.0f:370.0f)-b.tm_y:p.engine.bally-b.tm_y;
                const float d=std::hypot(dx,dy);p.set_input(d>1?dx/d:0,d>1?dy/d:0);p.set_tackle_held(true);
                if(style>=2 && owns && steps%120==0)p.pass(style==2 && steps%240==0);
                if(style>=3 && owns && b.tm_x>950){p.aim_at(1280,style%2?436.0f:364.0f);p.shoot();}
                if(style==4 && steps%70==0)p.switch_player();
                if(style==5 && steps%100==0){
                    p.toggle_pause();const int t=p.match_elapsed_ticks;for(int j=0;j<20;++j)p.step();
                    expect(p.match_elapsed_ticks==t,"Soak pause preserves clock");p.toggle_pause();
                }
            }
            p.step();
            expect(std::isfinite(p.engine.ballx) && std::isfinite(p.engine.bally) && std::isfinite(p.engine.ballz),"Soak ball remains finite");
            expect(p.selected==1 || p.selected==2,"Soak control stays with blue outfield");
            const auto owner=p.engine.ball_poss;
            expect(owner==0 || owner==2 || owner==3 || owner==13 || owner==14,"Soak possession remains a valid outfield player");
            for(int i:PlayerPractice::practice_players){const auto& b=p.engine.teams[std::size_t(i)];expect(std::isfinite(b.tm_x) && std::isfinite(b.tm_y),"Soak players remain finite");}
            consecutive_hold=p.keeper_holding>=0?consecutive_hold+1:0;
            consecutive_restart=p.restart_ticks?consecutive_restart+1:0;
            expect(consecutive_hold<=20,"Keeper cannot get stuck holding ball");
            expect(consecutive_restart<=40,"Restart cannot become stuck");
        }
        expect(p.match_finished && p.match_elapsed_ticks==PlayerPractice::match_duration_ticks,"Unattended full game reaches full time");
        expect(steps>=3600 && steps<8000,"All active ticks played with bounded stoppages");
        std::cout<<"Soak "<<style<<": "<<steps<<" ticks, score "<<p.blue_score<<"-"<<p.red_score
            <<", regains "<<p.blue_wins<<", red passes "<<p.red_passes<<", saves "<<p.keepers[0].saves+p.keepers[1].saves<<'\n';
        goals+=p.blue_score+p.red_score;saves+=p.keepers[0].saves+p.keepers[1].saves;
        red_passes+=p.red_passes;regains+=p.blue_wins;total_steps+=steps;
        p.reset();expect(p.match_elapsed_ticks==0 && !p.match_finished && p.engine.ball_poss==2,"Every completed game can rematch");
    }
    expect(goals>0 && saves>0 && red_passes>0 && regains>0,"Long scenarios exercise goals, saves, red passes and blue recoveries");
    std::cout<<"Passed "<<checks<<" match checks across "<<total_steps<<" simulation ticks (18 active match minutes).\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
