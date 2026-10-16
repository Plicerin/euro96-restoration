#include "original_movie.hpp"
#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace actua;
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
static std::uint64_t hash(SDL_Surface* source){
 auto* rgba=SDL_ConvertSurface(source,SDL_PIXELFORMAT_RGBA32);require(rgba,SDL_GetError());
 std::uint64_t result=1469598103934665603ull;
 for(int y=0;y<rgba->h;++y)for(int x=0;x<rgba->w*4;++x){result^=static_cast<unsigned char*>(rgba->pixels)[y*rgba->pitch+x];result*=1099511628211ull;}
 SDL_DestroySurface(rgba);return result;
}
int main(int argc,char** argv){try{
 require(argc==3,"Expected media and test directories");require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO),SDL_GetError());
 const std::filesystem::path root=argv[2];std::filesystem::create_directories(root/"VIDEO");
 {
 OriginalMovie movie(argv[1]);movie.start();require(movie.frames()==2869 && movie.audio_bytes()==5270353 && std::abs(movie.duration()-239.083333)<.00001,"Exact CD duration and soundtrack size");
 const std::array<int,5> frames{0,144,840,2160,2868};
 const std::array<std::uint64_t,5> hashes{7506888735744058243ull,7474355702564562895ull,1220142624948587039ull,12184047216624586459ull,8259515088564907111ull};
 for(int n=0;n<movie.frames();++n){auto* image=movie.read_frame(n);require(image->w==320 && image->h==240,"Every original movie frame decodes");
  for(int i=0;i<5;++i)if(n==frames[i])require(hash(image)==hashes[i],"Decoded pixels match independent importer samples");
  if(n==840)require(SDL_SavePNG(image,(root/"original-frame.png").string().c_str()),SDL_GetError());SDL_DestroySurface(image);
 }
 movie.update(.05,true,true,100);require(movie.queued_audio()>0,"Original PCM queued on SDL device");
 movie.update(.05,true,false,100);require(movie.queued_audio()==0 && movie.position()>.05,"Mute silences without stopping video");
 movie.seek(70);require(movie.frame()==840,"Seek selects native 12 Hz frame");
 movie.toggle_pause();const auto at=movie.position();movie.update(.2,true,true,100);require(movie.position()==at && movie.queued_audio()==0,"Pause freezes and clears audio");
 movie.toggle_pause();movie.update(.05,false,true,100);require(movie.paused() && movie.position()==at,"Focus loss pauses");
 movie.update(.05,true,true,100);require(movie.position()==at,"Focus return remains paused");
 movie.seek(-100);require(movie.frame()==0,"Seek clamps at start");movie.seek(900);require(movie.frame()==2868,"Seek clamps at end");
 movie.toggle_pause();movie.update(.2,true,true,100);require(!movie.active() && !movie.queued_audio(),"End closes playback and audio");
 movie.start();movie.stop();require(!movie.active(),"Repeated entry and skip clean up");
 // Tiny malformed files exercise fail-closed headers before any allocation or PNG decode.
 for(int mode=0;mode<3;++mode){
  std::array<unsigned char,80> bad{};if(mode){std::copy_n(reinterpret_cast<const unsigned char*>("ACTMOV1\0"),8,bad.begin());bad[8]=0xff;bad[9]=0xff;}
  std::ofstream f(root/"VIDEO/MAKING.AMV",std::ios::binary|std::ios::trunc);f.write(reinterpret_cast<const char*>(bad.data()),mode==2?10:bad.size());f.close();
  OriginalMovie broken(root,false);bool rejected=false;try{broken.start();}catch(const std::exception&){rejected=true;}
  require(rejected && !broken.active(),"Malformed movie rejected without active playback");
 }
 std::cout<<"All 2869 CD frames decoded; five independent pixel hashes, original audio, seek/pause/mute/focus/end and malformed-file checks passed.\n";
 }
 SDL_Quit();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
