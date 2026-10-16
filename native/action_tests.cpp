#include "original_actions.hpp"
#include "original_graphics.hpp"
#include <iostream>
#include <cmath>
#include <stdexcept>
static void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
int main(int argc,char** argv){try{
    require(argc>=2,"Data path required");actua::OriginalGraphics graphics(argv[1]);auto& s=*graphics.sequence;
    for(bool moving:{false,true}){
        s.start(moving);
        bool ran=false,waited=false;float travelled=0;
        for(int i=0;i<180 && s.running;++i){
            const float x=s.engine.teams[2].tm_x,y=s.engine.teams[2].tm_y;
            s.step();auto& p=s.engine.teams[2];
            ran|=p.tm_act==1;waited|=p.tm_act==18;
            travelled+=std::hypot(p.tm_x-x,p.tm_y-y);
            if(argc>2)std::cout<<moving<<','<<s.ticks<<','<<s.engine.ball_poss<<','<<s.engine.ballx<<','<<s.engine.bally<<','<<s.engine.ballz<<','<<p.tm_act<<','<<p.tm_frm<<','<<p.tm_ftime<<','<<p.tm_x<<','<<p.tm_y<<','<<p.go_cnt<<','<<p.go_tx<<','<<p.go_ty<<','<<p.tm_stime<<','<<p.tm_strike<<','<<p.tm_xdis<<','<<p.tm_ydis<<','<<int(p.go_stop)<<'\n';
        }
        require(s.releases==1,"Kick must release exactly once");
        require(ran && travelled>10,"Receiver must run to the original interception point");
        require(s.engine.strike[1].t>0,"Original planner must select a trap candidate");
        if(moving)require(waited,"Running-foot pass must exercise original wait_action");
        require(s.receptions==1,"Original trap must receive the pass");
        require(s.engine.ball_poss==3,"Receiver must own the ball");
        require(s.engine.teams[2].tm_act==0 && s.engine.teams[2].tm_ftime==0,"Trap must finish its recovery");
        std::cout<<"Sequence "<<moving<<": release "<<s.release_tick<<", trap "<<s.receive_tick<<", complete "<<s.ticks<<'\n';
    }
    for(float offset:{-60.0f,-30.0f,30.0f,60.0f}){
        s.start(false,offset);
        for(int i=0;i<180 && s.running;++i)s.step();
        std::cout<<"Angled pass "<<offset<<": receptions "<<s.receptions<<", complete "<<s.ticks<<'\n';
        require(s.releases==1 && s.receptions==1 && !s.miss,"Angled original interception must complete");
    }
    // No reachable candidate: the original planner must not invent a catch.
    s.reset();auto& planner=s.engine;auto& target=planner.teams[2];
    planner.holder_lose_ball();planner.user_controlled=target.control=1;planner.uf2=1;planner.uf1=planner.um=0;
    target.tm_x=1200;target.tm_y=700;
    for(auto& sample:planner.ball_pred_tab)sample={100,100,2};
    require(!planner.go_to_path(3),"Unreachable trajectory must have no interception");
    require(!target.tm_strike && !planner.ball_poss,"Failed plan must leave the ball free");
    for(auto& sample:planner.ball_pred_tab)sample={-10,400,2};
    require(!planner.go_to_path(3),"Out-of-bounds path must not produce an interception");
    // An intervening touch invalidates the already selected original run/wait plan.
    s.start();while(!s.releases)s.step();
    require(s.engine.teams[2].tm_strike>0,"Cancellation fixture requires a pending interception");
    s.engine.last_touch=14;s.step();
    require(s.engine.teams[2].tm_act==0 && !s.engine.teams[2].tm_strike && !s.receptions,"Changed last touch must cancel the interception");
    // Prediction samples match free-flight updates without changing the live ball position/velocity.
    s.start();while(!s.releases)s.step();
    auto flight=static_cast<actua::original::BallEngine&>(s.engine);flight.prediction=1;
    s.engine.predict_ball();
    require(s.engine.ballx==flight.ballx && s.engine.bally==flight.bally && s.engine.ballxdis==flight.ballxdis,"Predictor must restore live state");
    for(int i=0;i<50;++i){const auto& sample=s.engine.ball_pred_tab[std::size_t(i)];
        require(std::abs(sample[0]-flight.ballx)<.001f && std::abs(sample[1]-flight.bally)<.001f && std::abs(sample[2]-flight.ballz)<.001f,"Prediction must follow original trajectory");flight.ball_trajectory();}
    // Contact must be earned at the original animation phase and within its reach.
    s.reset();auto& e=s.engine;auto& receiver=e.teams[2];
    e.init_trap_anim(&receiver);receiver.contact=2.0f/3;receiver.tm_frm=receiver.contact-0.01f;
    e.ball_at_contact(&receiver);
    require(!e.control_interact(&receiver),"Trap must not catch before contact");
    receiver.tm_frm=receiver.contact;
    require(e.control_interact(&receiver),"Exact contact should be accepted");
    e.ballx+=30;require(!e.control_interact(&receiver),"Trap must not magnetize a distant ball");
    e.ball_at_contact(&receiver);e.ballz+=30;require(!e.control_interact(&receiver),"Trap must reject a ball above reach");
    // Losing possession during wind-up cancels the kick and cannot create a second ball release.
    s.start();s.engine.holder_lose_ball();s.engine.kick_action(&s.engine.teams[1]);
    require(s.engine.teams[1].tm_act==0 && s.engine.ball_poss==0,"Stolen ball must cancel kick");
    // Recovery uses original mocap coordinates, rotated into world space.
    s.reset();auto& p=s.engine.teams[2];p.ls_anim=54;p.ls_frm=.9f;p.tm_xdis=0;p.tm_ydis=1;
    const auto& m=s.engine.mcaps[54];const auto& f=s.engine.player_p[std::size_t(m.cappts+int(p.ls_frm*m.capfrms))];
    require(f[71]>0,"Trap recovery fixture needs a captured ball");s.engine.get_mcball_coords(&p);
    require(std::abs(s.engine.ballx-(p.tm_x+f[72]))<.001f && std::abs(s.engine.bally-(p.tm_y+f[70]))<.001f,"Captured ball must rotate with receiver");
    // The displayed capture and the original gameplay lookup must agree in every direction.
    // This catches a sign error anywhere in the gameplay -> render object -> captured ball chain.
    p.tm_z=0;
    for(int direction=0;direction<8;++direction){
        const float angle=float(direction)*3.1415926536f/4;
        p.tm_xdis=std::cos(angle);p.tm_ydis=std::sin(angle);
        s.engine.get_mcball_coords(&p);
        auto& pose=s.engine.player_p[std::size_t(m.cappts+int(p.ls_frm*m.capfrms))];
        auto object=actua::OriginalGraphics::player_object(pose.data(),nullptr,p);
        const auto ball=actua::OriginalGraphics::captured_ball(object,false);
        require(std::abs(ball[0]-s.engine.ballx)<.001f && std::abs(ball[1]-s.engine.ballz)<.001f && std::abs(ball[2]+s.engine.bally)<.001f,"Rendered ball and original gameplay facing disagree");
        // Mirroring must retain the forward component, while reversing only the lateral offset.
        const auto mirrored=actua::OriginalGraphics::captured_ball(object,true);
        const float forward=(mirrored[0]-p.tm_x)*p.tm_xdis+(-mirrored[2]-p.tm_y)*p.tm_ydis;
        require(std::abs(forward-pose[70])<.001f,"Mirrored pose must keep its forward direction");
    }
    std::cout<<"Original pass and trap contracts passed.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
