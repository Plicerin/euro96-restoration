#include "original_actions.hpp"
#include "original_graphics.hpp"
#include "action_constants.hpp"
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void near(float a,float b,const char* why){require(std::abs(a-b)<.002f,why);}
static original::PlayerEngine crossing(bool left,float y,float z){
 original::PlayerEngine e;e.ball_poss=0;e.ball_inair=1;e.shot_pending=1;e.seed=64;std::srand(1);
 e.prev_ballx=left?10.0f:1270.0f;e.ballx=left?-10.0f:1290.0f;
 e.prev_bally=e.bally=y;e.prev_ballz=e.ballz=z;e.ballxdis=left?-20.0f:20.0f;e.ballydis=0;e.ballzdis=3;
 return e;
}
static void detection(){
 for(bool left:{false,true}){
  auto e=crossing(left,400,15);e.ball_collision();
  require(e.ball_in_goal && e.goal_events==1 && e.goal_end==(left?-1:1),"Original detector must emit one goal at the crossed end");
  require(!e.shot_pending,"Preview goal boundary must clear pending shot reactions");
  for(int i=0;i<10;++i)e.ball_collision();require(e.goal_events==1,"Net collisions must never duplicate a goal");
  e=crossing(left,400,15);e.prev_bally=350;e.bally=450;e.prev_ballz=40;e.ballz=10;e.ball_collision();
  require(e.goal_events==1,"Goal uses interpolated crossing point, not end-of-tick position");
  for(float y:{340.0f,460.0f}){e=crossing(left,y,15);e.ball_collision();require(!e.goal_events && !e.rebound_ball,"Wide miss must remain a miss");}
  e=crossing(left,400,45);e.ball_collision();require(!e.goal_events && !e.rebound_ball,"Clear overbar miss must remain a miss");
  for(float y:{356.5f,442.5f}){
   e=crossing(left,y,15);const float speed=std::hypot(e.ballxdis,e.ballydis);e.ball_collision();
   require(e.rebound_ball && !e.goal_events && e.commentary_events==1,"Both posts must rebound without awarding a goal");
   near(std::hypot(e.ballxdis,e.ballydis),speed*float(original::action_constants::REBOUND_FACTOR),"Post keeps original 0.8 horizontal speed factor");
   near(e.ballzdis,3,"Post retains vertical velocity");near(e.ballx,left?1.0f:1279.0f,"Post puts ball back inside pitch");
   require(!e.shot_pending && !e.after_touch_on,"Frame impact clears shot state");
  }
  e=crossing(left,400,34.5f);const float speed=std::hypot(e.ballxdis,e.ballzdis);e.ball_collision();
  require(e.rebound_ball && !e.goal_events,"Crossbar must rebound without awarding a goal");
  require(left?e.ballxdis>0:e.ballxdis<0,"Bar redirects ball into the pitch");
  near(std::hypot(e.ballxdis,e.ballzdis),speed*.8f,"Bar retains original damped speed");
  // Test the original open goal-mouth edges; do not replace these with modern sphere rules.
  for(float y:{357.0f,442.0f}){e=crossing(left,y,15);e.ball_collision();require(e.rebound_ball && !e.goal_events,"Exact side edge is frame, not goal");}
  e=crossing(left,400,34);e.ball_collision();require(e.rebound_ball && !e.goal_events,"Exact height edge is crossbar");
  e=crossing(left,400,15);e.prev_ballx=e.ballx;e.ball_collision();require(!e.goal_events && std::isfinite(e.ballx),"Stationary ball behind goal must not invent a crossing");
 }
}
static void nets(){
 for(bool left:{false,true}){
  auto e=crossing(left,400,12);e.ball_collision();
  e.ballx=left?-35.0f:1315.0f;e.ballxdis=left?-10.0f:10.0f;e.ballydis=4;e.ballzdis=6;e.swerve=1;
  e.ball_collision();near(e.ballx,left?-27.0f:1307.0f,"Inside back net clamps to original depth");
  near(e.ballxdis,left?5.0f:-5.0f,"Back net returns ball at half speed");near(e.ballydis,2,"Back net damps lateral speed");near(e.ballzdis,3,"Back net damps vertical speed");require(!e.swerve,"Net contact clears spin");
  for(bool high:{false,true}){
   e.bally=high?350.0f:450.0f;e.ballydis=high?-8.0f:8.0f;e.ballxdis=4;e.ballzdis=6;e.ball_collision();
   near(e.bally,high?358.0f:441.0f,"Inside side net returns ball within goal");near(e.ballydis,high?4.0f:-4.0f,"Inside side net reverses lateral speed");
  }
  e.ballz=40;e.ballzdis=4;e.ball_collision();near(e.ballz,33,"Inside roof clamps below crossbar");near(e.ballzdis,0,"Inside roof kills vertical speed");
  e=crossing(left,400,32);e.prev_ballx=e.ballx=left?-10.0f:1290.0f;e.prev_ballz=38;e.ballzdis=-6;e.ball_collision();
  require(!e.ball_in_goal,"Landing on outside roof must not count as a goal");near(e.ballz,35,"Outside roof sits above crossbar");near(e.ballzdis,0,"Outside roof removes downward speed");
  for(bool high:{false,true}){
   e=crossing(left,high?360.0f:439.0f,12);e.prev_ballx=left?4.0f:1276.0f;e.ballx=left?-4.0f:1284.0f;
   e.prev_bally=high?348.0f:451.0f;e.ballydis=high?12.0f:-12.0f;e.ball_collision();
   require(!e.goal_events,"Side-net entry from outside must not count as a goal");near(e.bally,high?357.0f:442.0f,"Outer side net contact clamps to side");near(e.ballydis,high?-6.0f:6.0f,"Outer side net bounces outward");
  }
 }
}
int main(int argc,char** argv){try{
 require(argc>1,"Data required");detection();nets();
 require(SDL_Init(SDL_INIT_VIDEO),"SDL init failed");
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer!=nullptr,"Renderer failed");
 {
 OriginalGraphics g(argv[1]);g.action_view=true;
 for(bool left:{false,true})for(int stage=0;stage<5;++stage){
  auto& s=*g.sequence;s.start_goal_review(stage,left);bool contact=false;
  g.advance(0);g.paused=true;g.advance(.1);require(s.review_tick==0,"Pause freezes review");g.paused=false;
  for(int tick=1;tick<80;++tick){
   g.advance(.05);require(s.review_stage==stage,"Review must leave time to inspect each case");
   auto& e=s.engine;
   if(stage<2)contact|=e.rebound_ball!=0;
   if(stage==2)contact|=e.bally==357 && e.ballydis<0;
   if(stage==3)contact|=e.ballz==35 && e.ballzdis==0;
   if(stage==4)contact|=e.ball_in_goal && (left?e.ballxdis>0:e.ballxdis<0);
   g.draw(renderer);SDL_RenderPresent(renderer);
   require(g.scene_counts.goal_parts==8 && g.scene_counts.rotating_balls==1,"Goal review must render original goals and ball");
   if(argc>2 && contact && tick==38){
    const auto path=std::filesystem::path(argv[2])/(std::string(left?"left-":"right-")+std::to_string(stage)+".png");
    require(SDL_SavePNG(surface,path.string().c_str()),"Could not save goal review capture");
   }
  }
  if(!contact)throw std::runtime_error(std::string("Review failed to demonstrate ")+s.goal_review_name(stage)+(left?" left":" right"));
  require(s.engine.goal_events==(stage==4?1:0),"Only the scoring case may register a goal");
  s.step();require(stage==4?!s.running:s.review_stage==stage+1,"Review advances once, then stops after fifth case");
 }
 // Complete both automatic batches, including presentation resets between launches.
 for(bool left:{false,true}){
  g.sequence->start_goal_review(0,left);
  for(int i=0;i<420 && g.sequence->running;++i){g.advance(.05);g.draw(renderer);}
  require(!g.sequence->running && g.sequence->review_stage==4,"One replay must complete all five checks");
 }
 // Integration: the original attacking header continues through goal into the net.
 auto& s=*g.sequence;s.start_attack(400,400);bool back_net=false;
 while(s.running){s.step();back_net|=s.engine.ball_in_goal && s.engine.ballxdis<0;}
 require(s.goal_mouth && s.engine.goal_events==1 && back_net && s.ticks-s.goal_tick==60,"Attack must use original detection and continue net physics after scoring");
 s.start_continuation();while(s.running)s.step();
 require(s.catch_tick==59 && s.distribution_tick==171 && s.receive_tick==208 && s.ticks==229,"Goal integration must preserve approved keeper chain timing");
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();
 std::cout<<"Original goal crossing, both posts, crossbar, inner/outer nets, five-case rendered review and attack integration passed.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
