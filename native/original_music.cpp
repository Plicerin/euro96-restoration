#include "original_music.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace actua {
OriginalMusic::OriginalMusic(const std::filesystem::path& root,bool device){
 retail_theme_=std::filesystem::exists(root/"DIGI/MUSIC/EURO96.WAV");
 const char* names[]={"MENU","WIN","LOSE","DRAW"};
 for(int i=0;i<4;++i){
  const auto path=root/"DIGI/MUSIC"/(i==0 && retail_theme_?std::string("EURO96.WAV"):std::string(names[i])+".WAV");
  if(!std::filesystem::exists(path))continue;
  if(std::filesystem::file_size(path)>32*1024*1024)throw std::runtime_error("Music file too large");
  SDL_AudioSpec spec{};Uint8* pcm=nullptr;Uint32 count=0;auto u8=path.u8string();
  if(!SDL_LoadWAV(reinterpret_cast<const char*>(u8.c_str()),&spec,&pcm,&count))throw std::runtime_error(SDL_GetError());
  if(spec.freq!=22050 || spec.channels!=2 || spec.format!=SDL_AUDIO_U8 || !count || count%2){SDL_free(pcm);throw std::runtime_error("Unsupported original music format");}
  samples_[i].assign(pcm,pcm+count);SDL_free(pcm);
 }
 if(device && SDL_InitSubSystem(SDL_INIT_AUDIO)){
  const SDL_AudioSpec spec{SDL_AUDIO_F32,2,22050};
  stream_=SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,&spec,nullptr,nullptr);
 }
}
OriginalMusic::~OriginalMusic(){close();}
void OriginalMusic::close(){if(stream_)SDL_DestroyAudioStream(stream_);stream_=nullptr;}
void OriginalMusic::select(Track track){
 // The retail match returns to the frontend's EURO96 theme handler. Outcome
 // tracks are compatibility requests, resolved here before comparing cursors.
 if(retail_theme_ && (track==Track::win || track==Track::lose || track==Track::draw))track=Track::menu;
 if(track==track_)return;track_=track;cursor_=0;ended_=false;
 if(stream_){SDL_PauseAudioStreamDevice(stream_);SDL_ClearAudioStream(stream_);}
}
void OriginalMusic::mix(float* stereo,int frames){
 std::fill_n(stereo,std::size_t(frames)*2,0.0f);
 if(!available(track_) || ended_)return;
 const auto& sample=samples_[int(track_)-1];const auto length=sample.size()/2;
 for(int i=0;i<frames;++i){
  if(cursor_>=length){if(track_==Track::menu)cursor_=0;else{ended_=true;break;}}
  const float ramp=std::min({1.0f,float(cursor_)/1102,float(length-cursor_)/1102});
  for(int ch=0;ch<2;++ch)stereo[i*2+ch]=(float(sample[cursor_*2+ch])-128)/128*.5f*ramp*duck_*gain_;
  ++cursor_;
 }
}
void OriginalMusic::update(bool audible){
 if(!stream_)return;
 if(!audible || !available(track_)){SDL_PauseAudioStreamDevice(stream_);SDL_ClearAudioStream(stream_);return;}
 if(ended_)return; // Let the queued end of a result sting drain.
 const int queued=SDL_GetAudioStreamQueued(stream_);
 if(queued<0){close();return;}
 const int frames=std::clamp(882-queued/int(2*sizeof(float)),0,882);
 if(frames){std::array<float,1764> output{};mix(output.data(),frames);
  if(!SDL_PutAudioStreamData(stream_,output.data(),frames*int(2*sizeof(float)))){close();return;}}
 if(!SDL_ResumeAudioStreamDevice(stream_))close();
}
}
