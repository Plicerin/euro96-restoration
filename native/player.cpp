#include "player.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace actua {
static constexpr float pi=3.1415926536f;
PlayerPractice::PlayerPractice() { reset(); }
void PlayerPractice::select(int index) {
    selected=index;
    engine.teams[1].control=index==1?1:0;
    engine.teams[2].control=index==2?1:0;
}
void PlayerPractice::reset() {
    match_elapsed_ticks=0;match_finished=false;
    blue_score=red_score=restart_ticks=0;restart_goal=restart_red=false;
    engine=original::PlayerEngine{};
    for(int i=0;i<22;++i) engine.teams[std::size_t(i)].tm_player=short(i+1);
    for(int i:practice_players) {
        auto& p=engine.teams[std::size_t(i)];
        p.tm_x=i==1?500.0f:740.0f; p.tm_y=400;
        p.tm_xdis=i==1?1.0f:-1.0f;
        p.tm_rate=96;p.tm_pow=96;p.tm_cont=96;p.tm_ac=96;p.tm_flair=96;
        if(i>=12) {
            p.tm_x=i==12?590.0f:820.0f;p.tm_y=i==12?270.0f:530.0f;
            p.tm_xdis=-1;p.tm_ydis=0;p.tm_rate=64;
        }
        p.guy_on=(i<12 || opponents)?1:0;p.draw_me=static_cast<char>(p.guy_on);
    }
    reset_keepers(true);red_shot_cooldown=0;
    engine.user_controlled=1;
    select(1);engine.collect_ball(&engine.teams[1]);
    opponent_presser=-1;opponent_role_ticks=0;opponent_holder=0;dribble_ticks=0;
    ticks=0;cooldown=0;kicker=0;passes_received=0;outside=false;paused=false;
    input_x=input_y=held_input_x=held_input_y=0;await_input_release=false;trail_size=0;
    support_target_valid=false;assisted_receiver=0;receive_ticks=0;
    tackle_window=0;tackle_held=false;
    red_passes=red_completed=0;red_receiver=red_receive_ticks=red_pass_cooldown=red_decision_ticks=0;
    blue_wins=red_wins=0;possession_grace=0;tackle_cooldown.fill(0);opponent_windup.fill(0);
    status=opponents?"Blue attacks the RIGHT goal. Click toward goal, then X to shoot.":
        "Move with arrows or WASD. Space passes to your teammate.";
}
void PlayerPractice::set_opponents(bool enabled) { opponents=enabled;reset(); }
bool PlayerPractice::action() {
    return opponents && engine.ball_poss!=selected+1?challenge():shoot();
}
void PlayerPractice::set_tackle_held(bool down) { tackle_held=down && opponents && !match_finished; }
bool PlayerPractice::challenge() {
    if(match_finished || paused || outside || restart_ticks || !opponents) return false;
    if(engine.ball_poss<12) return false;
    if(tackle_cooldown[std::size_t(selected)]) {status="Tackle recovering - keep moving.";return false;}
    tackle_cooldown[std::size_t(selected)]=12;tackle_window=8;
    if(try_challenge(selected)) return true;
    status="Tackling - keep moving toward the ball. Hold X to keep challenging.";
    return false;
}
bool PlayerPractice::try_challenge(int index) {
    if(possession_grace || !engine.ball_poss || (index<11)==(engine.ball_poss<12)) return false;
    const auto& p=engine.teams[std::size_t(index)];
    const float dx=engine.ballx-p.tm_x,dy=engine.bally-p.tm_y,d=std::hypot(dx,dy);
    // Human tackle reach matches the visible player size; reds retain their
    // existing short reach and wind-up. Contact is still local and forward-facing.
    const float reach=index<11?40.0f:26.0f, facing=index<11?0.0f:0.25f;
    if(engine.ballz>=12 || d>reach || (d>1 && (dx*p.tm_xdis+dy*p.tm_ydis)/d<facing)) return false;
    gain_ball(index,true);
    return true;
}
void PlayerPractice::gain_ball(int index,bool tackle) {
    const bool red_pass_received=!tackle && index==red_receiver && red_receiver!=0;
    if(red_pass_received)++red_completed;
    red_receiver=red_receive_ticks=0;
    assisted_receiver=0;support_target_valid=false;tackle_window=0;
    // Only protect the receiver while the ball is in flight. Restore the latest
    // held direction at the catch (or interception), ready for the next tick.
    // Do not move twice on the contact tick or alter the ball's trajectory.
    if(await_input_release) {input_x=held_input_x;input_y=held_input_y;}
    await_input_release=false;
    const int previous=engine.ball_poss?engine.ball_poss:engine.last_touch;
    if(engine.ball_poss) engine.holder_lose_ball();
    auto& p=engine.teams[std::size_t(index)];
    if(tackle) p.tm_act=3; // Restored collect_ball records tackles won.
    engine.collect_ball(&p);
    if(tackle) p.tm_act=0;
    cooldown=0;possession_grace=12;opponent_windup.fill(0);
    if(previous && (previous<12)!=(index<11)) {
        if(index<11) ++blue_wins;else ++red_wins;
    }
    if(index<11) {
        select(index);
        if(!tackle) ++passes_received;
        status=tackle?"You won the ball - move or pass!":"Ball received - move, pass, or shoot.";
    } else status=red_pass_received?"Red completed a pass. Close down or cover the next receiver.":tackle?"Red won the ball. Chase while holding X to tackle.":
        "Red intercepted! Chase and hold X; E switches teammate.";
}
void PlayerPractice::set_input(float x,float y) {
    if(match_finished) return;
    if(!std::isfinite(x)||!std::isfinite(y)) {x=0;y=0;}
    const float length=std::sqrt(x*x+y*y);
    if(length>1) {x/=length;y/=length;}
    held_input_x=x;held_input_y=y;
    if(restart_ticks) {input_x=input_y=0;return;}
    if(await_input_release) {
        if(x==0 && y==0) await_input_release=false;
        input_x=input_y=0;
        return;
    }
    if(x!=0 || y!=0) assisted_receiver=0; // Fresh input overrides the receiving run.
    input_x=x;input_y=y;
}
void PlayerPractice::aim_at(float x,float y) {
    if(match_finished || restart_ticks) return;
    auto& p=engine.teams[std::size_t(selected)];
    float dx=x-p.tm_x,dy=y-p.tm_y,d=std::hypot(dx,dy);
    if(d>1 && std::isfinite(d)) {p.tm_xdis=dx/d;p.tm_ydis=dy/d;}
}
void PlayerPractice::switch_player() { if(match_finished || restart_ticks) return; tackle_window=0;await_input_release=false;assisted_receiver=0;support_target_valid=false;select(selected==1?2:1); }
void PlayerPractice::release(int receiver) {
    assisted_receiver=0;support_target_valid=false;
    kicker=engine.ball_poss;cooldown=10;
    engine.holder_lose_ball();
    engine.ball_still=0;engine.launch_ball(1);
    trail_size=0;
    if(receiver) {
        // The passer's held direction must not move the receiver away from the pass.
        await_input_release=await_input_release || held_input_x!=0 || held_input_y!=0;
        input_x=input_y=0;
        select(receiver-1);
    }
}
bool PlayerPractice::pass(bool loft) {
    if(match_finished || restart_ticks) return false;
    if(paused||outside||engine.ball_poss!=selected+1) {status="Get the ball with the selected player first.";return false;}
    const int receiver=selected==1?3:2;
    const auto& target=engine.teams[std::size_t(receiver-1)];
    float x=target.tm_x-engine.ballx,y=target.tm_y-engine.bally;
    if(std::hypot(x,y)<16) {status="Move farther from your teammate before passing.";return false;}
    float pass_speed=0,end_speed=5;
    engine.chip_pass=loft?1:0;
    engine.pass_ahead(receiver,x,y,pass_speed,end_speed,0);
    float d=std::hypot(x,y);
    engine.inc_pass(short(engine.ball_poss));engine.stop_ball_spin();engine.reset_shot();
    // Selected branches of INTELL.CPP pass_ball: practice uses accurate passes,
    // with explicit ground/lob selection and no kick animation or set-piece logic.
    engine.ball_inair=loft?1:0;
    if(loft) {
        pass_speed=float(end_speed+4+d*(1.0-0.986));
        // Match the discrete move-then-gravity steps so the ball descends at the
        // target. Cap flight time as well as height: capping vertical speed alone
        // made long lobs bounce early and sail over the receiver on the rebound.
        const int max_ticks=std::max(2,int(1+28.0f/engine.gravity));
        const int t=std::clamp(int(std::ceil(std::log((end_speed+4)/pass_speed)/std::log(0.986))),2,max_ticks);
        pass_speed=float(d*(1.0-0.986)/(1.0-std::pow(0.986,t)));
        engine.ballzdis=0.5f*engine.gravity*float(t-1)+(engine.ball_diam/2-engine.ballz)/float(t);
    } else {
        pass_speed=float(end_speed+d*(1.0-0.965));engine.ballzdis=0;engine.ballz=2;
    }
    engine.ballxdis=pass_speed*x/d;engine.ballydis=pass_speed*y/d;
    const BallPoint destination{engine.ballx+x,engine.bally+y,0};
    release(receiver);
    if(opponents) {assisted_receiver=receiver-1;receive_target=destination;receive_ticks=100;}
    status=opponents?"Pass away - teammate meets it. Hold direction to run on reception.":loft?"Lofted pass - hold direction to run on reception.":"Ground pass - hold direction to run on reception.";
    return true;
}
bool PlayerPractice::shoot() {
    if(match_finished || restart_ticks) return false;
    if(paused||outside||engine.ball_poss!=selected+1) {status="Get the ball with the selected player first.";return false;}
    auto& p=engine.teams[std::size_t(selected)];
    engine.inc_shot(short(engine.ball_poss));engine.stop_ball_spin();engine.reset_shot();
    // Explicit diagnostic shot: facing direction, power 20, vertical velocity 6.
    // This legacy practice view retains its prototype shot; OriginalActions uses restored shooting.
    engine.ballxdis=p.tm_xdis*20;engine.ballydis=p.tm_ydis*20;engine.ballzdis=6;engine.ball_inair=1;
    release(0);status="Shot released. Move to receive it, or R to reset.";return true;
}
bool PlayerPractice::receive(float px,float py,float pz,const std::array<BallPoint,22>& previous_players,float max_time) {
    if(engine.ball_poss || keeper_holding>=0) return false;
    // Native practice assistance: 16 world units of foot reach, below height 12.
    // Sweep relative ball/player motion and intersect the low-height time window.
    // This changes contact only: a released ball never homes toward its receiver.
    constexpr float reach=16, height=12;
    int receiver=-1;float earliest=2;
    for(int i:{0,1,2,11,12,13}) {
        const bool keeper=i==0 || i==11;
        const int side=i==11?1:0;
        if(keeper && (!opponents || !keepers[std::size_t(side)].active || keepers[std::size_t(side)].cooldown))continue;
        if(i>=11 && !opponents) continue;
        auto& p=engine.teams[std::size_t(i)];
        if(cooldown && p.tm_player==kicker) continue;
        const auto& previous=previous_players[std::size_t(i)];
        const float target_x=keeper?keepers[std::size_t(side)].x:p.tm_x;
        const float target_y=keeper?keepers[std::size_t(side)].y:p.tm_y;
        // Only a ball approaching from the field can be saved; never reach back
        // through the goal line to cancel a shot that already beat the keeper.
        if(keeper && (side==0?(px<target_x || engine.ballxdis>=0):(px>target_x || engine.ballxdis<=0)))continue;
        const float radius=keeper?14.0f:reach,contact_height=keeper?28.0f:height;
        const float rx=px-previous.x,ry=py-previous.y;
        const float dx=engine.ballx-px-(target_x-previous.x);
        const float dy=engine.bally-py-(target_y-previous.y);
        const float a=dx*dx+dy*dy,b=rx*dx+ry*dy,c=rx*rx+ry*ry-radius*radius;
        float enter=0,leave=max_time;
        if(a>0.000001f) {
            const float discriminant=b*b-a*c;
            if(discriminant<0) continue;
            const float root=std::sqrt(discriminant);
            enter=std::max(0.0f,(-b-root)/a);
            leave=std::min(max_time,(-b+root)/a);
        } else if(c>0) continue;
        const float dz=engine.ballz-pz;
        if(dz<0) enter=std::max(enter,(contact_height-pz)/dz);
        else if(dz>0) leave=std::min(leave,(contact_height-pz)/dz);
        else if(pz>=contact_height) continue;
        if(enter<=leave && enter<earliest) {receiver=i;earliest=enter;}
    }
    if(receiver<0)return false;
    if(receiver==0 || receiver==11) keeper_contact(receiver==11?1:0,
        px+(engine.ballx-px)*earliest,py+(engine.bally-py)*earliest,pz+(engine.ballz-pz)*earliest);
    else gain_ball(receiver,false);
    return true;
}
void PlayerPractice::move_player(int i,float x,float y) {
    auto& p=engine.teams[std::size_t(i)];p.go_txdis=p.go_tydis=0;
    if(x==0 && y==0) {p.tm_act=0;p.tm_frm=0;return;}
    const float desired=std::atan2(y,x),current=std::atan2(p.tm_ydis,p.tm_xdis);
    const float difference=std::remainder(desired-current,2*pi);
    const float max_turn=(float(p.tm_rate)/8+14)*pi/180;
    const float angle=current+std::clamp(difference,-max_turn,max_turn);
    p.tm_xdis=std::cos(angle);p.tm_ydis=std::sin(angle);
    p.go_tx=p.tm_x+x*256;p.go_ty=p.tm_y+y*256;p.tm_act=1;
    const float ox=p.tm_x,oy=p.tm_y;
    engine.go_forward(&p);
    // Native analog throttle; unit keyboard/AI inputs retain the original speed.
    const float throttle=std::min(1.0f,std::hypot(x,y));
    p.tm_x=ox+(p.tm_x-ox)*throttle;p.tm_y=oy+(p.tm_y-oy)*throttle;
    p.tm_x=std::clamp(p.tm_x,12.0f,float(engine.pitch_len)-12);
    p.tm_y=std::clamp(p.tm_y,12.0f,float(engine.pitch_wid)-12);
    p.go_txdis=p.tm_x-ox;p.go_tydis=p.tm_y-oy;
    p.tm_frm=std::fmod(p.tm_frm+0.08f,1.0f);
}
static float segment_distance(float x,float y,float ax,float ay,float bx,float by) {
    const float dx=bx-ax,dy=by-ay,len2=dx*dx+dy*dy;
    const float t=len2>0?std::clamp(((x-ax)*dx+(y-ay)*dy)/len2,0.0f,1.0f):0;
    return std::hypot(x-ax-t*dx,y-ay-t*dy);
}
float PlayerPractice::support_score(int index,float x,float y,float ax,float ay) const {
    const auto& p=engine.teams[std::size_t(index)];
    const float spacing=std::hypot(x-ax,y-ay);
    if(x<64 || x>float(engine.pitch_len)-64 || y<64 || y>float(engine.pitch_wid)-64 || spacing<150 || spacing>340)
        return -10000;
    float clearance=220,lane=140,route=100;
    for(int i:{12,13}) {
        const auto& defender=engine.teams[std::size_t(i)];
        clearance=std::min(clearance,std::hypot(x-defender.tm_x,y-defender.tm_y));
        lane=std::min(lane,segment_distance(defender.tm_x,defender.tm_y,ax,ay,x,y));
        route=std::min(route,segment_distance(defender.tm_x,defender.tm_y,p.tm_x,p.tm_y,x,y));
    }
    // Prefer an unmarked, visible passing option within one short pass. Travel
    // cost and target hysteresis stop indecisive zig-zags between similar spaces.
    return clearance*0.9f+lane*1.6f-std::hypot(x-p.tm_x,y-p.tm_y)*0.18f-
        std::abs(spacing-240)*0.25f-std::max(0.0f,48-route)*1.2f;
}
void PlayerPractice::move_to(int index,float x,float y) {
    const auto& p=engine.teams[std::size_t(index)];
    const float dx=x-p.tm_x,dy=y-p.tm_y,d=std::hypot(dx,dy);
    if(d<6) move_player(index,0,0);else move_player(index,dx/d,dy/d);
}
void PlayerPractice::move_teammate() {
    if(assisted_receiver) {
        if(receive_ticks<=0 || engine.ball_poss || engine.last_touch>=12) assisted_receiver=0;
        else --receive_ticks;
    }
    if(assisted_receiver) move_to(assisted_receiver,receive_target.x,receive_target.y);
    const int index=selected==1?2:1;
    if(engine.ball_poss==index+1) {support_target_valid=false;return;}
    if(keeper_holding==0) {
        support_target_valid=false;
        move_to(index,220.0f,index==1?300.0f:500.0f);return;
    }
    if(engine.ball_poss>=12 || keeper_holding==1 || (!engine.ball_poss && !assisted_receiver)) {
        support_target_valid=false;
        float tx=engine.ballx,ty=engine.bally;
        if(engine.ball_poss>=12) {
            const int outlet=engine.ball_poss==13?13:12;
            const auto& target=engine.teams[std::size_t(outlet)];
            if(target.guy_on) {
                // Cover the other red player's passing lane while the user presses.
                // This continuous target does not flip sides at halfway or a touchline.
                tx=engine.ballx+(target.tm_x-engine.ballx)*0.65f;
                ty=engine.bally+(target.tm_y-engine.bally)*0.65f;
            } else {
                // With no outlet, contain the carrier from our own (left) goal side.
                tx=engine.ballx-48;
            }
        } else {
            // Chase a loose ball/pass using a short lead; normal swept contact decides
            // possession. Never alter flight or interfere with the blue receiving run.
            tx+=engine.ballxdis*4;ty+=engine.ballydis*4;
        }
        move_to(index,std::clamp(tx,24.0f,float(engine.pitch_len)-24),
                      std::clamp(ty,24.0f,float(engine.pitch_wid)-24));
        return;
    }
    const auto& p=engine.teams[std::size_t(index)];
    const auto& carrier=engine.teams[std::size_t(selected)];
    const float ax=carrier.tm_x,ay=carrier.tm_y;
    if(!support_target_valid || ticks%6==0) {
        float best=support_score(index,p.tm_x,p.tm_y,ax,ay);
        BallPoint target{p.tm_x,p.tm_y,0};
        if(support_target_valid) {
            const float retained=support_score(index,support_target.x,support_target.y,ax,ay)+18;
            if(retained>best) {best=retained;target=support_target;}
        }
        for(float radius:{180.0f,240.0f,300.0f}) for(int sector=0;sector<24;++sector) {
            const float angle=float(sector)*2*pi/24;
            const float x=ax+radius*std::cos(angle),y=ay+radius*std::sin(angle);
            const float score=support_score(index,x,y,ax,ay);
            if(score>best) {best=score;target={x,y,0};}
        }
        support_target=target;support_target_valid=true;
    }
    // Steer around nearby defenders instead of running directly through a marker.
    float tx=support_target.x,ty=support_target.y;
    for(int i:{12,13}) {
        const auto& defender=engine.teams[std::size_t(i)];
        const float dx=p.tm_x-defender.tm_x,dy=p.tm_y-defender.tm_y,d=std::hypot(dx,dy);
        if(d>0.1f && d<64) {tx+=dx/d*(64-d)*2;ty+=dy/d*(64-d)*2;}
    }
    move_to(index,std::clamp(tx,48.0f,float(engine.pitch_len)-48),std::clamp(ty,48.0f,float(engine.pitch_wid)-48));
}
void PlayerPractice::move_opponents() {
    const auto distance=[&](int i,float x,float y) {
        const auto& p=engine.teams[std::size_t(i)];return std::hypot(p.tm_x-x,p.tm_y-y);
    };
    const int nearest=distance(12,engine.ballx,engine.bally)<=distance(13,engine.ballx,engine.bally)?12:13;
    if(opponent_role_ticks>0)--opponent_role_ticks;
    // Small distance changes must not swap the presser and lane defender each tick.
    if(opponent_presser<0 || (!opponent_role_ticks && nearest!=opponent_presser &&
        distance(nearest,engine.ballx,engine.bally)+32<distance(opponent_presser,engine.ballx,engine.bally))) {
        opponent_presser=nearest;opponent_role_ticks=12;
    }
    const int presser=opponent_presser;
    if(dribble_ticks>0)--dribble_ticks;
    const int red_holder=engine.ball_poss>=12?engine.ball_poss:0;
    const bool new_holder=red_holder!=opponent_holder;
    if(new_holder) {
        opponent_holder=red_holder;dribble_ticks=0;
        if(red_holder) {
            const auto& holder=engine.teams[std::size_t(red_holder-1)];
            // Hold the support side through midfield instead of flipping a 340x260 target.
            opponent_support_offset={holder.tm_x<640?170.0f:-170.0f,holder.tm_y<400?130.0f:-130.0f,0};
        }
    }
    if(red_receiver && (engine.ball_poss || engine.last_touch<12 || --red_receive_ticks<=0)) red_receiver=0;
    for(int i:{12,13}) {
        auto& p=engine.teams[std::size_t(i)];
        if(!p.guy_on)continue;
        float tx=engine.ballx,ty=engine.bally;
        if(keeper_holding==1) {tx=1060;ty=i==12?300.0f:500.0f;}
        else if(keeper_holding==0) {tx=300;ty=i==12?290.0f:510.0f;}
        else if(red_receiver) {
            if(i==red_receiver) {tx=red_receive_target.x;ty=red_receive_target.y;}
            else {tx=red_receive_target.x+140;ty=red_receive_target.y+(p.tm_y<red_receive_target.y?-100.0f:100.0f);}
        } else if(engine.ball_poss>=12) {
            const auto& holder=engine.teams[std::size_t(engine.ball_poss-1)];
            if(engine.ball_poss==i+1) {
                const int closest=distance(1,p.tm_x,p.tm_y)<distance(2,p.tm_x,p.tm_y)?1:2;
                const auto& pursuer=engine.teams[std::size_t(closest)];
                const float pressure=distance(closest,p.tm_x,p.tm_y);
                const float remaining=distance(i,dribble_target.x,dribble_target.y);
                const bool blocked=pressure<96 && remaining>1 &&
                    ((pursuer.tm_x-p.tm_x)*(dribble_target.x-p.tm_x)+
                     (pursuer.tm_y-p.tm_y)*(dribble_target.y-p.tm_y))>pressure*remaining*0.35f;
                // Commit to a safe waypoint until arrival or a pursuer blocks it.
                // Clamping the destination removes the old edge/centre toggle.
                if(new_holder || (!dribble_ticks && (remaining<36 || blocked))) {
                    float ax=p.tm_x-pursuer.tm_x,ay=p.tm_y-pursuer.tm_y,d=std::hypot(ax,ay);
                    if(d<1) {ax=-1;ay=0;d=1;}
                    dribble_target={std::clamp(p.tm_x+ax/d*240,80.0f,1200.0f),
                                    std::clamp(p.tm_y+ay/d*240,80.0f,720.0f),0};
                    if(pressure>130 && opponent_shooting)
                        dribble_target={std::max(80.0f,p.tm_x-240),std::clamp(p.tm_y,300.0f,500.0f),0};
                    else if(distance(i,dribble_target.x,dribble_target.y)<96)
                        dribble_target={640,p.tm_y<400?560.0f:240.0f,0};
                    dribble_ticks=12;
                }
                tx=dribble_target.x;ty=dribble_target.y;
            } else {tx=holder.tm_x+opponent_support_offset.x;ty=holder.tm_y+opponent_support_offset.y;}
        } else if(engine.ball_poss && i!=presser) {
            const int teammate=engine.ball_poss==2?2:1;
            const auto& target=engine.teams[std::size_t(teammate)];
            tx=(engine.ballx+target.tm_x)*0.5f;ty=(engine.bally+target.tm_y)*0.5f;
        } else if(!engine.ball_poss) {
            // A short lead helps chase a free ball without changing its flight.
            tx+=engine.ballxdis*4;ty+=engine.ballydis*4;
        }
        tx=std::clamp(tx,24.0f,float(engine.pitch_len)-24);
        ty=std::clamp(ty,24.0f,float(engine.pitch_wid)-24);
        const float dx=tx-p.tm_x,dy=ty-p.tm_y,d=std::hypot(dx,dy);
        const bool winding=opponent_windup[std::size_t(i-12)]>0;
        if(d<6 || winding) move_player(i,0,0);
        else move_player(i,dx/d,dy/d);
    }
}
bool PlayerPractice::consider_opponent_shot() {
    if(!opponent_shooting || red_shot_cooldown || keeper_holding>=0 || engine.ball_poss<13) return false;
    auto& p=engine.teams[std::size_t(engine.ball_poss-1)];
    if(p.tm_poss<12 || engine.ballx<45 || engine.ballx>360 || std::abs(engine.bally-400)>165) return false;
    // Carry closer against a set keeper; take an earlier clear chance only
    // when the keeper has left the middle of the goal.
    if(engine.ballx>150 && keepers[0].active && std::abs(keepers[0].y-400)<20) return false;
    const float goal_y=keepers[0].y<=400?436.0f:364.0f;
    for(int i:{1,2}) {
        const auto& blue=engine.teams[std::size_t(i)];
        if(segment_distance(blue.tm_x,blue.tm_y,engine.ballx,engine.bally,0,goal_y)<24) return false;
    }
    const float dx=-engine.ballx,dy=goal_y-engine.bally,d=std::hypot(dx,dy);
    engine.inc_shot(short(engine.ball_poss));engine.stop_ball_spin();engine.reset_shot();
    engine.ballxdis=20*dx/d;engine.ballydis=20*dy/d;engine.ballzdis=4;engine.ball_inair=1;
    release(0);red_receiver=0;red_shot_cooldown=40;
    status="RED SHOOTS! Follow the rebound or cover the next pass.";return true;
}
void PlayerPractice::consider_opponent_pass() {
    if(red_pass_cooldown || red_decision_ticks || engine.ball_poss<12) return;
    red_decision_ticks=6; // Read the situation every 0.3 seconds, not every frame.
    const int holder=engine.ball_poss-1,receiver=holder==12?13:12;
    const auto& carrier=engine.teams[std::size_t(holder)];
    const auto& target=engine.teams[std::size_t(receiver)];
    if(carrier.tm_poss<16 || !target.guy_on) return; // Allow control after a regain/catch.
    float x=target.tm_x-engine.ballx,y=target.tm_y-engine.bally;
    const float spacing=std::hypot(x,y);
    if(spacing<100 || spacing>420) return;
    float pass_speed=0,end_speed=5;
    const auto was_airborne=engine.ball_inair;const auto was_chip=engine.chip_pass;
    engine.chip_pass=0;
    engine.pass_ahead(receiver+1,x,y,pass_speed,end_speed,0);
    engine.ball_inair=was_airborne;engine.chip_pass=was_chip; // Planning must not alter ball state.
    const float tx=engine.ballx+x,ty=engine.bally+y,d=std::hypot(x,y);
    if(d<100 || d>460 || tx<32 || tx>engine.pitch_len-32 || ty<32 || ty>engine.pitch_wid-32) return;
    float pressure=2000,receiver_space=2000,lane=2000;
    for(int i:{1,2}) {
        const auto& blue=engine.teams[std::size_t(i)];
        pressure=std::min(pressure,std::hypot(blue.tm_x-carrier.tm_x,blue.tm_y-carrier.tm_y));
        receiver_space=std::min(receiver_space,std::hypot(blue.tm_x-tx,blue.tm_y-ty));
        lane=std::min(lane,segment_distance(blue.tm_x,blue.tm_y,engine.ballx,engine.bally,tx,ty));
    }
    const bool pressured=pressure<150 && receiver_space>pressure+24;
    const bool forward=carrier.tm_x-tx>64 && receiver_space>90; // Reds attack left.
    const bool circulate=carrier.tm_poss>=70 && receiver_space>110;
    if(lane<38 || receiver_space<64 || !(pressured || forward || circulate)) return;
    // Same ground-pass speed, drag, lead and local receiving rules as blue passes.
    // Freeze the destination at the kick: neither the ball nor the catcher homes.
    engine.inc_pass(short(holder+1));engine.stop_ball_spin();engine.reset_shot();
    engine.chip_pass=0;engine.ball_inair=0;engine.ballzdis=0;engine.ballz=2;
    pass_speed=float(end_speed+d*(1.0-0.965));
    engine.ballxdis=pass_speed*x/d;engine.ballydis=pass_speed*y/d;
    release(0); // Never switch user selection or suppress blue movement for a red pass.
    red_receiver=receiver;red_receive_target={tx,ty,0};red_receive_ticks=100;
    red_pass_cooldown=48;++red_passes;
    status="Red passes - cut the lane or close down the receiver.";
}
void PlayerPractice::challenge_opponents() {
    for(int i:{12,13}) {
        auto& windup=opponent_windup[std::size_t(i-12)];
        const auto& p=engine.teams[std::size_t(i)];
        const float dx=engine.ballx-p.tm_x,dy=engine.bally-p.tm_y,d=std::hypot(dx,dy);
        if(possession_grace || !engine.ball_poss || engine.ball_poss>=12 || tackle_cooldown[std::size_t(i)] ||
            d>26 || (d>1 && (dx*p.tm_xdis+dy*p.tm_ydis)/d<0.25f)) {windup=0;continue;}
        // Visible 0.4-second wind-up gives the user time to dodge or pass.
        if(++windup>=8) {
            windup=0;tackle_cooldown[std::size_t(i)]=20;
            try_challenge(i);
        }
    }
}
bool PlayerPractice::check_boundary(float x,float y,float z,const std::array<BallPoint,22>& previous_players) {
    if(!opponents) return false;
    const float radius=engine.ball_diam/2;
    const float dx=engine.ballx-x,dy=engine.bally-y;
    float earliest=2;int edge=-1;
    const auto crossing=[&](int candidate,float previous,float current,float plane,bool increasing) {
        if((increasing && previous<=plane && current>plane) || (!increasing && previous>=plane && current<plane)) {
            const float t=(plane-previous)/(current-previous);
            if(t<earliest) {earliest=t;edge=candidate;}
        }
    };
    // The full ball must leave the field. Check the first crossed boundary,
    // before collection, so a fast diagonal shot cannot score after going out.
    crossing(0,x,engine.ballx,-radius,false);
    crossing(1,x,engine.ballx,float(engine.pitch_len)+radius,true);
    crossing(2,y,engine.bally,-radius,false);
    crossing(3,y,engine.bally,float(engine.pitch_wid)+radius,true);
    if(edge<0) return false;
    // A player can intercept before the boundary, but never after it. Both tests
    // use the same fractional tick, including relative player motion and height.
    if(receive(x,y,z,previous_players,earliest)) {engine.get_ball_zone();return true;}
    const float cy=y+earliest*dy,cz=z+earliest*(engine.ballz-z);
    const float cx=x+earliest*dx;
    restart_goal=edge<2 && std::abs(cy-float(engine.cntspot_y))+radius<goal_half_width &&
        cz+radius<float(engine.goal_height);
    if(restart_goal) {
        if(edge==1) ++blue_score;else ++red_score; // Own goals count for the opposing end.
        restart_red=edge==1;restart_ticks=40;
        restart_point={float(engine.cntspot_x),float(engine.cntspot_y),2};
        status=std::string(edge==1?"BLUE GOAL!":"RED GOAL!")+"  "+std::to_string(blue_score)+" - "+std::to_string(red_score)+
            (restart_red?"  Red kicks off.":"  Blue kicks off.");
    } else {
        restart_red=engine.last_touch>0 && engine.last_touch<12;
        restart_ticks=20;
        restart_point={std::clamp(cx,30.0f,float(engine.pitch_len)-30),std::clamp(cy,30.0f,float(engine.pitch_wid)-30),2};
        status=restart_red?"Ball out - red restart. Score stays unchanged.":"Ball out - blue restart. Score stays unchanged.";
    }
    engine.holder_lose_ball();engine.stop_ball_spin();engine.reset_shot();
    engine.ballx=cx;engine.bally=cy;engine.ballz=cz;
    engine.ballxdis=engine.ballydis=engine.ballzdis=0;engine.ball_still=1;
    assisted_receiver=0;support_target_valid=false;tackle_window=0;tackle_held=false;
    input_x=input_y=0;outside=false;opponent_windup.fill(0);
    return true;
}
void PlayerPractice::finish_restart() {
    reset_keepers(false);red_shot_cooldown=0;
    red_receiver=red_receive_ticks=red_pass_cooldown=red_decision_ticks=0;
    engine.holder_lose_ball();engine.stop_ball_spin();engine.reset_shot();engine.reset_all_ideas();
    engine.ball_inair=0;engine.ball_in_hands=0;engine.last_touch=0;engine.prev_touch=0;
    engine.ballxdis=engine.ballydis=engine.ballzdis=0;
    engine.ballx=restart_point.x;engine.bally=restart_point.y;engine.ballz=engine.ball_diam/2;
    const int taker=restart_red?12:1,partner=restart_red?13:2;
    const float direction=restart_red?-1.0f:1.0f;
    for(int i:practice_players) {
        auto& p=engine.teams[std::size_t(i)];
        p.tm_act=0;p.tm_frm=0;p.tm_ftime=0;p.tm_poss=0;p.go_txdis=p.go_tydis=0;
        p.tm_xdis=i<11?1.0f:-1.0f;p.tm_ydis=0;
        if(i==taker) {p.tm_x=restart_point.x-direction*10;p.tm_y=restart_point.y;}
        else if(i==partner) {p.tm_x=restart_point.x-direction*160;p.tm_y=restart_point.y+(restart_point.y<400?120.0f:-120.0f);}
        else {p.tm_x=restart_point.x+direction*180;p.tm_y=restart_point.y+(i%2?110.0f:-110.0f);}
        p.tm_x=std::clamp(p.tm_x,16.0f,float(engine.pitch_len)-16);
        p.tm_y=std::clamp(p.tm_y,16.0f,float(engine.pitch_wid)-16);
    }
    engine.collect_ball(&engine.teams[std::size_t(taker)]);select(1);
    opponent_presser=-1;opponent_role_ticks=0;opponent_holder=0;dribble_ticks=0;
    cooldown=0;kicker=0;possession_grace=30;tackle_cooldown.fill(0);opponent_windup.fill(0);
    assisted_receiver=0;support_target_valid=false;tackle_window=0;tackle_held=false;trail_size=0;
    input_x=input_y=0;await_input_release=!restart_red;outside=false;
    status=restart_red?"Red restart - chase and HOLD X to win possession.":"Blue restart - attack RIGHT. Release direction, then move or pass.";
}
int PlayerPractice::match_seconds_left() const {
    return std::max(0,match_duration_ticks-match_elapsed_ticks+19)/20;
}
std::string PlayerPractice::match_result() const {
    return blue_score>red_score?"BLUE WINS":red_score>blue_score?"RED WINS":"DRAW";
}
void PlayerPractice::toggle_pause() { if(!match_finished) paused=!paused; }
void PlayerPractice::step() {
    if(paused || outside || match_finished) return;
    ++ticks;
    if(restart_ticks>0) {if(--restart_ticks==0) finish_restart();return;}
    step_play();
    // Count active play, including keeper possession. Finish AFTER contacts and
    // line crossings so a goal on the final simulation tick still counts.
    if(opponents && ++match_elapsed_ticks>=match_duration_ticks) {
        match_elapsed_ticks=match_duration_ticks;match_finished=true;
        restart_ticks=0;tackle_window=0;tackle_held=false;opponent_windup.fill(0);
        input_x=input_y=held_input_x=held_input_y=0;
        status="FULL TIME - "+match_result()+". Press R or controller Start for a rematch.";
    }
}
void PlayerPractice::step_play() {
    if(cooldown>0)--cooldown;
    if(red_shot_cooldown>0)--red_shot_cooldown;
    if(red_pass_cooldown>0)--red_pass_cooldown;
    if(red_decision_ticks>0)--red_decision_ticks;
    if(possession_grace>0)--possession_grace;
    for(auto& wait:tackle_cooldown) if(wait>0)--wait;
    // Held tackle repeats only while defending, so winning possession cannot
    // turn the same held button into an accidental shot.
    if(tackle_held && engine.ball_poss>=12 && !tackle_cooldown[std::size_t(selected)]) challenge();
    std::array<BallPoint,22> previous_players{};
    for(int i:practice_players) previous_players[std::size_t(i)]={engine.teams[std::size_t(i)].tm_x,engine.teams[std::size_t(i)].tm_y,0};
    for(int i:{1,2}) move_player(i,i==selected?input_x:0,i==selected?input_y:0);
    for(int side=0;side<2;++side)previous_players[side==0?0:11]={keepers[std::size_t(side)].x,keepers[std::size_t(side)].y,0};
    if(opponents) {move_keepers();move_teammate();move_opponents();}
    if(keeper_holding>=0) {
        const auto& k=keepers[std::size_t(keeper_holding)];
        engine.ballx=k.x+(keeper_holding==0?8.0f:-8.0f);engine.bally=k.y;engine.ballz=14;
        if(--keeper_hold_ticks<=0)keeper_release();
        engine.get_ball_zone();return;
    }
    float px=engine.ballx,py=engine.bally,pz=engine.ballz;
    if(engine.ball_poss) {
        auto& holder=engine.teams[std::size_t(engine.ball_poss-1)];
        holder.tm_poss=std::min<short>(holder.tm_poss,30000); // Bound old short possession timer.
        engine.hold_ball(&holder);
    } else engine.ball_trajectory();
    if(check_boundary(px,py,pz,previous_players)) return;
    receive(px,py,pz,previous_players);
    if(tackle_window>0) {
        if(!try_challenge(selected) && --tackle_window==0)
            status="Tackle missed - close in while holding X.";
    }
    if(opponents && keeper_holding<0) {challenge_opponents();if(!consider_opponent_shot())consider_opponent_pass();}
    outside=!opponents && (engine.ballx<0||engine.ballx>float(engine.pitch_len)||engine.bally<0||engine.bally>float(engine.pitch_wid));
    if(outside) status="Ball outside the practice pitch. Press R to reset.";
    else engine.get_ball_zone();
    if(!std::isfinite(engine.ballx)||!std::isfinite(engine.bally)||!std::isfinite(engine.ballz)) throw std::runtime_error("Invalid player practice state");
    if(trail_size<trail.size()) trail[trail_size++]={engine.ballx,engine.bally,engine.ballz};
    else {std::move(trail.begin()+1,trail.end(),trail.begin());trail.back()={engine.ballx,engine.bally,engine.ballz};}
}
}
