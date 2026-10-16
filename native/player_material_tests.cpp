#include "original_graphics.hpp"
#include "original_actions.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <algorithm>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 require(argc==4,"Source, CD media, captures required");require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
 auto* surface=SDL_CreateSurface(640,480,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 const std::filesystem::path folder=argv[3];std::filesystem::create_directories(folder);
 {
 const auto retail_palette=std::filesystem::exists(std::filesystem::path(argv[2])/"GRAPHICS/PLAYERS.BIN")?read_file(std::filesystem::path(argv[2])/"GRAPHICS/PLAYERS.BIN"):Bytes{};
 OriginalGraphics g(argv[1]);g.load_cd_graphics(argv[2]);g.action_view=true;auto& s=*g.sequence;s.reset();s.national_teams=true;s.home_team=5;s.away_team=5;s.running=false;
 auto& e=s.engine;e.ball_poss=0;e.ball_limbo_on=0;e.ballx=700;e.bally=400;auto& p=e.teams[1];p.tm_x=640;p.tm_y=400;p.tm_xdis=1;p.tm_ydis=0;
 for(int mirror=0;mirror<2;++mirror)for(int back=0;back<2;++back){
  g.angle=back?3.14159265f:0; p.tm_anim=mirror?39:78;p.tm_frm=.25f;s.visible_players.clear();++s.generation;
  g.draw(renderer,{0,0,640,480});SDL_RenderPresent(renderer);
  const auto* bytes=static_cast<unsigned char*>(surface->pixels);const std::vector<unsigned char> empty(bytes,bytes+surface->pitch*surface->h);
  for(int variant=0;variant<6;++variant){
   p.tm_player=variant<3?2:13;p.shirt=variant%3==1?2:9;if(variant%3==2){p.tm_player=variant<3?1:12;p.shirt=1;}
   s.visible_players={1};++s.generation;g.draw(renderer,{0,0,640,480});SDL_RenderPresent(renderer);
   const std::string name=std::to_string(variant)+"-"+std::to_string(mirror)+"-"+std::to_string(back);
   require(SDL_SaveBMP(surface,(folder/(name+".bmp")).string().c_str()),SDL_GetError());
   int changed=0,minx=640,maxx=0,miny=480,maxy=0;
   for(int y=0;y<480;++y)for(int x=0;x<640;++x){const int pos=y*surface->pitch+x*4;if(std::memcmp(bytes+pos,empty.data()+pos,3)){++changed;minx=std::min(minx,x);maxx=std::max(maxx,x);miny=std::min(miny,y);maxy=std::max(maxy,y);}}
   require(changed>550 && miny<215,"A player must have a visible head and body, not just a shirt number and boots");
   std::cout<<name<<" pixels="<<changed<<" bounds="<<minx<<","<<miny<<"-"<<maxx<<","<<maxy<<'\n';
  }
 }
 // Exercise actual starting rosters with their authored appearance flags. Keep
 // the match's material/half state but isolate one player in the fixed camera.
 for(int team=0;team<16;++team){
  s.next_home=team;s.next_away=(team+1)%16;s.start_match(true);s.team_play=s.play=false;s.running=false;
  for(auto& ref:e.refs)ref.fstep=0;
  e.ball_poss=e.ball_limbo_on=0;e.ballx=700;e.bally=400;g.angle=0;
  for(int half=0;half<2;++half){e.match_half=half;s.visible_players.clear();++s.generation;
   g.draw(renderer,{0,0,640,480});SDL_RenderPresent(renderer);
   const auto* bytes=static_cast<unsigned char*>(surface->pixels);const std::vector<unsigned char> empty(bytes,bytes+surface->pitch*surface->h);
   for(int slot=0;slot<22;++slot){auto& player=e.teams[slot];player.tm_x=640;player.tm_y=400;player.tm_z=0;player.tm_xdis=1;player.tm_ydis=0;player.tm_anim=78;player.tm_frm=.25f;
    s.visible_players={slot};++s.generation;g.draw(renderer,{0,0,640,480});SDL_RenderPresent(renderer);
    int body=0,keeper_jersey=0;
    for(int y=200;y<250;++y)for(int x=298;x<340;++x){const int pos=y*surface->pitch+x*4;
     body+=std::memcmp(bytes+pos,empty.data()+pos,3)!=0;
     for(int index=g.has_retail_players()?179:164;index<=(g.has_retail_players()?191:171);++index){const auto c=g.has_retail_players()?std::array<unsigned char,3>{retail_palette[8+index*3],retail_palette[9+index*3],retail_palette[10+index*3]}:g.palette_color(index);if(bytes[pos]==c[0]*4 && bytes[pos+1]==c[1]*4 && bytes[pos+2]==c[2]*4){++keeper_jersey;break;}}
    }
    if(body<400)throw std::runtime_error("Missing roster body: team "+std::to_string(team)+" half "+std::to_string(half)+" slot "+std::to_string(slot));
    if(slot==0 || slot==11)require(keeper_jersey>60,"Both match keepers must display the original purple jersey in either half");
   }
  }
 }
 std::cout<<"All 16 team selections, 22 roster slots and both half material bindings render complete bodies; both keepers retain their separate original kit.\n";
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
