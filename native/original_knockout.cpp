#include "original_actions.hpp"
#include "action_constants.hpp"
#include <cmath>
#include <algorithm>
namespace actua {
using namespace original::action_constants;
void OriginalActions::clear_penalty_input(){
    penalty_button=penalty_armed=false;penalty_direction=0;
    // Dropping focus or pausing cancels a charge, never releases a kick.
    if(penalty_human_taker() && engine.match_mode && engine.penalty_taker>0 && engine.teams[engine.penalty_taker-1].int_move!=15)
        engine.uf1=engine.f1_down=engine.setp_hgt=0;
}
void OriginalActions::set_penalty_input(float direction,bool shot){
    if(!shootout || cpu_vs_cpu || full_time || penalty_wait){clear_penalty_input();return;}
    if(!shot)penalty_armed=true; // Require neutral input after a menu/attempt/focus transition.
    penalty_button=penalty_armed && shot;
    penalty_direction=std::isfinite(direction)?std::clamp(direction,-1.0f,1.0f):0;
}
const char* OriginalActions::format_name(MatchFormat format){
    switch(format){case MatchFormat::knockout:return "Knockout";case MatchFormat::golden:return "Golden goal";case MatchFormat::penalties:return "Penalties only";default:return "Regulation";}
}
std::string OriginalActions::period_label()const{
    if(full_time)return result_label();
    if(shootout)return "PENALTY SHOOTOUT";
    if(interval_ticks)return interval_minute==90?"EXTRA TIME":interval_minute==105?"EXTRA-TIME INTERVAL":"HALF TIME";
    return engine.match_half==3?"Extra time - second half":engine.match_half==2?"Extra time - first half":engine.match_half==1?"Second half":"First half";
}
std::string OriginalActions::result_label()const{
    const int a=shootout?penalty_scores[0]:blue_goals,b=shootout?penalty_scores[1]:red_goals;
    if(a==b)return "FULL TIME - DRAW";
    return team_label(a<b)+" WINS"+(shootout?" ON PENALTIES":engine.golden_goal==2?" - GOLDEN GOAL":engine.match_half>1?" AFTER EXTRA TIME":"");
}
void OriginalActions::finish_match(){if(!full_time)engine.PlayCommentaryMessage(43);if(engine.golden_goal==2)engine.play_fx(4,engine.refs[0].x,engine.refs[0].y);full_time=true;running=false;engine.stop_clock=1;clear_play_input();status=result_label();}
void OriginalActions::start_shootout(){
    shootout_figures=match_figures();
    auto& e=engine;shootout=true;penalty_scores={};penalty_results={};pending_subs={};
    for(int side=0;side<2;++side){match_passes[side]=sub_passes[side];match_shots[side]=sub_shots[side];}
    for(int i=0;i<22;++i){match_passes[!is_blue(i)]+=e.teams[i].tm_pass;match_shots[!is_blue(i)]+=e.teams[i].tm_shots;}
    // FOOTBALL.init_match shootout initializers; watch_match_time resets only shootout scores.
    e.penalties=0;e.home_pens_left=e.away_pens_left=5;e.penalty_game=1;e.penalty_team=0;e.penalty_counter=0;e.pen_taker_a=11;e.pen_taker_b=22;
    e.team_a_goals=e.team_b_goals=0;e.match_mode=0;e.just_scored=0;e.reset_ball();e.stop_clock=1;
    restart_ticks=restart_mode=0;penalty_wait=40;penalty_age=0;penalty_released=false;clear_play_input();
    ++generation;status="Penalty shootout";
}
void OriginalActions::finish_penalty(bool goal){
    if(penalty_wait || full_time)return; // An attempt is recorded once, then motion continues.
    auto& e=engine;
    if(goal)e.record_goal_score(e.goal_end);
    penalty_scores={e.team_a_goals,e.team_b_goals};penalty_results[penalty_side].push_back(goal?1:0);
    status=team_label(penalty_side!=0)+(goal?" scores":" misses")+" - penalties "+std::to_string(penalty_scores[0])+":"+std::to_string(penalty_scores[1]);
    // Preserve the goal latch, flight, held ball and presentation snapshots until
    // prepare_restart resets them for the next kick. Recording is not a reset.
    e.match_mode=0;e.penalty_counter=0;e.stop_clock=1;restart_mode=0;penalty_wait=60;penalty_released=false;clear_play_input();
}
void OriginalActions::step_shootout(){
    auto& e=engine;++ticks;++e.logic_cnt;e.af_randomize();e.prediction=0;e.stop_clock=1;update_match_depth();
    if(penalty_wait){
        // penalties is zero only during the initial shootout introduction.
        if(e.penalties>0){
            e.user_controlled=e.uf1=e.uf2=e.um=0;e.ux=e.uy=0;
            e.prev_ballx=e.ballx;e.prev_bally=e.bally;e.prev_ballz=e.ballz;
            e.update_ball_limbo();
            if(!e.ball_poss && !e.ball_limbo_on){e.ball_trajectory();e.ball_collision();}
            e.keeper_boxes();
            const int taker=e.penalty_taker-1,keeper=taker<11?11:0;
            for(int i:{taker,keeper}){
                auto& p=e.teams[i];p.control=0;e.process_anims(&p);
                if(p.tm_limbo)continue;
                if(i==keeper)e.keeper_busy(&p);
                if(p.tm_act!=0)e.dispatch_action(&p);
                if(e.ball_poss==p.tm_player)e.hold_ball(&p);
                e.process_dir(&p);
            }
            // No new shot/save decisions or ball interactions: the outcome is final.
        }
        if(--penalty_wait)return;
        e.match_mode=0;e.penalty_counter=0;e.do_penalties();
        if(!e.penalty_game){finish_match();return;}
        restart_mode=e.match_mode;penalty_side=is_blue(e.penalty_taker-1)?0:1;
        prepare_restart();penalty_age=0;penalty_released=false;
        e.setp_power=e.setp_hgt=e.setp_bzd=0;e.setp_byd=e.cntspot_y;e.f1_down=e.f2_down=e.uf1=e.uf2=0;
        const int keeper=e.penalty_taker<=11?11:0;e.teams[keeper].tm_x=keeper?float(e.pitch_len):0;e.teams[keeper].tm_y=float(e.cntspot_y)-1;e.init_anim(&e.teams[keeper],MC_PENALTY);
        e.save_direction=0;selected=penalty_side==0?e.penalty_taker-1:keeper;
        for(auto& p:e.teams)p.control=0;
        if(!cpu_vs_cpu)e.teams[selected].control=1;
        if(head_to_head){auto scope=input_scope(1);selected=penalty_side==1?e.penalty_taker-1:keeper;e.teams[selected].control=2;}
        status=team_label(penalty_side!=0)+" penalty - "+player_label(e.penalty_taker-1);return;
    }
    if(e.match_mode){
        for(int i:visible_players)e.process_anims(&e.teams[i]);
        if(e.dead_ball_cnt>1){--e.dead_ball_cnt;return;}
        auto& taker=e.teams[e.penalty_taker-1];e.refs[0].act=2;
        auto taker_scope=input_scope(head_to_head?penalty_side:0);
        const bool human=penalty_human_taker();e.user_controlled=taker.control=human?char(input_side+1):0;
        if(human)e.uf1=penalty_button;
        if(e.ball_poss!=e.penalty_taker)e.collect_ball(&taker);
        e.user_pen_kick(&taker,human?1:0);e.process_dir(&taker);
        if(!e.match_mode){restart_mode=0;penalty_age=0;}
        else if(!human && ++penalty_age>1600)throw std::runtime_error("Original CPU penalty aim failed to release");
        return;
    }
    ++penalty_age;
    e.prev_ballx=e.ballx;e.prev_bally=e.bally;e.prev_ballz=e.ballz;e.update_ball_limbo();
    if(!e.ball_poss && !e.ball_limbo_on){
        if(e.shot_acknowledged)--e.shot_acknowledged;
        if(e.shot_pending){if(!++e.shot_pending)e.shot_pending=1;}else e.real_shot=0;
        e.ball_trajectory();e.ball_collision();
    }
    if(e.ball_in_goal){finish_penalty(true);return;}
    if(penalty_released && (e.ballx<0 || e.ballx>e.pitch_len || e.bally<0 || e.bally>e.pitch_wid || e.ball_in_hands || penalty_age>200 || (penalty_age>40 && std::hypot(e.ballxdis,e.ballydis)<.05f && e.ballz<3))){finish_penalty(false);return;}
    e.get_ball_zone();e.ball_zone1=std::clamp(e.ball_zone1,0,31);e.ball_zone2=std::clamp(e.ball_zone2,0,31);e.keeper_boxes();e.player_distances();e.predict_ball();e.get_nearest();e.get_opp_near_ball(e.ball_poss);
    e.ball_speed=int(std::sqrt(e.ballxdis*e.ballxdis+e.ballydis*e.ballydis+e.ballzdis*e.ballzdis));
    const int taker=e.penalty_taker-1,keeper=taker<11?11:0;
    // Original kick contacts/trajectory/save decisions; native shootout phase excludes follow-up shots.
    for(int i:{taker,keeper}){
        auto player_scope=input_scope(head_to_head && !is_blue(i)?1:0);
        auto& p=e.teams[i];const bool manual_keeper=i==keeper && penalty_human_keeper();
        e.user_controlled=p.control=(manual_keeper || (i==taker && penalty_human_taker()))?char(input_side+1):0;e.process_anims(&p);
        if(p.tm_limbo || (e.ball_limbo_on && e.ball_limbo_p==p.tm_player))continue;
        if(i==keeper || !penalty_released)e.ball_interact(&p);
        if(i==keeper){
            if(!manual_keeper)e.intelligence(&p);
            else if(!e.keeper_busy(&p) && p.tm_act<=1 && penalty_button && e.shot_pending>0 && !e.shot_acknowledged){
                const int direction=penalty_direction>.24f?1:penalty_direction<-.24f?-1:0;
                e.save_direction=direction*(keeper==11?-1:1);
                // The original EURO96 direction test and save selection run together.
                // Retain penalty end context for init_user_save after ready_set_kick.
                const auto mode=e.match_mode;e.match_mode=taker<11?PEN_KICK_A:PEN_KICK_B;
                e.go_to_save_path(&p);e.match_mode=mode;
            }
        }
        if(p.tm_act!=0)e.dispatch_action(&p);
        e.process_dir(&p);
    }
    // A kick can release through limbo after its animation contact.
    if(!penalty_released && !e.ball_poss && !e.ball_limbo_on && (e.shot_pending || e.real_shot)){penalty_released=true;++releases;release_tick=ticks;}
    if(penalty_age>400)throw std::runtime_error("Original penalty failed to release");
}
}

