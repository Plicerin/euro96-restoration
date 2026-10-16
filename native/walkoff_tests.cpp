#include "original_frontend.hpp"
#include "action_constants.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 {
  OriginalOcclusion cover;
  const auto v=[](float x,float y,float depth){OriginalOcclusion::Vertex p;p.draw.position={x,y};p.draw.tex_coord={x/10,y/10};p.depth=depth;return p;};
  cover.add({v(0,0,.2f),v(10,0,.2f),v(0,10,.2f)});
  require(cover.visible({v(0,0,.1f),v(10,0,.1f),v(0,10,.1f)}).empty(),"Opaque triangle hides farther face");
  const auto area=[](const auto& pieces){float total=0;for(const auto& p:pieces)for(std::size_t i=1;i+1<p.size();++i)total+=std::abs(OriginalOcclusion::cross(p[0].draw.position,p[i].draw.position,p[i+1].draw.position))*.5f;return total;};
  require(std::abs(area(cover.visible({v(0,0,.3f),v(10,0,.3f),v(0,10,.3f)}))-50)<.01f,"Nearer face remains whole");
  const auto crossing=cover.visible({v(0,0,.1f),v(10,0,.3f),v(0,10,.1f)});
  require(std::abs(area(crossing)-12.5f)<.01f,"Intersecting faces split by reciprocal depth, not average depth");
  for(const auto& p:crossing)for(const auto& point:p)require(std::abs(point.draw.tex_coord.x-point.draw.position.x/10)<.00001f && std::abs(point.draw.tex_coord.y-point.draw.position.y/10)<.00001f,"Clipping retains texture coordinates");
 }
 require(argc==4,"Expected source, media and captures");require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
 const std::filesystem::path folder=argv[3];std::filesystem::create_directories(folder);
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 {
 OriginalGraphics g(argv[1]);g.load_cd_graphics(argv[2]);g.action_view=true;auto& s=*g.sequence;auto& e=s.engine;
 OriginalFrontend front(argv[1],argv[2],folder/"unused.sav");
 const auto key=[&](SDL_Keycode key){SDL_Event v{};v.type=SDL_EVENT_KEY_DOWN;v.key.key=key;return front.event(v,g);};
 const auto pad=[&](int b){SDL_Event v{};v.type=SDL_EVENT_GAMEPAD_BUTTON_DOWN;v.gbutton.button=Uint8(b);return front.event(v,g);};
 const auto capture=[&](const std::string& name){g.draw(renderer,{48,8,864,648});front.draw(renderer,g,false,false);SDL_RenderPresent(renderer);require(SDL_SavePNG(surface,(folder/(name+".png")).string().c_str()),SDL_GetError());};
 int count=0;
 for(int venue=0;venue<5;++venue)for(int half=0;half<2;++half){
  g.next_stadium=venue;g.apply_stadium();s.start_match(true);if(half)e.swap_teams();
  const int slot=venue==4?0:2;const int index=s.blue_slot()+slot;const int oldshirt=e.teams[index].shirt;
  require(s.queue_substitution(false,slot,12),"Queue original bench player");s.apply_substitutions();
  require(e.teams[index].shirt==12 && s.walkoff_events.size()==1 && s.substitutions[0]==1,"Live substitution recorded exactly once");
  require(g.start_walkoff(),"Presentation consumes substitution event");
  const auto players=e.teams;const auto refs=e.refs;const auto figures=s.match_figures();
  const auto seed=e.seed;const auto tick=s.ticks;const auto sec=e.match_time.sec;const auto x=e.ballx,y=e.bally,z=e.ballz;
  bool inbound=false,touchline=false;int frames=0;
  std::array<int,2> occlusion_samples{};
  for(;g.walkoff.active() && frames<1201;++frames){
   const auto& actor=g.walkoff.engine->teams[index];
   require(actor.shirt==(g.walkoff.incoming()?12:oldshirt),"Outgoing identity retained until tunnel; correct incoming shirt afterward");
   touchline|=actor.tm_trap!=0;inbound|=g.walkoff.incoming();
   const int leg=g.walkoff.incoming()?1:0;
   if(actor.tm_y>1280 && occlusion_samples[leg]<3 && (leg?actor.tm_y<1420-45*occlusion_samples[leg]:actor.tm_y>1280+45*occlusion_samples[leg])){
    g.draw(renderer,{48,8,864,648});SDL_RenderPresent(renderer);
    const auto* pixels=static_cast<const unsigned char*>(surface->pixels);
    const std::vector<unsigned char> with_actor(pixels,pixels+surface->pitch*surface->h);
    if(venue==0 && !half)require(SDL_SavePNG(surface,(folder/(std::string(leg?"tunnel-entering-":"tunnel-leaving-")+std::to_string(occlusion_samples[leg])+".png")).string().c_str()),SDL_GetError());
    g.walkoff.engine->teams[index].draw_me=0;
    g.draw(renderer,{48,8,864,648});SDL_RenderPresent(renderer);
    int different=0;for(std::size_t p=0;p<with_actor.size();p+=4)if(std::memcmp(with_actor.data()+p,pixels+p,4))++different;
    g.walkoff.engine->teams[index].draw_me=1;
    std::cout<<"Close tunnel venue "<<venue<<" half "<<half<<" leg "<<leg<<" y "<<actor.tm_y<<" pixels "<<different<<'\n';
    require(different>10,"Close camera retains players visible through the real tunnel opening");++occlusion_samples[leg];
   }
   if(venue==0 && !half && (frames==30 || (inbound && count==0))){capture(inbound?"substitute-entering":"substitute-leaving");if(inbound)++count;require(g.scene_counts.home_players==11 && g.scene_counts.away_players==11 && g.scene_counts.keepers==2,"Full squads and correct keeper roles drawn");}
   g.advance(.05);front.observe(g,.05);
   require(e.seed==seed && s.ticks==tick && e.match_time.sec==sec && e.ballx==x && e.bally==y && e.ballz==z && s.match_figures()==figures && !std::memcmp(players.data(),e.teams.data(),sizeof(players)) && !std::memcmp(refs.data(),e.refs.data(),sizeof(refs)),"Walk-off cannot change live players, ball, officials, RNG, clock or stats");
  }
  require(!g.walkoff.active() && !g.walkoff.timed_out() && inbound && touchline,"Original two-stage route completes both legs without timeout");
  require(occlusion_samples[0]==3 && occlusion_samples[1]==3,"Both natural tunnel legs checked at multiple interior positions");
  require(s.substitutions[0]==1 && s.walkoff_events.empty() && !s.queue_substitution(false,3,12),"Natural completion cannot repeat substitution or permit reentry");
  require(g.opening_input_guard,"Action release required after completion");g.advance(.05);require(s.ticks==tick+1,"Live play resumes exactly once");
  std::cout<<"Venue "<<venue<<" half "<<half<<" substitution ticks="<<frames<<'\n';
 }
 std::cout<<"1. Original outgoing/incoming route, five venues, both halves, squad/stats retention passed.\n";
 // The close camera can see through the entrance. Probe each side of that
 // opening as well, where the same player must be behind opaque CD geometry.
 for(int venue=0;venue<5;++venue){
  g.next_stadium=venue;g.apply_stadium();s.start_match(true);
  s.queue_substitution(false,2,12);s.apply_substitutions();g.start_walkoff();
  for(int tick=0;tick<600 && g.walkoff.engine->teams[2].tm_y<1280;++tick)g.advance(.05);
  auto& copy=*g.walkoff.engine;for(auto& p:copy.teams)p.draw_me=0;for(auto& r:copy.refs)r.fstep=0;
  auto& actor=copy.teams[2];actor.tm_y=1340;actor.tm_z=0;copy.init_stand_act(&actor);
  for(float x:{500.0f,520.0f,595.0f,670.0f,690.0f}){
   actor.tm_x=x;actor.draw_me=1;g.walkoff.presentation=OriginalPresentation{};g.walkoff.presentation.observe(copy,1,0);
   g.draw(renderer,{48,8,864,648});SDL_RenderPresent(renderer);
   const auto* pixels=static_cast<const unsigned char*>(surface->pixels);const std::vector<unsigned char> visible(pixels,pixels+surface->pitch*surface->h);
   if(!venue)require(SDL_SavePNG(surface,(folder/("roof-probe-"+std::to_string(int(x))+".png")).string().c_str()),SDL_GetError());
   actor.draw_me=0;g.draw(renderer,{48,8,864,648});SDL_RenderPresent(renderer);
   int different=0;for(std::size_t p=0;p<visible.size();p+=4)if(std::memcmp(visible.data()+p,pixels+p,4))++different;
   std::cout<<"Roof probe venue "<<venue<<" x "<<x<<" pixels "<<different<<'\n';
   require(x==595?different>10:different==0,"Entrance remains open; flanking roof/crowd covers the same actor");
  }g.walkoff.finish();
 }
 for(bool second:{false,true}){
  s.start_match(false);e.seed=20;const int index=second?13:2;
  if(second){e.send_off_plr(index+1,30);s.update_match_depth();require(s.walkoff_events.empty(),"Yellow card has no walk-off");}
  e.send_off_plr(index+1,second?30:100);s.update_match_depth();
  require(!e.teams[index].guy_on && s.walkoff_events.size()==1,"Original direct/second-yellow dismissal remains authoritative");
  const auto book=e.teams[index].tm_book;g.start_walkoff();require(g.walkoff.active() && !g.walkoff.replacement(),"Dismissal gets exit only");
  if(!second){for(int n=0;n<30;++n)g.advance(.05);capture("sent-off");}
  for(int n=0;n<1201 && g.walkoff.active();++n){require(!g.walkoff.incoming(),"No replacement for red card");g.advance(.05);}
  require(!g.walkoff.active() && !g.walkoff.timed_out() && !e.teams[index].guy_on && e.teams[index].tm_book==book,"Natural dismissal preserves ten-player side and discipline");
  s.update_match_depth();require(s.walkoff_events.empty(),"Dismissal presentation cannot be duplicated");
 }
 std::cout<<"2. Direct red, second yellow, no resurrection/replacement and natural completion passed.\n";
 s.start_match(false);require(s.queue_substitution(false,0,12) && s.queue_substitution(true,3,13),"Two-side queued substitution fixture");s.apply_substitutions();require(g.start_walkoff(),"Start first queued event");
 g.paused=true;g.advance(.1);require(g.walkoff.ticks()==0,"Global pause also freezes presentation");g.paused=false;
 key(SDLK_P);const auto tick=g.walkoff.ticks();g.advance(.1);require(g.walkoff.ticks()==tick,"Keyboard pauses movement");pad(SDL_GAMEPAD_BUTTON_START);g.advance(.05);require(g.walkoff.ticks()==tick+1,"Controller resumes");
 SDL_Event focus{};focus.type=SDL_EVENT_WINDOW_FOCUS_LOST;front.event(focus,g);g.advance(.1);require(g.walkoff.paused && g.walkoff.ticks()==tick+1,"Focus loss freezes copy");focus.type=SDL_EVENT_WINDOW_FOCUS_GAINED;front.event(focus,g);require(g.walkoff.paused,"Focus gain does not resume");
 pad(SDL_GAMEPAD_BUTTON_START);focus.type=SDL_EVENT_GAMEPAD_REMOVED;front.event(focus,g);require(g.walkoff.paused,"Disconnect pauses");
 require(!key(SDLK_F11) && !key(SDLK_F8),"Display/audio shortcuts remain available");
 pad(SDL_GAMEPAD_BUTTON_SOUTH);require(g.walkoff.active() && s.walkoff_events.empty() && g.walkoff.ticks()==0,"Skip advances to second queued event without a game tick");
 pad(SDL_GAMEPAD_BUTTON_BACK);require(!g.walkoff.active() && s.full_match && !g.paused && s.substitutions[0]==1 && s.substitutions[1]==1,"Skip completes both substitutions without replacing match");
 std::cout<<"3. Queue order, keeper slot, keyboard/gamepad pause/skip, focus/disconnect and input guard passed.\n";
 s.start_match(true);s.queue_substitution(false,3,12);s.apply_substitutions();g.start_walkoff();s.start_match(true);g.advance(.05);require(!g.walkoff.active() && s.walkoff_events.empty(),"New match discards stale transition");
 s.queue_substitution(false,2,12);s.apply_substitutions();g.walkoffs_enabled=false;g.advance(.05);require(!g.walkoff.active() && s.walkoff_events.empty() && e.teams[2].shirt==12,"Diagnostics can omit presentation without dropping actual substitution");g.walkoffs_enabled=true;
 std::cout<<"4. Reset and diagnostic fallback preserve real match decisions passed.\n";
 s.start_match(true);auto& ref=e.refs[0];ref.x=100;ref.y=200;ref.goto_x=152;ref.goto_y=200;ref.act=1;
 for(int i=0;i<25 && ref.act==1;++i)e.goto_target(&ref);
 require(ref.act==3 && std::isfinite(ref.x) && std::abs(ref.x-152)<2.61f,"Original referee jog reaches target");
 ref.goto_x=ref.x;ref.goto_y=ref.y;ref.act=1;e.goto_target(&ref);require(ref.act==3 && std::isfinite(ref.dir_x),"Coincident target avoids division by zero");
 ref.target=3;e.teams[2].tm_x=ref.x;e.teams[2].tm_y=ref.y+100;ref.dir_x=1;ref.dir_y=0;
 for(int n=0;n<14;++n)e.at_target(&ref);require(std::abs(ref.dir_x)<.001f && ref.dir_y>.999f,"Referee faces player's actual Y coordinate");
 e.teams[2].tm_y=ref.y;e.at_target(&ref);require(ref.act==4 && std::isfinite(ref.dir_y),"Coincident facing remains finite");
 s.card_notice_ticks=0;e.card_events=s.seen_card_events;ref.act=1;ref.goto_x=ref.x+52;ref.goto_y=ref.y;e.match_mode=5;s.restart_ticks=20;const auto oldx=ref.x;
 s.update_match_depth();require(ref.x>oldx,"Live dead-ball scheduler invokes original referee movement");
 auto& assistant=e.refs[1];assistant.act=1;assistant.goto_x=assistant.x+52;assistant.goto_y=assistant.y;const auto ax=assistant.x;s.update_match_depth();require(assistant.x>ax,"Assistant restart route is connected");
 std::cout<<"5. Referee/assistant restart movement, accurate facing and zero-distance guards passed.\n";
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
