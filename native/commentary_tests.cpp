#include "original_commentary.hpp"
#include "original_graphics.hpp"
#include "action_constants.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <set>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 require(argc==3 || argc==4,"Expected source and audio roots, optional language");const int language=argc==4?std::atoi(argv[3]):0;require(SDL_Init(SDL_INIT_AUDIO),SDL_GetError());
 {
  OriginalGraphics graphics(argv[1]);auto& s=*graphics.sequence;auto& e=s.engine;s.start_match(true);s.ticks=10;
  OriginalCommentary missing(std::filesystem::path(argv[2])/"absent",false);require(!missing.loaded(),"Missing assets allow silent play");
  OriginalCommentary speech(argv[2],false,language);require(speech.loaded(),"Original phrase pack available");
  std::vector<float> pcm(11025*2);speech.update(s,0,true);
  e.prediction=1;e.PlayCommentaryMessage(22);require(!e.commentary_count,"Predictions emit no speech");e.prediction=0;
  for(int i=0;i<140;++i)e.PlayCommentaryMessage(1);require(e.commentary_count==128 && e.commentary_dropped==12,"Bounded event queue");speech.update(s,0,false);
  require(!e.commentary_count && !speech.pending(),"Inactive speech discards old requests");
  speech.update(s,0,true);speech.request(1);speech.update(s,0,true);require(speech.category()==1,"Restart phrase starts");
  speech.request(22);speech.update(s,0,true);require(speech.category()==22 && speech.stats().interrupted==1,"Goal supersedes restart");
  speech.request(43);speech.update(s,0,true);require(speech.category()==43,"Full-time supersedes goal");
  speech.mix(pcm.data(),11025);require(std::any_of(pcm.begin(),pcm.end(),[](float v){return std::abs(v)>.001f;}),"Original phrase has audible PCM");
  for(float v:pcm)require(std::isfinite(v) && std::abs(v)<=.75f,"Finite bounded commentary output");
  speech.set_volume(0);speech.mix(pcm.data(),11025);require(!speech.speaking() && std::all_of(pcm.begin(),pcm.end(),[](float v){return v==0;}),"Independent zero volume flushes/silences speech");
  speech.set_volume(100);speech.update(s,0,true);speech.request(30);speech.update(s,0,true);speech.update(s,0,false);require(!speech.speaking() && !speech.pending(),"Focus/pause discards speech");
  OriginalCommentary device(argv[2],true,language);require(device.ready(),"Dummy SDL commentary stream");device.update(s,0,true);device.request(30);device.update(s,.05,true);device.update(s,0,false);
  // Isolate speech selection's RNG check from the actual match simulation.
  std::srand(4321);const int expected_random=std::rand();std::srand(4321);
  speech.update(s,.05,true);speech.request(31);speech.update(s,.05,true);
  require(std::rand()==expected_random,"Speech selection does not consume gameplay RNG");
  // All verified categories decode and can be selected. Avoid rate limits with fresh instances.
  const bool retail_events=!language && std::filesystem::exists(std::filesystem::path(argv[2])/"COMMENTARY/EURO_EVENTS.BIN");
  for(int id=1;id<=43;++id){if(id>35 && id<42)continue;OriginalCommentary sample(argv[2],false,language);sample.update(s,0,true);sample.request(id);sample.update(s,0,true);
   const int expected=retail_events?(id==18?0:id==14?13:id):id;
   require(sample.category()==expected,"Imported category follows selected retail or compatibility route");sample.mix(pcm.data(),11025);require(std::any_of(pcm.begin(),pcm.end(),[](float v){return std::abs(v)>.001f;})==bool(expected),"Retail set-piece route is silent; other categories produce speech");}
  OriginalCommentary introduction(argv[2],false,language);introduction.update(s,0,true);
  const bool retail_sequences=!language && std::filesystem::exists(std::filesystem::path(argv[2])/"COMMENTARY/EURO_SEQUENCES.BIN");
  require(introduction.welcome(4,12)==!retail_sequences,"Retail exhibition is silent; legacy linked welcome remains");
  introduction.mix(pcm.data(),11025);require(std::any_of(pcm.begin(),pcm.end(),[](float v){return std::abs(v)>.001f;})==!retail_sequences,"Welcome PCM obeys retail context gate");
  introduction.update(s,0,false);introduction.update(s,0,true);
  const bool retail=!language && std::filesystem::exists(std::filesystem::path(argv[2])/"COMMENTARY/EURO_ENGLISH.BIN");
  require(introduction.welcome(1,12)==(retail && !retail_sequences) && introduction.welcome(4,15)==(retail && !retail_sequences),"Country fragments do not bypass retail welcome gate");
  // Exercise the actual crossing latch, including original branch precedence and asymmetry.
  struct GoalCase {bool right;int touch,header,rebound,wall;float curve,power;int expected;};
  const GoalCase goals[]={
   {true,2,0,0,0,0,18,22},{true,2,0,0,0,0,18.1f,24},
   {false,13,0,0,0,0,16,22},{false,13,0,0,0,0,16.1f,24},
   {true,2,1,1,1,.1f,25,25},{false,13,1,1,1,.1f,25,25},
   {true,2,0,1,1,.1f,25,27},{false,13,0,1,1,.1f,25,27},
   {true,2,0,0,0,.039f,25,28},{false,13,0,0,0,.039f,25,28},
   {true,2,0,0,1,.039f,25,29},{false,13,0,0,1,.039f,25,28},
   {true,2,0,0,1,.037f,10,22},{false,13,0,0,1,.037f,10,22},
   {true,12,1,1,1,.1f,25,26},{false,1,1,1,1,.1f,25,26},
   {true,13,1,1,1,.1f,25,23},{false,2,1,1,1,.1f,25,23},
   {true,0,0,0,0,0,0,22},{false,23,0,0,0,0,0,22}};
  for(const auto& g:goals){
   e.commentary_count=0;e.ball_in_goal=0;e.match_mode=0;e.prediction=0;e.last_touch=g.touch;
   e.headed_ball=char(g.header);e.rebound_ball=char(g.rebound);e.wall_is_up=char(g.wall);e.max_sw_factor=g.curve;e.shot_speed=g.power;e.shot_pending=1;
   if(g.right)e.go_right_goal();else e.go_left_goal();
   require(e.commentary_count==1 && e.commentary_queue[0]==g.expected,"Source goal classification at crossing");
   require(e.ball_in_goal && !e.shot_pending,"Goal latch still resets shot state");
   if(g.right)e.go_right_goal();else e.go_left_goal();
   require(e.commentary_count==1,"Repeated collision does not repeat goal commentary");
  }
  e.commentary_count=0;e.prediction=1;e.ball_in_goal=0;e.go_right_goal();require(!e.commentary_count,"Predicted goal is silent");
  e.prediction=0;e.match_mode=1;e.ball_in_goal=0;e.go_left_goal();require(!e.commentary_count,"Dead-ball crossing is silent");
  s.start_match(true);s.ticks=10;
  // Every exact score, both orderings when allowed, uses its one original WAV.
  int score_checks=0;
  for(int home=0;home<=7;++home)for(int away=0;away<=5;++away){
   OriginalCommentary sample(argv[2],false,language);sample.update(s,0,true);
   e.PlayScoreSample(home,away);sample.update(s,0,true);
   require(sample.category()==44 && sample.history().back().number==std::max(home,away)*10+std::min(home,away),"Exact original score recording, larger digit first");
   sample.mix(pcm.data(),11025);require(std::any_of(pcm.begin(),pcm.end(),[](float v){return std::abs(v)>.001f;}),"Score has audible original PCM");++score_checks;
  }
  OriginalCommentary scores(argv[2],false,language);scores.update(s,0,true);
  scores.score(8,0);scores.score(0,6);scores.score(-1,0);scores.request(44,67);scores.update(s,0,true);require(!scores.speaking(),"Unsupported score stays silent instead of speaking a different score");
  e.PlayScoreSample(8,0);e.PlayScoreSample(0,6);e.PlayScoreSample(-1,0);require(!e.commentary_count,"Engine preserves original score limits");
  e.prediction=1;e.PlayScoreSample(2,1);e.prediction=0;require(!e.commentary_count,"Predicted scores stay silent");
  scores.request(22);scores.update(s,0,true);scores.score(2,1);scores.update(s,0,true);
  require(scores.category()==22 && scores.pending()==1,"Score waits for goal reaction");
  scores.request(43);scores.update(s,0,true);require(scores.category()==43 && !scores.pending(),"Full-time clears pending score");
  scores.update(s,0,false);scores.update(s,0,true);scores.score(3,2);scores.update(s,0,false);scores.update(s,.25,true);require(!scores.speaking() && !scores.pending(),"Pause discards stale score");
  // One real scheduler transition from celebration to restart, without waiting for a random goal.
  s.start_match(true);e.match_mode=0;e.ball_poss=1;e.just_scored=1;s.restart_ticks=1;
  s.restart_mode=original::action_constants::CENTRE_B;e.team_a_goals=2;e.team_b_goals=1;e.commentary_count=0;
  s.step();require(!e.just_scored && std::count(e.commentary_queue.begin(),e.commentary_queue.begin()+e.commentary_count,277)==1,"Celebration expiry queues 2:1 exactly once");
  e.commentary_count=0;s.step();require(std::find(e.commentary_queue.begin(),e.commentary_queue.begin()+e.commentary_count,277)==e.commentary_queue.begin()+e.commentary_count,"Restart does not repeat score");
  e.commentary_count=0;e.just_scored=1;e.golden_goal=2;s.step_celebration();require(!e.commentary_count,"Golden goal leaves speech to final result");
  e.just_scored=1;e.golden_goal=0;s.shootout=true;s.step_celebration();require(!e.commentary_count,"Shootout does not announce match score");
  s.start_match(true);s.ticks=10;OriginalCommentary repeat_score(argv[2],false,language);repeat_score.update(s,0,true);
  repeat_score.score(2,1);repeat_score.update(s,0,true);require(repeat_score.history().back().number==21,"First exact score");
  s.start_match(true);s.ticks=1;repeat_score.update(s,0,false);s.ticks=10;repeat_score.update(s,0,true);
  repeat_score.score(1,2);repeat_score.update(s,0,true);
  require(repeat_score.category()==44 && repeat_score.history().size()==2 && repeat_score.history().back().number==21,"Repeated score after match reset never substitutes another recording");
  std::cout<<"Goal classification cases "<<std::size(goals)<<" exact home/away score cases "<<score_checks<<'\n';
  // Full original CPU match, draining speech at real simulation time without a device.
  s.start_match(true);OriginalCommentary live(argv[2],false,language);live.update(s,0,true);
  for(int tick=0;tick<20000 && s.running;++tick){
   s.step();const auto seed=e.seed;const auto x=e.ballx,y=e.bally,z=e.ballz;
   live.update(s,.05,true);live.mix(pcm.data(),551);
   require(e.seed==seed && e.ballx==x && e.bally==y && e.ballz==z,"Speech does not consume gameplay RNG or alter ball state");
  }
  require(s.full_time,"CPU commentary match finishes");require(live.stats().played[36]==(retail_sequences?0:1),"Retail exhibition suppresses welcome; legacy opening occurs once");require(live.stats().played[42]==1 && live.stats().played[43]==1,"Period announcements occur exactly once");
  require(e.commentary_dropped==0,"No engine speech event loss during match");
  require(live.stats().played[44]==unsigned(s.blue_goals+s.red_goals),"Live goals announce score after celebration");
  require(live.history().size()>4,"Live events produce commentary");
  std::set<std::pair<int,int>> unique;
  for(const auto& h:live.history()){
   require(unique.emplace(h.category,h.number).second || h.category>=42,"No repeated phrase in this short match");
   std::cout<<"Phrase "<<h.category<<'/'<<h.number<<" at "<<h.time<<" seconds\n";
  }
  std::cout<<"CPU commentary match "<<s.blue_goals<<':'<<s.red_goals<<" ticks "<<s.ticks<<" phrases "<<live.history().size()<<" dropped stale/duplicate requests "<<live.stats().dropped<<" priority interruptions "<<live.stats().interrupted<<'\n';
 }
 SDL_Quit();std::cout<<"Original commentary contracts passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';SDL_Quit();return 1;}}
