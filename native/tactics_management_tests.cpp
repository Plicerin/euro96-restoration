#include "original_frontend.hpp"
#include "action_constants.hpp"
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
using namespace actua;
using namespace actua::original::action_constants;
static void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
static void write_save(const std::filesystem::path& path,const std::string& body){std::uint64_t hash=1469598103934665603ull;for(unsigned char c:body){hash^=c;hash*=1099511628211ull;}std::ofstream out(path,std::ios::binary);out<<body<<hash<<'\n';}
int main(int argc,char** argv){try{
 require(argc==4,"Source, media and scratch required");require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO),SDL_GetError());
 const std::filesystem::path folder=argv[3];std::filesystem::create_directories(folder);const auto save=folder/"tactics.sav";
 {
 OriginalGraphics g(argv[1]);auto& s=*g.sequence;auto& e=s.engine;OriginalTournament t;t.reset(14);
 for(int team=0;team<16;++team)require(t.formations[team]==original_teams[team].formation && t.takers[team]==std::array<int,3>{},"Original formations and automatic takers default");
 t.formations[14]=9;t.formations[4]=3;t.takers[14]={11,10,9};t.takers[4]={7,6,5};t.players[14][1].fitness=60;t.prepare(s);s.start_match(false);
 require(s.home_team==14 && s.home_formation==9 && s.away_formation==3 && s.match_takers[0]==t.takers[14],"Human away-country mapping includes its saved tactics");
 require(s.preferred_taker(false,0)==11 && e.corn_kicker_b==16,"Preferred shirt maps to physical slot on each team");
 require(!s.set_interval_formation(false,3),"Cannot change formation during live play");
 Archive archive(std::filesystem::path(argv[1])/"EURO.DAT",std::filesystem::path(argv[1])/"EURO.OFF");
 s.interval_ticks=100;s.interval_minute=45;const auto players=e.teams;const auto tick=s.ticks;const auto generation=s.generation;
 for(int f=0;f<10;++f){require(s.set_interval_formation(false,f),"Every original formation selectable at interval");const auto tactics=decode_tactics(archive.entry(111+f));
  for(int z=0;z<70;++z)for(int p=0;p<10;++p)require(e.match_tactics1[z][p][0]==tactics[z][p].x && e.match_tactics1[z][p][1]==tactics[z][p].y,"Entire original tactics table reaches engine");}
 require(!s.set_interval_formation(false,10) && !std::memcmp(players.data(),e.teams.data(),sizeof(players)) && s.ticks==tick && s.generation==generation,"Formation edit is bounded and does not reset players or time");s.interval_ticks=0;
 const auto restart=[&](int mode){e.holder_lose_ball();e.reset_shot();e.reset_ball();e.just_scored=e.ball_in_goal=e.match_mode=0;
  const bool physicalB=mode==PEN_KICK_B || mode==DF_KICK_B || mode==IF_KICK_B;
  e.incident_x=physicalB?280:1000;e.incident_y=350;e.ballx=e.incident_x;e.bally=e.incident_y;e.ballz=2;e.ballxdis=e.ballydis=e.ballzdis=0;
  e.last_touch=physicalB?2:13;e.fkick_taker=physicalB?13:2;e.penalty_taker=0;s.restart_mode=mode;s.restart_ticks=1;s.step();};
 restart(PEN_KICK_A);require(e.setp_taker==11,"Awarded penalty uses selected player");
 restart(DF_KICK_A);require(e.setp_taker==10,"Shooting free kick uses selected player");
 restart(IF_KICK_A);require(e.setp_taker==2,"Indirect free kick retains original selected taker");
 restart(CORNER_TR);require(e.setp_taker==9,"Corner uses selected player");
 e.swap_teams();s.interval_ticks=100;s.set_interval_formation(false,5);s.interval_ticks=0;s.refresh_setpiece_takers();
 require(e.penalty_taker_b==22 && e.corn_kicker_a==5,"Takers follow countries after changing ends");
 const auto changed=decode_tactics(archive.entry(116));require(e.match_tactics2[20][3][0]==changed[20][3].x,"Formation follows home country after end swap");
 restart(PEN_KICK_B);require(e.setp_taker==22,"Second-half awarded penalty keeps preferred shirt");
 restart(DF_KICK_B);require(e.setp_taker==21,"Second-half free kick keeps preferred shirt");
 restart(CORNER_TL);require(e.setp_taker==20,"Second-half corner keeps preferred shirt");
 require(s.queue_substitution(false,10,s.bench_shirt(false,0)),"Substitute chosen penalty taker");s.apply_substitutions();s.refresh_setpiece_takers();require(s.preferred_taker(false,0)==0 && e.penalty_taker_b==0,"Replacement does not inherit outgoing player's preference");
 restart(PEN_KICK_B);require(e.teams[e.setp_taker-1].guy_on && e.setp_taker>12,"Unavailable taker falls back to eligible player on correct side");
 e.teams[19].guy_on=0;s.refresh_setpiece_takers();require(e.corn_kicker_b==0,"Dismissed preferred corner taker excluded");
 restart(CORNER_TL);require(e.setp_taker!=20 && e.teams[e.setp_taker-1].guy_on,"Corner fallback excludes dismissed player");
 const auto adjusted=conditioned_stats(original_teams[14].players[1],60);require(adjusted.pace==std::max(0,original_teams[14].players[1].pace-20) && adjusted.control==original_teams[14].players[1].control,"Attribute panel shares match fitness calculations");
 // Version 3 preserves preferences; real version 2 format migrates player state.
 t.save(save);OriginalTournament loaded;loaded.load(save);require(loaded.formations==t.formations && loaded.takers==t.takers,"Tactics save roundtrip");
 std::ifstream input(save,std::ios::binary);std::string bytes((std::istreambuf_iterator<char>(input)),{});input.close();std::istringstream lines(bytes);std::string line,legacy;int n=0;
 const int oldLines=2+int(t.results.size())+16*21;
 while(n<oldLines && std::getline(lines,line)){if(n==0)line="ACTUA96_TOURNAMENT 2";legacy+=line+'\n';++n;}
 write_save(folder/"v2.sav",legacy);loaded.load(folder/"v2.sav");require(loaded.players[14][1].fitness==60 && loaded.formations[14]==original_teams[14].formation && loaded.takers[14]==std::array<int,3>{},"v2 migration preserves condition and initializes absent preferences");
 std::string bad=legacy;bad.replace(0,std::string("ACTUA96_TOURNAMENT 2").size(),"ACTUA96_TOURNAMENT 3");for(int team=0;team<16;++team)bad+=std::to_string(team==0?10:4)+" 0 0 0\n";write_save(folder/"bad.sav",bad);
 bool rejected=false;try{loaded.load(folder/"bad.sav");}catch(...){rejected=true;}require(rejected && loaded.players[14][1].fitness==60,"Invalid checksummed formation rejected atomically");
 // The UI edits saved country choices and never restarts a match at the interval.
 t.reset(14);t.players[14][1].fitness=60;t.save(save);OriginalFrontend front(argv[1],argv[2],save);front.open(g);using Stage=OriginalFrontend::Stage;
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 // Render the destination between deliberate inputs; queued unseen-page input
 // is exercised separately by menu_transition_tests.
 const auto paint=[&]{front.draw(renderer,g,false,false);for(int i=0;i<9 && front.menu_fading_out();++i){front.observe(g,.1);front.draw(renderer,g,false,false);}};
 const auto key=[&](SDL_Keycode k){paint();SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=k;require(front.event(event,g),"Tactics input consumed");};
 const auto click=[&](float x,float y){paint();SDL_Event event{};event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.button=SDL_BUTTON_LEFT;event.button.x=48+x*1.35f;event.button.y=8+y*1.35f;front.event(event,g);};
 key(SDLK_L);key(SDLK_RETURN);key(SDLK_T);require(front.stage()==Stage::tactics,"Lineup opens tactics");const int before=front.tournament().formations[14];key(SDLK_RIGHT);loaded.load(save);require(loaded.formations[14]==(before+1)%10,"Formation change immediately saves");
 const int enemy=front.tournament().formations[4];key(SDLK_TAB);key(SDLK_RIGHT);require(front.tournament().formations[4]==enemy,"Opponent tactics view only");
 SDL_Event pad{};pad.type=SDL_EVENT_GAMEPAD_BUTTON_DOWN;pad.gbutton.button=SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;paint();front.event(pad,g);const int own=front.tournament().formations[14];click(230,430);require(front.tournament().formations[14]==(own+1)%10,"Mouse change and gamepad team navigation");
 const auto capture=[&](const char* name){front.draw(renderer,g,false,false);SDL_RenderPresent(renderer);require(SDL_SaveBMP(surface,(folder/(std::string(name)+".bmp")).string().c_str()),SDL_GetError());};
 capture("formation");pad.gbutton.button=SDL_GAMEPAD_BUTTON_NORTH;paint();front.event(pad,g);key(SDLK_RIGHT);key(SDLK_DOWN);key(SDLK_RIGHT);key(SDLK_DOWN);key(SDLK_RIGHT);require(front.tournament().takers[14]==std::array<int,3>{2,2,2},"Three roles selectable with source player identity");capture("takers");
 click(320,410);key(SDLK_RIGHT);capture("attributes");key(SDLK_RETURN);key(SDLK_RETURN);require(!front.visible() && s.home_formation==front.tournament().formations[14] && s.match_takers[0][0]==2,"Kickoff receives reviewed tactics");
 s.blue_goals=2;e.teams[1].tm_pass=17;s.interval_ticks=100;s.interval_minute=45;e.match_time.min=45;front.observe(g,0);require(front.stage()==Stage::interval,"Half-time page");click(390,430);require(front.stage()==Stage::tactics && g.paused,"Mouse opens half-time tactics");
 const auto frozen=e.teams;const int frozenTicks=s.ticks;const auto frozenGeneration=s.generation;s.cpu_vs_cpu=true;
 for(int i=0;i<80;++i)front.observe(g,.25);const int was=s.home_formation;key(SDLK_RIGHT);
 require(s.home_formation==(was+1)%10 && s.blue_goals==2 && s.ticks==frozenTicks && s.generation==frozenGeneration && !std::memcmp(frozen.data(),e.teams.data(),sizeof(frozen)),"Half-time tactics preserve score, clock, players and pause; CPU auto timer held");capture("halftime");
 key(SDLK_RETURN);require(front.stage()==Stage::interval,"Return to half-time without kickoff");key(SDLK_RETURN);s.step();require(e.match_half==1 && e.teams[12].tm_pass==17 && s.blue_goals==2 && s.ticks==frozenTicks+1,"Resume changes ends once and preserves statistics");
 const auto tactics=decode_tactics(archive.entry(111+s.home_formation));require(e.match_tactics2[69][4][1]==tactics[69][4].y,"Second half uses changed tactics table");
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);
 std::cout<<"Ten original formations, persistent country mapping, preferred set pieces, substitution/dismissal fallback, v2 migration, attribute parity, controls and non-destructive half-time changes passed.\n";
 }
 SDL_Quit();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
