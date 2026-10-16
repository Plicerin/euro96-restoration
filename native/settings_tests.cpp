#include "original_frontend.hpp"
#include "original_audio.hpp"
#include "original_commentary.hpp"
#include "action_constants.hpp"
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <numeric>
#include <stdexcept>
using namespace actua;
using namespace actua::original::action_constants;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static double energy(const std::vector<float>& v){double sum=0;for(float x:v){require(std::isfinite(x),"Finite PCM");sum+=std::abs(x);}return sum;}
static void signed_file(const std::filesystem::path& path,const std::string& body){std::uint64_t hash=1469598103934665603ull;for(unsigned char c:body){hash^=c;hash*=1099511628211ull;}std::ofstream out(path,std::ios::binary);out<<body<<hash<<'\n';}
int main(int argc,char** argv){try{
 require(argc==4,"Expected source, media and scratch");require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO),SDL_GetError());
 const std::filesystem::path folder=argv[3];std::filesystem::create_directories(folder);
 {
 OriginalGraphics g(argv[1]);auto& s=*g.sequence;auto& e=s.engine;
 float drift[4]{};
 for(int wind=0;wind<4;++wind){
  s.next_options={wind,true,true,true,96,96};s.start_match(true);
  require(s.match_options==s.next_options && e.wind_speed==wind*30 && bool(e.wind_on)==bool(wind),"Original wind strength copied at kickoff");
  e.ball_inair=1;e.ballz=40;e.ballxdis=5;e.ballydis=0;e.ball_friction();drift[wind]=e.ballxdis;
  require(e.ballydis==0,"Original initialization has fixed +X wind");
 }
 require(drift[0]<drift[1] && drift[1]<drift[2] && drift[2]<drift[3],"Each wind strength changes original ball physics");
 require(std::abs((drift[3]-drift[0])-3*(drift[1]-drift[0]))<.00001f,"Original strength scaling");
 // Referee vision changes the original linesman's decision at the same seed/position.
 for(int accuracy:{0,100}){
  s.next_options={0,true,true,true,accuracy,96};s.start_match(true);auto& p=e.teams[2];
  p.tm_x=890;p.tm_y=0;p.tm_off=-1;e.defense_b=880;e.refs[2].x=890;e.refs[2].y=816;e.seed=30;e.ball_released=40;
  e.init_offside(&p);require(p.tm_off==(accuracy?1:-1),"Referee accuracy affects original visibility decision");
 }
 // Strictness changes whether a flagged attacker counts as interfering.
 for(int strictness:{0,100}){
  s.next_options={0,true,true,true,96,strictness};s.start_match(true);s.restart_mode=s.restart_ticks=e.match_mode=0;e.holder_lose_ball();
  auto& p=e.teams[2];p.tm_x=1040;p.tm_y=400;p.tm_dist=30;p.tm_off=1;e.defense_b=880;e.can_be_offside=1;e.ball_released=0;e.last_touch=2;e.ballx=1030;e.bally=400;
  e.offside_rule(&p);require(e.offside_events==int(strictness!=0),"Referee strictness affects interference threshold");
 }
 for(bool enabled:{false,true}){
  s.next_options={0,true,enabled,enabled,96,96};s.start_match(true);e.man_down=4;e.teams[1].tm_x=640;e.teams[1].tm_y=400;e.init_foul(2,true,true);
  require(bool(e.rule_events)==enabled,"Foul switch reaches original foul gate");
  s.start_match(true);e.seed=0;e.send_off_plr(2,100);require(bool(e.card_events)==enabled,"Bookings switch reaches original discipline gate");
 }
 // Real matches at each wind strength, with independent rule variations.
 for(int wind=0;wind<4;++wind){
  s.next_options={wind,wind!=1,wind!=2,wind!=3,96,96};s.start_match(true);bool second=false;
  for(int i=0;i<60000 && s.running;++i){s.step();second|=e.match_half==1;
   require(std::isfinite(e.ballx) && std::isfinite(e.bally) && std::isfinite(e.ballz),"Finite wind match");
   for(const auto& p:e.teams)require(std::isfinite(p.tm_x) && std::isfinite(p.tm_y),"Finite players");
   require(e.wind_speed==wind*30,"Wind remains fixed when teams change ends");
  }
  require(s.full_time && second,"Wind/rules CPU match completes both halves");
  if(wind==1)require(e.offside_events==0,"Offside disabled through live scheduler");
  if(wind==2)require(e.foul_events==0 && e.offside_events==0,"Original fouls-off gate suppresses both types of call");
  if(wind==3)require(e.card_events==0,"No cards with bookings disabled");
  std::cout<<"Wind "<<wind<<": "<<s.blue_goals<<':'<<s.red_goals<<" fouls="<<e.foul_events<<" offsides="<<e.offside_events<<" cards="<<e.card_events<<'\n';
 }
 MatchPreferences prefs;prefs.match={3,false,true,false,42,76};prefs.music=25;prefs.effects=50;prefs.crowd=75;prefs.commentary=0;prefs.muted=true;
 const auto path=folder/"ui.prefs";prefs.save(path);MatchPreferences loaded;loaded.load(path);require(loaded==prefs,"Preferences roundtrip");
 auto revised=prefs;revised.music=50;revised.save(path);loaded.load(path.string()+".bak");require(loaded==prefs,"Previous preferences backed up");
 signed_file(folder/"invalid.prefs","ACTUA96_PREFERENCES 1\n4 1 1 1 96 96\n100 100 100 75 0\n");
 bool rejected=false;try{loaded.load(folder/"invalid.prefs");}catch(...){rejected=true;}require(rejected && loaded==prefs,"Invalid signed values rejected atomically");
 {std::ofstream broken(path,std::ios::binary);broken<<"truncated";}
 OriginalFrontend recovered(argv[1],argv[2],folder/"ui.sav",true);require(recovered.preferences()==prefs,"Frontend recovers previous valid preferences");
 // Saving after recovery must not replace the good backup with the corrupt file.
 recovered.audio_shortcut(true);loaded.load(path.string()+".bak");require(loaded==prefs,"Recovery preserves valid backup");
 // Seed a separate tournament save and exercise both input paths without modifying it.
 OriginalTournament tournament;tournament.reset(14);const auto save=folder/"controls.sav";tournament.save(save);
 std::ifstream beforeFile(save,std::ios::binary);const std::string before((std::istreambuf_iterator<char>(beforeFile)),{});beforeFile.close();
 MatchPreferences defaults;auto controlPrefs=save;controlPrefs.replace_extension(".prefs");defaults.save(controlPrefs);
 OriginalFrontend front(argv[1],argv[2],save,true);front.open(g);using Stage=OriginalFrontend::Stage;
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 // Render the destination between deliberate inputs; queued unseen-page input
 // is exercised separately by menu_transition_tests.
 const auto paint=[&]{front.draw(renderer,g,false,false);for(int i=0;i<9 && front.menu_fading_out();++i){front.observe(g,.1);front.draw(renderer,g,false,false);}};
 const auto key=[&](SDL_Keycode k){paint();SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=k;require(front.event(event,g),"Options input consumed");};
 const auto click=[&](float x,float y){paint();SDL_Event event{};event.type=SDL_EVENT_MOUSE_BUTTON_DOWN;event.button.button=SDL_BUTTON_LEFT;event.button.x=48+x*1.35f;event.button.y=8+y*1.35f;front.event(event,g);};
 const auto pad=[&](int button){paint();SDL_Event event{};event.type=SDL_EVENT_GAMEPAD_BUTTON_DOWN;event.gbutton.button=Uint8(button);front.event(event,g);};
 pad(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);require(front.stage()==Stage::options,"Gamepad opens options");key(SDLK_RIGHT);require(s.next_options.wind==1,"Wind change pending for next kickoff");
 key(SDLK_DOWN);key(SDLK_LEFT);key(SDLK_DOWN);key(SDLK_DOWN);key(SDLK_LEFT);key(SDLK_DOWN);key(SDLK_LEFT);key(SDLK_DOWN);key(SDLK_LEFT);
 require(!front.preferences().match.offside && !front.preferences().match.bookings && front.preferences().match.referee_accuracy==86 && front.preferences().match.referee_strictness==86,"Keyboard edits rule and referee settings");
 const auto capture=[&](const char* name){front.draw(renderer,g,false,front.preferences().muted,front.preferences().commentary);SDL_RenderPresent(renderer);require(SDL_SaveBMP(surface,(folder/(std::string(name)+".bmp")).string().c_str()),SDL_GetError());};
 capture("match-options");pad(SDL_GAMEPAD_BUTTON_NORTH);key(SDLK_LEFT);click(300,174);click(300,213);click(300,252);capture("audio-options");
 require(front.preferences().music==75 && front.preferences().effects==75 && front.preferences().crowd==75 && front.preferences().commentary==50,"Mouse and keyboard audio changes");
 click(480,295);require(front.preferences().muted,"Mouse master mute");front.audio_shortcut(true);front.audio_shortcut(false);
 loaded.load(controlPrefs);require(!loaded.muted && loaded.commentary==25,"F8/F9 persist without losing channel levels");
 key(SDLK_RETURN);key(SDLK_RETURN);key(SDLK_Y);key(SDLK_RETURN);require(s.match_options==loaded.match,"Exhibition kickoff gets pending settings");
 const auto active=s.match_options;const auto frozen=e.teams;const auto tick=s.ticks;const auto generation=s.generation;
 front.setup(g);key(SDLK_O);key(SDLK_RIGHT);require(s.match_options==active && s.next_options.wind==2 && s.ticks==tick && s.generation==generation && !std::memcmp(frozen.data(),e.teams.data(),sizeof(frozen)),"Mid-match settings edit never changes current match or players");
 key(SDLK_RETURN);key(SDLK_ESCAPE);require(!front.visible() && !g.paused,"Settings back returns to original pause state");
 // Half-time options hold the spectator countdown and resume the same match.
 s.interval_ticks=100;s.interval_minute=45;s.cpu_vs_cpu=true;front.observe(g,0);key(SDLK_O);for(int i=0;i<80;++i)front.observe(g,.25);
 require(front.stage()==Stage::options && g.paused && s.interval_ticks==100,"CPU interval timer held in options");key(SDLK_RETURN);key(SDLK_RETURN);s.step();require(e.match_half==1 && s.match_options==active,"Half-time options preserve active settings after end swap");
 OriginalFrontend reopened(argv[1],argv[2],save,true);reopened.open(g);require(reopened.preferences().match.wind==2 && s.next_options.wind==2,"App restart reloads pending match preferences");
 SDL_Event load{};load.type=SDL_EVENT_KEY_DOWN;load.key.key=SDLK_L;reopened.event(load,g);load.key.key=SDLK_RETURN;reopened.event(load,g);reopened.event(load,g);require(s.match_options.wind==2,"Tournament kickoff receives preferences");
 std::ifstream afterFile(save,std::ios::binary);const std::string after((std::istreambuf_iterator<char>(afterFile)),{});require(before==after,"Settings and same-state kickoff preserve tournament checkpoint contents");
 // Failed persistence is visible while the session remains usable.
 const auto blocker=folder/"not-a-directory";{std::ofstream f(blocker);f<<"file";}
 OriginalFrontend blocked(argv[1],argv[2],blocker/"tournament.sav",true);blocked.open(g);load.key.key=SDLK_O;blocked.event(load,g);load.key.key=SDLK_RIGHT;blocked.event(load,g);require(blocked.preferences().match.wind==1 && blocked.preferences_status().find("SAVE FAILED")!=std::string::npos && !std::filesystem::exists(blocker/"tournament.prefs"),"Save failure retains usable session settings");
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);
 // PCM comparisons verify independent gain, including in-flight effects and crowd ambience.
 if(std::filesystem::exists(std::filesystem::path(argv[2])/"DIGI/PITCH/PITCH.MAD") && std::filesystem::exists(std::filesystem::path(argv[2])/"DIGI/MUSIC/MENU.WAV")){
 std::vector<float> full(96000),half(96000);CameraPose camera{};
 for(bool crowd:{false,true}){
  OriginalAudio a(argv[2],false),b(argv[2],false);a.set_levels(crowd?0:100,crowd?100:0);b.set_levels(crowd?0:50,crowd?50:0);
  if(crowd){a.crowd(5);b.crowd(5);}else{a.pitch(5,640,400,camera);b.pitch(5,640,400,camera);}
  a.mix(full.data(),48000);b.mix(half.data(),48000);require(energy(full)>.01 && std::abs(energy(half)/energy(full)-.5)<.00001,"Independent pitch/crowd PCM level");
  a.set_levels(0,0);if(crowd)a.crowd(5);else a.pitch(5,640,400,camera);a.mix(full.data(),48000);require(energy(full)==0,"Channel zero produces silence");
 }
 {s.start_match(true);OriginalAudio a(argv[2],false),b(argv[2],false);a.set_levels(0,100);b.set_levels(0,50);a.update(s,camera,true);b.update(s,camera,true);a.mix(full.data(),48000);b.mix(half.data(),48000);require(energy(full)>.01 && std::abs(energy(half)/energy(full)-.5)<.00001,"Crowd ambience follows crowd level");}
 OriginalMusic a(argv[2],false),b(argv[2],false);a.select(OriginalMusic::Track::menu);b.select(OriginalMusic::Track::menu);b.set_volume(50);a.mix(full.data(),48000);b.mix(half.data(),48000);require(energy(full)>.01 && std::abs(energy(half)/energy(full)-.5)<.00001,"Music PCM level");
 a.set_volume(0);const auto cursor=a.cursor();a.mix(full.data(),48000);require(energy(full)==0 && a.cursor()!=cursor,"Music mute is silent while playback position advances");
 }else std::cout<<"PCM checks skipped: optional CD audio missing.\n";
 std::cout<<"Settings persistence/recovery, rule and wind behavior, referee decisions, controls, deferred match changes, independent audio PCM and four CPU matches passed.\n";
 }
 SDL_Quit();return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
