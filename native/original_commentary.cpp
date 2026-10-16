#include "original_commentary.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <set>
#include <stdexcept>
namespace actua {
namespace {
unsigned read(const std::vector<unsigned char>& b,std::size_t& p,int n){
 if(p+std::size_t(n)>b.size())throw std::runtime_error("Truncated commentary pack");
 unsigned result=0;for(int i=0;i<n;++i)result|=unsigned(b[p++])<<(8*i);return result;
}
}
OriginalCommentary::OriginalCommentary(const std::filesystem::path& root,bool device,int language):root_(root),device_(device),language_(language){
 if(language<0 || language>1)throw std::runtime_error("Unsupported commentary language");
 last_.fill(-1000);
 load(root/(language?"COMMENTARY/FRENCH.BIN":std::filesystem::exists(root/"COMMENTARY/EURO_EVENTS.BIN")?"COMMENTARY/EURO_EVENTS.BIN":std::filesystem::exists(root/"COMMENTARY/EURO_ENGLISH.BIN")?"COMMENTARY/EURO_ENGLISH.BIN":"COMMENTARY/ENGLISH3.BIN"),Pack::voice);
 // A names-only pack must never masquerade as complete commentary.
 if(loaded())load(root/(language?"COMMENTARY/FRENCH_NAMES.BIN":std::filesystem::exists(root/"COMMENTARY/EURO_NAMES.BIN")?"COMMENTARY/EURO_NAMES.BIN":"COMMENTARY/NAMES.BIN"),Pack::names);
 if(loaded() && !language){load(root/"COMMENTARY/EURO_PERIODS.BIN",Pack::periods);load(root/"COMMENTARY/EURO_SEQUENCES.BIN",Pack::sequences);}
 open_device();
}
void OriginalCommentary::open_device(){
 if(device_ && !stream_ && loaded() && SDL_InitSubSystem(SDL_INIT_AUDIO)){
  const SDL_AudioSpec spec{SDL_AUDIO_F32,2,11025};stream_=SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,&spec,nullptr,nullptr);
 }
}
bool OriginalCommentary::set_language(int language){
 if(language==language_)return loaded();
 try{
  OriginalCommentary next(root_,false,language);if(!next.loaded())return false;
  clear();clips_.swap(next.clips_);categories_.swap(next.categories_);fixtures_.swap(next.fixtures_);language_=language;
  retail_events_=next.retail_events_;
  introduction_.clear();recent_.clear();last_.fill(-1000);next_=clock_;
  // Retain observation of the match: changing voice must not replay kickoff.
  open_device();return true;
 }catch(const std::exception& error){SDL_Log("Commentary language unavailable: %s",error.what());return false;}
}
void OriginalCommentary::load(const std::filesystem::path& path,Pack pack){
 if(!std::filesystem::exists(path))return;
 const auto size=std::filesystem::file_size(path);if(size<12 || size>64*1024*1024)throw std::runtime_error("Invalid commentary pack size");
 std::vector<unsigned char> bytes(static_cast<std::size_t>(size));std::ifstream file(path,std::ios::binary);
 if(!file.read(reinterpret_cast<char*>(bytes.data()),std::streamsize(size)))throw std::runtime_error("Invalid commentary pack");
 const std::string magic(bytes.begin(),bytes.begin()+8);
 const bool names=pack==Pack::names,periods=pack==Pack::periods,sequences=pack==Pack::sequences;
 const bool variants=magic=="AC96COM2" && pack==Pack::voice;
 if(sequences?magic!="AC96SEQ1":periods?magic!="AC96PER1":magic!="AC96COM1" && !variants)throw std::runtime_error("Invalid commentary pack");
 std::size_t p=8;const auto count=read(bytes,p,4);if(count>1000)throw std::runtime_error("Too many commentary clips");
 if(sequences){
  std::set<int> numbers;
  for(int i=0;i<31;++i){auto& f=fixtures_[i];f={int(read(bytes,p,1)),int(read(bytes,p,1)),int(read(bytes,p,1))};
   if(f.number<1 || f.number>31 || !numbers.insert(f.number).second || (i<24?(f.home>=16 || f.away>=16 || f.home==f.away):(f.home!=255 || f.away!=255)))throw std::runtime_error("Invalid introduction fixture table");
  }
  if(!count)throw std::runtime_error("Empty sequence pack");
 }
 std::set<std::pair<int,int>> unique;
 for(unsigned i=0;i<count;++i){
  const int category=int(read(bytes,p,2)),encoded=int(read(bytes,p,2));const auto length=read(bytes,p,4);
  const bool score_record=category==44 || (periods && (category==96 || category==97));
  const bool encoded_variant=(variants && category==44) || (periods && score_record);
  const int variant=encoded_variant?encoded/100:0;
  const int number=encoded_variant?encoded%100:encoded;
  const bool valid_category=sequences?(category>=99 && category<=103 && number<=(category==99?31:16)):periods?((category>=96 && category<=98 && (category!=98 || number<=13)) || (category==104 && number<=32)):names?(category>=128 && category<448 && number>=1 && number<=3):((category>=1 && category<=36)||category==42||category==43||category==44||(category>=64 && category<96));
  if(!valid_category || variant>3 || number<(score_record?0:1) || number>99 || (score_record && (number/10>7 || number%10>5 || number/10<number%10)) || !length || length>1024*1024 || length>bytes.size()-p || !unique.emplace(category,encoded).second)throw std::runtime_error("Invalid commentary record");
  SDL_AudioSpec spec{};Uint8* pcm=nullptr;Uint32 frames=0;
  if(!SDL_LoadWAV_IO(SDL_IOFromConstMem(bytes.data()+p,length),true,&spec,&pcm,&frames))throw std::runtime_error(SDL_GetError());
  if(spec.freq!=11025 || spec.channels!=1 || spec.format!=SDL_AUDIO_U8 || !frames){SDL_free(pcm);throw std::runtime_error("Unsupported commentary PCM");}
  const int public_id=sequences?(category<=100?36:43):periods?(category==96?42:43):category;
  categories_[category].push_back(int(clips_.size()));clips_.push_back({public_id,number,{pcm,pcm+frames},variant});SDL_free(pcm);p+=length;
 }
 if(p!=bytes.size())throw std::runtime_error("Trailing commentary pack data");
 if(pack==Pack::voice)retail_events_=path.filename()=="EURO_EVENTS.BIN";

}
OriginalCommentary::~OriginalCommentary(){if(stream_)SDL_DestroyAudioStream(stream_);}
int OriginalCommentary::priority(int id){if(id>=128)return 0;return id==44?3:id>=42?4:id>=22 && id<=29?3:id>=30?2:id<=12?1:0;}
void OriginalCommentary::clear(){current_=-1;cursor_=0;pending_.clear();if(stream_){SDL_PauseAudioStreamDevice(stream_);SDL_ClearAudioStream(stream_);}}
void OriginalCommentary::set_volume(int percent){volume_=std::clamp(percent,0,100);if(!volume_)clear();}
void OriginalCommentary::score(int home,int away){
 if(home<0 || home>7 || away<0 || away>5)return;
 request(44,std::max(home,away)*10+std::min(home,away));
}
void OriginalCommentary::request(int category,int number){
 // Retail has_ball uses CPA for ordinary passes at every distance. Its
 // special set-piece branches clear comsetp and skip the commentary call.
 // Keep this revision policy at the audio boundary; source gameplay and
 // the older English/French banks retain their original event identities.
 if(retail_events_){
  if(category==18){++stats_.requests;++stats_.dropped;return;}
  if(category==14)category=13;
 }
 // Supplementary banks are private; source event IDs must not address them.
 if(category>=96 && category<=104){++stats_.requests;++stats_.dropped;return;}
 enqueue(category,number,category);
}
void OriginalCommentary::period(int category,const OriginalActions& s){
 // Retail tied 90-minute knockout branch: XTM(score+1), then extra time.
 // This is an interval announcement, not a match result or winner sequence.
 if(category==43 && s.full_match && !s.full_time && !s.shootout &&
    s.interval_ticks && s.interval_minute==90 && s.engine.need_result &&
    s.blue_goals==s.red_goals && s.blue_goals>=0){
  int number=s.blue_goals+1;
  if(s.blue_goals>8){
   std::vector<int> available;
   for(int i:categories_[104])if(clips_[i].number>=9)available.push_back(clips_[i].number);
   if(!available.empty()){
    random_^=random_<<13;random_^=random_>>17;random_^=random_<<5;
    number=available[random_%available.size()];
   }else number=-1;
  }
  if(number>0 && std::any_of(categories_[104].begin(),categories_[104].end(),[&](int i){return clips_[i].number==number;}))enqueue(43,number,104);
  // Without verified XTM audio, retain the whistle-only transition. A generic
  // final-result recording would incorrectly announce that this match is over.
  return;
 }
 const bool sequences=!categories_[99].empty();
 if(category==43 && sequences && s.full_match && s.national_teams){
  const int a=s.shootout?s.penalty_scores[0]:s.blue_goals,b=s.shootout?s.penalty_scores[1]:s.red_goals;
  const int winner=a>b?s.home_team:s.away_team;
  // Retail precedence: championship winner, shootout winner, extra-time winner.
  const int bank=s.tournament_fixture==30?101:s.shootout?102:s.engine.match_half>=2?103:0;
  if(a>=0 && b>=0 && a!=b && winner>=0 && winner<16 && bank &&
     std::any_of(categories_[bank].begin(),categories_[bank].end(),[&](int i){return clips_[i].number==winner+1;})){
   enqueue(43,winner+1,bank);return;
  }
 }
 const int high=std::max(s.blue_goals,s.red_goals),low=std::min(s.blue_goals,s.red_goals);
 // Without the sequence supplement, keep the previous generic knockout calls.
 // With it, an ordinary decisive knockout result also uses the retail FLL bank.
 const bool ordinary=s.full_match && !s.shootout && s.engine.match_half<2 && s.engine.golden_goal!=2 &&
     (category==42 || s.match_format==OriginalActions::MatchFormat::regulation || (sequences && high!=low));
 const int bank=category==42?96:high==low?98:97;
 const int number=bank==98?-1:high*10+low;
 if(ordinary && low>=0 && (bank==98 || (high<=7 && low<=5)) && !categories_[bank].empty() &&
    (number<0 || std::any_of(categories_[bank].begin(),categories_[bank].end(),[&](int i){return clips_[i].number==number;})))enqueue(category,number,bank);
 else request(category);
}
void OriginalCommentary::enqueue(int category,int number,int bank){
 ++stats_.requests;
 if(!enabled_ || !volume_ || category<1 || category>=512 || bank<1 || bank>=512 || categories_[bank].empty() || (category==44 && (number<0 || std::none_of(categories_[44].begin(),categories_[44].end(),[&](int i){return clips_[i].number==number;})))){++stats_.dropped;return;}
 if(category>=128 && (speaking() || !pending_.empty())){++stats_.dropped;return;}
 const int rank=priority(category);
 if(clock_-last_[category]<(rank>=3?2.0:rank==2?6.0:12.0) || this->category()==category || std::any_of(pending_.begin(),pending_.end(),[&](const Pending& v){return v.category==category;})){++stats_.dropped;return;}
 // Goals and period calls supersede lower-priority speech and discard obsolete reactions.
 if(rank>=3){
  std::erase_if(pending_,[&](const Pending& v){return priority(v.category)<rank;});
  if(speaking() && priority(this->category())<rank){++stats_.interrupted;current_=-1;if(stream_)SDL_ClearAudioStream(stream_);}
  next_=clock_;
 }
 if(pending_.size()>=4){++stats_.dropped;return;}
 pending_.push_back({category,number,clock_+(rank>=3?8.0:rank==2?3.0:1.5),bank});
}
void OriginalCommentary::start_next(){
 std::erase_if(pending_,[&](const Pending& v){if(v.expires<clock_){++stats_.dropped;return true;}return false;});
 if(speaking() || clock_<next_ || pending_.empty())return;
 const auto best=std::max_element(pending_.begin(),pending_.end(),[](const Pending& a,const Pending& b){return priority(a.category)<priority(b.category);});
 const int id=best->category,number=best->number,bank=best->bank;pending_.erase(best);
 std::vector<int> available;
 for(int clip:categories_[bank])if((number<0 || clips_[clip].number==number) && std::find(recent_.begin(),recent_.end(),clip)==recent_.end())available.push_back(clip);
 // Exact score/period recordings may recur; never substitute a different score to avoid repetition.
 if(available.empty() && id>=42)for(int clip:categories_[bank])if(number<0 || clips_[clip].number==number)available.push_back(clip);
 if(available.empty()){++stats_.dropped;return;}
 random_^=random_<<13;random_^=random_>>17;random_^=random_<<5;
 current_=available[random_%available.size()];cursor_=0;last_[id]=clock_;++stats_.played[id];
 recent_.push_back(current_);if(recent_.size()>64)recent_.pop_front();
 if(history_.size()<4096)history_.push_back({id,clips_[current_].number,clock_,clips_[current_].variant,bank});
}
bool OriginalCommentary::introduction(const std::vector<int>& clips,int number,int bank){
 if(clips.empty() || std::any_of(clips.begin(),clips.end(),[](int i){return i<0;}))return false;
 clear();introduction_.clear();
 for(int index:clips){const auto& data=clips_[index].pcm;introduction_.insert(introduction_.end(),data.begin(),data.end());}
 current_=-2;cursor_=0;last_[36]=clock_;++stats_.played[36];
 if(history_.size()<4096)history_.push_back({36,number,clock_,0,bank});return true;
}
bool OriginalCommentary::welcome(int home,int away,int fixture,bool original_draw){
 if(!enabled_ || !volume_ || home<0 || home>=16 || away<0 || away>=16)return false;
 // Retail welcome exits unless this is an original-draw championship.
 // Keep the old linked welcome only for French/legacy media installations.
 if(!categories_[99].empty() && (!original_draw || fixture<0 || fixture>=31))return false;
 if(original_draw && fixture>=0 && fixture<31 && !categories_[99].empty()){
  const auto& f=fixtures_[fixture];
  const auto clip=[&](int bank,int number){for(int i:categories_[bank])if(clips_[i].number==number)return i;return -1;};
  const bool same_pair=(home==f.home && away==f.away)||(home==f.away && away==f.home);
  const int opening=clip(99,f.number);
  if(fixture<24 && same_pair){
   if(opening>=0)return introduction({opening},f.number,99);
   // STT20 is absent on the verified disc. Its MCA fallback is absent too;
   // retail continues with the linked country fragments, without an ending.
   return introduction({clip(64+home,1),clip(80+away,1)},f.number,99);
  }
  if(fixture>=28 && opening>=0)return introduction({opening,clip(100,home+1),clip(80+away,1)},f.number,99);
  // Retail qualifier codes 16..23 resolve through its group-position table.
  // The four pairings match our bracket (quarters 1 and 4 reverse sides).
  // These STT recordings are complete introductions, without country suffixes.
  if(fixture>=24 && fixture<28 && opening>=0)return introduction({opening},f.number,99);
 }
 if(!enabled_ || !volume_ || home<0 || home>=16 || away<0 || away>=16 || categories_[64+home].empty() || categories_[80+away].empty() || categories_[36].empty())return false;
 std::vector<int> choices;for(int clip:categories_[36])if(std::find(recent_.begin(),recent_.end(),clip)==recent_.end())choices.push_back(clip);
 if(choices.empty())choices=categories_[36];
 random_^=random_<<13;random_^=random_>>17;random_^=random_<<5;
 const int ending=choices[random_%choices.size()];
 const bool started=introduction({categories_[64+home][0],categories_[80+away][0],ending},clips_[ending].number,36);
 if(started){recent_.push_back(ending);if(recent_.size()>64)recent_.pop_front();}return started;
}
void OriginalCommentary::mix(float* output,int frames){
 std::fill_n(output,std::size_t(frames)*2,0.0f);if(!enabled_ || !volume_ || !speaking())return;
 const auto& clip=current_==-2?introduction_:clips_[current_].pcm;
 for(int i=0;i<frames && cursor_<clip.size();++i,++cursor_){
  const float ramp=std::min({1.0f,float(cursor_)/55,float(clip.size()-cursor_)/55});
  const float value=(float(clip[cursor_])-128)/128*(float(volume_)/100)*ramp;
  output[i*2]=output[i*2+1]=value;
 }
 if(cursor_>=clip.size()){current_=-1;next_=clock_+1.0;}
}
void OriginalCommentary::pump(){
 if(!stream_)return;
 const int queued=SDL_GetAudioStreamQueued(stream_);if(queued<0){clear();return;}
 const int frames=std::clamp(441-queued/int(2*sizeof(float)),0,441);
 if(frames){std::array<float,882> output{};mix(output.data(),frames);if(!SDL_PutAudioStreamData(stream_,output.data(),frames*int(2*sizeof(float)))){clear();return;}}
 SDL_ResumeAudioStreamDevice(stream_);
}
void OriginalCommentary::update(OriginalActions& s,double elapsed,bool audible){
 auto& e=s.engine;clock_+=std::clamp(elapsed,0.0,.25);
 const bool reset=observed_ && (s.ticks<ticks_ || (s.generation!=generation_ && s.ticks<=1));
 if(reset){clear();last_.fill(-1000);next_=clock_;}
 enabled_=audible && loaded() && volume_>0;
 if(!enabled_)clear();
 else {
  if(s.full_match && !s.shootout && s.ticks<=2 && (!observed_ || reset))welcome(s.home_team,s.away_team,s.tournament_fixture,s.original_draw);
  for(unsigned i=0;i<e.commentary_count;++i){const int id=e.commentary_queue[i];if(id>=512 && id<632){
   const int value=id-512,side=value/60,shirt=(value%60)/3,tone=value%3;
   if(s.national_teams)request(128+(side?s.away_team:s.home_team)*20+shirt,tone+1);
  }else if(id>=256 && id<=331)request(44,id-256);else if(id==42 || id==43)period(id,s);else request(id);}
  start_next();pump();
 }
 e.commentary_count=0;observed_=true;ticks_=s.ticks;generation_=s.generation;
}
}
