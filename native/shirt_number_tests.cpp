#include "original_graphics.hpp"
#include "original_actions.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
static std::uint64_t hash(std::span<const unsigned char> data){std::uint64_t h=1469598103934665603ull;for(auto b:data){h^=b;h*=1099511628211ull;}return h;}
static std::uint64_t back_hash(SDL_Surface* surface){
 Bytes pixels;for(int y=205;y<252;++y)for(int x=300;x<340;++x){const auto* p=static_cast<unsigned char*>(surface->pixels)+y*surface->pitch+x*4;pixels.insert(pixels.end(),p,p+4);}return hash(pixels);
}
int main(int argc,char** argv){try{
 require(argc==4,"Source, media, scratch required");require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
 const std::filesystem::path media=argv[2],scratch=argv[3];std::filesystem::create_directories(scratch);
 OriginalShirtNumbers numbers;numbers.load(media);require(numbers.loaded(),"Original 1-20 atlas loaded");
 std::set<std::uint64_t> glyphs;
 for(int i=0;i<40;++i){const auto& uv=numbers.coordinates()[i];require(uv[0]>>24==16,"Numbers occupy their own texture page");
  int opaque=0,clear=0;Bytes pixels;
  const int x=(uv[4]&0xffffff)>>16,y=(uv[0]&0xffffff)>>16;
  for(int row=0;row<25;++row)for(int col=0;col<17;++col){const auto at=((y+row)*256+x+col)*4;auto p=std::span(numbers.rgba()).subspan(at,4);pixels.insert(pixels.end(),p.begin(),p.end());if(p[3])++opaque;else ++clear;}
  require(opaque>100 && clear>10,"Each original glyph has ink and transparent background");glyphs.insert(hash(pixels));
 }
 require(glyphs.size()==40,"All 20 numerals in both variants remain distinct");
 for(int player:{1,2,12,13})for(bool mirror:{false,true})for(bool skin:{false,true})for(bool dark:{false,true})for(int shirt=1;shirt<=20;++shirt){
  const auto* original=OriginalGraphics::player_faces(player,mirror,false,skin);
  const auto faces=OriginalGraphics::numbered_player_faces(player,mirror,shirt,skin,1500,dark);
  require(faces[0]==13 && faces[82]==-3500-(dark?20:0)-(shirt-1),"Every shirt binds its complete original glyph, including keepers and reserves");
  for(int i=0;i<87;++i)if(i!=82)require(faces[i]==original[i],"Number binding cannot alter body, head, keeper or mirrored geometry");
 }
 for(int shirt=16;shirt<=20;++shirt)require(OriginalGraphics::numbered_player_faces(2,false,shirt)[0]==12,"Missing optional media preserves the old safe fallback");
 const auto raw=read_file(media/"GRAPHICS/NUMBERS.BIN");const auto corrupt=scratch/"corrupt"/"GRAPHICS";std::filesystem::create_directories(corrupt);
 for(int mode=0;mode<5;++mode){auto data=raw;if(mode==0)data.pop_back();if(mode==1)data[0]='!';if(mode==2)data[8]=64;if(mode==3)data[104+3]=7;if(mode==4)data.back()=32;
  {std::ofstream out(corrupt/"NUMBERS.BIN",std::ios::binary);out.write(reinterpret_cast<const char*>(data.data()),data.size());}
  OriginalShirtNumbers invalid;bool rejected=false;try{invalid.load(corrupt.parent_path());}catch(const std::exception&){rejected=true;}require(rejected && !invalid.loaded(),"Malformed optional atlas rejected atomically");
 }
 numbers.load(scratch/"absent");require(!numbers.loaded(),"Removing optional media clears its state");
 auto* surface=SDL_CreateSurface(640,480,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 {
 OriginalGraphics g(argv[1]);g.load_cd_graphics(media);require(g.has_retail_numbers(),"Actual renderer imports numbers");g.action_view=true;g.angle=3.14159265f;
 auto& s=*g.sequence;auto& e=s.engine;
 for(int half=0;half<2;++half)for(int side=0;side<2;++side)for(int keeper=0;keeper<2;++keeper)for(int mirror=0;mirror<2;++mirror){
  s.next_home=4;s.next_away=5;s.start_match(true);if(half)e.swap_teams();s.team_play=s.play=false;s.running=false;
  for(auto& ref:e.refs)ref.fstep=0;e.ball_poss=e.ball_limbo_on=0;e.ballx=700;e.bally=400;
  const int slot=((side+half)%2)*11+(keeper?0:1);auto& p=e.teams[slot];p.tm_x=640;p.tm_y=400;p.tm_z=0;p.tm_xdis=1;p.tm_ydis=0;p.tm_anim=mirror?39:78;p.tm_frm=.25f;s.visible_players={slot};
  std::set<std::uint64_t> backs;
  for(int shirt=1;shirt<=20;++shirt){p.shirt=shirt;++s.generation;g.draw(renderer,{0,0,640,480});SDL_RenderPresent(renderer);backs.insert(back_hash(surface));
   if(!half && !keeper && !mirror)require(SDL_SaveBMP(surface,(scratch/("number-"+std::to_string(side)+"-"+std::to_string(shirt)+".bmp")).string().c_str()),SDL_GetError());
  }
  require(backs.size()==20,"Actual home/away, mirrored, keeper and end-swapped backs must show twenty distinct numbers");
 }
 // Reserve identity comes through the real substitution path, not just a test assignment.
 s.next_home=4;s.next_away=5;s.next_squad_enabled=true;
 for(int side=0;side<2;++side)for(int slot=0;slot<20;++slot){s.next_order[side][slot]=slot+1;s.next_fitness[side][slot]=100;s.next_available[side][slot]=1;}
 std::swap(s.next_order[0][11],s.next_order[0][17]);s.start_match(true);require(s.queue_substitution(false,1,18),"Reserve substitution accepted");s.apply_substitutions();require(e.teams[1].shirt==18,"Substitute keeps squad shirt 18");
 s.team_play=s.play=false;s.running=false;s.visible_players={1};auto& p=e.teams[1];p.tm_x=640;p.tm_y=400;p.tm_z=0;p.tm_xdis=1;p.tm_ydis=0;p.tm_anim=78;p.tm_frm=.25f;
 for(auto& ref:e.refs)ref.fstep=0;e.ball_poss=e.ball_limbo_on=0;e.ballx=700;e.bally=400;++s.generation;
 // Compare only pixels changed by the numeral. Pitch colours legitimately vary by venue.
 const auto numeral_signature=[&](){
  Bytes before;++s.generation;g.draw(renderer,{0,0,640,480});SDL_RenderPresent(renderer);
  for(int y=205;y<252;++y)for(int x=300;x<340;++x){const auto* q=static_cast<unsigned char*>(surface->pixels)+y*surface->pitch+x*4;before.insert(before.end(),q,q+4);}
  p.shirt=19;++s.generation;g.draw(renderer,{0,0,640,480});SDL_RenderPresent(renderer);Bytes changed;int i=0;
  for(int y=205;y<252;++y)for(int x=300;x<340;++x,++i){const auto* q=static_cast<unsigned char*>(surface->pixels)+y*surface->pitch+x*4;if(std::memcmp(before.data()+i*4,q,4)){changed.push_back(x-300);changed.push_back(y-205);changed.insert(changed.end(),before.begin()+i*4,before.begin()+i*4+4);changed.insert(changed.end(),q,q+4);}}
  p.shirt=18;require(!changed.empty(),"Substitute's actual number remains visible");return hash(changed);
 };
 const auto first=numeral_signature();
 // Stadium changes rebuild the UV table; number bindings must survive every venue.
 for(int venue=0;venue<5;++venue){g.next_stadium=venue;g.apply_stadium();require(first==numeral_signature(),"Venue changes preserve the player's number material");}
 g.load_cd_graphics(media);require(first==numeral_signature(),"Reloading media cannot duplicate or shift number bindings");
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();std::cout<<"Forty original glyphs, transparent backgrounds, shirts 1-20 on both teams/keepers/mirrored poses/both halves, reserve substitution, five venues and media reload passed.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';SDL_Quit();return 1;}}
