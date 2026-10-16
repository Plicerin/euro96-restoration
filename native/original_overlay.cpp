#include "original_overlay.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace actua {
OriginalOverlay::OriginalOverlay(const Archive& archive):pixels_(640*480){
 for(auto& page:pages)page.resize(65536);
 const auto load=[&](int record,int page,int at,int expected){auto data=archive.entry(record);if(data.size()!=expected || at+expected>65536)throw std::runtime_error("Original HUD atlas bounds");std::copy(data.begin(),data.end(),pages[page].begin()+at);};
 load(36,0,0,62*256);load(37,1,0,65536);load(121,0,180*256,76*256);
}
void OriginalOverlay::clear(const Palette& p){palette=p;std::fill(pixels_.begin(),pixels_.end(),0);}
void OriginalOverlay::pixel(int x,int y,int col){
 if(x<0 || y<0 || x>=640 || y>=480)return;
 const auto& c=palette.at(col);pixels_[y*640+x]=unsigned(c[0])*4 | (unsigned(c[1])*4<<8) | (unsigned(c[2])*4<<16) | 0xff000000u;
}
void OriginalOverlay::blit(const recovered::HudSprite& s,int x,int y,int col,int scale){
 if(s.page<0 || s.page>=2 || s.x<0 || s.y<0 || s.w<0 || s.h<0 || s.x+s.w>256 || s.y+s.h>256 || scale<1 || scale>2)throw std::runtime_error("Invalid original HUD sprite");
 for(int j=0;j<s.h;++j)for(int i=0;i<s.w;++i){const unsigned char value=pages[s.page][(s.y+j)*256+s.x+i];if(!value)continue;
  const int index=value==1?col:value-1;
  for(int dy=0;dy<scale;++dy)for(int dx=0;dx<scale;++dx)pixel(x+i*scale+dx,y+j*scale+dy,index);
 }
}
void OriginalOverlay::sprite(int id,int x,int y,int col,int scale){
 if(id<1 || id>=int(std::size(recovered::hud_sprites)))throw std::runtime_error("Invalid HUD sprite ID");
 blit(recovered::hud_sprites[id],x,y,col,scale);
}
int OriginalOverlay::glyph(unsigned char c){
 if(c==' ')c=';';if(c=='.')c='@';if(c==',')c='?';if(c=='>')c='A';
 if(c>='a' && c<='z')c-=32;if(c=='O')c='0';if(c=='&')c='O';
 return c>=48 && c<=90?c-48:11; // Original space as bounded fallback for unsupported text.
}
int OriginalOverlay::text_width(const std::string& value)const{int width=0;for(unsigned char c:value)width+=recovered::hud_fonts[2].widths[glyph(c)]+1;return width;}
int OriginalOverlay::text(int x,int y,const std::string& value,int col,int just){
 const auto& f=recovered::hud_fonts[2];const int width=text_width(value);x-=just==1?width:just==2?width/2:0;
 for(unsigned char c:value){const int index=glyph(c),cell=index+f.off;
  blit({f.page,f.x+f.w*(cell%f.n),f.y+f.h*(cell/f.n),f.widths[index],f.h},x,y,col,1);x+=f.widths[index]+1;
 }return width;
}
std::array<int,3> OriginalOverlay::marker(float x,float y,int slot,int half){
 // 3DENG.C scanner_h: plyrdat.z is the unnegated pitch y coordinate.
 if(!std::isfinite(x) || !std::isfinite(y))throw std::runtime_error("Invalid radar position");
 x=std::clamp(x,0.0f,1280.0f);y=std::clamp(y,0.0f,800.0f);
 return {int(15+2*(y*34)/800),int(480-21-2*(x*46)/1280),90+(((slot<11 && !(half&1)) || (slot>=11 && (half&1)))?0:2)};
}
void OriginalOverlay::scanner(const std::vector<Marker>& players,float bx,float by,int half){
 for(int y=368;y<464;++y)for(int x=16;x<88;++x)pixels_[y*640+x]=0x80000000u;
 sprite(88,18,370,0,2);
 for(const auto& p:players){const auto m=marker(p.x,p.y,p.slot,half);sprite(m[2],m[0],m[1]);}
 const auto b=marker(bx,by,0,half);sprite(89,b[0]+1,b[1]+1,31);
}
void OriginalOverlay::notice(const std::string& title,const std::string& detail,int icon){
 // Original goal/booked/offside menu font and icons, with a compact native
 // translucent panel instead of the DOS sliding menu buffer.
 for(int y=35;y<103;++y)for(int x=68;x<572;++x)pixels_[y*640+x]=0xd0081020u;
 std::string line=detail;while(!line.empty() && text_width(line)>430)line.pop_back();
 text(310,48,title,31,2);text(310,75,line,31,2);
 if(icon){sprite(icon,536,49);if(icon==44 || icon==45)sprite(46,536,55);}
}
void OriginalOverlay::score(const std::string& home,const std::string& away,int a,int b,const std::string& time,bool sub_home,bool sub_away){
 text(320,1,time,31,2);
 const int left=text(280,466,home,32,1);
 if(sub_home)sprite(86,280-left-20,460);
 text(320,466,std::to_string(a)+"="+std::to_string(b),31,2);
 const int right=text(360,466,away,56);
 if(sub_away)sprite(87,360+right+4,460);
}
}
