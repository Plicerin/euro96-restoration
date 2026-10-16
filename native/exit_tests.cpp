#include "original_frontend.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
using namespace actua;
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 require(argc==4,"Expected source, media, captures");require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
 const std::filesystem::path folder=argv[3];std::filesystem::create_directories(folder);
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 {
 OriginalGraphics g(argv[1]);g.load_cd_graphics(argv[2]);g.action_view=true;auto& s=*g.sequence;auto& e=s.engine;
 for(int venue=0;venue<5;++venue)for(int half=0;half<2;++half){
  g.next_stadium=venue;g.apply_stadium();s.start_match(true);for(int tick=0;tick<300;++tick)s.step();
  if(half)e.swap_teams();e.teams[3].guy_on=0;e.teams[3].draw_me=0;
  const auto officials=e.refs;const auto players=e.teams;const auto seed=e.seed;const auto ticks=s.ticks;const auto figures=s.match_figures();const auto min=e.match_time.min;const auto sec=e.match_time.sec;
  g.paused=true;g.start_exit();require(g.exit.active() && g.exit.remaining()==21,"Only active players enter exit");
  const auto start=g.exit.engine->teams;
  for(int n=0;n<899 && g.exit.active();++n){g.advance_exit(.05);
   if(n==40){require(g.exit.engine->teams[2].tm_x!=start[2].tm_x || g.exit.engine->teams[2].tm_y!=start[2].tm_y,"Original actions move players toward tunnel");
    g.exit.paused=true;const auto paused=g.exit.engine->teams;g.advance_exit(.1);require(!std::memcmp(paused.data(),g.exit.engine->teams.data(),sizeof(paused)),"Pause freezes exit poses");g.exit.paused=false;}
   if((n==100 || n==200 || n==230 || n==250 || n==270) && venue==0 && !half && g.exit.active()){
    g.draw(renderer,{48,8,864,648});SDL_RenderPresent(renderer);require(SDL_SaveBMP(surface,(folder/("exit-"+std::to_string(n)+".bmp")).string().c_str()),SDL_GetError());
    require(g.scene_counts.rotating_balls==0 && g.scene_counts.officials==3 && g.scene_counts.home_players+g.scene_counts.away_players>0,"Exit draws original players and all officials without match ball");
   }
   require(e.seed==seed && s.ticks==ticks && e.match_time.min==min && e.match_time.sec==sec && s.match_figures()==figures && !std::memcmp(players.data(),e.teams.data(),sizeof(players)) && !std::memcmp(officials.data(),e.refs.data(),sizeof(officials)),"Exit cannot change live RNG, match time, player state or statistics");
  }
  std::cout<<"Venue "<<venue<<" half "<<half<<" exit ticks "<<g.exit.ticks()<<" remaining "<<g.exit.remaining()<<'\n';
  require(!g.exit.active() && g.exit.ticks()<900,"Every active player reaches original tunnel before safety timeout");
 }
 // Compare actual rendered pixels against an empty stadium. Both tunnel lanes
 // must remain visible on approach, then be covered by the CD roof/crowd mesh.
 for(int venue=0;venue<5;++venue)for(int half=0;half<2;++half)for(int side=0;side<2;++side){
  g.next_stadium=venue;g.apply_stadium();s.start_match(true);if(half)e.swap_teams();
  for(auto& p:e.teams)p.guy_on=p.draw_me=0;for(auto& ref:e.refs)ref.fstep=0;
  const int index=side?13:2;auto& actor=e.teams[index];actor.guy_on=actor.draw_me=1;actor.tm_x=side?605.6667f:584.3333f;
  for(float y:{600.0f,800.0f,1040.0f,1140.0f,1280.0f,1340.0f,1380.0f}){
   actor.tm_y=y;e.init_stand_act(&actor);g.start_exit();
   g.draw(renderer,{48,8,864,648});SDL_RenderPresent(renderer);
   const auto* pixels=static_cast<const unsigned char*>(surface->pixels);
   const std::vector<unsigned char> visible(pixels,pixels+surface->pitch*surface->h);
   g.exit.engine->teams[index].draw_me=0;
   g.draw(renderer,{48,8,864,648});SDL_RenderPresent(renderer);
   int different=0;for(std::size_t i=0;i<visible.size();i+=4)if(std::memcmp(visible.data()+i,pixels+i,4))++different;
   std::cout<<"Occlusion venue "<<venue<<" half "<<half<<" side "<<side<<" y "<<y<<" actor pixels "<<different<<'\n';
   require(y<1200?different>10:different==0,"Approaching players visible; players inside tunnel cannot overpaint stadium");
   g.exit.finish();
  }
 }
 s.start_match(false);OriginalFrontend front(argv[1],argv[2]);front.open(g);
 const auto key=[&](SDL_Keycode k){SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=k;front.event(event,g);};
 key(SDLK_RETURN);key(SDLK_Y);key(SDLK_RETURN);if(g.prematch.active())key(SDLK_RETURN);s.interval_ticks=100;s.interval_minute=45;front.observe(g,.05);
 require(g.exit.active() && front.stage()==OriginalFrontend::Stage::interval,"Half-time exit precedes team talk");
 front.observe(g,0);require(g.exit.paused,"Focus loss freezes exit");front.observe(g,.05);require(g.exit.paused,"Focus return does not auto-resume");key(SDLK_P);front.observe(g,.1);require(g.exit.ticks()==2,"Keyboard resumes exit");
 g.draw(renderer,{48,8,864,648});front.draw(renderer,g,false,false);SDL_RenderPresent(renderer);require(SDL_SaveBMP(surface,(folder/"exit-controls.bmp").string().c_str()),SDL_GetError());
 key(SDLK_RETURN);require(!g.exit.active() && front.stage()==OriginalFrontend::Stage::interval && s.interval_ticks==100,"Skip opens team talk without advancing half");
 key(SDLK_RETURN);g.advance(.05);require(e.match_half==1 && !s.interval_ticks,"Continue swaps sides exactly once");
 s.finish_match();front.observe(g,.05);require(g.exit.active() && front.stage()==OriginalFrontend::Stage::result,"Full-time exit precedes result");
 SDL_Event pad{};pad.type=SDL_EVENT_GAMEPAD_BUTTON_DOWN;pad.gbutton.button=SDL_GAMEPAD_BUTTON_START;front.event(pad,g);require(g.exit.paused,"Controller pauses exit");
 pad.gbutton.button=SDL_GAMEPAD_BUTTON_SOUTH;front.event(pad,g);require(!g.exit.active() && front.stage()==OriginalFrontend::Stage::result,"Controller skips to result");
 key(SDLK_R);key(SDLK_RETURN);require(!s.full_time && !g.exit.active(),"Rematch clears exit presentation");if(g.prematch.active())key(SDLK_RETURN);
 s.interval_ticks=100;s.interval_minute=105;front.observe(g,.05);require(!g.exit.active(),"Extra-time end change does not invent a tunnel break");
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();std::cout<<"Original tunnel routes, five venues/both ends, immutable live state, period transitions, pause/focus and keyboard/gamepad skip passed.\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
