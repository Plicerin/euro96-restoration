#pragma once
#include "player_original.hpp"
#include <deque>
#include <filesystem>
#include <string>
#include <vector>
namespace actua {
struct ReplayFrame {
 std::array<original::match_player,22> players;
 std::array<original::PlayerEngine::officials,3> officials;
 float x,y,z,horizontal,vertical,xd,yd;
 int owner,limbo,limbo_player,last_touch,half;
};
// Versioned native visual recording, never a serialized PlayerEngine or DOS save.
struct SavedReplay {
 int home=0,away=1,home_goals=0,away_goals=0,stadium=0,camera=0;
 int wind_on=0,ball_still=0;
 float wind_speed=0,wind_x=1,wind_y=0;
 std::deque<ReplayFrame> frames;
 void validate(const original::PlayerEngine* captures=nullptr)const;
 static SavedReplay load(const std::filesystem::path&);
 std::filesystem::path save_new(const std::filesystem::path& directory)const;
 std::string label()const;
};
struct ReplayEntry {std::filesystem::path path;std::string label;bool valid=false;};
std::vector<ReplayEntry> replay_library(const std::filesystem::path& directory);
}
