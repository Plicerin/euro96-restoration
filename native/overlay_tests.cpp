#include "original_graphics.hpp"
#include "original_actions.hpp"
#include <iostream>
#include <cstring>
#include <stdexcept>
using namespace actua;
static void need(bool b,const char* m){if(!b)throw std::runtime_error(m);}
int main(int argc,char** argv){try{
 Archive a(std::filesystem::path(argv[1])/"EUROREND.DAT",std::filesystem::path(argv[1])/"EUROREND.OFF");
 OriginalOverlay overlay(a);OriginalOverlay::Palette palette{};
 for(int i=0;i<256;++i)palette[i]={static_cast<unsigned char>(i%64),static_cast<unsigned char>((i/4)%64),static_cast<unsigned char>((i/16)%64)};
 overlay.clear(palette);overlay.sprite(89,100,100,31);
 const auto raw=a.entry(37);const auto& descriptor=recovered::hud_sprites[89];
 for(int y=0;y<4;++y)for(int x=0;x<4;++x){const int v=raw[(descriptor.y+y)*256+descriptor.x+x];const int index=v==1?31:v-1;
  std::uint32_t expected=0;if(v){const auto c=palette[index];expected=unsigned(c[0])*4|(unsigned(c[1])*4<<8)|(unsigned(c[2])*4<<16)|0xff000000u;}
  need(overlay.pixels()[(100+y)*640+100+x]==expected,"Original sprite transparency/recolor/palette decoding");
 }
 need(overlay.text_width("O")==overlay.text_width("0"),"Original O/zero glyph mapping");
 need(overlay.text_width("1:00")==39,"Original proportional clock metrics");
 const auto first=OriginalOverlay::marker(640,400,0,0),second=OriginalOverlay::marker(640,400,11,1);
 need(first==std::array<int,3>{49,413,90} && first==second,"Home marker follows roster swap, not camera");
 need(OriginalOverlay::marker(640,400,0,1)[2]==92,"Away marker after ends change");
 need(OriginalOverlay::marker(0,0,0,0)[0]==15 && OriginalOverlay::marker(1280,800,0,0)[1]==367,"Original pitch coordinate endpoints");
 overlay.clear(palette);overlay.sprite(90,-4,-4);overlay.sprite(90,639,479);overlay.text(-2,475,"Czech Republic",31);need(overlay.pixels().size()==640*480,"Clipping preserves HUD buffer bounds");
 overlay.clear(palette);overlay.score("ENGLAND","SCOTLAND",2,1,"45:00",true,true);
 int clock=0,score=0;for(int y=0;y<480;++y)for(int x=0;x<640;++x)if(overlay.pixels()[y*640+x]){if(y<15)++clock;if(y>=460)++score;}
 need(clock>50 && score>200,"Recovered font and substitution artwork loaded");
 need(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);need(renderer,SDL_GetError());
 {
 OriginalGraphics g(argv[1]);g.action_view=true;auto& s=*g.sequence;s.start_match(true);for(int i=0;i<80;++i)g.advance(.05);
 auto& e=s.engine;std::vector<unsigned char> snapshot(sizeof(e));std::memcpy(snapshot.data(),&e,sizeof(e));
 for(int camera=0;camera<3;++camera){g.change_camera();g.draw(renderer,{48,4,864,648});}
 need(g.radar_players==22,"All active players appear on radar");
 need(!std::memcmp(snapshot.data(),&e,sizeof(e)),"HUD and enlarged viewport must not mutate engine");
 e.teams[4].guy_on=0;g.draw(renderer);need(g.radar_players==21,"Sent-off player excluded");e.teams[4].guy_on=1;
 g.show_radar=false;g.draw(renderer);need(g.radar_players==0,"Radar hide toggle");g.show_radar=true;
 g.match_panels=false;need(g.clean_match_view(),"Clean match default");g.paused=true;need(!g.clean_match_view(),"Pause restores help panels");g.paused=false;g.match_panels=true;need(!g.clean_match_view(),"Optional panels retained");g.match_panels=false;
 need(g.start_replay(),"Replay available");g.draw(renderer);need(g.radar_players==22,"Replay radar uses replay roster");g.finish_replay();
 s.practice_restart(1);need(!g.clean_match_view(),"Set-piece controls remain accessible");
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();std::cout<<"Original HUD atlas, metrics, halftime mapping, clipping, roster, replay and presentation isolation passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}}

