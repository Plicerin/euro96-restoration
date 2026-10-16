#include "original_actions.hpp"
#include "action_constants.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
namespace actua {
using namespace original::action_constants;
void OriginalActions::select_input(int side){
    if(side<0 || side>1)throw std::invalid_argument("Invalid local player");
    if(side==input_side)return;
    std::swap(selected,other_input.selected);
    std::swap(live_f1,other_input.live_f1);
    std::swap(live_f2,other_input.live_f2);
    std::swap(live_player,other_input.live_player);
    std::swap(selection_wait,other_input.selection_wait);
    std::swap(manual_selection_ticks,other_input.manual_selection_ticks);
    std::swap(seen_selection_events,other_input.seen_selection_events);
    std::swap(play_x,other_input.play_x);
    std::swap(play_y,other_input.play_y);
    std::swap(restart_x,other_input.restart_x);
    std::swap(restart_y,other_input.restart_y);
    std::swap(penalty_direction,other_input.penalty_direction);
    std::swap(live_controls,other_input.live_controls);
    std::swap(live_shot,other_input.live_shot);
    std::swap(live_pass,other_input.live_pass);
    std::swap(live_blocked,other_input.live_blocked);
    std::swap(restart_armed,other_input.restart_armed);
    std::swap(restart_shot,other_input.restart_shot);
    std::swap(restart_pass,other_input.restart_pass);
    std::swap(restart_loft,other_input.restart_loft);
    std::swap(restart_high_pass,other_input.restart_high_pass);
    std::swap(penalty_button,other_input.penalty_button);
    std::swap(penalty_armed,other_input.penalty_armed);
    std::swap(play_kick,other_input.play_kick);
    input_side=side;
}
void OriginalActions::clear_player_input(){
    play_x=play_y=0;play_kick=PlayKick::none;live_shot=live_pass=false;live_f1=live_f2=0;live_blocked=true;
    engine.users_dir[input_side].x=engine.users_dir[input_side].y=0;clear_penalty_input();clear_restart_input();
}
void OriginalActions::clear_play_input(){
    clear_player_input();
    if(head_to_head){auto scope=input_scope(1-input_side);clear_player_input();}
}
void OriginalActions::start_play(){
    reset();play=running=true;std::srand(1);
    // Two-player input fixture; original full-team AI is not active here.
    for(auto& p:engine.teams){p.guy_on=0;p.tm_x=p.tm_y=-1000;}
    for(int i:visible_players){
        auto& p=engine.teams[i];p.tm_x=i==1?900.0f:1040.0f;p.tm_y=400;
        p.tm_xdis=p.mface_x=i==1?1.0f:-1.0f;p.tm_ydis=p.mface_y=0;
        p.tm_vis=120;p.tm_stopped=1;p.guy_on=1;p.control=i==selected;
        engine.init_stand_act(&p);
    }
    engine.want_pass=0;engine.collect_ball(&engine.teams[selected]);engine.hold_ball(&engine.teams[selected]);
    status="Run, dribble, pass, chip and shoot with the original actions";
}
void OriginalActions::set_play_input(float x,float y){
    if(shootout || restart_mode)return;
    if(cpu_vs_cpu || interval_ticks || full_time){clear_play_input();return;}
    if(!std::isfinite(x)||!std::isfinite(y)){x=y=0;}
    const float d=std::hypot(x,y);
    // Original joystick actions consume direction, not a new analog speed curve.
    play_x=d>.05f?x/d:0;play_y=d>.05f?y/d:0;
}
void OriginalActions::set_play_buttons(bool shot,bool pass){
    live_controls=true;
    if(cpu_vs_cpu || interval_ticks || full_time || restart_mode || shootout){live_shot=live_pass=false;live_f1=live_f2=0;return;}
    // Menus, pause and explicit selection require a release before another action.
    if(live_blocked){if(shot || pass){live_shot=live_pass=false;return;}live_blocked=false;}
    live_shot=shot;live_pass=pass;
}
void OriginalActions::request_play_kick(PlayKick kick){
    if(shootout || cpu_vs_cpu || restart_mode || interval_ticks || full_time)return;
    if(running && human_keeper_hold()){play_kick=kick;return;}
    if(team_play && running && kick==PlayKick::shot && engine.ball_poss!=selected+1 && engine.teams[selected].tm_act<=1){play_kick=kick;return;}
    if(play && running && engine.ball_poss==selected+1 && engine.teams[selected].tm_act<=1 && !engine.ball_limbo_on)
        play_kick=kick;
}
void OriginalActions::switch_play_player(){
    if(shootout || !play || !running || cpu_vs_cpu || restart_mode || interval_ticks)return;
    if(team_play){
        engine.player_distances();engine.predict_ball();engine.get_nearest();
        const int first=control_slot()+1;
        int next=control_slot()?(engine.receiver_b?engine.receiver_b:engine.interceptor_b?engine.interceptor_b:engine.near_path_b)-1:
            (engine.receiver_a?engine.receiver_a:engine.interceptor_a?engine.interceptor_a:engine.near_path_a)-1;
        if(engine.ball_poss>first && engine.ball_poss<=first+10)next=engine.ball_poss-1;
        if(next<first || next>=first+10 || next==selected || !engine.teams[next].guy_on || engine.teams[next].tm_act==FALL_ACT){
            float best=1e9f;next=selected;
            for(int i=first;i<first+10;++i)if(i!=selected && engine.teams[i].guy_on && engine.teams[i].tm_act!=FALL_ACT && engine.teams[i].tm_dist<best){best=engine.teams[i].tm_dist;next=i;}
        }
        clear_player_input();engine.teams[selected].control=0;selected=next;engine.reset_ideas(&engine.teams[selected]);
        engine.teams[selected].control=char(input_side+1);manual_selection_ticks=12;selection_wait=11;return;
    }
    selected=selected==1?2:1;
}
void OriginalActions::update_auto_selection(){
    auto& e=engine;
    if(!play || !team_play || !running || cpu_vs_cpu || full_time || interval_ticks || restart_ticks || restart_mode || e.match_mode || shootout)return;
    // A manual choice has priority for one original selection cadence. The
    // original selection circle continues to protect a nearby player afterward.
    if(manual_selection_ticks>0){--manual_selection_ticks;seen_selection_events=e.selection_events;return;}
    const auto& current=e.teams[selected];
    // Never transfer input during the selected actor's kick/save contact.
    if(current.contact>0 || current.tm_limbo || (e.ball_limbo_on && e.ball_limbo_p==selected+1))return;
    const bool keeper=selected==control_slot();
    if(keeper && e.ball_poss==selected+1)return;
    const bool event=e.selection_events!=seen_selection_events;
    if(!event && !keeper && current.guy_on && selection_wait++<=10)return;
    selection_wait=0;seen_selection_events=e.selection_events;
    for(int i=control_slot();i<control_slot()+11;++i)e.teams[i].control=char(i==selected && e.teams[i].guy_on?input_side+1:0);
    e.users[input_side].plr=current.guy_on?short(selected+1):0;e.users[input_side].chng=false;
    e.user_taker2=0; // Set pieces and keeper distribution use their own adapter.
    if(control_slot())e.auto_select_b(input_side+1);else e.auto_select_a(input_side+1);
    const int next=e.users[input_side].plr-1;
    if(next>=0 && next<22 && is_controlled_team(next) && e.teams[next].guy_on && next!=selected){
        selected=next;live_f1=live_f2=0;live_player=selected;play_kick=PlayKick::none;
        // Preserve direction and held first-touch buttons; do not stop a receiver.
    }
    for(int i=control_slot();i<control_slot()+11;++i)e.teams[i].control=char(i==selected && e.teams[i].guy_on?input_side+1:0);
}
void OriginalActions::step_play(){
    ++ticks;auto& e=engine;e.prediction=0;e.af_randomize();
    e.prev_ballx=e.ballx;e.prev_bally=e.bally;e.prev_ballz=e.ballz;
    e.update_ball_limbo();
    if(!e.ball_poss && !e.ball_limbo_on){e.ball_trajectory();e.ball_collision();}
    if(e.ball_in_goal || e.ballx<0 || e.ballx>e.pitch_len || e.bally<0 || e.bally>e.pitch_wid){
        status=e.ball_in_goal?"Goal - R: reset practice":"Ball out - R: reset practice";
        // Leave time to see the original goal/net response before freezing the fixture.
        if(++play_end_ticks>=60)running=false;
        play_kick=PlayKick::none;return;
    }
    e.player_distances();e.ball_speed=int(std::sqrt(e.ballxdis*e.ballxdis+e.ballydis*e.ballydis+e.ballzdis*e.ballzdis));
    // Native button mapping selects recovered actions. Original kick decision,
    // animation, contact point and trajectory determine the result.
    if(play_kick!=PlayKick::none){
        auto& p=e.teams[selected];e.user_controlled=p.control=1;e.uf1=e.uf2=0;
        if(e.ball_poss==p.tm_player && p.tm_act<=1 && !e.ball_limbo_on){
            if(play_kick==PlayKick::shot){
                if(e.shoot_decide(&p)){
                    // Expose the original driven-shot branch as the single shot button.
                    e.drive_shot=1;e.make_shoot(&p);pass_target=0;status="Original driven shot";
                }else status="Outside original shooting range - move closer";
            }else{
                e.chip_pass=play_kick==PlayKick::loft;
                const int receiver=e.pass_decide(&p,0);
                if(receiver){
                    if(e.chip_pass && e.pass_type==5)e.pass_type=-1;
                    if(e.chip_pass && e.pass_type!=-1){status="Face your teammate for a lofted pass";e.chip_pass=0;}
                    else{e.make_pass(&p,receiver,0);pass_target=receiver;receiver_prepared=false;status=e.pass_type==-1?"Original lofted pass":"Original ground pass";}
                }else{e.chip_pass=0;status="No teammate in original passing range";}
            }
        }
        play_kick=PlayKick::none;
    }
    if(!e.ball_poss && !e.ball_limbo_on)e.predict_ball();
    for(int i:visible_players){
        auto& p=e.teams[i];p.control=1; // Both actors use neutral user control for first-touch decisions.
        e.user_controlled=1;e.game_action=0;e.uf1=0;e.uf2=0;e.um=0;e.ux=e.uy=0;e.f1_down=e.f2_down=0;
        if(p.int_cnt>0 && p.int_move==I_KICKED)--p.int_cnt;
        e.process_anims(&p);
        if(p.tm_limbo || (e.ball_limbo_on && e.ball_limbo_p==p.tm_player))continue;
        const int owner=e.ball_poss;
        if(owner==p.tm_player){
            if(p.tm_act==KICK_ACT)e.update_kick_contact(&p);else e.hold_ball(&p);
        }else if(!owner){
            if(p.tm_act==17 && p.tm_ftime>=0 && e.control_interact(&p))e.collect_controlled_ball(&p);
            // Supported normal-play foot-contact gates from BALLINT.ball_interact.
            // No tackle, foul, keeper or automatic high-ball branch is dispatched.
            else if(p.tm_act<=1 && !p.tm_strike && !(p.int_move==I_KICKED && p.int_cnt) &&
                    TOUCHB_BOX>=e.calc_dist(e.ballx-p.tm_x,e.bally-p.tm_y) &&
                    e.ballz<p.tm_z+(25/2) && e.ballz-e.ballzdis<p.tm_z+(25/2)){
                e.receiver_a=e.receiver_b=0;e.control_ball(&p);
            }
            if(e.ball_poss==p.tm_player){
                ++receptions;receive_tick=ticks;selected=i;pass_target=0;receiver_prepared=false;
                status="Received - keep moving or pass again";
            }
        }
        if(!e.ball_poss && pass_target==p.tm_player && !receiver_prepared && p.tm_act<=1 &&
           !e.ball_limbo_on){
            e.uf2=1;e.want_pass=0;e.predict_ball();receiver_prepared=e.go_to_path(p.tm_player)!=0;e.uf2=0;
        }
        if(p.tm_act<=1 && !(p.int_move==1 && p.tm_strike>0)){
            if(i==selected){e.ux=play_x;e.uy=play_y;e.um=(play_x!=0||play_y!=0);}
            // The native input adapter stops on neutral; do not ask user_run
            // to execute go_forward after init_run_act has already stopped.
            if(p.tm_act==1 && !e.um)e.init_stand_act(&p);
            else if(p.tm_act==1)e.user_run(&p);
            else if(e.um)e.user_stand(&p);
        }else e.dispatch_action(&p);
        if(owner && !e.ball_poss){++releases;release_tick=ticks;if(e.real_shot)++shots;}
        if(p.tm_act || e.ball_poss!=p.tm_player)e.process_dir(&p);
        if(!std::isfinite(p.tm_x)||!std::isfinite(p.tm_y)||!std::isfinite(p.tm_xdis)||!std::isfinite(p.tm_ydis))throw std::runtime_error("Non-finite original controlled player");
    }
    if(!std::isfinite(e.ballx)||!std::isfinite(e.bally)||!std::isfinite(e.ballz))throw std::runtime_error("Non-finite original controlled ball");
}
}
