#include "original_commentary.hpp"
#include "original_graphics.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static unsigned read(const Bytes& b,std::size_t p,int n){unsigned v=0;for(int i=0;i<n;++i)v|=unsigned(b.at(p+i))<<(8*i);return v;}
static void write(const std::filesystem::path& path,const Bytes& b){std::filesystem::create_directories(path.parent_path());std::ofstream f(path,std::ios::binary);f.write(reinterpret_cast<const char*>(b.data()),b.size());require(bool(f),"Fixture write");}
int main(int argc,char** argv){try{
 require(argc==4,"Source, media and scratch required");require(SDL_Init(SDL_INIT_AUDIO),SDL_GetError());
 {
 const std::filesystem::path media=argv[2],scratch=argv[3];
 const auto pack=read_file(media/"COMMENTARY/EURO_PERIODS.BIN");
 std::map<std::pair<int,int>,Bytes> expected;std::size_t at=12,end_offset=0;
 for(unsigned i=0;i<read(pack,8,4);++i){const int c=read(pack,at,2),n=read(pack,at+2,2);const auto size=read(pack,at+4,4);if(c==98 && !end_offset)end_offset=at;at+=8;
  SDL_AudioSpec spec{};Uint8* pcm=nullptr;Uint32 count=0;require(SDL_LoadWAV_IO(SDL_IOFromConstMem(pack.data()+at,size),true,&spec,&pcm,&count),SDL_GetError());
  expected[{c,n}]={pcm,pcm+count};SDL_free(pcm);at+=size;
 }
 require(expected.size()==282 && at==pack.size() && end_offset,"Complete independently verified period fixture");
 OriginalGraphics g(argv[1]);auto& s=*g.sequence;auto& e=s.engine;s.start_match(true);s.ticks=10;
 const auto compare=[&](OriginalCommentary& voice,int bank){
  const auto& h=voice.history().back();require(h.bank==bank,"Correct retail/compatibility bank");
  const auto& pcm=expected.at({bank,h.number+((bank==96 || bank==97)?100*h.variant:0)});
  // Compare the entire phrase, including fade edges and the exact end.
  std::vector<float> mixed((pcm.size()+10)*2);voice.mix(mixed.data(),int(pcm.size()+10));
  for(std::size_t i=0;i<pcm.size();++i){const auto ramp=std::min({1.0f,float(i)/55,float(pcm.size()-i)/55});const auto value=(float(pcm[i])-128)/128*ramp;
   require(std::abs(mixed[2*i]-value)<.000001f && mixed[2*i]==mixed[2*i+1],"Exact original period PCM including clip boundary");}
  require(!voice.speaking() && mixed.back()==0,"Period finishes without looping or appended speech");
 };
 const auto advance=[&](OriginalCommentary& voice){for(int i=0;i<12;++i)voice.update(s,.25,true);};
 // The real rule scheduler, not a synthetic queue event, must retain both
 // whistles, wait for a crucial shot, and announce each transition once.
 for(int id:{42,43}){
  s.start_match(true);s.ticks=10;s.blue_goals=2;s.red_goals=0;
  e.match_half=id==42?0:1;e.match_time.min=id==42?45:90;e.injury_time=0;
  e.match_mode=e.just_scored=e.dead_ball_cnt=e.game_action=0;s.restart_ticks=0;
  OriginalCommentary boundary(media,false);boundary.update(s,0,true);e.shot_pending=1;
  require(!e.regulation_period_end() && !e.commentary_count,"Period speech waits for original crucial-shot gate");e.shot_pending=0;
  s.step();boundary.update(s,0,true);
  require(boundary.category()==id && boundary.history().back().bank==(id==42?96:97) && boundary.history().back().number==20,"Actual whistle transition selects exact retail score");
  boundary.update(s,.05,true);require(boundary.stats().played[id]==1,"Boundary consumed once");
  require(id==42?s.interval_ticks>0:s.full_time,"Audio leaves interval/result transition intact");
 }
 s.start_match(true);s.ticks=10;
 unsigned cases=0;
 // Exercise the actual regulation scheduler for both knockout rules and every
 // score-specific XTM phrase, then the retail high-score generic range.
 for(auto format:{OriginalActions::MatchFormat::knockout,OriginalActions::MatchFormat::golden})for(int score=0;score<=9;++score){
  s.next_format=format;s.start_match(true);s.ticks=10;
  s.blue_goals=s.red_goals=e.team_a_goals=e.team_b_goals=score;
  e.match_half=1;e.match_time.min=90;e.match_time.sec=0;e.injury_time=0;
  e.match_mode=e.just_scored=e.dead_ball_cnt=e.game_action=e.shot_pending=0;s.restart_ticks=0;
  OriginalCommentary transition(media,false);transition.set_volume(100);transition.update(s,0,true);
  s.step();const auto seed=e.seed;transition.update(s,0,true);
  require(s.interval_ticks && s.interval_minute==90 && !s.full_time && !s.shootout,"XTM accompanies extra time, never ends the game");
  require(!transition.history().empty() && transition.history().back().bank==104,"Real 90-minute tie chooses XTM");
  const int clip=transition.history().back().number;
  require(score<=8?clip==score+1:clip>=9 && clip<=32,"Original tied-score filename selection");
  compare(transition,104);transition.update(s,.05,true);
  require(transition.stats().played[43]==1 && e.seed==seed,"Extra-time call consumed once without gameplay randomness");++cases;
  if(score==9){std::set<int> takes,available;for(const auto& [key,pcm]:expected)if(key.first==104 && key.second>=9)available.insert(key.second);
   for(int repeat=0;repeat<256 && takes!=available;++repeat){advance(transition);e.PlayCommentaryMessage(43);transition.update(s,0,true);
    const auto& h=transition.history().back();require(h.bank==104 && h.number>=9 && h.number<=32,"High draw uses only original generic range");takes.insert(h.number);compare(transition,104);++cases;}
   require(takes==available && e.seed==seed,"Every available XTM generic take exercised without gameplay RNG");}
  if(score==0){OriginalCommentary french_transition(media,false,1);french_transition.update(s,0,true);e.PlayCommentaryMessage(43);french_transition.update(s,0,true);
   require(french_transition.history().empty(),"Missing French XTM retains whistle-only transition, not a false final result");}
 }
 s.next_format=OriginalActions::MatchFormat::regulation;s.start_match(true);s.ticks=10;
 // Both team orders, every legal score, all available takes before repeats.
 for(int id:{42,43})for(int home=0;home<=7;++home)for(int away=0;away<=7;++away){
  const int high=std::max(home,away),low=std::min(home,away),score=10*high+low;
  if(low>5 || (id==43 && home==away))continue;
  s.blue_goals=home;s.red_goals=away;e.match_half=id==42?0:1;
  OriginalCommentary voice(media,false);voice.set_volume(100);voice.update(s,0,true);
  const int bank=id==42?96:97;std::set<int> variants,observed;
  for(const auto& [key,pcm]:expected)if(key.first==bank && key.second%100==score)variants.insert(key.second/100);
  if(variants.empty()){
   e.PlayCommentaryMessage(id);voice.update(s,0,true);
   require(voice.category()==id && voice.history().back().bank==id,"Missing half-time score uses generic compatibility call");++cases;continue;
  }
  for(std::size_t repeat=0;repeat<=variants.size();++repeat){const auto seed=e.seed;e.PlayCommentaryMessage(id);voice.update(s,0,true);
   require(voice.category()==id && voice.history().back().number==score,"Event retains public identity and correct sorted score");
   if(repeat<variants.size())require(observed.insert(voice.history().back().variant).second,"All available period takes before repetition");
   compare(voice,bank);advance(voice);require(seed==e.seed,"Period audio leaves gameplay RNG intact");++cases;
  }
  require(variants==observed,"Every imported take exercised");
 }
 // Retail ordinary drawn endings use END, even for scores beyond score-WAV bounds.
 for(int draw:{0,1,5,8}){s.blue_goals=s.red_goals=draw;OriginalCommentary voice(media,false);voice.set_volume(100);voice.update(s,0,true);std::set<int> endings;
  for(int i=0;i<14;++i){e.PlayCommentaryMessage(43);voice.update(s,0,true);require(voice.category()==43,"Draw retains full-time priority and stats");
   if(i<13)require(endings.insert(voice.history().back().number).second,"Thirteen retail drawn endings before repetition");compare(voice,98);advance(voice);++cases;}
 }
 // Out-of-range scores and unverified knockout contexts never play an unrelated score.
 for(int mode=0;mode<7;++mode){s.start_match(true);s.ticks=10;s.blue_goals=2;s.red_goals=1;e.match_half=1;
  if(mode==0)s.blue_goals=8;if(mode==1)s.blue_goals=-1;
  if(mode==2)s.shootout=true;if(mode==3)e.match_half=3;if(mode==4)e.golden_goal=2;
  if(mode==5)s.match_format=OriginalActions::MatchFormat::knockout;if(mode==6)s.match_format=OriginalActions::MatchFormat::golden;
  OriginalCommentary voice(media,false);voice.update(s,0,true);e.PlayCommentaryMessage(43);voice.update(s,0,true);
  const bool sequences=std::filesystem::exists(media/"COMMENTARY/EURO_SEQUENCES.BIN");
  const int expected_bank=sequences && mode==3?103:sequences && mode>=5?97:43;
  require(voice.history().back().bank==expected_bank,"Result uses verified sequence when available, otherwise compatibility call");
 }
 s.start_match(true);s.ticks=10;s.blue_goals=2;s.red_goals=0;s.match_format=OriginalActions::MatchFormat::knockout;
 OriginalCommentary knockout_half(media,false);knockout_half.update(s,0,true);e.PlayCommentaryMessage(42);knockout_half.update(s,0,true);require(knockout_half.history().back().bank==96,"Knockout first interval still uses HLF");
 s.start_match(true);s.ticks=10;s.blue_goals=2;s.red_goals=0;
 OriginalCommentary flow(media,false);flow.update(s,0,true);flow.request(22);flow.update(s,0,true);flow.score(2,0);
 e.PlayCommentaryMessage(42);flow.update(s,0,true);require(flow.category()==42 && flow.pending()==0 && flow.stats().interrupted==1,"Half-time replaces goal reaction and obsolete score");
 flow.update(s,0,false);flow.update(s,.25,true);require(!flow.speaking() && !flow.pending(),"Paused period is discarded");
 flow.set_volume(0);e.PlayCommentaryMessage(43);flow.update(s,0,true);require(!flow.speaking() && !e.commentary_count,"Muted period is consumed silently");
 flow.set_volume(100);require(flow.set_language(1),"French language available");flow.update(s,0,true);e.PlayCommentaryMessage(42);flow.update(s,0,true);
 require(flow.category()==42 && flow.history().back().bank==42,"French keeps its own period recording");
 require(flow.set_language(0),"English restores period overlay");flow.update(s,0,true);e.PlayCommentaryMessage(42);flow.update(s,0,true);require(flow.history().back().bank==96,"Atomic switch restores retail period routing");
 // An absent supplement leaves older installations working; malformed ones
 // must fail explicitly, including in a transactional language change.
 const auto legacy=scratch/"legacy";
 write(legacy/"COMMENTARY/ENGLISH3.BIN",read_file(media/"COMMENTARY/ENGLISH3.BIN"));
 OriginalCommentary old(legacy,false);old.update(s,0,true);e.PlayCommentaryMessage(42);old.update(s,0,true);require(old.history().back().bank==42,"Missing optional overlay retains original call");
 s.interval_ticks=100;s.interval_minute=90;s.full_time=false;e.need_result=1;s.blue_goals=s.red_goals=0;
 OriginalCommentary no_xtm(legacy,false);no_xtm.update(s,0,true);e.PlayCommentaryMessage(43);no_xtm.update(s,0,true);require(no_xtm.history().empty(),"Legacy extra-time transition never substitutes a final-result clip");
 s.interval_ticks=0;s.interval_minute=45;s.blue_goals=2;s.red_goals=0;e.need_result=0;
 const auto bad=scratch/"bad";write(bad/"COMMENTARY/ENGLISH3.BIN",read_file(media/"COMMENTARY/ENGLISH3.BIN"));write(bad/"COMMENTARY/FRENCH.BIN",read_file(media/"COMMENTARY/FRENCH.BIN"));
 OriginalCommentary french(bad,false,1);french.set_volume(50);french.update(s,0,true);french.request(1);french.update(s,0,true);
 for(int mode=0;mode<8;++mode){auto b=pack;
  if(mode==0)b[0]='!';if(mode==1)b.pop_back();if(mode==2)b[12]=95;
  if(mode==3){b[14]=144;b[15]=1;} // variant E
  if(mode==4){b[14]=67;b[15]=0;} // reversed/unsupported score
  if(mode==5)b[end_offset+2]=14; // END table has thirteen clips
  if(mode==6){const auto size=8+read(b,16,4);Bytes record(b.begin()+12,b.begin()+12+size);b.insert(b.end(),record.begin(),record.end());const auto n=read(b,8,4)+1;for(int i=0;i<4;++i)b[8+i]=(n>>(8*i))&255;}
  if(mode==7)b[7]='2';
  write(bad/"COMMENTARY/EURO_PERIODS.BIN",b);bool rejected=false;try{OriginalCommentary invalid(bad,false);}catch(const std::exception&){rejected=true;}require(rejected,"Malformed period overlay rejected");
  require(!french.set_language(0) && french.language()==1 && french.volume()==50 && french.category()==1,"Failed language swap retains voice, current speech and volume");
 }
 std::cout<<"Retail periods: "<<cases<<" score/order/take cases, exact full PCM, draw endings, missing/unsupported scores, knockout/French/legacy fallback, priority, pause, mute and malformed media passed.\n";
 }
 SDL_Quit();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';SDL_Quit();return 1;}}
