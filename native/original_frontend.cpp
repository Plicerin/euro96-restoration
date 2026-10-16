#include "original_frontend.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace actua {
namespace {
constexpr SDL_FRect view{48,8,864,648};
void check(bool ok){if(!ok)throw std::runtime_error(SDL_GetError());}

}
std::filesystem::path OriginalFrontend::default_save_path(){
#ifdef __EMSCRIPTEN__
 return "/saves/tournament.sav"; // Mounted/restored by the browser shell before main().
#else
 char* pref=SDL_GetPrefPath("Actua96 Restoration","Actua96");
 if(!pref)throw std::runtime_error(std::string("Cannot locate application save folder: ")+SDL_GetError());
 const std::filesystem::path folder(std::u8string(reinterpret_cast<const char8_t*>(pref)));SDL_free(pref);
 return folder/"tournament.sav";
#endif
}
OriginalFrontend::OriginalFrontend(const std::filesystem::path& data,const std::filesystem::path& cd,const std::filesystem::path& save_path,bool preferences_enabled,bool legacy_film_diagnostics)
 :art_(std::make_shared<OriginalMenuArt>(Archive(data/"EURO.DAT",data/"EURO.OFF"))),preferences_enabled_(preferences_enabled),save_path_(save_path.empty()?default_save_path():save_path),legacy_save_path_(save_path.empty()?data.parent_path()/"user-data"/"tournament.sav":std::filesystem::path{}){
 preferences_path_=save_path_;preferences_path_.replace_extension(".prefs");
 try{preview_=std::make_unique<OriginalPreview>(cd);}catch(const std::exception& e){SDL_Log("Euro96 previews: %s",e.what());}
 movie_available_=legacy_film_diagnostics && std::filesystem::exists(cd/"VIDEO/MAKING.AMV");
 intro_available_=legacy_film_diagnostics && std::filesystem::exists(cd/"VIDEO/INTRO.AMV");
 trophy_available_=std::filesystem::exists(cd/"VIDEO/EURO_CUP.BIN");
 french_available_=std::filesystem::exists(cd/"COMMENTARY/FRENCH.BIN");
 if(preferences_enabled_)try{if(std::filesystem::exists(preferences_path_))preferences_.load(preferences_path_);}
 catch(const std::exception& error){
  SDL_Log("Preferences: %s",error.what());preferences_status_="PREFERENCES INVALID - DEFAULTS LOADED";
  try{auto backup=preferences_path_;backup+=".bak";preferences_.load(backup);preferences_status_="BACKUP LOADED - S TO SAVE";}catch(const std::exception&){}
 }
 Archive euro(data/"EURO.DAT",data/"EURO.OFF");
 for(int i=0;i<10;++i)tactics_[i]=decode_tactics(euro.entry(111+i));
}

