#include "original_frontend.hpp"
#include "action_constants.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static bool near(float a,float b){return std::abs(a-b)<.002f;}
int main(int argc,char** argv){try{
 require(argc==4,"Expected source, media, captures");require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
 const std::filesystem::path folder=argv[3];std::filesystem::create_directories(folder);
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 {
 OriginalGraphics g(argv[1]);g.load_cd_graphics(argv[2]);g.action_view=true;auto& s=*g.sequence;auto& e=s.engine;
 OriginalFrontend front(argv[1],argv[2],folder/"unused.sav");
 const auto key=[&](SDL_Keycode k){SDL_Event v{};v.type=SDL_EVENT_KEY_DOWN;v.key.key=k;return front.event(v,g);};
 const auto pad=[&](int b){SDL_Event v{};v.type=SDL_EVENT_GAMEPAD_BUTTON_DOWN;v.gbutton.button=Uint8(b);return front.event(v,g);};
 const auto capture=[&](const std::string& name){g.draw(renderer,{48,8,864,648});front.draw(renderer,g,false,false);SDL_RenderPresent(renderer);require(SDL_SaveBMP(surface,(folder/(name+".bmp")).string().c_str()),SDL_GetError());};
 for(int venue=0;venue<5;++venue){
  g.next_stadium=venue;g.apply_stadium();s.start_match(true);
  // A chosen starter's shirt and statistics must survive the presentation copy.
  e.teams[2].shirt=14;e.teams[2].tm_rate=101;
  const auto players=e.teams;const auto refs=e.refs;const auto seed=e.seed;const auto figures=s.match_figures();const auto mode=e.match_mode;
  const auto bx=e.ballx,by=e.bally,bz=e.ballz;
  g.start_prematch();require(g.prematch.active(),"Opening starts with CD venue");
  for(int i=0;i<22;++i){const auto& p=g.prematch.engine->teams[i];
   require(near(p.tm_x,float(e.cntspot_x)+(i<11?-2:2)*e.prat) && near(p.tm_y,float(e.cntspot_y)+240-(i%11)*24),"Original two team lines");
   require(p.tm_xdis==(i<11?1:-1) && p.tm_ydis==0 && p.tm_anim==MC_STAND,"Original facing and stand clip");
   require(p.shirt==players[i].shirt && p.tm_rate==players[i].tm_rate,"Roster choice and stats retained");
  }
  const auto& officials=g.prematch.engine->refs;
  require(officials[0].anim==MC_PLEAD && near(officials[0].frm,.6f) && near(officials[0].fstep,1.0f/64),"Source referee animation and object timing");
  require(officials[1].anim==MC_STAND && officials[2].anim==MC_STAND && officials[1].dir_y==1 && officials[2].dir_y==-1,"Source assistants");
  const auto first=g.prematch.camera();require(near(first.x,640) && near(first.y,370) && near(first.z,20),"Original close opening camera");
  capture("venue-"+std::to_string(venue)+"-start");
  for(int tick=0;tick<180;++tick){
   g.advance(.05);require(s.ticks==0,"Renderer cannot advance match during opening");g.advance_prematch(.05);
   require(e.seed==seed && s.ticks==0 && e.match_time.min==0 && e.match_time.sec==0 && e.match_mode==mode && near(e.ballx,bx) && near(e.bally,by) && near(e.ballz,bz) && s.match_figures()==figures && !std::memcmp(players.data(),e.teams.data(),sizeof(players)) && !std::memcmp(refs.data(),e.refs.data(),sizeof(refs)),"Opening cannot change live state, ball, RNG, clock, officials or statistics");
   if(tick==44 || tick==89 || tick==159){
    capture("venue-"+std::to_string(venue)+"-"+std::to_string(tick+1));
    require(g.scene_counts.home_players==11 && g.scene_counts.away_players==11 && g.scene_counts.keepers==2 && g.scene_counts.officials==3 && g.scene_counts.rotating_balls==1 && g.radar_players==0 && g.tracker_markers==0,"Lineup renders full squads, keepers, officials and ball without gameplay overlays");
   }
   if(tick==159){const auto c=g.prematch.camera();require(near(c.x,640) && near(c.y,1021) && near(c.z,237),"Original orbit radius truncation and final hold");}
  }
  require(!g.prematch.active() && g.prematch.ticks()==180,"Natural nine-second finish");
  require(!g.paused && g.opening_input_guard,"Kickoff resumes with held-action guard");
  g.advance(.05);require(s.ticks==1,"Exactly one gameplay tick after completion");
 }
 std::cout<<"1. Original lineup and officials; 2. Camera orbit/hold; 3. Five venues and immutable live state passed.\n";
 g.next_stadium=5;g.apply_stadium();
 for(bool skip:{false,true}){
  s.start_match(false);g.start_prematch();
  if(skip)g.finish_prematch();else for(int tick=0;tick<180;++tick)g.advance_prematch(.05);
  require(!g.prematch.active() && s.ticks==0 && s.selected==e.setp_taker-1,"Natural/skipped opening reveals selected centre taker before any live tick");
  require(e.teams[s.selected].draw_me && e.teams[s.selected].guy_on,"Selected kickoff actor is visible");
  capture(skip?"kickoff-human-skipped":"kickoff-human-natural");
 }
 front.open(g);key(SDLK_RETURN);key(SDLK_Y);key(SDLK_RETURN);
 require(g.prematch.active() && !front.visible() && s.play && !s.cpu_vs_cpu,"Frontend human start enters opening");
 key(SDLK_P);const auto tick=g.prematch.ticks();front.observe(g,.1);require(g.prematch.ticks()==tick,"Keyboard pause freezes opening");
 pad(SDL_GAMEPAD_BUTTON_START);front.observe(g,.05);require(g.prematch.ticks()==tick+1,"Controller resumes opening");
 SDL_Event loss{};loss.type=SDL_EVENT_WINDOW_FOCUS_LOST;front.event(loss,g);front.observe(g,.1);require(g.prematch.paused && g.prematch.ticks()==tick+1,"Focus loss pauses opening");
 loss.type=SDL_EVENT_WINDOW_FOCUS_GAINED;front.event(loss,g);front.observe(g,.1);require(g.prematch.paused,"Focus gain does not resume");
 pad(SDL_GAMEPAD_BUTTON_START);loss.type=SDL_EVENT_GAMEPAD_REMOVED;front.event(loss,g);require(g.prematch.paused,"Controller disconnect pauses");
 require(!key(SDLK_F11) && !key(SDLK_F8) && !key(SDLK_F9),"Fullscreen and audio shortcuts remain available");
 for(int button:{SDL_GAMEPAD_BUTTON_SOUTH,SDL_GAMEPAD_BUTTON_EAST,SDL_GAMEPAD_BUTTON_BACK}){
  pad(button);require(!g.prematch.active() && s.full_match && !g.paused && s.ticks==0,"Pad skip preserves prepared match");g.start_prematch();
 }
 for(SDL_Keycode button:{SDLK_RETURN,SDLK_ESCAPE,SDLK_SPACE}){key(button);require(!g.prematch.active() && s.ticks==0,"Keyboard skip preserves kickoff");g.start_prematch();}
 g.prematch.advance(std::numeric_limits<double>::quiet_NaN());require(g.prematch.ticks()==0,"Ignore invalid elapsed time");
 loss={};loss.type=SDL_EVENT_MOUSE_BUTTON_DOWN;loss.button.button=SDL_BUTTON_LEFT;front.event(loss,g);require(!g.prematch.active(),"Pointer skip");
 std::cout<<"4. Keyboard/controller pause, skip, focus/disconnect and input guard passed.\n";
 s.interval_ticks=100;s.interval_minute=45;front.observe(g,.05);require(g.exit.active(),"Half-time still uses original tunnel exit");key(SDLK_RETURN);key(SDLK_RETURN);g.advance(.05);require(e.match_half==1 && !g.prematch.active(),"No new match opening during end change");
 s.finish_match();front.observe(g,.05);key(SDLK_RETURN);key(SDLK_R);key(SDLK_RETURN);require(g.prematch.active() && s.ticks==0 && !s.full_time,"Rematch gets fresh opening");key(SDLK_RETURN);
 OriginalGraphics no_cd(argv[1]);no_cd.sequence->start_match(true);no_cd.start_prematch();require(!no_cd.prematch.active(),"Missing optional CD stadium never blocks kickoff");
 std::cout<<"5. Frontend kickoff, interval, rematch and missing-media fallback passed.\n";
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
