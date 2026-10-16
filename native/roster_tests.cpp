#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "original_roster.hpp"
#include "graphics_data.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <cstdlib>
using namespace actua;
static void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
    require(argc==2,"Data path required");require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
    auto* surface=SDL_CreateSurface(960,700,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);
    require(renderer!=nullptr,SDL_GetError());
    {
    OriginalGraphics g(argv[1]);auto& s=*g.sequence;auto& e=s.engine;g.action_view=true;
    require(std::string(original_teams[4].players[0].name)=="David SEAMAN","Original England roster");
    require(original_teams[4].kit==14 && original_teams[6].kit==-1,"Old Actua kit mapping must not claim German artwork");
    for(int t=0;t<16;++t){
        s.next_home=t;s.next_away=(t+1)%16;s.next_home_formation=t%10;s.next_away_formation=(t+3)%10;s.start_match(true);
        require(s.national_teams,"Regulation match must use original rosters");
        for(int i=0;i<22;++i){auto& p=e.teams[i];const auto& stats=original_teams[i<11?t:(t+1)%16].players[i%11];
            require(p.tm_rate==stats.pace+28 && p.tm_cont==stats.control+28 && p.tm_vis==stats.vision+28,"Original simulation ratings must feed engine");
            require(p.tm_jump==31+(stats.power+28)/10 && std::abs(p.tm_mcspd-float(p.tm_rate+p.tm_flair)/128)<.0001f,"Original jump and animation-speed conversion");
        }
        const auto home_name=s.player_label(1),away_name=s.player_label(12);
        const int h=e.match_tactics1[69][3][0],a=e.match_tactics2[69][3][0];
        for(int i=0;i<80;++i)g.advance(.05);
        for(int camera=0;camera<3;++camera){g.change_camera();g.draw(renderer);require(g.scene_counts.stands==4 && g.scene_counts.home_players==11 && g.scene_counts.away_players==11,"All teams and cameras render full original stadium");}
        s.restart_mode=s.restart_ticks=e.match_mode=e.dead_ball_cnt=e.game_action=e.ball_in_goal=0;e.holder_lose_ball();e.reset_shot();e.match_time.min=45;
        // Isolate the regulation whistle from pending contacts.
        e.ballx=640;e.bally=400;e.ballz=2;e.ball_limbo_on=0;e.just_scored=0;
        for(int i=0;i<400 && !e.match_half;++i)s.step();
        require(e.match_half==1,"Original roster must change ends");
        require(s.player_label(12)==home_name && s.player_label(1)==away_name,"Names follow team at half-time");
        require(e.match_tactics2[69][3][0]==h && e.match_tactics1[69][3][0]==a,"Both formations follow teams after end swap");
        g.draw(renderer);
    }
    // Regression: pitch origin is behind this camera but the visible pitch must remain.
    {legacy::Core camera;camera.camera(900,200,500,1200,0,200,440,640,480);
        require(camera.rot.e.r23<5,"Clipping fixture must put mesh origin behind camera");
        legacy::obj pitch{recovered::pitch_p,recovered::pitch_f,0,0,0,1,0};
        camera.addobjyc(&pitch);require(camera.curpol>camera.polygons.data(),"Pitch must survive origin behind camera");
        for(auto* p=camera.points.data();p<camera.curpts;++p)require(std::isfinite(float(*p)),"Finite clipped pitch coordinates");
    }
    // All formations in full unscripted matches, every Euro team represented.
    for(int n=0;n<10;++n){
        s.next_home=n;s.next_away=(n+8)%16;s.next_home_formation=n;s.next_away_formation=9-n;s.start_match(true);std::srand(n+17);
        int stuck=0;
        for(int i=0;i<25000 && s.running;++i){g.advance(.05);
            require(std::isfinite(e.ballx)&&std::isfinite(e.bally)&&std::isfinite(e.ballz),"Finite national-team match ball");
            for(int p:s.visible_players){const auto v=g.presentation.player(e,p,.5f);for(float f:v.pose)require(std::isfinite(f),"Finite national-team pose");}
            stuck=s.restart_mode?stuck+1:0;require(stuck<1600,"National-team set-piece deadlock");
            if(i%250==0){g.change_camera();g.draw(renderer);const auto cam=g.camera_pose();require(std::isfinite(cam.x)&&std::isfinite(cam.y)&&std::isfinite(cam.z),"Finite match camera");}
        }
        std::cout<<s.team_label(false)<<' '<<s.blue_goals<<':'<<s.red_goals<<' '<<s.team_label(true)<<" formations "<<n<<'/'<<9-n<<" ticks "<<s.ticks<<'\n';
        require(s.full_time && e.match_half==1,"National-team match must finish both halves");
    }
    s.next_home=4;s.next_away=12;s.start_match(true);s.next_home=5;require(s.home_team==4,"Setup edits cannot change current roster");s.restart_play();require(s.home_team==5,"Rematch applies pending roster");
    s.next_home=s.next_away;bool invalid=false;try{s.start_match();}catch(const std::invalid_argument&){invalid=true;}require(invalid,"Reject identical teams");
    }
    SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();
    std::cout<<"16 national teams, ten formations, all camera modes and full stadium verified.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';SDL_Quit();return 1;}}
