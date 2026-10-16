#include "player.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using actua::PlayerPractice;
using actua::original::PlayerEngine;
static int checks=0;
static void expect(bool v,const char* m) {++checks;if(!v)throw std::runtime_error(m);}
static void near(float a,float b,const char* m) {expect(std::abs(a-b)<0.0003f,m);}
int main(int argc,char** argv) {
 try {
  if(argc==2 && std::string(argv[1])=="--movement-trace") {
   for(int rate:{0,32,64,96,127}) for(int mode=0;mode<5;++mode) {
    PlayerEngine e;auto& p=e.teams[1];p.tm_player=2;p.tm_rate=static_cast<unsigned char>(rate);p.tm_xdis=1;p.tm_x=500;p.tm_y=400;
    p.go_tx=1200;p.go_ty=400;e.user_controlled=1;
    if(mode==1)e.ball_poss=2;
    if(mode==2){p.go_step=1;p.go_txdis=2;p.go_tydis=-1;}
    if(mode==3){p.int_move=1;p.int_cnt=5;e.ball_poss=2;}
    if(mode==4)e.burst_timer[0]=10;
    std::cout.precision(9);
    for(int tick=1;tick<=20;++tick){e.go_forward(&p);std::cout<<rate<<','<<mode<<','<<tick<<','<<p.tm_x<<','<<p.tm_y<<','<<p.go_txdis<<','<<p.go_tydis<<','<<p.tm_work<<'\n';}
   }return 0;
  }
  PlayerPractice p;
  expect(p.engine.ball_poss==2,"Starts with selected player's possession");
  near(p.engine.ballx,510,"Original held ball offset");
  float x=p.engine.teams[1].tm_x;p.set_input(1,0);for(int i=0;i<20;++i)p.step();
  near(p.engine.teams[1].tm_x-x,20*(1280.0f/(14*20)),"Original running speed with possession");
  expect(p.engine.teams[1].tm_work==20,"Original work statistic increments");
  expect(p.engine.ballx>p.engine.teams[1].tm_x,"Original dribbling ball follows facing direction");
  p.reset();p.set_input(1,1);for(int i=0;i<20;++i)p.step();
  expect(std::hypot(p.engine.teams[1].tm_x-500,p.engine.teams[1].tm_y-400)<=20*(1280.0f/280)+0.001f,"Diagonal input cannot exceed original run speed");
  p.reset();expect(p.pass(),"Ground pass allowed in possession");
  expect(!p.engine.ball_poss && p.selected==2,"Pass releases ball and selects receiver");
  expect(!p.shoot(),"Cannot shoot without possession");
  for(int i=0;i<60 && !p.engine.ball_poss;++i)p.step();
  expect(p.engine.ball_poss==3,"Teammate receives ground pass");
  expect(p.passes_received==1,"Reception count");
  expect(p.engine.teams[1].tm_pass==1 && p.engine.teams[1].tm_pwon==1,"Original successful pass statistics");
  expect(p.pass(),"Receiver can return pass");
  for(int i=0;i<60 && !p.engine.ball_poss;++i)p.step();
  expect(p.engine.ball_poss==2 && p.passes_received==2,"Return pass collected by original sender");
  p.reset();expect(p.pass(true),"Lofted pass allowed");
  expect(p.engine.ball_inair && p.engine.ballzdis>0,"Loft uses airborne trajectory");
  for(int i=0;i<160 && !p.engine.ball_poss && !p.outside;++i)p.step();
  expect(p.engine.ball_poss==3,"Receiver collects lofted pass on descent");
  p.reset();p.aim_at(1000,200);expect(p.shoot(),"Facing shot allowed");
  expect(p.engine.ballxdis>0 && p.engine.ballydis<0 && p.engine.ballzdis==6,"Shot follows aim and explicit practice preset");
  expect(p.engine.teams[1].tm_shots==1,"Shot statistic increments");
  p.step();expect(!p.engine.ball_poss,"Shooter cannot immediately recapture the shot");
  p.paused=true;const auto ticks=p.ticks;const auto ballx=p.engine.ballx;p.step();
  expect(p.ticks==ticks && p.engine.ballx==ballx,"Pause freezes practice");
  p.reset();p.set_input(-1,-1);for(int i=0;i<500;++i)p.step();
  expect(!p.outside && p.engine.teams[1].tm_x>=12 && p.engine.teams[1].tm_y>=12,"Player stays on pitch and dribbles inside boundary");
  p.reset();p.engine.holder_lose_ball();p.engine.ballx=1279;p.engine.bally=200;p.engine.ballxdis=20;p.engine.ballydis=0;p.step();
  expect(p.outside,"Free ball leaving pitch stops practice");p.reset();expect(!p.outside && p.engine.ball_poss==2 && p.ticks==0,"Reset restores usable session");
  p.reset();p.switch_player();expect(p.selected==2 && p.engine.ball_poss==2,"Selection does not teleport possession");
  auto& target=p.engine.teams[2];target.tm_x=600;target.tm_y=400;
  p.engine.holder_lose_ball();p.engine.ballx=580;p.engine.bally=400;p.engine.ballz=2;p.engine.ballxdis=40;p.engine.ballydis=0;p.step();
  expect(p.engine.ball_poss==3,"Swept practice contact cannot skip receiver between ticks");
  p.reset();p.engine.holder_lose_ball();p.engine.ballx=730;p.engine.bally=400;p.engine.ballz=30;p.engine.ballxdis=10;p.engine.ball_inair=1;p.step();
  expect(!p.engine.ball_poss,"High ball is not magnetically collected");
  // Keeping the passer's movement held must not run the newly selected receiver away.
  p.reset();p.set_input(0,1);p.step();expect(p.pass(),"Moving player can pass");
  for(int i=0;i<100 && !p.engine.ball_poss;++i){p.set_input(0,1);p.step();}
  expect(p.engine.ball_poss==3,"Held passer input must not carry the receiver away from the pass");
  near(p.engine.teams[2].tm_y,400,"Held direction protects receiver until contact");
  p.set_input(0,0);p.set_input(0,1);p.step();
  expect(p.engine.teams[2].tm_y>400,"Receiver moves after releasing and pressing direction again");
  p.reset();p.set_input(0,1);p.step();
  for(int exchange=0;exchange<4;++exchange) {
   const int receiver=p.selected==1?3:2;p.pass();
   for(int i=0;i<100 && !p.engine.ball_poss;++i){p.set_input(0,1);p.step();}
   expect(p.engine.ball_poss==receiver,"Each return pass protects its receiver during flight");
  }
  // A low ball within foot reach should be trapped, but distant/high balls stay free.
  for(float offset:{-14.0f,14.0f,24.0f}) {
   p.reset();p.engine.holder_lose_ball();p.engine.ballx=710;p.engine.bally=400+offset;
   p.engine.ballz=2;p.engine.ballxdis=60;p.engine.ballydis=0;p.step();
   expect((p.engine.ball_poss==3)==(std::abs(offset)<16),"Reception respects bounded foot reach");
  }
  p.reset();p.engine.holder_lose_ball();p.engine.ballx=710;p.engine.bally=400;
  p.engine.ballz=14;p.engine.ballzdis=-6;p.engine.ball_inair=1;p.engine.ballxdis=60;p.engine.ballydis=0;p.step();
  expect(p.engine.ball_poss==3,"Descending ball is collected when contact becomes low enough during the tick");
  // Exercise ordinary short/long passes and held movement in every direction.
  for(bool loft:{false,true}) for(int distance:{64,120,240,400,600,800}) {
   p.reset();p.engine.teams[1].tm_x=200;p.engine.teams[2].tm_x=200+float(distance);p.step();p.pass(loft);
   for(int i=0;i<200 && !p.engine.ball_poss && !p.outside;++i)p.step();
   if(p.engine.ball_poss!=3) std::cerr<<"Missed distance "<<distance<<" loft "<<loft<<" ball "<<p.engine.ballx<<","<<p.engine.bally<<","<<p.engine.ballz<<" ticks "<<p.ticks<<'\n';
   expect(p.engine.ball_poss==3,"Short and long practice passes can be received");
  }
  for(bool loft:{false,true}) for(int direction=0;direction<8;++direction) {
   const float angle=float(direction)*3.1415926536f/4;
   const float ix=std::cos(angle),iy=std::sin(angle);
   p.reset();p.set_input(ix,iy);for(int i=0;i<5;++i)p.step();p.pass(loft);
   for(int i=0;i<200 && !p.engine.ball_poss && !p.outside;++i){p.set_input(ix,iy);p.step();}
   expect(p.engine.ball_poss==3,"Passes while moving in every direction reach a waiting teammate");
  }
  // Fresh receiver input is still honored; there is no homing or forced catch.
  p.reset();p.pass();p.set_input(0,1);for(int i=0;i<60 && !p.outside;++i)p.step();
  expect(!p.engine.ball_poss && p.engine.teams[2].tm_y>450,"Fresh input can deliberately move the receiver away");
  // Player and ball paths intersect early in the tick, before the player moves away.
  p.reset();p.switch_player();p.engine.holder_lose_ball();p.engine.teams[2].tm_xdis=0;p.engine.teams[2].tm_ydis=1;
  p.engine.ballx=739;p.engine.bally=385;p.engine.ballxdis=100;p.engine.ballydis=0;p.engine.ballz=2;
  p.set_input(0,1);p.step();expect(p.engine.ball_poss==3,"Contact uses the moving receiver's path");
  // A ball that only becomes low after passing the receiver must remain free.
  p.reset();p.engine.holder_lose_ball();p.engine.ballx=720;p.engine.bally=400;
  p.engine.ballz=30;p.engine.ballzdis=-25;p.engine.ball_inair=1;p.engine.ballxdis=100;p.engine.ballydis=0;p.step();
  expect(!p.engine.ball_poss,"Low-height window must overlap the horizontal contact window");
  PlayerPractice independent;p.set_input(1,0);p.step();near(independent.engine.teams[1].tm_x,500,"Player sessions do not share state");
  bool threw=false;try{p.engine.get_mcball_coords(&p.engine.teams[1]);}catch(const std::logic_error&){threw=true;}expect(threw,"Unsupported animation path fails explicitly");
  // Original missing ball.dta contact offsets recovered from ACTIONS.OBJ.
  p.reset();auto& contact=p.engine.teams[1];contact.tm_anim=0;contact.tm_x=100;contact.tm_y=200;contact.tm_z=3;
  contact.tm_xdis=1;contact.tm_ydis=0;p.engine.ball_at_contact(&contact);
  near(p.engine.ballx,100.70994f,"Original contact X offset");near(p.engine.bally,203.435731f,"Original contact Y sign");near(p.engine.ballz,7.18586f,"Original contact height plus player height");
  contact.tm_xdis=0;contact.tm_ydis=1;p.engine.ball_at_contact(&contact);
  near(p.engine.ballx,96.564269f,"Original contact rotates with facing");near(p.engine.bally,200.70994f,"Original rotated contact Y");
  for(int animation=0;animation<132;++animation){contact.tm_anim=short(animation);p.engine.ball_at_contact(&contact);expect(std::isfinite(p.engine.ballx) && std::isfinite(p.engine.bally) && std::isfinite(p.engine.ballz),"Every recovered contact offset is finite");}
  std::cout<<"Passed "<<checks<<" player movement and interaction checks.\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
