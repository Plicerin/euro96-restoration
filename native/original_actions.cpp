#include "original_actions.hpp"
#include "action_constants.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
namespace actua::original {
void PlayerEngine::init_anim(match_player* p,int animation) {
    // Dispatcher restricted to recovered branches of ACTIONS.CPP init_anim.
    if(animation>=MC_AFOOTB && animation<=MC_CJUMPBR){init_save_anim(p,animation);return;}
    switch(animation) {
    case MC_STAND:init_stand_anim(p);break;
    case MC_CELEB:case MC_PUMP:case MC_BABY:case MC_REEVES:case MC_MILLER:case MC_PLANE:case MC_FINGER:case MC_KNEE:case MC_MOON:case MC_DUCK:init_celeb_anim(p,animation);break;
    case MC_SHAME:init_shame_anim(p);break;
    case MC_RUN:init_run_anim(p);break;
    case MC_TROTA:init_trot_anim(p);break;
    case MC_PASSL:case MC_CHIPL:case MC_SHOOTL:case MC_BACKHEELL:init_kick_anim(p,animation);break;
    case MC_TRAPL:init_trap_anim(p);break;
    case MC_CHEST:init_chest_anim(p);break;
    case MC_D_HEAD:init_dnhead_anim(p);break;
    case MC_S_HEAD:init_shdr_anim(p);break;
    case MC_J_HEAD:init_jhdr_anim(p);break;
    case MC_DV_HEAD:init_dhdr_anim(p);break;
    case MC_OVERHEAD:init_ohd_anim(p);break;
    case MC_VOLLEYL:init_voll_anim(p);break;
    case MC_GETUP:init_getup_anim(p);break;
    case MC_GETUPF:init_getupf_anim(p);break;
    case MC_STANDB:init_standb_anim(p);break;
    case MC_RUNWITHB:init_runb_anim(p);break;
    case MC_STOSL:case MC_STOSR:init_tostand_anim(p,animation);break;
    case MC_STOSBL:case MC_STOSBR:init_tostandb_anim(p,animation);break;
    case MC_CRTOSTB:init_crtostb_anim(p);break;
    case MC_BOUNCE:init_bounce_anim(p);break;
    case MC_HALFVOLL:init_hvoll_anim(p);break;
    case MC_THROW:init_throw_anim(p);break;
    case MC_ROLL:init_roll_anim(p);break;
    case MC_WAVEUP:init_waveup_anim(p);break;
    case MC_THROWL:init_throwl_anim(p);break;
    case MC_THROWS:init_throws_anim(p);break;
    case MC_DIAGPASSL:case MC_DIAGBACKL:case MC_SIDEPASSL:init_kick_l_anim(p,animation);break;
    case MC_DIAGPASSR:case MC_DIAGBACKR:case MC_SIDEPASSR:init_kick_r_anim(p,animation);break;
    case MC_CROSSL:init_crl_anim(p);break;
    case MC_CROSSR:init_crr_anim(p);break;
    case MC_KICKOUT:init_kout_anim(p);break;
    case MC_S_TACKLE:init_tack_anim(p);break;
    case MC_PICKBALL:init_pickup_anim(p);break;
    case MC_FALLR:init_fallr_anim(p);break;
    case MC_RIDEL:case MC_RIDER:init_ride_anim(p);break;
    case MC_BARGE:init_barge_anim(p);break;
    case MC_PENALTY:init_penalty_anim(p);break;
    default:throw std::logic_error("Animation action is not restored: "+std::to_string(animation));
    }
}
}
namespace actua {
using namespace original::action_constants;
OriginalActions::OriginalActions(const std::vector<OriginalClip>& clips) {
    for(const auto& c:clips)if(c.animation>=0){
        capture_index[std::size_t(c.animation)]={int(captures.size()),short(c.frames.size())};
        captures.insert(captures.end(),c.frames.begin(),c.frames.end());
    }
    // Mirrored IDs share the source capture, as in render3d's mcaps[an-1] lookup.
    for(int i=9;i<=69;i+=2)capture_index[std::size_t(i)]=capture_index[std::size_t(i-1)];
    for(int i:{MC_PASSR,MC_TRAPR,MC_STAND})if(!capture_index[std::size_t(i)].capfrms)throw std::runtime_error("Required action capture missing");
    reset();
}
void OriginalActions::reset() {
    select_input(0);other_input={};head_to_head=false;
    walkoff_events.clear();
    possession_ticks={};departed_figures={};restart_figures={};shootout_figures={};match_notice={};
    national_teams=false;pending_subs={};used_squad={};substitutions={};sub_passes={};sub_shots={};seen_card_events=card_notice_ticks=0;card_notice.clear();sub_notice.clear();
    ++generation;
    live_controls=live_shot=live_pass=live_blocked=false;live_f1=live_f2=0;live_player=-1;
    selection_wait=manual_selection_ticks=seen_selection_events=0;
    play=team_play=cpu_vs_cpu=false;turnovers=ai_passes=ai_shots=0;selected=1;pass_target=play_end_ticks=0;play_x=play_y=0;play_kick=PlayKick::none;
    blue_goals=red_goals=restarts=restart_mode=restart_ticks=0;
    full_match=full_time=false;interval_ticks=seen_rule_events=0;interval_minute=45;
    tournament_fixture=next_tournament_fixture=-1;original_draw=next_original_draw=false;
    restart_practice=-1;restart_armed=restart_shot=restart_pass=restart_loft=restart_started=false;restart_x=restart_y=0;
    penalty_direction=0;penalty_button=penalty_armed=false;
    shootout=penalty_released=false;penalty_wait=penalty_age=penalty_side=0;penalty_results={};penalty_scores={};match_passes={};match_shots={};
    goal_review=review_left=false;review_stage=review_tick=0;goal_tick=-1;
    retreat_started=false;continuation=false;distributions=distribution_type=distribution_animation=carry_ticks=limbo_ticks=outlet=0;distribution_tick=catch_tick=recovery_tick=-1;
    attack=goal_mouth=false;visible_players={1,2};first_touches=shots=saves=planned_player=0;
    engine=original::PlayerEngine{};engine.player_p=captures;engine.mcaps=capture_index;
    ticks=releases=receptions=0;release_tick=receive_tick=-1;running=miss=receiver_prepared=false;
    for(int i=0;i<22;++i)engine.teams[std::size_t(i)].tm_player=short(i+1);
    for(int i:{1,2}){
        auto& p=engine.teams[std::size_t(i)];p.tm_x=i==1?580.0f:700.0f;p.tm_y=400;
        p.tm_xdis=i==1?1.0f:-1.0f;p.mface_x=p.tm_xdis;
        p.tm_rate=p.tm_pow=p.tm_cont=p.tm_flair=p.tm_ac=96;p.tm_mcspd=float(p.tm_flair+p.tm_rate)/128;p.guy_on=1; // Original init_team.
        p.tm_jump=40;
        engine.init_stand_act(&p);
    }
    engine.collect_ball(&engine.teams[1]);engine.want_pass=3;
    status="SPACE: original pass and trap";
}
void OriginalActions::start(bool moving,float receiver_offset) {
    reset();running=true;
    engine.teams[2].tm_y+=receiver_offset;
    auto& p=engine.teams[1];
    if(moving){p.tm_anim=MC_RUN;p.tm_frm=0.5f;} // Fixture input exercises the opposite-foot branch.
    engine.kick_type=KT_PASS;engine.pass_type=5;engine.receiver=3;
    engine.init_kick_act(&p,MC_PASSL,float(MCC_PASS));
    status="Kick wind-up";
}
void OriginalActions::prepare_receiver() {
    // Native fixture supplies the original control-button input.
    // INTELL chooses the meeting point, travel time, turn and animation schedule.
    engine.predict_ball();
    auto& p=engine.teams[2];
    engine.user_controlled=p.control=1;engine.uf1=0;engine.uf2=1;
    engine.um=0; // Control button with no direction: original decide_on_face applies.
    const bool found=engine.go_to_path(p.tm_player)!=0;
    status=found?"Original interception: run, turn, control":"No reachable interception";
    receiver_prepared=true;
}
void OriginalActions::step() {
    if(!running)return;
    // Snapshot observers consume this edge per logic tick, independently of rendering frequency.
    for(auto& p:engine.teams)p.tm_newanim=0;
    if(team_play){step_team_play();return;}
    if(play){step_play();return;}
    if(goal_review){step_goal_review();return;}
    if(attack){step_attack();return;}
    ++ticks;
    // FOOTBALL.do_logic processes the ball before ACTIONS.process_teams.
    engine.prediction=0;
    if(!engine.ball_poss)engine.ball_trajectory();
    if(receiver_prepared && !engine.ball_poss)engine.predict_ball();
    engine.ball_speed=int(std::sqrt(engine.ballxdis*engine.ballxdis+engine.ballydis*engine.ballydis+engine.ballzdis*engine.ballzdis));
    for(int i:{1,2}){
        auto& p=engine.teams[std::size_t(i)];engine.user_controlled=p.control;engine.process_anims(&p);
        // Supported branches of ball_interact, preserving interaction-before-action ordering.
        if(engine.ball_poss==p.tm_player){
            if(p.tm_act==KICK_ACT)engine.update_kick_contact(&p);
            else engine.hold_ball(&p);
        } else if(p.tm_act==17 && !engine.ball_poss && p.tm_ftime>=0 && engine.control_interact(&p)) {
            engine.collect_controlled_ball(&p);
            ++receptions;receive_tick=ticks;status="Original trap contact";
        }
        const int owner=engine.ball_poss;
        if(i==2 && releases && !receiver_prepared)prepare_receiver();
        if(p.tm_act==KICK_ACT)engine.kick_action(&p);
        else if(p.tm_act==17)engine.control_action(&p);
        else if(p.tm_act==1)engine.run_action(&p);
        else if(p.tm_act==20)engine.stop_action(&p);
        else if(p.tm_act==WAIT_ACT)engine.wait_action(&p);
        // Fixture ends with a stationary recovery; free dribbling input is not connected.
        if(i==2 && receiver_prepared && engine.ball_poss!=p.tm_player)engine.process_dir(&p);
        if(owner && !engine.ball_poss){++releases;release_tick=ticks;status="Ball released at kick contact";}
    }
    if(receptions && engine.teams[2].tm_act==0){
        status="Received - ball returning to feet";
        if(engine.teams[2].tm_ftime==0){running=false;status="Pass and trap complete - SPACE to replay";}
    }
    if(ticks>=180){running=false;miss=!receptions;status="Sequence ended - SPACE to replay";}
    if(!std::isfinite(engine.ballx)||!std::isfinite(engine.bally)||!std::isfinite(engine.ballz))throw std::runtime_error("Non-finite original action state");
}
}

