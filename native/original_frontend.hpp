#pragma once
#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "original_music.hpp"
#include "original_tournament.hpp"
#include "original_menu_fade.hpp"
#include "original_menu_art.hpp"
#include "original_preview.hpp"
#include <functional>
namespace actua {
// Original artwork and roster data, with native navigation around the restored match.
class OriginalFrontend {
public:
 enum class Stage {hidden,title,selection,lineup,interval,result,tournament_setup,tournament,simulating,tactics,options,replays};
 OriginalFrontend(const std::filesystem::path& data,const std::filesystem::path& cd,const std::filesystem::path& save_path={},bool preferences_enabled=false,bool legacy_film_diagnostics=false);
 ~OriginalFrontend();
 static std::filesystem::path default_save_path();
 const std::filesystem::path& save_path()const{return save_path_;}
 void open(OriginalGraphics&);
 void save_replay(OriginalGraphics&);
 const std::vector<ReplayEntry>& replays()const{return replay_entries_;}
 const std::string& replay_status()const{return replay_status_;}
 void setup(OriginalGraphics&);
 bool event(const SDL_Event&,OriginalGraphics&);
 void observe(OriginalGraphics&,double elapsed);
 void draw(SDL_Renderer*,const OriginalGraphics&,bool pad,bool muted,int commentary_volume=75);
 bool visible()const{return stage_!=Stage::hidden;}
 bool ending()const{return stage_==Stage::interval || stage_==Stage::result;}
 const OriginalTournament& tournament()const{return tournament_;}
 const std::string& tournament_error()const{return tournament_error_;}
 const std::string& preferences_status()const{return preferences_status_;}
 const MatchPreferences& preferences()const{return preferences_;}
 const std::filesystem::path& preferences_path()const{return preferences_path_;}
 void audio_shortcut(bool mute);
 void commentary_unavailable(int active_language){preferences_.commentary_language=active_language;preferences_status_="VOICE UNAVAILABLE - PREVIOUS LANGUAGE KEPT";}
 int connected_controllers=0;
 bool menu_fades=true;
 bool menu_fading_out()const{return menu_fades && (menu_reveal_requested_ || menu_fade_.outgoing() || (menu_frame_ready_ && visible() && menu_fade_.page_id()!=int(stage_)));}
 std::string local_message;
 bool quit=false;
 bool movie_requested=false,intro_requested=false;
 int trophy_requested=-1;
 std::string movie_error;
 Stage stage()const{return stage_;}
 int row()const{return row_;}
 OriginalMusic::Track music(const OriginalActions&)const;
 OriginalMenuArt& menu_art(){return *art_;}
private:
 std::shared_ptr<OriginalMenuArt> art_;
 std::unique_ptr<OriginalPreview> preview_;
 void menu_preview(int slot,int x,int y,int w,int h);
 double menu_clock_=0;
 int title_choice_=0;
 std::vector<std::pair<const char*,SDL_Keycode>> title_entries()const;
 OriginalMenuCarousel country_carousel_;
 Stage country_stage_=Stage::hidden;
 void menu_country(int team){country_carousel_.select(team,country_stage_!=stage_);country_stage_=stage_;art_->country_frame(country_carousel_.frame());}
 struct MenuHit {SDL_FRect bounds;std::function<void(OriginalGraphics&)> action;};
 std::vector<MenuHit> menu_hits_;
 Stage hit_stage_=Stage::hidden;
 void menu_button(int x,int y,int w,const std::string&,SDL_Keycode,bool selected=false);
 void menu_hit(int x,int y,int w,int h,std::function<void(OriginalGraphics&)>);
 void menu_footer(const std::string&);
 void menu_formation(const OriginalGraphics&,int team,int formation,const std::array<int,20>& order,int selected=-1);
 OriginalMenuFade menu_fade_;
 SDL_Texture* menu_frame_=nullptr;
 SDL_Texture* previous_menu_frame_=nullptr;
 bool menu_frame_ready_=false;
 bool menu_reveal_requested_=false;
 void draw_page(SDL_Renderer*,const OriginalGraphics&,bool pad,bool muted,int commentary_volume);
 Stage replay_return_=Stage::title;
 bool replay_return_paused_=true,replay_watching_=false;
 int replay_row_=0;
 std::vector<ReplayEntry> replay_entries_;
 std::string replay_status_;
 void open_replays(OriginalGraphics&);
 void refresh_replays();
 void play_saved_replay(OriginalGraphics&);
 bool movie_available_=false,intro_available_=false;
 bool trophy_available_=false,trophy_pending_=false;
 void request_trophy(const OriginalGraphics&);
 bool french_available_=false;
 MatchPreferences preferences_;
 std::filesystem::path preferences_path_;
 bool preferences_enabled_=false;
 Stage options_return_=Stage::title;
 int options_page_=0,options_row_=0;
 std::string preferences_status_;
 void save_preferences();
 void open_options(OriginalGraphics&);
 void options_key(SDL_Keycode,OriginalGraphics&);
 void options_change(OriginalGraphics&,int);
 void options_draw(SDL_Renderer*,bool);
 OriginalTournament tournament_;
 std::filesystem::path save_path_,legacy_save_path_;
 std::string tournament_error_,save_status_;
 bool tournament_mode_=false,tournament_match_=false,tournament_auto_=false;
 int tournament_team_=-1,tournament_page_=0,simulation_ticks_=0;
 double tournament_age_=0;
 void tournament_save();
 void tournament_load(OriginalGraphics&);
 void tournament_next(OriginalGraphics&,bool simulate=false);
 void tournament_result(OriginalGraphics&);
 void tournament_draw(SDL_Renderer*,const OriginalGraphics&,bool);
 Stage stage_=Stage::hidden,tactics_return_=Stage::lineup;
 int tactics_page_=0,tactics_row_=0,tactics_player_=0;
 std::string tactics_message_;
 void open_tactics(OriginalGraphics&);
 void tactics_key(SDL_Keycode,OriginalGraphics&);
 void tactics_change(OriginalGraphics&,int);
 void tactics_draw(SDL_Renderer*,const OriginalGraphics&,bool);
 int tactics_team(const OriginalGraphics&)const;
 std::array<int,20> tactics_squad(const OriginalGraphics&)const;
 bool tactics_editable(const OriginalGraphics&)const;
 int squad_cursor_=0,squad_pick_=-1;
 std::string squad_message_;
 void squad_select(OriginalGraphics&);
 int row_=0,side_=0,seen_interval_=0;
 bool cpu_=true,return_match_=false,was_paused_=false,seen_result_=false;
 double interval_age_=0;
  std::array<Tactics,10> tactics_;
 void key(SDL_Keycode,OriginalGraphics&);
 void change(OriginalGraphics&,int);
 void ready(OriginalGraphics&,bool cpu,bool local=false);
 void confirm(OriginalGraphics&);
 void back(OriginalGraphics&);
};
}
