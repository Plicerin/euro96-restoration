#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "action_constants.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <cstdlib>
using namespace actua;
using namespace actua::original::action_constants;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
    OriginalGraphics g(argv[1]);auto& s=*g.sequence;auto& e=s.engine;
    struct Case{float x,y;int touch,mode;};
    const Case cases[]={
        {640,-2,2,THROW_IN_B},{640,802,13,THROW_IN_A},
        {-2,200,2,CORNER_TL},{-2,600,2,CORNER_BL},
        {1282,200,13,CORNER_TR},{1282,600,13,CORNER_BR},
        {-2,200,13,GOAL_KICK_TL},{-2,600,13,GOAL_KICK_BL},
        {1282,200,2,GOAL_KICK_TR},{1282,600,2,GOAL_KICK_BR}};
    for(const auto& c:cases){
        s.start_team_play(true);e.holder_lose_ball();
        e.ballx=c.x;e.bally=c.y;e.ballz=2;e.ballxdis=e.ballydis=e.ballzdis=0;e.last_touch=c.touch;
        s.step();require(s.restart_mode==c.mode,"Original boundary rules chose wrong restart or side");
        const int start=s.ticks;int taker=0;bool released=false;
        for(int i=0;i<600;++i){
            s.step();
            if(e.match_mode)taker=e.setp_taker;
            if(argc>2 && c.mode==CORNER_TL && i==100){std::cout<<"piece "<<int(e.set_piece_on)<<" wait "<<e.ref_wait<<" shoot "<<int(e.must_shoot)<<" srng "<<int(e.teams[taker-1].tm_srng)<<"\n";for(int j=12;j<22;++j)std::cout<<j+1<<" @ "<<e.teams[j].tm_x<<','<<e.teams[j].tm_y<<'\n';}
            if(argc>2 && taker && i%40==0){auto& p=e.teams[taker-1];std::cout<<i<<" mode "<<int(e.match_mode)<<" dead "<<e.dead_ball_cnt<<" owner "<<e.ball_poss<<" act "<<p.tm_act<<" dir "<<int(p.dir_mode)<<" face "<<p.tm_xdis<<','<<p.tm_ydis<<" target "<<p.go_txdis<<','<<p.go_tydis<<" throw "<<e.throw_cnt<<" must "<<int(e.must_pass)<<"\n";}
            if(taker && (e.teams[taker-1].tm_pass || e.teams[taker-1].tm_shots || e.ball_limbo_on)){released=true;break;}
        }
        std::cout<<OriginalActions::restart_name(c.mode)<<": taker "<<taker<<" release after "<<s.ticks-start<<" ticks\n";
        require(released && !e.match_mode && !s.restart_ticks,"Restart must release automatically through original action");
        if(c.mode==THROW_IN_A || c.mode==THROW_IN_B)require(!s.distributions,"Throw-ins must not count as keeper deliveries");
        require(!s.blue_goals && !s.red_goals,"Out of play must not count a goal");
        require((c.mode==THROW_IN_A || c.mode==CORNER_TR || c.mode==CORNER_BR || c.mode==GOAL_KICK_TL || c.mode==GOAL_KICK_BL)==(taker<12),"Restart taker belongs to wrong team");
        for(int i=0;i<80;++i)s.step();
        require(s.running,"Play must continue after restart contact");
    }
    for(bool left:{false,true}){
        s.start_team_play(true);e.holder_lose_ball();
        e.ballx=left?2.0f:1278.0f;e.bally=400;e.ballz=12;e.ballxdis=left?-8.0f:8.0f;e.ballydis=e.ballzdis=0;e.ball_inair=1;
        e.prev_ballx=e.ballx;e.prev_bally=e.bally;e.prev_ballz=e.ballz;
        s.step();require(s.blue_goals==int(!left) && s.red_goals==int(left),"Goal must count once for the attacking end");
        require(s.restart_mode==(left?CENTRE_A:CENTRE_B),"Conceding side must take kickoff");
        for(int i=0;i<60;++i)s.step();
        require(s.blue_goals+s.red_goals==1,"Net rebounds must not count another goal");
        require(e.match_mode==(left?CENTRE_A:CENTRE_B),"Goal must prepare original centre formation");
        int taker=e.setp_taker,receiver=e.centre_guy_2;
        for(int i=0;i<120 && e.match_mode;++i)s.step();
        require(!e.match_mode && e.teams[taker-1].tm_act==KICK_ACT && e.receiver==receiver,"Original kickoff must wind up a pass to the second centre player");
        for(int i=0;i<50 && !e.teams[taker-1].tm_pass;++i)s.step();
        require(e.teams[taker-1].tm_pass==1,"Kickoff must release at original animation contact");
    }
    for(int keeper:{0,11}){
        s.start_team_play(true);e.holder_lose_ball();auto& k=e.teams[keeper];
        k.tm_x=keeper?1180.0f:100.0f;k.tm_y=400;k.tm_xdis=k.mface_x=keeper?-1.0f:1.0f;k.tm_ydis=k.mface_y=0;
        e.collect_ball(&k);e.ball_in_hands=1;e.init_kphold_act(&k);e.init_anim(&k,MC_STANDB);e.hold_ball(&k);
        bool released=false;
        for(int i=0;i<600;++i){s.step();if(s.distributions){released=true;break;}}
        require(released && !e.ball_poss && e.ball_limbo_on,"Both live keepers must distribute with animation contact delay");
    }
    int saved[2]{};
    for(int side=0;side<2;++side)for(int trial=0;trial<6;++trial){
        s.start_team_play(true);e.holder_lose_ball();auto& k=e.teams[side?11:0];
        // Incoming-shot fixtures exercise both mirrored keeper AI branches.
        k.tm_x=side?1200.0f:80.0f;k.tm_y=400;
        e.ballx=side?1080.0f:200.0f;e.bally=400+float(trial-3)*3;e.ballz=12;
        e.ballxdis=side?8.0f:-8.0f;e.ballydis=0;e.ballzdis=1;e.ball_inair=1;e.shot_pending=1;e.real_shot=1;e.last_touch=side?9:20;
        e.launch_ball(1);
        for(int i=0;i<100 && !s.restart_ticks;++i)s.step();
        saved[side]+=k.tm_kpsaves;
    }
    std::cout<<"Live keeper saves "<<saved[0]<<'/'<<saved[1]<<'\n';
    require(saved[0]>0 && saved[1]>0,"Both keepers must react and save live incoming shots");
    int restarts=0;
    for(int seed=1;seed<=8;++seed){
        s.start_team_play(true);std::srand(seed);
        for(int i=0;i<6000;++i){
            s.step();require(s.running,"CPU match must continue without manual reset");
            require(std::isfinite(e.ballx) && std::isfinite(e.bally) && std::isfinite(e.ballz),"Non-finite live or dead match ball");
            for(int p:s.visible_players)require(std::isfinite(e.teams[p].tm_x) && std::isfinite(e.teams[p].tm_y),"Non-finite match player");
        }
        restarts+=s.restarts;
        std::cout<<"seed "<<seed<<" score "<<s.blue_goals<<':'<<s.red_goals<<" restarts "<<s.restarts<<" saves "<<s.saves<<" deliveries "<<s.distributions<<" pending "<<s.restart_mode<<'\n';
    }
    require(restarts>8,"Long CPU runs must encounter automatic restarts");
    s.restart_play();require(s.cpu_vs_cpu && !s.blue_goals && !s.red_goals && !s.restarts,"Manual reset must clear match score and retain CPU mode");
    std::cout<<"Live keeper, distribution, goal/kickoff and all boundary restart contracts passed\n";
    return 0;
}catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 1;}}
