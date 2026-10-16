#include "original_frontend.hpp"
#include "original_menu_overlays.hpp"
#include <iostream>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 require(argc==4,"Expected source, media and capture directory");require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
 const std::filesystem::path folder=argv[3];std::filesystem::create_directories(folder);
 const auto save=folder/("menu-art-"+std::to_string(SDL_GetTicksNS())+".sav");
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 {
  Archive archive(std::filesystem::path(argv[1])/"EURO.DAT",std::filesystem::path(argv[1])/"EURO.OFF");OriginalMenuArt art(archive);
  for(int from=0;from<16;++from)for(int to=0;to<16;++to){
   OriginalMenuCarousel motion;motion.select(from);motion.select(to);
   const int start=motion.frame();motion.advance(0);require(motion.frame()==start,"Focus hold freezes carousel");
   int frames=0;while(motion.moving() && frames++<70)motion.advance(1.0/60);
   require(!motion.moving() && motion.frame()==menu_art::aim[to]*4,"All 256 country transitions settle on original target pose");
  }
  // 200 timer interrupts/second, with integer 200/30 == 6 ticks per
  // frontend iteration: 30 ms, not the old native 60 Hz display clock.
  for(int fps:{30,60,144}){
   OriginalMenuCarousel motion;motion.select(0);motion.select(8);
   for(int frame=0;frame<fps/2;++frame)motion.advance(1.0/fps);
   require(motion.moving() && motion.frame()==14,"Half-second carousel pose follows source cadence at every display rate");
   for(int frame=0;frame<fps*2;++frame)motion.advance(1.0/fps);
   require(!motion.moving() && motion.frame()==menu_art::aim[8]*4,"Carousel timing independent of render frequency");
  }
  {OriginalMenuCarousel motion;motion.select(0);motion.select(8);motion.advance(.089);
   require(motion.frame()==0,"Carousel waits for the third source tick before reaching pose one");
   motion.advance(.001);require(motion.frame()==1,"Third source tick advances original .2/.4/.8 acceleration");}
  {OriginalMenuCarousel motion;motion.select(0);motion.select(8);motion.advance(.1);motion.select(1);for(int i=0;i<70;++i)motion.advance(1.0/60);require(!motion.moving() && motion.frame()==menu_art::aim[1]*4,"Rapid retarget keeps newest choice");}
  art.begin(3);const auto raw=archive.entry(3),pal=archive.entry(19);
  for(int y=0;y<90;++y)for(int x=0;x<640;++x){const auto pixel=raw[y*640+x];if(pixel<16)continue;for(int c=0;c<3;++c)require(art.pixels()[(y*640+x)*4+c]==pal[pixel*3+c]*255/63,"Original screen header palette is preserved");}
  art.clear();art.text(0,0,"A",OriginalMenuArt::white);const auto& glyph=menu_art::images['A'-32];const auto atlas=archive.entry(glyph.record);int ink=0;
  for(int y=0;y<glyph.h;++y)for(int x=0;x<glyph.w;++x){const int index=atlas[(glyph.y+y)*640+glyph.x+x];if(!index)continue;++ink;for(int c=0;c<3;++c)require(art.pixels()[(y*640+x)*4+c]==pal[(index+15)*3+c]*255/63,"Original glyph uses frontend gradient, not match HUD font");}
  require(ink>10,"Font contract sampled actual glyph pixels");
  // Large next-match mascots have a baked shared palette and a mirrored home
  // pose, unlike the small squad mascot's per-country palette overlay.
  art.begin(10);art.image(menu_art::LRGE_GOTH+14,0,0,0,0,-1,true);const auto& mascot=menu_art::images[menu_art::LRGE_GOTH+14];const auto mascot_raw=archive.entry(mascot.record),mascot_pal=archive.entry(26);
  for(int y=0;y<mascot.h;++y)for(int x=0;x<mascot.w;++x){const auto index=mascot_raw[(mascot.y+y)*640+mascot.x+mascot.w-1-x];if(index<16)continue;for(int c=0;c<3;++c)require(art.pixels()[(y*640+x)*4+c]==mascot_pal[index*3+c]*255/63,"Large mascot preserves original shared palette and flip");}
  OriginalGraphics g(argv[1]);g.load_cd_graphics(argv[2]);OriginalFrontend front(argv[1],argv[2],save);front.menu_fades=false;front.open(g);g.prematches_enabled=false;g.exits_enabled=false;auto& s=*g.sequence;
  using Stage=OriginalFrontend::Stage;
  const auto capture=[&](const std::string& name){auto* frame=SDL_RenderReadPixels(renderer,nullptr);require(frame,SDL_GetError());require(SDL_SavePNG(frame,(folder/(name+".png")).string().c_str()),SDL_GetError());SDL_DestroySurface(frame);};
  // Visual review of every original selection-animation frame, alongside its
  // filtered counterpart. Keep this on the same presentation path as real menus.
  art.begin(3);art.panel(12,100,616,374);
  art.text(320,110,"SELECTION CIRCLE: ORIGINAL / SMOOTH",OriginalMenuArt::white,0,2);
  for(int frame=0;frame<12;++frame){
   const int x=35+(frame%4)*150,y=150+(frame/4)*92;
   art.image(menu_art::BUTN_WHTE+frame,x,y);
   art.button(x+60,y,48,"",true,(frame+.25)/12);
  }
  art.present(renderer);capture("selection-circle-frames");
  const auto draw=[&](const std::string& name){front.draw(renderer,g,false,false);capture(name);};
  const auto key=[&](SDL_Keycode k){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;front.event(e,g);};
  const auto click=[&](float x,float y){SDL_Event e{};e.type=SDL_EVENT_MOUSE_BUTTON_DOWN;e.button.button=SDL_BUTTON_LEFT;e.button.x=48+x*1.35f;e.button.y=8+y*1.35f;front.event(e,g);};
  draw("title");key(SDLK_I);key(SDLK_M);require(!front.intro_requested && !front.movie_requested && front.movie_error.empty(),"Legacy local films do not enter the Euro96 menu");
  key(SDLK_UP);key(SDLK_RETURN);require(front.quit,"Title wraps directly to Quit without hidden film entries");front.quit=false;front.open(g);draw("title");
  click(430,336);require(front.quit,"Sixth visible title entry is Quit");front.quit=false;front.open(g);draw("title");
  click(430,296);require(front.stage()==Stage::options,"Visible title options target opens options");draw("options-match");key(SDLK_Y);draw("options-sound");key(SDLK_ESCAPE);draw("title-return");
  click(430,139);require(front.stage()==Stage::selection,"Visible exhibition target opens team select");draw("selection");const int home=s.next_home;click(590,118);require(home!=s.next_home,"Right-hand team value changes home country");draw("selection-country");
  const auto country_start=front.menu_art().pixels();front.observe(g,.1);draw("selection-moving");require(country_start!=front.menu_art().pixels(),"Frontend displays intermediate original country sprites");
  for(int i=0;i<60;++i)front.observe(g,1.0/60);draw("selection-settled");
  click(300,435);require(front.stage()==Stage::lineup,"Mouse play target enters lineup");draw("lineup");key(SDLK_T);draw("tactics-formation");key(SDLK_Y);draw("tactics-takers");key(SDLK_Y);draw("tactics-player");key(SDLK_ESCAPE);draw("lineup-return");
  click(337,431);require(s.full_match && !s.cpu_vs_cpu && !front.visible(),"Visible kickoff target starts the chosen human match");
  g.paused=false;g.action_view=true;for(int i=0;i<80;++i)g.advance(.05);g.paused=true;
  SDL_SetRenderDrawColor(renderer,24,65,39,255);SDL_RenderClear(renderer);draw_menu_pause(renderer,g,false);capture("pause-keyboard");
  SDL_RenderClear(renderer);draw_menu_pause(renderer,g,true);capture("pause-controller");
  require(g.clean_match_view(),"Restored pause menu keeps the development header hidden");
  g.paused=false;g.match_panels=true;
  const auto panel_ticks=s.ticks;const auto panel_seed=s.engine.seed;
  SDL_RenderClear(renderer);draw_menu_match_panels(renderer,g,false);g.draw(renderer);capture("match-panels-keyboard");
  const auto radar_on=front.menu_art().pixels();g.show_radar=false;
  draw_menu_match_panels(renderer,g,false);require(radar_on!=front.menu_art().pixels(),"Match panel shows current radar state");
  SDL_RenderClear(renderer);draw_menu_match_panels(renderer,g,true);g.draw(renderer);capture("match-panels-controller");
  require(s.ticks==panel_ticks && s.engine.seed==panel_seed,"Match panels never advance gameplay");
  g.show_radar=true;g.match_panels=false;g.paused=true;
  draw_menu_setup(renderer,g,0);capture("match-setup");
  draw_menu_setup(renderer,g,6);capture("match-setup-format");
  draw_menu_squad(renderer,g,false,1,4,13,"");capture("substitutions");
  require(g.start_replay(),"Live match replay available");SDL_RenderClear(renderer);draw_menu_replay(renderer,g,false);capture("replay-controls");g.finish_replay();
  front.open(g);key(SDLK_F10);draw("saved-replays");key(SDLK_ESCAPE);key(SDLK_T);draw("championship-entry");key(SDLK_RETURN);draw("groups");
  require(front.tournament().active,"Championship still starts through original menu controls");
  key(SDLK_Y);require(front.stage()==Stage::simulating,"Simulate remains available");draw("simulating");key(SDLK_ESCAPE);draw("groups-return");
  // Exercise completed group and each knockout layout using isolated fixtures.
  OriginalTournament tournament;tournament.reset(-1);
  for(int i=0;i<31;++i){const auto f=tournament.next();tournament.record({f[0],f[1],2,1});}
  tournament.save(save);front.open(g);key(SDLK_L);draw("champion");
  key(SDLK_LEFT);draw("semifinals");key(SDLK_LEFT);draw("quarterfinals");key(SDLK_RIGHT);key(SDLK_RIGHT);key(SDLK_RIGHT);draw("completed-groups");
  require(front.stage()==Stage::tournament && front.tournament().results.size()==31,"Round navigation preserves all recorded results");
  const auto ticks=s.ticks;const auto seed=s.engine.seed;for(int i=0;i<3;++i)front.draw(renderer,g,true,false);require(s.ticks==ticks && s.engine.seed==seed,"Menu redraw never advances gameplay");
  front.open(g);key(SDLK_RETURN);key(SDLK_Y);key(SDLK_RETURN);s.interval_ticks=100;s.interval_minute=45;front.observe(g,0);draw("half-time");
  s.interval_minute=90;draw("extra-time-entry");s.interval_minute=105;draw("extra-time-interval");
  s.interval_minute=45;key(SDLK_RETURN);s.step();s.finish_match();front.observe(g,0);draw("full-time");
  std::cout<<"Original font/header pixels, mouse targets, menu state preservation, every frontend page and live overlays passed.\n";
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
