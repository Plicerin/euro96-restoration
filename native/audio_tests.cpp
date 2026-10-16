#include "original_audio.hpp"
#include "original_graphics.hpp"
#include "audio_data.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <numeric>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
    require(argc==3,"Expected SOURCE and audio paths");
    require(SDL_Init(SDL_INIT_AUDIO),SDL_GetError());
    {
        OriginalAudio missing(std::filesystem::path(argv[2])/"absent");require(!missing.loaded(),"Missing assets must allow silent play");
        OriginalAudio audio(argv[2],false);require(audio.loaded(),"Load actual disc samples");
        require(audio_data::PitchBank[0].sample==10 && audio_data::PitchBank[2].sample==19 && audio_data::PitchBank[3].sample==18,"Original whistle mappings");
        require(audio_data::PitchBank[4].sample==15 && audio_data::PitchBank[6].sample==12 && audio_data::CrowdBank[4].sample==4,"Original kick/goal mappings");
        CameraPose camera{};std::vector<float> pcm(48000*2);
        for(int id=1;id<=19;++id){
            audio.set_muted(true);audio.set_muted(false);audio.pitch(id,640,400,camera);audio.mix(pcm.data(),48000);
            require(std::any_of(pcm.begin(),pcm.end(),[](float v){return std::abs(v)>.001f;}),"Every mapped effect must yield audible PCM");
            require(std::all_of(pcm.begin(),pcm.end(),[](float v){return std::isfinite(v)&&std::abs(v)<=1;}),"Bounded finite mixer output");
        }
        audio.set_muted(true);audio.set_muted(false);audio.pitch(7,1000,400,camera);audio.mix(pcm.data(),48000);
        double left_energy=0,right_energy=0;for(int i=0;i<48000;++i){left_energy+=std::abs(pcm[i*2]);right_energy+=std::abs(pcm[i*2+1]);}
        require(right_energy>left_energy*1.2,"Screen-right contact pans right");
        audio.pitch(7,640,400,camera);audio.pitch(7,640,400,camera);require(audio.voice_count()==2,"Simultaneous effects overlap");
        std::srand(1234);const int expected=std::rand();std::srand(1234);audio.pitch(7,300,400,camera);require(std::rand()==expected,"Audio must not consume gameplay RNG");
        audio.set_muted(true);audio.pitch(1,0,0,camera);audio.mix(pcm.data(),48000);
        require(audio.voice_count()==0 && std::all_of(pcm.begin(),pcm.end(),[](float v){return v==0;}),"Mute clears voices and emits silence");audio.set_muted(false);
        OriginalGraphics graphics(argv[1]);auto& s=*graphics.sequence;auto& e=s.engine;
        s.start_match(true);audio.update(s,camera,true);
        e.play_fx(5,320,400);e.play_fx(13,650,400);const auto ball=e.ballx;const auto random_seed=e.seed;
        const auto before=audio.stats();audio.update(s,camera,true);
        require(audio.stats().pitch[5]==before.pitch[5]+1 && audio.stats().pitch[13]==before.pitch[13]+1,"Multiple events in one update must survive");
        require(e.ballx==ball && e.seed==random_seed && e.sound_count==0,"Audio drains events without changing simulation");
        e.prediction=1;e.play_fx(7,320,400);require(e.sound_count==0,"Predictions cannot emit audio");e.prediction=0;
        for(int i=0;i<140;++i)e.play_fx(7,320,400);
        require(e.sound_count==128 && e.sound_dropped==12,"Event storage stays bounded");audio.update(s,camera,false);
        require(e.sound_count==0 && audio.voice_count()==0,"Inactive view drains old events");
        audio.update(s,camera,true);++e.goal_events;audio.update(s,camera,true);const auto goals=audio.stats().crowd[5];
        audio.update(s,camera,true);require(goals && audio.stats().crowd[5]==goals,"Goal cheer fires once");
        ++s.saves;audio.update(s,camera,true);require(audio.stats().crowd[8],"Original save applause");
        ++e.card_events;audio.update(s,camera,true);require(audio.stats().crowd[10],"Original crowd foul reaction");
        e.ballx=e.pitch_len+1;e.bally=float(e.cntspot_y);e.ballz=0;e.shot_passed();audio.update(s,camera,true);require(audio.stats().crowd[7],"Original near-miss reaction");
        const auto misses=audio.stats().crowd[7];e.shot_passed();audio.update(s,camera,true);
        require(audio.stats().crowd[7]==misses+1,"Repeated near misses survive unchanged legacy timer values");
        s.start_match(true);audio.update(s,camera,true);require(audio.stats().crowd[5]==goals,"Restart never replays an old goal");
        OriginalAudio device(argv[2]);require(device.device_ready(),"SDL dummy playback device");
        device.update(s,camera,true);e.play_fx(7,640,400);device.update(s,camera,true);
        device.update(s,camera,false);require(!device.voice_count(),"Pause/focus loss flush device voices");
        device.set_muted(true);device.update(s,camera,true);require(!device.voice_count(),"Muted device stays empty");device.close();
        // One complete original CPU match exercises actual whistles, kicks, bounces and cheers.
        s.start_match(true);OriginalAudio live(argv[2],false);live.update(s,camera,true);
        for(int tick=0;tick<20000 && s.running;++tick){s.step();live.update(s,camera,true);live.mix(pcm.data(),2400);}
        require(s.full_time,"Audio-enabled CPU match reaches full time");
        const auto& stats=live.stats();
        require(stats.pitch[2] && stats.pitch[3] && stats.pitch[4],"Actual restart, half-time and full-time whistles");
        require(stats.pitch[5]+stats.pitch[6]+stats.pitch[7]>0 && stats.pitch[13]+stats.pitch[14]>0,"Actual contact and bounce sounds");
        require(e.sound_dropped==0,"No event loss during CPU match");
        std::cout<<"CPU audio match "<<s.blue_goals<<':'<<s.red_goals<<" ticks "<<s.ticks<<" pitch events "<<std::accumulate(stats.pitch.begin(),stats.pitch.end(),0u)<<" goal cheers "<<stats.crowd[5]<<" clipped frames "<<stats.clipped<<'\n';
    }
    SDL_Quit();std::cout<<"Original audio contracts passed\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';SDL_Quit();return 1;}}
