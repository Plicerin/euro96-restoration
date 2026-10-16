#pragma once
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace actua {
// Presentation-only polygon clipping. Reciprocal camera depth is linear on the
// screen, so intersecting faces need not rely on their average sorting depth.
class OriginalOcclusion {
public:
 struct Vertex { SDL_Vertex draw{}; float depth=0; };
 using Polygon=std::vector<Vertex>;
 struct Plane {
  float x=0,y=0,c=0;
  float at(const Vertex& v)const{return x*v.draw.position.x+y*v.draw.position.y+c;}
 };
 struct Cover {std::array<Plane,3> edges;Plane depth;};
 std::vector<Cover> covers;
 static float cross(SDL_FPoint a,SDL_FPoint b,SDL_FPoint c){return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);}
 static bool drawable(const Polygon& p){
  float area=0;for(std::size_t i=1;i+1<p.size();++i)area+=std::abs(cross(p[0].draw.position,p[i].draw.position,p[i+1].draw.position));return area>.001f;
 }
 static Vertex mix(const Vertex& a,const Vertex& b,float t){
  Vertex v=a;const auto lerp=[t](float a,float b){return a+(b-a)*t;};
  v.draw.position={lerp(a.draw.position.x,b.draw.position.x),lerp(a.draw.position.y,b.draw.position.y)};
  v.draw.tex_coord={lerp(a.draw.tex_coord.x,b.draw.tex_coord.x),lerp(a.draw.tex_coord.y,b.draw.tex_coord.y)};
  v.depth=lerp(a.depth,b.depth);return v;
 }
 template<class Distance> static Polygon clip(const Polygon& p,Distance distance,bool inside){
  Polygon out;if(p.empty())return out;
  auto a=p.back();float da=distance(a);if(!inside)da=-da;
  for(const auto& b:p){float db=distance(b);if(!inside)db=-db;
   if((da>=0)!=(db>=0))out.push_back(mix(a,b,da/(da-db)));
   if(db>=0)out.push_back(b);a=b;da=db;
  }return out;
 }
 void add(const std::array<Vertex,3>& v){
  const auto a=v[0].draw.position,b=v[1].draw.position,c=v[2].draw.position;
  const float area=cross(a,b,c);if(std::abs(area)<.01f)return;
  Cover cover{};const float sign=area>0?1.0f:-1.0f;
  for(int i=0;i<3;++i){const auto p=v[i].draw.position,q=v[(i+1)%3].draw.position;
   cover.edges[i]={sign*(p.y-q.y),sign*(q.x-p.x),sign*(q.y*p.x-q.x*p.y)};}
  const float d1=v[1].depth-v[0].depth,d2=v[2].depth-v[0].depth;
  cover.depth.x=(d1*(c.y-a.y)-d2*(b.y-a.y))/area;
  cover.depth.y=((b.x-a.x)*d2-(c.x-a.x)*d1)/area;
  cover.depth.c=v[0].depth-cover.depth.x*a.x-cover.depth.y*a.y;
  covers.push_back(cover);
 }
 std::vector<Polygon> visible(Polygon polygon)const{
  std::vector<Polygon> pieces{std::move(polygon)};
  for(const auto& cover:covers){
   std::vector<Polygon> next;
   for(auto& piece:pieces){
    bool overlap=true;
    for(const auto& edge:cover.edges)if(std::all_of(piece.begin(),piece.end(),[&](const auto& v){return edge.at(v)<0;})){overlap=false;break;}
    const auto behind=[&](const Vertex& v){return cover.depth.at(v)-v.depth-0.000001f;};
    if(!overlap || std::none_of(piece.begin(),piece.end(),[&](const auto& v){return behind(v)>0;})){next.push_back(std::move(piece));continue;}
    // Remove only the intersection of the cover triangle AND its nearer half-space.
    for(const auto& edge:cover.edges){
     auto outside=clip(piece,[&](const Vertex& v){return edge.at(v);},false);
     if(drawable(outside))next.push_back(std::move(outside));
     piece=clip(piece,[&](const Vertex& v){return edge.at(v);},true);if(!drawable(piece))break;
    }
    if(drawable(piece)){auto front=clip(piece,behind,false);if(drawable(front))next.push_back(std::move(front));}
   }
   pieces=std::move(next);if(pieces.empty())break;
  }return pieces;
 }
};
}
