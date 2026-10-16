#include "original_graphics.hpp"
#include "original_actions.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
using namespace actua;
static void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 require(argc==2,"Expected data directory");require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
 auto* surface=SDL_CreateSurface(960,700,SDL_PIXELFORMAT_RGBA32);
 auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 {
 OriginalGraphics g(argv[1]);g.action_view=true;auto& s=*g.sequence;auto& e=s.engine;
 s.start_match(true);require(!g.start_replay(),"Empty recording rejected");
 for(int i=0;i<480;++i)g.advance(.05);
 require(g.replay_frames()==300,"Original 15-second ring is bounded to 300 frames");
 g.paused=true;g.match_camera=2;require(g.start_replay(),"Recorded replay opens from pause");
 const int tick=s.ticks;const auto count=g.replay_frames();
 // Object representation comparison catches changes to every scalar and container
 // descriptor in the live engine; replay renders use the separate engine copy.
 std::array<unsigned char,sizeof(e)> bytes;std::memcpy(bytes.data(),&e,sizeof(e));
 auto unchanged=[&](){require(s.ticks==tick && g.replay_frames()==count && std::memcmp(bytes.data(),&e,sizeof(e))==0,"Replay must not mutate live engine or recording");};
 for(int speed=1;speed<=5;++speed){
  g.seek_replay(10);g.change_replay_speed(speed-g.replay_speed());g.toggle_replay_pause();
  for(int n=0;n<50;++n)g.advance(.05);
  require(g.replay_position()>=10+speed*10-1 && g.replay_position()<=10+speed*10,"Original speed/5 playback timing");unchanged();
 }
 g.seek_replay(100);require(g.paused && g.presentation_alpha()==1,"Seek pauses on exact frame");
 for(int n=0;n<20;++n)g.advance(.1);require(g.replay_position()==100,"Paused replay does not move");
 g.step_replay(-1);require(g.replay_position()==99,"Backward frame");g.step_replay(1);require(g.replay_position()==100,"Forward frame");
 g.step_replay(-20);require(g.replay_position()==80,"One-second rewind");
 g.seek_replay(-500);require(g.replay_position()==0,"Seek clamps at oldest frame");
 g.seek_replay(5000);require(g.replay_position()==299,"Seek clamps at latest frame");
 g.change_replay_speed(100);require(g.replay_speed()==5,"Maximum original replay speed");
 g.toggle_replay_pause();g.advance(.05);require(g.replaying && g.paused && !g.replay_position(),"Manual end rewinds and pauses like original menu");
 g.toggle_replay_loop();g.seek_replay(299);g.toggle_replay_pause();g.advance(.05);
 require(g.replaying && !g.paused && !g.replay_position(),"Opt-in loop wraps without resuming match");
 g.seek_replay(100);
 for(int camera=0;camera<5;++camera){g.match_camera=camera;g.change_camera(0);g.draw(renderer);require(g.polygon_count()>0,"All replay cameras render while paused");unchanged();}
 // Seeking away and back must not retain interpolation from the unrelated frame.
 g.match_camera=1;g.seek_replay(70);g.draw(renderer);
 std::vector<unsigned char> pixels(static_cast<unsigned char*>(surface->pixels),static_cast<unsigned char*>(surface->pixels)+surface->pitch*surface->h);
 g.seek_replay(210);g.draw(renderer);g.seek_replay(70);g.draw(renderer);
 require(std::memcmp(pixels.data(),surface->pixels,pixels.size())==0,"Repeated paused seek renders the same frame without stale blends");unchanged();
 g.finish_replay();require(g.paused && g.match_camera==2 && s.ticks==tick,"Exit restores live camera and prior pause");
 g.paused=false;g.advance(.05);require(s.ticks==tick+1,"Resume advances exactly one live tick");
 require(g.start_replay(true),"Automatic replay starts");for(int n=0;n<350 && g.replaying;++n)g.advance(.05);
 require(!g.replaying && !g.paused,"Automatic goal replay still returns to live play");
 require(g.start_replay(),"Manual replay restarts");g.seek_replay(42);g.finish_replay();
 s.start_match(true);require(!g.start_replay(),"New match rejects previous generation's recording");
 std::cout<<"Replay speed, exact stepping, bounded seeking, looping, five cameras, deterministic redraw, live-state isolation and automatic return passed.\n";
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
