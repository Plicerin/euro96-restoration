#include "original_tournament.hpp"
#include "original_actions.hpp"
#include "original_roster.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
namespace actua {
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
bool length_ok(int n){return n==5 || n==10 || n==20 || n==40 || n==90;}
std::uint64_t checksum(const std::string& s){std::uint64_t h=1469598103934665603ull;for(unsigned char c:s){h^=c;h*=1099511628211ull;}return h;}
}
void OriginalTournament::reset(int team,int length,int stadium,int view){
 require(team>=-1 && team<16 && length_ok(length) && stadium>=0 && stadium<13 && view>=0 && view<3,"Invalid tournament settings");
 *this=OriginalTournament{};user=team;minutes=length;venue=stadium;camera=view;active=true;
 for(int t=0;t<16;++t)for(int p=0;p<20;++p){squad[t][p]=p+1;players[t][p].fitness=original_teams[t].players[p].fitness;}
 for(int t=0;t<16;++t)formations[t]=original_teams[t].formation;
 rank();
}
std::string OriginalTournament::round(int index){return index<24?"GROUP MATCH "+std::to_string(index+1)+" OF 24":index<28?"QUARTER FINAL "+std::to_string(index-23):index<30?"SEMI FINAL "+std::to_string(index-27):index==30?"FINAL":"TOURNAMENT COMPLETE";}
std::array<int,2> OriginalTournament::fixture(int index)const{
 if(index<0 || index>=31)return {-1,-1};
 auto resolve=[&](int code){
  if(code>=20)return results.size()>std::size_t(28+code-20)?results[28+code-20].winner():-1;
  if(code>=16)return results.size()>std::size_t(24+code-16)?results[24+code-16].winner():-1;
  if(index>=24 && results.size()<24)return -1;
  const auto& slots=index<24?draw:positions;
  auto it=std::find(slots.begin(),slots.end(),code);return it==slots.end()?-1:int(it-slots.begin());
 };
 return {resolve(fixture_slots[index][0]),resolve(fixture_slots[index][1])};
}
int OriginalTournament::fixture_venue(int index)const{
 if(index<0 || index>=int(fixture_venues.size()))return -1;
 return scheduled_venues?fixture_venues[index]:venue;
}
void OriginalTournament::record(const Result& r){
 require(active && !complete(),"Tournament has no pending fixture");const auto teams=next();
 require(r.home==teams[0] && r.away==teams[1] && r.home>=0 && r.away>=0 && r.home!=r.away,"Result does not match next fixture");
 require(r.hg>=0 && r.hg<=1000 && r.ag>=0 && r.ag<=1000 && r.hp>=0 && r.hp<=1000 && r.ap>=0 && r.ap<=1000,"Invalid score");
 const bool group=results.size()<24;
 require(!group || (!r.extra && !r.pens),"Group match cannot use extra time or penalties");
 require(r.pens?(r.extra && r.hg==r.ag && r.hp!=r.ap):(r.hp==0 && r.ap==0),"Invalid penalty result");
 require(group || r.pens || r.hg!=r.ag,"Knockout match needs a winner");
 if(group){auto& h=table[r.home];auto& a=table[r.away];h.goalsFor+=r.hg;h.goalsAgainst+=r.ag;a.goalsFor+=r.ag;a.goalsAgainst+=r.hg;
  if(r.hg>r.ag){++h.won;++a.lost;}else if(r.hg<r.ag){++a.won;++h.lost;}else{++a.drew;++h.drew;}}
 results.push_back(r);rank();
}
bool OriginalTournament::available(int team,int shirt)const{
 const auto& p=players.at(team).at(shirt-1);return p.fitness>=25 && p.ban==0;
}
bool OriginalTournament::swap_squad(int team,int a,int b){
 if(team<0 || team>=16 || a<0 || a>=20 || b<0 || b>=20)return false;
 const auto eligible=[&](int slot,int shirt){return slot>=15 || (available(team,shirt) && (slot!=0 || std::string(original_teams[team].players[shirt-1].position)=="GK"));};
 if(!eligible(a,squad[team][b]) || !eligible(b,squad[team][a]))return false;
 std::swap(squad[team][a],squad[team][b]);return true;
}
void OriginalTournament::arrange(int team){
 // EURO_SQD moves unavailable players behind the playable squad. Keep a
 // goalkeeper in slot zero and prefer a matching role for native replacements.
 for(int slot=0;slot<15;++slot){const auto shirt=squad[team][slot];const auto role=std::string(original_teams[team].players[shirt-1].position);
  if(available(team,shirt) && (slot!=0 || role=="GK"))continue;
  int replacement=-1;
  for(int j=std::max(slot+1,slot<11?11:15);j<20;++j)if(available(team,squad[team][j])){
   const auto other=std::string(original_teams[team].players[squad[team][j]-1].position);
   if((slot==0 && other!="GK") || (slot>0 && slot<11 && other=="GK"))continue;
   if(replacement<0)replacement=j;
   if(other==role){replacement=j;break;}
  }
  if(replacement>=0)std::swap(squad[team][slot],squad[team][replacement]);
 }
}
void OriginalTournament::prepare(OriginalActions& s)const{
 require(active && !complete(),"No next tournament match");auto teams=next();require(teams[0]>=0 && teams[1]>=0,"Unresolved tournament fixture");
 // The native controller owns the home slot. Map the chosen country there,
 // then collect() maps the finished score back to the preserved fixture order.
 if(user==teams[1])std::swap(teams[0],teams[1]);
 s.next_home=teams[0];s.next_away=teams[1];s.next_home_formation=formations[teams[0]];s.next_away_formation=formations[teams[1]];
 s.next_takers={takers[teams[0]],takers[teams[1]]};
 for(int side=0;side<2;++side){const int team=teams[side];
  for(int j=0;j<20;++j){s.next_order[side][j]=squad[team][j];s.next_fitness[side][j]=players[team][j].fitness;s.next_available[side][j]=available(team,j+1);}
  for(int j=0;j<11;++j)require(available(team,squad[team][j]),"Not enough eligible starters - squad needs attention");
  require(std::string(original_teams[team].players[squad[team][0]-1].position)=="GK","Starting goalkeeper required");
 }
 s.next_squad_enabled=true;
 s.next_tournament_fixture=int(results.size());s.next_original_draw=draw==default_draw;
 s.set_match_minutes(minutes);s.next_format=results.size()<24?OriginalActions::MatchFormat::regulation:OriginalActions::MatchFormat::knockout;
}
bool OriginalTournament::collect(const OriginalActions& s){
 if(!s.full_time)return false;require(s.full_match && s.national_teams,"Expected a completed national-team match");
 const auto teams=next();const bool reverse=s.home_team==teams[1] && s.away_team==teams[0];
 require(reverse || (s.home_team==teams[0] && s.away_team==teams[1]),"Finished match has wrong teams");
 Result r{teams[0],teams[1],reverse?s.red_goals:s.blue_goals,reverse?s.blue_goals:s.red_goals};
 r.extra=s.engine.match_half>1 || s.shootout;r.pens=s.shootout;
 if(r.pens){r.hp=s.penalty_scores[reverse?1:0];r.ap=s.penalty_scores[reverse?0:1];}
 // Commit only completed fixtures. A cancellation neither heals players nor
 // serves a ban. Reports are indexed by stable country/shirt, including subs.
 const auto report=s.player_reports();record(r);
 for(int side=0;side<2;++side){const int team=side?s.away_team:s.home_team;
  for(int p=0;p<20;++p){auto& state=players[team][p];const auto& perf=report[side][p];
   // Native scheduling policy: one missed team fixture serves a ban; ten
   // fitness points recover per completed team fixture, before fresh injuries.
   if(state.ban>0)--state.ban;
   state.fitness=std::max(10,std::min(original_teams[team].players[p].fitness,state.fitness+10)-perf.injury);
   state.yellows+=perf.bookings%5;
   if(perf.bookings>=5 || state.yellows>=2){state.ban=1;state.yellows=0;}
  }
  arrange(team);
 }
 return true;
}
void OriginalTournament::save(const std::filesystem::path& path)const{
 require(active,"No tournament to save");std::ostringstream out;
 out<<"ACTUA96_TOURNAMENT "<<(scheduled_venues?4:3)<<'\n'<<user<<' '<<minutes<<' '<<venue<<' '<<camera<<' '<<results.size();
 if(scheduled_venues)out<<" 1";
 out<<'\n';
 for(const auto& r:results)out<<r.home<<' '<<r.away<<' '<<r.hg<<' '<<r.ag<<' '<<r.hp<<' '<<r.ap<<' '<<r.extra<<' '<<r.pens<<'\n';
 for(int t=0;t<16;++t){for(int shirt:squad[t])out<<shirt<<' ';out<<'\n';
  for(const auto& p:players[t])out<<p.fitness<<' '<<p.yellows<<' '<<p.ban<<'\n';}
 for(int t=0;t<16;++t)out<<formations[t]<<' '<<takers[t][0]<<' '<<takers[t][1]<<' '<<takers[t][2]<<'\n';
 const auto body=out.str();out<<checksum(body)<<'\n';
 std::filesystem::create_directories(path.parent_path());auto temporary=path;temporary+=".tmp";
 {std::ofstream file(temporary,std::ios::binary|std::ios::trunc);file<<out.str();file.flush();require(bool(file),"Could not write tournament save");}
 // Keep the old valid checkpoint recoverable; replace the live pointer atomically.
 if(std::filesystem::exists(path)){
  // A retry after corruption must not replace the last good backup with the
  // damaged primary. Validate the complete checkpoint, not just its header.
  bool valid=false;try{OriginalTournament previous;previous.load(path);valid=true;}catch(const std::exception&){}
  if(valid){auto backup=path;backup+=".bak";std::filesystem::copy_file(path,backup,std::filesystem::copy_options::overwrite_existing);}
 }
#ifdef _WIN32
 require(MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0,"Could not replace tournament save");
#else
 std::filesystem::rename(temporary,path);
#endif
}
void OriginalTournament::load(const std::filesystem::path& path){
 require(std::filesystem::file_size(path)<=32768,"Tournament save too large");std::ifstream file(path,std::ios::binary);std::string bytes((std::istreambuf_iterator<char>(file)),{});
 require(!bytes.empty() && bytes.back()=='\n',"Truncated tournament save");auto split=bytes.rfind('\n',bytes.size()-2);require(split!=std::string::npos,"Invalid tournament save");
 std::istringstream hash_in(bytes.substr(split+1));std::uint64_t stored=0;require(bool(hash_in>>stored),"Missing save checksum");hash_in>>std::ws;require(hash_in.eof() && stored==checksum(bytes.substr(0,split+1)),"Tournament save checksum mismatch");
 std::istringstream in(bytes.substr(0,split+1));std::string magic;int version,team,length,stadium,view,count;
 require(bool(in>>magic>>version>>team>>length>>stadium>>view>>count) && magic=="ACTUA96_TOURNAMENT" && (version>=1 && version<=4) && count>=0 && count<=31,"Invalid tournament save header");
 OriginalTournament candidate;candidate.reset(team,length,stadium,view);
 if(version>=4){int scheduled;require(bool(in>>scheduled) && (scheduled==0 || scheduled==1),"Invalid saved venue schedule");candidate.scheduled_venues=scheduled!=0;}
 for(int i=0;i<count;++i){Result r{};int extra,pens;require(bool(in>>r.home>>r.away>>r.hg>>r.ag>>r.hp>>r.ap>>extra>>pens) && (extra==0 || extra==1) && (pens==0 || pens==1),"Invalid saved result");r.extra=extra!=0;r.pens=pens!=0;candidate.record(r);}
 if(version>=2)for(int t=0;t<16;++t){std::array<bool,20> seen{};
  for(int& shirt:candidate.squad[t]){require(bool(in>>shirt) && shirt>=1 && shirt<=20,"Invalid saved squad");require(!seen[shirt-1],"Duplicate saved player");seen[shirt-1]=true;}
  for(auto& p:candidate.players[t])require(bool(in>>p.fitness>>p.yellows>>p.ban) && p.fitness>=10 && p.fitness<=100 && p.yellows>=0 && p.yellows<=1 && p.ban>=0 && p.ban<=1,"Invalid saved player condition");
  require(std::string(original_teams[t].players[candidate.squad[t][0]-1].position)=="GK","Invalid saved goalkeeper");
 }
 if(version>=3)for(int t=0;t<16;++t){require(bool(in>>candidate.formations[t]) && candidate.formations[t]>=0 && candidate.formations[t]<10,"Invalid saved formation");
  for(int& shirt:candidate.takers[t])require(bool(in>>shirt) && shirt>=0 && shirt<=20,"Invalid saved taker");}
 in>>std::ws;require(in.eof(),"Trailing tournament save data");*this=std::move(candidate);
}
}
