#pragma once
#include <array>
#include <vector>
#include <filesystem>
#include <string>
namespace actua {
class OriginalActions;
class OriginalTournament {
public:
 struct Standing {int won=0,drew=0,lost=0,goalsFor=0,goalsAgainst=0,points=0,goalDiff=0;};
 struct Result {int home,away,hg,ag,hp=0,ap=0;bool extra=false,pens=false;int winner()const{return (pens?hp>ap:hg>ag)?home:away;}};
 struct PlayerState {int fitness=99,yellows=0,ban=0;};
 std::array<std::array<PlayerState,20>,16> players{};
 std::array<std::array<int,20>,16> squad{};
 std::array<int,16> formations{};
 std::array<std::array<int,3>,16> takers{}; // Penalty, shooting free kick, corner; shirt 0 = auto.
 bool available(int team,int shirt)const;
 bool swap_squad(int team,int a,int b);
 void arrange(int team);
 static const std::array<int,16> default_draw;
 static const std::array<std::array<int,2>,31> fixture_slots;
 static const std::array<int,31> fixture_venues; // Source MatchVenues, retail IDs mapped to native 5-12.
 std::array<int,16> draw=default_draw,positions=default_draw;
 std::array<Standing,16> table{};
 std::vector<Result> results;
 int user=-1,minutes=5,venue=0,camera=0;
 bool active=false;
 bool scheduled_venues=false; // Old saves retain their single selected venue.
 void reset(int team,int length=5,int stadium=0,int view=0);
 void rank();
 std::array<int,2> fixture(int index)const;
 int fixture_venue(int index)const;
 std::array<int,2> next()const{return fixture(int(results.size()));}
 bool complete()const{return results.size()==31;}
 int champion()const{return complete()?results.back().winner():-1;}
 static std::string round(int index);
 void record(const Result&);
 void prepare(OriginalActions&)const;
 bool collect(const OriginalActions&);
 void save(const std::filesystem::path&)const;
 void load(const std::filesystem::path&);
};
}
