#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "original_frontend.hpp"
#include "original_commentary.hpp"
#include <iostream>
#include <set>
#include <stdexcept>
#include <algorithm>
using namespace actua;
static void require(bool v,const char* m){if(!v)throw std::runtime_error(m);}
static std::uint64_t hash(SDL_Surface* s){std::uint64_t h=1469598103934665603ull;auto* b=static_cast<unsigned char*>(s->pixels);for(int i=0;i<s->pitch*s->h;++i){h^=b[i];h*=1099511628211ull;}return h;}
int main(int argc,char** argv){try{
 require(argc==4,"Source, media and capture folder required");require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO),SDL_GetError());
 const std::filesystem::path folder=argv[3];std::filesystem::create_directories(folder);
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 {
 OriginalGraphics g(argv[1]);g.load_cd_graphics(argv[2]);g.exits_enabled=false;g.action_view=true;auto& s=*g.sequence;auto& e=s.engine;s.start_match(true);
 const auto capture=[&](const std::string& name){SDL_RenderPresent(renderer);require(SDL_SaveBMP(surface,(folder/(name+".bmp")).string().c_str()),SDL_GetError());};
 std::set<std::uint64_t> venues;
 for(int i=0;i<5;++i){require(g.stadium_available(i),"All five original venues imported");g.next_stadium=i;const auto seed=e.seed;const auto x=e.ballx;g.apply_stadium();g.draw(renderer);capture("venue-"+std::to_string(i));venues.insert(hash(surface));
  require(e.seed==seed && e.ballx==x && s.ticks==0,"Venue changes do not advance gameplay");require(g.scene_counts.flags==4 && g.scene_counts.stands==4,"Every venue has all stands and four original flags");
 }
 require(venues.size()==5,"Original venue sets must visibly differ");
 OriginalFrontend front(argv[1],argv[2]);front.open(g);
 // Render the destination between deliberate inputs; queued unseen-page input
 // is exercised separately by menu_transition_tests.
 const auto paint=[&]{front.draw(renderer,g,false,false);for(int i=0;i<9 && front.menu_fading_out();++i){front.observe(g,.1);front.draw(renderer,g,false,false);}};
 const auto key=[&](SDL_Keycode k){paint();SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=k;front.event(event,g);};
 key(SDLK_RETURN);for(int i=0;i<7;++i)key(SDLK_DOWN);require(front.row()==7,"Venue is reachable by keyboard/gamepad navigation");key(SDLK_RIGHT);require(g.next_stadium==(g.stadium_available(5)?5:0) && g.stadium()==4,"Setup selection does not change live venue");front.draw(renderer,g,false,false);capture("selection");
 key(SDLK_RETURN);key(SDLK_RETURN);require(g.stadium()==(g.stadium_available(5)?5:0),"Kickoff applies selected complete venue");if(g.prematch.active())key(SDLK_RETURN);
 // Close corner view, original wind path, paused render determinism.
 s.team_play=false;s.play=false;s.visible_players.clear();s.engine.ballx=640;s.engine.bally=400;g.overview=true;
 e.wind_on=1;e.wind_speed=80;e.wind_x=1;e.wind_y=0;s.ticks=0;g.draw(renderer);capture("flags-wind-0");const auto a=hash(surface);
 s.ticks=10;g.draw(renderer);capture("flags-wind-1");require(hash(surface)!=a,"Original wind flag geometry/texture visibly animates");const auto b=hash(surface);g.paused=true;g.advance(.1);g.draw(renderer);SDL_RenderPresent(renderer);require(hash(surface)==b,"Paused flags stay still");g.overview=false;g.paused=false;
 s.start_match(true);s.ticks=10;
 const auto figures=s.match_figures();require(figures[0][9]==0 && figures[1][9]==0,"Kickoff never counts as a corner");
 e.teams[1].tm_pass=8;e.teams[1].tm_pwon=6;e.teams[1].tm_tacks=4;e.teams[1].tm_twon=3;e.teams[1].tm_work=100;s.possession_ticks={30,10};
 s.record_restart_statistics(2);s.record_restart_statistics(16);e.offside_now=1;s.record_restart_statistics(13);
 auto f=s.match_figures();require(f[0][9]==1 && f[1][10]==1 && f[1][12]==1 && f[0][6]==30,"Match statistics retain correct awarded/committing side");
 require(s.queue_substitution(false,1,12),"Substitution fixture");s.apply_substitutions();f=s.match_figures();require(f[0][2]==8 && f[0][3]==6 && f[0][7]==100,"Outgoing player's original statistics survive substitution");
 e.swap_teams();f=s.match_figures();require(f[0][2]==8 && f[1][10]==1,"Statistics follow team identity after end swap");
 s.interval_ticks=100;s.interval_minute=45;front.observe(g,0);front.draw(renderer,g,false,false);capture("statistics");
 const auto frozen=s.match_figures();s.start_shootout();e.teams[0].tm_shots+=99;require(s.match_figures()==frozen,"Shootout cannot contaminate match statistics");
 s.start_match(true);g.paused=false;
 for(int n=0;n<5;++n){const char* titles[]={"GOAL","BOOKED, YELLOW CARD","RED CARD, SENT OFF","OFFSIDE","SUBSTITUTION"};const int icons[]={29,45,44,46,0};s.notice(titles[n],n==4?"ENG SHEARER OFF / ENG FERDINAND ON":"ENG SHEARER",icons[n]);g.draw(renderer);capture("notice-"+std::to_string(n));}
 s.start_match(true);s.ticks=10;require(e.say_names==1,"Original match-start name flag enabled");
 for(int half=0;half<2;++half){
  if(half)e.swap_teams();const int slot=half?19:8;require(e.teams[slot].shirt==9,"Shearer fixture follows end swap");
  OriginalCommentary voice(argv[2],false);voice.update(s,0,true);const auto seed=e.seed;e.PlayTeamSample(9,0);voice.update(s,0,true);
  require(voice.category()==128+4*20+8 && voice.history().back().number==1,"Original name callback selects matched player and intonation in either half");require(e.seed==seed,"Name speech does not consume gameplay RNG");
  std::vector<float> pcm(22050);voice.mix(pcm.data(),11025);require(std::any_of(pcm.begin(),pcm.end(),[](float v){return v!=0;}),"Name clip contains audible original PCM");
  voice.update(s,0,false);e.prediction=1;e.PlayTeamSample(9,1);e.prediction=0;require(!e.commentary_count,"Prediction emits no player names");
 }
 s.start_match(true);s.ticks=10;e.teams[8].shirt=15;OriginalCommentary subvoice(argv[2],false);subvoice.update(s,0,true);e.PlayTeamSample(9,1);subvoice.update(s,0,true);require(subvoice.category()==128+4*20+14,"Substitute gets his own matched recording, not outgoing player's name");
 subvoice.request(22);subvoice.update(s,0,true);require(subvoice.category()==22,"Goal commentary interrupts low-priority name");
 s.start_match(true);s.ticks=10;OriginalCommentary unknown(argv[2],false);unknown.update(s,0,true);s.home_team=1;e.PlayTeamSample(9,0);unknown.update(s,0,true);require(unknown.speaking()==std::filesystem::exists(std::filesystem::path(argv[2])/"COMMENTARY/EURO_NAMES.BIN"),"Croatia name is available only with the retail overlay");
 s.start_match(true);e.prediction=0;e.predict_ball();require(!e.prediction,"Live prediction scope restored before player callbacks");
 e.prediction=1;e.predict_ball();require(e.prediction,"Nested prediction preserves its caller's scope");e.prediction=0;
 OriginalCommentary live(argv[2],false);std::vector<float> sound(1102);
 for(int tick=0;tick<20000 && s.running;++tick){s.step();live.update(s,.05,true);live.mix(sound.data(),551);}
 unsigned names=0;for(int i=128;i<448;++i)names+=live.stats().played[i];std::cout<<"Name diagnostics: callbacks="<<e.team_sample_events<<" requested="<<live.stats().requests<<" dropped="<<live.stats().dropped<<" played="<<names<<" enabled="<<int(e.say_names)<<" ticks="<<s.ticks<<" possessions="<<e.possession_events<<" shots="<<s.ai_shots<<" full_time="<<s.full_time<<" home="<<s.home_team<<"\n";require(names>0,"Real CPU play must produce original player-name commentary");std::cout<<"CPU player-name calls played: "<<names<<'\n';
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();std::cout<<"Five venues, original flags, notices, statistics retention and matched player-name commentary passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
