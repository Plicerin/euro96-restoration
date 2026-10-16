#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "action_constants.hpp"
#include <iostream>
#include <cmath>
#include <stdexcept>
using namespace actua::original::action_constants;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static void run(actua::OriginalActions& s,bool trace=false){
 bool recovery_pose=false,held_pose=false;int frozen=0;
 float caught_x=0,minimum_x=2000;
 while(s.running){
  auto& e=s.engine;auto& k=e.teams[11];
  const bool limbo=e.ball_limbo_on && e.ball_limbo_p==12 && k.tm_frm+k.tm_fstep<=e.ball_limbo_c;
  const float bx=e.ballx,by=e.bally,bz=e.ballz;
  s.step();
  if(limbo){require(e.ballx==bx && e.bally==by && e.ballz==bz,"Distribution physics must wait for original animation contact");++frozen;}
  if(s.catch_tick==s.ticks)caught_x=k.tm_x;
  if(e.ball_in_hands){require(e.ball_poss==12,"Held ball must remain with keeper");minimum_x=std::min(minimum_x,k.tm_x);}
  recovery_pose|=k.tm_anim==MC_STOSBL || k.tm_anim==MC_STOSBR || k.tm_anim==MC_CRTOSTB;
  held_pose|=k.tm_anim==MC_STANDB || k.tm_anim==MC_BOUNCE || k.tm_anim==MC_WAVEUP;
  if(trace)std::cout<<s.ticks<<" owner "<<e.ball_poss<<" keeper "<<k.tm_act<<","<<k.tm_anim<<","<<k.int_cnt<<" ball "<<e.ballx<<","<<e.bally<<","<<e.ballz<<" limbo "<<int(e.ball_limbo_on)<<" "<<s.status<<'\n';
 }
 std::cout<<"Catch "<<s.catch_tick<<", recovery "<<s.recovery_tick<<", carry "<<s.carry_ticks<<", distribution "<<s.distribution_tick<<" type "<<s.distribution_type<<" animation "<<s.distribution_animation<<", reception "<<s.receive_tick<<", end "<<s.ticks<<'\n';
 require(s.shots==1 && s.saves==1 && s.distributions==1 && s.receptions==1 && s.engine.ball_poss==13,"Complete attack/save/distribution/reception chain must finish once");
 require(recovery_pose && held_pose && s.recovery_tick>s.catch_tick,"Original get-up and held-ball captures must run before distribution");
 require(s.carry_ticks>0 && minimum_x<caught_x,"Keeper must physically carry the ball upfield");
 require(frozen>1 && s.receive_tick>s.distribution_tick,"Distribution must preserve animation delay before reception");
 require(!s.engine.ball_limbo_on && !s.engine.ball_in_hands && !s.miss,"Reception must clear hand/limbo state");
 const int tick=s.ticks;const float x=s.engine.ballx;s.step();require(s.ticks==tick && s.engine.ballx==x,"Completed continuation must be stable");
}
int main(int argc,char** argv){try{
 actua::OriginalGraphics g(argv[1]);auto& s=*g.sequence;
 s.start_continuation(argc>3?std::stof(argv[3]):1060,argc>4?std::stof(argv[4]):260);run(s,argc>2);
 if(argc>2)return 0;
 for(auto target:{std::pair{1050.0f,280.0f},std::pair{1040.0f,270.0f}}){s.start_continuation(target.first,target.second);run(s);}
 // Isolated original decisions: provide explicit keeper possession and an open outlet.
 auto setup=[&](float distance){
  s.start_continuation();auto& e=s.engine;auto& k=e.teams[11];auto& p=e.teams[12];
  for(int i=0;i<11;++i)e.teams[i].guy_on=0;
  e.holder_lose_ball();k.tm_x=1200;k.tm_y=400;k.tm_xdis=k.mface_x=-1;k.tm_ydis=k.mface_y=0;
  e.collect_ball(&k);e.ball_in_hands=1;e.init_kphold_act(&k);e.init_anim(&k,MC_STANDB);e.hold_ball(&k);
  p.tm_x=e.ballx-distance;p.tm_y=e.bally;p.control=1;
  e.want_pass=13; // Original request-pass input also allows a longer half-volley outlet.
  e.player_distances();e.get_opp_near_ball(12);e.keeper_boxes();e.seed=127;
 };
 for(auto mode:{std::pair{60.0f,9},std::pair{150.0f,10},std::pair{250.0f,11}}){
  setup(mode.first);auto& e=s.engine;auto& k=e.teams[11];int chosen=0;
  for(int attempt=0;attempt<32 && !chosen;++attempt)chosen=e.pass_decide(&k,0);
  require(chosen==13 && e.pass_type==mode.second,"Original distance rules must choose roll, throw and half-volley");
  e.make_pass(&k,chosen,0);
  const int expected=mode.second==9?MC_ROLL:mode.second==10?MC_THROW:MC_HALFVOLL;
  require(k.tm_anim==expected && e.ball_limbo_on && !e.ball_poss,"Keeper distribution must enter its original capture and limbo");
  int delay=0;
  while(e.ball_limbo_on && delay<100){e.update_ball_limbo();if(e.ball_limbo_on)e.process_anims(&k);++delay;}
  require(delay>1 && delay<100 && !e.ball_limbo_on,"Each keeper delivery must release at its animation contact");
 }
 setup(150);auto& e=s.engine;auto& k=e.teams[11];auto& p=e.teams[12];
 e.teams[3].guy_on=1;e.teams[3].tm_x=p.tm_x;e.teams[3].tm_y=p.tm_y;
 require(e.pass_decide(&k,0)==0 && e.ball_poss==12,"Keeper must reject a marked outlet without losing possession");
 setup(400);require(e.pass_decide(&k,0)==0,"Keeper must reject an out-of-range outlet");
 setup(150);e.must_punt=1;e.distribute_held_ball(&k);
 require(k.tm_anim==MC_KICKOUT && e.ball_limbo_on && !e.ball_poss,"Original forced punt must use kick-out release timing");
 setup(150);e.init_get_up(&k,0.1f);const int duration=k.int_cnt;
 for(int i=1;i<duration;++i)require(e.keeper_busy(&k)!=0,"Keeper recovery must stay busy until its final tick");
 require(e.keeper_busy(&k)==0 && !k.int_cnt && !k.int_move,"Recovery expiry must reset the original intent");
 std::cout<<"Original keeper recovery, carrying, outlet rejection, roll/throw/half-volley, punt and reception contracts passed.\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
