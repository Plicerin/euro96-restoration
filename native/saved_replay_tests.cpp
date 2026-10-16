#include "original_frontend.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
using namespace actua;
namespace fs=std::filesystem;
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
static void put(const fs::path& p,const Bytes& b){std::ofstream f(p,std::ios::binary);f.write(reinterpret_cast<const char*>(b.data()),b.size());require(bool(f),"Test file write");}
static void word(Bytes& b,int at,std::uint32_t n){for(int i=0;i<4;++i)b.at(at+i)=static_cast<unsigned char>(n>>(i*8));}
static void hash(Bytes& b){std::uint32_t h=2166136261;for(std::size_t i=0;i<b.size()-4;++i)h=(h^b[i])*16777619u;word(b,int(b.size())-4,h);}
static std::uint32_t pixels(SDL_Surface* s){std::uint32_t h=2166136261;auto* b=static_cast<unsigned char*>(s->pixels);for(int i=0;i<s->pitch*s->h;++i)h=(h^b[i])*16777619u;return h;}
int main(int argc,char** argv){try{
 require(argc==4,"Expected data, media and scratch directories");require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
 // SDL ticks restart with each process and can collide with prior test runs.
 // Claim a fresh directory instead of silently inheriting an old replay library.
 const fs::path root=argv[3];fs::create_directories(root);fs::path folder;
 const auto prefix=std::to_string(SDL_GetTicksNS());
 for(unsigned suffix=0;;++suffix){folder=root/(prefix+"-"+std::to_string(suffix));if(fs::create_directory(folder))break;}
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 {
 OriginalGraphics g(argv[1]);g.load_cd_graphics(argv[2]);g.action_view=true;g.walkoffs_enabled=false;auto* live=g.sequence.get();
 live->start_match(true);for(int i=0;i<450;++i)g.advance(.05);g.paused=true;g.match_camera=1;require(g.start_replay(),"Record replay");g.seek_replay(110);
 const auto path=g.save_replay(folder/"replays");const auto second=g.save_replay(folder/"replays");require(path!=second && read_file(path)==read_file(second),"Unique immutable saves retain exact visual bytes");
 const auto clip=SavedReplay::load(path);clip.validate(&live->engine);require(clip.frames.size()==300 && clip.home==live->home_team && clip.away==live->away_team,"Metadata and original 15-second ring persist");
 auto render=[&]{SDL_SetRenderDrawColor(renderer,0,0,0,255);SDL_RenderClear(renderer);g.draw(renderer,{48,4,864,648});SDL_RenderPresent(renderer);return pixels(surface);};
 const auto expected=render();require(SDL_SavePNG(surface,(folder/"original-replay.png").string().c_str()),SDL_GetError());std::array<std::uint32_t,5> camera_pixels{};
 for(int camera=0;camera<5;++camera){g.match_camera=camera;g.seek_replay(110);camera_pixels[camera]=render();}
 g.match_camera=1;g.seek_replay(110);g.toggle_replay_pause();g.advance(.025);const auto fractional=render();g.finish_replay();
 const auto tick=live->ticks;const auto history=g.replay_frames();std::vector<unsigned char> engine(sizeof(live->engine));std::memcpy(engine.data(),&live->engine,engine.size());
 g.open_saved_replay(path);require(g.sequence.get()!=live && g.saved_replay_active(),"Saved replay owns separate scene");g.seek_replay(110);
 require(render()==expected,"Saved visual fields reproduce original replay pixels");require(SDL_SavePNG(surface,(folder/"saved-replay.png").string().c_str()),SDL_GetError());
 for(int i=0;i<5;++i){g.match_camera=i;g.seek_replay(110);require(render()==camera_pixels[i],"All five saved cameras reproduce original pixels");}
 g.match_camera=1;g.seek_replay(110);g.toggle_replay_pause();g.advance(.025);require(render()==fractional,"Saved replay retains subframe player and camera interpolation");g.toggle_replay_pause();
 g.change_replay_speed(-4);g.toggle_replay_loop();g.toggle_replay_pause();for(int i=0;i<12;++i)g.advance(.1);
 require(live->ticks==tick && !std::memcmp(engine.data(),&live->engine,engine.size()),"Live engine remains byte-identical during saved playback");g.finish_replay();
 require(g.sequence.get()==live && g.paused && g.match_camera==1 && g.replay_frames()==history,"Exact live scene, pause, camera and ring return");require(g.start_replay(),"Original match recording still available");g.finish_replay();
 std::cout<<"1. Immutable unique files, visual roundtrip, five cameras and live-state retention passed.\n";

 // Field/index failures are checked even when a corruptor recomputes the checksum.
 const auto good=read_file(path);int invalid=0;
 const auto reject=[&](Bytes b){const auto p=folder/("invalid-"+std::to_string(invalid++)+".a96r");put(p,b);bool rejected=false;try{g.open_saved_replay(p);}catch(const std::exception&){rejected=true;}
  require(rejected && g.sequence.get()==live && !g.replaying && g.replay_frames()==history,"Invalid file rejected without scene mutation");};
 {auto b=good;b.resize(20);reject(b);}
 {auto b=good;b.push_back(0);reject(b);}
 {auto b=good;b[100]^=1;reject(b);}
 for(auto [offset,value]:std::vector<std::pair<int,std::uint32_t>>{{4,9},{8,16},{32,301},{56,0},{60,21},{72,255},{80,0x7fc00000},{56+22*52+3*32+28,23},{56+22*52+3*32+44,4}}){auto b=good;word(b,offset,value);hash(b);reject(b);}
 {auto b=good;b.resize(1024*1024+1);reject(b);}
 require(!std::memcmp(engine.data(),&live->engine,engine.size()),"Rejected loads retain live engine");
 std::cout<<"2. Truncation, size/version/checksum, identities, capture indices, NaNs and ball bounds rejected safely.\n";

 put(folder/"blocked",Bytes{1});require(g.start_replay(),"Open save failure fixture");bool failed=false;try{g.save_replay(folder/"blocked"/"replays");}catch(const std::exception&){failed=true;}
 require(failed && g.replaying && read_file(path)==good,"Write failure keeps existing clips and viewer");g.finish_replay();
 {OriginalGraphics fresh(argv[1]);fresh.load_cd_graphics(argv[2]);fresh.open_saved_replay(path);fresh.seek_replay(200);fresh.draw(renderer);fresh.finish_replay();require(!fresh.action_view,"Fresh application opens and restores title scene");}
 // Different halves, substitute shirts, missing sent-off player and venue survive reload.
 auto changed=clip;changed.home=0;changed.away=3;changed.stadium=4;changed.wind_on=1;changed.wind_speed=50.5f;changed.wind_x=.6f;changed.wind_y=.8f;
 for(int shirt=16;shirt<=20;++shirt)for(int slot:{1,2,12,13})for(bool mirrored:{false,true})require(OriginalGraphics::numbered_player_faces(slot,mirrored,shirt)[0]==12,"Missing reserve number preserves the original body faces");
 for(auto& f:changed.frames){f.half=1;f.players[2].shirt=16;f.players[4].guy_on=0;f.players[0].shirt=12;}
 const auto other=changed.save_new(folder/"replays");const int venue=g.stadium();g.open_saved_replay(other);g.seek_replay(3);render();require(g.sequence->engine.wind_speed==50.5f && g.sequence->engine.wind_on && g.stadium()==4 && g.scene_counts.keepers==2 && g.scene_counts.home_players+g.scene_counts.away_players==21,"Venue, keeper, substitute and red-card scene reload");g.finish_replay();require(g.stadium()==venue && g.sequence.get()==live,"Venue and match restored");
 std::cout<<"3. Write failure, fresh application, second-half shirts/red cards/keepers and venue restoration passed.\n";

 OriginalFrontend front(argv[1],argv[2],folder/"tournament.sav");front.open(g);
 // Each intentional navigation step sees its destination before the next
 // input. Buffered input against an undrawn page is tested separately.
 auto paint=[&]{front.draw(renderer,g,false,false);for(int i=0;i<9 && front.visible();++i){front.observe(g,.1);front.draw(renderer,g,false,false);}};
 auto key=[&](SDL_Keycode k){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;front.event(e,g);paint();};
 auto button=[&](SDL_GamepadButton b){SDL_Event e{};e.type=SDL_EVENT_GAMEPAD_BUTTON_DOWN;e.gbutton.button=b;front.event(e,g);paint();};
 key(SDLK_F10);require(front.stage()==OriginalFrontend::Stage::replays && front.replays().size()==3,"Title library finds only completed clips");
 front.draw(renderer,g,false,false);SDL_RenderPresent(renderer);require(SDL_SavePNG(surface,(folder/"library.png").string().c_str()),SDL_GetError());
 button(SDL_GAMEPAD_BUTTON_SOUTH);require(g.saved_replay_active() && !front.visible(),"Controller opens saved clip");g.finish_replay();front.observe(g,.05);require(front.stage()==OriginalFrontend::Stage::replays,"Playback returns to library");
 button(SDL_GAMEPAD_BUTTON_BACK);require(front.stage()==OriginalFrontend::Stage::title && g.sequence.get()==live,"Library returns to originating title");
 key(SDLK_RETURN);key(SDLK_RETURN);g.prematches_enabled=false;key(SDLK_RETURN);g.paused=true;const auto liveTick=g.sequence->ticks;
 button(SDL_GAMEPAD_BUTTON_EAST);require(front.stage()==OriginalFrontend::Stage::replays,"Paused match library controller shortcut");key(SDLK_RETURN);g.finish_replay();front.observe(g,.05);key(SDLK_ESCAPE);require(front.stage()==OriginalFrontend::Stage::hidden && g.paused && g.sequence->ticks==liveTick,"Paused match return never restarts or advances game");
 std::cout<<"4. Title and paused-match libraries, controller/keyboard navigation and safe return passed.\n";

 put(folder/"replays"/"zzz-damaged.a96r",Bytes{1,2});put(folder/"replays"/"ignored.a96r.tmp",Bytes{1});
 key(SDLK_F10);key(SDLK_UP);key(SDLK_PAGEUP);require(!front.replays().front().valid,"Damaged entry labelled");key(SDLK_RETURN);require(front.stage()==OriginalFrontend::Stage::replays && !front.replay_status().empty() && !g.replaying,"Damaged selection reports error without leaving library");key(SDLK_DOWN);key(SDLK_RETURN);require(g.replaying,"Valid clip still plays beside damaged file");g.finish_replay();front.observe(g,.05);key(SDLK_ESCAPE);
 OriginalFrontend empty(argv[1],argv[2],folder/"empty"/"tournament.sav");empty.open(g);SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=SDLK_F10;empty.event(event,g);require(empty.replays().empty(),"Missing library is an empty state");event.key.key=SDLK_RETURN;empty.event(event,g);require(!g.replaying,"Empty library confirm is safe");
 std::cout<<"5. Empty, damaged, partial and mixed replay libraries passed. Captures: "<<folder.string()<<'\n';
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
