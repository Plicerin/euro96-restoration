#include "original_environment.hpp"
#include "original_graphics.hpp"
#include "graphics_data.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace actua {
namespace {constexpr std::size_t player_uv=8+768+16,player_pages=player_uv+180*32;}
void OriginalEuroPlayers::load(const std::filesystem::path& root){
 const auto path=root/"GRAPHICS/PLAYERS.BIN";
 if(!std::filesystem::exists(path)){bytes_.clear();return;}
 if(std::filesystem::file_size(path)!=player_pages+6*65536)throw std::runtime_error("Invalid retail player pack size");
 auto b=read_file(path);if(std::string(b.begin(),b.begin()+8)!="EU96PLY1")throw std::runtime_error("Invalid retail player signature");
 for(int i=8;i<776;++i)if(b[i]>63)throw std::runtime_error("Invalid retail player palette");
 for(int i=776;i<792;++i)if(b[i]>1)throw std::runtime_error("Invalid retail team skin mapping");
 for(int i=0;i<180;++i)for(int j=0;j<8;++j){const auto page=b[player_uv+i*32+j*4+3];
  const unsigned expected=j>=4?0:i<120?0:i<48+120?12:13;
  if(page!=expected)throw std::runtime_error("Invalid retail head texture page");
 }
 bytes_=std::move(b);
}
int OriginalEuroPlayers::skin_shift(int team)const{return loaded() && team>=0 && team<16?6*bytes_[776+team]:-1;}
void OriginalEuroPlayers::bind(std::vector<std::array<std::uint32_t,8>>& coords)const{
 if(!loaded())return;if(coords.size()<821)throw std::runtime_error("Missing player coordinate table");
 for(int i=0;i<180;++i)for(int j=0;j<8;++j){std::uint32_t v=0;for(int k=0;k<4;++k)v|=std::uint32_t(bytes_[player_uv+i*32+j*4+k])<<(k*8);coords[i<120?1+i:761+i-120][j]=v&0xffffff00;}
}
void OriginalEuroPlayers::overlay(Bytes& rgba,int page,int home,int away)const{
 if(!loaded() || skin_shift(home)<0 || skin_shift(away)<0)return;
 if(rgba.size()!=65536*4)throw std::runtime_error("Invalid retail player destination");
 const int source=page==0?0:page==3?1:(page==4 || page==14)?2:page==12?3:page==13?4:page==5?5:-1;
 if(source<0)return;
 const int begin=page==3?158*256:0,end=page==3?225*256:65536;
 for(int i=begin;i<end;++i){int index=bytes_[player_pages+source*65536+(page==3?i-begin:i)];
  const bool dark=(page==12 && i>=158*256) || (page==13 && i>=158*256);
  const bool second=page==14 || page==13 || (page==0 && i>=128*256);
  const int shift=dark?12:skin_shift(second?away:home);
  if(index>=32 && index<64)index=std::max(32,index-shift);
  else if(page==0 && shift && index>=5 && index<32)index-=4;
  for(int c=0;c<3;++c)rgba[i*4+c]=bytes_[8+index*3+c]*4;rgba[i*4+3]=index?255:0;
 }
}
void OriginalShirtNumbers::load(const std::filesystem::path& root){
 const auto path=root/"GRAPHICS/NUMBERS.BIN";
 if(!std::filesystem::exists(path)){rgba_.clear();coordinates_={};return;}
 constexpr std::size_t atlas_at=8+96+40*32;
 if(std::filesystem::file_size(path)!=atlas_at+65536)throw std::runtime_error("Invalid shirt-number pack size");
 const auto bytes=read_file(path);
 if(std::string(bytes.begin(),bytes.begin()+8)!="EU96NUM1")throw std::runtime_error("Invalid shirt-number signature");
 for(std::size_t i=8;i<104;++i)if(bytes[i]>63)throw std::runtime_error("Invalid shirt-number palette");
 decltype(coordinates_) coords{};
 for(int i=0;i<40;++i){
  const int variant=i/20,n=i%20;
  const unsigned x=(n<15?n:n-15+variant*5)*17,y=n<15?181+variant*25:231;
  const std::array<unsigned,8> expected{y,y,y+24,y+24,x,x+16,x+16,x};
  for(int j=0;j<8;++j){std::uint32_t v=0;for(int k=0;k<4;++k)v|=std::uint32_t(bytes[104+i*32+j*4+k])<<(8*k);
   if((v>>24)!=(j<4?6u:0u) || ((v&0xffffff)>>16)!=expected[j])throw std::runtime_error("Invalid shirt-number UV rectangle");
   coords[i][j]=(v&0x00ffff00)|(j<4?16u<<24:0);
  }
 }
 Bytes rgba(65536*4);
 for(std::size_t i=0;i<65536;++i){const auto index=bytes[atlas_at+i];if(index>31)throw std::runtime_error("Invalid shirt-number colour index");
  for(int c=0;c<3;++c)rgba[i*4+c]=bytes[8+index*3+c]*4;rgba[i*4+3]=index?255:0;
 }
 coordinates_=coords;rgba_=std::move(rgba);
}
namespace {
constexpr std::size_t euro_torso=8+768+19968,euro_stride=65536+96;
constexpr std::size_t euro_clashes=euro_torso+32*euro_stride,euro_appearance=euro_clashes+256;
}
void OriginalEuroKits::load(const std::filesystem::path& root){
 const auto path=root/"GRAPHICS/EURO96.BIN";
 if(!std::filesystem::exists(path)){bytes_.clear();return;}
 if(std::filesystem::file_size(path)!=euro_appearance+320)throw std::runtime_error("Invalid Euro96 kit size");
 auto data=read_file(path);
 if(std::string(data.begin(),data.begin()+8)!="EU96KIT1")throw std::runtime_error("Invalid Euro96 kit pack");
 const auto palette_ok=[&](std::size_t at,std::size_t n){return std::all_of(data.begin()+at,data.begin()+at+n,[](unsigned char v){return v<=63;});};
 if(!palette_ok(8,768))throw std::runtime_error("Invalid Euro96 base palette");
 for(int i=0;i<32;++i)if(!palette_ok(euro_torso+i*euro_stride+65536,96))throw std::runtime_error("Invalid Euro96 uniform palette");
 for(std::size_t i=euro_clashes;i<euro_appearance;++i)if(data[i]>1)throw std::runtime_error("Invalid Euro96 kit selection");
 for(std::size_t i=euro_appearance;i<data.size();++i)if(data[i]!=0 && data[i]!=1 && data[i]!=255)throw std::runtime_error("Invalid Euro96 appearance");
 bytes_=std::move(data);
}
int OriginalEuroKits::selection(int home,int away,bool second)const{
 if(!loaded() || home<0 || home>=16 || away<0 || away>=16)return -1;
 return second?away+16*bytes_[euro_clashes+home*16+away]:home;
}
int OriginalEuroKits::appearance(int team,int shirt)const{
 if(!loaded() || team<0 || team>=16 || shirt<1 || shirt>20)return -1;
 const int v=bytes_[euro_appearance+team*20+shirt-1];return v==255?-1:v;
}
void OriginalEuroKits::overlay(Bytes& rgba,int page,int home,int away,int home_skin,int away_skin)const{
 if(!loaded() || selection(home,away,false)<0)return;
 if(rgba.size()!=65536*4)throw std::runtime_error("Invalid Euro96 destination texture");
 const auto region=[&](bool second,bool limbs,std::size_t destination){
  const int kit=selection(home,away,second);const auto torso=euro_torso+kit*euro_stride;
  const std::size_t at=limbs?8+768:torso,n=limbs?19968:65536;
  for(std::size_t i=0;i<n;++i){int index=bytes_[at+i];
   // Without the retail player pack, retain the restored skin palette. With
   // it, apply the executable's skin remap to the retail limb/torso pixels.
   if(index>=32 && index<64){if(home_skin<0 || away_skin<0)continue;index=std::max(32,index-(page>=10?12:second?away_skin:home_skin));}
   const std::size_t pal=index>=64 && index<96?torso+65536+(index-64)*3:8+index*3;
   for(int c=0;c<3;++c)rgba[(destination+i)*4+c]=bytes_[pal+c]*4;
   rgba[(destination+i)*4+3]=index?255:0;
  }
 };
 if(page==1 || page==10)region(false,false,0);
 else if(page==2 || page==11)region(true,false,0);
 else if(page==3 || page==12){region(false,true,0);region(true,true,80*256);}
}
void OriginalCdKits::load(const std::filesystem::path& root){
 const auto path=root/"GRAPHICS/TEAMS.BIN";if(!std::filesystem::exists(path))return;
 if(std::filesystem::file_size(path)>3*1024*1024)throw std::runtime_error("Oversized CD kit pack");
 const auto bytes=read_file(path);std::size_t at=8;
 if(bytes.size()<12 || (std::string(bytes.begin(),bytes.begin()+8)!="AC96KIT1" && std::string(bytes.begin(),bytes.begin()+8)!="AC96KIT2"))throw std::runtime_error("Invalid CD kit pack");
 const auto word=[&](){if(bytes.size()-at<4)throw std::runtime_error("Truncated CD kit pack");unsigned v=0;for(int i=0;i<4;++i)v|=unsigned(bytes[at++])<<(i*8);return v;};
 const bool extended=bytes[7]=='2';
 const auto count=word();if(count>(extended?18:16))throw std::runtime_error("Invalid CD kit count");
 constexpr std::array<unsigned,8> sizes{32768,65536,19968,13824,72,24,24,24};
 decltype(kits_) loaded;
 for(unsigned i=0;i<count;++i){
  const auto team=word();if(team>=(extended?18u:16u) || !loaded[team][0].empty())throw std::runtime_error("Invalid CD team identity");
  for(int part=0;part<8;++part){const auto size=word();if(size!=sizes[part] || size>bytes.size()-at)throw std::runtime_error("Invalid CD kit component");
   loaded[team][part].assign(bytes.begin()+at,bytes.begin()+at+size);at+=size;
   if(part>=4 && *std::max_element(loaded[team][part].begin(),loaded[team][part].end())>63)throw std::runtime_error("Invalid CD kit palette");
  }
 }
 decltype(clashes_) clashes{};decltype(appearance_) appearance{};
 if(extended){
  if(bytes.size()-at!=576)throw std::runtime_error("Invalid CD presentation metadata");
  std::copy_n(bytes.begin()+at,256,clashes.begin());at+=256;std::copy_n(bytes.begin()+at,320,appearance.begin());at+=320;
  for(auto c:clashes)if(c>2 || (c && loaded[15+c][0].empty()))throw std::runtime_error("Invalid alternate kit selection");
  for(auto v:appearance)if(v!=0 && v!=1 && v!=255)throw std::runtime_error("Invalid player appearance flag");
 }
 if(at!=bytes.size())throw std::runtime_error("Trailing CD kit data");kits_=std::move(loaded);clashes_=clashes;appearance_=appearance;
}
OriginalEnvironment::OriginalEnvironment(const Archive& archive):pixels_(640*480){
 const auto pitch=archive.entry(recovered::pitch_bitmap),sky=archive.entry(recovered::sky_bitmap);
 if(pitch.size()!=64*256 || sky.size()!=640*480)throw std::runtime_error("Invalid original ground maps");
 set_stadium(pitch,sky,recovered::stad_width,recovered::stad_length,recovered::stad_height);
}
void OriginalEnvironment::set_stadium(std::span<const unsigned char> pitch,std::span<const unsigned char> sky,int width,int length,int height){
 if(pitch.size()!=64*256 || sky.size()!=640*480 || width<0 || width>2000 || length<0 || length>2000 || height<1 || height>2000)throw std::runtime_error("Invalid stadium environment");
 pitch_.assign(pitch.begin(),pitch.end());sky_.assign(sky.begin(),sky.end());
 stadium_width_=width;stadium_length_=length;stadium_height_=height;
}
unsigned char OriginalEnvironment::pitch_index(double x,double z)const{
 // ground detail 2: psh=16, pan=0x3f3f, 64x64 tile with a 256-byte row stride.
 const auto wrap=[](double v){double w=std::fmod(std::floor(v),64.0);return int(w<0?w+64:w);};
 return pitch_[wrap(z)*256+wrap(x)];
}
void OriginalEnvironment::render(const legacy::Core& core,const MatchPalette& palette){
 std::fill(pixels_.begin(),pixels_.end(),0);ground_pixels=sky_pixels=0;
 std::array<std::uint32_t,256> colors;
 for(int i=0;i<256;++i)colors[i]=std::uint32_t(palette[i][0]*4)|(std::uint32_t(palette[i][1]*4)<<8)|(std::uint32_t(palette[i][2]*4)<<16)|0xff000000u;
 const auto& m=core.rot.e;const double q=core.q,cph=m.r11,sph=m.r21;
 if(!std::isfinite(q) || q<=0)return;
 // Original ground() chooses the farthest stadium roof corner, with ST_DP=200.
 double depth=0,edge=240;
 for(float x:{-float(stadium_length_+200),float(1280+stadium_length_+200)})
  for(float z:{float(stadium_width_+200),-float(800+stadium_width_+200)}){
   const double rz=x*m.r20+stadium_height_*m.r21+z*m.r22+m.r23;
   if(rz>depth){depth=rz;edge=q*(x*m.r10+stadium_height_*m.r11+z*m.r12+m.r13)/rz;}
  }
 const int ye=std::clamp(int(std::clamp(edge,-240.0,240.0)),-240,240),height=240-ye;
 // Source high-resolution sky: three repeats per turn, heading scroll, pitch-dependent crop.
 if(height && std::abs(cph)>1e-6){
  int yd=int(std::clamp(ye+q*sph/cph,0.0,10000.0));if(479-yd<240-ye)yd=240+ye;
  const int offset=int(640*std::acos(std::clamp(double(m.r00),-1.0,1.0))/(2*3.1415/3))%640;
  const int shift=(-m.r02<0)?-offset:offset;
  for(int y=0;y<height;++y){const int row=std::clamp(480-yd-height+y,0,479);
   for(int x=0;x<640;++x){pixels_[y*640+x]=colors[sky_[row*640+(x+shift+640)%640]];++sky_pixels;}
  }
 }
 if(core.eye_y<=0)return;
 // Inverse of the original camera matrix equals ground()'s per-scanline tx/tz and increments.
 // Explicit pitch bounds and horizon checks replace DOS buffer assumptions.
 for(int y=0;y<480;++y){
  const double screen_y=240.0-y,dy=(screen_y/q)*m.r11+m.r21;if(dy>=-1e-6)continue;
  const double distance=-core.eye_y/dy;
  double wx=core.eye_x+distance*((-320.0/q)*m.r00+(screen_y/q)*m.r10+m.r20);
  double wz=core.eye_z+distance*((-320.0/q)*m.r02+(screen_y/q)*m.r12+m.r22);
  const double dx=distance*m.r00/q,dz=distance*m.r02/q;
  if(!std::isfinite(wx) || !std::isfinite(wz))continue;
  for(int x=0;x<640;++x,wx+=dx,wz+=dz){
   if(wx<-stadium_length_ || wx>1280+stadium_length_ || wz>stadium_width_ || wz<-800-stadium_width_)continue;
   pixels_[y*640+x]=colors[pitch_[(int(std::floor(wz))&63)*256+(int(std::floor(wx))&63)]];++ground_pixels;
  }
 }
}
}
