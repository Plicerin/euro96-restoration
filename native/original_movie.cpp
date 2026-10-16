#include "original_movie.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <stdexcept>
namespace actua {
namespace {
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
std::uint32_t read32(std::ifstream& f){unsigned char b[4]{};require(bool(f.read(reinterpret_cast<char*>(b),4)),"Truncated CD movie header");return std::uint32_t(b[0])|(std::uint32_t(b[1])<<8)|(std::uint32_t(b[2])<<16)|(std::uint32_t(b[3])<<24);}
std::uint64_t read64(std::ifstream& f){const auto low=read32(f),high=read32(f);return low|(std::uint64_t(high)<<32);}
}
void OriginalMovie::start(bool intro){
 stop();intro_=intro;path_=root_/"VIDEO"/(intro?"INTRO.AMV":"MAKING.AMV");
 try{
  const auto size=std::filesystem::file_size(path_);require(size>=52 && size<=512ull*1024*1024,"Invalid CD movie size");
  file_.open(path_,std::ios::binary);char magic[8]{};file_.read(magic,8);require(!std::memcmp(magic,"ACTMOV1\0",8),"Unsupported CD movie");
  const auto width=read32(file_),height=read32(file_),num=read32(file_),den=read32(file_),frames=read32(file_),rate=read32(file_),audio=read32(file_);
  require(width==320 && height==240 && num==12 && den==1 && frames>=2 && frames<=4000 && rate==22050 && audio==frames*1837,"Invalid CD movie metadata");
  frames_=int(frames);offsets_.resize(frames+1);for(auto& n:offsets_)n=read64(file_);
  require(offsets_[0]==36+8ull*(frames+1)+audio && offsets_.back()==size,"Invalid CD movie offsets");
  for(std::size_t i=1;i<offsets_.size();++i)require(offsets_[i]>offsets_[i-1] && offsets_[i]-offsets_[i-1]<=256*1024,"Invalid CD movie frame bounds");
  audio_.resize(audio);require(bool(file_.read(reinterpret_cast<char*>(audio_.data()),audio)),"Truncated CD soundtrack");
  channels_=1;open_audio();
  active_=true;position_=0;paused_=false;
 }catch(...){stop();throw;}
}
void OriginalMovie::open_audio(){if(device_ && SDL_InitSubSystem(SDL_INIT_AUDIO)){const SDL_AudioSpec spec{SDL_AUDIO_U8,channels_,22050};stream_=SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,&spec,nullptr,nullptr);}}
void OriginalMovie::start_trophy(int film){
 stop();intro_=false;path_=root_/"VIDEO/EURO_CUP.BIN";
 try{
  require(film>=0 && film<32,"Invalid Euro96 trophy country/kit");const auto size=std::filesystem::file_size(path_);
  require(size>=268 && size<=1024ull*1024*1024,"Invalid Euro96 trophy size");file_.open(path_,std::ios::binary);
  char magic[8]{};file_.read(magic,8);require(!std::memcmp(magic,"EU96CUP1",8) && read32(file_)==32,"Invalid Euro96 trophy header");
  file_.seekg(12+film*8);const auto entry=read64(file_);require(entry>=268 && entry<=size && size-entry>=16,"Invalid Euro96 trophy entry");
  file_.seekg(std::streamoff(entry));const auto frames=read32(file_),audio=read32(file_);const auto pcm=read64(file_);
  require(frames==983 && audio==frames*1837*2 && size-entry-16>=frames*12ull && pcm>=268 && pcm<=size && audio<=size-pcm,"Invalid Euro96 trophy metadata");
  frames_=int(frames);offsets_.resize(frames);sizes_.resize(frames);
  for(unsigned i=0;i<frames;++i){offsets_[i]=read64(file_);sizes_[i]=read32(file_);require(offsets_[i]>=268 && offsets_[i]<=size && sizes_[i]>=33 && sizes_[i]<=256*1024 && sizes_[i]<=size-offsets_[i],"Invalid Euro96 trophy frame bounds");}
  audio_.resize(audio);file_.seekg(std::streamoff(pcm));require(bool(file_.read(reinterpret_cast<char*>(audio_.data()),audio)),"Truncated Euro96 trophy PCM");
  channels_=2;open_audio();active_=true;position_=0;paused_=false;
 }catch(...){stop();throw;}
}
void OriginalMovie::silence(){if(stream_){SDL_PauseAudioStreamDevice(stream_);SDL_ClearAudioStream(stream_);}audio_cursor_=std::min(audio_.size(),std::size_t(position_*22050)*channels_);}
void OriginalMovie::stop(){
 if(stream_)SDL_DestroyAudioStream(stream_);stream_=nullptr;
 if(texture_)SDL_DestroyTexture(texture_);texture_=nullptr;
 file_.close();file_.clear();audio_.clear();offsets_.clear();sizes_.clear();active_=paused_=false;position_=0;frames_=0;uploaded_=-1;audio_cursor_=0;channels_=1;
}
int OriginalMovie::frame()const{return active_?std::clamp(int(position_*12),0,frames_-1):0;}
void OriginalMovie::seek(double seconds){if(!active_ || !std::isfinite(seconds))return;position_=std::clamp(seconds,0.0,(frames_-1)/12.0);uploaded_=-1;silence();}
void OriginalMovie::update(double seconds,bool focused,bool audible,int volume){
 if(!active_)return;
 if(!focused)paused_=true;
 if(!paused_ && std::isfinite(seconds))position_+=std::clamp(seconds,0.0,.25);
 if(position_>=duration()){stop();return;}
 if(!stream_)return;
 if(paused_ || !focused || !audible){silence();return;}
 SDL_SetAudioStreamGain(stream_,std::clamp(volume,0,100)/100.0f);
 const auto now=std::min(audio_.size(),std::size_t(position_*22050)*channels_);
 // Keep queued PCM within 100 ms of the presentation clock; seek/focus loss
 // discard stale audio rather than playing it after a return to the title.
 const int queued=SDL_GetAudioStreamQueued(stream_);
 if(queued<0){SDL_DestroyAudioStream(stream_);stream_=nullptr;return;}
 const auto played=audio_cursor_>=std::size_t(queued)?audio_cursor_-std::size_t(queued):0;
 if(std::abs(double(played)-double(now))>2205*channels_){silence();}
 const auto end=std::min(audio_.size(),now+2205*channels_);
 if(end>audio_cursor_){
  if(!SDL_PutAudioStreamData(stream_,audio_.data()+audio_cursor_,int(end-audio_cursor_))){SDL_DestroyAudioStream(stream_);stream_=nullptr;return;}
  audio_cursor_=end;
 }
 if(!SDL_ResumeAudioStreamDevice(stream_)){SDL_DestroyAudioStream(stream_);stream_=nullptr;}
}
SDL_Surface* OriginalMovie::read_frame(int index){
 require(active_ && index>=0 && index<frames_,"CD movie frame outside recording");
 std::vector<unsigned char> png(sizes_.empty()?std::size_t(offsets_[index+1]-offsets_[index]):sizes_[index]);file_.clear();file_.seekg(std::streamoff(offsets_[index]));
 require(bool(file_.read(reinterpret_cast<char*>(png.data()),png.size())),"Truncated CD movie frame");
 const unsigned char signature[]={137,80,78,71,13,10,26,10};
 require(png.size()>=33 && !std::memcmp(png.data(),signature,8) && !std::memcmp(png.data()+12,"IHDR",4),"Invalid CD movie PNG");
 const auto be=[&](int n){return (std::uint32_t(png[n])<<24)|(std::uint32_t(png[n+1])<<16)|(std::uint32_t(png[n+2])<<8)|png[n+3];};
 require(be(16)==320 && be(20)==240,"Unexpected CD movie frame dimensions");
 auto* image=SDL_LoadPNG_IO(SDL_IOFromConstMem(png.data(),png.size()),true);require(image!=nullptr,SDL_GetError());return image;
}
void OriginalMovie::draw(SDL_Renderer* r){
 if(!active_)return;
 if(uploaded_!=frame()){
  auto* image=read_frame(frame());auto* next=SDL_CreateTextureFromSurface(r,image);SDL_DestroySurface(image);require(next!=nullptr,SDL_GetError());
  if(texture_)SDL_DestroyTexture(texture_);texture_=next;uploaded_=frame();SDL_SetTextureScaleMode(texture_,SDL_SCALEMODE_NEAREST);
 }
 // Present the complete 4:3 film at full height, with no restoration toolbar.
 // Lower captions stay within the image; unused side margins remain black.
 constexpr float height=664, width=height*4/3;
 SDL_SetRenderDrawColor(r,0,0,0,255);SDL_RenderClear(r);const SDL_FRect area{(960-width)/2,0,width,height};
 require(SDL_RenderTexture(r,texture_,nullptr,&area),SDL_GetError());
}
}
