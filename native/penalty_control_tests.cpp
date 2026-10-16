#include "original_graphics.hpp"
#include "original_actions.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <cstdlib>
using namespace actua;
static void require(bool b,const char* m){if(!b)throw std::runtime_error(m);}
int main(int argc,char** argv){try{
 OriginalGraphics g(argv[1]);auto& s=*g.sequence;auto& e=s.engine;g.action_view=true;
 auto ready=[&]{s.next_format=OriginalActions::MatchFormat::penalties;s.start_match(false);
  for(int i=0;i<2000 && !(s.penalty_human_taker() && s.penalty_aiming());++i){s.set_penalty_input(0,false);s.step();}
  require(s.penalty_human_taker() && s.penalty_aiming(),"Human taker must become ready");
  require(s.is_blue(s.selected) && s.selected==e.penalty_taker-1,"Human selection must follow home taker after end swap");
 };
 ready();const int count=e.penalties;const float aim=e.setp_byd;
 for(int i=0;i<2000;++i){s.set_penalty_input(0,false);s.step();}
 require(e.penalties==count && !e.f1_down && s.penalty_aiming(),"Human must be allowed to wait without CPU timeout or auto-shot");
 require(e.setp_byd!=aim,"Original target sweeps while waiting");
 for(int i=0;i<8;++i){s.set_penalty_input(0,true);s.step();}
 require(e.setp_hgt==8,"Held shot controls original height");
 s.clear_play_input();require(!e.f1_down && !e.setp_hgt,"Pause/focus cancel charge without kicking");
 for(int i=0;i<12;++i){s.set_penalty_input(0,true);s.step();}
 require(!e.f1_down && s.penalty_aiming(),"Held button on resume must not charge");
 s.set_penalty_input(0,false);s.step();
 float vz[2]{};
 for(int trial=0;trial<2;++trial){ready();const int hold=trial?16:4;
  for(int i=0;i<hold;++i){s.set_penalty_input(0,true);s.step();}
  require(e.setp_hgt==hold,"Low/high original charge value");
  s.set_penalty_input(0,false);s.step();require(e.teams[e.penalty_taker-1].int_move==15 && e.penalty_shot,"Release starts original run-up");
  const float dx=e.setp_kick_x,dy=e.setp_kick_y;
  for(int i=0;i<200 && !s.penalty_released;++i)s.step();
  require(s.penalty_released && s.releases>0 && !e.ball_limbo_on,"Original animation contact releases human shot");
  require(std::abs(e.ballzdis-float(hold)/3)<.001f,"Actual launch height must equal original get_traj, not CPU automatic shot");
  require(e.setp_kick_x==dx && e.setp_kick_y==dy,"Run-up must retain chosen direction");
  const float speed=std::hypot(e.ballxdis,e.ballydis);
  require(std::abs(speed-e.get_power())<.001f,"Penalty launch speed uses original power");
  require((e.ballxdis*dx+e.ballydis*dy)/speed>.9999f,"Actual shot follows the timed aim marker");vz[trial]=e.ballzdis;
 }
 require(vz[1]>vz[0],"Longer hold must produce a higher original trajectory");
 ready();for(int i=0;i<70;++i){s.set_penalty_input(0,true);s.step();}require(e.setp_hgt==30 && e.f1_down==30,"Charge is capped at original 30 ticks");
 // Full human/CPU shootouts exercise directional saves and neutral centre saves.
 int dives=0,saved=0,manual_kicks=0;
 for(int seed=1;seed<=12;++seed){s.start_match(false);std::srand(seed);int age=0,attempt=-1;
  for(int tick=0;tick<20000 && s.running;++tick){
   if(e.penalties!=attempt){attempt=e.penalties;age=0;}
   bool down=false;float dir=0;
   if(s.penalty_human_taker() && s.penalty_aiming()){down=age>0 && age<1+(seed%2?4:16);++age;}
   if(s.penalty_human_keeper()){down=e.shot_pending>0;dir=float(seed%3-1);require(s.selected==s.blue_slot(),"Home keeper selected after end swap");}
   s.set_penalty_input(dir,down);const auto old=e.teams[s.blue_slot()].tm_act;s.step();
   if(old!=10 && e.teams[s.blue_slot()].tm_act==10)++dives;
   require(std::isfinite(e.ballx)&&std::isfinite(e.ballz)&&std::isfinite(e.teams[s.blue_slot()].tm_y),"Finite human shootout");
  }
  require(s.full_time && s.penalty_scores[0]!=s.penalty_scores[1],"Human/CPU shootout completes with winner");
  saved+=e.teams[s.blue_slot()].tm_kpsaves;manual_kicks+=int(s.penalty_results[0].size());
  std::cout<<"human shootout "<<seed<<" "<<s.penalty_scores[0]<<":"<<s.penalty_scores[1]<<" saves "<<e.teams[s.blue_slot()].tm_kpsaves<<std::endl;
 }
 require(dives>0 && saved>0 && manual_kicks>=36,"Manual keeper must animate and make real original saves");
 s.next_format=OriginalActions::MatchFormat::regulation;s.start_match(false);require(!s.shootout,"Regulation reset clears shootout");s.set_play_input(1,0);s.set_penalty_input(0,true);
 std::cout<<"Human timed aim/height/release, cancellation, identity and 12 shootouts passed; "<<dives<<" dives, "<<saved<<" saves, "<<manual_kicks<<" manual kicks; launch vz "<<vz[0]<<" / "<<vz[1]<<std::endl;
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 1;}}