OriginalFrontend::~OriginalFrontend(){SDL_DestroyTexture(menu_frame_);SDL_DestroyTexture(previous_menu_frame_);}
void OriginalFrontend::menu_preview(int slot,int x,int y,int w,int h){
 if(!preview_)return;
 try{if(preview_->select(slot))art_->rgba(preview_->pixels(),preview_->width(),preview_->height(),x,y,w,h);
  else if(slot<320)art_->text(x+w/2,y+h/2,"NO PREVIEW",OriginalMenuArt::blue,0,2,w);}
 catch(const std::exception& e){SDL_Log("Euro96 preview: %s",e.what());preview_.reset();}
}
std::vector<std::pair<const char*,SDL_Keycode>> OriginalFrontend::title_entries()const{
 std::vector<std::pair<const char*,SDL_Keycode>> entries{{"EXHIBITION",SDLK_RETURN},{"CHAMPIONSHIP",SDLK_T},{"RESUME",SDLK_L},{"SAVED REPLAYS",SDLK_F10},{"OPTIONS",SDLK_O}};
 if(intro_available_)entries.emplace_back("INTRODUCTION",SDLK_I);
 if(movie_available_)entries.emplace_back("MAKING OF",SDLK_M);
 entries.emplace_back("QUIT",SDLK_ESCAPE);return entries;
}
void OriginalFrontend::open(OriginalGraphics& g){g.menu_ui=art_;title_choice_=0;g.prematch.finish();g.walkoff.finish();g.sequence->next_options=preferences_.match;stage_=Stage::title;g.paused=true;g.sequence->clear_play_input();}
void OriginalFrontend::save_replay(OriginalGraphics& g){
 try{g.save_replay(save_path_.parent_path()/"replays");}catch(const std::exception& error){SDL_Log("Replay save: %s",error.what());g.replay_message="SAVE FAILED - CHECK REPLAY FOLDER";}
}
void OriginalFrontend::refresh_replays(){
 replay_status_.clear();
 try{replay_entries_=replay_library(save_path_.parent_path()/"replays");}
 catch(const std::exception& error){SDL_Log("Replay library: %s",error.what());replay_entries_.clear();replay_status_="CANNOT READ REPLAY FOLDER";}
 replay_row_=std::clamp(replay_row_,0,std::max(0,int(replay_entries_.size())-1));
}
void OriginalFrontend::open_replays(OriginalGraphics& g){
 if(g.replaying || replay_watching_ || stage_==Stage::replays)return;
 replay_return_=stage_;replay_return_paused_=g.paused;stage_=Stage::replays;g.paused=true;g.sequence->clear_play_input();refresh_replays();
}
void OriginalFrontend::play_saved_replay(OriginalGraphics& g){
 if(replay_entries_.empty())return;
 try{g.open_saved_replay(replay_entries_.at(replay_row_).path);replay_watching_=true;stage_=Stage::hidden;replay_status_.clear();}
 catch(const std::exception& error){SDL_Log("Replay load: %s",error.what());replay_status_="CANNOT PLAY - INVALID FILE OR MISSING MEDIA";}
}
void OriginalFrontend::setup(OriginalGraphics& g){
 g.menu_ui=art_;
 if(tournament_mode_){if(stage_==Stage::hidden){was_paused_=g.paused;g.paused=true;stage_=Stage::tournament;g.sequence->clear_play_input();}return;}
 return_match_=g.sequence->team_play && !g.sequence->full_time;was_paused_=g.paused;
 stage_=Stage::selection;g.paused=true;g.sequence->clear_play_input();
}
void OriginalFrontend::change(OriginalGraphics& g,int delta){
 auto& a=*g.sequence;
 if(row_<2)a.change_team(row_==1,delta);
 else if(row_<4){int& f=row_==2?a.next_home_formation:a.next_away_formation;f=(f+delta+10)%10;}
 else if(row_==4){const int lengths[]={5,10,20,40,90};for(int i=0;i<5;++i)if(a.match_minutes==lengths[i]){a.set_match_minutes(lengths[(i+delta+5)%5]);break;}}
 else if(row_==5)g.change_camera(delta);
 else if(row_==7)g.change_stadium(delta);
 else a.next_format=static_cast<OriginalActions::MatchFormat>((int(a.next_format)+delta+4)%4);
}
void OriginalFrontend::ready(OriginalGraphics& g,bool cpu,bool local){if(local && connected_controllers<2){local_message="CONNECT TWO CONTROLLERS";return;}local_message.clear();g.sequence->next_head_to_head=local;cpu_=cpu;side_=0;squad_cursor_=0;squad_pick_=-1;squad_message_.clear();if(!tournament_mode_){g.sequence->next_squad_enabled=false;g.sequence->next_tournament_fixture=-1;g.sequence->next_original_draw=false;}stage_=Stage::lineup;g.paused=true;g.sequence->clear_play_input();}
void OriginalFrontend::confirm(OriginalGraphics& g){
 if(stage_==Stage::title){tournament_mode_=false;g.sequence->next_takers={};return_match_=false;stage_=Stage::selection;return;}
 if(stage_==Stage::tournament_setup){
  tournament_.reset(tournament_team_,g.sequence->match_minutes,g.next_stadium,g.match_camera%3);
  tournament_.scheduled_venues=std::all_of(OriginalTournament::fixture_venues.begin(),OriginalTournament::fixture_venues.end(),[&](int venue){return g.stadium_available(venue);});
  tournament_mode_=true;tournament_match_=false;tournament_auto_=false;tournament_error_.clear();tournament_page_=tournament_team_<0?0:tournament_.draw[tournament_team_]/4;tournament_save();stage_=Stage::tournament;return;
 }
 if(stage_==Stage::tournament){if(tournament_.complete())request_trophy(g);else tournament_next(g);return;}
 if(stage_==Stage::selection){ready(g,true);return;}
 if(stage_==Stage::lineup){
  if(g.sequence->next_head_to_head && connected_controllers<2){local_message="CONNECT TWO CONTROLLERS TO START";return;}
  if(tournament_mode_){tournament_save();if(!tournament_error_.empty())return;
   try{tournament_.prepare(*g.sequence);}catch(const std::exception& e){squad_message_=e.what();return;}}
  g.action_view=true;g.overview=false;g.apply_stadium();g.sequence->start_match(cpu_);if(tournament_mode_)tournament_match_=true;g.paused=false;stage_=Stage::hidden;seen_interval_=0;seen_result_=false;g.start_prematch();return;
 }
 if(stage_==Stage::interval){g.sequence->interval_ticks=1;g.sequence->clear_play_input();g.paused=false;stage_=Stage::hidden;return;}
 if(stage_==Stage::result){return_match_=false;stage_=tournament_mode_?Stage::tournament:Stage::selection;tournament_age_=0;if(trophy_pending_)request_trophy(g);}
}
void OriginalFrontend::back(OriginalGraphics& g){
 if(stage_==Stage::replays){stage_=replay_return_;g.paused=replay_return_paused_;g.opening_input_guard=true;return;}
 if(stage_==Stage::options){stage_=options_return_;interval_age_=0;tournament_age_=0;return;}
 if(stage_==Stage::tactics){stage_=tactics_return_;interval_age_=0;return;}
 if(stage_==Stage::simulating){tournament_match_=false;tournament_auto_=false;stage_=Stage::tournament;return;}
 if(stage_==Stage::tournament_setup){stage_=Stage::title;return;}
 if(stage_==Stage::tournament){if(tournament_match_){stage_=Stage::hidden;g.paused=was_paused_;}else{stage_=Stage::title;tournament_mode_=false;tournament_auto_=false;}return;}
 if(tournament_mode_ && stage_==Stage::lineup){stage_=Stage::tournament;return;}
 if(stage_==Stage::title){quit=true;return;}
 if(stage_==Stage::lineup){stage_=Stage::selection;return;}
 if(stage_==Stage::selection){stage_=return_match_?(g.sequence->interval_ticks?Stage::interval:Stage::hidden):Stage::title;if(return_match_)g.paused=was_paused_;g.sequence->clear_play_input();}
}
void OriginalFrontend::squad_select(OriginalGraphics& g){
 const int team=side_?g.sequence->next_away:g.sequence->next_home;
 if(tournament_.user>=0 && tournament_.user!=team){squad_message_="OPPOSITION SQUAD - VIEW ONLY";return;}
 if(squad_pick_<0){squad_pick_=squad_cursor_;squad_message_="CHOOSE SECOND PLAYER - SPACE TO SWAP";return;}
 if(!tournament_.swap_squad(team,squad_pick_,squad_cursor_))squad_message_="UNAVAILABLE PLAYER OR GOALKEEPER REQUIRED";
 else{tournament_save();squad_message_=tournament_error_.empty()?"SQUAD SAVED":tournament_error_;}
 squad_pick_=-1;
}
void OriginalFrontend::key(SDL_Keycode key,OriginalGraphics& g){
 if(stage_==Stage::replays){
  if(key==SDLK_ESCAPE || key==SDLK_F10)back(g);
  else if(key==SDLK_RETURN || key==SDLK_SPACE)play_saved_replay(g);
  else if(key==SDLK_R)refresh_replays();
  else {const int delta=key==SDLK_UP?-1:key==SDLK_DOWN?1:key==SDLK_PAGEUP?-6:key==SDLK_PAGEDOWN?6:0;replay_row_=std::clamp(replay_row_+delta,0,std::max(0,int(replay_entries_.size())-1));}
  return;
 }
 if(key==SDLK_F10 && (stage_==Stage::title || stage_==Stage::result || (stage_==Stage::hidden && g.paused))){open_replays(g);return;}
 if(stage_==Stage::options){options_key(key,g);return;}
 if(key==SDLK_O && visible() && stage_!=Stage::simulating){open_options(g);return;}
 if(stage_==Stage::tactics){tactics_key(key,g);return;}
 if((stage_==Stage::lineup || stage_==Stage::interval) && key==SDLK_T){open_tactics(g);return;}
 if(stage_==Stage::title){
  const auto entries=title_entries();const int count=int(entries.size());
  if(key==SDLK_UP || key==SDLK_DOWN){title_choice_=(title_choice_+(key==SDLK_UP?count-1:1))%count;return;}
  if(key==SDLK_RETURN && title_choice_){this->key(entries[title_choice_].second,g);return;}
  if((key==SDLK_I && !intro_available_) || (key==SDLK_M && !movie_available_))return;
  if(key==SDLK_I && intro_available_){movie_error.clear();intro_requested=true;return;}
  if(key==SDLK_M && movie_available_){movie_error.clear();movie_requested=true;return;}
  if(key==SDLK_T){tournament_team_=-1;tournament_error_.clear();stage_=Stage::tournament_setup;return;}
  if(key==SDLK_L){tournament_load(g);return;}
 }
 if(stage_==Stage::tournament_setup){
  if(key==SDLK_UP || key==SDLK_DOWN){const int lengths[]={5,10,20,40,90};for(int i=0;i<5;++i)if(g.sequence->match_minutes==lengths[i]){g.sequence->set_match_minutes(lengths[(i+(key==SDLK_UP?1:4))%5]);break;}}
  if(key==SDLK_LEFT)tournament_team_=(tournament_team_+17)%17-1;
  if(key==SDLK_RIGHT)tournament_team_=(tournament_team_+2)%17-1;
 }
 if(stage_==Stage::tournament){
  if(key==SDLK_S){tournament_save();return;}
  if(key==SDLK_LEFT)tournament_page_=(tournament_page_+6)%7;
  if(key==SDLK_RIGHT)tournament_page_=(tournament_page_+1)%7;
  if(key==SDLK_Y){tournament_next(g,true);return;}
  if(key==SDLK_A){tournament_auto_=!tournament_auto_;tournament_age_=0;return;}
  if(key==SDLK_T){tournament_match_=false;tournament_auto_=false;tournament_mode_=false;stage_=Stage::title;return;}
 }
 if(tournament_mode_ && stage_==Stage::lineup){
  if(key==SDLK_UP){squad_cursor_=(squad_cursor_+19)%20;return;}
  if(key==SDLK_DOWN){squad_cursor_=(squad_cursor_+1)%20;return;}
  if(key==SDLK_SPACE || key==SDLK_Y){squad_select(g);return;}
  if(key==SDLK_LEFT || key==SDLK_RIGHT || key==SDLK_TAB){side_=1-side_;squad_pick_=-1;squad_message_.clear();return;}
 }
 if(tournament_mode_ && key==SDLK_R)return;
 if(stage_==Stage::simulating && key!=SDLK_ESCAPE)return;
 if(key==SDLK_ESCAPE){back(g);return;}
 if(key==SDLK_F2){if(stage_==Stage::selection)back(g);else if(stage_!=Stage::lineup)setup(g);return;}
 if(key==SDLK_RETURN || key==SDLK_SPACE){confirm(g);return;}
 if(stage_==Stage::selection){
  if(key==SDLK_UP)row_=(row_+7)%8;if(key==SDLK_DOWN)row_=(row_+1)%8;
  if(key==SDLK_LEFT)change(g,-1);if(key==SDLK_RIGHT)change(g,1);
  if(key==SDLK_Y)ready(g,false);
  if(key==SDLK_H)ready(g,false,true);
 }else if(stage_==Stage::lineup){if(key==SDLK_LEFT || key==SDLK_RIGHT || key==SDLK_TAB)side_=1-side_;}
 else if(stage_==Stage::interval && key==SDLK_P)confirm(g);
 else if(!tournament_mode_ && stage_==Stage::result && key==SDLK_R)ready(g,g.sequence->cpu_vs_cpu,g.sequence->head_to_head);
}
bool OriginalFrontend::event(const SDL_Event& e,OriginalGraphics& g){
 if(e.type==SDL_EVENT_RENDER_TARGETS_RESET){menu_frame_ready_=menu_reveal_requested_=false;menu_fade_.reset();}
 // While the old page is visible, the first navigation press reveals the
 // destination only. It must not activate a control the user cannot see yet.
 const bool global=e.type==SDL_EVENT_KEY_DOWN && (e.key.key==SDLK_F8 || e.key.key==SDLK_F9 || e.key.key==SDLK_F11);
 const bool navigation=(e.type==SDL_EVENT_KEY_DOWN && !e.key.repeat) || e.type==SDL_EVENT_GAMEPAD_BUTTON_DOWN || e.type==SDL_EVENT_MOUSE_BUTTON_DOWN;
 if(navigation && !global){const bool reveal_only=menu_fading_out();menu_fade_.finish();if(reveal_only){menu_reveal_requested_=true;return true;}}
 if(g.prematch.active() || g.walkoff.active()){
  auto& presentation_paused=g.walkoff.active()?g.walkoff.paused:g.prematch.paused;
  const auto skip=[&]{if(g.walkoff.active())g.finish_walkoff();else g.finish_prematch();};
  if(e.type==SDL_EVENT_WINDOW_FOCUS_LOST || e.type==SDL_EVENT_GAMEPAD_REMOVED)presentation_paused=true;
  if(e.type==SDL_EVENT_KEY_DOWN){
   if(e.key.key==SDLK_F8 || e.key.key==SDLK_F9 || e.key.key==SDLK_F11)return false;
   if(!e.key.repeat){if(e.key.key==SDLK_P){presentation_paused=!presentation_paused;g.paused=false;}
    if(e.key.key==SDLK_RETURN || e.key.key==SDLK_ESCAPE || e.key.key==SDLK_SPACE)skip();}
   return true;
  }
  if(e.type==SDL_EVENT_GAMEPAD_BUTTON_DOWN){
   if(e.gbutton.button==SDL_GAMEPAD_BUTTON_START){presentation_paused=!presentation_paused;g.paused=false;}
   if(e.gbutton.button==SDL_GAMEPAD_BUTTON_SOUTH || e.gbutton.button==SDL_GAMEPAD_BUTTON_EAST || e.gbutton.button==SDL_GAMEPAD_BUTTON_BACK)skip();
   return true;
  }
  if(e.type==SDL_EVENT_MOUSE_BUTTON_DOWN){if(e.button.button==SDL_BUTTON_LEFT)skip();return true;}
  return e.type==SDL_EVENT_KEY_UP || e.type==SDL_EVENT_GAMEPAD_BUTTON_UP || e.type==SDL_EVENT_MOUSE_MOTION || e.type==SDL_EVENT_MOUSE_WHEEL;
 }

 if(g.exit.active()){
  if(e.type==SDL_EVENT_WINDOW_FOCUS_LOST)g.exit.paused=true;
  if(e.type==SDL_EVENT_KEY_DOWN){
   if(e.key.key==SDLK_F8 || e.key.key==SDLK_F9 || e.key.key==SDLK_F11)return false;
   if(!e.key.repeat){if(e.key.key==SDLK_P)g.exit.paused=!g.exit.paused;
    if(e.key.key==SDLK_RETURN || e.key.key==SDLK_ESCAPE || e.key.key==SDLK_SPACE)g.exit.finish();}
   return true;
  }
  if(e.type==SDL_EVENT_GAMEPAD_BUTTON_DOWN){
   if(e.gbutton.button==SDL_GAMEPAD_BUTTON_START)g.exit.paused=!g.exit.paused;
   if(e.gbutton.button==SDL_GAMEPAD_BUTTON_SOUTH || e.gbutton.button==SDL_GAMEPAD_BUTTON_EAST || e.gbutton.button==SDL_GAMEPAD_BUTTON_BACK)g.exit.finish();
   return true;
  }
  if(e.type==SDL_EVENT_MOUSE_BUTTON_DOWN){if(e.button.button==SDL_BUTTON_LEFT)g.exit.finish();return true;}
  return e.type==SDL_EVENT_KEY_UP || e.type==SDL_EVENT_GAMEPAD_BUTTON_UP;
 }
 // The old preview tabs can replace the simulation. Tournament play has its
 // own exit through the hub; its on-pitch controls do not use mouse clicks.
 if(tournament_mode_ && !visible() && e.type==SDL_EVENT_MOUSE_BUTTON_DOWN)return true;
 if(e.type==SDL_EVENT_KEY_DOWN){
  if(e.key.key==SDLK_F10 && !e.key.repeat){key(SDLK_F10,g);return true;}
  if(e.key.key==SDLK_F9 || e.key.key==SDLK_F8 || e.key.key==SDLK_F11)return false;
  if(tournament_mode_ && !visible() && (e.key.key==SDLK_R || e.key.key==SDLK_Q || e.key.key==SDLK_T || e.key.key==SDLK_Y || e.key.key==SDLK_F5 || e.key.key==SDLK_M || e.key.key==SDLK_N || e.key.key==SDLK_G || e.key.key==SDLK_H || e.key.key==SDLK_L || e.key.key==SDLK_B || (e.key.key>=SDLK_1 && e.key.key<=SDLK_5)))return true;
  if(!visible() && e.key.key!=SDLK_F2)return false;
  if(!e.key.repeat)key(e.key.key,g);return true;
 }
 if(e.type==SDL_EVENT_GAMEPAD_BUTTON_DOWN){
  if(e.gbutton.button==SDL_GAMEPAD_BUTTON_EAST && stage_==Stage::hidden && g.paused){key(SDLK_F10,g);return true;}
  if(tournament_mode_ && !visible() && (e.gbutton.button==SDL_GAMEPAD_BUTTON_BACK || (e.gbutton.button==SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER && g.sequence->cpu_vs_cpu) || (e.gbutton.button==SDL_GAMEPAD_BUTTON_RIGHT_STICK && g.paused)))return true;
  if(!visible() && e.gbutton.button!=SDL_GAMEPAD_BUTTON_LEFT_STICK)return false;
  SDL_Keycode k=0;
  switch(e.gbutton.button){
   case SDL_GAMEPAD_BUTTON_SOUTH:case SDL_GAMEPAD_BUTTON_START:k=SDLK_RETURN;break;
   case SDL_GAMEPAD_BUTTON_NORTH:k=stage_==Stage::replays?SDLK_R:stage_==Stage::title?SDLK_I:SDLK_Y;break;
   case SDL_GAMEPAD_BUTTON_BACK:case SDL_GAMEPAD_BUTTON_EAST:k=SDLK_ESCAPE;break;
   case SDL_GAMEPAD_BUTTON_LEFT_STICK:k=stage_==Stage::title?SDLK_F10:SDLK_F2;break;
   case SDL_GAMEPAD_BUTTON_RIGHT_STICK:k=stage_==Stage::title?SDLK_L:stage_==Stage::tournament?SDLK_A:SDLK_R;break;
   case SDL_GAMEPAD_BUTTON_WEST:k=stage_==Stage::selection?SDLK_H:SDLK_T;break;
   case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:k=stage_==Stage::title?SDLK_M:stage_==Stage::tactics?SDLK_TAB:stage_==Stage::tournament?SDLK_S:0;break;
   case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:k=stage_==Stage::tactics?SDLK_TAB:SDLK_O;break;
   case SDL_GAMEPAD_BUTTON_DPAD_UP:k=SDLK_UP;break;case SDL_GAMEPAD_BUTTON_DPAD_DOWN:k=SDLK_DOWN;break;
   case SDL_GAMEPAD_BUTTON_DPAD_LEFT:k=SDLK_LEFT;break;case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:k=SDLK_RIGHT;break;
   default:break;
  }
  key(k,g);return true;
 }
 if(visible() && e.type==SDL_EVENT_MOUSE_BUTTON_DOWN){
  if(e.button.button!=SDL_BUTTON_LEFT)return true;
  const float x=(e.button.x-view.x)/1.35f,y=(e.button.y-view.y)/1.35f;
  if(hit_stage_==stage_)for(const auto& hit:menu_hits_)if(x>=hit.bounds.x && x<hit.bounds.x+hit.bounds.w && y>=hit.bounds.y && y<hit.bounds.y+hit.bounds.h){auto action=hit.action;action(g);break;}
  return true;
 }
 if(stage_==Stage::replays && e.type==SDL_EVENT_MOUSE_WHEEL){replay_row_=std::clamp(replay_row_+(e.wheel.y>0?-1:e.wheel.y<0?1:0),0,std::max(0,int(replay_entries_.size())-1));return true;}
 return visible() && (e.type==SDL_EVENT_MOUSE_MOTION || e.type==SDL_EVENT_MOUSE_WHEEL || e.type==SDL_EVENT_KEY_UP || e.type==SDL_EVENT_GAMEPAD_BUTTON_UP);
}
void OriginalFrontend::observe(OriginalGraphics& g,double elapsed){
 menu_clock_+=std::clamp(elapsed,0.0,.25);
 if(preview_ && visible())preview_->advance(elapsed);
 if(stage_==Stage::selection || stage_==Stage::tournament_setup){if(!menu_fading_out())country_carousel_.advance(elapsed);}
 else country_stage_=Stage::hidden;
 if(visible() && !g.exit.active() && !g.prematch.active() && !g.walkoff.active())menu_fade_.advance(elapsed);
 if(replay_watching_){if(!g.replaying){replay_watching_=false;stage_=Stage::replays;g.paused=true;}return;}
 auto& s=*g.sequence;
 if(g.walkoff.active()){if(elapsed<=0)g.walkoff.paused=true;return;}
 if(g.prematch.active()){if(elapsed<=0)g.prematch.paused=true;g.advance_prematch(elapsed);return;}
 if(g.exit.active()){if(elapsed<=0)g.exit.paused=true;g.advance_exit(elapsed);return;}
 if(stage_==Stage::simulating){
  if(elapsed<=0)return;
  for(int i=0;i<200 && s.running;++i){s.step();if(++simulation_ticks_>tournament_.minutes*4800+20000){tournament_error_="SIMULATION TIMEOUT - WATCH OR RETRY";tournament_match_=false;tournament_auto_=false;stage_=Stage::tournament;return;}}
  if(s.full_time){tournament_result(g);stage_=Stage::tournament;tournament_age_=0;}return;
 }
 if(tournament_mode_ && tournament_auto_ && !tournament_match_ && !tournament_.complete() && tournament_error_.empty() && (stage_==Stage::tournament || stage_==Stage::result)){
  tournament_age_+=std::clamp(elapsed,0.0,.25);
  if(tournament_age_>=5){stage_=Stage::tournament;tournament_next(g);if(stage_==Stage::lineup && cpu_)confirm(g);return;}
 }
 if(stage_==Stage::hidden && s.full_match && !g.replaying){
  if(s.full_time && !seen_result_){if(tournament_match_)tournament_result(g);seen_result_=true;tournament_age_=0;stage_=Stage::result;g.paused=true;s.clear_play_input();g.start_exit();}
  else if(s.interval_ticks && s.interval_minute!=seen_interval_){seen_interval_=s.interval_minute;interval_age_=0;stage_=Stage::interval;g.paused=true;s.clear_play_input();if(s.interval_minute==45)g.start_exit();}
 }
 if(g.exit.active())return;
 if(stage_==Stage::interval && s.cpu_vs_cpu){interval_age_+=std::clamp(elapsed,0.0,.25);if(interval_age_>=8)confirm(g);}
}
OriginalMusic::Track OriginalFrontend::music(const OriginalActions& s)const{
 if(stage_==Stage::hidden)return OriginalMusic::Track::none;
 if(stage_!=Stage::result)return OriginalMusic::Track::menu;
 const int home=s.shootout?s.penalty_scores[0]:s.blue_goals,away=s.shootout?s.penalty_scores[1]:s.red_goals;
 if(home==away)return OriginalMusic::Track::draw;
 return s.cpu_vs_cpu || home>away?OriginalMusic::Track::win:OriginalMusic::Track::lose;
}
void OriginalFrontend::draw(SDL_Renderer* r,const OriginalGraphics& g,bool pad,bool muted,int commentary_volume){
 const bool page_visible=visible() && !g.exit.active() && !g.prematch.active() && !g.walkoff.active();
 if(!page_visible || !menu_fades){menu_fade_.reset();menu_frame_ready_=menu_reveal_requested_=false;draw_page(r,g,pad,muted,commentary_volume);return;}
 if(!menu_frame_){
  for(auto** frame:{&menu_frame_,&previous_menu_frame_}){
   *frame=SDL_CreateTexture(r,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_TARGET,960,664);check(*frame!=nullptr);
   check(SDL_SetTextureScaleMode(*frame,SDL_SCALEMODE_LINEAR));check(SDL_SetTextureBlendMode(*frame,SDL_BLENDMODE_NONE));
  }
 }
 if(menu_fade_.page(int(stage_),menu_frame_ready_) && menu_frame_ready_)std::swap(menu_frame_,previous_menu_frame_);
 if(menu_reveal_requested_){menu_fade_.finish();menu_reveal_requested_=false;}
 auto* target=SDL_GetRenderTarget(r);
 check(SDL_SetRenderTarget(r,menu_frame_));
 try{draw_page(r,g,pad,muted,commentary_volume);}catch(...){SDL_SetRenderTarget(r,target);throw;}
 check(SDL_SetRenderTarget(r,target));menu_frame_ready_=true;
 // Keep settled text identical to direct rendering at fractional window/DPI
 // scales. The retained logical-size textures are only displayed during fades.
 if(!menu_fade_.outgoing() && !menu_fade_.shade()){draw_page(r,g,pad,muted,commentary_volume);return;}
 const SDL_FRect screen{0,0,960,664};
 check(SDL_SetRenderDrawColor(r,3,5,26,255));check(SDL_RenderClear(r));
 check(SDL_RenderTexture(r,menu_fade_.outgoing()?previous_menu_frame_:menu_frame_,nullptr,&screen));
 if(menu_fade_.shade()){
  SDL_BlendMode blend;Uint8 red,green,blue,alpha;
  check(SDL_GetRenderDrawBlendMode(r,&blend));check(SDL_GetRenderDrawColor(r,&red,&green,&blue,&alpha));
  check(SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND));check(SDL_SetRenderDrawColor(r,0,0,0,menu_fade_.shade()));
  check(SDL_RenderFillRect(r,nullptr));
  check(SDL_SetRenderDrawBlendMode(r,blend));check(SDL_SetRenderDrawColor(r,red,green,blue,alpha));
 }
}

}

