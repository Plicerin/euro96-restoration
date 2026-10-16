#include "original_frontend.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace actua;
void require(bool condition,const char* message){if(!condition)throw std::runtime_error(message);}
int main(int argc,char** argv){try{
 require(argc==3,"Expected source and CD roots");require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO),SDL_GetError());
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 {
  OriginalGraphics graphics(argv[1]);OriginalFrontend front(argv[1],argv[2]);OriginalMusic music(argv[2],false);
  auto& s=*graphics.sequence;using Stage=OriginalFrontend::Stage;using Track=OriginalMusic::Track;
  const auto key=[&](SDL_Keycode k){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;require(front.event(e,graphics),"Menu key consumed");};
  front.open(graphics);require(front.stage()==Stage::title && graphics.paused,"Title owns input and pauses match");
  key(SDLK_RETURN);const int initial=s.next_home;key(SDLK_RIGHT);require(s.next_home!=initial && s.next_home!=s.next_away,"Team selector skips opponent");
  key(SDLK_DOWN);key(SDLK_DOWN);const int formation=s.next_home_formation;key(SDLK_RIGHT);require(s.next_home_formation==(formation+1)%10,"Formation choice");
  key(SDLK_Y);require(front.stage()==Stage::lineup && !s.running,"Preview does not start simulation");
  key(SDLK_RIGHT);key(SDLK_RETURN);require(s.full_match && !s.cpu_vs_cpu && !front.visible(),"Human match confirmed");
  const int home=s.home_team;front.setup(graphics);key(SDLK_ESCAPE);require(!front.visible() && !graphics.paused && s.home_team==home,"Cancel setup preserves current match");
  key(SDLK_F2);key(SDLK_F2);require(!front.visible() && !graphics.paused,"Setup shortcut toggles without losing pause state");
  s.interval_ticks=100;s.interval_minute=45;front.observe(graphics,.05);require(front.stage()==Stage::interval && graphics.paused,"Interval stops simulation");
  for(int i=0;i<100;++i)front.observe(graphics,.25);require(front.stage()==Stage::interval && s.interval_ticks==100,"Human interval waits for confirmation");
  key(SDLK_F2);key(SDLK_ESCAPE);require(front.stage()==Stage::interval,"Cancel settings returns to half-time");
  key(SDLK_RETURN);s.step();require(!s.interval_ticks && s.engine.match_half==1 && !graphics.paused,"Original end swap after continue");
  s.blue_goals=1;s.red_goals=2;s.finish_match();front.observe(graphics,0);require(front.stage()==Stage::result && front.music(s)==Track::lose,"Home loss music");
  s.blue_goals=2;require(front.music(s)==Track::draw,"Draw music");s.blue_goals=3;require(front.music(s)==Track::win,"Home win music");
  s.cpu_vs_cpu=true;s.blue_goals=1;require(front.music(s)==Track::win,"CPU spectator result celebrates winner");
  s.shootout=true;s.penalty_scores={3,4};s.cpu_vs_cpu=false;require(front.music(s)==Track::lose,"Shootout outcome chooses music");
  key(SDLK_R);require(front.stage()==Stage::lineup,"Rematch has pre-match review");key(SDLK_ESCAPE);
  front.menu_fades=false;front.draw(renderer,graphics,false,false);
  SDL_Event click{};click.type=SDL_EVENT_MOUSE_BUTTON_DOWN;click.button.button=SDL_BUTTON_LEFT;click.button.x=48+100*1.35f;click.button.y=8+435*1.35f;
  require(front.event(click,graphics) && front.stage()==Stage::lineup,"Mouse watch button");key(SDLK_RETURN);require(s.cpu_vs_cpu,"Mouse chooses CPU match");
  s.interval_ticks=100;s.interval_minute=45;front.observe(graphics,0);for(int i=0;i<33;++i)front.observe(graphics,.25);
  require(!front.visible() && s.interval_ticks==1,"CPU interval automatically continues");
  std::vector<float> output(22050*2);
  for(auto track:{Track::menu,Track::win,Track::lose,Track::draw}){
   require(music.available(track),"Original music imported");music.select(track);music.mix(output.data(),22050);
   double energy=0;for(float v:output){require(std::isfinite(v) && std::abs(v)<=.5f,"Bounded music PCM");energy+=std::abs(v);}
   require(energy>1,"Track decodes to non-silent music");const auto cursor=music.cursor();music.select(track);require(music.cursor()==cursor,"Repeated selection never restarts music");
   if(track!=Track::menu){for(int i=0;i<8;++i)music.mix(output.data(),22050);require(music.uses_retail_theme()?(music.track()==Track::menu && !music.ended()):music.ended(),"Retail results use continuing theme; compatibility stings play once");}
  }
  music.select(Track::none);music.mix(output.data(),22050);for(float v:output)require(v==0,"No music during match");
  OriginalMusic dummy(argv[2]);require(dummy.ready(),"Dummy music stream opens");dummy.select(Track::menu);dummy.update(true);const auto cursor=dummy.cursor();dummy.update(false);require(dummy.cursor()==cursor,"Focus loss holds music position");dummy.update(true);
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);
 SDL_Quit();std::cout<<"Original front-end and music contracts passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';SDL_Quit();return 1;}}
