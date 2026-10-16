#include "original_frontend.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::string bytes(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
static void write_save(const std::filesystem::path& p,const std::string& body){
 std::uint64_t h=1469598103934665603ull;for(unsigned char c:body){h^=c;h*=1099511628211ull;}
 std::ofstream f(p,std::ios::binary);f<<body<<h<<'\n';
}
int main(int argc,char** argv){try{
 require(argc==4,"Expected source, media, scratch");require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO),SDL_GetError());
 const std::filesystem::path folder=argv[3];std::filesystem::create_directories(folder);const auto save=folder/"venues.sav";
 // Independent retail byte table, not the generated source array being tested.
 constexpr int expected[]={0,2,4,6,1,3,5,7,3,1,7,5,0,2,4,6,3,2,0,1,7,6,5,4,0,5,4,1,4,0,0};
 OriginalTournament t,loaded;t.reset(14,5,3,2);t.scheduled_venues=true;
 for(int i=0;i<31;++i){
  require(t.fixture_venue(i)==expected[i]+5,"Source schedule matches retail fixture index");
  t.save(save);loaded.load(save);loaded.save(folder/"roundtrip.sav");require(loaded.scheduled_venues && loaded.fixture_venue(i)==expected[i]+5 && bytes(save)==bytes(folder/"roundtrip.sav"),"Scheduled checkpoint roundtrips exactly");
  const auto pair=t.next();t.record({pair[0],pair[1],2,1});
 }
 require(t.fixture_venue(-1)==-1 && t.fixture_venue(31)==-1,"Invalid fixture has no venue");
 // Legacy versions never acquire a schedule, including when loaded over a v4 object.
 for(int venue=0;venue<13;++venue){
  t.reset(-1,5,venue,0);t.save(save);const auto v3=bytes(save);
  std::istringstream lines(v3);std::vector<std::string> body;std::string line;while(std::getline(lines,line))body.push_back(line);
  for(int version=1;version<=3;++version){
   std::string old="ACTUA96_TOURNAMENT "+std::to_string(version)+"\n";
   const int end=version==1?2:version==2?338:354;
   for(int i=1;i<end;++i)old+=body.at(i)+'\n';write_save(folder/"legacy.sav",old);
   loaded.scheduled_venues=true;loaded.load(folder/"legacy.sav");
   require(!loaded.scheduled_venues && loaded.venue==venue,"Old format retains selected venue");
   for(int i=0;i<31;++i)require(loaded.fixture_venue(i)==venue,"Every legacy fixture retains venue");
   loaded.save(folder/"roundtrip.sav");require(bytes(folder/"roundtrip.sav")==v3,"Legacy migration retains all default state and v3 bytes");
  }
 }
 t.reset(14,5,3,2);t.scheduled_venues=true;t.save(save);
 auto body=bytes(save);body=body.substr(0,body.rfind('\n',body.size()-2)+1);
 const auto flag=body.find("14 5 3 2 0 1\n");require(flag!=std::string::npos,"Expected v4 settings");body.replace(flag,12,"14 5 3 2 0 2\n");write_save(folder/"bad.sav",body);
 loaded=t;bool rejected=false;try{loaded.load(folder/"bad.sav");}catch(...){rejected=true;}
 require(rejected && loaded.scheduled_venues && loaded.user==14 && loaded.results.empty(),"Invalid schedule with valid checksum cannot mutate progress");
 {
  OriginalGraphics g(argv[1]);g.load_cd_graphics(argv[2]);
  t.prepare(*g.sequence);require(g.sequence->next_home==14 && t.fixture_venue(0)==5,"Home-slot swap preserves Wembley opening");
  using Stage=OriginalFrontend::Stage;
  const auto key=[&](OriginalFrontend& f,SDL_Keycode k){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;require(f.event(e,g),"Venue menu key consumed");};
  OriginalFrontend front(argv[1],argv[2],save);front.open(g);key(front,SDLK_L);key(front,SDLK_RETURN);
  require(front.stage()==Stage::lineup && g.next_stadium==5,"Saved scheduled opener chooses Wembley for away human");
  // Reopen a genuine pre-schedule checkpoint with all retail media installed.
  t.reset(-1,5,3,0);t.save(save);OriginalFrontend legacy(argv[1],argv[2],save);legacy.open(g);key(legacy,SDLK_L);key(legacy,SDLK_RETURN);
  require(legacy.stage()==Stage::lineup && g.next_stadium==3,"Frontend does not upgrade old venue policy");
 }
 {
  // A source-only installation has no retail packs; do not modify real media.
  OriginalGraphics g(argv[1]);using Stage=OriginalFrontend::Stage;
  const auto key=[&](OriginalFrontend& f,SDL_Keycode k){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;require(f.event(e,g),"Missing-media key consumed");};
  OriginalFrontend fallback(argv[1],argv[2],folder/"fallback.sav");fallback.open(g);key(fallback,SDLK_T);key(fallback,SDLK_RETURN);
  require(fallback.stage()==Stage::tournament && !fallback.tournament().scheduled_venues,"No-retail installation keeps fixed venue");
  t.reset(-1);t.scheduled_venues=true;t.save(save);OriginalFrontend missing(argv[1],argv[2],save);missing.open(g);key(missing,SDLK_L);key(missing,SDLK_RETURN);
  require(missing.stage()==Stage::tournament && missing.tournament_error()=="SCHEDULED VENUE UNAVAILABLE" && missing.tournament().results.empty(),"Missing scheduled media cannot launch wrong venue or alter progress");
 }
 SDL_Quit();std::cout<<"31 scheduled checkpoints, 39 legacy versions/venues, away human, invalid policy and absent-media handling passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