namespace actua {
void OriginalFrontend::tournament_save(){
 try{tournament_.save(save_path_);save_status_="PROGRESS SAVED";tournament_error_.clear();}
 catch(const std::exception& error){SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Tournament save failed at %s: %s",save_path_.string().c_str(),error.what());tournament_error_="SAVE FAILED - S TO RETRY";save_status_.clear();tournament_auto_=false;}
}
void OriginalFrontend::tournament_load(OriginalGraphics& g){
 try{
  bool legacy=false,recovered=false;
  try{tournament_.load(save_path_);}
  catch(const std::exception&){
   auto backup=save_path_;backup+=".bak";
   try{tournament_.load(backup);recovered=true;}
   catch(const std::exception&){
    legacy=!std::filesystem::exists(save_path_) && !legacy_save_path_.empty() && std::filesystem::exists(legacy_save_path_);
    if(!legacy)throw;
    tournament_.load(legacy_save_path_);
   }
  }
  if(legacy)tournament_.save(save_path_); // Copy validated progress; leave the legacy file untouched.
  tournament_mode_=true;tournament_match_=false;tournament_auto_=false;tournament_error_.clear();save_status_=recovered?"BACKUP LOADED - PREVIOUS CHECKPOINT":"SAVE LOADED";tournament_page_=tournament_.results.size()>=30?6:tournament_.results.size()>=28?5:tournament_.results.size()>=24?4:0;stage_=Stage::tournament;g.paused=true;g.sequence->clear_play_input();}
 catch(const std::exception& error){SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Tournament load failed at %s: %s",save_path_.string().c_str(),error.what());tournament_error_="SAVE MISSING OR INVALID";}
}
void OriginalFrontend::tournament_next(OriginalGraphics& g,bool simulate){
 if(tournament_match_){stage_=Stage::hidden;g.paused=was_paused_;return;}
 if(tournament_.complete())return;
 const auto teams=tournament_.next();const bool own=teams[0]==tournament_.user || teams[1]==tournament_.user;
 if(simulate && own){tournament_error_="YOUR MATCH - ENTER TO PLAY";return;}
 if(tournament_error_=="SAVE FAILED - S TO RETRY")return;
 const int venue=tournament_.fixture_venue(int(tournament_.results.size()));
 if(tournament_.scheduled_venues && !g.stadium_available(venue)){tournament_error_="SCHEDULED VENUE UNAVAILABLE";tournament_auto_=false;return;}
 tournament_error_.clear();try{tournament_.prepare(*g.sequence);}catch(const std::exception& e){tournament_error_=e.what();tournament_auto_=false;return;}
 g.next_stadium=venue;g.match_camera=tournament_.camera;g.action_view=true;g.overview=false;
 if(simulate || (tournament_.user>=0 && !own)){
  g.sequence->start_match(true);g.paused=true;simulation_ticks_=0;tournament_match_=true;stage_=Stage::simulating;
 }else ready(g,!own);
}
void OriginalFrontend::tournament_result(OriginalGraphics& g){
 if(!tournament_match_)return;
 tournament_.collect(*g.sequence);tournament_match_=false;tournament_save();
 if(tournament_.results.size()>=24)tournament_page_=tournament_.results.size()>=30?6:tournament_.results.size()>=28?5:4;
 if(tournament_.complete()){tournament_auto_=false;trophy_pending_=true;if(stage_==Stage::simulating)request_trophy(g);}
}
void OriginalFrontend::request_trophy(const OriginalGraphics& g){
 trophy_pending_=false;if(!trophy_available_ || !tournament_.complete())return;
 const auto& final=tournament_.results.back();trophy_requested=g.trophy_film(final.home,final.away,tournament_.champion());movie_error.clear();
 if(trophy_requested<0)movie_error="TROPHY FILM UNAVAILABLE";
}

}

namespace actua {
int OriginalFrontend::tactics_team(const OriginalGraphics& g)const{
 const auto& s=*g.sequence;
 return tactics_return_==Stage::interval?(side_?s.away_team:s.home_team):(side_?s.next_away:s.next_home);
}
std::array<int,20> OriginalFrontend::tactics_squad(const OriginalGraphics& g)const{
 const auto& s=*g.sequence;std::array<int,20> order{};
 if(tactics_return_==Stage::interval){
  order=s.match_order[side_];const int start=side_?11-s.blue_slot():s.blue_slot();
  // List the actual eleven after substitutions, then the other squad members once.
  for(int j=0;j<11;++j)order[j]=s.engine.teams[start+j].shirt;
  int at=11;for(int shirt=1;shirt<=20;++shirt)if(std::find(order.begin(),order.begin()+11,shirt)==order.begin()+11)order[at++]=shirt;
 }else if(tournament_mode_)order=tournament_.squad[tactics_team(g)];
 else for(int j=0;j<20;++j)order[j]=j+1;
 return order;
}
bool OriginalFrontend::tactics_editable(const OriginalGraphics& g)const{
 if(tournament_mode_)return tournament_.user<0 || tournament_.user==tactics_team(g);
 return (tactics_return_==Stage::interval?(g.sequence->cpu_vs_cpu || g.sequence->head_to_head):(cpu_ || g.sequence->next_head_to_head)) || side_==0;
}
void OriginalFrontend::open_tactics(OriginalGraphics& g){
 if(stage_!=Stage::lineup && stage_!=Stage::interval)return;
 tactics_return_=stage_;tactics_page_=0;tactics_row_=0;tactics_player_=std::min(squad_cursor_,10);tactics_message_.clear();
 if(tactics_return_==Stage::interval)side_=0;
 stage_=Stage::tactics;g.paused=true;g.sequence->clear_play_input();
}
void OriginalFrontend::tactics_key(SDL_Keycode key,OriginalGraphics& g){
 if(key==SDLK_ESCAPE || key==SDLK_RETURN || key==SDLK_T || key==SDLK_F2){back(g);return;}
 if(key==SDLK_TAB){side_=1-side_;tactics_message_.clear();return;}
 if(key==SDLK_Y){tactics_page_=(tactics_page_+1)%3;if(tactics_page_==0)tactics_player_%=11;tactics_message_.clear();return;}
 if(key==SDLK_LEFT || key==SDLK_RIGHT){tactics_change(g,key==SDLK_LEFT?-1:1);return;}
 if(key==SDLK_UP || key==SDLK_DOWN){const int delta=key==SDLK_UP?-1:1;
  if(tactics_page_==1)tactics_row_=(tactics_row_+delta+3)%3;
  else{const int count=tactics_page_==0?11:20;tactics_player_=(tactics_player_+delta+count)%count;}
 }
 if(key==SDLK_S && tournament_mode_){tournament_save();tactics_message_=tournament_error_.empty()?"TACTICS SAVED":tournament_error_;}
}
void OriginalFrontend::tactics_change(OriginalGraphics& g,int delta){
 if(tactics_page_==2){tactics_player_=(tactics_player_+delta+20)%20;return;}
 if(!tactics_editable(g)){tactics_message_="OPPOSITION - VIEW ONLY";return;}
 auto& s=*g.sequence;const bool live=tactics_return_==Stage::interval;const int team=tactics_team(g);
 if(tactics_page_==0){
  int& formation=live?(side_?s.away_formation:s.home_formation):(side_?s.next_away_formation:s.next_home_formation);
  const int next=(formation+delta+10)%10;
  if(live){if(!s.set_interval_formation(side_!=0,next)){tactics_message_="FORMATION CHANGE NEEDS HALF TIME";return;}}
  else formation=next;
  if(tournament_mode_)tournament_.formations[team]=next;
 }else{
  auto& preferences=live?s.match_takers[side_]:s.next_takers[side_];
  std::vector<int> options{0};const auto order=tactics_squad(g);
  const int start=side_?11-s.blue_slot():s.blue_slot();
  for(int j=1;j<11;++j)if(live?s.engine.teams[start+j].guy_on:!tournament_mode_ || tournament_.available(team,order[j]))options.push_back(order[j]);
  const auto found=std::find(options.begin(),options.end(),preferences[tactics_row_]);
  const int current=found==options.end()?0:int(found-options.begin());
  preferences[tactics_row_]=options[(current+delta+int(options.size()))%options.size()];
  if(live)s.refresh_setpiece_takers();
  if(tournament_mode_)tournament_.takers[team]=preferences;
 }
 if(tournament_mode_){tournament_save();tactics_message_=tournament_error_.empty()?"TACTICS SAVED":tournament_error_;}
 else tactics_message_=live?"APPLIED FOR NEXT HALF":"READY FOR KICK OFF";
}

}

namespace actua {
void OriginalFrontend::save_preferences(){
 try{if(preferences_enabled_)preferences_.save(preferences_path_);preferences_status_="SETTINGS SAVED";}
 catch(const std::exception& error){SDL_Log("Preferences save: %s",error.what());preferences_status_="SAVE FAILED - S TO RETRY";}
}
void OriginalFrontend::audio_shortcut(bool mute){
 if(mute)preferences_.muted=!preferences_.muted;
 else preferences_.commentary=preferences_.commentary?std::max(0,preferences_.commentary-25):100;
 save_preferences();
}
void OriginalFrontend::open_options(OriginalGraphics& g){
 options_return_=stage_;stage_=Stage::options;options_page_=options_row_=0;g.paused=true;g.sequence->clear_play_input();
}
void OriginalFrontend::options_key(SDL_Keycode key,OriginalGraphics& g){
 if(key==SDLK_ESCAPE || key==SDLK_RETURN || key==SDLK_O || key==SDLK_F2){back(g);return;}
 if(key==SDLK_Y){options_page_=1-options_page_;options_row_=0;return;}
 const int count=6;
 if(key==SDLK_UP)options_row_=(options_row_+count-1)%count;
 if(key==SDLK_DOWN)options_row_=(options_row_+1)%count;
 if(key==SDLK_LEFT || key==SDLK_RIGHT)options_change(g,key==SDLK_LEFT?-1:1);
 if(key==SDLK_S)save_preferences();
}
void OriginalFrontend::options_change(OriginalGraphics& g,int delta){
 auto& p=preferences_;auto& m=p.match;
 if(!options_page_){switch(options_row_){
  case 0:m.wind=(m.wind+delta+4)%4;break;
  case 1:m.offside=!m.offside;break;
  case 2:m.fouls=!m.fouls;break;
  case 3:m.bookings=!m.bookings;break;
  case 4:m.referee_accuracy=std::clamp(m.referee_accuracy+delta*10,0,100);break;
  case 5:m.referee_strictness=std::clamp(m.referee_strictness+delta*10,0,100);break;
 }g.sequence->next_options=m;
 }else{
  int* levels[]={&p.music,&p.effects,&p.crowd,&p.commentary};
  if(options_row_<4)*levels[options_row_]=std::clamp(*levels[options_row_]+delta*25,0,100);
  else if(options_row_==4)p.muted=!p.muted;
  else if(french_available_)p.commentary_language=1-p.commentary_language;
  else {preferences_status_="FRENCH RECORDINGS UNAVAILABLE";return;}
 }
 save_preferences();
}

}

#include "original_frontend_pages.inl"
