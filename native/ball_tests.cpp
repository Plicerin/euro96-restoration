#include "ball.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
using actua::original::BallEngine;
static int checks=0;
static void expect(bool value,const char* why) { ++checks; if(!value) throw std::runtime_error(why); }
static void near(float a,float b,const char* why,float tolerance=0.0002f) { expect(std::abs(a-b)<=tolerance,why); }
int main(int argc,char** argv) {
 try {
  if(argc==2 && std::string(argv[1])=="--trace") {
   // Stable harness for optional comparison to the preserved x86 object code.
   for(int scenario=0;scenario<6;++scenario) {
    BallEngine b;
    b.ballx=320; b.bally=400; b.ballxdis=10; b.ballydis=3;
    b.ballzdis=scenario==0?0.0f:8.0f; b.ball_inair=scenario!=0;
    b.prediction=1; // Position/velocity comparison excludes renderer orientation and sound.
    if(scenario==2) {b.wind_on=1;b.wind_speed=45;b.wind_x=0;b.wind_y=-1;}
    if(scenario==3) {b.swerve=1;b.full_xyspin=0.055f;b.full_zspin=0.015f;}
    if(scenario>=4) {b.after_touch_on=1;b.users_dir[0].x=scenario==4?0.25f:0.0f;b.users_dir[0].y=1;b.shoot_x=1;b.full_xyspin=0.055f;b.full_zspin=0.015f;}
    std::cout.precision(9);
    for(int tick=1;tick<=80;++tick) {
     b.ball_trajectory();
     std::cout<<scenario<<','<<tick<<','<<b.ballx<<','<<b.bally<<','<<b.ballz<<','<<b.ballxdis<<','<<b.ballydis<<','<<b.ballzdis<<','<<b.ball_inair<<'\n';
    }
   }
   return 0;
  }
  BallEngine b;
  expect(b.pitch_len==1280 && b.pitch_wid==800,"Recovered pitch size");
  near(b.ball_diam,4,"Recovered ball diameter"); near(b.bounce_dis,2.2f,"Recovered bounce loss");
  near(b.gravity,0.6f,"Match startup gravity override");
  near(b.zone_wid,182,"Original integer zone width"); near(b.zone_hgt,266,"Original integer zone height");
  b.ballxdis=10; b.ballydis=-10; b.ball_friction();
  near(b.ballxdis,9.65f,"Ground friction"); near(b.ballydis,-9.65f,"Negative ground friction");
  b.ballxdis=0.259f; b.ballydis=-0.259f; b.ball_friction();
  expect(b.ballxdis==0 && b.ballydis==0,"Original stop threshold after friction");
  b=BallEngine{}; b.ball_inair=1; b.ballxdis=10; b.ballzdis=8; b.ball_trajectory();
  near(b.ballx,650,"Movement precedes friction"); near(b.ballz,10,"Movement precedes gravity");
  near(b.ballxdis,9.86f,"Air friction"); near(b.ballzdis,7.4f,"Gravity step");
  b=BallEngine{}; b.ball_inair=1; b.ballz=3; b.ballzdis=-6; b.ballxdis=10; b.ball_trajectory();
  near(b.ballz,2,"Bounce floor"); near(b.ballzdis,3.2f,"Bounce loss then gravity");
  near(b.ballxdis,8.874f,"Landing friction then air friction");
  expect(b.bounce_events==1 && b.last_sound==13,"Soft bounce emitted through native interface");
  b=BallEngine{}; b.ball_inair=1; b.ballzdis=-12; b.move_ball();
  expect(b.last_sound==14,"Hard bounce event");
  b=BallEngine{}; b.ball_inair=1; b.ballzdis=-1; b.move_ball();
  expect(!b.ball_inair && b.ballzdis==0,"Weak bounce settles");
  b=BallEngine{}; b.ball_inair=1; b.ballzdis=-6; b.prediction=1; b.move_ball();
  expect(b.bounce_events==0,"Prediction suppresses audio");
  near(b.ns_ballz,2,"Prediction snapshot precedes vertical movement");
  b=BallEngine{}; b.ball_poss=1; b.ballxdis=8; b.ball_trajectory();
  near(b.ballx,640,"Possession stops free-flight integration");
  b=BallEngine{}; b.ballxdis=10; b.wind_on=1; b.wind_speed=100000; b.wind_x=1; b.ball_friction();
  near(b.ballxdis,10.615f,"Wind clamp recovered from original object");
  b=BallEngine{}; b.swerve=-1; b.full_xyspin=0.1f; b.full_zspin=0.2f; b.inc_swerve();
  near(b.ball_xyspin,-0.08f,"Lateral spin decay"); near(b.ball_zspin,-0.16f,"Vertical spin decay");
  b.stop_ball_spin(); expect(b.swerve==0 && b.ball_xyspin==0 && b.full_zspin==0,"Spin reset");
  b.ballx=0; b.bally=0; b.get_ball_zone(); expect(b.ball_zone1==0 && b.ball_zone2==31,"Opposing team zones mirror");
  b.ballx=1280; b.bally=800; b.get_ball_zone(); expect(b.ball_zone1==31 && b.ball_zone2==0,"Far pitch corner zones");
  b.ballx=90; b.bally=0; b.get_ball_zone(); expect(b.ball_zone1==0,"Before rounded zone boundary");
  b.ballx=91; b.get_ball_zone(); expect(b.ball_zone1==1,"At rounded zone boundary");
  b.ball_in_hands=1; b.ball_poss=1; b.get_ball_zone(); expect(b.ball_zone1==11 && b.ball_zone2==20,"Keeper A held-ball zone");
  b.ball_poss=12; b.get_ball_zone(); expect(b.ball_zone1==19 && b.ball_zone2==12,"Keeper B held-ball zone");
  b.ball_out_of_play=1; b.ball_in_hands=0; b.get_ball_zone(); expect(b.ball_zone1==19,"Out of play retains zones");
  b=BallEngine{}; b.get_xydis_from_ang(90,10); near(b.ballxdis,0,"Angle launch x");near(b.ballydis,-10,"Angle launch y convention");
  b.get_zdis_from_ang(30,10);near(b.ballzdis,5,"Elevation launch");
  actua::FixedStep clock;int total=0;
  for(int i=0;i<144;++i) total+=clock.advance(1.0/144,true);
  expect(total==20,"Simulation rate independent of render rate");
  expect(clock.advance(30,true)==5,"Bounded stall catch-up");
  expect(clock.advance(1,false)==0 && clock.advance(0.01,true)==0,"Pause discards accumulated time");
  for(int p=0;p<3;++p) for(int curve=-1;curve<=1;++curve) for(int power=3;power<=24;power+=3) {
   actua::BallSandbox lab; lab.preset=p;lab.curve=curve;lab.power=float(power);lab.aim=90;lab.launch();
   for(int i=0;i<500;++i) {
    lab.step(); expect(std::isfinite(lab.engine.ballz) && lab.engine.ballz>=2,"Launch remains finite above ground");
    expect(lab.engine.ball_zone1>=0 && lab.engine.ball_zone1<32,"Sandbox never exports an invalid tactics index");
   }
   lab.reset();near(lab.engine.ballx,640,"Reset returns ball to centre");expect(lab.trail_size==1 && lab.ticks==0,"Reset clears history");
  }
  actua::BallSandbox a,c;a.launch();for(int i=0;i<10;++i)a.step();near(c.engine.ballx,640,"Independent engine instances");
  a.paused=true;auto x=a.engine.ballx;a.step();near(a.engine.ballx,x,"Paused sandbox does not advance");
  std::cout<<"Passed "<<checks<<" original physics and native integration checks.\n";
  return 0;
 } catch(const std::exception& e) { std::cerr<<e.what()<<'\n';return 1; }
}
