#include "original_menu_art.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace actua {
OriginalMenuArt::OriginalMenuArt(const Archive& a):frame_(640*480*4),upload_(640*480*4){
 for(int i=0;i<111;++i){const auto e=a.entry(i);entries_[i]={e.begin(),e.end()};}
 font_palette_=entries_[19];
 // The retail cyan ramp reverses the white font's shading, leaving pale rims
 // around dark interiors. Tint the readable white ramp for selected text and
 // headings instead, retaining the glyphs and all other palette entries.
 constexpr int cyan_tint[]={24,54,63};
 for(int shade=1;shade<=7;++shade)for(int c=0;c<3;++c)
  font_palette_[(36+shade)*3+c]=font_palette_[(15+shade)*3+c]*cyan_tint[c]/63;
 for(const auto& s:menu_art::images)if(s.record>=0 && (s.x<0 || s.y<0 || s.w<0 || s.h<0 || s.x+s.w>640 || std::size_t(s.y+s.h)*640>entries_[s.record].size()))throw std::runtime_error("Original menu sprite exceeds archive page");
}
OriginalMenuArt::~OriginalMenuArt(){SDL_DestroyTexture(texture_);}
void OriginalMenuArt::clear(){std::fill(frame_.begin(),frame_.end(),0);palette_=entries_[19];std::copy(entries_[15].begin(),entries_[15].end(),palette_.begin());}
void OriginalMenuArt::begin(int screen,int team){
 if(screen<0 || screen>=15)throw std::out_of_range("Original menu screen");
 palette_=entries_[16+screen];std::copy(entries_[15].begin(),entries_[15].end(),palette_.begin());
 if(team>=0 && team<16 && (screen==1 || screen==5)){
  std::copy(entries_[31+team].begin(),entries_[31+team].end(),palette_.begin()+48);
  constexpr int formations[]={49,48,52,52,47,50,52,51,47,52,52,47,47,52,52,52};
  std::copy(entries_[formations[team]].begin(),entries_[formations[team]].end(),palette_.begin()+96);
 }
 const auto& raw=entries_[screen];for(std::size_t i=0;i<raw.size();++i){for(int c=0;c<3;++c)frame_[i*4+c]=palette_[raw[i]*3+c]*255/63;frame_[i*4+3]=255;}
}
void OriginalMenuArt::blit(int rec,int sx,int sy,int sw,int sh,int x,int y,int w,int h,const Bytes& pal,int offset,bool panel_mask,bool mirror){
 if(rec<0 || rec>=111 || sw<=0 || sh<=0 || w<=0 || h<=0)return;
 const auto& page=entries_[rec];if(sx<0 || sy<0 || sx+sw>640 || std::size_t(sy+sh)*640>page.size())throw std::out_of_range("Menu image bounds");
 for(int yy=std::max(0,-y);yy<std::min(h,480-y);++yy)for(int xx=std::max(0,-x);xx<std::min(w,640-x);++xx){
  const int source_x=xx*sw/w;
  const int pixel=page[(sy+yy*sh/h)*640+sx+(mirror?sw-1-source_x:source_x)];if(!pixel)continue;const int index=(pixel+offset)&255;const auto to=((y+yy)*640+x+xx)*4;
  // The rounded panel was baked onto white artwork. Exclude that exterior
  // when reusing its navy border over a different screen or the live pitch.
  if(panel_mask && (pal[index*3]>15 || pal[index*3+1]>15))continue;
  for(int c=0;c<3;++c)frame_[to+c]=pal[index*3+c]*255/63;frame_[to+3]=255;
 }
}
void OriginalMenuArt::crop(int rec,int sx,int sy,int sw,int sh,int x,int y,int w,int h){blit(rec,sx,sy,sw,sh,x,y,w,h,palette_);}
void OriginalMenuArt::rgba(const Bytes& pixels,int sw,int sh,int x,int y,int w,int h){
 if(sw<=0 || sh<=0 || w<=0 || h<=0 || pixels.size()!=std::size_t(sw)*sh*4)return;
 for(int dy=std::max(0,y);dy<std::min(480,y+h);++dy)for(int dx=std::max(0,x);dx<std::min(640,x+w);++dx){
  const auto src=((dy-y)*sh/h*sw+(dx-x)*sw/w)*4;std::copy_n(pixels.begin()+src,4,frame_.begin()+(dy*640+dx)*4);
 }
}
void OriginalMenuArt::image(int id,int x,int y,int w,int h,int team,bool mirror){
 const auto& s=menu_art::images[std::clamp(id,0,1158)];auto pal=palette_;
 if(team>=0 && team<16)std::copy(entries_[31+team].begin(),entries_[31+team].end(),pal.begin()+48);
 blit(s.record,s.x,s.y,s.w,s.h,x,y,w?w:s.w,h?h:s.h,pal,0,false,mirror);
}
void OriginalMenuArt::panel(int x,int y,int w,int h){
 // Nine-slice the original dark statistics panel; preserve its rounded edge.
 const auto pal=entries_[21];constexpr int sx=19,sy=95,sw=258,sh=160,k=12;
 const int dx[]={x,x+k,x+w-k},dy[]={y,y+k,y+h-k};const int ws[]={k,w-2*k,k},hs[]={k,h-2*k,k};
 const int xs[]={sx,sx+k,sx+sw-k},ys[]={sy,sy+k,sy+sh-k};const int srcw[]={k,sw-2*k,k},srch[]={k,sh-2*k,k};
 for(int j=0;j<3;++j)for(int i=0;i<3;++i){
  if(i==1 && j==1)blit(5,40,112,1,1,dx[i],dy[j],ws[i],hs[j],pal);
  else blit(5,xs[i],ys[j],srcw[i],srch[j],dx[i],dy[j],ws[i],hs[j],pal,0,true);
 }
}
void OriginalMenuArt::bar(int x,int y,int w){
 const auto& s=menu_art::images[menu_art::INFO_NBAR];
 blit(s.record,s.x,s.y,38,38,x,y,38,38,palette_);
 blit(s.record,s.x+38,s.y,1,38,x+38,y,w-76,38,palette_);
 image(menu_art::INFO_NBAR+1,x+w-38,y);
}
int OriginalMenuArt::width(const std::string& s,int size)const{int w=0;for(unsigned char c:s)w+=menu_art::images[size*130+(c>=32 && c<=126?c-32:'?'-32)].w+1;return w;}
void OriginalMenuArt::text(int x,int y,const std::string& value,Ink ink,int size,int align,int limit){
 auto s=value;size=std::clamp(size,0,2);
 if(width(s,size)>limit){while(!s.empty() && width(s+"...",size)>limit)s.pop_back();s+="...";}
 const int w=width(s,size);if(align==1)x-=w;else if(align==2)x-=w/2;
 constexpr int offsets[]={15,22,29,36};
 for(unsigned char c:s){const auto& a=menu_art::images[size*130+(c>=32 && c<=126?c-32:'?'-32)];blit(a.record,a.x,a.y,a.w,a.h,x,y,a.w,a.h,font_palette_,offsets[ink]);x+=a.w+1;}
}
void OriginalMenuArt::button(int x,int y,int w,const std::string& label,bool selected,double clock){
 const int frame=selected?int(clock*12)%12:0;
 const auto& sprite=menu_art::images[(selected?menu_art::BUTN_WHTE:menu_art::BUTN_BLUE)+frame];
 const auto& page=entries_[sprite.record];
 // A light, one-pixel coverage filter smooths both the rim and rotating detail
 // in the original sprites. Sample only this frame, never its atlas neighbours.
 // Composite premultiplied colour/coverage before returning to straight RGBA.
 constexpr int kernel[]={1,4,1},weight=36;
 for(int yy=std::max(-1,-y);yy<std::min(sprite.h+1,480-y);++yy)
  for(int xx=std::max(-1,-x);xx<std::min(sprite.w+1,640-x);++xx){
   int coverage=0,rgb[3]{};
   for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx){
    const int sx=xx+dx,sy=yy+dy;
    if(sx<0 || sy<0 || sx>=sprite.w || sy>=sprite.h)continue;
    const int index=page[(sprite.y+sy)*640+sprite.x+sx];if(!index)continue;
    const int tap=kernel[dx+1]*kernel[dy+1];coverage+=tap;
    for(int c=0;c<3;++c)rgb[c]+=palette_[index*3+c]*255/63*tap;
   }
   if(!coverage)continue;
   const int to=((y+yy)*640+x+xx)*4,dest_alpha=frame_[to+3];
   const int alpha=coverage*255+(weight-coverage)*dest_alpha;
   for(int c=0;c<3;++c)frame_[to+c]=static_cast<std::uint8_t>((rgb[c]*255+frame_[to+c]*dest_alpha*(weight-coverage)+alpha/2)/alpha);
   frame_[to+3]=static_cast<std::uint8_t>((alpha+weight/2)/weight);
  }
 text(x+48,y+5,label,selected?cyan:white,1,0,w-48);
}
void OriginalMenuArt::country(int team){
 country_frame(menu_art::aim[std::clamp(team,0,15)]*4);
}
void OriginalMenuArt::country_frame(int frame){
 const int start=(frame&63)*8;
 for(int i=0;i<8;++i){const auto& c=menu_art::carousel[start+i];if(c.image>=0)image(560+c.image,c.x,c.y+26);}
}
void OriginalMenuArt::present(SDL_Renderer* r,SDL_FRect view){
 if(owner_!=r || !texture_){
  SDL_DestroyTexture(texture_);texture_=SDL_CreateTexture(r,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_STREAMING,640,480);
  if(!texture_)throw std::runtime_error(SDL_GetError());owner_=r;
  if(!SDL_SetTextureScaleMode(texture_,SDL_SCALEMODE_LINEAR) || !SDL_SetTextureBlendMode(texture_,SDL_BLENDMODE_BLEND_PREMULTIPLIED))throw std::runtime_error(SDL_GetError());
 }
 // Smooth the original bitmap font at fractional window/DPI scales. Premultiply
 // before filtering so transparent edges don't acquire a dark fringe; keep the
 // authored pixels and layout unchanged in the CPU composition.
 if(!SDL_PremultiplyAlpha(640,480,SDL_PIXELFORMAT_RGBA32,frame_.data(),640*4,SDL_PIXELFORMAT_RGBA32,upload_.data(),640*4,false)
  || !SDL_UpdateTexture(texture_,nullptr,upload_.data(),640*4) || !SDL_RenderTexture(r,texture_,nullptr,&view))throw std::runtime_error(SDL_GetError());
}
}
