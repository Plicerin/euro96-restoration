#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "graphics_data.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <set>
#include <stdexcept>
using namespace actua;
static void require(bool v,const char* m){if(!v)throw std::runtime_error(m);}
static std::uint64_t hash(const void* data,std::size_t size){auto* p=static_cast<const unsigned char*>(data);std::uint64_t h=1469598103934665603ull;while(size--){h^=*p++;h*=1099511628211ull;}return h;}
static void save(SDL_Surface* s,const std::filesystem::path& folder,const char* name){if(!folder.empty()){std::filesystem::create_directories(folder);require(SDL_SaveBMP(s,(folder/name).string().c_str()),SDL_GetError());}}
int main(int argc,char** argv){try{
 require(argc>=3,"Source and CD media roots required");require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
 const std::filesystem::path captures=argc>3?argv[3]:"";
 Archive archive(std::filesystem::path(argv[1])/"EUROREND.DAT",std::filesystem::path(argv[1])/"EUROREND.OFF");
 OriginalCdKits kits;kits.load(argv[2]);int count=0;
 for(int team=0;team<16;++team){require(kits.has(team)==(team!=1 && team!=15),"CD matches fourteen Euro countries; absent countries remain explicit");if(kits.has(team))++count;}
 OriginalCdKits missing;missing.load(std::filesystem::path(argv[2])/"missing");require(!missing.has(5),"Optional media absence retains original fallback");
 for(int team=0;team<16;++team)if(original_teams[team].kit>=0)for(int part=0;part<8;++part){const auto old=archive.entry(recovered::kits[original_teams[team].kit][part]);require(std::equal(old.begin(),old.end(),kits.part(team,part).begin(),kits.part(team,part).end()),"CD kit is byte-identical wherever the preserved source has artwork");}
 MatchPalette pal{};const auto base=archive.entry(0);std::memcpy(pal.data(),base.data(),768);
 const auto pitchpal=archive.entry(68),skypal=archive.entry(recovered::sky_palette);std::memcpy(pal[128].data(),pitchpal.data(),48);std::memcpy(pal[208].data(),skypal.data(),48);
 OriginalEnvironment env(archive);legacy::Core core;
 // Real original goal meshes: their origin can pass behind the near plane while net faces remain visible.
 const auto uv=archive.entry(1);std::memcpy(core.textures[1],uv.data(),uv.size());core.clip_base=core.clip_tex=int(uv.size()/32)+1;
 int retained=0;
 for(float x:{-45.0f,-28.0f,-10.0f,10.0f,30.0f})for(float y:{10.0f,25.0f,50.0f})for(float z:{350.0f,400.0f,450.0f}){
  core.camera(x,y,z,0,15,400,440,640,480);
  legacy::obj goal{nullptr,nullptr,-28,0,-400,1,0};core.objdepd(&goal,recovered::goals[3]);core.addobjy(&goal);
  const bool discarded=core.curpol==core.polygons.data();core.clear();core.addobjnc(&goal);
  if(discarded && core.curpol>core.polygons.data())++retained;
 }
 require(retained>0,"Original clipped goal path retains visible net faces when object origin is behind camera");
 const auto color=[&](int i){return std::uint32_t(pal[i][0]*4)|(std::uint32_t(pal[i][1]*4)<<8)|(std::uint32_t(pal[i][2]*4)<<16)|0xff000000u;};
 require(env.pitch_index(-1,-1)==env.pitch_index(63,63),"Original pitch tile wraps correctly at negative coordinates");
 core.camera(640.25f,260,800.25f,640.25f,0,400.25f,440,640,480);env.render(core,pal);
 require(env.ground_pixels>100000,"Original pitch texture covers the visible ground");
 require(env.pixels()[240*640+320]==color(env.pitch_index(640.25,-400.25)),"Ground projection meets camera target in world coordinates");
 std::set<std::uint32_t> grass;for(auto c:env.pixels())if(c)grass.insert(c);require(grass.size()>8,"Pitch includes original texture detail");
 core.camera(640,100,900,640,80,400,440,640,480);env.render(core,pal);require(env.sky_pixels>30000,"Low camera reveals original sky");
 const auto sky_hash=hash(env.pixels().data(),640*80*4);
 auto* ground_surface=SDL_CreateSurface(640,480,SDL_PIXELFORMAT_RGBA32);require(ground_surface!=nullptr,SDL_GetError());
 for(int y=0;y<480;++y)std::memcpy(static_cast<char*>(ground_surface->pixels)+y*ground_surface->pitch,env.pixels().data()+y*640,640*4);
 save(ground_surface,captures,"graphics-sky-pitch.bmp");SDL_DestroySurface(ground_surface);
 core.camera(640,100,-100,640,80,400,440,640,480);env.render(core,pal);require(hash(env.pixels().data(),640*80*4)!=sky_hash,"Sky scroll follows original heading");
 for(float height:{0.0f,1.0f,100.0f,1700.0f})for(float target:{-500.0f,0.0f,100.0f,2000.0f}){core.camera(640,height,900,640,target,400,440,640,480);env.render(core,pal);require(env.ground_pixels<=640*480 && env.sky_pixels<=640*480,"Horizon and steep-camera raster stays bounded");}
 std::srand(5432);const int next=std::rand();std::srand(5432);env.render(core,pal);require(std::rand()==next,"Environment never consumes gameplay RNG");
 auto* surface=SDL_CreateSurface(960,700,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer!=nullptr,SDL_GetError());
 {
 OriginalGraphics g(argv[1]);g.action_view=true;auto& s=*g.sequence;s.next_home=5;s.next_away=6;s.start_match(true);g.draw(renderer);SDL_RenderPresent(renderer);
 const auto red_band_pixels=[&](){int count=0;for(int y=116;y<276;++y)for(int x=24;x<664;++x){const auto* pixel=static_cast<const unsigned char*>(surface->pixels)+y*surface->pitch+x*4;if(pixel[0]>180 && pixel[1]<120 && pixel[2]<130)++count;}return count;};
 const int old_red=red_band_pixels();require(old_red>5000 && !g.has_cd_stadium(),"Fixture reproduces the preserved Euro stadium red band");
 const auto substitute=hash(surface->pixels,surface->pitch*surface->h);require(!g.has_original_kit(5),"France initially reports missing source kit");
 g.load_cd_graphics(argv[2]);g.draw(renderer);SDL_RenderPresent(renderer);require(g.has_original_kit(5) && g.has_original_kit(1)==g.has_euro96_kits(),"Kit availability reports actual imported assets");
 const bool stadium_available=std::filesystem::exists(std::filesystem::path(argv[2])/"GRAPHICS/STADIUM.BIN");
 require(g.has_cd_stadium()==stadium_available,"Matched CD stadium assets load when available");
 if(stadium_available)require(red_band_pixels()<old_red/5,"Completed CD stadium frame removes the solid red stadium band");
 save(surface,captures,"stadium-corrected-auto.bmp");
 g.match_camera=2;g.change_camera(0);g.draw(renderer);SDL_RenderPresent(renderer);save(surface,captures,"stadium-corrected-follow.bmp");
 require(hash(surface->pixels,surface->pitch*surface->h)!=substitute,"Imported kits/crowd change actual rendered pixels");
 for(int team=0;team<16;++team){s.next_home=team;s.next_away=(team+3)%16;s.start_match(true);
  for(int camera=0;camera<3;++camera){g.match_camera=camera;g.change_camera(0);g.draw(renderer);SDL_RenderPresent(renderer);require(g.ground_pixels()>1000 && g.scene_counts.stands==4 && g.scene_counts.goal_parts==8,"All countries/cameras render textured pitch, stadium and goals");}
  for(int side=0;side<2;++side){const int id=side?s.away_team:s.home_team;if(!kits.has(id))continue;
   auto crowd=kits.part(id,side?7:6);for(int i=0;i<8;++i)for(int channel=0;channel<3;++channel)require(g.palette_color((side?232:224)+i)[channel]==crowd[i*3+channel],"Crowd palette follows actual country rather than substitute uniform");
  }
 }
 s.next_home=5;s.next_away=6;s.start_match(true);g.match_camera=0;g.change_camera(0);
 for(int i=0;i<180;++i)g.advance(.05);g.draw(renderer);SDL_RenderPresent(renderer);save(surface,captures,"graphics-france-germany.bmp");
 g.overview=true;g.angle=.7f;g.draw(renderer);SDL_RenderPresent(renderer);save(surface,captures,"graphics-stadium.bmp");g.overview=false;
 // Close-up original pose, showing an imported kit with the original number atlas.
 g.action_view=false;g.angle=-.65f;g.draw(renderer);SDL_RenderPresent(renderer);save(surface,captures,"graphics-france-kit.bmp");g.action_view=true;
 s.start_goal_review();g.angle=-.65f;g.draw(renderer);SDL_RenderPresent(renderer);save(surface,captures,"graphics-goal-close.bmp");
 g.paused=true;g.draw(renderer);SDL_RenderPresent(renderer);const auto still=hash(surface->pixels,surface->pitch*surface->h);g.draw(renderer);SDL_RenderPresent(renderer);require(hash(surface->pixels,surface->pitch*surface->h)==still,"Repeated paused rendering is stable");g.paused=false;
 s.next_home=4;s.next_away=12;s.start_match(true);int rendered=0;
 for(int i=0;i<20000 && s.running;++i){g.advance(.05);if(g.replaying)g.finish_replay();if(i%37==0){g.draw(renderer);SDL_RenderPresent(renderer);++rendered;}}
 require(s.full_time && s.blue_goals==1 && s.red_goals==0 && s.ticks==7328,"Full CPU match result/timing unchanged by graphics");
 std::cout<<"CD kits "<<count<<"; 16 country pairs / 3 cameras; CPU "<<s.blue_goals<<':'<<s.red_goals<<" ticks "<<s.ticks<<" rendered frames "<<rendered<<'\n';
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();std::cout<<"Original graphics environment contracts passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';SDL_Quit();return 1;}}
