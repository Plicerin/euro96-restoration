#include "original_graphics.hpp"
#include "original_actions.hpp"
#include <cstdlib>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <vector>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 require(argc==2,"Expected source directory");OriginalGraphics graphics(argv[1]);auto& s=*graphics.sequence;auto& e=s.engine;
 const auto trace=[&](){
  std::vector<float> result;
  for(int tick=0;tick<400;++tick){
   s.step();result.insert(result.end(),{e.ballx,e.bally,e.ballz,float(e.ball_poss),float(e.card_events)});
   for(const auto& p:e.teams)result.insert(result.end(),{p.tm_x,p.tm_y,float(p.tm_act),float(p.tm_book),p.tm_frm});
  }
  return result;
 };
 std::vector<float> first;
 for(unsigned int seed:{0u,1u,12345u,std::numeric_limits<unsigned int>::max()}){
  s.match_seed_override=seed;std::srand(999);s.start_match(true);const auto a=trace();
  std::srand(888);s.restart_play();require(s.match_seed==seed && trace()==a,"Explicit seed reproduces full opening, including card decisions, despite previous RNG state");
  if(seed==1)first=a;
 }
 s.match_seed_override=2;s.start_match(true);require(trace()!=first,"Different seeds produce different live match openings");
 s.match_seed_override.reset();std::set<unsigned int> seeds;std::set<std::vector<float>> openings;
 for(int match=0;match<24;++match){
  s.next_head_to_head=match%3==1;s.next_format=static_cast<OriginalActions::MatchFormat>(match%4);
  if(match%3==2)s.restart_play();else s.start_match(match%3==0);
  require(seeds.insert(s.match_seed).second,"Every fresh match/rematch receives a distinct seed");
  require(!s.match_seed_override && s.match_options==MatchOptions{} && e.ref_strictness==96 && e.EUROmatch_info.bookings,"Randomness policy survives reset and preserves original referee settings");
  openings.insert(trace());
 }
 require(openings.size()>1,"Fresh matches do not repeat one opening");
 // Keep direct-red and second-yellow rules intact at a controlled draw.
 s.match_seed_override=1;s.start_match(true);e.seed=20;e.send_off_plr(2,30);
 require(e.teams[1].tm_book==1 && e.teams[1].guy_on,"Original yellow threshold retained");
 e.send_off_plr(2,30);require(e.teams[1].tm_book==6 && !e.teams[1].guy_on,"Original second-yellow dismissal retained");
 e.send_off_plr(13,100);require(e.teams[12].tm_book==5 && !e.teams[12].guy_on,"Original direct-red threshold retained");
 std::cout<<"24 fresh starts/rematches; fixed seeds 0, 1, 12345 and UINT_MAX reproduce 400-tick openings; original card decisions retained.\n";
 return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
