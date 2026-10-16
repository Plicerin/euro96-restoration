#include "original_graphics.hpp"
#include "original_actions.hpp"
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 require(argc>1,"Data required");auto graphics=std::make_unique<OriginalGraphics>(argv[1]);auto& g=*graphics;g.action_view=true;auto& s=*g.sequence;auto& e=s.engine;
 s.next_format=OriginalActions::MatchFormat::penalties;
 SDL_Surface* surface=nullptr;SDL_Renderer* renderer=nullptr;
 if(argc>2){require(SDL_Init(SDL_INIT_VIDEO),"SDL init");surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);renderer=SDL_CreateSoftwareRenderer(surface);require(renderer!=nullptr,"Capture renderer");}
 auto capture=[&](const std::string& name){if(!renderer)return;g.draw(renderer);SDL_RenderPresent(renderer);const auto path=std::filesystem::path(argv[2])/(name+".png");require(SDL_SavePNG(surface,path.string().c_str()),"Save outcome capture");};
 auto start=[&]{s.start_match(true);while(s.penalty_wait)g.advance(.05);};
 auto launch=[&](float y){
  e.match_mode=e.dead_ball_cnt=e.game_action=0;s.restart_mode=0;e.reset_ball();e.ball_limbo_on=e.ball_in_hands=0;
  const bool left=e.penalty_taker>11;
  e.ballx=left?2.0f:float(e.pitch_len)-2;e.bally=y;e.ballz=12;e.ballxdis=left?-12.0f:12.0f;e.ballydis=0;e.ballzdis=2;
  e.ball_inair=1;e.ball_still=0;e.last_touch=e.penalty_taker;e.shot_pending=e.real_shot=1;s.penalty_released=true;s.penalty_age=0;
  // This fixture changes the launch only; original detection, nets and result scheduling run normally.
  ++s.generation;g.advance(0);
 };
 start();
 for(int attempt=0;attempt<2;++attempt){
  const bool left=e.penalty_taker>11;const int attempts=e.penalties,events=e.goal_events;
  launch(400);const auto launch_generation=s.generation;g.advance(.05);
  require(s.penalty_wait==60 && s.penalty_scores[0]+s.penalty_scores[1]==attempt+1,"Crossing records one penalty goal");
  require(e.ball_in_goal && e.ballz>2 && std::abs(e.ballxdis)>1,"Scored penalty must retain ball height, velocity and net state");
  require(s.generation==launch_generation,"Result must preserve presentation continuity");
  const float cross_x=e.ballx;bool bounced=false;
  capture(std::string(left?"left":"right")+"-crossing");
  g.paused=true;g.advance(.1);require(s.penalty_wait==60 && e.ballx==cross_x,"Pause still freezes outcome motion");g.paused=false;
  g.advance(.05);require(e.ballx!=cross_x,"Ball must keep moving after goal-line crossing");
  const auto a=g.presentation.displayed_ball(e,.25f),b=g.presentation.displayed_ball(e,.75f);
  require(std::abs(a.x-b.x)>.1f,"Displayed ball must interpolate through the result interval");
  for(int i=1;i<59;++i){
   g.advance(.05);bounced|=left?e.ballxdis>0:e.ballxdis<0;
   if(i==6)capture(std::string(left?"left":"right")+"-net");
   require(e.goal_events==events+1 && s.penalty_scores[0]+s.penalty_scores[1]==attempt+1,"Net follow-through must not score twice");
   require(e.penalties==attempts && s.penalty_wait>0,"Next kick waits for result interval");
   require(std::isfinite(e.ballx)&&std::isfinite(e.ballz),"Finite net motion");
  }
  require(bounced,"Penalty must reach original back net and rebound");
  g.advance(.05);require(e.penalties==attempts+1 && !s.penalty_wait && !e.ball_in_goal,"Next kick resets ball only after follow-through");
  std::cout<<(left?"Left":"Right")<<" goal: motion, net rebound, render interpolation, pause and single scoring passed\n";
 }
 start();launch(320);g.advance(.05);require(s.penalty_wait==60 && !e.ball_in_goal,"Wide penalty is a miss");const float wide_x=e.ballx;
 for(int i=0;i<10;++i)g.advance(.05);
 require(std::abs(e.ballx-wide_x)>20 && e.goal_events==0 && s.penalty_scores[0]+s.penalty_scores[1]==0,"Wide miss keeps flying without changing result");
 // Exercise real catches and dives during complete CPU shootouts, not fabricated catch poses.
 int catches=0,recoveries=0;
 for(int seed=1;seed<=8;++seed){s.start_match(true);std::srand(seed);
  for(int tick=0;tick<18000 && s.running;++tick){
   const int wait=s.penalty_wait;const int owner=e.ball_poss;
   const int keeper=e.penalty_taker>11?0:11;const float phase=e.teams[keeper].tm_frm;
   g.advance(.05);
   if(!wait && s.penalty_wait && e.ball_in_hands){++catches;require(e.ball_poss==keeper+1,"Caught ball remains attached during result");}
   if(wait>1 && owner){require(e.ball_poss==owner && e.ball_in_hands,"Result must not reset a keeper-held ball");}
   if(wait>1 && phase!=e.teams[keeper].tm_frm)++recoveries;
  }
  require(s.full_time,"Shootout must still finish after result follow-through");
 }
 require(catches>0 && recoveries>0,"Result interval must include real catches and continuing animations");
 std::cout<<"Wide misses and eight complete shootouts passed; "<<catches<<" retained catches, "<<recoveries<<" result animation ticks\n";
 graphics.reset();if(renderer)SDL_DestroyRenderer(renderer);if(surface)SDL_DestroySurface(surface);if(argc>2)SDL_Quit();
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
