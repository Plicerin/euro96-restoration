#include "original_frontend.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace actua;
static void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
static void checkpoint(const std::filesystem::path& path,const std::string& body){
 std::uint64_t hash=1469598103934665603ull;for(unsigned char c:body){hash^=c;hash*=1099511628211ull;}
 std::ofstream out(path,std::ios::binary);out<<body<<hash<<'\n';
}
int main(int argc,char** argv){try{
 require(argc==4,"source, media, scratch required");require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO),SDL_GetError());
 const std::filesystem::path folder=argv[3];std::filesystem::create_directories(folder);const auto path=folder/"continuity.sav";
 {
 OriginalGraphics g(argv[1]);auto& s=*g.sequence;auto& e=s.engine;OriginalTournament t;t.reset(4);
 require(t.players[4][0].fitness==original_teams[4].players[0].fitness,"Original SCRIPT fitness imported");
 t.players[4][1].fitness=60;t.prepare(s);s.start_match(false);
 const auto adjusted=s.match_stats(false,2);require(e.teams[1].tm_rate==std::max(0,original_teams[4].players[1].pace-20)+28,"EURO_INF fitness adjustment reaches engine pace");
 const int initial=e.teams[1].tm_rate;e.teams[1].tm_time=80;e.apply_stamina(&e.teams[1],initial);require(e.teams[1].tm_rate<initial,"Original fatigue applies on top of starting fitness");
 // Live original collision, with tournament-only native injury trigger.
 s.restart_ticks=s.restart_mode=e.match_mode=e.dead_ball_cnt=0;e.just_scored=0;
 e.holder_lose_ball();for(auto& p:e.teams){p.tm_x=50;p.tm_y=50;p.tm_act=5;}
 auto& a=e.teams[1];auto& b=e.teams[12];a.tm_x=b.tm_x=640;a.tm_y=b.tm_y=400;a.tm_act=3;a.go_cnt=30;a.go_txdis=4;a.go_tydis=0;b.tm_act=0;e.player_num=2;e.seed=0;e.player_ints(&a);
 require(b.tm_act==5 && b.tm_inj==20,"Crunching tackle reports injury on the victim");
 // Outgoing player's identity and report survive replacement and end swaps.
 t.reset(14);t.players[14][1].yellows=1;t.prepare(s);s.start_match(false);e.inc_yellow(2);e.inc_inj(2,80);
 require(s.queue_substitution(false,1,s.bench_shirt(false,0)),"Queue substitute");s.apply_substitutions();
 e.swap_teams();const auto report=s.player_reports();require(report[0][1].bookings==1 && report[0][1].injury==80,"Substituted player's report retained by home-country identity");
 require(!t.collect(s) && t.players[14][1].ban==0,"Unfinished match cannot consume or alter condition");
 s.blue_goals=2;s.red_goals=1;s.finish_match();require(t.collect(s),"Completed match collected");
 require(t.players[14][1].ban==1 && t.players[14][1].yellows==0 && t.players[14][1].fitness==19,"Second yellow earns ban; injury deducted and preserved");
 require(!t.available(14,2),"Injured/banned player unavailable");
 for(int i=0;i<15;++i)require(t.squad[14][i]!=2,"Unavailable player moved out of match squad");
 t.save(path);OriginalTournament loaded;loaded.load(path);require(loaded.players[14][1].fitness==19 && loaded.players[14][1].ban==1 && loaded.squad==t.squad,"Condition, discipline and squad save roundtrip");
 while(t.next()[0]!=14 && t.next()[1]!=14){t.prepare(s);s.start_match(true);s.finish_match();t.collect(s);require(t.players[14][1].ban==1 && t.players[14][1].fitness==19,"Other teams' fixtures do not serve a ban or heal injury");}
 t.prepare(s);s.start_match(true);require(!s.used_squad[0][1],"Banned selected country player not starting");
 require(!s.bench_available(false,2),"Banned player cannot enter from bench");s.finish_match();t.collect(s);
 require(t.players[14][1].ban==0 && t.players[14][1].fitness==29 && t.available(14,2),"Own completed fixture serves one match ban and recovers fitness");
 // Role-safe selection and arbitrary shirts on original four-slot match bench.
 t.reset(-1);require(!t.swap_squad(4,0,1),"Cannot replace starting goalkeeper with outfielder");
 require(t.swap_squad(4,1,19),"Reserve can start");require(t.swap_squad(4,11,18),"Reserve can join match bench");
 t.prepare(s);s.start_match(true);require(e.teams[1].shirt==20 && s.bench_shirt(false,0)==19,"Configured starters and bench reach actual game");
 require(s.queue_substitution(false,1,19),"Arbitrary selected bench shirt can substitute");s.apply_substitutions();require(e.teams[1].shirt==19 && !s.bench_available(false,20),"Outgoing player cannot reenter");
 e.inc_red(3);s.finish_match();t.collect(s);require(t.players[4][2].ban==1,"Direct red creates next-fixture suspension");
 s.start_match(true);require(!e.tournament_conditions && e.teams[1].shirt==2 && s.bench_shirt(false,0)==12,"Exhibition resets one-shot tournament lineup and injury policy");
 // Old checkpoints upgrade without losing fixture results.
 checkpoint(folder/"old.sav","ACTUA96_TOURNAMENT 1\n4 5 0 0 1\n4 14 2 1 0 0 0 0\n");loaded.load(folder/"old.sav");require(loaded.results.size()==1 && loaded.players[4][1].ban==0 && loaded.squad[4][1]==2,"v1 save migration preserves results, initializes missing player history");
 t.save(path);std::ifstream in(path,std::ios::binary);std::string bytes((std::istreambuf_iterator<char>(in)),{});in.close();auto end=bytes.rfind('\n',bytes.size()-2);auto body=bytes.substr(0,end+1);const auto row=body.find("1 2 3 4 5 6 7 8 9 10 11");require(row!=std::string::npos,"Fixture has default squad row");body[row]='2';checkpoint(folder/"duplicate.sav",body);bool rejected=false;try{loaded.load(folder/"duplicate.sav");}catch(...){rejected=true;}require(rejected && loaded.results.size()==1,"Checksummed duplicate squad rejected without changing loaded progress");
 // Real frontend controls, durable selection, keyboard/gamepad and readable capture.
 t.reset(4);t.players[4][2].fitness=19;t.players[4][4].ban=1;t.players[4][6].yellows=1;t.arrange(4);for(int i=1;i<11;++i)require(std::string(original_teams[4].players[t.squad[4][i]-1].position)!="GK","Auto replacement does not field reserve keeper as outfielder");t.save(path);
 OriginalFrontend front(argv[1],argv[2],path);front.open(g);const auto key=[&](SDL_Keycode k){SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=k;front.event(event,g);};
 key(SDLK_L);key(SDLK_RETURN);require(front.stage()==OriginalFrontend::Stage::lineup,"Tournament lineup opens");
 int benchSlot=11;while(std::string(original_teams[4].players[front.tournament().squad[4][benchSlot]-1].position)=="GK")++benchSlot;
 const int chosen=front.tournament().squad[4][benchSlot];
 key(SDLK_DOWN);key(SDLK_SPACE);for(int i=0;i<benchSlot-1;++i)key(SDLK_DOWN);key(SDLK_SPACE);
 loaded.load(path);require(loaded.squad[4][1]==front.tournament().squad[4][1] && loaded.squad[4][1]==chosen,"Keyboard swap immediately persists");
 const auto opponent=front.tournament().squad[14];key(SDLK_RIGHT);key(SDLK_SPACE);key(SDLK_DOWN);key(SDLK_SPACE);require(front.tournament().squad[14]==opponent,"Human cannot edit opponent squad");key(SDLK_LEFT);
 SDL_Event pad{};pad.type=SDL_EVENT_GAMEPAD_BUTTON_DOWN;pad.gbutton.button=SDL_GAMEPAD_BUTTON_DPAD_UP;front.event(pad,g);pad.gbutton.button=SDL_GAMEPAD_BUTTON_NORTH;front.event(pad,g);
 const int padIncoming=front.tournament().squad[4][benchSlot];pad.gbutton.button=SDL_GAMEPAD_BUTTON_DPAD_UP;front.event(pad,g);pad.gbutton.button=SDL_GAMEPAD_BUTTON_NORTH;front.event(pad,g);
 require(front.tournament().squad[4][benchSlot-1]==padIncoming,"Gamepad selects and swaps squad players");
 const int mouseIncoming=front.tournament().squad[4][19];
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());front.menu_fades=false;
 for(int slot:{18,19}){front.draw(renderer,g,false,false);SDL_Event click{};click.type=SDL_EVENT_MOUSE_BUTTON_DOWN;click.button.button=SDL_BUTTON_LEFT;click.button.x=48+550*1.35f;click.button.y=8+(105+slot*18)*1.35f;front.event(click,g);}
 loaded.load(path);require(loaded.squad[4][18]==mouseIncoming,"Mouse selection swaps and persists reserves");
 front.draw(renderer,g,false,false);SDL_RenderPresent(renderer);require(SDL_SaveBMP(surface,(folder/"squad.bmp").string().c_str()),SDL_GetError());
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);
 key(SDLK_RETURN);require(!front.visible() && e.teams[1].shirt==chosen,"Saved lineup starts on pitch");
 std::cout<<"Original fitness and fatigue, live tackle injuries, accumulated cards, served bans, substitution/end mapping, v1 migration, validated v2 saves, squad controls and kickoff passed.\n";
 }
 SDL_Quit();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
