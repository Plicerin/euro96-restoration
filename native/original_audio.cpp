#include "original_audio.hpp"
#include "audio_data.hpp"
#include "crowd_mapping.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <map>
#include <stdexcept>
namespace actua {
namespace {
constexpr int rate=48000;
unsigned read32(const std::vector<unsigned char>& b,std::size_t p){
    if(p+4>b.size())throw std::runtime_error("Truncated pitch bank");
    return unsigned(b[p]) | unsigned(b[p+1])<<8 | unsigned(b[p+2])<<16 | unsigned(b[p+3])<<24;
}
}
OriginalAudio::OriginalAudio(const std::filesystem::path& root,bool open_device){
    retail_=std::filesystem::exists(root/"DIGI/PITCH/EURO_PITCH.MAD");
    const auto path=root/(retail_?"DIGI/PITCH/EURO_PITCH.MAD":"DIGI/PITCH/PITCH.MAD");
    if(!std::filesystem::exists(path))return;
    const auto size=std::filesystem::file_size(path);
    if(size<24 || size>16*1024*1024)throw std::runtime_error("Invalid pitch bank size");
    std::vector<unsigned char> bytes(static_cast<std::size_t>(size));
    std::ifstream file(path,std::ios::binary);
    if(!file.read(reinterpret_cast<char*>(bytes.data()),std::streamsize(size)))throw std::runtime_error("Cannot read pitch bank");
    const auto directory=read32(bytes,16);
    if(directory<24 || directory>size || directory%24)throw std::runtime_error("Invalid pitch directory");
    std::map<std::string,std::pair<unsigned,unsigned>> entries;
    unsigned end=directory;
    for(unsigned p=0;p<directory;p+=24){
        std::string name;
        for(unsigned i=0;i<16 && bytes[p+i];++i){char c=char(bytes[p+i]);name+=c>='a' && c<='z'?char(c-'a'+'A'):c;}
        const auto offset=read32(bytes,p+16),length=read32(bytes,p+20);
        if(name.empty() || offset!=end || offset>size || length>size-offset || !length || !entries.emplace(name,std::pair{offset,length}).second)
            throw std::runtime_error("Invalid or duplicate pitch entry");
        end=offset+length;
    }
    if(end!=size)throw std::runtime_error("Trailing pitch bank data");
    for(std::size_t i=0;i<samples_.size();++i){
        auto it=entries.find(audio_data::names[i]);
        if(it==entries.end())throw std::runtime_error("Missing original pitch sample");
        auto [offset,length]=it->second;SDL_AudioSpec spec{};Uint8* pcm=nullptr;Uint32 count=0;
        if(!SDL_LoadWAV_IO(SDL_IOFromConstMem(bytes.data()+offset,length),true,&spec,&pcm,&count))throw std::runtime_error(SDL_GetError());
        if(spec.format!=SDL_AUDIO_U8 || spec.channels!=1 || spec.freq!=11025 || !count){SDL_free(pcm);throw std::runtime_error("Unsupported original pitch WAV format");}
        auto& sample=samples_[i];sample.rate=spec.freq;sample.pcm.resize(count);
        for(unsigned j=0;j<count;++j)sample.pcm[j]=(float(pcm[j])-128)/128;
        SDL_free(pcm);
    }
    loaded_=true;status_=retail_?"Euro 96 match sounds ready":"Original match sounds ready";
    if(open_device){
        if(SDL_InitSubSystem(SDL_INIT_AUDIO)){
            const SDL_AudioSpec spec{SDL_AUDIO_F32,2,rate};
            stream_=SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,&spec,nullptr,nullptr);
        }
        if(!stream_)status_=std::string("Audio device unavailable: ")+SDL_GetError();
    }
}
OriginalAudio::~OriginalAudio(){close();}
void OriginalAudio::close(){if(stream_)SDL_DestroyAudioStream(stream_);stream_=nullptr;voices_.clear();}
void OriginalAudio::set_muted(bool value){
    muted_=value;
    if(value){voices_.clear();if(stream_){SDL_PauseAudioStreamDevice(stream_);SDL_ClearAudioStream(stream_);}}
}
void OriginalAudio::start(int sample,float gain,float pan,double frequency,bool reaction){
    if(!loaded_ || muted_)return;
    // One crowd reaction at a time; repeated net/ball contacts can still overlap.
    if(reaction && std::any_of(voices_.begin(),voices_.end(),[&](const Voice& v){return v.reaction && v.sample==sample;}))return;
    if(voices_.size()>=32)voices_.erase(voices_.begin());
    pan=std::clamp(pan,-1.0f,1.0f);
    voices_.push_back({sample,0,samples_[sample].rate*frequency/rate,
        gain*(pan>0?1-pan:1),gain*(pan<0?1+pan:1),0,reaction});
}
void OriginalAudio::pitch(int id,float x,float y,const CameraPose& camera){
    if(id<1 || id>19 || !loaded_ || muted_)return;
    const auto& mapping=audio_data::PitchBank[id-1];
    float pan=0; // Native screen-relative stereo; avoids MY_AUDIO.CPP's y-camera_x typo.
    float attenuation=1;
    if(mapping.flags&1){
        const float dx=x-camera.x,dy=y-camera.y,depth=std::hypot(camera.tx-camera.x,camera.ty-camera.y);
        const float distance=std::max(1.0f,std::hypot(std::hypot(dx,dy),camera.z));
        if(depth>.001f)pan=(dy*(camera.tx-camera.x)-dx*(camera.ty-camera.y))/(depth*distance);
        attenuation=std::min(1.0f,600.0f/distance);
    }
    random_^=random_<<13;random_^=random_>>17;random_^=random_<<5;
    const int frequency=mapping.freq+((mapping.flags&2)?int(random_%1024)-512:0);
    start(mapping.sample,float(mapping.amp)/32767*attenuation,pan,double(frequency)/32768,false);
    ++stats_.pitch[id];
}
void OriginalAudio::crowd(int id){
    if(id<1 || id>10 || !loaded_ || muted_)return;
    const auto& mapping=audio_data::CrowdBank[id-1];
    // FOOT.CPP default CrowdVolume=0x1fff; PitchVolume=0x7fff.
    // Retail CrowdBank has the same ten mappings with a quieter foul reaction.
    // Verified executable field: file offset 0x1424a4 (import_euro96_audio.py).
    const int amplitude=retail_ && id==10?25600:mapping.amp;
    start(mapping.sample,float(amplitude)/32767*(8191.0f/32767),0,double(mapping.freq)/32768,true);
    ++stats_.crowd[id];
}
void OriginalAudio::mix(float* stereo,int frames){
    std::fill_n(stereo,std::size_t(frames)*2,0.0f);
    if(muted_ || !loaded_)return;
    for(int frame=0;frame<frames;++frame){
        float left=0,right=0;
        if(ambience_){
            for(int bank=1;bank<=6;++bank){
                auto& gain=ambience_gains_[bank-1];const float target=ambience_targets_[bank-1];
                gain+=std::clamp(target-gain,-1.0f/24000,1.0f/24000);
                const auto& mapping=audio_data::CrowdBank[bank-1];const auto& s=samples_[mapping.sample];
                auto& position=ambience_positions_[bank-1];const auto i=std::size_t(position);const float t=float(position-i);
                const float value=(s.pcm[i]*(1-t)+s.pcm[(i+1)%s.pcm.size()]*t)*gain*(float(mapping.amp)/32767)*(8191.0f/32767);
                left+=value*crowd_gain_;right+=value*crowd_gain_;position+=double(s.rate)/rate;if(position>=s.pcm.size())position-=s.pcm.size();
            }
        }
        for(auto& voice:voices_){
            const auto& s=samples_[voice.sample];if(voice.position>=s.pcm.size())continue;
            const auto i=std::size_t(voice.position);const float t=float(voice.position-i);
            const float envelope=std::min({1.0f,float(voice.age)/96,float((s.pcm.size()-voice.position)/voice.step)/96});
            const float value=(s.pcm[i]*(1-t)+s.pcm[std::min(i+1,s.pcm.size()-1)]*t)*envelope;
            const float gain=voice.reaction?crowd_gain_:effects_gain_;
            left+=value*voice.left*gain;right+=value*voice.right*gain;voice.position+=voice.step;++voice.age;
        }
        if(std::abs(left)>1 || std::abs(right)>1)++stats_.clipped;
        stereo[frame*2]=std::clamp(left,-1.0f,1.0f)*duck_;stereo[frame*2+1]=std::clamp(right,-1.0f,1.0f)*duck_;
    }
    std::erase_if(voices_,[&](const Voice& v){return v.position>=samples_[v.sample].pcm.size();});
}
void OriginalAudio::pump(){
    if(!stream_)return;
    // Keep 40 ms queued, independent of render rate; never accumulate stale audio.
    const int queued=SDL_GetAudioStreamQueued(stream_);
    if(queued<0){status_=SDL_GetError();close();return;}
    const int frames=std::clamp(1920-queued/int(2*sizeof(float)),0,1920);
    if(frames){std::array<float,3840> output{};mix(output.data(),frames);
        if(!SDL_PutAudioStreamData(stream_,output.data(),frames*int(2*sizeof(float)))){status_=SDL_GetError();close();return;}}
    if(!SDL_ResumeAudioStreamDevice(stream_)){status_=SDL_GetError();close();}
}
void OriginalAudio::update(OriginalActions& s,const CameraPose& camera,bool active,bool tail){
    auto& e=s.engine;
    const bool reset=observed_ && (s.ticks<ticks_ || (s.generation!=generation_ && s.ticks<=1));
    if(reset){crowd_state_={};mood_tick_=-1;miss_timer_=0;ambience_gains_={};voices_.clear();if(stream_)SDL_ClearAudioStream(stream_);observed_=false;}
    if(active && !muted_){
        for(unsigned i=0;i<e.sound_count;++i){const auto& event=e.sound_events[i];pitch(event.id,event.x,event.y,camera);}
        if(observed_){
            if(e.goal_events>goals_)crowd(5);
            else if(s.saves>saves_)crowd(8);
            else if(e.card_events>cards_)crowd(10);
            else if(e.near_miss_events>missed_)crowd(7);
        }
    }
    if(s.ticks!=mood_tick_ || e.goal_events!=goals_ || s.saves!=saves_ || e.card_events!=cards_ || e.near_miss_events!=missed_){
        if(observed_){
            if(e.near_miss_events>missed_ || (e.shot_missed<0 && miss_timer_>=0))crowd_state_.shot_missed=e.shot_missed;
            if(s.saves>saves_)crowd_state_.shot_saved=std::max(1,int(e.shot_saved));
            if(e.card_events>cards_)crowd_state_.card_is_out=std::max(1,int(e.card_is_out));
        }
        const auto old=moods_;moods_=crowd_state_.update(e,s.ticks<40);
        if(active && !muted_ && observed_ && (moods_[0]==9 || moods_[1]==9) && old!=moods_)crowd(9);
        ambience_targets_={};for(const int mood:moods_){const int bank=crowd_ambience_bank.at(mood);if(bank)ambience_targets_[bank-1]+=.5f;}
        mood_tick_=s.ticks;miss_timer_=e.shot_missed;
    }
    e.sound_count=0;
    goals_=e.goal_events;saves_=s.saves;cards_=e.card_events;missed_=e.near_miss_events;
    ticks_=s.ticks;generation_=s.generation;observed_=true;
    ambience_=active && s.team_play && !s.full_time;
    if((!active && !tail) || muted_){
        voices_.clear();
        if(stream_ && active_){SDL_PauseAudioStreamDevice(stream_);SDL_ClearAudioStream(stream_);}
    }else pump();
    active_=(active || tail) && !muted_;
}
}