namespace actua {
void OriginalActions::start_attack(float target_y,float keeper_y) {
    reset();std::srand(1);attack=true;running=true;visible_players={1,2,3,11};
    // Fixture roster and starting positions. Decisions, trajectories and contacts use original routines.
    for(int i=0;i<22;++i){auto& p=engine.teams[std::size_t(i)];p.guy_on=0;p.tm_x=-1000;p.tm_y=-1000;}
    for(int i:visible_players){
        auto& p=engine.teams[std::size_t(i)];p={};p.tm_player=short(i+1);
        p.tm_x=i==1?850.0f:i==2?700.0f:i==3?1160.0f:1250.0f;
        p.tm_y=i==11?keeper_y:i==3?target_y:400.0f;
        p.tm_xdis=p.mface_x=(i==11||i==1)?-1.0f:1.0f;
        p.tm_rate=p.tm_pow=p.tm_cont=p.tm_flair=p.tm_ac=p.tm_vis=96;
        p.tm_vis=120;p.tm_mcspd=1.5f;p.tm_jump=40;p.guy_on=1;p.tm_stopped=1;
        engine.init_stand_act(&p);
    }
    engine.collect_ball(&engine.teams[1]);engine.user_controlled=0;
    engine.kick_type=KT_PASS;engine.pass_type=5;engine.receiver=3;engine.want_pass=4;
    engine.init_kick_act(&engine.teams[1],MC_PASSL,float(MCC_PASS));
    status="Original attack: build-up pass";
}
void OriginalActions::start_continuation(float outlet_x,float outlet_y){
    start_attack();continuation=true;visible_players.push_back(12);
    auto& p=engine.teams[12];p={};p.tm_player=13;
    p.tm_rate=p.tm_pow=p.tm_cont=p.tm_flair=p.tm_ac=96;p.tm_vis=120;
    p.tm_mcspd=1.5f;p.tm_jump=40;p.guy_on=1;p.tm_stopped=1;
    outlet_target_x=outlet_x;outlet_target_y=outlet_y;
    p.tm_x=1200;p.tm_y=200;p.tm_xdis=p.mface_x=-1;
    engine.init_stand_act(&p);
}
const char* OriginalActions::goal_review_name(int stage){
    static const char* names[]{"Post rebound","Crossbar rebound","Outside side net","Roof of the net","Goal and back net"};
    return names[std::clamp(stage,0,4)];
}
void OriginalActions::start_goal_review(int stage,bool left){
    if(stage<0 || stage>4)throw std::out_of_range("Goal review stage");
    reset();goal_review=true;review_left=left;review_stage=stage;running=true;
    visible_players.clear();engine.holder_lose_ball();
    auto& e=engine;e.ball_poss=0;e.ball_limbo_on=0;e.ball_in_hands=0;
    e.ballx=1200;e.bally=400;e.ballz=2;e.ballxdis=12;e.ballydis=e.ballzdis=0;
    e.ball_inair=0;e.ball_still=0;e.shot_pending=1;e.seed=64;std::srand(1);
    if(stage==0)e.bally=float(e.top_post_y)-.5f;
    if(stage==1){
        e.ball_inair=1;e.ballx=1220;e.ballxdis=10;e.ballz=100;
        // Aim this diagnostic input at the bar using the original trajectory.
        // No collision result is forced; the same routine detects the impact below.
        auto probe=static_cast<original::BallEngine&>(e);
        for(int i=0;i<30;++i){
            const float x=probe.ballx,z=probe.ballz;probe.ball_trajectory();
            if(probe.ballx>e.pitch_len){
                const float t=(e.pitch_len-x)/(probe.ballx-x);
                e.ballz+=float(e.goal_height)+.5f-(z+t*(probe.ballz-z));break;
            }
        }
    }
    if(stage==2){e.ballx=1276;e.bally=348;e.ballz=12;e.ballxdis=8;e.ballydis=12;e.ball_inair=1;}
    if(stage==3){e.ballx=1290;e.ballz=70;e.ballxdis=.2f;e.ballzdis=-1;e.ball_inair=1;}
    if(stage==4){e.ballz=12;e.ball_inair=1;e.ballzdis=2;}
    if(left){e.ballx=e.pitch_len-e.ballx;e.ballxdis=-e.ballxdis;}
    e.prev_ballx=e.ballx;e.prev_bally=e.bally;e.prev_ballz=e.ballz;e.launch_ball(1);
    status=std::to_string(stage+1)+" / 5: "+goal_review_name(stage)+" - original collision routines";
}
void OriginalActions::step_goal_review(){
    ++ticks;++review_tick;
    // Give each launch a one-second lead-in, then three seconds to see the result.
    if(review_tick>20){engine.ball_trajectory();engine.ball_collision();}
    if(!std::isfinite(engine.ballx)||!std::isfinite(engine.bally)||!std::isfinite(engine.ballz)||
       !std::isfinite(engine.ballxdis)||!std::isfinite(engine.ballydis)||!std::isfinite(engine.ballzdis))
        throw std::runtime_error("Non-finite original goal collision");
    if(review_tick>=80){
        if(review_stage<4)start_goal_review(review_stage+1,review_left);
        else{running=false;status="Five goal checks complete - G: replay / H: other end";}
    }
}
void OriginalActions::step_attack(){
    ++ticks;engine.prediction=0;
    // Continue FOOTBALL.do_logic random updates after the fixed attack setup.
    if(continuation && catch_tick>=0)engine.af_randomize();
    engine.prev_ballx=engine.ballx;engine.prev_bally=engine.bally;engine.prev_ballz=engine.ballz;
    // Shot reaction counter excerpt from BALL.CPP process_ball; this is not the full match loop.
    engine.update_ball_limbo();
    if(engine.ball_limbo_on)++limbo_ticks;
    if(!engine.ball_poss && !engine.ball_limbo_on){
        if(engine.shot_acknowledged)--engine.shot_acknowledged;
        if(engine.shot_pending){if(!++engine.shot_pending)engine.shot_pending=1;}
        else engine.real_shot=0;
        engine.ball_trajectory();
        engine.ball_collision();
    }
    if(engine.ball_in_goal){
        goal_mouth=true;
        if(goal_tick<0)goal_tick=ticks;
        status="Original goal crossing and net rebound - G: goal review";
        if(ticks-goal_tick>=60)running=false;
        return; // No player can recover a scored ball; full restarts remain separate.
    }
    engine.ball_speed=int(std::sqrt(engine.ballxdis*engine.ballxdis+engine.ballydis*engine.ballydis+engine.ballzdis*engine.ballzdis));
    engine.keeper_boxes();
    engine.player_distances();
    engine.get_opp_near_ball(engine.ball_poss);engine.predict_ball();
    for(int i:visible_players){
        auto& p=engine.teams[std::size_t(i)];
        // Keep the fixture's selected receiver under neutral input after contact.
        engine.user_controlled=p.control=continuation && receptions && p.tm_player==outlet;
        engine.process_anims(&p);
        if(p.tm_limbo || (engine.ball_limbo_on && engine.ball_limbo_p==p.tm_player))continue;
        if(engine.ball_poss==p.tm_player){
            if(p.tm_act==KICK_ACT)engine.update_kick_contact(&p);else engine.hold_ball(&p);
        }else if(p.tm_act==STRIKE_ACT && !engine.ball_poss && p.tm_ftime>=0 && engine.control_interact(&p)){
            engine.strike_ball_off(&p);++first_touches;++releases;release_tick=ticks;
            if(engine.real_shot){++shots;status="Original first-time shot";}
            else {status="Original first-time pass";planned_player=0;}
        }else if(p.tm_act==17 && !engine.ball_poss && p.tm_ftime>=0 && engine.control_interact(&p)){
            engine.collect_controlled_ball(&p);++receptions;receive_tick=ticks;
            engine.user_controlled=p.control=1;
            status="Original control contact";
        }else if(p.tm_act==10 && !engine.ball_in_hands){
            const int before=engine.shot_saved;engine.update_save_contact(&p);
            if(engine.shot_saved && !before){++saves;status=engine.ball_in_hands?"Original goalkeeper catch":"Original goalkeeper block";}
        }
        if(engine.ball_in_hands && engine.ball_poss==p.tm_player){
            engine.hold_ball(&p);
            if(!continuation){running=false;break;}
            if(catch_tick<0){
                catch_tick=ticks;engine.want_pass=0;
                // Fixture movement input; original run_action executes the outlet's run.
                engine.init_run_act(&engine.teams[12],outlet_target_x,outlet_target_y,0);
                engine.teams[12].int_cnt=engine.teams[12].go_cnt+1; // Finite fixture run; no zonal follow-up.
            }
        }
        if(continuation && i==11 && catch_tick>=0){
            const int busy=engine.keeper_busy(&p);
            if(recovery_tick<0 && p.tm_act==12){recovery_tick=ticks;status="Original keeper recovery and holding";}
            if(engine.ball_poss==12 && engine.ball_in_hands && engine.keeper_b_in_box && p.tm_act<=2 && !busy){
                engine.distribute_held_ball(&p);
                if(!engine.ball_poss){++distributions;distribution_type=engine.pass_type;distribution_animation=p.tm_anim;distribution_tick=ticks;outlet=engine.receiver;planned_player=0;status="Original keeper distribution wind-up";}
            }
        }
        if(i==11 && engine.shot_pending>0 && !engine.shot_acknowledged && p.tm_act!=10){engine.predict_ball();engine.go_to_save_path(&p,0);}
        if(continuation && catch_tick>=0 && i==3 && p.tm_act==0 && !retreat_started){
            // Fixture movement input: the shooter retreats, opening the outlet lane.
            engine.init_run_act(&p,950,430,0);p.int_cnt=p.go_cnt+1;retreat_started=true;
        }
        const int next=distributions?outlet:releases==1?3:releases==2&&!shots?4:0;
        if(next && p.tm_player==next && planned_player!=next && !engine.ball_poss && !engine.ball_limbo_on){
            engine.want_pass=next==3?4:0;
            // Fixture supplies the original control button for the outlet reception.
            if(distributions){engine.user_controlled=p.control=1;engine.uf1=0;engine.uf2=1;engine.um=0;}
            engine.predict_ball();
            if(engine.go_to_path(next)){planned_player=next;status=distributions?"Original outlet reception approach":next==3?"Original first-time pass approach":"Original first-time finish approach";}
        }
        const float action_x=p.tm_x,action_y=p.tm_y;
        const int owner=engine.ball_poss;
        if(!p.tm_limbo && (!engine.ball_limbo_on || engine.ball_limbo_p!=p.tm_player))engine.dispatch_action(&p);
        if(i==11 && engine.ball_in_hands && (p.tm_x!=action_x || p.tm_y!=action_y) && p.tm_act==1)++carry_ticks;
        if(owner && !engine.ball_poss){++releases;release_tick=ticks;}
        if(!p.tm_limbo && (!engine.ball_limbo_on || engine.ball_limbo_p!=p.tm_player))engine.process_dir(&p);
    }
    // Original collision runs before this fixture-only out-of-play stop.
    if(engine.ballx>engine.pitch_len || engine.ballx<0 || engine.bally<0 || engine.bally>engine.pitch_wid){running=false;status=saves?"Save complete":goal_mouth?"Shot through goal mouth - SPACE to replay":"Shot missed - SPACE to replay";}
    if(continuation && receptions && engine.ball_poss==outlet && engine.teams[std::size_t(outlet-1)].tm_act==0 && engine.teams[std::size_t(outlet-1)].tm_ftime==0){running=false;status="Keeper outlet received - SPACE to replay";}
    if(ticks>=(continuation?600:240)){running=false;miss=continuation && !receptions;status="Sequence time limit reached";}
    if(!std::isfinite(engine.ballx)||!std::isfinite(engine.bally)||!std::isfinite(engine.ballz))throw std::runtime_error("Non-finite original attack state");
}
}
