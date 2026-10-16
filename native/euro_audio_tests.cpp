#include "original_audio.hpp"
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
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
static unsigned read(const Bytes& b,std::size_t p,int n){unsigned v=0;for(int i=0;i<n;++i)v|=unsigned(b.at(p+i))<<(8*i);return v;}
static void write(const std::filesystem::path& path,const Bytes& b){std::filesystem::create_directories(path.parent_path());std::ofstream f(path,std::ios::binary);f.write(reinterpret_cast<const char*>(b.data()),b.size());require(bool(f),"Write fixture");}
int main(int argc,char** argv){try{
 require(argc==4,"Source, media, scratch required");require(SDL_Init(SDL_INIT_AUDIO),SDL_GetError());
 const std::filesystem::path media=argv[2],scratch=argv[3];
 {
 OriginalGraphics g(argv[1]);auto& s=*g.sequence;auto& e=s.engine;s.start_match(true);s.ticks=10;
 const auto pack=read_file(media/"COMMENTARY/EURO_EVENTS.BIN");
 std::map<std::pair<int,int>,Bytes> expected;std::map<int,int> last_number;std::size_t at=12,score_offset=0;
 for(unsigned i=0;i<read(pack,8,4);++i){int c=read(pack,at,2),n=read(pack,at+2,2);const auto size=read(pack,at+4,4);if(c==44 && !score_offset)score_offset=at;
  at+=8;SDL_AudioSpec spec{};Uint8* pcm=nullptr;Uint32 count=0;
  require(SDL_LoadWAV_IO(SDL_IOFromConstMem(pack.data()+at,size),true,&spec,&pcm,&count),SDL_GetError());expected[{c,n}]={pcm,pcm+count};SDL_free(pcm);at+=size;
  if(c<=35)last_number[c]=std::max(last_number[c],n);
 }
 require(at==pack.size() && expected.size()==927 && score_offset,"Complete verified fixture");
 const auto compare=[&](OriginalCommentary& voice,int c,int n,bool complete=false){
  const auto& bytes=expected.at({c,n});const int frames=complete?int(bytes.size()):std::min(5000,int(bytes.size()));std::vector<float> mixed(frames*2);voice.mix(mixed.data(),frames);
  for(int i=55;i<frames && i+55<int(bytes.size());++i){const auto value=(float(bytes[i])-128)/128;
   require(std::abs(mixed[2*i]-value)<.00001f && mixed[2*i]==mixed[2*i+1],"Live mixer emits selected original PCM");}
 };
 // Source events follow the selected retail audio policy; gameplay IDs stay intact.
 for(auto [c,n]:last_number){OriginalCommentary voice(media,false);voice.set_volume(100);voice.update(s,0,true);voice.request(c,n);voice.update(s,0,true);
  if(c==18){require(!voice.speaking() && !voice.pending(),"Retail set-piece reaction remains silent");continue;}
  const int routed=c==14?13:c;
  require(voice.category()==routed && voice.history().back().number==n,"Source category follows verified retail route");compare(voice,routed,n);}
 // Every CPA take is available through the short-pass event, with shared
 // cooldown so alternating short/long source IDs cannot duplicate reactions.
 for(int n=1;n<=17;++n){OriginalCommentary voice(media,false);voice.set_volume(100);voice.update(s,0,true);voice.request(14,n);voice.update(s,0,true);
  require(voice.category()==13 && voice.history().back().number==n,"Short pass selects original retail CPA take");compare(voice,13,n,true);}
 for(int first:{13,14}){OriginalCommentary voice(media,false);voice.update(s,0,true);voice.request(first,1);voice.request(first==13?14:13,2);voice.update(s,0,true);
  require(voice.category()==13 && !voice.pending() && voice.history().size()==1,"Short and long passes share pending-event identity");
  std::vector<float> drain(110250*2);voice.mix(drain.data(),110250);voice.request(first==13?14:13,2);voice.update(s,0,true);
  require(!voice.speaking() && !voice.pending(),"Short and long passes share cooldown");}
 // Exercise the original reception routine for both teams, both distances,
 // and both set-piece branches. Speech policy must not alter possession state.
 for(int receiver:{3,14})for(int kind:{0,1,2})for(bool distant:{false,true}){
  s.start_match(true);s.ticks=10;OriginalCommentary voice(media,false);voice.set_volume(100);voice.update(s,0,true);
  const int passer=receiver-1;auto& p=e.teams[receiver-1];auto& previous=e.teams[passer-1];
  p.tm_x=600;p.tm_y=400;previous.tm_x=distant?100:590;previous.tm_y=400;
  e.prediction=0;e.prev_touch=passer;e.comm_pass=0;e.comm_pass_type=char(kind);e.comsetp=kind?1:0;e.seed=0;e.commentary_count=0;
  e.has_ball(&p);const int original_event=kind?18:distant?13:14;
  require(e.commentary_count==1 && e.commentary_queue[0]==original_event,"Original reception emits its unchanged source event");
  require(e.comm_pass==-1 && !e.comsetp,"Original reception clears pass and set-piece flags");
  const auto seed=e.seed;const auto x=e.ballx,y=e.bally;const auto possession=e.ball_poss;voice.update(s,0,true);
  require(e.seed==seed && e.ballx==x && e.bally==y && e.ball_poss==possession,"Audio routing preserves gameplay state and RNG");
  if(kind)require(!voice.speaking() && !voice.pending() && voice.history().empty(),"Both retail set-piece branches stay silent");
  else{require(voice.category()==13,"Both reception distances use CPA");compare(voice,13,voice.history().back().number,true);}
 }
 s.start_match(true);s.ticks=10;
 // The policy must change atomically with English/French, including when a
 // forbidden switch fails; do not leave retail muting enabled in French.
 OriginalCommentary language(media,false);language.update(s,0,true);
 for(int lang:{1,0,1,0}){require(language.set_language(lang),"Pass policy language switch");language.update(s,0,true);language.request(18,1);language.update(s,0,true);
  require(language.category()==(lang?18:0),"Set-piece policy follows active language");}
 require(!language.set_language(2),"Invalid language remains rejected");language.request(18,1);language.update(s,0,true);require(!language.speaking(),"Failed language switch preserves retail silence");
 // The renamed retail POS bank supplies all eight GL_POST takes. Compare
 // full decoded recordings, then dispatch through both actual goal branches.
 for(int n=1;n<=8;++n){OriginalCommentary voice(media,false);voice.set_volume(100);voice.update(s,0,true);voice.request(27,n);voice.update(s,0,true);
  require(voice.category()==27 && voice.history().back().number==n,"Every rebound-goal take remains addressable");compare(voice,27,n,true);}
 for(bool right:{false,true}){
  s.start_match(true);s.ticks=10;OriginalCommentary voice(media,false);voice.set_volume(100);voice.update(s,0,true);
  e.commentary_count=0;e.ball_in_goal=0;e.match_mode=0;e.prediction=0;e.last_touch=right?2:13;
  e.headed_ball=0;e.rebound_ball=1;e.wall_is_up=0;e.max_sw_factor=0;e.shot_speed=10;e.shot_pending=1;
  if(right)e.go_right_goal();else e.go_left_goal();
  require(e.commentary_count==1 && e.commentary_queue[0]==27,"Actual rebound goal queues original source event");
  const auto seed=e.seed;voice.update(s,0,true);
  require(voice.category()==27 && e.seed==seed,"Retail rebound speech preserves goal event and gameplay RNG");
  compare(voice,27,voice.history().back().number,true);
  if(right)e.go_right_goal();else e.go_left_goal();
  require(!e.commentary_count,"Latched goal cannot repeat rebound speech");
 }
 s.start_match(true);s.ticks=10;
 // Every legal score gets all four A-D takes before repeating. The public
 // score identity remains the two digits; variant cannot select another score.
 unsigned score_cases=0;std::vector<float> drain(110250*2);
 for(int home=0;home<=7;++home)for(int away=0;away<=5 && away<=home;++away){
  OriginalCommentary voice(media,false);voice.set_volume(100);voice.update(s,0,true);std::set<int> variants;
  for(int repeat=0;repeat<4;++repeat){const auto seed=e.seed;e.PlayScoreSample(home,away);voice.update(s,0,true);
   require(voice.category()==44 && voice.history().back().number==home*10+away,"Exact score survives retail variant selection");
   const int variant=voice.history().back().variant;require(variants.insert(variant).second,"Four distinct retail score takes before repetition");
   compare(voice,44,home*10+away+variant*100);voice.mix(drain.data(),110250);for(int i=0;i<12;++i)voice.update(s,.25,true);
   require(e.seed==seed,"Retail speech does not consume gameplay RNG");++score_cases;
  }
 }
 // Explicit old-media fallback remains usable and is not overwritten.
 const auto legacy=scratch/"legacy";
 write(legacy/"COMMENTARY/ENGLISH3.BIN",read_file(media/"COMMENTARY/ENGLISH3.BIN"));
 write(legacy/"DIGI/PITCH/PITCH.MAD",read_file(media/"DIGI/PITCH/PITCH.MAD"));
 OriginalCommentary fallback(legacy,false);fallback.update(s,0,true);fallback.score(2,1);fallback.update(s,0,true);require(fallback.category()==44 && fallback.history().back().variant==0,"Legacy COM1 scores still work");
 // The older disc lacks CPB01; take 2 exists in both compatibility categories.
 for(int id:{14,18}){OriginalCommentary old(legacy,false);old.update(s,0,true);old.request(id,2);old.update(s,0,true);require(old.category()==id,"Older English retains distinct pass recordings");}
 OriginalAudio old_pitch(legacy,false),new_pitch(media,false);require(new_pitch.status()=="Euro 96 match sounds ready","Retail pitch bank preferred");
 std::vector<float> a(4800*2),b(a.size());CameraPose camera{};
 for(int id=1;id<=19;++id){old_pitch.set_muted(true);old_pitch.set_muted(false);new_pitch.set_muted(true);new_pitch.set_muted(false);old_pitch.pitch(id,640,400,camera);new_pitch.pitch(id,640,400,camera);old_pitch.mix(a.data(),4800);new_pitch.mix(b.data(),4800);require(a==b,"All 19 retail pitch effects preserve identical PCM and mapping");}
 for(int id=1;id<=10;++id){old_pitch.set_muted(true);old_pitch.set_muted(false);new_pitch.set_muted(true);new_pitch.set_muted(false);old_pitch.crowd(id);new_pitch.crowd(id);old_pitch.mix(a.data(),4800);new_pitch.mix(b.data(),4800);
  for(std::size_t i=0;i<a.size();++i)require(std::abs(b[i]-a[i]*(id==10?25600.0f/26624:1.0f))<.000001f,"Retail crowd mapping and foul amplitude");}
 const auto bad=scratch/"bad";
 for(int mode=0;mode<7;++mode){auto bytes=pack;
  if(mode==0)bytes[7]='?';if(mode==1)bytes.pop_back();
  if(mode==2){bytes[score_offset+2]=144;bytes[score_offset+3]=1;} // variant E
  if(mode==3){bytes[score_offset+2]=67;bytes[score_offset+3]=0;} // illegal 6:7
  if(mode==4){bytes[14]=101;bytes[15]=0;} // variants only exist for scores
  if(mode==5){const auto len=8+read(bytes,16,4);Bytes duplicate(bytes.begin()+12,bytes.begin()+12+len);bytes.insert(bytes.end(),duplicate.begin(),duplicate.end());const unsigned count=read(bytes,8,4)+1;for(int i=0;i<4;++i)bytes[8+i]=(count>>(8*i))&255;}
  if(mode==6)bytes[7]='1'; // encoded variants are invalid in COM1
  write(bad/"COMMENTARY/EURO_EVENTS.BIN",bytes);bool rejected=false;try{OriginalCommentary invalid(bad,false);}catch(const std::exception&){rejected=true;}require(rejected,"Invalid/ambiguous variant packs rejected");
 }
 write(bad/"DIGI/PITCH/EURO_PITCH.MAD",Bytes{'b','a','d'});write(bad/"DIGI/PITCH/PITCH.MAD",read_file(media/"DIGI/PITCH/PITCH.MAD"));
 bool rejected=false;try{OriginalAudio invalid(bad,false);}catch(const std::exception&){rejected=true;}require(rejected,"Corrupt retail audio must not silently use legacy bank");
 std::cout<<"Retail audio: source category routing, 17 short-pass takes, 12 reception cases, shared pass cooldown, language/fallback policy, 8 rebound takes and both goal triggers, "<<score_cases<<" exact score variants, 19 pitch effects, 10 crowd mappings, legacy fallback and malformed packs passed.\n";
 }
 SDL_Quit();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';SDL_Quit();return 1;}}
