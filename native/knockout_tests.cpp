#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "action_constants.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cmath>
using namespace actua;
using namespace actua::original::action_constants;
static void require(bool b,const char* m){if(!b)throw std::runtime_error(m);}
int main(int argc,char** argv){try{
 OriginalGraphics g(argv[1]);g.action_view=true;auto& s=*g.sequence;auto& e=s.engine;
 auto quiet=[&](int half,int minute){while(e.match_half<half)e.swap_teams();e.match_time.min=minute;e.match_time.sec=0;e.match_mode=e.dead_ball_cnt=e.game_action=0;s.restart_ticks=s.restart_mode=0;e.reset_ball();e.ballx=640;e.bally=400;e.ballz=2;e.shot_pending=e.real_shot=e.ball_limbo_on=e.ball_in_goal=0;};
 s.next_format=OriginalActions::MatchFormat::knockout;s.start_match(true);quiet(1,90);s.step();require(s.interval_ticks && s.interval_minute==90 && !s.full_time,"Tied knockout enters extra time");
 for(int i=0;i<100;++i)s.step();require(e.match_half==2 && e.match_time.min==90 && s.blue_slot()==0,"Extra time preserves roster/end identity");
 quiet(2,105);s.step();require(s.interval_ticks && s.interval_minute==105,"105-minute interval");for(int i=0;i<100;++i)s.step();require(e.match_half==3 && e.match_time.min==105 && s.blue_slot()==11,"Extra time end swap");
 quiet(3,120);s.step();require(s.shootout && !s.full_time,"Tied extra time enters shootout");
 s.start_match(true);s.blue_goals=e.team_a_goals=1;quiet(1,90);s.step();require(s.full_time && !s.shootout,"Decided regulation knockout ends at 90");
 s.start_match(true);s.blue_goals=e.team_a_goals=2;s.red_goals=e.team_b_goals=1;quiet(3,120);s.step();require(s.full_time && !s.shootout,"Decided extra time ends at 120");
 s.next_format=OriginalActions::MatchFormat::golden;s.start_match(true);quiet(2,94);e.ballx=1281;e.last_touch=2;e.go_right_goal();s.step();require(e.golden_goal==2 && s.blue_goals==1,"Original extra-time golden goal flag");
 for(int i=0;i<1000 && s.running;++i)g.advance(.05);require(s.full_time && !s.shootout && e.match_half==2,"Golden goal finishes after celebration and replay");
 // Original early-clinch and sudden death predicates independent of random shot outcomes.
 s.next_format=OriginalActions::MatchFormat::penalties;s.start_match(true);e.match_mode=0;e.penalty_counter=0;e.penalties=6;e.team_a_goals=3;e.team_b_goals=0;e.away_pens_left=2;e.home_pens_left=2;e.do_penalties();require(!e.penalty_game,"3-0 after three each clinches");
 s.start_match(true);e.match_mode=0;e.penalty_counter=0;e.penalties=10;e.team_a_goals=e.team_b_goals=4;e.do_penalties();require(e.penalty_game && e.penalties==11,"Tied five each continues sudden death");
 e.match_mode=0;e.penalty_counter=0;e.team_a_goals=5;e.do_penalties();require(e.penalty_game && e.penalties==12,"Sudden death must give both teams their kick");
 e.match_mode=0;e.penalty_counter=0;e.do_penalties();require(!e.penalty_game,"Unequal completed sudden-death pair finishes");
 // A shootout preserves the drawn match score, statistics and selected format.
 s.next_format=OriginalActions::MatchFormat::knockout;s.start_match(true);s.blue_goals=s.red_goals=e.team_a_goals=e.team_b_goals=2;e.teams[1].tm_pass=7;quiet(3,120);s.step();require(s.shootout && s.blue_goals==2 && s.red_goals==2 && s.match_passes[0]==7,"Shootout must preserve match score/statistics after swapping ends");
 s.next_format=OriginalActions::MatchFormat::penalties;
 int goals=0,misses=0;
 for(int seed=1;seed<=12;++seed){s.start_match(true);std::srand(seed);e.teams[9].guy_on=0;e.teams[20].guy_on=0;
  for(int tick=0;tick<16000 && s.running;++tick){const int previous_releases=s.releases;g.advance(.05);if(s.releases>previous_releases)require(std::abs(e.ballzdis-float(e.setp_hgt)/3)<.001f,"CPU penalty uses original selected height at contact");if(e.penalty_taker)require(e.teams[e.penalty_taker-1].guy_on,"Dismissed player cannot take a penalty");require(std::isfinite(e.ballx)&&std::isfinite(e.ballz),"Finite shootout physics");}
  std::cout<<"shootout "<<seed<<" result "<<s.penalty_scores[0]<<":"<<s.penalty_scores[1]<<" kicks "<<s.penalty_results[0].size()<<":"<<s.penalty_results[1].size()<<" tick "<<s.ticks<<std::endl;
  require(s.full_time && s.penalty_scores[0]!=s.penalty_scores[1],"Shootout must finish with a winner");require(!s.blue_goals && !s.red_goals,"Penalty goals must not change match score");
  for(const auto& side:s.penalty_results)for(int goal:side){goals+=goal;misses+=!goal;}
 }
 require(goals && misses,"Original shots must produce both scores and misses");
 // Full matches forced level at 89 minutes exercise the whole extended scheduler.
 for(auto format:{OriginalActions::MatchFormat::knockout,OriginalActions::MatchFormat::golden})for(int seed=1;seed<=2;++seed){
  s.next_format=format;s.start_match(true);std::srand(seed+10);bool extra=false;
  for(int tick=0;tick<30000 && s.running;++tick){
   if(e.match_half==1 && e.match_time.min>=89)s.blue_goals=s.red_goals=e.team_a_goals=e.team_b_goals=0;
   g.advance(.05);extra|=e.match_half==2;
   require(std::isfinite(e.ballx)&&std::isfinite(e.ballz),"Finite extended match");
  }
  std::cout<<"extended "<<int(format)<<" seed "<<seed<<" "<<s.result_label()<<" at "<<s.clock_label()<<std::endl;
  require(extra && s.full_time,"Extended CPU match must reach extra time and finish");
 }
 std::cout<<"Knockout phases, golden goal, original clinch/rotation and 12 CPU shootouts passed: "<<goals<<" goals / "<<misses<<" misses\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 1;}}


