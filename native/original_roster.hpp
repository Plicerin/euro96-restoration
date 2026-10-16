#pragma once
#include <array>
#include "player_original.hpp"
namespace actua {
struct OriginalPlayer {const char* name; int pace,power,control,flair,vision,accuracy,stamina,discipline; const char* position; int fitness=99;};
struct OriginalTeam {const char* name; const char* code; int formation,kit; std::array<OriginalPlayer,20> players;};
extern const std::array<OriginalTeam,16> original_teams;
OriginalPlayer conditioned_stats(const OriginalPlayer&,int fitness);
void apply_original_stats(original::match_player& player,const OriginalPlayer& stats);
}
