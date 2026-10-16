#include "original_graphics.hpp"
#include "original_actions.hpp"
#include <iostream>
#include <cmath>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static float distance(const OriginalPresentation::Ball& a,const OriginalPresentation::Ball& b){
 return std::sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y)+(a.z-b.z)*(a.z-b.z));
}
static void sentinels(){
 original::PlayerEngine e;
 for(int anim:{72,78,81}){
  e.mcaps[anim]={int(e.player_p.size()),2};
  OriginalPresentation::Pose pose{};pose[0]=28;pose[70]=4;pose[71]=anim==78?-10000.0f:20.0f;
  e.player_p.push_back(pose);pose[70]=8;pose[71]=anim==78?-10000.0f:24.0f;e.player_p.push_back(pose);
 }
 auto& p=e.teams[12];p.guy_on=1;p.tm_xdis=1;p.tm_anim=78;p.tm_fstep=.05f;e.ball_poss=13;e.ballz=2;
 OriginalPresentation view;view.observe(e,1,0);
 p.tm_anim=81;p.tm_newanim=1;p.tm_frm=.25f;view.observe(e,1,1);
 for(float alpha:{0.0f,.25f,.5f,1.0f}){
  const auto v=view.player(e,12,alpha);
  require(std::abs(v.pose[71]-22)<.001f,"Absent source ball must never be blended into a real capture");
  require(std::abs(v.pose[70]-6)<.001f,"Capture coordinates must interpolate between original frames");
 }
 const auto last=view.displayed_ball(e,1);
 p.tm_anim=78;p.tm_frm=0;view.observe(e,1,2);
 require(view.player(e,12,.5f).pose[71]==-10000,"Absent target must remain a sentinel");
 require(distance(last,view.displayed_ball(e,0))<.001f,"Captured-to-physical handoff must start at the displayed ball");
 require(std::abs(view.displayed_ball(e,.5f).z-12)<.001f && view.displayed_ball(e,1).z==2,"Handoff must reach the physical ball continuously");
}
int main(int argc,char** argv){try{
 require(argc>1,"Data path required");sentinels();const bool trace=argc>2;
 for(auto target:{std::pair{1060.0f,260.0f},std::pair{1050.0f,280.0f},std::pair{1040.0f,270.0f}}){
  for(int fps:{30,60,144}){
   OriginalGraphics g(argv[1]);g.action_view=true;g.sequence->start_continuation(target.first,target.second);g.advance(0);
   auto& s=*g.sequence;auto& e=s.engine;
   auto last=g.presentation.displayed_ball(e,0);float largest=0,control_largest=0;int chest_samples=0,frames=0;
   while(s.running && frames<fps*30){
    g.advance(1.0/fps);const float alpha=s.running?g.presentation_alpha():1;
    const auto b=g.presentation.displayed_ball(e,alpha);
    require(std::isfinite(b.x)&&std::isfinite(b.y)&&std::isfinite(b.z),"Displayed ball must remain finite");
    const float delta=distance(last,b);
    if(s.ticks>=160){largest=std::max(largest,delta);if(s.receptions)control_largest=std::max(control_largest,delta);}
    if(b.owner==13 && e.ball_limbo_on && e.ball_limbo_p==13){
     const auto v=g.presentation.player(e,12,alpha);
     require(v.pose[71]>0 && b.z>1,"Chest-controlled ball must stay above ground while captured");++chest_samples;
    }
    if(trace && fps==60 && target.first==1060 && s.ticks>=160)
     std::cout<<"tick "<<s.ticks<<" a "<<alpha<<" ball "<<b.x<<","<<b.y<<","<<b.z<<" delta "<<delta<<'\n';
    last=b;++frames;
   }
   require(!s.running && s.receptions==1 && e.ball_poss==13 && chest_samples>0,"Each outlet must complete its original chest control");
   // The reported bug jumped 19.32 units in one 60 Hz frame. Bound by
   // elapsed time so the same contract covers different display rates.
   require(largest*fps<420,"Keeper flight/reception must not teleport between display frames");
   require(control_largest*fps<330,"Controlled ball must not fall to ground then jump back to the chest");
   const auto stopped=g.presentation.displayed_ball(e,1);g.advance(.1);
   require(distance(stopped,g.presentation.displayed_ball(e,1))==0,"Completed reception must remain stable");
   std::cout<<"Outlet "<<target.first<<","<<target.second<<" at "<<fps<<" Hz: max step "<<largest<<", reception "<<control_largest<<", end "<<s.ticks<<'\n';
  }
  OriginalGraphics g(argv[1]);g.sequence->start_continuation(target.first,target.second);
  auto& s=*g.sequence;g.presentation.observe(s.engine,s.generation,s.ticks);
  while(s.running){
   const auto before=g.presentation.displayed_ball(s.engine,1);s.step();g.presentation.observe(s.engine,s.generation,s.ticks);
   const auto after=g.presentation.displayed_ball(s.engine,0);
   if(s.ticks>=160)require(distance(before,after)<.01f,"Ball handoffs must be continuous at the logic boundary");
  }
 }
 std::cout<<"Ball sentinel, subframe capture, visual handoff and three live outlets passed at 30/60/144 Hz.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
