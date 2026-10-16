#pragma once
#include "original_actions.hpp"
#include <SDL3/SDL.h>
#include <array>
#include <deque>
#include <filesystem>
#include <vector>
namespace actua {
// Original phrase IDs and PCM; native scheduling/device boundary, no gameplay RNG.
class OriginalCommentary {
public:
 explicit OriginalCommentary(const std::filesystem::path&,bool device=true,int language=0);
 ~OriginalCommentary();
 OriginalCommentary(const OriginalCommentary&)=delete;
 OriginalCommentary& operator=(const OriginalCommentary&)=delete;
 void update(OriginalActions&,double elapsed,bool audible);
 void request(int category,int number=-1);
 void score(int home,int away);
 bool welcome(int home,int away,int fixture=-1,bool original_draw=false);
 void mix(float* stereo,int frames);
 void set_volume(int percent);
 void cycle_volume(){set_volume(volume_?volume_-25:100);}
 int volume()const{return volume_;}
 int language()const{return language_;}
 bool set_language(int); // Atomic swap; missing/corrupt packs leave the current voice intact.
 bool loaded()const{return !clips_.empty();}
 bool ready()const{return stream_!=nullptr;}
 bool speaking()const{return current_!=-1;}
 int category()const{return current_==-2?36:current_<0?0:clips_[current_].category;}
 std::size_t pending()const{return pending_.size();}
 struct Played {int category,number;double time;int variant=0,bank=0;};
 struct Stats {unsigned requests=0,dropped=0,interrupted=0;std::array<unsigned,512> played{};};
 const Stats& stats()const{return stats_;}
 const std::vector<Played>& history()const{return history_;}
private:
 std::filesystem::path root_;
 bool device_=true;
 bool retail_events_=false;
 int language_=0;
 void open_device();
 struct Clip {int category,number;std::vector<unsigned char> pcm;int variant=0;};
 struct Pending {int category,number;double expires;int bank;};
 std::vector<Clip> clips_;
 std::array<std::vector<int>,512> categories_;
 std::vector<unsigned char> introduction_;
 struct Fixture {int number=0,home=255,away=255;};
 std::array<Fixture,31> fixtures_{};
 std::deque<Pending> pending_;
 std::deque<int> recent_;
 std::vector<Played> history_;
 std::array<double,512> last_{};
 SDL_AudioStream* stream_=nullptr;
 int current_=-1,volume_=75,ticks_=0;
 std::size_t cursor_=0;
 unsigned random_=0x1996ac96;
 std::uint64_t generation_=0;
 bool observed_=false,enabled_=false;
 double clock_=0,next_=0;
 Stats stats_;
 enum class Pack {voice,names,periods,sequences};
 void load(const std::filesystem::path&,Pack);
 bool introduction(const std::vector<int>& clips,int number,int bank);
 void enqueue(int category,int number,int bank);
 void period(int category,const OriginalActions&);
 static int priority(int);
 void clear();
 void start_next();
 void pump();
};
}
