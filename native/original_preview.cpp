#include "original_preview.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>
namespace actua {
namespace {
void require(bool ok){if(!ok)throw std::runtime_error("Invalid Euro96 preview media");}
unsigned u32(const Bytes& b,std::size_t at){require(at<=b.size() && b.size()-at>=4);return unsigned(b[at])|(unsigned(b[at+1])<<8)|(unsigned(b[at+2])<<16)|(unsigned(b[at+3])<<24);}
unsigned u16(const Bytes& b,std::size_t at){require(at<=b.size() && b.size()-at>=2);return unsigned(b[at])|(unsigned(b[at+1])<<8);}
}
std::vector<Bytes> OriginalPreview::decode(const Bytes& b,int w,int h){
 require(((w==140 && h==285)||(w==320 && h==240)) && b.size()>=1026 && b.size()<=16*1024*1024 && b.back()==0);
 std::vector<unsigned> sizes;std::size_t total=1025;bool ended=false;
 for(int i=0;i<256;++i){auto n=u32(b,i*4);if(!n)ended=true;else{require(!ended && n<=b.size()-std::min(total,b.size()));sizes.push_back(n);total+=n;}}
 require(ended && !sizes.empty() && total==b.size());
 Bytes pixels(w*h);std::vector<Bytes> frames;std::size_t at=1024;
 for(std::size_t i=0;i<sizes.size();++i){const auto end=at+sizes[i];std::size_t pos=0;unsigned kind=2;
  if(i){require(at<end);kind=b[at++];require(kind==1 || kind==2);}
  for(;;){require(at<end);auto run=b[at++];if(!run)break;
   require(run<=end-at && run<=pixels.size()-pos);std::copy_n(b.begin()+at,run,pixels.begin()+pos);at+=run;pos+=run;
   require(at<end);auto skip=b[at++];require(skip<=pixels.size()-pos);
   if(kind==2)std::fill_n(pixels.begin()+pos,skip,pixels[pos-1]);pos+=skip;
  }
  require(at==end && (i || pos==pixels.size()));frames.push_back(pixels);
 }
 return frames;
}
OriginalPreview::OriginalPreview(const std::filesystem::path& root){
 mapping_.fill(65535);const auto path=root/"GRAPHICS/EURO_PREVIEWS.BIN";if(!std::filesystem::exists(path))return;
 const auto size=std::filesystem::file_size(path);require(size>=668 && size<=256ull*1024*1024);
 Bytes b(static_cast<std::size_t>(size));std::ifstream f(path,std::ios::binary);require(bool(f.read(reinterpret_cast<char*>(b.data()),b.size())));
 require(std::string(b.begin(),b.begin()+8)=="EU96PRV1");const auto count=u32(b,8);require(count>0 && count<=200);
 for(int i=0;i<328;++i){mapping_[i]=static_cast<unsigned short>(u16(b,12+i*2));require(mapping_[i]==65535 || mapping_[i]<count);}
 std::size_t at=668;
 for(unsigned i=0;i<count;++i){Entry e{};e.width=u16(b,at);e.height=u16(b,at+2);auto n=u32(b,at+4);at+=8;
  require(((e.width==140 && e.height==285)||(e.width==320 && e.height==240)) && n>=1026 && n<=16*1024*1024 && at<=b.size() && b.size()-at>=768ull+n);
  e.palette.assign(b.begin()+at,b.begin()+at+768);at+=768;require(*std::max_element(e.palette.begin(),e.palette.end())<=63);
  e.data.assign(b.begin()+at,b.begin()+at+n);at+=n;entries_.push_back(std::move(e));
 }
 require(at==b.size());
}
bool OriginalPreview::select(int slot){return select_entry(slot>=0 && slot<328 && mapping_[slot]!=65535?mapping_[slot]:-1);}
bool OriginalPreview::select_entry(int id){
 if(id==selected_)return !frames_.empty();selected_=-1;frames_.clear();rgba_.clear();width_=height_=frame_=0;clock_=0;
 if(id<0 || id>=int(entries_.size()))return false;
 const auto& e=entries_[id];auto decoded=decode(e.data,e.width,e.height);frames_=std::move(decoded);selected_=id;width_=e.width;height_=e.height;colour();return true;
}
void OriginalPreview::colour(){
 const auto& pal=entries_[selected_].palette;const auto& pixels=frames_[frame_];rgba_.resize(pixels.size()*4);
 for(std::size_t i=0;i<pixels.size();++i){for(int c=0;c<3;++c)rgba_[i*4+c]=pal[pixels[i]*3+c]*255/63;rgba_[i*4+3]=255;}
}
void OriginalPreview::advance(double seconds){
 if(frames_.empty() || !std::isfinite(seconds) || seconds<=0)return;
 clock_=std::fmod(clock_+std::min(seconds,.25)*20.4,double(frames_.size()));const int next=int(clock_);
 if(next!=frame_){frame_=next;colour();}
}
}
