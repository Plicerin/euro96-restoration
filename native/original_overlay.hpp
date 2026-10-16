#pragma once
#include "archive.hpp"
#include "hud_data.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <vector>
namespace actua {
// Render-only original 640x480 HUD. No engine reference or gameplay mutation.
class OriginalOverlay {
public:
 using Palette=std::array<std::array<unsigned char,3>,256>;
 struct Marker {float x,y;int slot;};
 explicit OriginalOverlay(const Archive&);
 void clear(const Palette&);
 void scanner(const std::vector<Marker>&,float ball_x,float ball_y,int half);
 void score(const std::string& home,const std::string& away,int a,int b,const std::string& time,bool sub_home,bool sub_away);
 void notice(const std::string& title,const std::string& detail,int icon);
 void sprite(int id,int x,int y,int col=0,int scale=1);
 int text(int x,int y,const std::string&,int col,int justification=0);
 int text_width(const std::string&)const;
 static std::array<int,3> marker(float x,float y,int slot,int half);
 const std::vector<std::uint32_t>& pixels()const{return pixels_;}
private:
 std::array<Bytes,2> pages;
 Palette palette{};
 std::vector<std::uint32_t> pixels_;
 void blit(const recovered::HudSprite&,int,int,int,int);
 void pixel(int,int,int);
 static int glyph(unsigned char);
};
}
