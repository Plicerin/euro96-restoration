#include "saved_replay.hpp"
#include "browser_storage.hpp"
#include "original_roster.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <limits>
#include <source_location>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
namespace actua { namespace {
constexpr std::size_t max_bytes=1024*1024;
void need(bool ok,const std::source_location where=std::source_location::current()){if(!ok)throw std::runtime_error("Invalid or unsupported replay file (check "+std::to_string(where.line())+")");}
struct Codec {
 std::vector<unsigned char> bytes;std::size_t at=0;bool reading=false;
 void word(std::uint32_t& n){
  if(reading){need(at+4<=bytes.size());n=0;for(int i=0;i<4;++i)n|=std::uint32_t(bytes[at++])<<(8*i);}
  else for(int i=0;i<4;++i)bytes.push_back(static_cast<unsigned char>(n>>(8*i)));
 }
 template<class T>void integer(T& n){std::uint32_t v=std::uint32_t(n);word(v);if(reading){need(v<=65535 && v<=std::uint32_t(std::numeric_limits<T>::max()));n=static_cast<T>(v);}}
 void real(float& n){auto v=std::bit_cast<std::uint32_t>(n);word(v);if(reading)n=std::bit_cast<float>(v);}
 void frame(ReplayFrame& f){
  for(auto& p:f.players){
   integer(p.tm_player);integer(p.shirt);integer(p.guy_on);integer(p.draw_me);integer(p.tm_anim);integer(p.tm_newanim);
   real(p.tm_x);real(p.tm_y);real(p.tm_z);real(p.tm_xdis);real(p.tm_ydis);real(p.tm_frm);real(p.tm_fstep);
  }
  for(auto& r:f.officials){real(r.x);real(r.y);real(r.z);real(r.dir_x);real(r.dir_y);real(r.frm);real(r.fstep);integer(r.anim);}
  real(f.x);real(f.y);real(f.z);real(f.horizontal);real(f.vertical);real(f.xd);real(f.yd);
  integer(f.owner);integer(f.limbo);integer(f.limbo_player);integer(f.last_touch);integer(f.half);
 }
};
std::uint32_t checksum(const std::vector<unsigned char>& b,std::size_t n){std::uint32_t h=2166136261u;for(std::size_t i=0;i<n;++i)h=(h^b[i])*16777619u;return h;}
void number(float n,float limit){if(!std::isfinite(n) || std::abs(n)>limit)throw std::runtime_error("Invalid replay number "+std::to_string(n)+" (bound "+std::to_string(limit)+")");}
void animation(int anim,float phase,float step,const original::PlayerEngine* e){
 // Original phases accumulate across cycles; presentation samples their fractional part.
 need(anim>=0 && anim<256);number(phase,1000000);number(step,10);need(step>=0);
 if(e)need(std::size_t(anim)<e->mcaps.size() && e->mcaps[anim].capfrms>0);
}
}
void SavedReplay::validate(const original::PlayerEngine* e)const{
 need(home>=0 && home<16 && away>=0 && away<16 && home!=away);
 need(home_goals>=0 && home_goals<=999 && away_goals>=0 && away_goals<=999);
 need(stadium>=0 && stadium<13 && camera>=0 && camera<5 && frames.size()>=2 && frames.size()<=300);
 need((wind_on==0 || wind_on==1) && (ball_still==0 || ball_still==1));number(wind_speed,128);need(wind_speed>=0);number(wind_x,1.01f);number(wind_y,1.01f);
 for(const auto& f:frames){
  need(f.owner>=0 && f.owner<=22 && (f.limbo==0 || f.limbo==1) && f.limbo_player>=0 && f.limbo_player<=22 && (!f.limbo || f.limbo_player>0) && f.last_touch>=0 && f.last_touch<=22 && f.half>=0 && f.half<=3);
  for(float v:{f.x,f.y,f.z})number(v,10000);for(float v:{f.horizontal,f.vertical})number(v,100);number(f.xd,1000);number(f.yd,1000);
  for(int i=0;i<22;++i){const auto& p=f.players[i];
   need(p.tm_player==i+1 && p.shirt>=1 && p.shirt<=20 && (p.guy_on==0 || p.guy_on==1) && (p.draw_me==0 || p.draw_me==1) && (p.tm_newanim==0 || p.tm_newanim==1));
   for(float v:{p.tm_x,p.tm_y,p.tm_z})number(v,10000);number(p.tm_xdis,1.01f);number(p.tm_ydis,1.01f);
   animation(p.tm_anim,p.tm_frm,p.tm_fstep,(p.guy_on || f.owner==i+1 || (f.limbo && f.limbo_player==i+1))?e:nullptr);
  }
  for(const auto& r:f.officials){for(float v:{r.x,r.y,r.z})number(v,10000);number(r.dir_x,1.01f);number(r.dir_y,1.01f);animation(r.anim,r.frm,r.fstep,r.fstep>0?e:nullptr);}
 }
}
SavedReplay SavedReplay::load(const std::filesystem::path& path){
 std::ifstream file(path,std::ios::binary|std::ios::ate);if(!file)throw std::runtime_error("Cannot open replay file");
 const auto size=file.tellg();need(size>=44 && size<=max_bytes);Codec c;c.reading=true;c.bytes.resize(std::size_t(size));
 file.seekg(0);need(bool(file.read(reinterpret_cast<char*>(c.bytes.data()),size)));
 std::uint32_t magic=0,version=0,count=0;c.word(magic);c.word(version);need(magic==0x52363941 && version==1);
 SavedReplay r;c.integer(r.home);c.integer(r.away);c.integer(r.home_goals);c.integer(r.away_goals);c.integer(r.stadium);c.integer(r.camera);c.word(count);need(count>=2 && count<=300);
 c.integer(r.wind_on);c.integer(r.ball_still);c.real(r.wind_speed);c.real(r.wind_x);c.real(r.wind_y);
 // Exact extent precedes allocation/decoding. Each visual frame is 1,288 bytes.
 need(c.bytes.size()==56+std::size_t(count)*1288+4);const auto expected=checksum(c.bytes,c.bytes.size()-4);
 const auto header=c.at;c.at=c.bytes.size()-4;std::uint32_t actual=0;c.word(actual);need(actual==expected);c.at=header;
 r.frames.resize(count);for(auto& f:r.frames)c.frame(f);need(c.at==c.bytes.size()-4);r.validate();return r;
}
std::filesystem::path SavedReplay::save_new(const std::filesystem::path& directory)const{
 validate();Codec c;std::uint32_t magic=0x52363941,version=1,count=std::uint32_t(frames.size());c.word(magic);c.word(version);
 auto copy=*this;c.integer(copy.home);c.integer(copy.away);c.integer(copy.home_goals);c.integer(copy.away_goals);c.integer(copy.stadium);c.integer(copy.camera);c.word(count);
 c.integer(copy.wind_on);c.integer(copy.ball_still);c.real(copy.wind_speed);c.real(copy.wind_x);c.real(copy.wind_y);
 for(auto& f:copy.frames)c.frame(f);auto hash=checksum(c.bytes,c.bytes.size());c.word(hash);need(c.bytes.size()<=max_bytes);
 std::filesystem::create_directories(directory);const auto now=std::time(nullptr);std::tm utc{};
#ifdef _WIN32
 gmtime_s(&utc,&now);
#else
 gmtime_r(&now,&utc);
#endif
 std::ostringstream name;name<<std::put_time(&utc,"%Y%m%d-%H%M%S")<<'-'<<original_teams[home].code<<'-'<<original_teams[away].code;
 for(int suffix=0;suffix<10000;++suffix){
  auto path=directory/(name.str()+"-"+std::to_string(suffix)+".a96r");auto temp=path;temp+=".tmp";
#ifdef _WIN32
  if(std::filesystem::exists(path))continue;
  HANDLE handle=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
  if(handle==INVALID_HANDLE_VALUE){if(GetLastError()==ERROR_FILE_EXISTS)continue;throw std::runtime_error("Cannot create replay file");}
  DWORD written=0;const bool ok=WriteFile(handle,c.bytes.data(),DWORD(c.bytes.size()),&written,nullptr) && written==c.bytes.size() && FlushFileBuffers(handle);CloseHandle(handle);
  if(!ok){DeleteFileW(temp.c_str());throw std::runtime_error("Cannot write replay file");}
  if(MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_WRITE_THROUGH))return path;
  const auto error=GetLastError();DeleteFileW(temp.c_str());if(error==ERROR_ALREADY_EXISTS || error==ERROR_FILE_EXISTS)continue;
  throw std::runtime_error("Cannot finish replay save");
#else
  if(std::filesystem::exists(path) || std::filesystem::exists(temp))continue;
  std::ofstream out(temp,std::ios::binary);out.write(reinterpret_cast<const char*>(c.bytes.data()),c.bytes.size());out.close();
  if(!out){std::filesystem::remove(temp);throw std::runtime_error("Cannot write replay file");}std::filesystem::rename(temp,path);browser_save_changed();return path;
#endif
 }
 throw std::runtime_error("Replay name capacity reached");
}
std::string SavedReplay::label()const{return std::string(original_teams[home].code)+" "+std::to_string(home_goals)+" - "+std::to_string(away_goals)+" "+original_teams[away].code+"   "+std::to_string(frames.size()/20)+"s";}
std::vector<ReplayEntry> replay_library(const std::filesystem::path& directory){
 std::vector<ReplayEntry> result;if(!std::filesystem::exists(directory))return result;
 for(const auto& f:std::filesystem::directory_iterator(directory)){
  if(f.path().extension()!=".a96r" || !f.is_regular_file() || f.is_symlink())continue;
  if(result.size()>=1000)throw std::runtime_error("Replay library exceeds 1000 clips");
  result.push_back({f.path(),{},false});
 }
 std::sort(result.begin(),result.end(),[](const auto& a,const auto& b){return a.path.filename()>b.path.filename();});
 for(auto& entry:result)try{entry.label=SavedReplay::load(entry.path).label();entry.valid=true;}catch(const std::exception&){entry.label="UNREADABLE REPLAY";}
 return result;
}
}
