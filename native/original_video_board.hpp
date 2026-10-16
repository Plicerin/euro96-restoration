#pragma once
#include "archive.hpp"
#include <array>
#include <memory>
namespace actua {
class OriginalActions;
// Bounded native implementation of 3DENG.C init_anim/do_anim delta streams.
class OriginalVideoBoard {
public:
 explicit OriginalVideoBoard(const std::filesystem::path& source);
 enum Event {idle,extra_time,foul,full_time,half_time,penalty,goal,yellow_card,red_card};
 void load_retail(const std::filesystem::path& root);
 void use_retail(bool enabled);
 bool retail_frame()const{return retail_clip_>=0;}
 const std::array<unsigned char,768>& retail_palette()const{return retail_->palette;}
 void play(Event event);
 void advance(double seconds);
 void observe(const OriginalActions&);
 int event()const{return event_;}
 unsigned revision()const{return revision_;}
 const std::array<unsigned char,10240>& pixels()const{return pixels_;}
 const std::array<unsigned char,48>& colors()const{return colors_[event_];}
 static std::size_t delta(std::span<const unsigned char>,std::size_t,std::span<unsigned char>);
private:
 struct RetailClip {int event,record;Bytes frames;};
 struct Retail {std::array<unsigned char,768> palette{};std::vector<RetailClip> clips;};
 std::shared_ptr<const Retail> retail_;
 int retail_clip_=-1,retail_frame_=0,cards_=0;
 unsigned variant_=0;
 bool retail_enabled_=false;
 std::array<Bytes,7> streams_;
 std::array<std::array<unsigned char,48>,7> colors_{};
 std::array<unsigned char,10240> pixels_{};
 std::size_t cursor_=0;
 unsigned revision_=0;
 int event_=0,remaining_=0,tick_=-1,goals_=0,fouls_=0,mode_=0,interval_=0;
 std::uint64_t generation_=~std::uint64_t(0);
 bool ended_=false;
 double time_=0;
 void first_frame();
 void copy_retail_frame();
};
}
