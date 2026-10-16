#include "original_menu_overlays.hpp"
#include "original_menu_art.hpp"
#include "original_actions.hpp"
#include <algorithm>
namespace actua {
using Ink=OriginalMenuArt::Ink;
void draw_menu_power(SDL_Renderer* r,const OriginalGraphics& g,float charge){
 if(!g.menu_ui)return;auto& art=*g.menu_ui;art.clear();art.panel(196,428,248,36);
 art.text(320,434,"POWER - RELEASE TO KICK",Ink::white,0,2,225);art.present(r);
 SDL_SetRenderDrawColor(r,20,24,94,255);SDL_FRect meter{324,620,312,8};SDL_RenderFillRect(r,&meter);
 SDL_SetRenderDrawColor(r,0,190,235,255);meter.w*=std::clamp(charge,0.0f,1.0f);SDL_RenderFillRect(r,&meter);
}
void draw_menu_hint(SDL_Renderer* r,const OriginalGraphics& g,const std::string& hint){
 if(!g.menu_ui)return;auto& art=*g.menu_ui;art.clear();art.text(320,0,hint,Ink::white,0,2,630);
 // The prompt belongs beneath the pitch, outside the score display.
 art.present(r,{48,648,864,360});
}
void draw_menu_match_panels(SDL_Renderer* r,const OriginalGraphics& g,bool pad){
 if(!g.menu_ui)return;auto& art=*g.menu_ui;const auto& s=*g.sequence;const auto& e=s.engine;
 // Reuse the original title bar, panel and font without covering the existing
 // pitch viewport or changing its mouse aiming coordinates.
 art.begin(3);art.clear();art.panel(10,8,623,65);art.bar(22,21,598);art.text(320,24,"UEFA EURO 96",Ink::white,2,2,565);
 if(!s.human_restart() && !(s.penalty_human_taker() && e.match_mode)){
  art.panel(450,83,183,352);
  art.text(541,96,g.replaying?"REPLAY":s.cpu_vs_cpu?"CPU V CPU":s.human_keeper_hold()?"KEEPER BALL":"MATCH",Ink::cyan,0,2,163);
  art.text(464,128,s.team_label(false),Ink::white,0,0,121);art.text(619,128,std::to_string(s.shootout?s.penalty_scores[0]:s.blue_goals),Ink::white,0,1,28);
  art.text(464,153,s.team_label(true),Ink::white,0,0,121);art.text(619,153,std::to_string(s.shootout?s.penalty_scores[1]:s.red_goals),Ink::white,0,1,28);
  art.text(541,186,s.clock_label(),Ink::cyan,0,2,163);
  if(s.cpu_vs_cpu){
   int passes[2]{s.sub_passes[0],s.sub_passes[1]},shots[2]{s.sub_shots[0],s.sub_shots[1]};
   for(int i:s.visible_players){passes[!s.is_blue(i)]+=e.teams[i].tm_pass;shots[!s.is_blue(i)]+=e.teams[i].tm_shots;}
   art.text(541,219,"PASSES "+std::to_string(passes[0])+" : "+std::to_string(passes[1]),Ink::white,0,2,163);
   art.text(541,245,"SHOTS "+std::to_string(shots[0])+" : "+std::to_string(shots[1]),Ink::white,0,2,163);
   art.text(541,271,"SAVES "+std::to_string(e.teams[s.blue_slot()].tm_kpsaves)+" : "+std::to_string(e.teams[11-s.blue_slot()].tm_kpsaves),Ink::white,0,2,163);
  }else{
   art.text(541,219,"YOUR PLAYER",Ink::cyan,0,2,163);
   art.text(541,241,s.player_label(s.selected),Ink::white,0,2,163);
   art.text(541,277,g.camera_name(),Ink::white,0,2,163);
  }
  art.text(541,303,std::string("RADAR ")+(g.show_radar?"ON":"OFF"),Ink::cyan,0,2,163);
  art.text(541,324,pad?"F7   RADAR":"TAB / F7   RADAR",Ink::white,0,2,163);
  art.text(541,355,pad?"START   PAUSE":"P   PAUSE",Ink::white,0,2,163);
  art.text(541,378,pad?"R3   CLOSE PANELS":"F6   CLOSE PANELS",Ink::white,0,2,163);
  art.text(541,410,s.cpu_vs_cpu?(pad?"RB   TAKE CONTROL":"B   TAKE CONTROL"):(pad?"LB / RB   SWITCH":"E   SWITCH PLAYER"),Ink::white,0,2,163);
  art.panel(10,443,623,34);
  art.text(320,454,s.human_keeper_hold()?(pad?"STICK AIM    A PASS    Y LOFT    X PUNT":"ARROWS AIM    SPACE PASS    C LOFT    X PUNT"):pad?"STICK MOVE    A PASS    X SHOOT / TACKLE":"ARROWS MOVE    SPACE PASS    X SHOOT / TACKLE",Ink::white,0,2,597);
 }
 art.present(r,{0,0,960,664});
}
void draw_menu_setup(SDL_Renderer* r,const OriginalGraphics& g,int row){
 if(!g.menu_ui)return;auto& art=*g.menu_ui;const auto& s=*g.sequence;
 art.begin(3);art.panel(22,96,596,377);art.bar(38,104,564);art.text(320,110,"MATCH SETUP",Ink::white,1,2,540);
 const std::string labels[]={"HOME TEAM","AWAY TEAM","HOME TACTICS","AWAY TACTICS","MATCH LENGTH","CAMERA","FORMAT"};
 const std::string values[]={original_teams[s.next_home].name,original_teams[s.next_away].name,formation_names[s.next_home_formation],formation_names[s.next_away_formation],std::to_string(s.match_minutes)+" MINUTES",g.camera_name(),OriginalActions::format_name(s.next_format)};
 for(int i=0;i<7;++i){const int y=153+i*33;art.text(43,y,(i==row?"> ":"  ")+labels[i],i==row?Ink::cyan:Ink::white,0,0,235);art.text(297,y,values[i],i==row?Ink::cyan:Ink::white,0,0,292);}
 art.text(320,395,"ARROWS / D-PAD   SELECT AND CHANGE",Ink::white,0,2,550);
 art.text(320,420,"ENTER / A   WATCH      Y / NORTH   PLAY",Ink::cyan,0,2,550);
 art.text(320,449,"F2 / L3 / BACK   RETURN",Ink::white,0,2,550);art.present(r);
}
void draw_menu_pause(SDL_Renderer* r,const OriginalGraphics& g,bool pad){
 const auto& s=*g.sequence;
 if(!g.menu_ui || !g.paused || g.replaying || s.full_time || s.interval_ticks)return;
 auto& art=*g.menu_ui;art.clear();art.panel(25,125,590,350);
 art.text(320,139,s.cpu_vs_cpu?"PAUSED - COMPUTER V COMPUTER":s.team_play?"PAUSED - AUTO SELECT ON":"PAUSED",Ink::cyan,1,2,552);
 const std::string actions[]={pad?"START   RESUME":"P   RESUME",pad?"Y / NORTH   SUBSTITUTIONS":"F3   SUBSTITUTIONS",pad?"X / WEST   RECENT REPLAY":"F4   RECENT REPLAY",pad?"L3   MATCH SETUP":"F2   MATCH SETUP",pad?"R3   RESTART MATCH":"R   RESTART MATCH",pad?"B / EAST   SAVED REPLAYS":"F10   SAVED REPLAYS"};
 for(int i=0;i<6;++i)art.text(43,190+i*39,actions[i],Ink::white,0,0,254);
 art.text(459,183,"ORIGINAL BALL CONTROLS",Ink::cyan,0,2,280);
 const std::string shot=pad?"X":"X",pass=pad?"A":"SPACE";
 const std::string controls[]={"MOVE + "+shot+": SHOOT / AFTERTOUCH","STILL: HOLD THEN RELEASE "+shot+" / "+pass,"BOTH; RELEASE "+shot+": CHIP","BOTH; RELEASE "+pass+": HIGH PASS","LOOSE BALL: "+shot+" FOR FIRST TIME",pass+": TRAP / CHEST / HEAD","BOTH: FIRST-TIME PASS"};
 for(int i=0;i<7;++i)art.text(319,214+i*28,controls[i],Ink::white,0,0,278);
 art.text(320,452,s.head_to_head?"TWO CONTROLLERS - SHARED MATCH":pad?"F5   SET-PIECE PRACTICE (NEW MATCH)":"F5   SET-PIECE PRACTICE (NEW MATCH)",Ink::cyan,0,2,552);
 art.present(r);
}
void draw_menu_replay(SDL_Renderer* r,const OriginalGraphics& g,bool pad){
 if(!g.menu_ui || !g.replaying)return;
 auto& art=*g.menu_ui;art.clear();art.panel(97,335,508,139);
 art.text(111,341,g.replay_message.empty()?(g.saved_replay_active()?"SAVED REPLAY":pad?"B / EAST: SAVE REPLAY":"S: SAVE REPLAY"):g.replay_message,Ink::cyan,0,0,480);
 art.text(111,361,std::string(g.paused?"REPLAY PAUSED":"REPLAY")+"   "+std::to_string(g.replay_speed()*20)+"%   "+(g.replay_looping()?"LOOP ON":"LOOP OFF"),Ink::white,0,0,480);
 art.text(111,381,"FRAME "+std::to_string(g.replay_position()+1)+" / "+std::to_string(g.replay_frames())+"     "+g.camera_name(),Ink::white,0,0,480);
 art.text(111,422,pad?"START PAUSE    D-PAD: FRAME / SPEED":"P PAUSE    ARROWS: FRAME / SPEED",Ink::white,0,0,480);
 art.text(111,439,pad?"LB/RB SEEK    Y START    X LOOP":"PGUP/PGDN SEEK    HOME/END EDGES    L LOOP",Ink::white,0,0,480);
 art.text(111,456,pad?"R3 CAMERA    A RETURN    CLICK BAR TO SEEK":"V CAMERA    SPACE RETURN    CLICK BAR TO SEEK",Ink::cyan,0,0,480);
 art.present(r);
 // Keep the existing seek hit target exactly aligned with the visible track.
 const float progress=g.replay_frames()>1?float(g.replay_position())/float(g.replay_frames()-1):0;
 SDL_SetRenderDrawColor(r,20,24,94,255);SDL_FRect track{200,552,640,10};SDL_RenderFillRect(r,&track);
 SDL_SetRenderDrawColor(r,0,190,235,255);track.w*=progress;SDL_RenderFillRect(r,&track);
 SDL_FRect thumb{198+640*progress,549,4,16};SDL_SetRenderDrawColor(r,230,238,245,255);SDL_RenderFillRect(r,&thumb);
}
void draw_menu_squad(SDL_Renderer* r,const OriginalGraphics& g,bool away,int row,int slot,int shirt,const std::string& message){
 if(!g.menu_ui)return;
 auto& art=*g.menu_ui;const auto& s=*g.sequence;const int team=away?s.away_team:s.home_team,start=away?11-s.blue_slot():s.blue_slot();
 art.begin(1,team);art.panel(15,99,451,350);art.panel(476,99,159,350);
 art.image(menu_art::SMLL_GOTH+team,550,2,0,0,team);
 art.text(240,108,(row==0?"> ":"")+s.team_label(away)+"  CHANGES "+std::to_string(s.substitutions[away])+" / 3",Ink::cyan,1,2,423);
 art.text(32,151,"ON PITCH",Ink::white);art.text(327,151,"PACE",Ink::white);art.text(395,151,"CARD",Ink::white);
 for(int j=0;j<11;++j){const auto& p=s.engine.teams[start+j];const auto ink=j==slot?Ink::cyan:Ink::white;const int y=178+j*23;
  art.text(31,y,(j==slot && row==1?"> ":"  ")+s.player_label(start+j),ink,0,0,281);
  art.text(370,y,std::to_string(int(s.pace_remaining(start+j)*100))+"%",ink,0,1,57);
  art.text(393,y,!p.guy_on?"OFF":p.tm_book>=5?"RED":p.tm_book?"YELLOW":"-",ink,0,0,60);
 }
 art.text(555,110,"MATCH BENCH",Ink::cyan,0,2,147);
 for(int n=12;n<=15;++n){const int actual=s.bench_shirt(away,n-12);const auto& p=original_teams[team].players[actual-1];const std::string name=p.name;const auto ink=n==shirt?Ink::cyan:Ink::white;const int y=156+(n-12)*68;
  art.text(482,y,(n==shirt && row==2?"> ":"")+std::to_string(actual)+" "+name.substr(name.find(' ')+1),ink,0,0,148);
  art.text(487,y+20,p.position,Ink::white,0,0,140);art.text(487,y+38,s.bench_available(away,actual)?"AVAILABLE":"UNAVAILABLE",Ink::white,0,0,140);
 }
 art.panel(6,451,628,29);art.text(320,458,message.empty()?(s.sub_notice.empty()?"ARROWS CHOOSE    ENTER / A QUEUE    F3 / BACK CLOSE":s.sub_notice):message,Ink::white,0,2,603);
 art.present(r);
}
}
