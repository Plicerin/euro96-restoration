#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "action_constants.hpp"
#include <algorithm>
namespace actua {
void OriginalGraphics::record_replay(){
    const auto& s=*sequence;const auto& e=s.engine;
    // Original replay_seconds=15, REAL_SPEED=20; native owned snapshots replace plyrdat memory.
    if(!s.full_match || s.shootout){history.clear();recorded_tick=-1;replayed_goals=0;return;}
    if(s.ticks<recorded_tick || (s.ticks<=1 && history_generation!=s.generation)){history.clear();replayed_goals=0;}
    history_generation=s.generation;recorded_tick=s.ticks;
    if(s.restart_ticks && !e.just_scored || e.just_scored>0 && e.just_scored<original::action_constants::ORIGINAL_SCORE_WAIT || s.interval_ticks || s.full_time)return;
    history.push_back({e.teams,e.refs,e.ballx,e.bally,e.ballz,e.ball_horz_ang,e.ball_vert_ang,e.ballxdis,e.ballydis,e.ball_poss,e.ball_limbo_on,e.ball_limbo_p,e.last_touch,e.match_half});
    if(history.size()>300)history.pop_front();
}
bool OriginalGraphics::start_replay(bool automatic){
    if(replaying || !sequence->full_match || sequence->shootout || history.size()<2 || sequence->ticks<recorded_tick || history_generation!=sequence->generation)return false;
    replay_engine=std::make_unique<original::PlayerEngine>(sequence->engine);
    replay_saved_camera=match_camera;camera_motion={};replaying=true;replay_was_paused=paused;paused=false;sequence->clear_play_input();
    replay_cursor=automatic?std::max(0,int(history.size())-160):0;
    replay_rate=5;replay_loop=false;replay_automatic=automatic;clock=0;replay_message.clear();
    replayed_goals=sequence->engine.goal_events;replay_presentation={};load_replay_frame();
    replay_presentation.observe(*replay_engine,1,replay_cursor);camera_generation=~std::uint64_t(0);step_camera();return true;
}
void OriginalGraphics::seek_replay(int frame){
    if(!replaying)return;
    replay_cursor=std::clamp(frame,0,int(history.size())-1);paused=true;clock=0;replay_automatic=false;
    load_replay_frame();replay_presentation={};replay_presentation.observe(*replay_engine,1,replay_cursor);
    camera_motion={};camera_generation=~std::uint64_t(0);step_camera();
}
void OriginalGraphics::change_replay_speed(int delta){
    if(!replaying)return;
    // Original 3DENG.C replay menu uses speed/5, with five moving speeds.
    replay_rate=std::clamp(replay_rate+delta,1,5);replay_automatic=false;
}
void OriginalGraphics::toggle_replay_pause(){
    if(!replaying)return;
    paused=!paused;replay_automatic=false;
    if(paused)seek_replay(replay_cursor);
}
void OriginalGraphics::load_replay_frame(){
    const auto& f=history.at(replay_cursor);auto& e=*replay_engine;
    e.teams=f.players;e.refs=f.officials;e.ballx=f.x;e.bally=f.y;e.ballz=f.z;e.ball_horz_ang=f.horizontal;e.ball_vert_ang=f.vertical;
    e.ballxdis=f.xd;e.ballydis=f.yd;e.ball_poss=f.owner;e.ball_limbo_on=f.limbo;e.ball_limbo_p=f.limbo_player;e.last_touch=f.last_touch;e.match_half=char(f.half);
    e.just_scored=e.match_mode=0;
}
void OriginalGraphics::finish_replay(){
    if(!replaying)return;match_camera=replay_saved_camera;camera_motion={};replaying=false;replay_engine.reset();paused=replay_was_paused;
    sequence->clear_play_input();clock=0;camera_generation=~std::uint64_t(0);opening_input_guard=true;
    if(saved_return)restore_replay_scene();else step_camera();
}
std::filesystem::path OriginalGraphics::save_replay(const std::filesystem::path& directory){
 if(!replaying || saved_return || !sequence->national_teams)throw std::runtime_error("Open a match replay before saving");
 SavedReplay clip;clip.home=sequence->home_team;clip.away=sequence->away_team;clip.home_goals=sequence->blue_goals;clip.away_goals=sequence->red_goals;
 clip.stadium=stadium_index;clip.camera=match_camera;clip.frames=history;clip.validate(replay_engine.get());
 clip.wind_on=sequence->engine.wind_on!=0;clip.wind_speed=sequence->engine.wind_speed;clip.wind_x=sequence->engine.wind_x;clip.wind_y=sequence->engine.wind_y;clip.ball_still=replay_engine->ball_still;
 const auto path=clip.save_new(directory);replay_message="REPLAY SAVED";return path;
}
void OriginalGraphics::open_saved_replay(const std::filesystem::path& path){
 if(replaying || prematch.active() || walkoff.active() || exit.active())throw std::runtime_error("Finish the current presentation first");
 auto clip=SavedReplay::load(path);clip.validate(&sequence->engine);
 if(clip.stadium!=stadium_index && !stadium_available(clip.stadium))throw std::runtime_error("Replay venue media is unavailable");
 auto scene=std::make_unique<OriginalActions>(clips);
 scene->full_match=scene->team_play=scene->national_teams=scene->play=scene->running=true;
 scene->engine.wind_on=short(clip.wind_on);scene->engine.wind_speed=clip.wind_speed;scene->engine.wind_x=clip.wind_x;scene->engine.wind_y=clip.wind_y;scene->engine.ball_still=char(clip.ball_still);
 scene->home_team=clip.home;scene->away_team=clip.away;scene->blue_goals=clip.home_goals;scene->red_goals=clip.away_goals;
 scene->visible_players.clear();for(int i=0;i<22;++i)scene->visible_players.push_back(i);
 auto back=std::make_unique<ReplayReturn>(*video_board);
 back->recorded_tick=recorded_tick;back->replayed_goals=replayed_goals;back->history_generation=history_generation;
 back->stadium=stadium_index;back->next_stadium=next_stadium;back->camera=match_camera;
 back->action_view=action_view;back->overview=overview;back->match_panels=match_panels;back->paused=paused;back->clock=clock;
 back->motion=camera_motion;back->previous=camera_previous;back->current=camera_current;back->camera_generation=camera_generation;
 back->presentation=presentation;back->presentation_generation=presentation_generation;
 back->sequence=std::move(sequence);back->history=std::move(history);saved_return=std::move(back);sequence=std::move(scene);
 try{
  history=std::move(clip.frames);recorded_tick=0;history_generation=sequence->generation;replayed_goals=0;
  action_view=true;overview=match_panels=false;next_stadium=clip.stadium;apply_stadium();match_camera=clip.camera;
  video_board->play(OriginalVideoBoard::idle);video_revision=~0u;
  if(!start_replay())throw std::runtime_error("Cannot start saved replay");
  replay_message="SAVED REPLAY";
 }catch(...){replaying=false;replay_engine.reset();restore_replay_scene();throw;}
}
void OriginalGraphics::restore_replay_scene(){
 auto back=std::move(saved_return);sequence=std::move(back->sequence);history=std::move(back->history);
 recorded_tick=back->recorded_tick;replayed_goals=back->replayed_goals;history_generation=back->history_generation;
 next_stadium=back->stadium;apply_stadium();next_stadium=back->next_stadium;match_camera=back->camera;
 action_view=back->action_view;overview=back->overview;match_panels=back->match_panels;paused=back->paused;clock=back->clock;
 camera_motion=back->motion;camera_previous=back->previous;camera_current=back->current;camera_generation=back->camera_generation;
 presentation=back->presentation;presentation_generation=back->presentation_generation;*video_board=std::move(back->board);video_revision=~0u;
 tracker_generation=~std::uint64_t(0);texture_home=texture_away=-1;
}
}
