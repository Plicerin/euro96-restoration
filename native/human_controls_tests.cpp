#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "action_constants.hpp"
#include <cmath>
#include <iostream>
#include <set>
#include <stdexcept>
using namespace actua;
using namespace actua::original::action_constants;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 require(argc==2,"Expected archive path");OriginalGraphics g(argv[1]);auto& s=*g.sequence;auto& e=s.engine;
 auto setup=[&](){
  s.start_match(false);s.full_match=false;s.restart_mode=s.restart_ticks=0;
  e.match_mode=e.set_piece_on=e.dead_ball_cnt=e.game_action=e.just_scored=0;e.ball_out_of_play=0;e.reset_shot();e.holder_lose_ball();
  for(auto& p:e.teams){p.guy_on=0;p.tm_x=80;p.tm_y=80;}
  auto& p=e.teams[1];p.guy_on=1;p.tm_x=950;p.tm_y=400;p.tm_xdis=p.mface_x=1;p.tm_ydis=p.mface_y=0;
  p.control=1;p.tm_vis=p.tm_cont=p.tm_ac=p.tm_pow=p.tm_flair=p.tm_rate=96;p.tm_mcspd=1.5f;p.tm_jump=40;
  e.reset_ideas(&p);e.init_stand_act(&p);s.selected=1;e.collect_ball(&p);e.hold_ball(&p);
  s.set_play_input(0,0);s.set_play_buttons(false,false);
 };
 auto tick=[&](bool shot,bool pass,float x=0,float y=0){s.set_play_input(x,y);s.set_play_buttons(shot,pass);s.step();require(std::isfinite(e.ballx)&&std::isfinite(e.bally)&&std::isfinite(e.ballz),"Finite human ball");};
 auto release=[&](){for(int t=0;t<90 && e.ball_poss==2;++t)tick(false,false);require(e.ball_poss!=2 && s.releases>0,"Kick must release at original contact");};
 setup();for(int i=0;i<10;++i)tick(true,false);require(s.live_charge()==10 && e.ball_poss==2 && e.teams[1].tm_act==0,"Stationary shot charges without releasing");tick(false,false);require(e.teams[1].tm_act==15 && e.drive_shot,"Stationary release selects original drive");release();require(!e.after_touch_on,"Driven shot disables aftertouch");
 setup();tick(true,false,1,0);require(e.teams[1].tm_act==15 && !e.drive_shot,"Moving shot uses original normal shot");release();require(e.after_touch_on==1,"Normal human shot enables aftertouch");
 std::cout<<"normal release: v="<<e.ballxdis<<","<<e.ballydis<<","<<e.ballzdis<<" air="<<int(e.ball_inair)<<" swerve="<<int(e.swerve)<<" xy="<<e.full_xyspin<<" z="<<e.full_zspin<<" spin="<<e.spin_cnt<<" after="<<int(e.after_touch_on)<<"\n";
 const auto original=e;auto curved=original,neutral=original;curved.users_dir[0].x=0;curved.users_dir[0].y=1;neutral.users_dir[0].x=neutral.users_dir[0].y=0;
 for(int t=0;t<8;++t){curved.ball_trajectory();neutral.ball_trajectory();}
 std::cout<<"aftertouch delta="<<curved.bally-neutral.bally<<"\n";require(std::abs(curved.bally-neutral.bally)>.1f,"Original aftertouch bends normal shot");
 auto lofted=original,lowered=original;lofted.users_dir[0].x=-1;lofted.users_dir[0].y=0;lowered.users_dir[0].x=1;lowered.users_dir[0].y=0;
 for(int t=0;t<4;++t){lofted.ball_trajectory();lowered.ball_trajectory();}
 require(lofted.ballz>lowered.ballz,"Back/forward aftertouch changes shot height");
 setup();for(int i=0;i<12;++i)tick(false,true);require(s.live_charge()==12 && e.ball_poss==2,"Power pass charges");tick(false,false);require(e.teams[1].tm_act==15 && e.pass_type==5,"Pass release selects original ground kick");release();require(e.teams[1].tm_pass>0,"Power pass counted at contact");
 setup();for(int i=0;i<9;++i)tick(true,true);tick(false,true);require(e.chip_pass && e.teams[1].tm_act==15,"Release shot from both buttons selects chip");release();require(e.ballzdis>0,"Chip lifts ball");
 setup();for(int i=0;i<9;++i)tick(true,true);tick(true,false);require(e.chip_pass && e.pass_type==-1 && e.teams[1].tm_act==15,"Release pass from both buttons selects high pass");release();
 setup();for(int i=0;i<9;++i)tick(false,true);tick(false,false,0,1);require(e.teams[1].tm_act==15 && e.setp_kick_y>.9f,"Stationary pass can disguise direction at release");release();require(e.ballydis>0,"Disguised pass uses selected direction");
 setup();tick(false,true,1,0);require(e.teams[1].tm_act==15,"Moving pass works without eligible teammate via original directional fallback");release();
 setup();for(int i=0;i<8;++i)tick(true,false);s.clear_play_input();tick(true,false);require(!s.live_charge() && e.teams[1].tm_act==0,"Pause clears charge and blocks held button");tick(false,false);require(e.teams[1].tm_act==0,"Pause does not fire old charge on release");tick(true,false);require(s.live_charge()==1,"New press re-arms after release");

 std::set<int> strikes,controlAnims,shotAnims;int touched=0;
 for(bool trap:{false,true})for(int height:{6,18,30})for(int vz:{0,3,6}){
  setup();e.holder_lose_ball();e.ballx=1040;e.bally=400;e.ballz=float(height);e.ballxdis=-3;e.ballydis=0;e.ballzdis=float(vz);e.ball_inair=1;e.last_touch=3;e.ball_released=0;
  bool contact=false;
  for(int t=0;t<160 && !contact;++t){tick(!trap,trap);
   const auto& p=e.teams[1];if(p.tm_strike>0)strikes.insert(p.tm_strike);
   if(p.tm_act==7)shotAnims.insert(p.tm_anim);
   if(p.tm_act==17)controlAnims.insert(p.tm_anim);
   contact=e.ball_poss==2 || p.tm_shots>0;
  }
  touched+=contact;
 }
 std::cout<<"First-time selections:";for(int i:strikes)std::cout<<' '<<i;
 std::cout<<"; shot animations:";for(int i:shotAnims)std::cout<<' '<<i;
 std::cout<<"; control animations:";for(int i:controlAnims)std::cout<<' '<<i;
 std::cout<<"; contacts="<<touched<<"/18\n";
 require(!shotAnims.empty() && !controlAnims.empty() && touched>0,"Live predictor selects and completes original strike and control actions");


 // Two explicit reachable incoming trajectories prove a kick and standing header
 // are released by the live human-input scheduler, not just selected by prediction.
 for(int rise:{-2,4}){
  setup();e.holder_lose_ball();e.ballx=980;e.bally=400;e.ballz=8;e.ballxdis=-2;e.ballydis=0;e.ballzdis=float(rise);e.ball_inair=1;e.last_touch=3;e.ball_released=0;
  int planned=0;bool fired=false;
  for(int t=0;t<120;++t){tick(true,false);const auto& p=e.teams[1];if(p.tm_strike>0)planned=p.tm_strike;if(p.tm_shots){fired=true;break;}if(e.ball_poss==2)break;}
  require(fired && planned==(rise<0?8:9),"Live first-time foot/header release");
 }
 // Original animation scheduler supports volley contact when the interception
 // decision has selected it; no direct velocity injection replaces its kick.
 setup();e.holder_lose_ball();e.ballx=970;e.bally=400;e.ballz=15;e.ballxdis=-2;e.ballydis=0;e.ballzdis=0;e.ball_inair=1;e.last_touch=3;
 auto& volley=e.teams[1];e.user_controlled=1;volley.tm_strike=5;volley.tm_stime=8;volley.tm_ftime=0;volley.tm_srng=1;volley.int_move=1;e.init_strike_act(&volley);
 for(int t=0;t<100 && !volley.tm_shots;++t)tick(true,false);
 require(volley.tm_shots>0 && e.last_touch==2,"Original volley animation releases through human scheduler");

 setup();auto& winger=e.teams[1];winger.tm_x=1150;winger.tm_y=700;winger.tm_act=1;e.hold_ball(&winger);
 auto& target=e.teams[2];target.guy_on=1;target.tm_x=1130;target.tm_y=400;e.init_stand_act(&target);
 tick(false,true,1,0);require(e.teams[1].tm_act==15 && (e.pass_type==16 || e.pass_type==17),"Original running wing pass selects cross");release();require(e.ballydis<0,"Cross travels into the box");
 // Steal is distinct from the sliding tackle and requires original proximity.
 setup();e.holder_lose_ball();auto& opponent=e.teams[12];opponent.guy_on=1;opponent.tm_x=951;opponent.tm_y=400;e.init_stand_act(&opponent);e.collect_ball(&opponent);e.ballx=951;e.bally=400;e.ballz=2;
 tick(false,true);require(e.teams[1].tm_act==14 || e.ball_poss==2,"Pass button invokes close original steal");
 // The EURO both-button path carries receiver/type only for chip and headers.
 // Exercise real predicted trajectories through contact on both pitch halves.
 auto incoming=[&](bool second,int distance,int height,int rise,bool teammate=true){
  setup();e.holder_lose_ball();e.match_half=second?1:0;
  const int actor=second?12:1,target=actor+1;const float sign=second?-1.f:1.f;
  if(second){e.teams[actor]=e.teams[1];e.teams[actor].tm_player=actor+1;e.teams[1].guy_on=0;}
  auto& p=e.teams[actor];p.tm_x=second?e.pitch_len-950:950;p.tm_xdis=p.mface_x=sign;
  s.selected=actor;
  auto& mate=e.teams[target];mate.guy_on=teammate;mate.tm_x=p.tm_x+sign*float(distance);mate.tm_y=400;e.init_stand_act(&mate);
  e.ballx=p.tm_x+sign*30;e.bally=400;e.ballz=float(height);e.ballxdis=-sign*2;e.ballydis=0;e.ballzdis=float(rise);e.ball_inair=1;e.last_touch=target+1;e.ball_released=0;
 };
 struct IncomingCase{int distance,height,rise,action;};
 for(bool second:{false,true})for(const auto c:{IncomingCase{250,8,-2,10},IncomingCase{100,8,4,9},IncomingCase{100,8,6,7}}){
  incoming(second,c.distance,c.height,c.rise);const int actor=s.selected,target=actor+1;int planned=0,recipient=0;bool collected=false;
  for(int t=0;t<80;++t){tick(true,true);const auto& p=e.teams[actor];if(p.tm_strike>0){planned=p.tm_strike;recipient=p.tm_fpass_to;}
   collected|=e.ball_poss==actor+1;if(p.tm_pass || collected || p.tm_shots)break;
  }
  const auto& p=e.teams[actor];
  require(p.tm_pass==1 && !p.tm_shots && !collected,"Both buttons release a first-time pass without collecting or shooting");
  require(planned==c.action && recipient==target+1 && e.last_touch==actor+1,"Original predicted chip/header preserves receiver through contact");
  require(!e.ball_poss && (second?e.ballxdis<0:e.ballxdis>0),"First-time pass leaves toward teammate on either half");
  std::cout<<"First-time pass: half="<<int(second)<<" action="<<planned<<" receiver="<<recipient<<" contacts="<<p.tm_pass<<" shots="<<p.tm_shots<<"\n";
 }
 // No valid teammate means no invented receiver or fallback first-time shot.
 for(bool second:{false,true}){
  incoming(second,100,8,4,false);const int actor=s.selected;
  for(int t=0;t<12;++t){tick(true,true);const auto& p=e.teams[actor];require(!p.tm_fpass_type && !p.tm_pass && !p.tm_shots,"No eligible receiver must not create a first-time pass or shot");}
 }
 // Release-to-rearm applies to the combination too, before any strike is planned.
 setup();s.clear_play_input();tick(true,true);
 require(!e.teams[1].tm_fpass_type && !s.live_charge(),"Pause blocks held combination");
 tick(false,false);tick(true,true);require(s.live_charge()>0,"Both-button input re-arms after release");
 std::cout<<"Original shots, ground/high/chip/disguised passes, crossing, first-time foot/header/volley contacts, control, steal, aftertouch and pause gating passed.\n";
 return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
