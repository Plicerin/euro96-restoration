#pragma once
#include "player_original.hpp"
#include <string>
#include <deque>
#include <algorithm>
#include <cstdint>
#include <optional>
#include "original_motion.hpp"
#include "archive.hpp"
#include "original_roster.hpp"
#include "match_preferences.hpp"
namespace actua {

// Original-action replays and live team integration. Native scheduling/input and
// restart transitions surround recovered AI, contacts and set-piece decisions.
class OriginalActions {
public:
    explicit OriginalActions(const std::vector<OriginalClip>& clips);
    std::uint64_t generation=0;
    // Rendering-only event records; live substitutions/discipline remain authoritative.
    struct WalkoffEvent {int index;original::match_player outgoing,incoming;bool replacement;std::string label;};
    bool walkoff_events_enabled=false;
    std::deque<WalkoffEvent> walkoff_events;
    original::PlayerEngine engine;
    int ticks=0,releases=0,receptions=0,release_tick=-1,receive_tick=-1;
    bool running=false,miss=false;
    std::string status="SPACE: original pass and trap";
    void reset();
    void start(bool moving=false,float receiver_offset=0);
    void step();
    void start_attack(float target_y=400,float keeper_y=415);
    void start_continuation(float outlet_x=1060,float outlet_y=260);
    // One automatic five-case review; inputs are diagnostic launches.
    void start_goal_review(int stage=0,bool left=false);
    bool goal_review=false,review_left=false;
    int review_stage=0,review_tick=0,goal_tick=-1;
    static const char* goal_review_name(int stage);
    void start_play();
    void start_team_play(bool cpu=false);
    void start_match(bool cpu=false);
    // Diagnostic fixtures retain their original seed. The interactive app opts
    // into fresh match seeds with nullopt; an explicit override is repeatable.
    std::optional<unsigned int> match_seed_override=1;
    unsigned int match_seed=1;
    // The recovered routines use a current-user input context. Scope it per actor
    // while retaining one shared engine, clock and ball for local head-to-head.
    bool next_head_to_head=false,head_to_head=false;
    class InputScope {
        OriginalActions& owner;int previous;
    public:
        InputScope(OriginalActions& s,int side):owner(s),previous(s.input_side){s.select_input(side);}
        ~InputScope(){owner.select_input(previous);}
        InputScope(const InputScope&)=delete;
        InputScope& operator=(const InputScope&)=delete;
    };
    InputScope input_scope(int side){return InputScope(*this,side);}
    int input_player()const{return input_side;}
    int selected_for(int side)const{return side==input_side?selected:other_input.selected;}
    int control_slot()const{return input_side?11-blue_slot():blue_slot();}
    bool is_controlled_team(int index)const{return is_blue(index)==(input_side==0);}
    int human_for(int index)const{return cpu_vs_cpu?-1:is_blue(index)?0:head_to_head?1:-1;}
    MatchOptions next_options{},match_options{};
    using Takers=std::array<std::array<int,3>,2>;
    Takers next_takers{},match_takers{};
    int preferred_taker(bool away,int role)const;
    void refresh_setpiece_takers();
    bool set_interval_formation(bool away,int formation);
    using SquadOrder=std::array<std::array<int,20>,2>;
    bool next_squad_enabled=false;
    SquadOrder next_order{},next_fitness{},next_available{};
    SquadOrder match_order{},match_fitness{},match_available{};
    struct PlayerReport {int injury=0,bookings=0;};
    using Reports=std::array<std::array<PlayerReport,20>,2>;
    Reports departed_reports{};
    Reports player_reports()const;
    OriginalPlayer match_stats(bool away,int shirt)const;
    int bench_shirt(bool away,int index)const{return match_order[away][11+index];}
    void set_match_tactics(const std::array<Tactics,10>& values){formations=values;rosters_ready=true;}
    int next_home=4,next_away=12,next_home_formation=4,next_away_formation=2;
    int home_team=4,away_team=12,home_formation=4,away_formation=2;
    bool national_teams=false;
    void change_team(bool away,int delta);
    std::string team_label(bool away) const;
    std::string player_label(int index) const;
    std::string match_text(std::string value) const;
    int match_minutes=5; // Original time_table choices: 5, 10, 20, 40, 90.
    void set_match_minutes(int minutes);
    bool full_match=false,full_time=false;
    // Presentation context, consumed at launch; never part of gameplay or save data.
    int next_tournament_fixture=-1,tournament_fixture=-1;
    bool next_original_draw=false,original_draw=false;
    int interval_ticks=0,interval_minute=45;
    enum class MatchFormat {regulation,knockout,golden,penalties};
    MatchFormat next_format=MatchFormat::regulation,match_format=MatchFormat::regulation;
    static const char* format_name(MatchFormat);
    bool shootout=false,penalty_released=false;
    int penalty_wait=0,penalty_age=0,penalty_side=0;
    std::array<std::vector<int>,2> penalty_results;
    std::array<int,2> penalty_scores{},match_passes{},match_shots{};
    void start_shootout();
    bool penalty_human_taker()const{return shootout && !cpu_vs_cpu && !penalty_wait && !full_time && penalty_side==input_side;}
    bool penalty_human_keeper()const{return shootout && !cpu_vs_cpu && !penalty_wait && !full_time && penalty_side!=input_side;}
    bool penalty_aiming()const{return shootout && !penalty_wait && !full_time && engine.match_mode && engine.dead_ball_cnt<=1 && engine.penalty_taker>0 && engine.teams[engine.penalty_taker-1].int_move!=15;}
    void set_penalty_input(float direction,bool shot);
    void clear_penalty_input();
    std::string result_label()const;
    std::string period_label()const;
    void finish_match();
    void step_shootout();
    void finish_penalty(bool goal);
    struct Substitution {int slot=-1,shirt=0;};
    std::array<Substitution,2> pending_subs{};
    std::array<std::array<bool,20>,2> used_squad{};
    std::array<int,2> substitutions{},sub_passes{},sub_shots{};
    using Figures=std::array<std::array<int,15>,2>;
    std::array<int,2> possession_ticks{};
    Figures departed_figures{},restart_figures{},shootout_figures{};
    Figures match_figures()const;
    static std::array<int,15> player_figures(const original::match_player&);
    void record_restart_statistics(int mode);
    struct Notice {std::string title,detail;int sprite=0,ticks=0;};
    Notice match_notice;
    void notice(std::string title,std::string detail,int sprite=0){match_notice={std::move(title),std::move(detail),sprite,100};}
    int seen_card_events=0,card_notice_ticks=0;
    std::string card_notice,sub_notice;
    bool queue_substitution(bool away,int slot,int shirt);
    void apply_substitutions();
    void update_match_depth();
    void step_celebration();
    float pace_remaining(int index) const;
    bool bench_available(bool away,int shirt) const;

