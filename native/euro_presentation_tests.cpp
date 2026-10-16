#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "original_commentary.hpp"
#include "original_music.hpp"
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace actua;
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
static std::uint64_t pcm_hash(const std::vector<float>& pcm,int begin,int end,int stride,float scale){
 std::uint64_t h=1469598103934665603ull;for(int i=begin;i<end;i+=stride){require(std::isfinite(pcm[i]),"Finite mixer output");h^=static_cast<unsigned char>(std::lround(pcm[i]*scale)+128);h*=1099511628211ull;}return h;
}
int main(int argc,char** argv){try{
 require(argc==4,"Source, media and captures required");require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO),SDL_GetError());
 const std::filesystem::path media=argv[2],scratch=argv[3];std::filesystem::create_directories(scratch);
 OriginalEuroPlayers art;art.load(media);require(art.loaded() && art.skin_shift(4)==0 && art.skin_shift(8)==6 && art.skin_shift(15)==6,"Verified retail team skin table");
 const auto raw=read_file(media/"GRAPHICS/PLAYERS.BIN");const auto bad=scratch/"bad"/"GRAPHICS";std::filesystem::create_directories(bad);
 for(int mode=0;mode<5;++mode){auto b=raw;if(mode==0)b.pop_back();if(mode==1)b[0]='!';if(mode==2)b[8]=64;if(mode==3)b[776]=2;if(mode==4)b[792+3]=16;
  {std::ofstream f(bad/"PLAYERS.BIN",std::ios::binary);f.write(reinterpret_cast<const char*>(b.data()),b.size());}
  OriginalEuroPlayers invalid;bool rejected=false;try{invalid.load(bad.parent_path());}catch(const std::exception&){rejected=true;}require(rejected && !invalid.loaded(),"Reject invalid player media without partial state");
 }
 art.load(scratch/"absent");require(!art.loaded(),"Optional absence restores fallback");
 auto* surface=SDL_CreateSurface(640,480,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 {
 OriginalGraphics g(argv[1]);g.load_cd_graphics(media);require(g.has_retail_players(),"Player art reaches live renderer");g.action_view=true;auto& s=*g.sequence;auto& e=s.engine;
 // Real starter identities: default, medium, dark and both keepers, front/back
 // and mirrored poses in both halves. Existing roster test covers all 16 sides.
 for(int team:{4,5,8,15})for(int half=0;half<2;++half)for(int mirror=0;mirror<2;++mirror)for(int back=0;back<2;++back){
  s.next_home=team;s.next_away=(team+1)%16;s.start_match(true);if(half)e.swap_teams();s.team_play=s.play=false;s.running=false;
  for(auto& ref:e.refs)ref.fstep=0;e.ball_poss=e.ball_limbo_on=0;e.ballx=700;e.bally=400;g.angle=back?3.14159265f:0;
  s.visible_players.clear();++s.generation;g.draw(renderer,{0,0,640,480});SDL_RenderPresent(renderer);
  const auto* pixels=static_cast<unsigned char*>(surface->pixels);Bytes background(pixels,pixels+surface->pitch*surface->h);
  for(int slot:{0,1,8,11,12,19}){auto& p=e.teams[slot];p.tm_x=640;p.tm_y=400;p.tm_z=0;p.tm_xdis=1;p.tm_ydis=0;p.tm_anim=mirror?39:78;p.tm_frm=.25f;s.visible_players={slot};++s.generation;
   g.draw(renderer,{0,0,640,480});SDL_RenderPresent(renderer);int body=0,head=0;
   // The mirrored running pose lowers the head below the standing pose.
   for(int y=195;y<255;++y)for(int x=298;x<340;++x){const int at=y*surface->pitch+x*4;if(std::memcmp(pixels+at,background.data()+at,3)){++body;if(y<219)++head;}}
   if(body<=400 || head<=15){SDL_SaveBMP(surface,(scratch/"failed-player.bmp").string().c_str());throw std::runtime_error("Player visibility team="+std::to_string(team)+" half="+std::to_string(half)+" mirror="+std::to_string(mirror)+" back="+std::to_string(back)+" slot="+std::to_string(slot)+" body="+std::to_string(body)+" head="+std::to_string(head));}
   if(!half && !mirror && slot<11)require(SDL_SaveBMP(surface,(scratch/("player-"+std::to_string(team)+"-"+std::to_string(slot)+"-"+std::to_string(back)+".bmp")).string().c_str()),SDL_GetError());
  }
 }
 s.ticks=10;
 OriginalCommentary voice(media,false);voice.set_volume(100);voice.update(s,0,true);
 const auto legacy=scratch/"welcome-compatibility"/"COMMENTARY";std::filesystem::create_directories(legacy);
 std::filesystem::copy_file(media/"COMMENTARY/EURO_ENGLISH.BIN",legacy/"EURO_ENGLISH.BIN",std::filesystem::copy_options::overwrite_existing);
 OriginalCommentary compatibility(legacy.parent_path(),false);compatibility.set_volume(100);compatibility.update(s,0,true);
 for(int home=0;home<16;++home)for(int away=0;away<16;++away){
  require(!voice.welcome(home,away) && !voice.speaking(),"All 256 retail exhibitions skip compatibility introduction");
  require(compatibility.welcome(home,away),"All 256 country fragments remain available without sequence overlay");
  if((home==1 || home==15) && away==4){std::vector<float> pcm(22050);compatibility.mix(pcm.data(),11025);require(pcm_hash(pcm,110,10000,2,128)==(home==1?13097125201340349182ull:2834331754771017440ull),"Country introduction PCM matches independent retail MAD sample");}
 }
 OriginalMusic music(media,false);music.select(OriginalMusic::Track::menu);std::vector<float> pcm(10000);music.mix(pcm.data(),5000);
 require(pcm_hash(pcm,2204,6000,1,256)==8281174073776877299ull,"Menu mixer plays the original Euro96 WAV samples");
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();std::cout<<"Retail player bodies/heads/keepers, both halves/mirrors, media validation, 256 introductions and independent PCM checks passed.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';SDL_Quit();return 1;}}
