#include "original_actions.hpp"
#include "original_graphics.hpp"
#include <iostream>
#include <cmath>
#include <stdexcept>
static void require(bool condition,const char* message){if(!condition)throw std::runtime_error(message);}
int main(int argc,char** argv){try{
    actua::OriginalGraphics graphics(argv[1]);auto& s=*graphics.sequence;
    s.start_attack(argc>3?std::stof(argv[3]):400,argc>4?std::stof(argv[4]):415);
    while(s.running){s.step();if(argc>2){auto& e=s.engine;auto& p=e.teams[2];auto& q=e.teams[3];auto& k=e.teams[11];
        std::cout<<s.ticks<<","<<s.releases<<","<<s.first_touches<<","<<s.shots<<","<<s.saves<<","<<e.ball_poss<<","<<e.ballx<<","<<e.bally<<","<<e.ballz<<", p "<<p.tm_act<<","<<p.tm_x<<","<<p.tm_y<<","<<p.tm_strike<<","<<p.tm_ftime<<","<<p.go_cnt<<","<<p.tm_stime<<","<<p.tm_anim<<", q "<<q.tm_act<<","<<q.tm_strike<<","<<q.tm_ftime<<", k "<<k.tm_act<<","<<e.shot_pending<<","<<int(e.shot_acknowledged)<<","<<e.keeper_will_save<<","<<int(e.save_block)<<","<<k.tm_frm<<","<<e.keeper_contact<<",kp "<<k.tm_x<<","<<k.tm_y<<","<<e.save_xoff<<","<<e.save_yoff<<" "<<s.status<<'\n';}}
    std::cout<<"Attack: "<<s.releases<<" releases, "<<s.first_touches<<" first touches, "<<s.shots<<" shots, "<<s.saves<<" saves\n";
    if(argc>2)return s.shots?0:1;
    require(s.releases==3 && s.first_touches==2 && s.shots==1 && s.saves==1,"Original pass-pass-shot-save chain must complete once");
    require(s.engine.ball_poss==12 && s.engine.ball_in_hands,"Catch must give goalkeeper possession");
    require(s.engine.teams[11].tm_kpsaves==1 && s.engine.teams[3].tm_shots==1,"Original statistics must record the shot and save");
    require(s.engine.top_post_y==357 && s.engine.bot_post_y==442,"Watcom goal-post startup initialization must run");
    const float caught_x=s.engine.ballx;s.step();require(s.engine.ballx==caught_x,"Completed fixture must stay stable");
    // A nearby alternative keeper position must be allowed to miss; never grant a scripted save.
    s.start_attack(400,400);while(s.running)s.step();
    require(s.shots==1 && !s.saves && !s.engine.ball_poss && s.goal_mouth,"An earned goal-mouth crossing must remain a miss for the keeper");
    for(float y:{405.0f,425.0f,445.0f}){
        s.start_attack(400,y);while(s.running)s.step();
        require(s.shots==1 && s.saves==1 && s.engine.ball_poss==12,"Keeper-position regression must catch once");
    }
    // Original keeper contact gates: too early, too far, ball already in goal, then a legal catch.
    s.start_attack();auto& e=s.engine;auto& k=e.teams[11];e.holder_lose_ball();
    e.keeper_spd=1.5f;e.max_kp_margin=2;e.ballx=k.tm_x;e.bally=k.tm_y;e.ballz=2;
    e.save_in_zone_a(&k,k.tm_x,k.tm_y,2,5,0);
    e.ballx=k.tm_x+e.save_xoff;e.bally=k.tm_y+e.save_yoff;
    k.tm_frm=0;e.update_save_contact(&k);require(!e.ball_poss,"Save cannot occur before animation contact");
    k.tm_frm=e.keeper_contact;e.ballx+=100;e.update_save_contact(&k);
    require(!e.ball_poss && e.keeper_will_save==0,"Distant ball must downgrade a catch to a miss");
    e.save_in_zone_a(&k,k.tm_x,k.tm_y,2,5,0);e.ballx=k.tm_x+e.save_xoff;e.bally=k.tm_y+e.save_yoff;
    k.tm_frm=e.keeper_contact;e.ball_in_goal=1;e.update_save_contact(&k);require(!e.ball_poss,"Keeper cannot retrieve an already scored ball");
    e.ball_in_goal=0;e.save_in_zone_a(&k,k.tm_x,k.tm_y,2,5,0);
    e.ballx=k.tm_x+e.save_xoff;e.bally=k.tm_y+e.save_yoff;k.tm_frm=e.keeper_contact;e.update_save_contact(&k);
    require(e.ball_poss==12 && e.ball_in_hands,"Valid original catch must collect the ball");
    // Fast zone-A attempt selects the original block/rebound branch, with no possession award.
    e.holder_lose_ball();e.ballx=k.tm_x;e.bally=k.tm_y;e.ballz=2;e.ballxdis=12;e.ballydis=0;e.ballzdis=0;
    e.save_in_zone_a(&k,k.tm_x,k.tm_y,2,3,0);
    e.ballx=k.tm_x+e.save_xoff;e.bally=k.tm_y+e.save_yoff;k.tm_frm=e.keeper_contact;
    require(e.save_block && e.keeper_will_save>0,"Fast shot must select a viable block");
    e.update_save_contact(&k);require(!e.ball_poss && !e.ball_in_hands && e.keeper_will_save==-1 && e.ball_inair,"Original block must rebound and leave ball free");
    require(std::isfinite(e.ballxdis) && e.ballxdis<0,"Block must redirect the incoming shot");
    std::cout<<"Original attack decisions, two first touches, shot tracking, catches, missed save and block contracts passed.\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
