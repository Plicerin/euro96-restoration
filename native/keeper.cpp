#include "player.hpp"
#include <algorithm>
#include <cmath>
namespace actua {
void PlayerPractice::reset_keepers(bool clear_stats) {
    keeper_holding=-1;keeper_hold_ticks=0;
    for(int side=0;side<2;++side){
        auto& k=keepers[std::size_t(side)];
        if(clear_stats)k=Keeper{};
        k.active=opponents;k.x=side==0?22.0f:1258.0f;k.y=400;k.cooldown=0;k.reaction=0;k.tracking=false;
    }
}
void PlayerPractice::move_keepers() {
    for(int side=0;side<2;++side){
        auto& k=keepers[std::size_t(side)];if(!k.active)continue;
        if(k.cooldown>0)--k.cooldown;
        if(keeper_holding==side)continue;
        float target=400+(engine.bally-400)*0.18f;
        const bool incoming=!engine.ball_poss && (side==0?engine.ballxdis<-0.1f:engine.ballxdis>0.1f);
        const float travel=incoming?(k.x-engine.ballx)/engine.ballxdis:0;
        const bool threatening=incoming && travel>0 && travel<24;
        if(threatening){
            if(!k.tracking)k.reaction=4; // A visible 0.2-second response, not perfect instant aim.
            if(k.reaction>0){--k.reaction;target=k.y;}
            else target=engine.bally+engine.ballydis*travel;
        }else k.reaction=0;
        k.tracking=threatening;
        target=std::clamp(target,364.0f,436.0f);
        k.y+=std::clamp(target-k.y,-2.2f,2.2f);
    }
}
void PlayerPractice::keeper_contact(int side,float x,float y,float z) {
    auto& k=keepers[std::size_t(side)];++k.saves;
    const int previous=engine.last_touch;
    const float speed=std::hypot(engine.ballxdis,engine.ballydis);
    engine.stop_ball_spin();engine.reset_shot();
    engine.last_touch=side==0?1:12;
    red_receiver=0;assisted_receiver=0;support_target_valid=false;tackle_window=0;
    await_input_release=false;input_x=held_input_x;input_y=held_input_y;
    if(speed<=18 && std::abs(y-k.y)<10 && z<22){
        keeper_holding=side;keeper_hold_ticks=20;
        engine.ballx=k.x+(side==0?8.0f:-8.0f);engine.bally=k.y;engine.ballz=14;
        engine.ballxdis=engine.ballydis=engine.ballzdis=0;engine.ball_inair=0;engine.ball_still=1;
        if(previous && (previous<12)!=(side==0)){if(side==0)++blue_wins;else ++red_wins;}
        status=side==0?"Blue keeper catches it - looking for an outlet.":"Red keeper catches it - cover the release.";
    }else{
        ++k.parries;k.cooldown=8;
        engine.ballx=x;engine.bally=y;engine.ballz=std::max(2.0f,z);
        engine.ballxdis=(side==0?1.0f:-1.0f)*std::clamp(speed*0.5f,6.0f,12.0f);
        engine.ballydis=(y>=k.y?1.0f:-1.0f)*6;engine.ballzdis=3;
        engine.ball_inair=1;engine.ball_still=0;
        status=side==0?"Blue keeper parries - loose ball!":"Red keeper parries - follow the rebound!";
    }
}
void PlayerPractice::keeper_release() {
    const int side=keeper_holding;auto& k=keepers[std::size_t(side)];
    const int first=side==0?1:12;
    int receiver=first;float best=-1e9f;
    for(int i:{first,first+1}){
        const auto& p=engine.teams[std::size_t(i)];float space=2000;
        for(int j:side==0?std::array<int,2>{12,13}:std::array<int,2>{1,2}){
            const auto& marker=engine.teams[std::size_t(j)];space=std::min(space,std::hypot(p.tm_x-marker.tm_x,p.tm_y-marker.tm_y));
        }
        const float score=space-0.35f*std::hypot(p.tm_x-k.x,p.tm_y-k.y);
        if(score>best){best=score;receiver=i;}
    }
    const auto& target=engine.teams[std::size_t(receiver)];
    float x=target.tm_x-engine.ballx,y=target.tm_y-engine.bally,speed=0,end_speed=5;
    engine.ballz=2;engine.chip_pass=0;
    engine.pass_ahead(receiver+1,x,y,speed,end_speed,0);
    const float d=std::max(1.0f,std::hypot(x,y));speed=float(end_speed+d*(1.0-0.965));
    engine.ballxdis=speed*x/d;engine.ballydis=speed*y/d;engine.ballzdis=0;
    engine.ball_inair=0;engine.ball_still=0;engine.launch_ball(1);
    engine.inc_pass(short(side==0?1:12));
    keeper_holding=-1;keeper_hold_ticks=0;k.cooldown=24;trail_size=0;
    if(side==0){
        select(receiver);assisted_receiver=receiver;receive_target={engine.ballx+x,engine.bally+y,0};receive_ticks=100;
        await_input_release=held_input_x!=0 || held_input_y!=0;input_x=input_y=0;
    }else{
        red_receiver=receiver;red_receive_target={engine.ballx+x,engine.bally+y,0};red_receive_ticks=100;
        red_pass_cooldown=48;++red_passes;
    }
    status=side==0?"Blue keeper releases to a teammate.":"Red keeper releases - intercept the outlet.";
}
}
