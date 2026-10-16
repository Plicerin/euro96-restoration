#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "graphics_data.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <cstdint>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::uint64_t pixels(SDL_Surface* surface){
 const auto* data=static_cast<const unsigned char*>(surface->pixels);std::uint64_t hash=1469598103934665603ull;
 for(int y=116;y<596;++y)for(int x=24*4;x<664*4;++x){hash^=data[y*surface->pitch+x];hash*=1099511628211ull;}
 return hash;
}
static void rotation(){
 original::PlayerEngine engine;OriginalPresentation view;engine.ball_horz_ang=3.1f;engine.ball_vert_ang=3.1f;view.observe(engine,1,0);
 engine.ball_horz_ang=-3.1f;engine.ball_vert_ang=-3.1f;view.observe(engine,1,1);
 const auto r=view.ball_rotation(.5f);
 require(std::abs(r[0])<.001f && r[1]>.999f && r[2]<-.999f && std::abs(r[3])<.001f,"Ball rotation must cross angle wrap without reversing a full turn");
 for(float alpha:{0.0f,.25f,.5f,.75f,1.0f}){const auto v=view.ball_rotation(alpha);require(std::abs(v[0]*v[0]+v[1]*v[1]-1)<.0001f && std::abs(v[2]*v[2]+v[3]*v[3]-1)<.0001f,"Ball transforms must remain normalized");}
 legacy::Core core;core.camera(640,40,490,640,0,400,500,640,480);
 legacy::obj ball{recovered::footy_p,recovered::footy_f,640,2,-400,1,0};core.addobjxy(&ball,1,0);
 const float x=core.rotated[1],y=core.rotated[2];core.clear();core.addobjxy(&ball,0,1);
 require(std::abs(core.rotated[1]-x)+std::abs(core.rotated[2]-y)>.1f,"Original two-axis transform must actually rotate the ball geometry");
}
static void goal_details(){
 legacy::Core core;core.rot.e.r20=0;core.rot.e.r21=0;core.rot.e.r22=1;core.rot.e.r23=0;core.q=100;
 for(auto& levels:recovered::goals){
  int level=0;for(float depth:{500.0f,350.0f,200.0f,100.0f,50.0f}){
   legacy::obj goal{nullptr,nullptr,0,0,depth,1,0};core.objdepd(&goal,levels);
   require(goal.points==levels[level].dpoints && goal.faces==levels[level].dfaces,"Original camera depth must choose the preserved goal detail entry");
   require(goal.points[0]>=4 && goal.faces[0]>=2,"Every goal detail must include its geometry and net faces");++level;
  }
 }
}
static void halfway_clipping(){
 // Looking across midfield from either side: the nearby half's origin is
 // behind the cutoff, but its endpoint at the centre spot is still on screen.
 for(bool opposite:{false,true}){
  legacy::Core core;core.camera(640,100,opposite?480.0f:320.0f,640,0,400,440,640,480);
  legacy::obj half{opposite?recovered::l6_p1:recovered::l5_p1,recovered::l_f3a,640,0,opposite?-600.0f:-200.0f,1,0};
  core.addobjy(&half);require(core.curpol==core.polygons.data(),"Fixture must reproduce discarded halfway segment");
  core.clear();core.addobjfc(&half);
  require(core.curpol-core.polygons.data()==1,"Visible near half must survive with its offscreen touchline clipped");
  const auto* line=core.polygons[0].pts;require(line[0]==2,"Halfway marking must remain a line");
  require(std::abs(float(line[4])-320)<2 && std::abs(float(line[5])-240)<2,"Clipped halfway line must still reach the centre spot");
  core.clear();half.points=opposite?recovered::l5_p1:recovered::l6_p1;half.z=opposite?-200.0f:-600.0f;core.addobjfc(&half);
  require(core.curpol-core.polygons.data()==2,"Far halfway segment and touchline must both remain");
 }
}
static void textured_clipping(){
 // One corner behind the original near plane produces five vertices and ten UV words.
 for(bool transparent:{false,true}){
  legacy::Core core;core.rot.e.r00=core.rot.e.r11=core.rot.e.r22=1;
  core.q=100;core.persc=100.0f/15;core.render_buff.clip_xmid=320;core.render_buff.clip_ymid=240;
  core.clip_base=core.clip_tex=2;
  const legacy::dword uv[8]={1<<24,(1<<24)|(128<<16),(1<<24)|(128<<16),1<<24,0,0,128<<16,128<<16};
  std::copy_n(uv,8,core.textures[1]);
  float points[]={4,-2,-2,10,-2,2,30,2,2,30,2,-2,30};
  short faces[]={1,4,short(transparent?-2001:-1),0,1,2,3};
  legacy::obj object{points,faces,0,0,0,1,0};core.addobjnc(&object);
  require(core.curpol-core.polygons.data()==1 && core.polygons[0].pts[0]==5,"Textured near-plane clipping must retain the visible pentagon");
  require(core.polygons[0].pts[1]==(transparent?-2002:-2) && core.clip_tex==4,"Clipped polygon must use two UV scratch records and retain transparency");
  const auto* clipped=core.textures[2];
  for(int i=0;i<5;++i){require((clipped[i]>>24)==1,"Clipping must preserve the texture page");require((clipped[i]&0xffffff)<=(128<<16) && clipped[5+i]>=0 && clipped[5+i]<=(128<<16),"Clipped UV coordinates must remain within the original face");}
  require(std::equal(uv,uv+8,core.textures[1]),"Clipping must preserve source texture coordinates");
  core.clear();require(core.clip_tex==2,"UV scratch resets between render batches");
 }
}
static void shirt_materials(){
 for(int number=1;number<=22;++number)for(bool mirrored:{false,true}){
  const auto* original=OriginalGraphics::player_faces(number,mirrored,false);
  const short old_material=original[82];const auto faces=OriginalGraphics::numbered_player_faces(number,mirrored);
  for(int shirt=12;shirt<=15;++shirt){
   const auto bench=OriginalGraphics::numbered_player_faces(number,mirrored,shirt);
   require(bench[82]==-2000-(number<=11?549:564)-(shirt-1),"Substitute must retain actual squad shirt, including away and mirrored faces");
  }
  const int expected=(number<=11?549:564)+(number-1)%11;
  require(faces[82]==-2000-expected,"Shirt face must use its team's number atlas, never the head page");
  require(original[82]==old_material,"Per-player numbers must not mutate a shared face table");
  for(std::size_t i=0;i<faces.size();++i)if(i!=82)require(faces[i]==original[i],"Number binding must preserve body geometry and other materials");
 }
}
int main(int argc,char** argv){try{
 require(argc>1,"Data path required");rotation();goal_details();halfway_clipping();textured_clipping();shirt_materials();require(SDL_Init(SDL_INIT_VIDEO),"SDL init failed");
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer!=nullptr,"Software renderer failed");
 {
 OriginalGraphics g(argv[1]);g.action_view=true;
 // Same pose/location, different original material. A missing or shared texture
 // produces equal pixels and fails these checks even if meshes were submitted.
 for(int kit=0;kit<16;++kit){
  g.change_kit(kit?1:0);g.sequence->reset();auto& e=g.sequence->engine;g.sequence->visible_players={1};e.ball_poss=0;e.ball_limbo_on=0;
  auto& p=e.teams[1];p.tm_x=640;p.tm_y=400;p.tm_xdis=1;p.tm_ydis=0;p.tm_anim=78;
  for(bool mirrored:{false,true}){
   p.tm_anim=mirrored?39:78;p.tm_frm=.25f;
   std::array<std::uint64_t,4> hashes{};int index=0;
   for(int number:{2,13,1,12}){
    p.tm_player=short(number);++g.sequence->generation;g.draw(renderer);SDL_RenderPresent(renderer);hashes[index++]=pixels(surface);
    require(g.scene_counts.markings==13 && g.scene_counts.goal_parts==8,"Full scene must include markings and all eight goal components");
    require(g.scene_counts.player_shadows==1 && g.scene_counts.ball_shadows==1 && g.scene_counts.rotating_balls==1,"Player and ball need their original shadow and ball transform");
   }
   require(hashes[0]!=hashes[1],"Home and away uniforms must produce distinct pixels");
   require(hashes[0]!=hashes[2] && hashes[1]!=hashes[3],"Keeper materials must differ from outfield uniforms");
   // Rear view: changing only the shirt number must change visible pixels.
   g.angle=3.14159265f;
   for(int first:{2,13}){
    p.tm_player=short(first);++g.sequence->generation;g.draw(renderer);SDL_RenderPresent(renderer);const auto a=pixels(surface);
    p.tm_player=short(first+1);++g.sequence->generation;g.draw(renderer);SDL_RenderPresent(renderer);
    require(a!=pixels(surface),"Consecutive home/away numbers must visibly differ, including mirrored poses");
   }
   g.angle=.4f;
  }
 }
 g.change_kit(-15);g.sequence->start_continuation();g.advance(0);
 for(int tick=0;tick<230;++tick){
  const auto& e=g.sequence->engine;const float x=e.ballx,y=e.bally,z=e.ballz;const int t=g.sequence->ticks;
  g.draw(renderer);SDL_RenderPresent(renderer);
  require(x==e.ballx && y==e.bally && z==e.ballz && t==g.sequence->ticks,"Scene rendering must not change original gameplay");
  require(g.scene_counts.home_players==3 && g.scene_counts.away_players==2 && g.scene_counts.keepers==1,"Fixture must use both teams and the goalkeeper material");
  if(argc>2 && (tick==59 || tick==180 || tick==215)){
   const auto path=std::filesystem::path(argv[2])/("scene-"+std::to_string(tick)+".png");require(SDL_SavePNG(surface,path.string().c_str()),"Scene capture failed");
  }
  g.advance(.05);
 }
 require(g.sequence->catch_tick==59 && g.sequence->distribution_tick==171 && g.sequence->receive_tick==208 && g.sequence->ticks==229,"Scene batch must preserve the approved action chain");
 // Orbit both ends and the stadium. Original nets are two-sided; all views must render.
 for(bool wide:{false,true})for(int angle=0;angle<8;++angle){g.overview=wide;g.angle=float(angle)*.785398f;g.draw(renderer);SDL_RenderPresent(renderer);require(g.polygon_count()>30,"Scene orbit lost its geometry");}
 if(argc>2){
  g.action_view=false;g.overview=false;g.angle=.4f;
  for(std::size_t i=0;i<g.clips.size();++i)if(g.clips[i].animation==72)g.clip=int(i);
  g.frame=12;g.draw(renderer);SDL_RenderPresent(renderer);
  const auto path=std::filesystem::path(argv[2])/"shirt-number-closeup.png";require(SDL_SavePNG(surface,path.string().c_str()),"Shirt close-up capture failed");
 }
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();
 std::cout<<"Sixteen kit pairs, both keeper materials, mirror variants, full markings/goals, shadows, ball rotation, camera orbit and unchanged action timings passed.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';SDL_Quit();return 1;}}
