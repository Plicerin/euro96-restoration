#include "original_frontend.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
using namespace actua;
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
static std::uint64_t hash(SDL_Surface* s){std::uint64_t h=1469598103934665603ull;const auto* b=static_cast<unsigned char*>(s->pixels);for(int i=0;i<s->pitch*s->h;++i){h^=b[i];h*=1099511628211ull;}return h;}
int main(int argc,char** argv){try{
 require(argc==4,"Source, media and captures required");require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
 const std::filesystem::path media=argv[2],folder=argv[3];std::filesystem::create_directories(folder);
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* r=SDL_CreateSoftwareRenderer(surface);require(r,SDL_GetError());
 {
 OriginalGraphics g(argv[1]);g.load_cd_graphics(media);require(g.stadium()==5 && g.next_stadium==5,"Retail Wembley is default");g.action_view=true;auto& s=*g.sequence;auto& e=s.engine;
 const auto draw=[&](const std::string& name){g.draw(r,{48,8,864,648});SDL_RenderPresent(r);require(SDL_SavePNG(surface,(folder/(name+".png")).string().c_str()),SDL_GetError());};
 std::set<std::uint64_t> pictures;
 for(int venue=5;venue<13;++venue){
  require(g.stadium_available(venue),"All eight retail stadium files available");g.next_stadium=venue;g.apply_stadium();s.start_match(true);g.overview=true;
  const auto seed=e.seed;const auto ticks=s.ticks;draw("venue-"+std::to_string(venue));pictures.insert(hash(surface));
  require(g.scene_counts.stands==4 && g.scene_counts.flags==4 && e.seed==seed && s.ticks==ticks,"Venue render preserves four stands, flags and live state");
  g.paused=true;const auto first=hash(surface);g.draw(r,{48,8,864,648});SDL_RenderPresent(r);require(first==hash(surface),"Frozen retail venue is deterministic");g.paused=false;g.overview=false;
  for(int camera=0;camera<3;++camera){g.match_camera=camera;draw("camera-"+std::to_string(venue)+"-"+std::to_string(camera));}
  for(int half=0;half<2;++half){s.start_match(true);if(half)e.swap_teams();const auto snapshot=e.teams;const auto random=e.seed;
   g.start_exit();require(g.exit.active(),"Retail exit starts");for(int tick=0;tick<900 && g.exit.active();++tick){g.advance_exit(.05);if(tick==180 || tick==250)draw("exit-"+std::to_string(venue)+"-"+std::to_string(half)+"-"+std::to_string(tick));}
   if(g.exit.ticks()>=900)throw std::runtime_error("Retail tunnel timeout at venue "+std::to_string(venue));
   require(!std::memcmp(snapshot.data(),e.teams.data(),sizeof(snapshot)) && random==e.seed,"Retail exits preserve live players/RNG");
  }
  OriginalTournament t;t.reset(4,5,venue,0);const auto save=folder/("venue-"+std::to_string(venue)+".sav");t.save(save);OriginalTournament loaded;loaded.load(save);require(loaded.venue==venue,"Retail venue survives tournament save/load");
 }
 require(pictures.size()==8,"All eight stadiums have distinct rendered frames");
 g.next_stadium=12;g.change_stadium(1);require(g.next_stadium==5,"Retail selector wraps to Wembley");g.change_stadium(-1);require(g.next_stadium==12,"Retail selector wraps backwards");
 for(int legacy=0;legacy<5;++legacy){g.next_stadium=legacy;g.apply_stadium();require(g.stadium()==legacy,"Old saved venue IDs remain available");}
 g.next_stadium=12;g.apply_stadium();s.start_match(true);for(int i=0;i<90;++i)g.advance(.05);require(g.start_replay(),"Retail match produces replay");const auto replay=g.save_replay(folder/"replays");g.finish_replay();g.next_stadium=5;g.apply_stadium();g.open_saved_replay(replay);require(g.stadium()==12,"Saved replay restores retail venue");g.finish_replay();require(g.stadium()==5,"Closing replay restores live venue");
 const auto good=read_file(media/"GRAPHICS/EURO_STADIUM0.BIN");const auto bad=folder/"bad"/"GRAPHICS";std::filesystem::create_directories(bad);
 for(int mode=0;mode<4;++mode){auto b=good;if(mode==0)b.resize(500);if(mode==1)b[248]=64;if(mode==2)b[0]='!';if(mode==3)b.push_back(0);{std::ofstream f(bad/"EURO_STADIUM0.BIN",std::ios::binary);f.write(reinterpret_cast<const char*>(b.data()),b.size());}bool rejected=false;try{OriginalGraphics invalid(argv[1]);invalid.load_cd_graphics(bad.parent_path());}catch(const std::exception&){rejected=true;}require(rejected,"Invalid retail stadium rejected");}
 }
 SDL_DestroyRenderer(r);SDL_DestroySurface(surface);SDL_Quit();std::cout<<"Eight retail stadiums, 24 cameras, both-half tunnel exits, saves/replays, legacy IDs and malformed media passed.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';SDL_Quit();return 1;}}
