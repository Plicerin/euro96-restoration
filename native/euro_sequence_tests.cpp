#include "original_commentary.hpp"
#include "original_graphics.hpp"
#include "original_tournament.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
using namespace actua;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
static unsigned read(const Bytes& b,std::size_t p,int n){unsigned v=0;for(int i=0;i<n;++i)v|=unsigned(b.at(p+i))<<(8*i);return v;}
static void write(const std::filesystem::path& p,const Bytes& b){std::filesystem::create_directories(p.parent_path());std::ofstream f(p,std::ios::binary);f.write(reinterpret_cast<const char*>(b.data()),b.size());require(bool(f),"Fixture write");}
int main(int argc,char** argv){try{
 require(argc==4,"Source, media and scratch required");require(SDL_Init(SDL_INIT_AUDIO),SDL_GetError());
 {
 const std::filesystem::path media=argv[2],scratch=argv[3];
 std::map<std::pair<int,int>,Bytes> pcm;
 const auto unpack=[&](const Bytes& b,std::size_t at){for(unsigned i=0;i<read(b,8,4);++i){const int c=read(b,at,2),n=read(b,at+2,2);const auto size=read(b,at+4,4);at+=8;
  SDL_AudioSpec spec{};Uint8* sample=nullptr;Uint32 count=0;require(SDL_LoadWAV_IO(SDL_IOFromConstMem(b.data()+at,size),true,&spec,&sample,&count),SDL_GetError());pcm[{c,n}]={sample,sample+count};SDL_free(sample);at+=size;
 }require(at==b.size(),"Complete WAV fixture");};
 const auto pack=read_file(media/"COMMENTARY/EURO_SEQUENCES.BIN");unpack(pack,105);unpack(read_file(media/"COMMENTARY/EURO_EVENTS.BIN"),12);
 const auto compare=[&](OriginalCommentary& voice,std::initializer_list<std::pair<int,int>> clips){Bytes expected;
  for(auto key:clips){const auto& sample=pcm.at(key);expected.insert(expected.end(),sample.begin(),sample.end());}
  std::vector<float> out((expected.size()+8)*2);voice.mix(out.data(),int(expected.size()+8));
  for(std::size_t i=0;i<expected.size();++i){const float ramp=std::min({1.0f,float(i)/55,float(expected.size()-i)/55});const float value=(float(expected[i])-128)/128*ramp;
   require(std::abs(out[2*i]-value)<.000001f && out[2*i]==out[2*i+1],"Original PCM and fragment order across every boundary");}
  require(!voice.speaking() && out.back()==0,"Sequence ends exactly without looping/appended compatibility phrase");
 };
 OriginalGraphics g(argv[1]);auto& s=*g.sequence;auto& e=s.engine;s.start_match(true);s.ticks=10;
 OriginalCommentary voice(media,false);voice.set_volume(100);voice.update(s,0,true);
 const int numbers[]={1,2,3,4,5,6,7,8,10,9,12,11,13,14,15,16,18,20,19,17,22,24,21,23};
 OriginalTournament tournament;tournament.reset(-1);unsigned openings=0,winners=0;
 for(int fixture=0;fixture<24;++fixture){const auto teams=tournament.fixture(fixture);
  for(int reverse=0;reverse<2;++reverse){const int home=teams[reverse],away=teams[1-reverse];
   require(voice.welcome(home,away,fixture,true) && voice.history().back().bank==99 && voice.history().back().number==numbers[fixture],"Correct fixture lookup including native home/away swap");
   if(numbers[fixture]==20)compare(voice,{{64+home,1},{80+away,1}});else compare(voice,{{99,numbers[fixture]}});++openings;
  }
 }
 for(int fixture=28;fixture<=30;++fixture)for(int home=0;home<16;++home){const int away=(home+7)%16;
  require(voice.welcome(home,away,fixture,true),"Semi/final linked introduction");compare(voice,{{99,fixture+1},{100,home+1},{80+away,1}});++openings;
 }
 // Every possible qualifier pairing, on both native sides. Quarter-final
 // recordings already contain the full opening: country fragments must not follow.
 const int opposing_group[]={1,0,3,2};
 const auto country=[&](int slot){for(int t=0;t<16;++t)if(tournament.draw[t]==slot)return t;throw std::runtime_error("Missing group slot");};
 for(int q=0;q<4;++q)for(int a=0;a<4;++a)for(int b=0;b<4;++b)for(int reverse=0;reverse<2;++reverse){
  const int first=country(4*q+a),second=country(4*opposing_group[q]+b);
  const int home=reverse?second:first,away=reverse?first:second;
  require(voice.welcome(home,away,24+q,true) && voice.history().back().bank==99 && voice.history().back().number==25+q,"Quarter-final fixture selects authored recording for either side");
  compare(voice,{{99,25+q}});++openings;
  require(!voice.welcome(home,away,24+q,false) && !voice.speaking(),"Retail custom-draw welcome exits without compatibility speech");
 }
 for(int fixture:{-1,31})require(!voice.welcome(1,4,fixture,true),"Invalid retail fixture cannot invent a welcome");
 require(voice.welcome(1,4,0,true) && voice.history().back().bank==36,"Mismatched original-draw fixture retains compatibility fallback");
 voice.update(s,0,false);voice.update(s,0,true);
 require(!voice.welcome(4,14,0,false) && !voice.speaking(),"Custom draw never announces fixed retail teams");
 for(int reverse=0;reverse<2;++reverse){OriginalTournament bracket;bracket.reset(-1);
  for(int i=0;i<24;++i){const auto f=bracket.next();bracket.record({f[0],f[1],reverse?0:2,reverse?2:0});}
  for(int q=0;q<4;++q){const auto f=bracket.next();bracket.user=f[reverse];
   std::filesystem::create_directories(scratch);bracket.save(scratch/"quarter.sav");OriginalTournament resumed;resumed.load(scratch/"quarter.sav");
   require(resumed.next()==f,"Saved quarter-final retains actual qualifiers");resumed.prepare(s);s.start_match(false);
   require(s.tournament_fixture==24+q && s.original_draw && s.home_team==f[reverse],"Resumed quarter-final supplies context and human team");
   OriginalCommentary quarter(media,false);quarter.set_volume(100);quarter.update(s,0,true);require(quarter.history().back().bank==99,"Resumed quarter-final kickoff uses retail speech");
   compare(quarter,{{99,25+q}});quarter.update(s,0,true);require(quarter.stats().played[36]==1,"Quarter-final opening plays only once");
   bracket.record({f[0],f[1],2,1});
  }
 }
 // Presentation context must survive tournament preparation, be consumed once,
 // and never leak into the next exhibition or persist in the checkpoint format.
 tournament.reset(14);tournament.prepare(s);require(s.next_tournament_fixture==0 && s.next_original_draw,"Preparation supplies original fixture context");
 s.start_match(false);require(s.tournament_fixture==0 && s.home_team==14 && s.away_team==4 && s.next_tournament_fixture==-1,"Launch consumes context with human country reversal");
 OriginalCommentary live(media,false);live.set_volume(100);live.update(s,0,true);require(live.history().back().bank==99,"Live kickoff selects tournament introduction");compare(live,{{99,1}});
 live.update(s,0,true);require(live.stats().played[36]==1,"Kickoff introduction only once");
 s.start_match(true);live.update(s,0,true);require(s.tournament_fixture==-1 && !s.original_draw && !live.speaking() && live.stats().played[36]==1,"Subsequent exhibition does not inherit or append a tournament introduction");
 std::swap(tournament.draw[0],tournament.draw[1]);tournament.prepare(s);s.start_match(true);require(!s.original_draw,"Custom draw marked explicitly");
 tournament.prepare(s);s.start_play();require(s.next_tournament_fixture==-1 && s.tournament_fixture==-1,"Practice/reset clears abandoned tournament preparation");
 // Every country on either native side, including tied match scores resolved
 // on penalties. Championship precedence must win over extra-time/shootout.
 for(int team=0;team<16;++team)for(int side=0;side<2;++side)for(int result=0;result<5;++result){
  s.next_home=side?(team+1)%16:team;s.next_away=side?team:(team+1)%16;s.start_match(true);s.ticks=10;
  s.blue_goals=side?1:2;s.red_goals=side?2:1;
  s.tournament_fixture=result<3?30:28;e.match_half=result==0?1:3;
  s.shootout=result==2 || result==4;if(s.shootout){s.blue_goals=s.red_goals=1;s.penalty_scores={side?3:4,side?4:3};}
  const int bank=result<3?101:result==3?103:102;
  OriginalCommentary sample(media,false);sample.set_volume(100);sample.update(s,0,true);const auto seed=e.seed;
  s.finish_match();sample.update(s,0,true);require(sample.category()==43 && sample.history().back().bank==bank && sample.history().back().number==team+1,"Correct winner and final/penalty/extra-time precedence");
  compare(sample,{{bank,team+1}});require(e.seed==seed,"Winner speech leaves gameplay RNG intact");++winners;
 }
 // Golden-goal result, decisive regulation knockout result, incomplete/tied
 // result, muted and paused winner announcements.
 s.start_match(true);s.ticks=10;s.blue_goals=3;s.red_goals=2;e.match_half=2;e.golden_goal=2;
 OriginalCommentary golden(media,false);golden.update(s,0,true);s.finish_match();golden.update(s,0,true);require(golden.history().back().bank==103,"Golden goal uses extra-time winner speech");
 s.start_match(true);s.ticks=10;s.blue_goals=3;s.red_goals=2;s.match_format=OriginalActions::MatchFormat::knockout;e.match_half=1;
 OriginalCommentary regulation(media,false);regulation.update(s,0,true);s.finish_match();regulation.update(s,0,true);require(regulation.history().back().bank==97 && regulation.history().back().number==32,"Decisive regulation knockout uses exact FLL score");
 s.start_match(true);s.ticks=10;s.shootout=true;s.penalty_scores={4,4};s.tournament_fixture=30;
 OriginalCommentary tied(media,false);tied.update(s,0,true);s.finish_match();tied.update(s,0,true);require(tied.history().back().bank==43,"Tied penalty scores cannot name a champion");
 tied.set_volume(0);e.PlayCommentaryMessage(43);tied.update(s,0,true);require(!tied.speaking() && !e.commentary_count,"Muted ending discarded");
 golden.update(s,0,false);golden.update(s,0,true);require(!golden.speaking(),"Paused ending not replayed");
 s.start_match(true);s.ticks=10;s.tournament_fixture=30;s.blue_goals=2;s.red_goals=1;
 OriginalCommentary french(media,false,1);french.update(s,0,true);s.finish_match();french.update(s,0,true);require(french.history().back().bank==43,"French keeps its own result voice");
 require(french.set_language(0),"Switch to English");french.update(s,0,true);e.PlayCommentaryMessage(43);french.update(s,0,true);require(french.history().back().bank==101,"Language switch restores winner routes");
 require(french.set_language(1),"Switch back to French");french.update(s,0,true);for(int fixture:{0,24,25,26,27})require(french.welcome(4,14,fixture,true) && french.history().back().bank==36,"French does not retain English fixture metadata");
 const auto legacy=scratch/"legacy";write(legacy/"COMMENTARY/ENGLISH3.BIN",read_file(media/"COMMENTARY/ENGLISH3.BIN"));
 OriginalCommentary old(legacy,false);old.update(s,0,true);require(old.welcome(4,14,0,true) && old.history().back().bank==36,"Absent supplement leaves old introductions usable");e.PlayCommentaryMessage(43);old.update(s,0,true);require(old.history().back().bank==43,"Absent supplement leaves old result call usable");
 const auto bad=scratch/"bad";write(bad/"COMMENTARY/ENGLISH3.BIN",read_file(media/"COMMENTARY/ENGLISH3.BIN"));
 for(int mode=0;mode<9;++mode){auto b=pack;
  if(mode==0)b.pop_back();if(mode==1)b[7]='2';if(mode==2)b[12]=32;if(mode==3)b[15]=b[12];if(mode==4)b[13]=16;if(mode==5)b[85]=0;
  if(mode==6)b[105]=104;if(mode==7)b[107]=32;
  if(mode==8){const auto size=8+read(b,109,4);Bytes record(b.begin()+105,b.begin()+105+size);b.insert(b.end(),record.begin(),record.end());const auto n=read(b,8,4)+1;for(int i=0;i<4;++i)b[8+i]=(n>>(8*i))&255;}
  write(bad/"COMMENTARY/EURO_SEQUENCES.BIN",b);bool rejected=false;try{OriginalCommentary invalid(bad,false);}catch(const std::exception&){rejected=true;}require(rejected,"Malformed sequence media/fixture metadata rejected");
 }
 std::cout<<"Retail sequences: "<<openings<<" exact introductions, "<<winners<<" country/side/result cases, knockout routing, tournament context, legacy/French, mute/pause and malformed media passed.\n";
 }
 SDL_Quit();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';SDL_Quit();return 1;}}
