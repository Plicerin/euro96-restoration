#include "frame_task.hpp"
#include "original_graphics.hpp"
#include "original_actions.hpp"
#include <iostream>
#include <stdexcept>
using namespace actua;
static void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
static FrameTask lifetime(int& steps,int& destroyed,bool fail){
    struct Guard{int& count;~Guard(){++count;}} guard{destroyed};
    ++steps;co_await std::suspend_always{};
    ++steps;if(fail)throw std::runtime_error("Expected frame failure");
    co_return 7;
}
int main(int argc,char** argv){try{
    int steps=0,destroyed=0;
    {auto task=lifetime(steps,destroyed,false);require(!steps,"Task starts only when scheduled");require(task.step() && steps==1 && destroyed==0,"Frame locals survive suspension");require(!task.step() && steps==2 && destroyed==1 && task.result()==7,"Exit releases locals and preserves return code");}
    {auto task=lifetime(steps,destroyed,false);task.step();}require(destroyed==2,"Abandoned frame releases locals");
    {auto task=lifetime(steps,destroyed,true);task.step();require(!task.step(),"Exception ends task");bool caught=false;try{task.result();}catch(const std::runtime_error&){caught=true;}require(caught && destroyed==3,"Frame exception propagates with cleanup");}
    require(argc>=3,"Source and media paths required");
    Archive archive(std::filesystem::path(argv[1])/"EUROREND.DAT",std::filesystem::path(argv[1])/"EUROREND.OFF");
    OriginalEnvironment env(archive);legacy::Core core;MatchPalette palette{};
    for(int i=0;i<256;++i)palette[i]={static_cast<unsigned char>(i%64),static_cast<unsigned char>(i/4),0};
    core.camera(640.25f,260,800.25f,640.25f,0,400.25f,440,640,480);env.render(core,palette);
    const auto classic=env.pixels();
    for(const auto size:{std::pair{1280,720},std::pair{1920,1080},std::pair{2520,1080},std::pair{640,480}}){
        const auto [w,h]=size;core.camera(640.25f,260,800.25f,640.25f,0,400.25f,440*h/480.0f,w,h);env.render(core,palette);
        require(env.width()==w && env.height()==h && env.pixels().size()==std::size_t(w*h),"Environment follows projection dimensions");
        require(env.pixels()[(h/2)*w+w/2]==classic[240*640+320],"Resolution/aspect preserve camera target");
        require(env.ground_pixels<=unsigned(w*h) && env.sky_pixels<=unsigned(w*h),"Environment raster remains bounded");
    }
    require(env.pixels()==classic,"Returning to classic exactly restores original ground projection");
    require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
    auto* surface=SDL_CreateSurface(1920,1080,SDL_PIXELFORMAT_RGBA32);require(surface,SDL_GetError());
    auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
    {
        OriginalGraphics graphics(argv[1]);graphics.load_cd_graphics(argv[2]);graphics.action_view=true;
        graphics.sequence->start_match(true);graphics.presentation_height=1080;
        for(int camera=0;camera<5;++camera){graphics.match_camera=camera;for(int tick=0;tick<10;++tick)graphics.advance(.05);graphics.draw(renderer,{0,0,1920,1080});require(graphics.polygon_count()>100 && graphics.scene_counts.home_players==11 && graphics.scene_counts.away_players==11,"HD camera retains both complete teams");}
        if(argc>3){std::filesystem::create_directories(argv[3]);require(SDL_SaveBMP(surface,(std::filesystem::path(argv[3])/"wide1080.bmp").string().c_str()),SDL_GetError());}
    }
    SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();
    std::cout<<"Frame lifetime, exception cleanup, 4 display sizes, classic round-trip and 5 HD cameras passed.\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<"\n";return 1;}}