    int blue_slot() const {return full_match && (engine.match_half&1)?11:0;}
    bool is_blue(int index) const {return (index<11)==(blue_slot()==0);}
    std::string clock_label() const;
    std::string match_restart_label() const;
    void set_cpu_control(bool enabled);
    void restart_play(){if(full_match)start_match(cpu_vs_cpu);else if(team_play)start_team_play(cpu_vs_cpu);else start_play();}
    void set_team_tactics(const Tactics& tactics){team_tactics=tactics;tactics_ready=true;}
    bool team_play=false,cpu_vs_cpu=false;
    int turnovers=0,ai_passes=0,ai_shots=0;
    int blue_goals=0,red_goals=0,restarts=0,restart_mode=0,restart_ticks=0;
    static const char* restart_name(int mode);
    void set_play_input(float x,float y);
    void set_play_buttons(bool shot,bool pass);
    int live_charge()const{return std::max(live_f1,live_f2);}
    bool live_controls=false;
    bool human_restart()const;
    bool human_keeper_hold()const;
    void set_restart_input(float x,float y,bool shot,bool pass,bool loft);
    void clear_restart_input();
    void practice_restart(int stage);
    int restart_practice=-1;
    std::string restart_help()const;
    enum class PlayKick {none,ground,loft,shot};
    bool play=false;
    int selected=1,pass_target=0;
    void request_play_kick(PlayKick kick);
    void switch_play_player();
    // One live tick after distances/prediction; extracted USER.CPP selection rules.
    void update_auto_selection();
    void clear_play_input();
    void clear_player_input();
    bool continuation=false;
    int distributions=0,distribution_type=0,distribution_animation=0,distribution_tick=-1,catch_tick=-1,recovery_tick=-1;
    int carry_ticks=0,limbo_ticks=0,outlet=0;
    bool attack=false,goal_mouth=false;
    int first_touches=0,shots=0,saves=0,planned_player=0;
    std::vector<int> visible_players{1,2};
private:
    original::PlayerEngine::Frames captures;
    struct InputState {
        int selected=12,live_f1=0,live_f2=0,live_player=-1,selection_wait=0,manual_selection_ticks=0,seen_selection_events=0;
        float play_x=0,play_y=0,restart_x=0,restart_y=0,penalty_direction=0;
        bool live_controls=false,live_shot=false,live_pass=false,live_blocked=false;
        bool restart_armed=false,restart_shot=false,restart_pass=false,restart_loft=false,restart_high_pass=false;
        bool penalty_button=false,penalty_armed=false;
        PlayKick play_kick=PlayKick::none;
    } other_input;
    int input_side=0;
    void select_input(int side);
    std::array<original::PlayerEngine::Capture,132> capture_index{};
    float outlet_target_x=1060,outlet_target_y=260;
    bool receiver_prepared=false,retreat_started=false;
    void prepare_receiver();
    void step_attack();
    void step_goal_review();
    void step_play();
    void step_team_play();
    void prepare_restart();
    void step_restart();
    bool step_match_rules();
    void accept_rule_restart();
    int seen_rule_events=0;
    void load_match_tactics();
    std::array<Tactics,10> formations{};
    bool rosters_ready=false;
    Tactics team_tactics{};
    bool tactics_ready=false;
    bool restart_armed=false,restart_shot=false,restart_pass=false,restart_loft=false,restart_started=false,restart_high_pass=false;
    float restart_x=0,restart_y=0;
    float play_x=0,play_y=0;
    bool live_shot=false,live_pass=false,live_blocked=false;
    int live_f1=0,live_f2=0,live_player=-1;
    int selection_wait=0,manual_selection_ticks=0,seen_selection_events=0;
    float penalty_direction=0;
    bool penalty_button=false,penalty_armed=false;
    PlayKick play_kick=PlayKick::none;
    int play_end_ticks=0;
};
}
