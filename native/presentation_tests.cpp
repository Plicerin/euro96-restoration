#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "original_presentation.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static bool near(float a,float b,float tolerance=0.0001f){return std::abs(a-b)<tolerance;}
static float gap(const OriginalPresentation::Pose& a,const OriginalPresentation::Pose& b){float largest=0;for(int i=1;i<85;++i)largest=std::max(largest,std::abs(a[i]-b[i]));return largest;}
static void synthetic(){
 OriginalPresentation::Pose a{},b{},out{};a[0]=b[0]=28;
 for(int i=1;i<85;++i){a[i]=float(i);b[i]=float(i)*3;}
 legacy::blend_original_pose(out.data(),a.data(),b.data(),.25f,false,false);
 for(int i=1;i<85;++i)require(near(out[i],a[i]*.25f+b[i]*.75f),"Original same-side blend must use every pose component");
 constexpr int mirror[28]={0,1,2,3,4,8,9,10,5,6,7,12,11,17,18,19,20,13,14,15,16,22,21,23,25,24,27,26};
 legacy::blend_original_pose(out.data(),a.data(),b.data(),0,true,false);
 for(int p=0;p<28;++p)for(int c=0;c<3;++c)require(near(out[1+p*3+c],b[1+mirror[p]*3+c]*(c==2?-1.0f:1.0f)),"Mirrored transitions must swap the original limb indices and lateral sign");
 b[71]=-1;legacy::blend_original_pose(out.data(),a.data(),b.data(),0,true,false);
 for(int i=70;i<=72;++i)require(out[i]==a[i],"Original missing-ball sentinel must not reflect an absent captured ball");
 original::match_player old{},current{};old.tm_anim=current.tm_anim=72;old.tm_frm=.95f;current.tm_frm=.05f;
 old.tm_x=10;current.tm_x=30;old.tm_xdis=1;current.tm_ydis=1;
 auto mid=OriginalPresentation::interpolate(old,current,.5f);
 require(near(mid.tm_x,20) && near(mid.tm_frm,1),"Interpolation must move forward across the phase wrap");
 require(near(mid.tm_xdis,std::sqrt(.5f)) && near(mid.tm_ydis,std::sqrt(.5f)),"Facing must follow the shortest rotation");
 current.tm_newanim=1;current.tm_frm=.17f;mid=OriginalPresentation::interpolate(old,current,.5f);
 require(near(mid.tm_frm,.17f),"Restarting a capture must use the new initial phase");
 // Constant synthetic clips expose transition continuity independent of capture motion.
 original::PlayerEngine e;for(int anim:{72,78,96}){e.mcaps[anim]={int(e.player_p.size()),1};auto pose=a;for(int i=1;i<85;++i)pose[i]+=float(anim);e.player_p.push_back(pose);}
 auto& p=e.teams[1];p.guy_on=1;p.tm_anim=72;p.tm_fstep=.1f;p.tm_xdis=1;
 OriginalPresentation view;view.observe(e,1,0);const auto initial=view.player(e,1,1).pose;
 p.tm_anim=78;p.tm_newanim=1;view.observe(e,1,1);
 require(gap(initial,view.player(e,1,0).pose)<.0001f,"Transition must begin at the previous displayed pose");
 auto half=view.player(e,1,.5f).pose;require(gap(initial,half)>0 && gap(initial,half)<1,"Blend must progress between logic ticks");
 auto last=view.player(e,1,1).pose;p.tm_anim=96;view.observe(e,1,2);
 require(gap(last,view.player(e,1,0).pose)<.0001f,"Interrupted transition must start at the current blended pose");
 require(view.player(e,1,.5f).pose==view.player(e,1,.5f).pose,"Repeated draws must not advance blending");
 e.ball_poss=2;view.observe(e,1,3);require(view.ball(.5f).owner==0 && view.ball(1).owner==2,"Possession changes must align with the presentation boundary");
 view.observe(e,2,0);require(gap(view.player(e,1,0).pose,e.player_p.back())<.0001f,"Replay must clear previous transition state even at the same tick");
}
int main(int argc,char** argv){try{
 synthetic();
 require(SDL_Init(SDL_INIT_VIDEO),"SDL video initialization failed");
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);
 auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer!=nullptr,"Software renderer failed");
 // Compare identical wall-clock instants at several rendering rates.
 std::array<OriginalPresentation::Pose,22> reference{};std::array<original::match_player,22> reference_players{};
 for(int fps:{30,60,144}){
  OriginalGraphics g(argv[1]);g.action_view=true;g.sequence->start_continuation();
  for(int frame=0;frame<fps*9;++frame){
   g.advance(1.0/fps);
   if(fps==60){
    g.draw(renderer);require(g.polygon_count()>0,"Blended sequence must produce original polygons");
    if(argc>2 && frame==60*9-1){SDL_RenderPresent(renderer);require(SDL_SavePNG(surface,argv[2]),"Presentation capture failed");}
   }
   for(int i:g.sequence->visible_players){auto sample=g.presentation.player(g.sequence->engine,i,g.presentation_alpha());for(float value:sample.pose)require(std::isfinite(value),"Live blended poses must be finite");}
  }
  for(int i:g.sequence->visible_players){auto sample=g.presentation.player(g.sequence->engine,i,g.presentation_alpha());
   if(fps==30){reference[i]=sample.pose;reference_players[i]=sample.state;}
   else {require(gap(reference[i],sample.pose)<.05f,"Presentation must be independent of display rate");require(near(reference_players[i].tm_x,sample.state.tm_x,.01f),"World interpolation must be independent of display rate");}
  }
  const int paused_tick=g.sequence->ticks;const auto paused_pose=g.presentation.player(g.sequence->engine,11,g.presentation_alpha()).pose;
  g.paused=true;g.advance(.1);require(g.sequence->ticks==paused_tick && paused_pose==g.presentation.player(g.sequence->engine,11,g.presentation_alpha()).pose,"Pause must freeze both motion and blending");g.paused=false;
  while(g.sequence->running){g.advance(1.0/fps);if(fps==60){g.draw(renderer);require(g.polygon_count()>0,"Keeper release and chest reception must render");}}
  require(g.sequence->catch_tick==59 && g.sequence->distribution_tick==171 && g.sequence->receive_tick==208 && g.sequence->ticks==229,"Rendering must not change original gameplay contact timing");
  g.sequence->start_continuation();g.advance(0);require(g.sequence->ticks==0,"Replay must reset without an old partial tick");
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();
 std::cout<<"Original blend/mirror mapping, interruption, replay, phase wrap, ownership, pause and 30/60/144 Hz timing passed.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
