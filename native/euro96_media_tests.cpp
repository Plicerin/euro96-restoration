#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "original_commentary.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <map>
#include <cmath>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(int argc,char** argv){try{
 require(argc==4,"Source, media, scratch required");require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO),SDL_GetError());
 const std::filesystem::path media=argv[2],scratch=argv[3];std::filesystem::create_directories(scratch);
 OriginalEuroKits kits;kits.load(media);require(kits.loaded(),"Retail overlay available");
 OriginalEuroKits absent;absent.load(scratch/"absent");require(!absent.loaded() && absent.appearance(4,19)==-1,"Optional absence preserves existing appearance");
 require(kits.selection(4,6,true)==22 && kits.selection(4,12,true)==28 && kits.selection(1,15,true)==31,"Retail country-specific alternate choices");
 require(kits.appearance(4,19)==1 && kits.appearance(4,9)==0,"Exact-name Ince/Shearer appearance mapping");
 require(kits.appearance(9,3)==1 && kits.appearance(9,12)==1,"Verified Helder and Neno appearance mapping");
 for(int team=0;team<16;++team)for(int shirt=1;shirt<=20;++shirt)require(kits.appearance(team,shirt)>=0,"Every preserved player has a verified appearance");
 require(kits.selection(-1,0,true)==-1 && kits.appearance(16,1)==-1 && kits.appearance(1,21)==-1,"Out-of-range identities rejected");
 std::set<int> variants;
 for(int home=0;home<16;++home)for(int away=0;away<16;++away){variants.insert(kits.selection(home,away,false));variants.insert(kits.selection(home,away,true));}
 require(variants.size()==32,"All authored primary and alternate strips reachable");
 Bytes rgba(65536*4,123),untouched=rgba;kits.overlay(rgba,4,1,15);require(rgba==untouched,"Keeper/referee pages are never replaced");
 kits.overlay(rgba,1,1,15);require(rgba!=untouched,"Croatia uniform changes actual texture pixels");
 const auto raw=read_file(media/"GRAPHICS/EURO96.BIN");
 const auto malformed=scratch/"corrupt"/"GRAPHICS";std::filesystem::create_directories(malformed);
 for(int mode=0;mode<4;++mode){auto data=raw;
  if(mode==0)data.pop_back();if(mode==1)data[0]='?';if(mode==2)data[8]=64;if(mode==3)data[data.size()-321]=2;
  {std::ofstream out(malformed/"EURO96.BIN",std::ios::binary);out.write(reinterpret_cast<const char*>(data.data()),data.size());}
  bool rejected=false;try{absent.load(malformed.parent_path());}catch(const std::exception&){rejected=true;}
  require(rejected && !absent.loaded(),"Malformed media rejected without installing partial state");
 }
 auto* surface=SDL_CreateSurface(640,480,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 {
 OriginalGraphics g(argv[1]);g.load_cd_graphics(media);require(g.has_euro96_kits(),"Renderer loads retail overlay");g.action_view=true;
 auto& s=*g.sequence;auto& e=s.engine;
 for(int team=0;team<16;++team)for(int alternate=0;alternate<2;++alternate){
  int opponent=0;while(opponent<16 && (opponent==team || kits.selection(opponent,team,true)!=(team+16*alternate)))++opponent;
  require(opponent<16,"Fixture for every strip exists");s.next_home=opponent;s.next_away=team;s.start_match(true);s.team_play=s.play=false;s.running=false;
  for(auto& ref:e.refs)ref.fstep=0;
  e.ball_poss=e.ball_limbo_on=0;e.ballx=700;e.bally=400;
  auto& p=e.teams[12];p.tm_x=640;p.tm_y=400;p.tm_z=0;p.tm_xdis=1;p.tm_ydis=0;p.tm_anim=78;p.tm_frm=.25f;
  for(int back=0;back<2;++back){g.angle=back?3.14159265f:0;s.visible_players.clear();++s.generation;g.draw(renderer,{0,0,640,480});SDL_RenderPresent(renderer);
   auto* bytes=static_cast<unsigned char*>(surface->pixels);Bytes background(bytes,bytes+surface->pitch*surface->h);
   s.visible_players={12};++s.generation;g.draw(renderer,{0,0,640,480});SDL_RenderPresent(renderer);
   int body=0;for(int y=200;y<250;++y)for(int x=298;x<340;++x)body+=std::memcmp(bytes+y*surface->pitch+x*4,background.data()+y*surface->pitch+x*4,3)!=0;
   require(body>400 && g.has_original_kit(team),"Every retail strip renders a complete visible player");
   require(SDL_SaveBMP(surface,(scratch/("kit-"+std::to_string(team)+"-"+std::to_string(alternate)+"-"+std::to_string(back)+".bmp")).string().c_str()),SDL_GetError());
  }
 }
 // Independent provenance tests validate WAV bytes against the CD inventory.
 // Here exercise all 960 tones through the real callback in both halves,
 // including reserve shirt identities, and compare the complete mixed PCM.
 const auto names=read_file(media/"COMMENTARY/EURO_NAMES.BIN");
 const auto number=[&](std::size_t p,int n){unsigned v=0;for(int i=0;i<n;++i)v|=unsigned(names.at(p+i))<<(8*i);return v;};
 std::map<std::pair<int,int>,Bytes> expected;std::size_t at=12;
 for(unsigned i=0;i<number(8,4);++i){const int c=number(at,2),t=number(at+2,2);const auto size=number(at+4,4);at+=8;
  SDL_AudioSpec spec{};Uint8* pcm=nullptr;Uint32 count=0;
  require(SDL_LoadWAV_IO(SDL_IOFromConstMem(names.data()+at,size),true,&spec,&pcm,&count),SDL_GetError());
  expected[{c,t}]={pcm,pcm+count};SDL_free(pcm);at+=size;
 }
 require(expected.size()==960 && at==names.size(),"All player name tones present");
 for(int team=0;team<16;++team)for(int half=0;half<2;++half){
  s.next_home=team;s.next_away=(team+1)%16;s.start_match(true);s.ticks=10;if(half)e.swap_teams();
  const int slot=half?19:8;
  OriginalCommentary voice(media,false);voice.set_volume(100);voice.update(s,0,true);
  for(int shirt=1;shirt<=20;++shirt)for(int tone=0;tone<3;++tone){
  e.teams[slot].shirt=shirt;const auto seed=e.seed;e.PlayTeamSample(9,tone);voice.update(s,0,true);
  if(voice.category()!=128+team*20+shirt-1 || voice.history().empty() || voice.history().back().number!=tone+1)throw std::runtime_error("Retail voice fixture team="+std::to_string(team)+" half="+std::to_string(half)+" shirt="+std::to_string(shirt)+" tone="+std::to_string(tone)+" category="+std::to_string(voice.category()));
  const auto& pcm=expected.at({128+team*20+shirt-1,tone+1});std::vector<float> out((pcm.size()+8)*2);voice.mix(out.data(),int(pcm.size()+8));
  for(std::size_t i=0;i<pcm.size();++i){const auto ramp=std::min({1.0f,float(i)/55,float(pcm.size()-i)/55});const auto value=(float(pcm[i])-128)/128*ramp;
   require(std::abs(out[2*i]-value)<.000001f && out[2*i]==out[2*i+1],"Exact full player-name PCM");}
  require(!voice.speaking() && out.back()==0 && e.seed==seed,"Name finishes exactly without gameplay RNG change");
  for(int tick=0;tick<52;++tick)voice.update(s,.25,true);
  }
 }
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();
 std::cout<<"Euro96: 32 strips front/back, country alternates, 320 appearance identities, 1920 complete name PCM/callback cases, optional absence and malformed media passed.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';SDL_Quit();return 1;}}
