#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "action_constants.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <cstring>
using namespace actua;
static void require(bool b,const char* m){if(!b)throw std::runtime_error(m);}
int main(int argc,char** argv){try{
 require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 {
 OriginalGraphics g(argv[1]);g.action_view=true;auto& s=*g.sequence;auto& e=s.engine;
 for(int side=0;side<2;++side)for(int stage:{0,1,2,3,5}){
  s.practice_restart(stage);
  if(side){using namespace original::action_constants;e.swap_teams();e.match_mode=0;e.penalty_taker=0;e.fkick_taker=13;e.last_touch=2;
   const int modes[]={PEN_KICK_B,DF_KICK_B,IF_KICK_B,CORNER_TL,THROW_IN_B,GOAL_KICK_TR};s.restart_mode=modes[stage];s.restart_ticks=1;g.advance(.05);
  }
  for(int i=0;i<151;++i){s.set_restart_input(0,0,false,false,false);g.advance(.05);}
  require(g.tracker_active(),"Guide must appear for human kicks including minimum power");
  e.setp_power=16;
  for(int hold=0;hold<(stage==5?40:12);++hold){s.set_restart_input(0,0,true,false,false);g.advance(.05);}
  g.update_tracker();auto points=g.tracker_points();
  require(points.size()>=16 && points.size()<=98 && points.size()%2==0,"Bounded paired original tracker points");
  bool elevated=false;
  for(std::size_t i=0;i<points.size();i+=2){require(points[i].ref==0 && points[i+1].ref==1 && points[i].x==points[i+1].x && points[i].y==points[i+1].y && points[i+1].z==0,"Original dot/shadow pairing");elevated|=points[i].z>5;}
  require(elevated,"Height must produce an airborne arc");
  auto probe=std::make_unique<original::PlayerEngine>(e);probe->tracker_sprites.clear();probe->draw_tracker(32000,e.setp_kick_x,e.setp_kick_y,.8f);
  require(probe->tracker_sprites.size()==98,"Oversized minimum length cannot exceed original prediction table");
  std::vector<unsigned char> state(sizeof(e));std::memcpy(state.data(),&e,sizeof(e));const int ticks=s.ticks;
  for(int camera=0;camera<3;++camera){g.change_camera();g.draw(renderer);require(g.tracker_markers==int(points.size()),"All cameras render original tracker sprites");}
  require(!std::memcmp(&e,state.data(),sizeof(e)),"Rendering guide must not mutate any live engine state");
  g.paused=true;const auto frozen=g.tracker_points();g.advance(.1);g.draw(renderer);
  require(s.ticks==ticks && frozen.size()==g.tracker_points().size() && !std::memcmp(frozen.data(),g.tracker_points().data(),frozen.size()*sizeof(frozen[0])),"Paused tracker must not move with redraws");g.paused=false;
  if(argc>2 && stage==1)require(SDL_SavePNG(surface,argv[2]),SDL_GetError());
  s.clear_play_input();g.update_tracker();
  for(const auto& point:g.tracker_points())require(point.z<=e.ball_diam/2,"Pause cancellation refreshes guide without a simulation tick");
  s.set_restart_input(0,0,false,false,false);g.advance(.05);
  s.set_restart_input(0,0,true,false,false);g.advance(.05);s.set_restart_input(0,0,false,false,false);g.advance(.05);g.draw(renderer);
  require(!g.tracker_active() && g.tracker_points().empty(),"Guide disappears when run-up starts");
 }
 s.practice_restart(4);for(int i=0;i<150;++i)g.advance(.05);g.update_tracker();require(g.tracker_points().empty(),"Do not invent a kick guide for throws");
 s.start_match(true);for(int i=0;i<160;++i)g.advance(.05);g.update_tracker();require(g.tracker_points().empty(),"CPU play must not display human aim");
 s.next_format=OriginalActions::MatchFormat::penalties;s.start_match(false);
 for(int t=0;t<500 && !(s.penalty_human_taker() && s.penalty_aiming() && e.ball_poss);++t){s.set_penalty_input(0,false);g.advance(.05);}
 g.update_tracker();require(g.tracker_active() && !g.tracker_points().empty(),"Shootout uses the same original on-pitch guide");
 // Preview visibility must not consume randomness or change any later match motion.
 std::vector<float> reference;
 for(int trial=0;trial<2;++trial){s.practice_restart(1);std::srand(123);std::vector<float> trace;
  for(int t=0;t<260;++t){s.set_restart_input(0,t<180?-1.0f:0,t>=175 && t<185,false,false);g.advance(.05);
   if(trial)for(int paints=0;paints<3;++paints)g.draw(renderer);
   trace.insert(trace.end(),{e.ballx,e.bally,e.ballz,e.ballxdis,e.ballydis,e.ballzdis,float(e.seed),float(e.ball_poss)});
  }
  if(!trial)reference=trace;else require(trace==reference,"Drawing guide must leave subsequent live ball/RNG identical");
 }
 std::cout<<"Original tracker pairing, bounds, cameras, pause, release and render-independent gameplay passed\n";
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();return 0;
}catch(const std::exception& x){std::cerr<<x.what()<<"\n";return 1;}}
