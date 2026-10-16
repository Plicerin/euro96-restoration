#include "original_graphics.hpp"
#include "original_actions.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <set>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(int argc,char** argv){try{
 require(argc==2,"Expected source archive directory");
 OriginalGraphics g(argv[1]);auto& s=*g.sequence;auto& e=s.engine;
 std::set<int> countries,formations,actions,animations;int goals=0,shootouts=0,extended=0;
 for(int test=0;test<32;++test){
  s.next_home=test%16;s.next_away=(test*7+3)%16;
  if(s.next_away==s.next_home)s.next_away=(s.next_away+1)%16;
  s.next_home_formation=test%10;s.next_away_formation=(test*3+1)%10;
  countries.insert(s.next_home);countries.insert(s.next_away);
  formations.insert(s.next_home_formation);formations.insert(s.next_away_formation);
  s.next_format=static_cast<OriginalActions::MatchFormat>(test<30?test%4:0);
  const int minutes=test==31?90:test==30?10:5;s.set_match_minutes(minutes);
  s.next_options={test%4,test%5!=1,test%7!=2,test%6!=3,60+(test%5)*10,60+(test%5)*10};
  s.start_match(true);std::srand(1009+test*97);
  bool second=false,extra=false;int restartAge=0,lastRestart=0;
  const int limit=minutes*2400+40000;
  std::cout<<"Case "<<test<<": "<<s.team_label(false)<<" / "<<s.team_label(true)
           <<" formations="<<s.home_formation<<','<<s.away_formation<<" minutes="<<minutes
           <<" format="<<int(s.match_format)<<" wind="<<s.match_options.wind<<std::endl;
  for(int tick=0;tick<limit && s.running;++tick){
   s.step();g.presentation.observe(e,s.generation,s.ticks);
   second|=e.match_half>=1;extra|=e.match_half>=2;
   require(std::isfinite(e.ballx)&&std::isfinite(e.bally)&&std::isfinite(e.ballz),"Nonfinite match ball");
   require(e.ball_poss>=0 && e.ball_poss<=22,"Invalid ball owner");
   require(s.home_team==s.next_home && s.away_team==s.next_away,"Country identity changed during match");
   // A restart may immediately lead to another restart. Count one episode only.
   if(s.restart_mode && s.restarts==lastRestart)++restartAge;else restartAge=0;
   lastRestart=s.restarts;
   require(restartAge<2400,"One CPU restart stalled for two minutes");
   for(int i=0;i<22;++i){const auto& p=e.teams[i];
    require(std::isfinite(p.tm_x)&&std::isfinite(p.tm_y)&&std::isfinite(p.tm_z),"Nonfinite player");
    require(p.tm_player==i+1 && p.shirt>=1 && p.shirt<=20,"Invalid player slot/shirt after end change");
    actions.insert(p.tm_act);animations.insert(p.tm_anim);
    // Sample every tick; checks interpolation and every encountered animation, without rendering.
    const auto sample=g.presentation.player(e,i,.5f);
    for(float v:sample.pose)require(std::isfinite(v),"Nonfinite animation pose");
   }
  }
  require(s.full_time && !s.running,"CPU fixture did not complete within bounded time");
  require(second || s.match_format==OriginalActions::MatchFormat::penalties,"Regulation second half was skipped");
  if(s.match_format!=OriginalActions::MatchFormat::regulation)
   require(s.blue_goals!=s.red_goals || s.penalty_scores[0]!=s.penalty_scores[1],"Deciding format has no winner");
  if(!s.match_options.fouls)require(!e.foul_events && !e.offside_events,"Fouls-off calls present");
  if(!s.match_options.offside)require(!e.offside_events,"Disabled offside called");
  if(!s.match_options.bookings)require(!e.card_events,"Disabled bookings called");
  shootouts+=s.shootout;extended+=extra;goals+=s.blue_goals+s.red_goals;
  std::cout<<"  "<<s.result_label()<<" ticks="<<s.ticks<<" restarts="<<s.restarts<<std::endl;
 }
 require(countries.size()==16 && formations.size()==10,"Matrix omitted country or formation");
 std::cout<<"Release matrix: 32 completed fixtures, "<<countries.size()<<" countries, "<<formations.size()
          <<" formations, "<<goals<<" match goals, "<<shootouts<<" shootouts, "<<extended<<" extended matches.\nObserved action IDs:";
 for(int value:actions)std::cout<<' '<<value;
 std::cout<<"\nObserved animation IDs:";for(int value:animations)std::cout<<' '<<value;
 std::cout<<"\nThis matrix is sampled coverage, not exhaustive original-game parity.\n";
 return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
