#include "original_music.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 require(argc==3,"Expected media and scratch");require(SDL_Init(SDL_INIT_AUDIO),SDL_GetError());
 const std::filesystem::path media=argv[1],scratch=argv[2];
 const auto legacy=scratch/"legacy";std::filesystem::create_directories(legacy/"DIGI/MUSIC");
 for(const char* name:{"MENU.WAV","WIN.WAV","LOSE.WAV","DRAW.WAV"})std::filesystem::copy_file(media/"DIGI/MUSIC"/name,legacy/"DIGI/MUSIC"/name,std::filesystem::copy_options::overwrite_existing);
 using Track=OriginalMusic::Track;
 SDL_AudioSpec spec{};Uint8* raw=nullptr;Uint32 size=0;
 require(SDL_LoadWAV((media/"DIGI/MUSIC/EURO96.WAV").string().c_str(),&spec,&raw,&size),"Retail theme is present");
 require(spec.freq==22050 && spec.channels==2 && spec.format==SDL_AUDIO_U8 && size>8192,"Retail PCM contract");
 const std::vector<unsigned char> pcm(raw,raw+size);SDL_free(raw);const auto frames=pcm.size()/2;
 {
  OriginalMusic music(media,false),reference(media,false);require(music.uses_retail_theme(),"Euro media selects retail policy");
  std::array<float,8192> output{},expected{};
  for(auto result:{Track::win,Track::lose,Track::draw}){
   music.select(Track::none);reference.select(Track::none);music.select(result);reference.select(Track::menu);
   require(music.track()==Track::menu && music.cursor()==0,"Every outcome resumes Euro theme from start after a match");
   music.mix(output.data(),4096);reference.mix(expected.data(),4096);
   require(output==expected,"Result PCM is identical to the menu theme");
   for(int frame=1102;frame<4096;++frame)for(int ch=0;ch<2;++ch)require(output[frame*2+ch]==(float(pcm[frame*2+ch])-128)/256.0f,"Result uses original Euro WAV samples");
   // Cross the entire song boundary, repeatedly requesting the same result as
   // the real frame loop does. It must neither restart nor become a sting.
   for(std::size_t mixed=4096;mixed<frames+4096;mixed+=4096){
    const auto cursor=music.cursor();music.select(result);require(music.cursor()==cursor,"Repeated result selection preserves position");
    music.mix(output.data(),4096);reference.mix(expected.data(),4096);require(output==expected && !music.ended(),"Euro result loops exactly with the frontend theme");
   }
   const auto cursor=music.cursor();music.select(Track::menu);require(music.cursor()==cursor,"Result-to-menu transition continues the same song");
  }
  music.set_volume(0);music.mix(output.data(),4096);for(float v:output)require(v==0,"Zero music volume is silent");
  music.select(Track::none);music.mix(output.data(),4096);require(music.cursor()==0,"Pitch has no music playback");for(float v:output)require(v==0,"Match silence");
  OriginalMusic old(legacy,false);require(!old.uses_retail_theme(),"Missing Euro track keeps compatibility policy");
  for(auto result:{Track::win,Track::lose,Track::draw}){
   old.select(result);require(old.track()==result && old.available(result),"Compatibility outcome selects its own imported sting");
   double energy=0;for(int i=0;i<200 && !old.ended();++i){old.mix(output.data(),4096);for(float v:output)energy+=std::abs(v);}
   require(old.ended() && energy>1,"Compatibility sting plays audibly once");old.mix(output.data(),4096);for(float v:output)require(v==0,"Finished sting does not restart");
  }
  OriginalMusic absent(scratch/"missing",false);absent.select(Track::win);absent.mix(output.data(),4096);for(float v:output)require(v==0,"Absent music remains silent");
  OriginalMusic device(media);require(device.ready(),"Dummy audio stream opens");device.select(Track::lose);device.update(true);const auto cursor=device.cursor();device.update(false);require(device.cursor()==cursor,"Focus/movie mute holds result theme position");device.update(true);
 }
 SDL_Quit();std::cout<<"Retail result music: three outcomes, full-track PCM and looping, menu continuity, legacy stings, absent media, volume and focus passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
