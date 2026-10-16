#include "match_preferences.hpp"
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
namespace actua {
namespace {
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
std::uint64_t checksum(const std::string& s){std::uint64_t h=1469598103934665603ull;for(unsigned char c:s){h^=c;h*=1099511628211ull;}return h;}
bool percent(int value){return value>=0 && value<=100;}
}
bool MatchOptions::valid()const{return wind>=0 && wind<=3 && percent(referee_accuracy) && percent(referee_strictness);}
bool MatchPreferences::valid()const{return match.valid() && percent(music) && percent(effects) && percent(crowd) && percent(commentary) && commentary_language>=0 && commentary_language<=1;}
void MatchPreferences::save(const std::filesystem::path& path)const{
    require(valid(),"Invalid preferences");
    std::ostringstream out;
    out<<"ACTUA96_PREFERENCES 2\n"<<match.wind<<' '<<match.offside<<' '<<match.fouls<<' '<<match.bookings<<' '<<match.referee_accuracy<<' '<<match.referee_strictness<<'\n'
       <<music<<' '<<effects<<' '<<crowd<<' '<<commentary<<' '<<muted<<' '<<commentary_language<<'\n';
    const auto body=out.str();out<<checksum(body)<<'\n';
    if(!path.parent_path().empty())std::filesystem::create_directories(path.parent_path());
    auto temporary=path;temporary+=".tmp";
    {std::ofstream file(temporary,std::ios::binary|std::ios::trunc);file<<out.str();file.flush();require(bool(file),"Could not write preferences");}
    if(std::filesystem::exists(path)){
        bool validOld=false;try{MatchPreferences old;old.load(path);validOld=true;}catch(const std::exception&){}
        if(validOld){auto backup=path;backup+=".bak";std::filesystem::copy_file(path,backup,std::filesystem::copy_options::overwrite_existing);}
    }
#ifdef _WIN32
    require(MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0,"Could not replace preferences");
#else
    std::filesystem::rename(temporary,path);
#endif
}
void MatchPreferences::load(const std::filesystem::path& path){
    require(std::filesystem::file_size(path)<=4096,"Preferences file too large");
    std::ifstream file(path,std::ios::binary);std::string bytes((std::istreambuf_iterator<char>(file)),{});
    require(bytes.size()>2 && bytes.back()=='\n',"Truncated preferences");
    const auto split=bytes.rfind('\n',bytes.size()-2);require(split!=std::string::npos,"Invalid preferences");
    std::uint64_t stored=0;std::istringstream hash(bytes.substr(split+1));require(bool(hash>>stored),"Missing preferences checksum");hash>>std::ws;
    require(hash.eof() && stored==checksum(bytes.substr(0,split+1)),"Preferences checksum mismatch");
    std::istringstream in(bytes.substr(0,split+1));std::string magic;int version=0,offside=0,fouls=0,bookings=0,muteFlag=0;MatchPreferences next;
    require(bool(in>>magic>>version) && magic=="ACTUA96_PREFERENCES" && (version==1 || version==2),"Unsupported preferences");
    require(bool(in>>next.match.wind>>offside>>fouls>>bookings>>next.match.referee_accuracy>>next.match.referee_strictness
        >>next.music>>next.effects>>next.crowd>>next.commentary>>muteFlag),"Incomplete preferences");
    for(int flag:{offside,fouls,bookings,muteFlag})require(flag==0 || flag==1,"Invalid preferences switch");
    if(version==2)require(bool(in>>next.commentary_language),"Missing commentary language");
    next.match.offside=offside!=0;next.match.fouls=fouls!=0;next.match.bookings=bookings!=0;next.muted=muteFlag!=0;
    in>>std::ws;require(in.eof() && next.valid(),"Invalid preferences values");*this=next;
}
}
