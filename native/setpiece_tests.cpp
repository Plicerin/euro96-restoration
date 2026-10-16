#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "action_constants.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
using namespace actua;
static void require(bool b,const char* m){if(!b)throw std::runtime_error(m);}
int main(int argc,char** argv){try{
 OriginalGraphics g(argv[1]);auto& s=*g.sequence;auto& e=s.engine;
 for(int half=0;half<2;++half)for(int stage=0;stage<6;++stage)for(int kind=0;kind<(stage==0?1:3);++kind){
  s.practice_restart(stage);
  if(half){e.swap_teams();e.match_half=1; // Re-award the corresponding original restart at the other end.
   const int modes[]={original::action_constants::PEN_KICK_B,original::action_constants::DF_KICK_B,original::action_constants::IF_KICK_B,original::action_constants::CORNER_TL,original::action_constants::THROW_IN_B,original::action_constants::GOAL_KICK_TR};
   e.match_mode=0;s.restart_mode=modes[stage];s.restart_ticks=1;e.last_touch=2;e.fkick_taker=13;e.penalty_taker=0;s.step();
  }
  require(s.human_restart(),"Home restart must use human control, including changed ends");
  for(int i=0;i<150;++i){s.set_restart_input(0,0,false,false,false);s.step();}
  require(s.human_restart() && e.ball_poss==e.setp_taker,"No CPU auto-release while waiting");
  require(s.selected==e.setp_taker-1 && s.is_blue(s.selected),"Original taker selected");
  const float bx=e.ballx,by=e.bally;
  if(stage!=4){
   for(int i=0;i<5;++i){s.set_restart_input(0,0,true,false,false);s.step();}
   s.clear_play_input();for(int i=0;i<5;++i){s.set_restart_input(0,0,true,false,false);s.step();}
   require(s.human_restart() && !e.setp_hgt,"Pause cancels charge and requires neutral");
   s.set_restart_input(0,0,false,false,false);s.step();
   if(stage!=0){const float old=e.setp_kick_y;s.set_restart_input(1,-1,false,false,false);s.step();require(e.setp_power>0 && e.setp_kick_y!=old,"Original aiming and power input");}
   require(e.ballx==bx && e.bally==by,"Aiming must not drag the ball");
  }
  for(int i=0;i<(kind?100:8) && s.human_restart();++i){s.set_restart_input(0,stage==4?1:0,kind==0,kind==1,kind==2);s.step();}
  s.set_restart_input(0,0,false,false,false);
  for(int i=0;i<200 && e.match_mode;++i)s.step();
  if(e.match_mode)std::cerr<<"stuck "<<half<<" stage "<<stage<<" kind "<<kind<<" act "<<e.teams[s.selected].tm_act<<" f2 "<<e.f2_down<<"\n";
  require(!e.match_mode && !s.restart_mode,"Manual release must resume match");
  bool released=!e.ball_poss;
  for(int i=0;i<100;++i){s.step();released|=!e.ball_poss;require(std::isfinite(e.ballx) && std::isfinite(e.ballz),"Finite ball after original contact");}
  require(released,"Original animation must release the ball");
  require(!s.shootout && e.penalties==0,"Match penalties must not become shootout attempts");
  std::cout<<"half "<<half<<" stage "<<stage<<" kind "<<kind<<" passed\n";
 }
 for(auto kick:{OriginalActions::PlayKick::ground,OriginalActions::PlayKick::loft,OriginalActions::PlayKick::shot}){
  s.practice_restart(6);for(int i=0;i<160;++i)s.step();require(s.human_keeper_hold(),"Home keeper waits for distribution input");
  std::cout<<"keeper kind "<<int(kick)<<" act "<<e.teams[s.selected].tm_act<<" busy "<<e.teams[s.selected].int_cnt<<std::endl;
  s.set_play_input(1,0);s.request_play_kick(kick);bool released=false;
  for(int i=0;i<140;++i){s.step();released|=!e.ball_poss;}
  require(released,"Keeper pass/loft/punt must release at original contact");
 }
 // Human selection must not strand either the ground pickup countdown or a
 // completed save recovery. Existing delivery checks alone miss looping poses.
 for(int half=0;half<2;++half)for(int recovery:{0,-1,1,2}){
  s.start_match(false);if(half){e.swap_teams();e.match_half=1;}s.restart_ticks=s.restart_mode=e.match_mode=0;
  const int slot=s.blue_slot();auto& keeper=e.teams[slot];s.selected=slot;
  e.holder_lose_ball();keeper.tm_x=slot?1210:70;keeper.tm_y=400;
  e.collect_ball(&keeper);
  if(!recovery){e.ball_in_hands=0;e.init_pickup_act(&keeper);}
  else{e.ball_in_hands=1;keeper.tm_act=10;e.keeper_down=recovery;e.init_stand_act(&keeper);e.hold_ball(&keeper);}
  const auto initial=keeper.tm_anim;int ticks=0;
  while(ticks++<160 && (keeper.int_cnt || keeper.tm_anim!=MC_STANDB))s.step();
  require(ticks<160 && keeper.tm_anim!=initial && keeper.tm_anim==MC_STANDB,"Human keeper finishes pickup/get-up in standing hold pose");
  require(s.human_keeper_hold() && e.ball_poss==slot+1,"Recovery keeps the caught ball with its keeper");
  for(int i=0;i<30;++i){s.step();require(keeper.tm_anim==MC_STANDB,"Recovered keeper stays standing while awaiting input");}
  s.set_play_input(slot?-1.f:1.f,0);s.request_play_kick(OriginalActions::PlayKick::shot);
  bool released=false;for(int i=0;i<120;++i){s.step();released|=!e.ball_poss;}
  require(released,"Recovered keeper can distribute normally");
 }
 int catches=0;
 for(int seed=1;seed<=8 && catches<2;++seed){
  s.start_match(true);std::srand(seed);
  for(int t=0;t<12000 && s.running;++t){s.step();
   if(e.ball_in_hands && e.ball_poss==s.blue_slot()+1 && e.teams[s.blue_slot()].tm_act==10){
    s.set_cpu_control(false);for(int recovery=0;recovery<260;++recovery)s.step();
    require(s.human_keeper_hold(),"Caught ball must survive original recovery and await human delivery");
    s.set_play_input(s.blue_slot()?-1.0f:1.0f,0);s.request_play_kick(OriginalActions::PlayKick::shot);
    bool released=false;for(int i=0;i<150;++i){s.step();released|=!e.ball_poss;}
    require(released,"Real caught ball must allow a human punt after recovery");++catches;break;
   }
  }
 }
 require(catches>=1,"Exercise actual original goalkeeper catch/recovery, not only placement");
 std::cout<<"Human restarts at both ends, cancellation and keeper deliveries passed\n";return 0;
}catch(const std::exception& x){std::cerr<<x.what()<<"\n";return 1;}}

