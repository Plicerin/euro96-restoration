#include "original_presentation.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
namespace actua {
OriginalPresentation::Snapshot OriginalPresentation::snapshot(const original::PlayerEngine& e){
 Snapshot result;std::copy(e.teams.begin(),e.teams.end(),result.players.begin());
 for(int i=0;i<3;++i){const auto& ref=e.refs[i];auto& p=result.players[22+i];
  p.tm_player=short(23+i);p.guy_on=ref.fstep>0;p.draw_me=1;p.tm_x=ref.x;p.tm_y=ref.y;p.tm_z=ref.z;
  p.tm_xdis=ref.dir_x;p.tm_ydis=ref.dir_y;p.tm_anim=static_cast<unsigned short>(ref.anim);p.tm_frm=ref.frm;p.tm_fstep=ref.fstep;
 }
 result.x=e.ballx;result.y=e.bally;result.z=e.ballz;result.owner=e.ball_poss?e.ball_poss:e.ball_limbo_on?e.ball_limbo_p:0;
 result.ball_horizontal=e.ball_horz_ang;result.ball_vertical=e.ball_vert_ang;return result;
}
OriginalPresentation::Pose OriginalPresentation::capture(const original::PlayerEngine& e,const original::match_player& p){
 const auto& clip=e.mcaps.at(std::size_t(p.tm_anim));
 if(clip.capfrms<=0)throw std::runtime_error("Unrestored presentation capture "+std::to_string(p.tm_anim)+" for player "+std::to_string(p.tm_player));
 const float phase=p.tm_frm-std::floor(p.tm_frm);
 const float position=phase*clip.capfrms;
 const int frame=std::clamp(int(position),0,int(clip.capfrms)-1);
 auto pose=e.player_p.at(std::size_t(clip.cappts+frame));
 const auto& next=e.player_p.at(std::size_t(clip.cappts+std::min(frame+1,int(clip.capfrms)-1)));
 // Native subframe sampling: never interpolate the absent-ball sentinel as geometry.
 for(int i=1;i<85;++i){
  if(i>=70 && i<=72 && (pose[71]<0 || next[71]<0))continue;
  pose[i]+=(next[i]-pose[i])*(position-frame);
 }
 return pose;
}
original::match_player OriginalPresentation::interpolate(const original::match_player& old,const original::match_player& current,float alpha){
 // 3D_UPD2.CPP realtime_coords: positions, shortest-arc facing and forward frame wrap.
 alpha=std::clamp(alpha,0.0f,1.0f);auto p=current;
 p.tm_x=old.tm_x+(current.tm_x-old.tm_x)*alpha;
 p.tm_y=old.tm_y+(current.tm_y-old.tm_y)*alpha;
 p.tm_z=old.tm_z+(current.tm_z-old.tm_z)*alpha;
 float difference=std::acos(std::clamp(current.tm_xdis*old.tm_xdis+current.tm_ydis*old.tm_ydis,-1.0f,1.0f))*alpha;
 if(current.tm_xdis*old.tm_ydis<current.tm_ydis*old.tm_xdis)difference=-difference;
 p.tm_xdis=old.tm_xdis*std::cos(difference)+old.tm_ydis*std::sin(difference);
 p.tm_ydis=old.tm_ydis*std::cos(difference)-old.tm_xdis*std::sin(difference);
 if(current.tm_anim==old.tm_anim && !current.tm_newanim){
  float frame=current.tm_frm;if(frame<old.tm_frm)frame+=1;
  p.tm_frm=old.tm_frm+(frame-old.tm_frm)*alpha;
 }
 return p;
}
OriginalPresentation::Player OriginalPresentation::player(const original::PlayerEngine& e,int index,float alpha)const{
 if(!ready)throw std::logic_error("Presentation not initialized");
 alpha=std::clamp(alpha,0.0f,1.0f);const auto i=std::size_t(index);
 Player result;result.state=interpolate(before.players.at(i),after.players.at(i),alpha);
 result.mirrored=mirrored(result.state.tm_anim);result.pose=capture(e,result.state);
 const auto& t=transitions.at(i);
 if(t.active){
  // 3DENG.C rate: fstep * 2 * elapsed logic cycles. Absolute time avoids FPS drift.
  const float amount=std::clamp(float((tick-1+double(alpha)-t.start)*t.rate),0.0f,1.0f);
  if(amount<1){
   auto target=result.pose;legacy::blend_original_pose(result.pose.data(),target.data(),t.from.data(),amount,result.mirrored,mirrored(t.source_animation));
   if(target[71]<0 || t.from[71]<0)for(int coordinate=70;coordinate<=72;++coordinate)result.pose[coordinate]=target[coordinate];
  }
 }
 return result;
}
void OriginalPresentation::observe(const original::PlayerEngine& e,std::uint64_t next_generation,int next_tick){
 if(!ready || generation!=next_generation || next_tick<tick || next_tick>tick+1){
  before=after=snapshot(e);transitions={};ball_handoff=false;generation=next_generation;tick=next_tick;ready=true;return;
 }
 if(next_tick==tick)return;
 const auto previous_ball=displayed_ball(e,1);
 const bool was_captured=after.owner && player(e,after.owner-1,1).pose[71]>=0;
 auto next=snapshot(e);
 for(std::size_t i=0;i<25;++i){
  if(next.players[i].guy_on && after.players[i].guy_on && next.players[i].tm_anim!=after.players[i].tm_anim){
   const auto from=player(e,int(i),1).pose;
   transitions[i]={from,after.players[i].tm_anim,double(tick),next.players[i].tm_fstep*2,true};
  }
 }
 before=after;after=next;tick=next_tick;
 const bool is_captured=after.owner && player(e,after.owner-1,1).pose[71]>=0;
 ball_handoff=before.owner!=after.owner || was_captured!=is_captured ||
  (after.owner && before.players[after.owner-1].tm_anim!=after.players[after.owner-1].tm_anim);
 handoff_from=previous_ball;handoff_to=attached_ball(e,1);
}
OriginalPresentation::Ball OriginalPresentation::ball(float alpha)const{
 alpha=std::clamp(alpha,0.0f,1.0f);
 // Ownership switches at the simulation boundary, never midway through a visual release.
 return {before.x+(after.x-before.x)*alpha,before.y+(after.y-before.y)*alpha,before.z+(after.z-before.z)*alpha,
         alpha<1 && before.owner!=after.owner?before.owner:after.owner};
}
std::array<float,4> OriginalPresentation::ball_rotation(float alpha)const{
 constexpr float pi=3.1415926536f;
 alpha=std::clamp(alpha,0.0f,1.0f);
 const float h=before.ball_horizontal+std::remainder(after.ball_horizontal-before.ball_horizontal,2*pi)*alpha-pi/2;
 const float v=before.ball_vertical+std::remainder(after.ball_vertical-before.ball_vertical,2*pi)*alpha;
 // 3D_UPD2.CPP new_update_coords -> 3DENG.C addobjxy.
 return {std::cos(h),std::sin(h),std::cos(v),std::sin(v)};
}
OriginalPresentation::Ball OriginalPresentation::attached_ball(const original::PlayerEngine& e,float alpha)const{
 auto result=ball(alpha);
 if(result.owner){
  const auto v=player(e,result.owner-1,alpha);
  if(v.pose[71]>=0){
   const float lateral=v.mirrored?-v.pose[72]:v.pose[72];
   result.x=v.pose[70]*v.state.tm_xdis+lateral*v.state.tm_ydis+v.state.tm_x;
   result.y=v.pose[70]*v.state.tm_ydis-lateral*v.state.tm_xdis+v.state.tm_y;
   result.z=v.pose[71]+v.state.tm_z;
  }
 }
 return result;
}
OriginalPresentation::Ball OriginalPresentation::displayed_ball(const original::PlayerEngine& e,float alpha)const{
 if(!ball_handoff)return attached_ball(e,alpha);
 // Native visual handoff: join the last captured position to the next physics
 // position over this display interval; neither endpoint changes the simulation.
 alpha=std::clamp(alpha,0.0f,1.0f);
 return {handoff_from.x+(handoff_to.x-handoff_from.x)*alpha,
         handoff_from.y+(handoff_to.y-handoff_from.y)*alpha,
         handoff_from.z+(handoff_to.z-handoff_from.z)*alpha,ball(alpha).owner};
}
}
