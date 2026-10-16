#pragma once
#include "player_original.hpp"
#include "archive.hpp"
namespace actua {
struct CameraPose {float x=640,y=860,z=180,tx=640,ty=400,tz=0,scale=512;};
struct CameraMotion {float dx=0,dy=0,cxs=0,cys=0;};
CameraPose original_camera(int mode,const original::PlayerEngine& engine,float x,float y,float z,const CameraPose& previous,CameraMotion* motion=nullptr);
}
