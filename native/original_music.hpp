#pragma once
#include <SDL3/SDL.h>
#include <filesystem>
#include <array>
#include <algorithm>
#include <vector>
#include <cstddef>
namespace actua {
class OriginalMusic {
public:
 enum class Track {none,menu,win,lose,draw};
 explicit OriginalMusic(const std::filesystem::path&,bool device=true);
 ~OriginalMusic();
 OriginalMusic(const OriginalMusic&)=delete;
 OriginalMusic& operator=(const OriginalMusic&)=delete;
 void select(Track);
 void update(bool audible);
 void set_duck(bool enabled){duck_=enabled?.25f:1.0f;}
 void mix(float* stereo,int frames);
 int volume()const{return int(gain_*100+.5f);}
 void set_volume(int percent){gain_=std::clamp(percent,0,100)/100.0f;}
 void close();
 Track track()const{return track_;}
 bool ready()const{return stream_!=nullptr;}
 bool available(Track t)const{return t!=Track::none && !samples_[int(t)-1].empty();}
 std::size_t cursor()const{return cursor_;}
 bool ended()const{return ended_;}
 bool uses_retail_theme()const{return retail_theme_;}
private:
 std::array<std::vector<unsigned char>,4> samples_;
 SDL_AudioStream* stream_=nullptr;
 Track track_=Track::none;
 std::size_t cursor_=0;
 bool ended_=false;
 bool retail_theme_=false;
 float duck_=1,gain_=1;
};
}
