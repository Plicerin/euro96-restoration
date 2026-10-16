#include "original_frontend.hpp"
#include "action_constants.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 require(argc==4,"Expected source, media and captures");require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
 const std::filesystem::path folder=argv[3];std::filesystem::create_directories(folder);
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* r=SDL_CreateSoftwareRenderer(surface);require(r,SDL_GetError());
 {
  OriginalGraphics g(argv[1]);OriginalFrontend front(argv[1],argv[2]);auto& s=*g.sequence;
  const auto key=[&](SDL_Keycode k){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;front.event(e,g);};
  const auto frame=[&](const std::string& name){
   front.draw(r,g,false,false);SDL_RenderPresent(r);auto* pixels=SDL_RenderReadPixels(r,nullptr);require(pixels,SDL_GetError());
   auto* rgba=SDL_ConvertSurface(pixels,SDL_PIXELFORMAT_RGBA32);require(rgba,SDL_GetError());std::uint64_t energy=0;
   for(int y=0;y<rgba->h;++y){auto* row=static_cast<Uint8*>(rgba->pixels)+y*rgba->pitch;for(int x=0;x<rgba->w;++x)for(int c=0;c<3;++c)energy+=row[x*4+c];}
   if(!name.empty())require(SDL_SavePNG(rgba,(folder/(name+".png")).string().c_str()),SDL_GetError());
   SDL_DestroySurface(rgba);SDL_DestroySurface(pixels);return energy;
  };
  const auto fade=[&](const std::string& name){
   front.menu_fades=false;const auto full=frame("");require(full>100000,"Menu contains artwork and text");front.menu_fades=true;
   const auto players=s.engine.teams;const auto seed=s.engine.seed;const auto ticks=s.ticks;
   require(frame("")==0,"New screen starts at black");
   front.observe(g,.1);front.observe(g,.1);const auto halfway=frame(name+"-fade");require(halfway>full/3 && halfway<full*2/3,"Composed artwork and text fade together");
   front.observe(g,0);require(frame("")==halfway,"Unfocused menu fade holds its exact frame");
   for(int i=0;i<3;++i)front.observe(g,.1);
   const auto settled=frame(name);front.menu_fades=false;require(frame("")==settled,"Fade finishes at exact unfaded menu pixels at the same animation time");front.menu_fades=true;
   require(s.engine.seed==seed && s.ticks==ticks && !std::memcmp(players.data(),s.engine.teams.data(),sizeof(players)),"Menu transition cannot change live gameplay");
  };
  front.open(g);fade("title");key(SDLK_RETURN);fade("selection");key(SDLK_Y);fade("lineup");
  key(SDLK_RETURN);s.interval_ticks=100;s.interval_minute=45;front.observe(g,.05);fade("interval");
  key(SDLK_RETURN);s.step();s.finish_match();front.observe(g,.05);fade("result");
  key(SDLK_RETURN);frame("");front.observe(g,.1);key(SDLK_RIGHT);const auto input=frame("");front.menu_fades=false;require(frame("")==input,"Navigation immediately restores legibility without changing input dispatch");
  std::cout<<"Five frontend screens: original fade-up progression, exact final pixels, focus hold, navigation and immutable live state passed.\n";
  s.start_match(true);auto& e=s.engine;
  for(auto& ref:e.refs){e.init_ref_jog(&ref);ref.act=0;}
  e.refs[2].act=1;e.refs[2].goto_x=e.refs[2].x+100;e.refs[2].goto_y=e.refs[2].y;
  const auto live=e.refs;const auto seed=e.seed;OriginalExit exit;exit.start(e,595,1425);exit.advance(.05);const auto& refs=exit.engine->refs;
  require(refs[0].anim==MC_STAND && refs[0].x==live[0].x && refs[0].y==live[0].y,"Original SWAP_ENDS referee remains standing");
  require(refs[1].anim==MC_STAND && refs[1].x==live[1].x,"Idle assistant stands during original nonzero match mode");
  require(refs[2].x>live[2].x,"Assistant already walking to a target retains original dispatch");
  const auto held=refs;exit.paused=true;exit.advance(.1);require(!std::memcmp(held.data(),exit.engine->refs.data(),sizeof(held)),"Exit pause freezes all official poses");
  require(e.seed==seed && !std::memcmp(live.data(),e.refs.data(),sizeof(live)),"Exit officials never alter the live match");
  std::cout<<"Original period-end referee/assistant dispatch, animation, pause and live-state isolation passed.\n";
 }
 SDL_DestroyRenderer(r);SDL_DestroySurface(surface);SDL_Quit();return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
