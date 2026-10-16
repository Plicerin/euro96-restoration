#pragma once
#include "player_original.hpp"
#include "ball.hpp"
#include <string>
namespace actua {
// Practice orchestration is new. Recovered engine routines own speed, movement,
// dribbling, possession changes, pass lead calculation and player statistics.
class PlayerPractice {
public:
    struct Keeper {bool active=false;float x=0,y=400;int saves=0,parries=0,cooldown=0,reaction=0;bool tracking=false;};
    std::array<Keeper,2> keepers{};
    int keeper_holding=-1,keeper_hold_ticks=0;
    bool opponent_shooting=true; // Allows isolated passing/steering regression scenarios.

    original::PlayerEngine engine;
    int selected=1, cooldown=0, kicker=0, passes_received=0;
    bool paused=false,outside=false,opponents=false;
    int blue_wins=0,red_wins=0,red_passes=0,red_completed=0;
    int blue_score=0,red_score=0,restart_ticks=0;
    bool restart_goal=false,restart_red=false;
    static constexpr int match_duration_ticks=3*60*20;
    int match_elapsed_ticks=0;
    bool match_finished=false;
    int match_seconds_left() const;
    std::string match_result() const;
    void toggle_pause();
    static constexpr float goal_half_width=42;
    std::array<int,2> opponent_windup{};
    static constexpr std::array<int,4> practice_players{1,2,12,13};
    std::uint64_t ticks=0;
    float input_x=0,input_y=0;
    std::string status="Move with arrows or WASD. Space passes to your teammate.";
    std::array<BallPoint,240> trail{};
    std::size_t trail_size=0;
    PlayerPractice();
    void reset();
    void set_opponents(bool enabled);
    bool challenge();
    bool action();
    void set_tackle_held(bool down);
    bool tackling() const { return tackle_window>0; }
    void set_input(float x,float y);
    void step();
    bool pass(bool loft=false);
    bool shoot();
    void switch_player();
    void aim_at(float x,float y);
private:
    void step_play();
    bool await_input_release=false;
    float held_input_x=0,held_input_y=0;
    int possession_grace=0,tackle_window=0;
    bool tackle_held=false;
    std::array<int,22> tackle_cooldown{};
    BallPoint restart_point{};
    bool check_boundary(float x,float y,float z,const std::array<BallPoint,22>& previous_players);
    void finish_restart();
    void move_player(int index,float x,float y);
    void move_opponents();
    void consider_opponent_pass();
    bool consider_opponent_shot();
    int red_shot_cooldown=0;
    void reset_keepers(bool clear_stats);
    void move_keepers();
    void keeper_contact(int side,float x,float y,float z);
    void keeper_release();
    int red_receiver=0,red_receive_ticks=0,red_pass_cooldown=0,red_decision_ticks=0;
    BallPoint red_receive_target{};
    int opponent_presser=-1,opponent_role_ticks=0,opponent_holder=0,dribble_ticks=0;
    BallPoint dribble_target{},opponent_support_offset{};
    void move_teammate();
    void move_to(int index,float x,float y);
    float support_score(int index,float x,float y,float anchor_x,float anchor_y) const;
    bool support_target_valid=false;
    BallPoint support_target{},receive_target{};
    int assisted_receiver=0,receive_ticks=0;
    void challenge_opponents();
    bool try_challenge(int index);
    void gain_ball(int index,bool tackle);
    void release(int receiver);
    bool receive(float previous_x,float previous_y,float previous_z, const std::array<BallPoint,22>& previous_players,float max_time=1);
    void select(int index);
};
}
