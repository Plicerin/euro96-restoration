#pragma once
#include "player_original.hpp"
#include <array>
namespace actua {
class OriginalCrowd {
public:
 std::array<int,2> update(const original::PlayerEngine& e,bool kickoff_value=false){kickoff=kickoff_value;process_crowd_noise(e);return {crowd_mood_a,crowd_mood_b};}
 int shot_missed=0,shot_saved=0,card_is_out=0;
private:
 float poss_held=0,possession=0;
 bool kickoff=false;
 int crowd_mood_a=1,crowd_mood_b=1;
 void reset_possession();
 void accumulate_possession(const original::PlayerEngine&);
 void process_crowd_noise(const original::PlayerEngine&);
};
}
