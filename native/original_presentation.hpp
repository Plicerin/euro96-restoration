#pragma once
#include "player_original.hpp"
#include <array>
#include <cstdint>
namespace actua::legacy {
void blend_original_pose(float*,const float*,const float*,float,bool,bool);
}
namespace actua {
// Render-only state. Sampling is pure: extra draws never advance gameplay or blending.
class OriginalPresentation {
public:
 using Pose=std::array<float,85>;
 struct Player {original::match_player state{};Pose pose{};bool mirrored=false;};
 struct Ball {float x=0,y=0,z=0;int owner=0;};
 void observe(const original::PlayerEngine&,std::uint64_t generation,int tick);
 Player player(const original::PlayerEngine&,int index,float alpha)const;
 Ball ball(float alpha)const;
 Ball displayed_ball(const original::PlayerEngine&,float alpha)const;
 std::array<float,4> ball_rotation(float alpha)const;
 static original::match_player interpolate(const original::match_player&,const original::match_player&,float);
 static bool mirrored(int animation){return (animation&1) && animation>=8 && animation<=69;}
private:
 struct Snapshot {std::array<original::match_player,25> players{};float x=0,y=0,z=0;int owner=0;float ball_horizontal=0,ball_vertical=0;};
 struct Transition {Pose from{};int source_animation=0;double start=0;float rate=0;bool active=false;};
 Snapshot before{},after{};
 Ball handoff_from{},handoff_to{};
 bool ball_handoff=false;
 std::array<Transition,25> transitions{};
 std::uint64_t generation=0;int tick=0;bool ready=false;
 static Pose capture(const original::PlayerEngine&,const original::match_player&);
 static Snapshot snapshot(const original::PlayerEngine&);
 Ball attached_ball(const original::PlayerEngine&,float alpha)const;
};
}
