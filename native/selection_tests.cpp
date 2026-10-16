#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "action_constants.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace actua;
using namespace actua::original::action_constants;
static void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 require(argc==2,"Expected original data root");OriginalGraphics graphics(argv[1]);auto& s=*graphics.sequence;auto& e=s.engine;
 s.start_match(false);
 std::cout<<"Opening kickoff selected="<<s.selected+1<<" taker="<<e.setp_taker<<" receiver="<<e.centre_guy_2<<" distance="<<e.teams[s.selected].tm_dist<<'\n';
 require(s.selected==e.setp_taker-1 || s.selected==e.centre_guy_2-1,"Opening kickoff must select one of the two centre players before the first tick");
 int kickoffs=0;
 for(bool cpu:{false,true})for(bool local:{false,true})for(int half=0;half<4;++half)for(int mode:{CENTRE_A,CENTRE_B}){
  s.next_head_to_head=local;s.start_match(cpu);e.match_half=half;
  e.match_mode=0;s.restart_mode=mode;s.restart_ticks=1;s.step();
  const int taker=e.setp_taker,receiver=e.centre_guy_2;
  for(int side=0;!cpu && side<(local?2:1);++side){auto scope=s.input_scope(side);
   require(e.teams[s.selected].guy_on && s.is_controlled_team(s.selected),"Kickoff selects an active player on each human's own team after changing ends");
   if(s.is_controlled_team(taker-1))require(s.selected==taker-1,"Kickoff taker is selected before the first tick");
   else for(int i=s.control_slot()+1;i<s.control_slot()+11;++i)if(e.teams[i].guy_on)
    require(e.teams[s.selected].tm_dist<=e.teams[i].tm_dist,"Defending kickoff selects closest eligible outfield player");
  }
  for(int tick=0;tick<200 && e.match_mode;++tick){
   for(const auto& p:e.teams)require(!p.control,"Prepared kickoff retains original automatic actor control");
   s.step();
  }
  require(!e.match_mode && !s.restart_mode && e.teams[taker-1].tm_act==KICK_ACT && e.receiver==receiver,"Original centre pass still releases automatically to second centre player");
  s.step();
  for(int side=0;side<(s.head_to_head?2:1);++side){int controlled=0;
   for(int i=0;i<22;++i)if(e.teams[i].control==side+1){++controlled;require(i==s.selected_for(side),"Live control matches visible selection after centre release");}
   require(controlled==(cpu?0:1),"Kickoff hands off exactly one player per human; spectators control neither side");
  }
  ++kickoffs;
 }
 s.next_head_to_head=false;
 std::cout<<kickoffs<<" kickoff cases passed: both sides, all four periods, single/local/spectator modes, immediate selection and automatic pass handoff.\n";
 for(int half:{0,1}){
  const int base=half?11:0,opponent=half?0:11;
  auto fixture=[&](){
   s.start_team_play();s.full_match=true;e.match_half=half;e.holder_lose_ball();
   e.receiver_a=e.receiver_b=e.interceptor_a=e.interceptor_b=e.near_path_a=e.near_path_b=0;
   e.ball_limbo_on=0;e.ball_poss=0;s.selected=base+8;e.sel_circle.fill(0);
   for(auto& p:e.teams){p.control=0;p.tm_dist=900;p.contact=0;p.tm_limbo=0;e.reset_ideas(&p);e.init_stand_act(&p);}
   e.teams[s.selected].control=1;e.teams[base].tm_dist=1;e.teams[base+2].tm_dist=60;
  };
  auto select=[&](){++e.selection_events;s.update_auto_selection();int users=0;for(int i=0;i<22;++i){users+=e.teams[i].control!=0;if(e.teams[i].control)require(s.is_blue(i),"Never control an opponent after changing ends");}require(users==1,"Exactly one controlled player");};
  fixture();select();require(s.selected==base+2 && !e.ball_poss,"Loose-ball selection excludes keeper and never moves possession");
  e.teams[base+3].tm_dist=20;e.teams[base+3].tm_act=FALL_ACT;e.teams[base+4].tm_dist=10;e.teams[base+4].guy_on=0;select();
  require(s.selected==base+2,"Fallen and off-field candidates excluded");

  fixture();e.ball_poss=opponent+2;select();require(s.selected==base+2,"Defensive automatic selection");
  e.teams[base+3].tm_dist=5;e.sel_circle[s.selected]=1;select();require(s.selected==base+2,"Selection circle prevents switching between nearby defenders");
  e.sel_circle[s.selected]=0;select();require(s.selected==base+3,"Defender switches when outside selection circle");

  fixture();e.sel_circle[s.selected]=1;
  (half?e.receiver_b:e.receiver_a)=base+5;select();require(s.selected==base+4 && !e.ball_poss,"Intended receiver overrides selection circle before reception");
  (half?e.receiver_b:e.receiver_a)=0;(half?e.interceptor_b:e.interceptor_a)=base+6;e.sel_circle[s.selected]=0;select();require(s.selected==base+5,"Interceptor has priority over raw distance");
  (half?e.interceptor_b:e.interceptor_a)=0;(half?e.near_path_b:e.near_path_a)=base+7;select();require(s.selected==base+6,"Predicted path candidate has priority over raw distance");
  e.ball_poss=base+3;select();require(s.selected==base+2,"Teammate possession transfers control");

  fixture();s.selected=base;e.ball_poss=base+1;e.ball_in_hands=1;
  for(auto& p:e.teams)p.control=0;e.teams[base].control=1;select();require(s.selected==base,"Keeper keeps distribution control");
  e.ball_poss=0;e.teams[base].contact=2;select();require(s.selected==base,"Keeper contact animation keeps control");
  e.teams[base].contact=0;e.ball_in_hands=0;select();require(s.selected==base+2,"Keeper release returns control to outfield");

  fixture();s.switch_play_player();const int manual=s.selected;
  (half?e.receiver_b:e.receiver_a)=base+10;
  for(int tick=0;tick<12;++tick){select();require(s.selected==manual,"Manual switch wins for a complete selection cadence");}
  select();require(s.selected==base+9,"Automatic receiver selection resumes after manual priority");
  const int held=s.selected;s.interval_ticks=100;select();require(s.selected==held,"Interval cannot reselect");s.interval_ticks=0;
  s.restart_mode=1;select();require(s.selected==held,"Restart retains designated taker");s.restart_mode=0;
  s.shootout=true;select();require(s.selected==held,"Shootout retains dedicated control");s.shootout=false;
  s.set_cpu_control(true);s.update_auto_selection();for(const auto& p:e.teams)require(!p.control,"Spectator mode has no human selection");
 }
 // Real scheduler: displaced controlled player must hand off while the ball
 // remains loose, and held movement must reach the newly selected player.
 s.start_team_play();e.holder_lose_ball();e.ballx=600;e.bally=400;e.ballz=80;e.ballxdis=e.ballydis=e.ballzdis=0;e.ball_inair=1;
 for(int i=0;i<22;++i){auto& p=e.teams[i];p.tm_x=i<11?100:1100;p.tm_y=100;e.reset_ideas(&p);e.init_stand_act(&p);}
 e.teams[2].tm_x=600;e.teams[2].tm_y=430;e.teams[2].tm_xdis=e.teams[2].mface_x=1;e.teams[2].tm_ydis=e.teams[2].mface_y=0;
 const int previous=s.selected;bool switched=false;float selected_x=0;
 s.set_play_input(1,0);
 for(int tick=0;tick<16;++tick){s.step();if(s.selected!=previous && !switched){switched=true;selected_x=e.teams[s.selected].tm_x;require(!e.ball_poss,"Live loose-ball switch occurs before possession");}}
 require(switched && s.selected==2 && e.teams[2].tm_x>selected_x,"Held direction moves the automatically selected player");
 require(std::isfinite(e.ballx) && std::isfinite(e.bally) && std::isfinite(e.ballz),"Finite ball after selection");
 {
  for(int moving=0;moving<2;++moving){
   s.start_match(false);int changes=0,defensive=0,loose=0,far=0,streak=0,longest=0;
   for(int tick=0;tick<12000 && s.running;++tick){
    const int old=s.selected;const bool shot=tick%50<10,pass=tick%60>=20 && tick%60<24;
    s.set_play_input(moving?(s.blue_slot()?-1.f:1.f):0,0);s.set_play_buttons(shot,pass);
    s.set_penalty_input(0,shot);s.set_restart_input(0,0,shot,pass,false);
    if(s.human_keeper_hold())s.request_play_kick(OriginalActions::PlayKick::shot);
    s.step();auto& p=e.teams[s.selected];const bool defending=e.ball_poss && !s.is_blue(e.ball_poss-1);
    if(s.selected!=old){++changes;defensive+=defending;loose+=!e.ball_poss;}
    if(!s.restart_mode && !s.restart_ticks && !e.match_mode && !s.interval_ticks && p.tm_dist>250 && defending){
     ++far;if(s.selected==old)++streak;else streak=0;longest=std::max(longest,streak);
     if(streak==60)std::cout<<"STALL tick="<<tick<<" sel="<<s.selected<<" dist="<<p.tm_dist<<" act="<<int(p.tm_act)<<" contact="<<p.contact<<" limbo="<<p.tm_limbo<<" balllimbo="<<int(e.ball_limbo_on)<<" receiver="<<e.receiver_a<<","<<e.receiver_b<<" near="<<e.near_path_a<<","<<e.near_path_b<<" circle="<<int(e.sel_circle[s.selected])<<"\n";
    }else streak=0;
   }
   require(s.full_time && changes>20 && defensive>5 && loose>5,"Automatic selection must operate through real kickoff, passes and defensive turnovers without manual switching");
   std::cout<<"Full-match selection moving="<<moving<<" ticks="<<s.ticks<<" changes="<<changes<<" defensive="<<defensive<<" loose="<<loose<<" far="<<far<<" longest="<<longest<<"\n";
  }
 }
 std::cout<<"Five selection groups passed on both pitch halves: loose ball, defense/circle, receivers/interception, goalkeeper handoff, manual priority. Live scheduler and held movement passed.\n";
 return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
