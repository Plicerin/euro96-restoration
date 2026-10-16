#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "action_constants.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <cstring>
#include <string_view>
using namespace actua;
static void require(bool b,const char* m){if(!b)throw std::runtime_error(m);}
int main(int argc,char** argv){try{
 require(argc>1,"Data path");require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
 auto* surface=SDL_CreateSurface(960,700,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 {
 OriginalGraphics g(argv[1]);g.action_view=true;auto& s=*g.sequence;auto& e=s.engine;
 // Deterministic original disciplinary thresholds: yellow, second booking, direct red, keeper exclusion.
 s.start_match(true);e.seed=20;e.send_off_plr(2,30);require(e.teams[1].tm_book==1 && e.teams[1].guy_on,"Original yellow threshold");
 e.send_off_plr(2,30);require(e.teams[1].tm_book==6 && !e.teams[1].guy_on && e.players_off_a==1,"Second booking removes player");
 e.send_off_plr(13,100);require(e.teams[12].tm_book==5 && !e.teams[12].guy_on && e.players_off_b==1,"Direct red removes player");
 e.send_off_plr(1,100);require(e.teams[0].guy_on && !e.teams[0].tm_book,"Preserve original keeper booking exclusion");
 require(!s.queue_substitution(false,1,12),"Cannot replace dismissed player");
 s.restart_ticks=1;e.match_mode=0;g.advance(.05);g.draw(renderer);require(!e.teams[1].guy_on && g.scene_counts.home_players==10 && g.scene_counts.away_players==10,"Restart/render must not restore sent-off players");
 require(g.scene_counts.officials==3,"Render referee and both linesmen");
 // Punishment must invoke discipline through the live foul path too.
 s.start_match(true);e.seed=20;e.teams[1].tm_dist=100;e.man_down=1;e.incident_x=640;e.incident_y=400;e.punish_foul(2,true);
 require(e.card_events==1 && e.rule_events==1 && !e.teams[1].guy_on,"Foul pipeline must issue a card and award a restart");
 // Native swap follows original ratings/shirt and resets incoming player's own state.
 s.start_match(true);auto& p=e.teams[1];const int original_rate=p.tm_rate;p.tm_time=80;p.tm_book=1;p.tm_pass=4;
 e.apply_stamina(&p,original_rate);require(p.tm_rate<original_rate,"Original fatigue lowers pace");
 require(s.queue_substitution(false,1,12),"Queue original bench player");require(p.shirt==2,"Do not substitute during live play");
 s.restart_ticks=1;e.match_mode=0;g.advance(.05);require(p.shirt==12 && !p.tm_time && !p.tm_book && p.tm_rate==original_teams[s.home_team].players[11].pace+28,"Incoming player's identity, fresh pace, time and bookings");
 require(s.sub_passes[0]==4 && s.substitutions[0]==1 && !s.queue_substitution(false,2,12),"Retain team statistics and prohibit reentry");
 require(!s.queue_substitution(false,2,16),"Only original four match substitutes supported");
 for(int shirt=13;shirt<=14;++shirt){require(s.queue_substitution(false,shirt-10,shirt),"Queue remaining permitted changes");s.apply_substitutions();}
 require(!s.queue_substitution(false,6,15),"Three-change cap");
 e.swap_teams();require(e.teams[12].shirt==12 && s.player_label(12).find("SCT")==std::string::npos,"Substitute identity survives end swap");
 require(s.queue_substitution(true,2,15),"Queue away substitute after end swap");s.apply_substitutions();require(e.teams[2].shirt==15,"Away substitution uses swapped physical slot");
 // Live replay must freeze every gameplay field and random-independent state.
 s.start_match(true);for(int i=0;i<180;++i)g.advance(.05);
 const auto players=e.teams;const int tick=s.ticks;const float ball=e.ballx,sec=e.match_time.sec;
 require(g.start_replay(),"Recorded replay available");
 for(int i=0;i<30;++i){g.advance(.05);g.draw(renderer);require(s.ticks==tick && e.ballx==ball && e.match_time.sec==sec && !std::memcmp(players.data(),e.teams.data(),sizeof(players)),"Replay must never advance or mutate the live simulation");}
 g.finish_replay();g.advance(.05);require(s.ticks==tick+1,"Resume exactly one live tick after replay");
 // Both goal ends and own goals exercise original celebration clips and camera.
 for(int side=0;side<2;++side)for(bool own:{false,true}){
  s.start_match(true);for(int i=0;i<150;++i)g.advance(.05);
  e.reset_ball();s.restart_ticks=s.restart_mode=e.match_mode=e.dead_ball_cnt=0;
  e.last_touch=e.pre_kp_touch=(side^own)?13:2;e.goal_scorer=e.last_touch;
  e.ballx=side?-1.0f:1281.0f;e.bally=400;e.ballz=2;e.ballxdis=e.ballydis=e.ballzdis=0;
  if(side)e.go_left_goal();else e.go_right_goal();
  bool celebration=false,replay=false,close_camera=false,celeb_action=false;
  for(int i=0;i<520;++i){g.advance(.05);g.draw(renderer);SDL_RenderPresent(renderer);
   if(own)celeb_action|=e.teams[e.goal_scorer-1].tm_anim==MC_SHAMEW;
   else for(const auto& p:e.teams)celeb_action|=p.tm_act==original::action_constants::CELEB_ACT || p.int_move==16;
   celebration|=e.just_scored>0;replay|=g.replaying;close_camera|=e.just_scored>0 && e.just_scored<50 && g.camera_pose().tz>10;
   for(int n:s.visible_players)if(e.teams[n].guy_on){auto sample=g.presentation.player(e,n,.5f);for(float v:sample.pose)require(std::isfinite(v),"Finite celebration capture");}
   if(argc>2 && i==100 && !side && !own)require(SDL_SavePNG(surface,(std::filesystem::path(argv[2])/"celebration.png").string().c_str()),"Capture celebration");
  }
  std::cout<<"goal fixture "<<side<<" own "<<own<<" celebration "<<celebration<<" replay "<<replay<<" camera "<<close_camera<<" wait "<<e.just_scored<<" restart "<<s.restart_mode<<" anim "<<e.teams[e.goal_scorer-1].tm_anim<<"\n";
  require(celeb_action && celebration && replay && close_camera && !e.just_scored && s.restarts>1,"Goal celebration, original scorer camera, replay and kickoff must complete");
  require(s.blue_goals+s.red_goals==1,"Replay cannot count a goal twice");
 }
 // Completed games cover persistent cards, stamina, CPU substitutions, goals and all render views.
 int cards=0,subs=0,goals=0;
 for(int seed=1;seed<=6;++seed){s.start_match(true);std::srand(seed+20);
  for(int frame=0;frame<25000 && s.running;++frame){g.advance(.05);
   if(frame%80==0){g.change_camera();g.draw(renderer);}
   for(auto& p:e.teams)require(std::isfinite(p.tm_x) && std::isfinite(p.tm_y),"Finite match players");
  }
  require(s.full_time,"Full match must finish with depth features enabled");
  for(int n=0;n<22;++n)if(n%11){const auto& p=e.teams[n];const auto& stats=original_teams[s.is_blue(n)?s.home_team:s.away_team].players[p.shirt-1];require(std::string_view(stats.position)!="GK","CPU must retain the reserve keeper instead of using him as an outfielder");}
  cards+=e.card_events;subs+=s.substitutions[0]+s.substitutions[1];goals+=s.blue_goals+s.red_goals;
  std::cout<<"seed "<<seed<<" score "<<s.blue_goals<<":"<<s.red_goals<<" cards "<<e.card_events<<" subs "<<s.substitutions[0]<<"/"<<s.substitutions[1]<<"\n";
 }
 require(cards && subs,"CPU matches exercise cards and substitutions");
 std::cout<<"Depth verified; CPU totals cards="<<cards<<" substitutions="<<subs<<" goals="<<goals<<"\n";
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<"\n";SDL_Quit();return 1;}}


