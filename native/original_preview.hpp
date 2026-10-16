#pragma once
#include "archive.hpp"
#include <array>
namespace actua {
// Retail FAP codec; compressed media is retained, only the selected loop is decoded.
class OriginalPreview {
public:
 explicit OriginalPreview(const std::filesystem::path&);
 static std::vector<Bytes> decode(const Bytes&,int width,int height);
 bool select(int slot);
 void advance(double seconds);
 const Bytes& pixels()const{return rgba_;}
 int width()const{return width_;} int height()const{return height_;}
 int frame()const{return frame_;}
 int count()const{return int(entries_.size());}
 const std::vector<Bytes>& frames()const{return frames_;}
 bool select_entry(int index);
private:
 struct Entry {int width,height;Bytes palette,data;};
 std::vector<Entry> entries_;std::array<unsigned short,328> mapping_{};
 std::vector<Bytes> frames_;Bytes rgba_;
 int selected_=-1,width_=0,height_=0,frame_=0;double clock_=0;
 void colour();
};
}
