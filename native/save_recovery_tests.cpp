#include "original_frontend.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::string read(const std::filesystem::path& path){std::ifstream f(path,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
static void damage(const std::filesystem::path& path){std::ofstream f(path,std::ios::binary|std::ios::trunc);f<<"interrupted save";}
int main(int argc,char** argv){try{
 require(argc==4,"Expected source, media and isolated test folder");require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
 const std::filesystem::path root=argv[3];std::filesystem::create_directories(root);const auto path=root/"recovery.sav";auto backup=path;backup+=".bak";
 OriginalTournament t;t.reset(-1);t.record({4,14,2,1});t.save(path);const auto good=read(path);
 const auto next=t.next();t.record({next[0],next[1],1,0});t.save(path);require(read(backup)==good,"Valid previous checkpoint retained");
 damage(path);t.save(path);require(read(backup)==good,"Repairing damaged primary must preserve good backup");
 OriginalTournament loaded;loaded.load(path);require(loaded.results.size()==2,"Repair writes current valid state");
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 {
 OriginalGraphics game(argv[1]);damage(path);
 OriginalFrontend front(argv[1],argv[2],path);front.open(game);
 // Render the destination between deliberate inputs; queued unseen-page input
 // is exercised separately by menu_transition_tests.
 const auto paint=[&]{front.draw(renderer,game,false,false);for(int i=0;i<9 && front.menu_fading_out();++i){front.observe(game,.1);front.draw(renderer,game,false,false);}};
 const auto key=[&](SDL_Keycode k){paint();SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;front.event(e,game);};
 key(SDLK_L);require(front.stage()==OriginalFrontend::Stage::tournament && front.tournament().results.size()==1,"Resume recovers previous valid checkpoint");
 require(read(path)=="interrupted save" && read(backup)==good,"Loading backup does not overwrite either file");
 front.draw(renderer,game,false,false);require(SDL_RenderPresent(renderer),SDL_GetError());require(SDL_SavePNG(surface,(root/"recovered.png").string().c_str()),SDL_GetError());
 key(SDLK_S);loaded.load(path);require(loaded.results.size()==1 && read(backup)==good,"Explicit save of recovered progress preserves backup");
 // A missing primary is recoverable too. Move only this isolated fixture file.
 std::filesystem::rename(path,root/"held.sav");
 OriginalFrontend missing(argv[1],argv[2],path);missing.open(game);SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=SDLK_L;missing.event(e,game);
 require(missing.stage()==OriginalFrontend::Stage::tournament && missing.tournament().results.size()==1,"Missing primary recovers backup");
 std::filesystem::rename(root/"held.sav",path);
 damage(path);damage(backup);key(SDLK_ESCAPE);key(SDLK_L);
 require(!front.tournament_error().empty() && front.tournament().results.size()==1 && front.stage()==OriginalFrontend::Stage::title,"Two invalid files leave loaded state unchanged and report error");
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);
 SDL_Quit();std::cout<<"Damaged/missing primary recovery, valid-backup preservation, non-mutating resume and double-failure checks passed.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
