#include "original_frontend.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::string bytes(const std::filesystem::path& path){std::ifstream f(path,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
int main(int argc,char** argv){try{
 require(argc==4,"Expected source, media, scratch");require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO),SDL_GetError());
 const std::filesystem::path folder=argv[3];std::filesystem::create_directories(folder);const auto save=folder/"session.sav";
 {
 OriginalGraphics g(argv[1]);g.load_cd_graphics(argv[2]);auto& s=*g.sequence;
 using Stage=OriginalFrontend::Stage;
 std::unique_ptr<OriginalFrontend> front;
 const auto reopen=[&]{front=std::make_unique<OriginalFrontend>(argv[1],argv[2],save);front->open(g);};
 const auto key=[&](SDL_Keycode k){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;require(front->event(e,g),"Session key consumed");};
 const auto update=[&]{g.advance(.05);front->observe(g,.05);require(front->tournament_error().empty(),"Tournament reported an error");};
 reopen();key(SDLK_T);key(SDLK_RETURN);require(front->stage()==Stage::tournament,"Create spectator competition through title");
 const bool scheduled=std::all_of(OriginalTournament::fixture_venues.begin(),OriginalTournament::fixture_venues.end(),[&](int v){return g.stadium_available(v);});
 const int fixedVenue=front->tournament().venue;
 require(front->tournament().scheduled_venues==scheduled,"New retail championship uses original venue schedule when all packs are present");
 constexpr int venues[]={0,2,4,6,1,3,5,7,3,1,7,5,0,2,4,6,3,2,0,1,7,6,5,4,0,5,4,1,4,0,0};std::set<int> visited;
 int walks=0,openings=0,exits=0,halves=0,replays=0,goals=0,extra=0,pens=0;
 for(int fixture=0;fixture<31;++fixture){
  const auto pair=front->tournament().next();key(SDLK_RETURN);require(front->stage()==Stage::lineup,"Next fixture opens lineup");key(SDLK_RETURN);
  require(front->stage()==Stage::hidden && s.cpu_vs_cpu && g.prematch.active(),"Lineup starts CPU match opening");++openings;
  const int venue=scheduled?venues[fixture]+5:fixedVenue;
  require(g.next_stadium==venue && g.stadium()==venue,"Watched match loads authored stadium before opening");visited.insert(g.stadium());
  bool oldWalk=false,oldExit=false,oldReplay=false,paused=false;int previousWalkTick=-1;int halfSeen=-1;int frames=0;
  for(;frames<50000;++frames){
   update();
   require(std::isfinite(s.engine.ballx) && std::isfinite(s.engine.bally) && std::isfinite(s.engine.ballz),"Session ball remains finite");
   require(s.engine.ball_poss>=0 && s.engine.ball_poss<=22,"Session possession valid");
   if(g.walkoff.active() && (!oldWalk || g.walkoff.ticks()<previousWalkTick))++walks;
   if(oldWalk && !g.walkoff.active())require(!g.walkoff.timed_out(),"Individual transition completes without timeout");
   oldWalk=g.walkoff.active();previousWalkTick=g.walkoff.ticks();
   if(g.replaying && !oldReplay)++replays;oldReplay=g.replaying;
   if(g.exit.active() && !oldExit){
    ++exits;const int tick=s.ticks;key(SDLK_P);const int exitTick=g.exit.ticks();
    for(int hold=0;hold<5;++hold)update();
    require(s.ticks==tick && g.exit.ticks()==exitTick,"Paused exit freezes both copies");key(SDLK_P);
   }
   oldExit=g.exit.active();
   if(front->stage()==Stage::interval && s.interval_minute!=halfSeen){halfSeen=s.interval_minute;++halves;}
   // CPU half-time automatically resumes after the complete exit and menu delay.
   if(!paused && s.ticks>=200 && !g.replaying && !g.exit.active() && !g.walkoff.active() && front->stage()==Stage::hidden){
    key(SDLK_F2);require(front->stage()==Stage::tournament && g.paused,"Live match hub pauses");const int tick=s.ticks;
    for(int hold=0;hold<5;++hold)update();require(s.ticks==tick,"Hub holds live match");
    key(SDLK_ESCAPE);require(front->stage()==Stage::hidden && !g.paused,"Hub return resumes same match");paused=true;
   }
   if(front->stage()==Stage::result && !g.exit.active())break;
  }
  if(frames==50000){std::cerr<<"Stalled fixture="<<fixture+1<<" stage="<<int(front->stage())<<" clock="<<s.clock_label()<<" ticks="<<s.ticks<<" exit="<<g.exit.active()<<" replay="<<g.replaying<<'\n';throw std::runtime_error("Watched session failed to reach result");}
  require(!g.prematch.active() && g.prematch.ticks()==180,"Watched opening completes naturally");
  require(s.full_time && front->tournament().results.size()==std::size_t(fixture+1),"Exactly one genuine completed result");
  const auto result=front->tournament().results.back();require(result.home==pair[0] && result.away==pair[1] && result.hg==s.blue_goals && result.ag==s.red_goals,"Saved score matches watched fixture");
  goals+=result.hg+result.ag;extra+=result.extra;pens+=result.pens;
  const auto checkpoint=bytes(save);for(int hold=0;hold<5;++hold)update();require(bytes(save)==checkpoint,"Result screen never duplicates checkpoint");
  OriginalTournament loaded;loaded.load(save);loaded.save(folder/"roundtrip.sav");require(bytes(folder/"roundtrip.sav")==checkpoint,"Entire squad, condition, tactics and results roundtrip exactly");
  key(SDLK_RETURN);require(front->stage()==Stage::tournament,"Result returns to hub");
  const auto next=front->tournament().next();reopen();key(SDLK_L);require(front->stage()==Stage::tournament && front->tournament().results.size()==std::size_t(fixture+1) && front->tournament().next()==next,"Fresh frontend resumes exact next fixture");
  require(front->tournament().scheduled_venues==scheduled,"Venue policy survives every frontend reload");
  std::cout<<"Fixture "<<fixture+1<<" "<<original_teams[result.home].code<<' '<<result.hg<<':'<<result.ag<<' '<<original_teams[result.away].code<<" venue="<<OriginalGraphics::stadium_names[venue]<<" frames="<<frames<<" extra="<<result.extra<<" pens="<<result.pens<<std::endl;
 }
 require(front->tournament().complete() && front->tournament().champion()>=0,"Champion survives final reload");
 require(scheduled?(visited.size()==8 && g.stadium()==5):(visited.size()==1 && g.stadium()==fixedVenue),"Championship visits all eight retail venues and finishes at Wembley, or retains its legacy venue");
 require(walks>0 && openings==31 && exits==62 && halves>=31 && replays>0,"Whole-session transitions and replays exercised");
 const auto champion=front->tournament().champion();key(SDLK_RETURN);key(SDLK_Y);update();require(front->tournament().champion()==champion && front->tournament().results.size()==31,"Completed competition cannot add another fixture");
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 front->draw(renderer,g,false,false);require(SDL_RenderPresent(renderer),SDL_GetError());require(SDL_SavePNG(surface,(folder/"champion.png").string().c_str()),SDL_GetError());front.reset();SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);
 std::cout<<"Full frontend session: 31 matches, "<<walks<<" individual transitions, 31 natural openings, "<<exits<<" natural tunnel exits, "<<halves<<" intervals, "<<replays<<" replays, "<<goals<<" goals, "<<extra<<" extra-time matches, "<<pens<<" shootouts; champion="<<original_teams[champion].name<<". All 31 saved checkpoints resumed.\n";
 }
 SDL_Quit();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
