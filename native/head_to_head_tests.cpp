#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "original_frontend.hpp"
#include "gamepad.hpp"
#include "action_constants.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
using namespace actua;
using namespace actua::original::action_constants;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 require(argc==2,"Expected archive directory");
 OriginalGraphics g(argv[1]);auto& s=*g.sequence;auto& e=s.engine;s.next_head_to_head=true;
 auto fixture=[&](int half=0){
  s.next_format=OriginalActions::MatchFormat::regulation;s.start_match(false);
  s.restart_mode=s.restart_ticks=0;e.match_mode=e.set_piece_on=e.dead_ball_cnt=e.game_action=e.ball_out_of_play=0;
  e.holder_lose_ball();e.reset_shot();e.match_half=half;
  for(int i=0;i<22;++i){auto& p=e.teams[i];p.guy_on=0;p.control=0;p.tm_x=i<11?400.f:880.f;p.tm_y=400;e.reset_ideas(&p);e.init_stand_act(&p);}
  for(int i:{1,2,12,13}){auto& p=e.teams[i];p.guy_on=1;p.tm_xdis=p.mface_x=i<11?1.f:-1.f;p.tm_ydis=p.mface_y=0;p.tm_y=i%11==1?400.f:500.f;p.tm_rate=p.tm_pow=p.tm_cont=p.tm_flair=p.tm_vis=p.tm_ac=96;p.tm_mcspd=1.5f;p.tm_jump=40;}
  for(int side=0;side<2;++side){auto scope=s.input_scope(side);s.selected=s.control_slot()+1;s.set_play_input(0,0);s.set_play_buttons(false,false);}
  e.ballx=640;e.bally=400;e.ballz=80;e.ballxdis=e.ballydis=e.ballzdis=0;e.ball_inair=1;
 };
 auto input=[&](int side,float x,float y,bool shot=false,bool pass=false){auto scope=s.input_scope(side);s.set_play_input(x,y);s.set_play_buttons(shot,pass);};
 for(int half:{0,1}){
  fixture(half);const int p1=s.selected_for(0),p2=s.selected_for(1);const float a=e.teams[p1].tm_x,b=e.teams[p2].tm_x;
  for(int tick=0;tick<6;++tick){input(0,half?-1.f:1.f,0);input(1,half?1.f:-1.f,0);s.step();}
  require((e.teams[p1].tm_x-a)*(half?-1:1)>0 && (e.teams[p2].tm_x-b)*(half?1:-1)>0,"Both controllers move their own player simultaneously at either end");
  require(e.teams[p1].control==1 && e.teams[p2].control==2,"Original control IDs remain distinct");
  {auto scope=s.input_scope(1);s.switch_play_player();require(s.selected!=p2 && s.is_controlled_team(s.selected),"P2 switches only within own team");}
  require(s.selected_for(0)==p1,"P2 selection does not alter P1 selection");
 }
 std::cout<<"Independent movement, original user IDs and switching at both ends passed.\n";
 for(int side:{0,1})for(int half:{0,1}){
  fixture(half);const int actor=s.selected_for(side);auto& p=e.teams[actor];p.tm_x=actor<11?950.f:330.f;
  e.ball_inair=0;e.ballz=2;e.collect_ball(&p);e.hold_ball(&p);
  for(int tick=0;tick<8;++tick){input(side,0,0,true);input(1-side,0,0,false,true);s.step();}
  {auto scope=s.input_scope(side);require(s.live_charge()==8,"Owner has an independent shot charge");}
  {auto scope=s.input_scope(1-side);require(s.live_charge()==0,"Other controller cannot borrow the ball owner's charge");}
  input(side,0,0);input(1-side,0,0);for(int tick=0;tick<90 && !p.tm_shots;++tick)s.step();
  require(p.tm_shots==1 && e.last_touch==actor+1,"Both teams shoot at original contact");
  require((actor<11?e.ballxdis:-e.ballxdis)>0,"Both teams shoot toward their attacking goal");
 }
 std::cout<<"Independent charges and original shot releases for both teams/halves passed.\n";
 for(int side:{0,1})for(int half:{0,1})for(bool loft:{false,true}){
  fixture(half);const int actor=s.selected_for(side),receiver=actor+1;const float sign=actor<11?1.f:-1.f;
  auto& p=e.teams[actor];auto& mate=e.teams[receiver];mate.tm_x=p.tm_x+sign*140;mate.tm_y=p.tm_y;
  e.ball_inair=0;e.ballz=2;e.collect_ball(&p);e.hold_ball(&p);
  {auto scope=s.input_scope(side);s.set_play_input(sign,0);if(loft)s.request_play_kick(OriginalActions::PlayKick::loft);else s.set_play_buttons(false,true);}
  s.step();input(side,0,0);int pass_count=0;bool caught=false;
  for(int tick=0;tick<160;++tick){if(!e.ball_poss)input(side,0,0,false,true);s.step();pass_count=std::max(pass_count,int(p.tm_pass));if(e.ball_poss==receiver+1){caught=true;break;}}
  if(!caught)std::cerr<<"ball="<<e.ballx<<","<<e.bally<<","<<e.ballz<<" target="<<mate.tm_x<<","<<mate.tm_y<<" selected="<<s.selected_for(side)<<" pass_type="<<e.pass_type<<" fpass="<<p.tm_fpass_to<<" receiver="<<e.receiver_a<<","<<e.receiver_b<<" restart="<<s.restart_mode<<"\n"<<"pass fixture side="<<side<<" half="<<half<<" loft="<<loft<<" owner="<<e.ball_poss<<" passes="<<pass_count<<'\n';
  require(pass_count==1 && caught && s.selected_for(side)==receiver,"Ground/loft pass reception hands control to correct team's receiver");
  input(side,sign,0);const float before=e.teams[receiver].tm_x;for(int tick=0;tick<8;++tick)s.step();
  require((e.teams[receiver].tm_x-before)*sign>0,"Each controller moves its receiver after contact");
 }
 for(int side:{0,1}){
  fixture();const int actor=s.selected_for(side),opponent=s.selected_for(1-side);auto& p=e.teams[actor];auto& enemy=e.teams[opponent];
  enemy.tm_x=p.tm_x+(actor<11?1.f:-1.f);enemy.tm_y=p.tm_y;e.ball_inair=0;e.ballz=2;e.collect_ball(&enemy);e.ballx=enemy.tm_x;e.bally=enemy.tm_y;
  input(side,0,0,false,true);s.step();require(p.tm_act==14 || e.ball_poss==actor+1,"Both teams can invoke original close steal");
 }
 std::cout<<"Eight ground/loft receptions with original control input and subsequent movement, and both teams' close steal passed.\n";
 fixture();const int sent0=s.selected_for(0),sent1=s.selected_for(1);e.teams[sent0].guy_on=e.teams[sent1].guy_on=0;
 for(int side:{0,1}){auto scope=s.input_scope(side);++e.selection_events;s.update_auto_selection();require(s.selected!=(side?sent1:sent0) && e.teams[s.selected].guy_on && s.is_controlled_team(s.selected),"Dismissed selected players are replaced within each team");}
 s.start_match(false);require(s.queue_substitution(false,1,12) && s.queue_substitution(true,1,12),"Both teams can queue substitutions");s.restart_ticks=1;e.match_mode=0;s.restart_mode=CENTRE_A;s.step();
 require(s.substitutions[0]==1 && s.substitutions[1]==1 && e.teams[1].shirt==12 && e.teams[12].shirt==12,"Both substitutions apply at a stoppage");
 std::cout<<"Dismissed-player reselection and both teams' substitutions passed.\n";
 for(int side:{0,1})for(int half:{0,1})for(int stage=0;stage<6;++stage){
  s.practice_restart(stage);e.match_half=half;
  const bool physicalB=(side!=half);
  const int a[]={PEN_KICK_A,DF_KICK_A,IF_KICK_A,CORNER_TR,THROW_IN_A,GOAL_KICK_TL};
  const int b[]={PEN_KICK_B,DF_KICK_B,IF_KICK_B,CORNER_TL,THROW_IN_B,GOAL_KICK_TR};
  e.match_mode=0;s.restart_mode=(physicalB?b:a)[stage];s.restart_ticks=1;e.last_touch=physicalB?2:13;e.fkick_taker=physicalB?13:2;e.penalty_taker=0;s.step();
  for(int tick=0;tick<150;++tick){
   {auto scope=s.input_scope(side);require(s.human_restart(),"Restart belongs to correct controller");s.set_restart_input(0,0,false,false,false);}
   {auto scope=s.input_scope(1-side);require(!s.human_restart(),"Opponent cannot own the restart");s.set_restart_input(1,1,true,true,true);}
   s.step();
  }
  require(e.match_mode && !e.f1_down && !e.setp_hgt,"Opponent buttons cannot charge or release restart");
  for(int tick=0;tick<8 && e.match_mode;++tick){{auto scope=s.input_scope(side);s.set_restart_input(0,stage==4?1.f:0.f,true,false,false);}s.step();}
  {auto scope=s.input_scope(side);s.set_restart_input(0,0,false,false,false);}
  for(int tick=0;tick<200 && e.match_mode;++tick)s.step();
  require(!e.match_mode && !s.restart_mode,"Both teams can release all original awarded restarts");
 }
 std::cout<<"24 awarded restart cases passed: six types, two teams, two halves; opponent input ignored.\n";
 for(int side:{0,1}){
  fixture();const int keeper=side?11:0;auto& p=e.teams[keeper];p.guy_on=1;p.tm_x=side?1210.f:70.f;p.tm_y=400;p.tm_xdis=p.mface_x=side?-1.f:1.f;e.init_stand_act(&p);
  e.ball_inair=0;e.ball_in_hands=1;e.collect_ball(&p);e.hold_ball(&p);
  for(int tick=0;tick<100;++tick)s.step();
  {auto scope=s.input_scope(side);require(s.human_keeper_hold() && s.selected==keeper,"Keeper waits for correct controller");s.set_play_input(side?-1.f:1.f,0);s.request_play_kick(OriginalActions::PlayKick::shot);}
  for(int tick=0;tick<150 && !s.distributions;++tick)s.step();require(s.distributions==1,"Both keepers distribute at original contact");
 }
 fixture();s.interval_ticks=1;s.interval_minute=45;e.match_half=1;s.step();
 require(s.is_blue(s.selected_for(0)) && !s.is_blue(s.selected_for(1)),"Half-time keeps both controllers with their own teams");
 require(s.selected_for(s.human_for(e.setp_taker-1))==e.setp_taker-1,"Half-time selects the new kickoff taker instead of retaining a distant previous player");
 s.restart_play();require(s.head_to_head && !s.cpu_vs_cpu,"Rematch retains local mode");s.set_cpu_control(true);require(!s.cpu_vs_cpu,"Spectator shortcut cannot remove opponent");
 s.next_format=OriginalActions::MatchFormat::penalties;s.start_match(false);int attempt=-1,age=0,manual[2]{};
 for(int tick=0;tick<16000 && s.running;++tick){
  if(e.penalties!=attempt){attempt=e.penalties;age=0;}
  if(s.penalty_aiming())++age;
  for(int side=0;side<2;++side){auto scope=s.input_scope(side);if(s.penalty_human_taker()){
    if(age==1)++manual[side];s.set_penalty_input(0,age>=3 && age<10);
   }else s.set_penalty_input((attempt&1)?-1.f:1.f,s.penalty_released);}
  s.step();
 }
 require(s.full_time && manual[0]>=3 && manual[1]>=3,"Shootout completes with both human takers and keeper inputs");
 std::cout<<"Both keepers, half-time, rematch protection and complete human head-to-head shootout passed.\n";
 // Pin input devices; another controller must never steal a player's assignment.
 require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMEPAD),"SDL input init");SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");
 auto attach=[](){SDL_VirtualJoystickDesc d{};SDL_INIT_INTERFACE(&d);d.type=SDL_JOYSTICK_TYPE_GAMEPAD;d.naxes=6;d.nbuttons=15;d.name="Head-to-head contract pad";return SDL_AttachVirtualJoystick(&d);};
 auto id0=attach(),id1=attach();auto* pad0=SDL_OpenJoystick(id0);auto* pad1=SDL_OpenJoystick(id1);
 {GamepadInput pads(true);auto pump=[&](){SDL_UpdateJoysticks();SDL_Event event;bool disconnected=false;while(SDL_PollEvent(&event))disconnected|=pads.event(event)==PadEvent::disconnected;return disconnected;};pump();pads.set_local_match(true);const auto p0=pads.player_id(0),p1=pads.player_id(1);
  require(pads.local_ready() && p0!=p1,"Two distinct assignments");
  SDL_SetJoystickVirtualAxis(pad0,0,32767);SDL_SetJoystickVirtualAxis(pad1,0,-32768);pump();
  require(pads.player_movement(pads.player_for(id0)).x>0 && pads.player_movement(pads.player_for(id1)).x<0,"Independent device axes");
  const auto extra=attach();auto* third=SDL_OpenJoystick(extra);pump();SDL_SetJoystickVirtualButton(third,SDL_GAMEPAD_BUTTON_START,true);pump();
  require(pads.player_id(0)==p0 && pads.player_id(1)==p1 && pads.player_for(extra)<0,"Unassigned controller cannot steal a slot");
  const int lost=pads.player_for(id1);SDL_CloseJoystick(pad1);SDL_DetachVirtualJoystick(id1);require(pump() && !pads.local_ready(),"Either assigned disconnect is reported");
  require(!pads.player_movement(lost).x,"Disconnected axes are neutral");
  SDL_SetJoystickVirtualButton(third,SDL_GAMEPAD_BUTTON_START,false);pump();SDL_SetJoystickVirtualButton(third,SDL_GAMEPAD_BUTTON_START,true);pump();
  require(pads.local_ready() && pads.player_for(extra)==lost && pads.player_for(id0)!=lost,"Replacement joins only vacant slot");
  SDL_CloseJoystick(pad0);SDL_DetachVirtualJoystick(id0);require(pump() && !pads.local_ready() && pads.player_for(extra)==lost,"P1 disconnect keeps P2 assignment");
  SDL_CloseJoystick(third);SDL_DetachVirtualJoystick(extra);pump();
 }
 OriginalFrontend front(argv[1],{});front.open(g);auto key=[&](SDL_Keycode k){SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=k;front.event(event,g);};
 key(SDLK_RETURN);front.connected_controllers=1;key(SDLK_H);require(front.stage()==OriginalFrontend::Stage::selection && !front.local_message.empty(),"Two-player launch refuses missing controller");
 front.connected_controllers=2;key(SDLK_H);require(front.stage()==OriginalFrontend::Stage::lineup && s.next_head_to_head,"Two-player lineup accepted");
 front.connected_controllers=1;key(SDLK_RETURN);require(front.stage()==OriginalFrontend::Stage::lineup,"Disconnect during lineup prevents kickoff");
 SDL_Quit();std::cout<<"Pinned controllers, third-device isolation, disconnect and replacement passed.\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
