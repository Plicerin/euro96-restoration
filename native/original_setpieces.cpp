#include "original_actions.hpp"
#include "action_constants.hpp"
#include <cmath>
#include <algorithm>
namespace actua {
using namespace original::action_constants;
bool OriginalActions::human_restart()const{
 return team_play && !shootout && !cpu_vs_cpu && !full_time && engine.match_mode && engine.setp_taker>0 && is_controlled_team(engine.setp_taker-1) && engine.set_piece_on!=SETP_CENTRE;
}
bool OriginalActions::human_keeper_hold()const{
 return team_play && !shootout && !cpu_vs_cpu && !restart_mode && engine.ball_in_hands && engine.ball_poss==control_slot()+1;
}
void OriginalActions::clear_restart_input(){
 restart_armed=restart_shot=restart_pass=restart_loft=restart_high_pass=false;restart_x=restart_y=0;
 if(human_restart() && engine.teams[engine.setp_taker-1].int_move!=15){engine.setp_hgt=engine.f1_down=engine.f2_down=engine.uf1=engine.uf2=0;}
}
void OriginalActions::set_restart_input(float x,float y,bool shot,bool pass,bool loft){
 if(!human_restart()){clear_restart_input();return;}
 if(!shot && !pass && !loft)restart_armed=true;
 restart_shot=restart_armed && shot;restart_pass=restart_armed && pass;restart_loft=restart_armed && loft;
 restart_x=std::isfinite(x)?std::clamp(x,-1.0f,1.0f):0;
 restart_y=std::isfinite(y)?std::clamp(y,-1.0f,1.0f):0;
 const float length=std::hypot(restart_x,restart_y);if(length>1){restart_x/=length;restart_y/=length;}
}
std::string OriginalActions::restart_help()const{
 if(human_keeper_hold())return "Aim direction; pass, loft or punt";
 if(engine.set_piece_on==SETP_PENALTY)return "Time the aim; hold shot, release";
 if(engine.set_piece_on==SETP_THROW)return "Aim into pitch + pass / shot";
 return "Left/right aim; up/down power";
}
void OriginalActions::practice_restart(int stage){
 // Diagnostic placement invokes the same original setup as an awarded restart.
 // Start a separate practice match, so a fixture never changes the user's score.
 next_format=MatchFormat::regulation;start_match(false);restart_practice=(stage%7+7)%7;
 auto& e=engine;restart_ticks=0;e.match_mode=0;e.dead_ball_cnt=0;
 e.incident_x=restart_practice==4?640:1000;e.incident_y=restart_practice==4?0:350;
 e.ballx=e.incident_x;e.bally=e.incident_y;e.last_touch=13;e.fkick_taker=2;
 if(restart_practice==6){
     restart_mode=0;e.set_piece_on=0;e.reset_ball();e.ball_out_of_play=0;
     auto& keeper=e.teams[0];keeper.tm_x=70;keeper.tm_y=400;e.reset_ideas(&keeper);e.init_stand_act(&keeper);
     e.ball_in_hands=1;e.collect_ball(&keeper);e.hold_ball(&keeper);selected=0;
     status="Keeper distribution practice - aim, pass or punt";return;
 }
 const int modes[]={PEN_KICK_A,DF_KICK_A,IF_KICK_A,CORNER_TR,THROW_IN_A,GOAL_KICK_TL};
 restart_mode=modes[restart_practice];prepare_restart();
}
}
