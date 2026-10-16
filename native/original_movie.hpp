#pragma once
#include <SDL3/SDL.h>
#include <filesystem>
#include <fstream>
#include <vector>
namespace actua {
// Lossless imported CD frames and PCM; no external player or gameplay state.
class OriginalMovie {
public:
 explicit OriginalMovie(std::filesystem::path root,bool device=true):root_(std::move(root)),path_(root_/"VIDEO/MAKING.AMV"),device_(device){}
 ~OriginalMovie(){stop();}
 bool available()const{return std::filesystem::exists(path_);}
 void start(bool intro=false);
 void start_trophy(int film);
 int channels()const{return channels_;}
 bool intro()const{return intro_;}
 void stop();
 void update(double seconds,bool focused,bool audible,int volume);
 void seek(double seconds);
 void toggle_pause(){if(active_)paused_=!paused_;}
 void draw(SDL_Renderer*);
 SDL_Surface* read_frame(int frame);
 bool active()const{return active_;}
 bool paused()const{return paused_;}
 double position()const{return position_;}
 double duration()const{return frames_/12.0;}
 int frame()const;
 int frames()const{return frames_;}
 std::size_t audio_bytes()const{return audio_.size();}
 int queued_audio()const{return stream_?SDL_GetAudioStreamQueued(stream_):0;}
private:
 std::filesystem::path root_,path_;
 bool intro_=false;
 std::ifstream file_;
 std::vector<std::uint64_t> offsets_;
 std::vector<std::uint32_t> sizes_;
 std::vector<unsigned char> audio_;
 SDL_Texture* texture_=nullptr;
 SDL_AudioStream* stream_=nullptr;
 bool device_=true,active_=false,paused_=false;
 double position_=0;
 int frames_=0,uploaded_=-1;
 int channels_=1;
 std::size_t audio_cursor_=0;
 void silence();
 void open_audio();
};
}
