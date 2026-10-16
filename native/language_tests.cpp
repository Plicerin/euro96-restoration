#include "original_commentary.hpp"
#include "original_frontend.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace actua;
static void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
static void signed_file(const std::filesystem::path& path,const std::string& body){
 std::uint64_t h=1469598103934665603ull;for(unsigned char c:body){h^=c;h*=1099511628211ull;}
 std::ofstream f(path,std::ios::binary);f<<body<<h<<'\n';
}
// Hash the un-ramped portion of the original U8 PCM recovered from the mixer.
// Expected values were read independently from the CD MADs, not from the packs.
static std::uint64_t pcm_hash(OriginalCommentary& voice){
 std::vector<float> out(22050);voice.mix(out.data(),11025);std::uint64_t h=1469598103934665603ull;
 for(int i=55;i<5000;++i){require(out[2*i]==out[2*i+1] && std::isfinite(out[2*i]),"Finite mono commentary");h^=static_cast<unsigned char>(std::lround(out[2*i]*128)+128);h*=1099511628211ull;}return h;
}
int main(int argc,char** argv){try{
 require(argc==4,"Expected source, media, scratch");require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO),SDL_GetError());
 const std::filesystem::path folder=argv[3],media=argv[2];std::filesystem::create_directories(folder);
 {
 OriginalGraphics g(argv[1]);auto& s=*g.sequence;auto& e=s.engine;s.start_match(true);s.ticks=10;
 OriginalCommentary voice(media,false);voice.set_volume(100);voice.update(s,0,true);
 for(int language:{0,1,0,1}){
  const auto seed=e.seed;const auto tick=s.ticks;const auto x=e.ballx;
  require(voice.set_language(language) && voice.language()==language,"Language swap succeeds");
  voice.update(s,0,true);voice.score(2,1);voice.update(s,0,true);
  require(voice.category()==44,"Score selects exact record");
  // Independently read from retail E_SCO.MAD/e_sco21[a-d].wav.
  constexpr std::uint64_t retail_score[]={10837981196595183013ull,13908444828897383067ull,7394948417291621856ull,3325182179221601748ull};
  const bool retail=std::filesystem::exists(media/"COMMENTARY/EURO_EVENTS.BIN");
  require(pcm_hash(voice)==(language?17709157884234379779ull:retail?retail_score[voice.history().back().variant]:3069458697871301755ull),"Score PCM matches selected language on original CD");
  require(e.seed==seed && s.ticks==tick && e.ballx==x,"Language changes do not advance gameplay or consume its RNG");
 }
 // Changing while a clip is active discards both current and queued old speech.
 voice.request(43);voice.update(s,0,true);require(voice.speaking(),"Active period speech");
 voice.set_language(0);require(!voice.speaking() && !voice.pending(),"No old-language speech survives switch");
 for(int language:{0,1}){
  require(voice.set_language(language),"Name language switch");voice.update(s,0,true);e.PlayTeamSample(9,2);voice.update(s,0,true);
  require(voice.category()==216 && voice.history().back().number==3,"Original name callback selects Shearer and intonation");
  // Euro96 E_ENG.MAD/e_eng09z.wav has a different original take from Actua.
  const bool euro=std::filesystem::exists(media/"COMMENTARY/EURO_NAMES.BIN");
  require(pcm_hash(voice)==(language?1279415740754936303ull:euro?1190461055352382394ull:3481462811507089274ull),"Player-name PCM matches original CD language and revision");
 }
 voice.set_volume(0);require(voice.set_language(0) && voice.volume()==0,"Language switch preserves independent mute");voice.update(s,0,true);voice.score(2,1);require(!voice.speaking() && !voice.pending(),"Muted switch stays silent");
 require(!voice.set_language(-1) && !voice.set_language(2) && voice.language()==0,"Unsupported language rejected");
 OriginalCommentary device(media);device.update(s,0,true);device.request(22);device.update(s,.05,true);require(device.ready() && device.set_language(1) && device.ready() && !device.speaking(),"Live SDL stream survives switch without queued old voice");
 device.update(s,0,false);device.update(s,0,true);require(!device.speaking(),"Focus return cannot resume discarded speech");
 // Missing/corrupt optional packs never destroy the working voice.
 for(const char* name:{"missing","corrupt"}){
  const auto root=folder/name;std::filesystem::create_directories(root/"COMMENTARY");
  std::filesystem::copy_file(media/"COMMENTARY/ENGLISH3.BIN",root/"COMMENTARY/ENGLISH3.BIN",std::filesystem::copy_options::overwrite_existing);
  if(std::string(name)=="corrupt"){std::ofstream bad(root/"COMMENTARY/FRENCH.BIN",std::ios::binary);bad<<"AC96COM1broken-data";}
  OriginalCommentary fallback(root,false);fallback.update(s,0,true);fallback.score(2,1);fallback.update(s,0,true);
  require(!fallback.set_language(1) && fallback.language()==0 && fallback.category()==44,"Unavailable French pack keeps working English voice intact");
 }
 const auto pref=folder/"language.prefs";
 signed_file(pref,"ACTUA96_PREFERENCES 1\n2 0 1 1 86 76\n25 50 75 100 1\n");
 MatchPreferences old;old.load(pref);require(old.commentary_language==0 && old.music==25 && old.muted && old.match.wind==2,"V1 preferences migrate to English without losing values");
 auto next=old;next.commentary_language=1;next.save(pref);MatchPreferences loaded;loaded.load(pref);require(loaded==next,"French preference persists");loaded.load(pref.string()+".bak");require(loaded==old,"Legacy backup still readable");
 signed_file(folder/"bad.prefs","ACTUA96_PREFERENCES 2\n0 1 1 1 96 96\n100 100 100 75 0 2\n");
 bool rejected=false;try{loaded.load(folder/"bad.prefs");}catch(...){rejected=true;}require(rejected && loaded==old,"Unknown saved language rejected atomically");
 const auto save=folder/"ui.sav";auto uiPref=save;uiPref.replace_extension(".prefs");MatchPreferences{}.save(uiPref);
 OriginalFrontend front(argv[1],media,save,true);front.open(g);
 const auto key=[&](SDL_Keycode k){SDL_Event v{};v.type=SDL_EVENT_KEY_DOWN;v.key.key=k;front.event(v,g);};
 const auto pad=[&](Uint8 button){SDL_Event v{};v.type=SDL_EVENT_GAMEPAD_BUTTON_DOWN;v.gbutton.button=button;front.event(v,g);};
 key(SDLK_O);key(SDLK_Y);key(SDLK_UP);key(SDLK_RIGHT);require(front.preferences().commentary_language==1,"Keyboard selects French");
 pad(SDL_GAMEPAD_BUTTON_DPAD_LEFT);require(front.preferences().commentary_language==0,"Gamepad selects English");
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 front.menu_fades=false;front.draw(renderer,g,false,false);
 SDL_Event click{};click.type=SDL_EVENT_MOUSE_BUTTON_DOWN;click.button.button=SDL_BUTTON_LEFT;click.button.x=48+500*1.35f;click.button.y=8+331*1.35f;front.event(click,g);
 require(front.preferences().commentary_language==1,"Mouse selects French");
 front.draw(renderer,g,false,false);SDL_RenderPresent(renderer);require(SDL_SaveBMP(surface,(folder/"french-options.bmp").string().c_str()),SDL_GetError());SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);
 OriginalFrontend reopened(argv[1],media,save,true);require(reopened.preferences().commentary_language==1,"Frontend restores saved language");
 OriginalFrontend absent(argv[1],folder/"missing",folder/"absent.sav",false);absent.open(g);
 for(SDL_Keycode k:{SDLK_O,SDLK_Y,SDLK_UP,SDLK_RIGHT}){SDL_Event v{};v.type=SDL_EVENT_KEY_DOWN;v.key.key=k;absent.event(v,g);}
 require(absent.preferences().commentary_language==0 && absent.preferences_status().find("UNAVAILABLE")!=std::string::npos,"Missing French option explains availability");
 }
 SDL_Quit();std::cout<<"Original French/English PCM, switching, focus/mute, missing/corrupt packs, preferences migration and keyboard/gamepad/mouse language controls passed.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
