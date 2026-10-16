#pragma once
#include "original_actions.hpp"
#include "original_camera.hpp"
#include "original_crowd.hpp"
#include <SDL3/SDL.h>
#include <array>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>
namespace actua {
// Native mixer/device boundary. Sound selection and amplitudes come from AUDIO.OBJ.
class OriginalAudio {
public:
    explicit OriginalAudio(const std::filesystem::path& root,bool open_device=true);
    ~OriginalAudio();
    OriginalAudio(const OriginalAudio&)=delete;
    OriginalAudio& operator=(const OriginalAudio&)=delete;
    void close();
    void update(OriginalActions&,const CameraPose&,bool active,bool tail=false);
    void set_muted(bool);
    int effects_volume()const{return int(effects_gain_*100+.5f);}
    int crowd_volume()const{return int(crowd_gain_*100+.5f);}
    void set_levels(int effects,int crowd){effects_gain_=std::clamp(effects,0,100)/100.0f;crowd_gain_=std::clamp(crowd,0,100)/100.0f;}
    void set_duck(bool enabled){duck_=enabled?.45f:1.0f;}
    bool muted()const{return muted_;}
    bool loaded()const{return loaded_;}
    bool device_ready()const{return stream_!=nullptr;}
    const std::string& status()const{return status_;}
    void pitch(int id,float x,float y,const CameraPose&);
    void crowd(int id);
    void mix(float* stereo,int frames);
    std::size_t voice_count()const{return voices_.size();}
    struct Stats {std::array<unsigned,20> pitch{};std::array<unsigned,11> crowd{};unsigned clipped=0;};
    std::array<int,2> moods()const{return moods_;}
    const Stats& stats()const{return stats_;}
private:
    struct Sample {std::vector<float> pcm;int rate=0;};
    struct Voice {int sample=0;double position=0,step=0;float left=0,right=0;int age=0;bool reaction=false;};
    std::array<Sample,20> samples_;
    std::vector<Voice> voices_;
    SDL_AudioStream* stream_=nullptr;
    bool loaded_=false,muted_=false,active_=false,observed_=false,ambience_=false,retail_=false;
    std::string status_="Original audio data unavailable";
    Stats stats_;
    float duck_=1,effects_gain_=1,crowd_gain_=1;
    unsigned random_=0x19661996;
    OriginalCrowd crowd_state_;
    std::array<int,2> moods_{1,1};
    std::array<double,6> ambience_positions_{};
    std::array<float,6> ambience_gains_{},ambience_targets_{};
    int mood_tick_=-1,miss_timer_=0;
    int ticks_=0,goals_=0,saves_=0,cards_=0,missed_=0;
    std::uint64_t generation_=0;
    void start(int sample,float gain,float pan,double frequency,bool reaction);
    void pump();
};
}
