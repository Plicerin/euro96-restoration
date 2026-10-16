#include "original_frontend.hpp"
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
using namespace actua;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 require(argc==4,"Expected source, media and scratch folder");require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO),SDL_GetError());
 const std::filesystem::path folder=argv[3];std::filesystem::create_directories(folder);const auto save=folder/"tournament.sav";
 OriginalTournament t;t.reset(-1);require(t.next()==std::array<int,2>{4,14},"Original opening England/Switzerland fixture");
 std::array<int,16> games{};std::set<std::pair<int,int>> pairs;
 for(int i=0;i<24;++i){auto f=t.fixture(i);++games[f[0]];++games[f[1]];require(t.draw[f[0]]/4==t.draw[f[1]]/4,"Group opponents share group");require(pairs.emplace(std::min(f[0],f[1]),std::max(f[0],f[1])).second,"Every group pairing unique");}
 for(int n:games)require(n==3,"Every team plays three group games");
 t.table[4]={1,0,1,2,6};t.table[14]={0,3,0,10,0};t.rank();require(t.positions[4]<t.positions[14],"Original wins precede goal difference when points tie");
 t.reset(-1);t.record({4,14,2,1});t.save(save);OriginalTournament loaded;loaded.load(save);require(loaded.results.size()==1 && loaded.table[4].goalsFor==2,"Round trip checkpoint");
 {std::ofstream corrupt(folder/"bad.sav");corrupt<<"bad";}bool rejected=false;try{loaded.load(folder/"bad.sav");}catch(...){rejected=true;}require(rejected && loaded.results.size()==1,"Bad save cannot mutate valid in-memory progress");
 t.reset(-1);for(int i=0;i<24;++i){const auto f=t.next();t.record({f[0],f[1],2,1});}
 for(const auto& row:t.table)require(row.won+row.drew+row.lost==3 && row.goalsFor+row.goalsAgainst==9,"Goals accumulate across all group matches");
 const auto table=t.table;for(int i=24;i<31;++i){const auto f=t.next();require(f[0]>=0 && f[1]>=0,"Bracket resolves from actual qualifiers and winners");t.record({f[0],f[1],1,1,4,3,true,true});t.save(save);loaded.load(save);require(loaded.next()==t.next() && loaded.results.size()==t.results.size(),"Knockout save retains winning teams and penalty scores");}
 require(t.complete() && t.champion()>=0,"Final produces champion");for(int i=0;i<16;++i)require(t.table[i].goalsFor==table[i].goalsFor,"Knockout goals do not enter group table");
 {
 OriginalGraphics g(argv[1]);g.load_cd_graphics(argv[2]);auto& s=*g.sequence;
 // Full competition through the real original CPU engine, checkpoint after every fixture.
 t.reset(-1);int extra=0,pens=0,injury_points=0,booked=0,reds=0;
 for(int team=0;team<16;++team){t.formations[team]=(original_teams[team].formation+1)%10;t.takers[team]={9,8,7};}
 while(!t.complete()){
  const int fixture=int(t.results.size());t.prepare(s);s.start_match(true);int ticks=0;
  while(s.running && ticks++<60000)s.step();
  if(!s.full_time){std::cerr<<"Unfinished fixture "<<fixture<<" teams "<<s.home_team<<'/'<<s.away_team<<" time "<<s.clock_label()<<" mode "<<s.engine.match_mode<<" poss "<<s.engine.ball_poss<<'\n';throw std::runtime_error("CPU tournament fixture must finish without invented results");}
  for(const auto& side:s.player_reports())for(const auto& p:side){injury_points+=p.injury;booked+=p.bookings%5>0;reds+=p.bookings>=5;}
  require(t.collect(s),"Collect actual result");extra+=t.results.back().extra;pens+=t.results.back().pens;t.save(save);loaded.load(save);t=loaded;
  std::cout<<fixture+1<<" "<<original_teams[s.home_team].code<<' '<<s.blue_goals<<':'<<s.red_goals<<' '<<original_teams[s.away_team].code<<" ticks="<<ticks<<" penalties="<<s.shootout<<'\n';
 }
 std::cout<<"31 original-engine matches complete; champion="<<original_teams[t.champion()].name<<" extra="<<extra<<" shootouts="<<pens<<" injury_points="<<injury_points<<" booked="<<booked<<" dismissed="<<reds<<'\n';
 // A scheduled away country is controlled in the existing native home slot;
 // its scores must still return to the original tournament fixture order.
 OriginalTournament own;own.reset(14);own.prepare(s);require(s.next_home==14 && s.next_away==4,"Away chosen country owns native human slot");s.start_match(false);s.blue_goals=3;s.red_goals=1;s.finish_match();own.collect(s);require(own.results[0].hg==1 && own.results[0].ag==3,"Controlled away-country result maps back to fixture");
 OriginalFrontend front(argv[1],argv[2],folder/"ui.sav");using Stage=OriginalFrontend::Stage;
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 // Render the destination between deliberate inputs; queued unseen-page input
 // is exercised separately by menu_transition_tests.
 const auto paint=[&]{front.draw(renderer,g,false,false);for(int i=0;i<9 && front.menu_fading_out();++i){front.observe(g,.1);front.draw(renderer,g,false,false);}};
 const auto key=[&](SDL_Keycode k){paint();SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;require(front.event(e,g),"Tournament key consumed");};
 // A named capture must show its destination, not the previous page held by
 // an outgoing fade. Transition timing is covered by menu_transition_tests.
 const auto capture=[&](const char* name){const bool fades=front.menu_fades;front.menu_fades=false;front.draw(renderer,g,false,false);SDL_RenderPresent(renderer);require(SDL_SaveBMP(surface,(folder/(std::string(name)+".bmp")).string().c_str()),SDL_GetError());front.menu_fades=fades;};
 front.open(g);key(SDLK_T);require(front.stage()==Stage::tournament_setup,"Title opens tournament entry");capture("entry");key(SDLK_RETURN);require(front.tournament().active,"Watch-all tournament created");capture("groups");
 key(SDLK_Y);require(front.stage()==Stage::simulating,"Skip runs a real CPU match");const int before=s.ticks;front.observe(g,0);require(s.ticks==before,"Focus loss holds background simulation");
 for(int frames=0;frames<400 && front.stage()==Stage::simulating;++frames)front.observe(g,.05);
 require(front.tournament().results.size()==1 && front.stage()==Stage::tournament,"Background result returns to tournament exactly once");front.observe(g,.05);require(front.tournament().results.size()==1,"No duplicated result");capture("group-result");
 key(SDLK_RETURN);key(SDLK_RETURN);require(!front.visible(),"Watch next fixture launches match");require(g.prematch.active(),"Watched tournament includes opening");key(SDLK_RETURN);
 SDL_Event shoulder{};shoulder.type=SDL_EVENT_GAMEPAD_BUTTON_DOWN;shoulder.gbutton.button=SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER;
 require(front.event(shoulder,g) && s.cpu_vs_cpu,"Watched tournament blocks CPU takeover");
 s.set_cpu_control(false);require(!front.event(shoulder,g) && !s.cpu_vs_cpu,"Human tournament passes RB through for player selection");s.set_cpu_control(true);
 const int home=s.home_team;key(SDLK_R);key(SDLK_F5);key(SDLK_G);key(SDLK_H);key(SDLK_M);key(SDLK_N);require(s.home_team==home && s.ticks==0,"Practice/rematch shortcuts cannot replace tournament fixture");
 key(SDLK_F2);require(g.paused && front.stage()==Stage::tournament,"Tournament hub pauses current match");key(SDLK_ESCAPE);require(!front.visible() && !g.paused,"Returning preserves live match");
 key(SDLK_F2);key(SDLK_T);key(SDLK_L);require(front.stage()==Stage::tournament && front.tournament().results.size()==1,"Resume restores completed fixtures, unfinished game restarts");
 key(SDLK_A);for(int frame=0;frame<21;++frame)front.observe(g,.25);require(!front.visible() && s.cpu_vs_cpu,"Watch-all auto continues into the next CPU match");if(g.prematch.active())key(SDLK_RETURN);key(SDLK_F2);key(SDLK_T);
 t.save(folder/"ui.sav");key(SDLK_L);require(front.tournament().complete(),"Completed tournament resumes with champion");capture("champion");
 key(SDLK_ESCAPE);SDL_Event pad{};pad.type=SDL_EVENT_GAMEPAD_BUTTON_DOWN;pad.gbutton.button=SDL_GAMEPAD_BUTTON_WEST;paint();front.event(pad,g);require(front.stage()==Stage::tournament_setup,"Gamepad tournament entry");
 key(SDLK_RIGHT);key(SDLK_RETURN);require(front.tournament().user==0,"Country selection preserved");key(SDLK_RETURN);require(front.stage()==Stage::simulating,"Other teams simulate when playing a selected country");key(SDLK_ESCAPE);require(front.tournament().results.empty(),"Cancelled simulation never invents a result");
 OriginalFrontend blocked(argv[1],argv[2],folder/"bad.sav"/"child.sav");blocked.open(g);const auto blockedkey=[&](SDL_Keycode k){SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=k;blocked.event(event,g);};blockedkey(SDLK_T);blockedkey(SDLK_RETURN);require(!blocked.tournament_error().empty(),"Save failure is visible to user");blockedkey(SDLK_RETURN);require(blocked.stage()==Stage::tournament,"Cannot silently continue without requested checkpoint");
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);
 }
 SDL_Quit();std::cout<<"Tournament fixtures, ranking, knockout, save/load, controls and full CPU competition passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
