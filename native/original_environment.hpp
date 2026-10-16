#pragma once
#include "archive.hpp"
#include <array>
#include <cstdint>
namespace actua::legacy {class Core;}
namespace actua {
using MatchPalette=std::array<std::array<unsigned char,3>,256>;
class OriginalEuroPlayers {
public:
 void load(const std::filesystem::path& root);
 bool loaded()const{return !bytes_.empty();}
 int skin_shift(int team)const;
 void bind(std::vector<std::array<std::uint32_t,8>>&)const;
 void overlay(Bytes& rgba,int page,int home,int away)const;
private:
 Bytes bytes_;
};
class OriginalShirtNumbers {
public:
 void load(const std::filesystem::path& root);
 bool loaded()const{return !rgba_.empty();}
 const Bytes& rgba()const{return rgba_;}
 const std::array<std::array<std::uint32_t,8>,40>& coordinates()const{return coordinates_;}
private:
 Bytes rgba_;
 std::array<std::array<std::uint32_t,8>,40> coordinates_{};
};
// Retail Euro96 outfield overlay. Its expanded palette is decoded per texture;
// it must not overwrite the preserved stadium/keeper palette or head layout.
class OriginalEuroKits {
public:
 void load(const std::filesystem::path& root);
 bool loaded()const{return !bytes_.empty();}
 int selection(int home,int away,bool second)const;
 int appearance(int team,int shirt)const;
 void overlay(Bytes& rgba,int page,int home,int away,int home_skin=-1,int away_skin=-1)const;
private:
 Bytes bytes_;
};
class OriginalCdKits {
public:
 void load(const std::filesystem::path& root);
 bool has(int team)const{return team>=0 && team<18 && !kits_[team][0].empty();}
 std::span<const unsigned char> part(int team,int field)const{return kits_.at(team).at(field);}
 int away_kit(int home,int away)const {if(home<0 || home>=16 || away<0 || away>=16)return away;const int c=clashes_[home*16+away];return c && has(15+c)?15+c:away;}
 bool alternate_skin(int team,int shirt)const{return team>=0 && team<16 && shirt>=1 && shirt<=20 && appearance_[team*20+shirt-1]==1;}
private:
 std::array<std::array<Bytes,8>,18> kits_;
 std::array<unsigned char,256> clashes_{};
 std::array<unsigned char,320> appearance_{};
};
// Original ground projection and indexed maps; native bounded replacement for DOS scanline assembly.
class OriginalEnvironment {
public:
 explicit OriginalEnvironment(const Archive&);
 void render(const legacy::Core&,const MatchPalette&);
 void set_stadium(std::span<const unsigned char> pitch,std::span<const unsigned char> sky,int width,int length,int height);
 const std::vector<std::uint32_t>& pixels()const{return pixels_;}
 int width()const{return width_;}
 int height()const{return height_;}
 unsigned ground_pixels=0,sky_pixels=0;
 unsigned char pitch_index(double x,double z)const;
private:
 Bytes pitch_,sky_;
 int width_=640,height_=480;
 int stadium_width_=0,stadium_length_=0,stadium_height_=0;
 std::vector<std::uint32_t> pixels_;
};
}
