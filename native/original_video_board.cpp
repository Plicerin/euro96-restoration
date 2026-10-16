#include "original_video_board.hpp"
#include "original_actions.hpp"
#include <algorithm>
#include <stdexcept>
namespace actua {
void OriginalVideoBoard::load_retail(const std::filesystem::path& root){
 const auto path=root/"GRAPHICS/EURO_BOARDS.BIN";
 if(!std::filesystem::exists(path)){retail_.reset();play(idle);return;}
 if(std::filesystem::file_size(path)>16*1024*1024)throw std::runtime_error("Oversized retail board pack");
 const auto bytes=read_file(path);std::size_t at=8;
 const auto word=[&](){if(at+4>bytes.size())throw std::runtime_error("Truncated retail board pack");unsigned n=0;for(int i=0;i<4;++i)n|=unsigned(bytes[at++])<<(8*i);return n;};
 if(bytes.size()<780 || std::string(bytes.begin(),bytes.begin()+8)!="EU96BRD1" || word()!=10)throw std::runtime_error("Invalid retail board header");
 auto loaded=std::make_shared<Retail>();std::copy_n(bytes.begin()+at,768,loaded->palette.begin());at+=768;
 if(*std::max_element(loaded->palette.begin(),loaded->palette.end())>63)throw std::runtime_error("Invalid retail board palette");
 constexpr int events[]{0,2,2,4,4,6,7,7,8,8},records[]{0,3,4,10,11,1,14,15,8,9};
 for(int i=0;i<10;++i){const auto event=word(),record=word(),count=word();
  if(event!=events[i] || record!=records[i] || count<1 || count>255 || std::size_t(count)*10240>bytes.size()-at)throw std::runtime_error("Invalid retail board frames");
  const auto end=at+std::size_t(count)*10240;loaded->clips.push_back({int(event),int(record),Bytes(bytes.begin()+at,bytes.begin()+end)});at=end;
 }
 if(at!=bytes.size())throw std::runtime_error("Trailing retail board data");
 retail_=std::move(loaded);play(idle);
}
void OriginalVideoBoard::use_retail(bool enabled){if(retail_enabled_!=enabled){retail_enabled_=enabled;play(idle);}}
void OriginalVideoBoard::copy_retail_frame(){const auto& frames=retail_->clips[retail_clip_].frames;std::copy_n(frames.begin()+std::size_t(retail_frame_)*10240,10240,pixels_.begin());++revision_;}
std::size_t OriginalVideoBoard::delta(std::span<const unsigned char> data,std::size_t at,std::span<unsigned char> frame){
 std::size_t pos=0;
 const auto byte=[&](){if(at>=data.size())throw std::runtime_error("Truncated stadium animation");return data[at++];};
 while(const auto run=byte()){
  if(run>data.size()-at || run>frame.size()-pos)throw std::runtime_error("Stadium animation run outside frame");
  std::copy_n(data.begin()+at,run,frame.begin()+pos);pos+=run;at+=run;
  const auto skip=byte();if(skip>frame.size()-pos)throw std::runtime_error("Stadium animation skip outside frame");pos+=skip;
 }
 return at;
}
OriginalVideoBoard::OriginalVideoBoard(const std::filesystem::path& source){
 Archive fap(source/"FAP.DAT",source/"FAP.OFF");
 constexpr int records[]{0,4,6,8,10,12,14};
 for(int e=0;e<7;++e){
  auto data=fap.entry(records[e]),pal=fap.entry(records[e]+1);
  if(data.size()<10241 || data.size()>280000 || pal.size()!=48 || *std::max_element(pal.begin(),pal.end())>63)throw std::runtime_error("Invalid original stadium animation");
  std::array<unsigned char,10240> frame;std::copy_n(data.begin(),10240,frame.begin());std::size_t at=10240;
  while(at<data.size() && data[at])at=delta(data,at,frame);
  if(at+1!=data.size())throw std::runtime_error("Invalid stadium animation terminator");
  streams_[e].assign(data.begin(),data.end());std::copy(pal.begin(),pal.end(),colors_[e].begin());
 }
 play(idle);
}
void OriginalVideoBoard::first_frame(){std::copy_n(streams_[event_].begin(),10240,pixels_.begin());cursor_=10240;++revision_;}
void OriginalVideoBoard::play(Event event){
 if(event<idle || event>red_card)throw std::out_of_range("Stadium event");
 retail_clip_=-1;retail_frame_=0;
 if(retail_enabled_ && retail_){
  std::vector<int> choices;for(int i=0;i<int(retail_->clips.size());++i)if(retail_->clips[i].event==event)choices.push_back(i);
  if(!choices.empty()){
   retail_clip_=choices[variant_%choices.size()];if(event!=idle)++variant_;
   constexpr int limits[]{0,200,100,200,300,300,400,200,200};
   event_=event;remaining_=limits[event];time_=0;copy_retail_frame();return;
  }
 }
 // Older installations keep their original foul animation for bookings.
 if(event==yellow_card || event==red_card)event=foul;
 constexpr int limits[]{0,200,100,200,300,100,200};event_=event;remaining_=limits[event];time_=0;first_frame();
}
void OriginalVideoBoard::advance(double seconds){
 if(retail_frame()){
  time_+=std::clamp(seconds,0.0,0.1);
  while(event_ && time_>=.05){time_-=.05;if(--remaining_<=0){play(idle);break;}
   retail_frame_=(retail_frame_+1)%int(retail_->clips[retail_clip_].frames.size()/10240);copy_retail_frame();
  }return;
 }
 // Original 200 Hz timer threshold count > time+10; use 11 timer units.
 time_+=std::clamp(seconds,0.0,0.1);
 while(event_ && time_>=.055){time_-=.055;
  if(!streams_[event_][cursor_])first_frame();
  else if(--remaining_<=0){play(idle);break;}
  else{cursor_=delta(streams_[event_],cursor_,pixels_);++revision_;}
 }
}
void OriginalVideoBoard::observe(const OriginalActions& s){
 const auto& e=s.engine;
 if(tick_<0 || s.ticks<tick_ || (generation_!=s.generation && s.ticks<=1)){variant_=0;play(idle);goals_=e.goal_events;fouls_=e.foul_events;cards_=e.card_events;mode_=e.match_mode;interval_=0;ended_=false;}
 if(s.full_time && !ended_)play(full_time);
 // FOOTBALL.CPP uses half-time again at 105; only the 90-minute break
 // announces the start of extra time.
 else if(s.interval_ticks && !interval_)play(s.interval_minute==90?extra_time:half_time);
 else if(e.goal_events>goals_)play(goal);
 else if(retail_enabled_ && retail_ && e.card_events>cards_ && e.card_player>=1 && e.card_player<=22)play(e.teams[e.card_player-1].tm_book>=5?red_card:yellow_card);
 else if((e.match_mode==17 || e.match_mode==18) && mode_!=e.match_mode)play(penalty);
 else if(e.foul_events>fouls_)play(foul);
 generation_=s.generation;tick_=s.ticks;goals_=e.goal_events;fouls_=e.foul_events;cards_=e.card_events;mode_=e.match_mode;interval_=s.interval_ticks;ended_=s.full_time;
}
}
