#include "original_exit.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace actua {
void OriginalExit::start(const original::PlayerEngine& source,float x,float y){
 finish();engine=std::make_unique<original::PlayerEngine>(source);auto& e=*engine;
 presentation=OriginalPresentation{};presentation.observe(source,1,-1);
 e.tunnel_x=x;e.tunnel_y=y;e.ball_poss=e.ball_limbo_on=e.ball_in_hands=e.just_scored=e.shot_pending=e.user_controlled=e.sender_1_2=0;
 e.uf1=e.uf2=e.um=e.must_face=e.stop_and_face=0;e.match_mode=0;
 for(auto& p:e.teams){p.control=0;p.tm_limbo=p.tm_strike=p.int_move=p.int_cnt=0;p.tm_z=p.tm_zdis=0;e.init_stand_act(&p);p.go_ty=-9999;}
 ticks_=0;presentation.observe(e,1,0);
}
int OriginalExit::remaining()const{if(!engine)return 0;return int(std::count_if(engine->teams.begin(),engine->teams.end(),[](const auto& p){return p.guy_on && p.draw_me;}));}
void OriginalExit::advance(double seconds){
 if(!active() || paused || !std::isfinite(seconds))return;
 clock_+=std::clamp(seconds,0.0,.1)*20;
 while(clock_>=1 && active()){
  auto& e=*engine;
  for(int i=0;i<22;++i){auto& p=e.teams[i];if(!p.guy_on || !p.draw_me)continue;
   float x=0,y=0;e.player_num=p.tm_player;e.tunnel_pos(i<11?1:2,i%11,x,y);
   if(i>=11){x=1280-x;y=800-y;} // Original get_tac converts team B's relative target to world space.
   if(!p.draw_me)continue;
   if(p.tm_act!=1 || x!=p.go_tx || y!=p.go_ty)e.init_run_act(&p,x,y,false);
   e.process_anims(&p);if(p.tm_act==1)e.run_action(&p);e.process_dir(&p);
   if(!std::isfinite(p.tm_x) || !std::isfinite(p.tm_y))throw std::runtime_error("Non-finite tunnel movement");
  }
  step_original_exit_officials(e);
  ++ticks_;clock_-=1;presentation.observe(e,1,ticks_);
  // Native timeout guarantees an unusual original action cannot block menus.
  if(!remaining() || ticks_>=900)finish();
 }
}
}
