#include "archive.hpp"
#include "frame_task.hpp"
#include "model.hpp"
#include "ball.hpp"
#include "player.hpp"
#include "gamepad.hpp"
#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "original_audio.hpp"
#include "original_commentary.hpp"
#include "original_frontend.hpp"
#include "original_menu_overlays.hpp"
#include "original_movie.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <charconv>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace fs = std::filesystem;
using namespace actua;
static int presentation_height=0;
#ifdef __EMSCRIPTEN__
extern "C" EMSCRIPTEN_KEEPALIVE void set_presentation(int height){presentation_height=height==1080?1080:height==720?720:0;}
#endif

static void check(bool ok, const char* operation) {
    if(!ok) throw std::runtime_error(std::string(operation)+": "+SDL_GetError());
}
struct Options {
    fs::path data, capture, audio_data;
    std::string hud_capture,frontend_capture;
    bool replay_file_smoke=false,head_to_head_smoke=false;
    bool startup_smoke=false;
    bool front_end=false,frontend_smoke=false,tournament_save_smoke=false,movie_smoke=false,intro_smoke=false;
    bool goals_smoke=false,goal_review=false;
    bool original_play=false,control_smoke=false;
    bool team_play=false,team_smoke=false,cpu_vs_cpu=false,cpu_smoke=false;
    bool actions_smoke=false,action_scene=false,stadium=false,original3d=false,graphics_smoke=false,result_capture=false, verify=false, smoke=false, tactics=false, physics=false, physics_smoke=false, practice=false, practice_smoke=false, opponents=false, opponents_smoke=false, gamepad_smoke=false;
    bool setpiece_smoke=false,audio_smoke=false,commentary_smoke=false;
    bool setup=false,setup_smoke=false,depth_smoke=false,penalty_smoke=false;
    int home=4,away=12,camera=0,format=0;
    int screen=0,match_minutes=5,match_ticks=80;
    std::optional<unsigned int> fixed_match_seed;
    bool random_match_seed=false;
};
static fs::path find_data() {
    const auto base=fs::path(std::u8string(reinterpret_cast<const char8_t*>(SDL_GetBasePath())));
    for(const auto& candidate : {base/"data",base/"SOURCE",base/"../SOURCE",
                                 base/"../../SOURCE",base/"../../../SOURCE",fs::path("SOURCE")})
        if(fs::exists(candidate/"EURO.OFF")) return fs::weakly_canonical(candidate);
    throw std::runtime_error("Game data not found. Keep the data folder beside actua96.exe or use --data PATH.");
}
static Options options(int argc, char** argv) {
    Options result;
    for(int i=1;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg=="--wide720")presentation_height=720;
        else if(arg=="--wide1080")presentation_height=1080;
        else if(arg=="--front-end")result.original3d=result.front_end=true;
        else if(arg=="--head-to-head-smoke")result.original3d=result.front_end=result.frontend_smoke=result.head_to_head_smoke=true;
        else if(arg=="--replay-file-smoke")result.original3d=result.front_end=result.frontend_smoke=result.replay_file_smoke=true;
        else if(arg=="--startup-smoke")result.original3d=result.front_end=result.frontend_smoke=result.startup_smoke=true;
        else if(arg=="--frontend-smoke")result.original3d=result.front_end=result.frontend_smoke=true;
        else if(arg=="--intro-smoke")result.original3d=result.front_end=result.intro_smoke=true;
        else if(arg=="--movie-smoke")result.original3d=result.front_end=result.movie_smoke=true;
        else if(arg=="--tournament-save-smoke")result.tournament_save_smoke=true;
        else if(arg=="--frontend-capture"){
            if(++i>=argc)throw std::runtime_error("Missing front-end capture state");
            result.frontend_capture=argv[i];result.original3d=result.front_end=true;
            if(result.frontend_capture!="title" && result.frontend_capture!="selection" && result.frontend_capture!="lineup" && result.frontend_capture!="interval" && result.frontend_capture!="result" && result.frontend_capture!="movie" && result.frontend_capture!="intro")throw std::runtime_error("Invalid front-end capture state");
        }
        else if(arg=="--hud-capture"){
            if(++i>=argc)throw std::runtime_error("Missing HUD capture state");
            result.hud_capture=argv[i];result.original3d=result.team_play=true;
            if(result.hud_capture!="attack" && result.hud_capture!="defend" && result.hud_capture!="keeper" && result.hud_capture!="watch" && result.hud_capture!="paused" && result.hud_capture!="charge" && result.hud_capture!="replay" && result.hud_capture!="panels")throw std::runtime_error("HUD state: attack, defend, keeper, watch, paused, charge, replay, panels");
        }
        else if(arg=="--commentary-smoke"){result.original3d=result.front_end=result.frontend_smoke=result.commentary_smoke=true;}
        else if(arg=="--audio-smoke"){result.original3d=result.team_play=result.team_smoke=result.audio_smoke=true;}
        else if(arg=="--setpiece-smoke"){result.original3d=result.team_play=result.penalty_smoke=result.setpiece_smoke=true;}
        else if(arg=="--penalty-smoke"){result.original3d=result.team_play=result.penalty_smoke=true;result.format=3;}
        else if(arg=="--depth-smoke") result.original3d=result.team_play=result.cpu_vs_cpu=result.team_smoke=result.depth_smoke=true;
        else if(arg=="--setup-smoke") result.original3d=result.team_smoke=result.cpu_smoke=result.setup_smoke=true;
        else if(arg=="--match-setup") result.original3d=result.setup=true;
        else if(arg=="--cpu-smoke") result.original3d=result.team_smoke=result.cpu_smoke=true;
        else if(arg=="--cpu-teams") result.original3d=result.team_play=result.cpu_vs_cpu=true;
        else if(arg=="--team-smoke") result.original3d=result.team_play=result.team_smoke=true;
        else if(arg=="--original-teams") result.original3d=result.team_play=true;
        else if(arg=="--control-smoke") result.original3d=result.control_smoke=true;
        else if(arg=="--original-play") result.original3d=result.original_play=true;
        else if(arg=="--goals-smoke") result.original3d=result.goals_smoke=true;
        else if(arg=="--goal-review") result.original3d=result.goal_review=true;
        else if(arg=="--actions-smoke") result.original3d=result.action_scene=result.actions_smoke=true;
        else if(arg=="--actions") result.original3d=result.action_scene=true;
        else if(arg=="--original-3d") result.original3d=true;
        else if(arg=="--stadium") result.original3d=result.stadium=true;
        else if(arg=="--graphics-smoke") result.original3d=result.graphics_smoke=true;
        else if(arg=="--verify") result.verify=true;
        else if(arg=="--match-seed"){
            if(++i>=argc)throw std::runtime_error("Missing match seed: unsigned integer or random");
            const std::string value=argv[i];
            result.random_match_seed=value=="random";result.fixed_match_seed.reset();
            if(!result.random_match_seed){
                unsigned int seed=0;const auto parsed=std::from_chars(value.data(),value.data()+value.size(),seed);
                if(parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size())throw std::runtime_error("Match seed must be 0 through 4294967295, or random");
                result.fixed_match_seed=seed;
            }
        }
        else if(arg=="--smoke-test") result.smoke=true;
        else if(arg=="--tactics") result.tactics=true;
        else if(arg=="--gamepad-smoke") {result.practice=true;result.gamepad_smoke=true;}
        else if(arg=="--opponents") { result.practice=true;result.opponents=true; }
        else if(arg=="--opponents-smoke") { result.practice=true;result.opponents=true;result.opponents_smoke=true; }
        else if(arg=="--practice") result.practice=true;
        else if(arg=="--practice-smoke") { result.practice=true; result.practice_smoke=true; }
        else if(arg=="--physics") result.physics=true;
        else if(arg=="--physics-smoke") { result.physics=true; result.physics_smoke=true; }
        else if(arg=="--match-format"){
            if(++i>=argc)throw std::runtime_error("Missing match format");
            const std::string value=argv[i];if(value=="regulation")result.format=0;else if(value=="knockout")result.format=1;else if(value=="golden")result.format=2;else if(value=="penalties")result.format=3;else throw std::runtime_error("Match format: regulation, knockout, golden or penalties");
        }
        else if(arg=="--audio-data" || arg=="--data" || arg=="--capture" || arg=="--result-capture" || arg=="--screen" || arg=="--match-minutes" || arg=="--match-ticks" || arg=="--home-team" || arg=="--away-team" || arg=="--camera") {
            if(++i>=argc) throw std::runtime_error("Missing value for "+arg);
            if(arg=="--audio-data") result.audio_data=fs::path(std::u8string(reinterpret_cast<const char8_t*>(argv[i])));
            else if(arg=="--data") result.data=fs::path(std::u8string(reinterpret_cast<const char8_t*>(argv[i])));
            else if(arg=="--capture" || arg=="--result-capture") {
                result.capture=fs::path(std::u8string(reinterpret_cast<const char8_t*>(argv[i])));
                if(arg=="--result-capture")result.result_capture=result.practice=result.opponents=true;
            }
            else if(arg=="--home-team" || arg=="--away-team" || arg=="--camera"){
                std::size_t used=0;int value=std::stoi(argv[i],&used);
                if(used!=std::string(argv[i]).size() || value<0 || value>=(arg=="--camera"?3:16))throw std::runtime_error("Invalid team/camera index");
                if(arg=="--home-team")result.home=value;else if(arg=="--away-team")result.away=value;else result.camera=value;
            }
            else if(arg=="--match-minutes" || arg=="--match-ticks"){
                std::size_t used=0;const int value=std::stoi(argv[i],&used);
                if(used!=std::string(argv[i]).size())throw std::runtime_error("Invalid match option");
                if(arg=="--match-minutes"){
                    if(value!=5 && value!=10 && value!=20 && value!=40 && value!=90)throw std::runtime_error("Match minutes: 5, 10, 20, 40 or 90");
                    result.match_minutes=value;
                }else{if(value<0 || value>500000)throw std::runtime_error("Match ticks: 0 through 500000");result.match_ticks=value;}
            }else {
                std::size_t used=0; result.screen=std::stoi(argv[i],&used);
                if(used!=std::string(argv[i]).size() || result.screen<0 || result.screen>14)
                    throw std::runtime_error("Screen must be 0 through 14");
            }
        } else throw std::runtime_error("Unknown option: "+arg);
    }
    if((!result.hud_capture.empty() || !result.frontend_capture.empty()) && result.capture.empty())throw std::runtime_error("HUD review requires --capture PATH");
    if(result.home==result.away)throw std::runtime_error("Choose two different teams");
    if(result.data.empty()) result.data=find_data();
    return result;
}
static void verify(const fs::path& root, const Archive& euro) {
    for(std::size_t i=0;i<15;++i) (void)decode_screen(euro,i);
    for(std::size_t i=0;i<10;++i) (void)decode_tactics(euro.entry(111+i));
    std::cout << "EURO: " << euro.count() << " entries, 15 backgrounds, 10 tactics tables verified.\n";
    for(const auto* stem : {"EUROREND","FAP","FAPF"}) {
        const Archive a(root/(std::string(stem)+".DAT"),root/(std::string(stem)+".OFF"));
        std::cout << stem << ": " << a.count() << " bounded entries, " << a.size() << " bytes.\n";
    }
}
static void color(SDL_Renderer* r, Uint8 red, Uint8 green, Uint8 blue) {
    check(SDL_SetRenderDrawColor(r,red,green,blue,255),"Set draw color");
}
static void text(SDL_Renderer* r, float x,float y,const std::string& value) {
    check(SDL_RenderDebugText(r,x,y,value.c_str()),"Draw text");
}
static void large_text(SDL_Renderer* r,float x,float y,const std::string& value,float size=1.5f){
    float sx,sy;check(SDL_GetRenderScale(r,&sx,&sy),"Read text scale");
    check(SDL_SetRenderScale(r,sx*size,sy*size),"Enlarge text");
    text(r,x/size,y/size,value);check(SDL_SetRenderScale(r,sx,sy),"Restore text scale");
}
static void rect(SDL_Renderer* r,float x,float y,float w,float h,bool fill=true) {
    const SDL_FRect area{x,y,w,h};
    check(fill?SDL_RenderFillRect(r,&area):SDL_RenderRect(r,&area),"Draw rectangle");
}
// Native presentation only: retain original match state and input bindings.
static void hud_text(SDL_Renderer* r,float x,float y,const std::string& value,float width,float size=1.5f){
    const float fitted=value.empty()?size:std::min(size,width/(8.0f*float(value.size())));
    large_text(r,x,y,value,fitted);
}
static void hud_control(SDL_Renderer* r,float y,const std::string& key,const std::string& action){
    color(r,30,47,57);rect(r,696,y-9,62,32);
    color(r,134,222,150);hud_text(r,703,y,key,48,1.25f);
    color(r,238,242,247);hud_text(r,770,y,action,166,1.25f);
}
static void pause_reference(SDL_Renderer* r,const OriginalGraphics& g,bool pad){draw_menu_pause(r,g,pad);}
static void live_power(SDL_Renderer* r,const OriginalGraphics& graphics){
    auto owner_scope=graphics.sequence->input_scope(graphics.sequence->head_to_head && graphics.sequence->engine.ball_poss>0 && !graphics.sequence->is_blue(graphics.sequence->engine.ball_poss-1)?1:0);
    const auto& s=*graphics.sequence;const auto& e=s.engine;
    if(graphics.paused || graphics.replaying || e.ball_poss!=s.selected+1 || e.teams[s.selected].tm_act>1 || s.live_charge()<=0)return;
    draw_menu_power(r,graphics,std::clamp(s.live_charge()/30.0f,0.0f,1.0f));
}
static void match_sidebar(SDL_Renderer* r,const OriginalGraphics& graphics,bool pad){
    const auto& s=*graphics.sequence;const auto& e=s.engine;
    if(s.team_play){live_power(r,graphics);return;}
    const bool keeper=s.human_keeper_hold(),owns=e.ball_poss==s.selected+1;
    const bool home_ball=e.ball_poss>0 && s.is_controlled_team(e.ball_poss-1);
    const std::string mode=graphics.replaying?"REPLAY":graphics.paused?"PAUSED":s.full_time?"FULL TIME":s.interval_ticks?s.period_label():s.head_to_head?"LOCAL HEAD TO HEAD":s.cpu_vs_cpu?"CPU v CPU":s.penalty_human_keeper()?"SAVE THE PENALTY":s.shootout?"PENALTIES":keeper?"KEEPER BALL":s.restart_mode?"RESTART":home_ball?"ATTACKING":e.ball_poss?"DEFENDING":"LOOSE BALL";
    color(r,134,222,150);hud_text(r,696,130,mode,240);
    color(r,238,242,247);hud_text(r,696,174,s.team_label(false)+" "+std::to_string(s.shootout?s.penalty_scores[0]:s.blue_goals)+" - "+std::to_string(s.shootout?s.penalty_scores[1]:s.red_goals)+" "+s.team_label(true),240,1.75f);
    color(r,173,189,202);hud_text(r,696,211,s.shootout?"PENALTY SHOOTOUT":s.full_match?s.clock_label():"PRACTICE",240,1.25f);
    color(r,46,64,72);rect(r,696,242,240,1);
    color(r,173,189,202);large_text(r,696,266,s.full_time?"RESULT":s.cpu_vs_cpu?"ON THE BALL":s.head_to_head?(s.input_player()?"P2 PLAYER":"P1 PLAYER"):"YOUR PLAYER",1.25f);
    color(r,238,242,247);hud_text(r,696,294,s.full_time?s.period_label():s.player_label(s.cpu_vs_cpu?e.ball_poss-1:s.selected),240,1.25f);
    if(graphics.replaying){
        hud_control(r,350,pad?"South":"Space","SKIP REPLAY");
        hud_control(r,400,pad?"Start":"P","PAUSE / RESUME");
        if(!pad)hud_control(r,450,"V","CAMERA");
    }else if(graphics.paused || s.cpu_vs_cpu || s.full_time){
        int passes[2]{s.sub_passes[0],s.sub_passes[1]},shots[2]{s.sub_shots[0],s.sub_shots[1]};
        for(int i:s.visible_players){passes[s.is_blue(i)?0:1]+=e.teams[i].tm_pass;shots[s.is_blue(i)?0:1]+=e.teams[i].tm_shots;}
        const auto stat=[&](float y,const std::string& label,int a,int b){color(r,173,189,202);large_text(r,696,y,label,1.25f);color(r,238,242,247);hud_text(r,852,y,std::to_string(a)+" : "+std::to_string(b),84,1.25f);};
        if(s.shootout){stat(350,"TAKEN",int(s.penalty_results[0].size()),int(s.penalty_results[1].size()));stat(396,"SCORED",s.penalty_scores[0],s.penalty_scores[1]);}
        else{stat(350,"PASSES",passes[0],passes[1]);stat(396,"SHOTS",shots[0],shots[1]);}
        stat(442,"SAVES",e.teams[s.blue_slot()].tm_kpsaves,e.teams[11-s.blue_slot()].tm_kpsaves);
    }else if(s.penalty_human_keeper()){
        hud_control(r,350,pad?"Stick":"Up/Dn","CHOOSE SIDE");
        hud_control(r,400,pad?"West":"X","DIVE / SAVE");
        color(r,173,189,202);hud_text(r,696,452,"Hold direction + shot",240,1.25f);
    }else if(s.shootout){
        color(r,173,189,202);large_text(r,696,350,s.penalty_released?"Ball in play...":"Next attempt...",1.25f);
    }else if(keeper || owns){
        hud_control(r,350,pad?"South":"Space","PASS");
        hud_control(r,396,pad?"North":"C","LOFT");
        hud_control(r,442,pad?"West":"X",keeper?"PUNT":"SHOOT");
    }else if(!e.ball_poss){
        hud_control(r,350,pad?"West":"X","FIRST TIME");
        hud_control(r,396,pad?"South":"Space","CONTROL");
        hud_control(r,442,pad?"East":"E","SWITCH PLAYER");
    }else{
        hud_control(r,350,pad?"West":"X","TACKLE");
        hud_control(r,396,pad?"East":"E","SWITCH PLAYER");
        hud_control(r,442,pad?"South":"Space","STEAL");
    }
    if(s.full_time && !graphics.replaying){
        hud_control(r,510,pad?"R3":"R","NEW MATCH");
        hud_control(r,556,pad?"L3":"F2","MATCH SETUP");
    }else if(!graphics.replaying){
        hud_control(r,510,pad?"Start":"P",graphics.paused?"RESUME":"PAUSE / HELP");
        if(s.team_play && !s.head_to_head){if(pad && !s.cpu_vs_cpu)hud_control(r,556,"LB/RB","SWITCH PLAYER");else hud_control(r,556,pad?"RB":"B",s.cpu_vs_cpu?"TAKE CONTROL":"WATCH CPU");}
    }
    live_power(r,graphics);
    color(r,173,189,202);hud_text(r,696,596,graphics.camera_name(),240,1.0f);
}
static constexpr float replay_bar_left=200,replay_bar_width=640,replay_bar_top=552;
static void replay_controls(SDL_Renderer* r,const OriginalGraphics& g,bool pad){draw_menu_replay(r,g,pad);}
static void circle(SDL_Renderer* r,float x,float y,float radius,bool fill) {
    if(fill) {
        for(int yoff=-int(radius);yoff<=int(radius);++yoff) {
            float width=std::sqrt(std::max(0.0f,radius*radius-float(yoff*yoff)));
            check(SDL_RenderLine(r,x-width,y+float(yoff),x+width,y+float(yoff)),"Draw circle");
        }
    } else {
        for(int i=0;i<64;++i) {
            float a=float(i)*6.2831853f/64, b=float(i+1)*6.2831853f/64;
            check(SDL_RenderLine(r,x+std::cos(a)*radius,y+std::sin(a)*radius,
                                x+std::cos(b)*radius,y+std::sin(b)*radius),"Draw circle");
        }
    }
}
static void button(SDL_Renderer* r,float x,float y,float w,const char* title) {
    color(r,32,46,63); rect(r,x,y,w,30);
    color(r,223,235,244); text(r,x+12,y+11,title);
}
static std::string number(float value) {
    char buffer[40]; SDL_snprintf(buffer,sizeof(buffer),"%.1f",double(value)); return buffer;
}
static void draw_physics(SDL_Renderer* r,const BallSandbox& lab) {
    const auto& b=lab.engine;
    color(r,143,157,174); text(r,24,119,"ORIGINAL BALL ROUTINES / 20 STEPS PER SECOND");
    text(r,696,119,"PHYSICS SANDBOX");
    for(int stripe=0;stripe<8;++stripe) {
        color(r,Uint8(24+stripe%2*3),Uint8(78+stripe%2*7),49);
        rect(r,24+float(stripe)*80,156,80,400);
    }
    color(r,154,192,161); rect(r,24,156,640,400,false);
    check(SDL_RenderLine(r,344,156,344,556),"Draw halfway line");circle(r,344,356,48,false);
    rect(r,24,242,86,228,false); rect(r,578,242,86,228,false);
    rect(r,24,304,29,104,false); rect(r,635,304,29,104,false);
    const float goal_half=b.prat*2;
    rect(r,14,356-goal_half,10,goal_half*2,false); rect(r,664,356-goal_half,10,goal_half*2,false);
    circle(r,344,356,3,true);
    color(r,87,149,104);
    for(std::size_t i=1;i<lab.trail_size;++i) {
        const auto& a=lab.trail[i-1]; const auto& c=lab.trail[i];
        check(SDL_RenderLine(r,24+a.x*0.5f,156+a.y*0.5f,24+c.x*0.5f,156+c.y*0.5f),"Draw flight trail");
    }
    const float angle=lab.aim*3.1415926536f/180;
    color(r,244,205,105);
    check(SDL_RenderLine(r,344,356,344+std::cos(angle)*65,356-std::sin(angle)*65),"Draw aim");
    const float sx=std::clamp(24+b.ballx*0.5f,16.0f,672.0f),sy=std::clamp(156+b.bally*0.5f,150.0f,562.0f);
    const float by=std::max(137.0f,sy-(b.ballz-2)*0.45f);
    color(r,12,34,24);circle(r,sx,sy,5,true);
    color(r,112,161,121);check(SDL_RenderLine(r,sx,sy,sx,by),"Draw ball height");
    color(r,242,240,217);circle(r,sx,by,6,true);
    color(r,39,47,44);circle(r,sx-1,by-1,2,true);
    color(r,134,222,150); text(r,696,151,lab.preset_name());
    color(r,222,232,241);
    text(r,696,178,"Power  "+number(lab.power)+"    Aim "+number(lab.aim));
    text(r,696,202,"Curve  "+std::string(lab.curve==0?"off":(lab.curve>0?"right":"left"))+"    Wind "+(b.wind_on?"on":"off"));
    text(r,696,240,"Height  "+number(b.ballz-2));
    text(r,696,264,"Speed   "+number(std::sqrt(b.ballxdis*b.ballxdis+b.ballydis*b.ballydis)));
    text(r,696,288,"Bounces "+std::to_string(b.bounce_events));
    text(r,696,312,"Time    "+number(float(lab.ticks)/20)+" sec");
    color(r,244,205,105);
    const char* status=lab.paused?"PAUSED":(lab.outside?"OUTSIDE PITCH - RESET":(b.ball_still?"READY":"BALL IN MOTION"));
    text(r,696,346,status);
    button(r,696,382,232,"SPACE  KICK FROM CENTRE");
    button(r,696,422,112,"R  RESET");button(r,816,422,112,lab.paused?"P  RESUME":"P  PAUSE");
    color(r,143,157,174);
    text(r,696,475,"Arrows: aim / power"); text(r,696,497,"V: shot type   C: curve");
    text(r,696,519,"W: wind   IJKL: aftertouch");text(r,696,541,"TAB: view   F11: fullscreen");
    color(r,211,232,218);text(r,24,578,"Click the pitch to aim, then press SPACE. IJKL bends an uncurved airborne shot.");
    color(r,32,46,63);rect(r,24,612,912,1);
    color(r,143,157,174);text(r,24,632,"Engine checkpoint: original ball physics. Players, goals and match rules are not connected yet.");
}
static void draw_practice(SDL_Renderer* r,const PlayerPractice& practice,const GamepadInput& pad,bool pad_prompts) {
    const auto& b=practice.engine;
    color(r,143,157,174);text(r,24,119,practice.opponents?"BLUE 2,3: YOUR TEAM  /  RED 13,14: OPPONENTS":"BLUE 2,3: TEAMMATES / GOLD RING: SELECTED PLAYER");
    text(r,696,119,practice.opponents?"BLUE ATTACKS RIGHT ->":"TWO-PLAYER PASSING DRILL");
    for(int stripe=0;stripe<8;++stripe) {color(r,Uint8(24+stripe%2*3),Uint8(78+stripe%2*7),49);rect(r,24+float(stripe)*80,156,80,400);}
    color(r,154,192,161);rect(r,24,156,640,400,false);
    check(SDL_RenderLine(r,344,156,344,556),"Draw halfway line");circle(r,344,356,48,false);
    rect(r,24,242,86,228,false);rect(r,578,242,86,228,false);
    rect(r,24,304,29,104,false);rect(r,635,304,29,104,false);
    color(r,107,185,224);rect(r,14,356-PlayerPractice::goal_half_width/2,10,PlayerPractice::goal_half_width,false);
    color(r,235,105,102);rect(r,664,356-PlayerPractice::goal_half_width/2,10,PlayerPractice::goal_half_width,false);
    color(r,96,151,105);
    for(std::size_t i=1;i<practice.trail_size;++i) {
        const auto& a=practice.trail[i-1];const auto& c=practice.trail[i];
        check(SDL_RenderLine(r,24+a.x/2,156+a.y/2,24+c.x/2,156+c.y/2),"Draw pass trail");
    }
    for(int index:PlayerPractice::practice_players) {
        if(index>=12 && !practice.opponents) continue;
        const auto& p=b.teams[std::size_t(index)];float x=24+p.tm_x/2,y=156+p.tm_y/2;
        color(r,11,33,25);circle(r,x+2,y+3,11,true);
        if(index==practice.selected) {
            color(r,251,213,105);circle(r,x,y,15,false);
            if(practice.tackling()) {color(r,174,241,255);circle(r,x,y,20,false);}
        }
        if(index>=12 && practice.opponent_windup[std::size_t(index-12)]>0) {
            color(r,255,168,68);circle(r,x,y,18+float(practice.opponent_windup[std::size_t(index-12)]),false);
        }
        if(b.ball_poss==p.tm_player) {color(r,247,248,234);circle(r,x,y,12,false);}
        if(index>=12) color(r,235,105,102);else color(r,107,185,224);
        circle(r,x,y,10,true);color(r,15,27,32);text(r,x-(p.tm_player>=10?8.0f:4.0f),y-4,std::to_string(p.tm_player));
        if(index>=12) color(r,242,145,125);else color(r,169,217,250);check(SDL_RenderLine(r,x+p.tm_xdis*17,y+p.tm_ydis*17,x+p.tm_xdis*34,y+p.tm_ydis*34),"Draw player facing");
    }
    for(int side=0;side<2;++side) {
        const auto& keeper=practice.keepers[std::size_t(side)];if(!keeper.active)continue;
        const float x=24+keeper.x/2,y=156+keeper.y/2;
        color(r,11,33,25);rect(r,x-10,y-10,22,22);
        if(side==0)color(r,91,222,177);else color(r,247,193,91);
        rect(r,x-9,y-11,18,22);color(r,15,27,32);text(r,x-8,y-4,"GK");
        if(practice.keeper_holding==side){color(r,247,248,234);circle(r,x,y,15,false);}
    }
    const float bx=std::clamp(24+b.ballx/2,16.0f,672.0f),sy=std::clamp(156+b.bally/2,150.0f,562.0f);
    const float by=std::max(137.0f,sy-(b.ballz-2)*0.45f);
    color(r,11,33,25);circle(r,bx,sy,4,true);color(r,134,173,138);check(SDL_RenderLine(r,bx,sy,bx,by),"Draw height");
    color(r,252,250,227);circle(r,bx,by,4,true);
    if(practice.opponents) {
        color(r,107,185,224);text(r,696,154,"BLUE "+std::to_string(practice.blue_score));
        color(r,222,232,241);text(r,792,154,"-");
        color(r,235,105,102);text(r,824,154,std::to_string(practice.red_score)+" RED");
        color(r,107,185,224);text(r,696,181,"YOU: BLUE "+std::to_string(practice.selected+1));
        const int seconds=practice.match_seconds_left();
        color(r,seconds<=30?251:222,seconds<=30?213:232,seconds<=30?105:241);
        text(r,880,181,std::to_string(seconds/60)+":"+(seconds%60<10?"0":"")+std::to_string(seconds%60));
        color(r,222,232,241);text(r,696,207,practice.keeper_holding>=0?(practice.keeper_holding==0?"Ball: BLUE KEEPER":"Ball: RED KEEPER"):b.ball_poss?(b.ball_poss<12?"Ball: BLUE ":"Ball: RED ")+std::to_string(b.ball_poss):"Ball: free");
    } else {
        color(r,107,185,224);text(r,696,154,"CONTROLLING BLUE "+std::to_string(practice.selected+1));
        color(r,222,232,241);text(r,696,181,b.ball_poss?"Ball: BLUE "+std::to_string(b.ball_poss):"Ball: free");
        text(r,696,207,"Passes "+std::to_string(b.teams[1].tm_pass+b.teams[2].tm_pass)+"   Catches "+std::to_string(practice.passes_received));
    }
    if(practice.opponents) text(r,696,233,"Regains "+std::to_string(practice.blue_wins)+"   Losses "+std::to_string(practice.red_wins));
    else text(r,696,233,"Shots "+std::to_string(b.teams[1].tm_shots+b.teams[2].tm_shots));
    if(practice.opponents) {color(r,143,157,174);text(r,696,251,"Red passes "+std::to_string(practice.red_completed)+"/"+std::to_string(practice.red_passes)+" complete");}
    color(r,251,213,105);
    const bool warning=practice.opponent_windup[0]>0 || practice.opponent_windup[1]>0;
    if(practice.match_finished) text(r,696,275,"FULL TIME");
    else if(practice.paused) text(r,696,275,"PAUSED");
    else if(practice.keeper_holding>=0) text(r,696,275,"KEEPER RELEASE IN "+number(float(practice.keeper_hold_ticks)/20)+"s");
    else if(practice.restart_ticks) text(r,696,275,std::string(practice.restart_red?"RED RESTART ":"BLUE RESTART ")+number(float(practice.restart_ticks)/20)+"s");
    else text(r,696,275,practice.paused?"PAUSED":(practice.outside?"OUTSIDE PITCH - RESET":(warning?"RED TACKLE - MOVE / PASS!":(b.ball_poss>=12?"WIN THE BALL BACK":(!b.ball_poss && b.last_touch>=12?"CHASE THE LOOSE BALL":"KEEP THE BALL MOVING")))));
    if(practice.opponents){color(r,143,157,174);text(r,696,293,"Shots "+std::to_string(b.teams[1].tm_shots+b.teams[2].tm_shots)+":"+std::to_string(b.teams[12].tm_shots+b.teams[13].tm_shots)+"  Saves "+std::to_string(practice.keepers[0].saves)+":"+std::to_string(practice.keepers[1].saves));}
    button(r,696,310,232,pad_prompts?(pad.label(SDL_GAMEPAD_BUTTON_SOUTH)+"  GROUND PASS").c_str():"SPACE  GROUND PASS");
    button(r,696,350,232,pad_prompts?(pad.label(SDL_GAMEPAD_BUTTON_NORTH)+"  LOFTED PASS").c_str():"C      LOFTED PASS");
    const bool defending=practice.opponents && b.ball_poss!=practice.selected+1;
    const std::string action_label=pad_prompts?(defending?"HOLD ":"")+pad.label(SDL_GAMEPAD_BUTTON_WEST)+(defending?"  TACKLE":"  SHOOT"):(defending?"HOLD X  TACKLE":"X      SHOOT");
    button(r,696,390,232,action_label.c_str());
    const std::string mode_label=std::string(pad_prompts?"BACK ":"O ")+(practice.opponents?"OPPONENTS ON / RESET":"OPPONENTS OFF / RESET");
    button(r,696,430,232,mode_label.c_str());
    color(r,143,157,174);
    if(pad_prompts) {
        text(r,696,484,"LS / D-pad move   RS aim");
        text(r,696,506,pad.label(SDL_GAMEPAD_BUTTON_EAST)+"/LB switch; Start pause");
        text(r,696,528,"RB view; R3 reset (paused)");
    } else {
        text(r,696,484,"Arrows / WASD: move");text(r,696,506,"Click pitch: aim shot");
        text(r,696,528,"E: switch  R: reset  P: pause");
    }
    if(pad.connected()){color(r,134,222,150);text(r,696,550,pad.name());}
    if(practice.match_finished) {
        color(r,16,28,42);rect(r,172,278,344,164);
        color(r,245,219,130);text(r,200,296,"FULL TIME - "+practice.match_result());
        color(r,235,242,248);text(r,200,324,"BLUE "+std::to_string(practice.blue_score)+" - "+std::to_string(practice.red_score)+" RED");
        text(r,200,350,"Shots "+std::to_string(b.teams[1].tm_shots+b.teams[2].tm_shots)+" : "+std::to_string(b.teams[12].tm_shots+b.teams[13].tm_shots)+"   Saves "+std::to_string(practice.keepers[0].saves)+" : "+std::to_string(practice.keepers[1].saves));
        button(r,200,386,288,pad_prompts?"START / OPTIONS: REMATCH":"R / CLICK: REMATCH");
    } else if(practice.restart_ticks) {
        color(r,16,28,42);rect(r,188,308,312,88);
        color(r,245,219,130);text(r,220,326,practice.restart_goal?"GOAL!":"BALL OUT OF PLAY");
        color(r,235,242,248);text(r,220,350,std::string(practice.restart_red?"RED":"BLUE")+(practice.restart_goal?" KICKOFF IN ":" RESTART IN ")+number(float(practice.restart_ticks)/20)+"s");
        text(r,220,374,"BLUE "+std::to_string(practice.blue_score)+" - "+std::to_string(practice.red_score)+" RED");
    }
    color(r,211,232,218);text(r,24,578,pad_prompts?pad.hint(practice.status):practice.status);
    color(r,32,46,63);rect(r,24,612,912,1);
    color(r,143,157,174);
    if(pad_prompts) text(r,24,632,"Right stick aims; "+pad.label(SDL_GAMEPAD_BUTTON_WEST)+" shoots / hold to tackle. Start pauses; R3 resets while paused. RB changes view.");
    else text(r,24,632,practice.opponents?"Attack RIGHT. Click toward goal + X to shoot. Hold X to tackle. Goals and out balls restart automatically.":"Hold direction through the pass to run on reception. O starts the opponent challenge.");
}
struct MatchSetup {
    bool open=false,was_paused=false;int row=0;
    void toggle(OriginalGraphics& g){open=!open;if(open){was_paused=g.paused;g.paused=true;g.sequence->clear_play_input();}else g.paused=was_paused;}
    void change(OriginalGraphics& g,int delta){auto& a=*g.sequence;
        if(row<2)a.change_team(row==1,delta);
        else if(row<4){int& f=row==2?a.next_home_formation:a.next_away_formation;f=(f+delta+10)%10;}
        else if(row==4){const int lengths[]={5,10,20,40,90};for(int i=0;i<5;++i)if(a.match_minutes==lengths[i]){a.set_match_minutes(lengths[(i+delta+5)%5]);break;}}
        else if(row==5)g.change_camera(delta);
        else a.next_format=static_cast<OriginalActions::MatchFormat>((int(a.next_format)+delta+4)%4);
    }
    void start(OriginalGraphics& g,bool cpu){open=false;g.action_view=true;g.overview=false;g.paused=false;g.sequence->next_head_to_head=false;g.sequence->start_match(cpu);}
    void draw(SDL_Renderer* r,const OriginalGraphics& g) const {
        if(open)draw_menu_setup(r,g,row);
    }
};
struct SquadMenu {
    bool open=false,was_paused=false,away=false;int row=0,slot=1,shirt=13;std::string message;
    void toggle(OriginalGraphics& g){
        if(!g.sequence->national_teams || g.replaying)return;
        open=!open;if(open){was_paused=g.paused;g.paused=true;g.sequence->clear_play_input();message.clear();}
        else g.paused=was_paused;
    }
    void change(int delta){if(row==0)away=!away;else if(row==1)slot=(slot+delta+11)%11;else shirt=12+(shirt-12+delta+4)%4;}
    void confirm(OriginalGraphics& g){message=g.sequence->queue_substitution(away,slot,g.sequence->bench_shirt(away,shirt-12))?"QUEUED - CLOSE AND RESUME; CHANGE AT NEXT STOPPAGE":"UNAVAILABLE - CHECK PLAYER, BENCH AND THREE-CHANGE LIMIT";}
    void draw(SDL_Renderer* r,const OriginalGraphics& g)const{if(open)draw_menu_squad(r,g,away,row,slot,shirt,message);}
};
static void draw(SDL_Renderer* r,SDL_Texture* texture,const Navigation& nav,
                 const std::array<Tactics,10>& tactics,const BallSandbox& lab,const PlayerPractice& practice,const GamepadInput& pad,bool pad_prompts,OriginalGraphics& graphics) {
    auto& scene=*graphics.sequence;const int hud_actor=scene.engine.match_mode?scene.engine.setp_taker-1:scene.engine.ball_poss-1;
    auto hud_scope=scene.input_scope(scene.head_to_head && hud_actor>=0 && !scene.is_blue(hud_actor)?1:0);
    color(r,10,17,27); check(SDL_RenderClear(r),"Clear");
    if(nav.page==4 && graphics.clean_match_view()){
        graphics.presentation_height=presentation_height;
        if(presentation_height){
            int width=0,height=0;check(SDL_GetRenderOutputSize(r,&width,&height),"Get display size");
            check(SDL_SetRenderLogicalPresentation(r,0,0,SDL_LOGICAL_PRESENTATION_DISABLED),"Set scene size");
            graphics.draw(r,{0,0,float(width),float(height)});
            check(SDL_SetRenderLogicalPresentation(r,960,664,SDL_LOGICAL_PRESENTATION_LETTERBOX),"Restore menu size");
        }else graphics.draw(r,{48,4,864,648});
        live_power(r,graphics);
        replay_controls(r,graphics,pad_prompts);
        pause_reference(r,graphics,pad_prompts);
        const auto& s=*graphics.sequence;
        if(s.national_teams && (!graphics.has_original_kit(s.home_team) || !graphics.has_original_kit(s.away_team))){color(r,173,189,202);text(r,160,32,"Substitute kit: missing original artwork");}
        if(!graphics.paused && !graphics.prematch.active() && !graphics.walkoff.active() && !graphics.exit.active())draw_menu_hint(r,graphics,graphics.replaying?(pad_prompts?"R3 CAMERA    A RETURN    START PAUSE":"V CAMERA    SPACE RETURN    P PAUSE"):s.human_keeper_hold()?(pad_prompts?"STICK AIM    A PASS    Y LOFT    X PUNT    START HELP":"ARROWS AIM    SPACE PASS    C LOFT    X PUNT    P HELP"):pad_prompts?"START HELP    R3 PANELS    F7 RADAR":"P HELP    F6 PANELS    TAB RADAR    V CAMERA");
        return;
    }

    graphics.presentation_height=0;
    if(nav.page==4 && scene.team_play)draw_menu_match_panels(r,graphics,pad_prompts);
    else {
    color(r,134,222,150); text(r,24,22,"UEFA EURO 96");
    color(r,238,242,247); text(r,24,43,"WINDOWS RESTORATION");
    color(r,143,157,174); text(r,600,30,"NATIVE " ACTUA_VERSION " / ORIGINAL ENGINE");
    button(r,24,68,184,nav.page==0?"[1] ORIGINAL SCREENS":"1  ORIGINAL SCREENS");
    button(r,220,68,184,nav.page==1?"[2] TACTICS TABLES":"2  TACTICS TABLES");
    button(r,416,68,184,nav.page==2?"[3] BALL PHYSICS":"3  BALL PHYSICS");
    button(r,612,68,184,nav.page==3?"[4] PLAYER PRACTICE":"4  PLAYER PRACTICE");
    button(r,804,68,132,nav.page==4?"[5] ORIGINAL 3D":"5 ORIGINAL 3D");
    }
    if(nav.page==4) {
        graphics.draw(r);
        if(graphics.action_view && graphics.sequence->play){
            const auto& s=*graphics.sequence;
            const bool manual_setpiece=!graphics.replaying && (s.human_restart() || (s.penalty_human_taker() && s.engine.match_mode));
            if(manual_setpiece){
                const auto& e=s.engine;const bool penalty=e.set_piece_on==4,throwing=e.set_piece_on==2;
                const std::string shot=pad_prompts?"West":"X",pass=pad_prompts?"South":"Space",loft=pad_prompts?"North":"C";
                color(r,134,222,150);large_text(r,696,130,penalty?"PENALTY":throwing?"THROW-IN":"SET PIECE");
                color(r,238,242,247);large_text(r,696,174,s.team_label(false)+" "+std::to_string(s.shootout?s.penalty_scores[0]:s.blue_goals)+" - "+std::to_string(s.shootout?s.penalty_scores[1]:s.red_goals)+" "+s.team_label(true));
                color(r,143,157,174);text(r,696,209,s.player_label(s.selected));
                color(r,238,242,247);large_text(r,696,266,shot+(throwing?"  THROW":"  HOLD / RELEASE"),1.25f);
                color(r,143,157,174);large_text(r,696,294,throwing?"Aim into the pitch":"Hold to raise height",1.25f);
                if(!penalty){
                    color(r,238,242,247);large_text(r,696,350,pass+"  PASS",1.25f);
                    large_text(r,696,396,loft+"  LOFT",1.25f);
                }else{large_text(r,696,350,"Time the moving aim",1.25f);}
                color(r,143,157,174);large_text(r,696,484,pad_prompts?"Start  PAUSE":"P  PAUSE",1.25f);
                large_text(r,696,530,"F5  NEXT PRACTICE",1.25f);
                if(!throwing){
                    // Compact meters stay next to the pitch; the restored dots supply aim/range.
                    color(r,10,20,30);rect(r,128,502,520,74);
                    const float maximum=float((e.teams[e.setp_taker-1].tm_pow/6)+8);
                    const float power=penalty?1.0f:std::clamp((float(e.setp_power)+16)/(maximum+16),0.0f,1.0f);
                    const float height=std::clamp(float(e.setp_hgt)/30,0.0f,1.0f);
                    color(r,238,242,247);large_text(r,144,514,penalty?"POWER: AUTO":"POWER");large_text(r,404,514,"HEIGHT");
                    color(r,46,64,72);rect(r,144,545,236,18);rect(r,404,545,228,18);
                    color(r,134,222,150);rect(r,144,545,236*power,18);
                    color(r,244,208,91);if(height>0)rect(r,404,545,228*height,18);
                }
                color(r,238,242,247);large_text(r,32,616,graphics.paused?"PAUSED":penalty?"Time the aim. Hold "+shot+", then release.":throwing?"Choose direction, then throw or pass.":"Left/right: AIM     Up/down: POWER",1.5f);
                color(r,143,157,174);text(r,32,646,e.teams[e.setp_taker-1].int_move==15?"Taking kick...":e.dead_ball_cnt>1?"Getting into position...":throwing?"Outward direction selects a higher throw.":"Bright dots: flight path     Dark dots: ground path");
                pause_reference(r,graphics,pad_prompts);
                return;
            }
            match_sidebar(r,graphics,pad_prompts);
            if(s.national_teams && (!graphics.has_original_kit(s.home_team) || !graphics.has_original_kit(s.away_team))){text(r,24,116,"Substitute kit: missing original team artwork");}
            if(!graphics.replaying && (s.full_time || s.interval_ticks)){
                color(r,13,24,34);SDL_FRect panel{100,218,492,270};SDL_RenderFillRect(r,&panel);
                color(r,244,208,91);text(r,140,250,s.period_label());
                color(r,222,232,241);text(r,140,290,s.match_format==OriginalActions::MatchFormat::penalties?"PENALTIES ONLY":s.team_label(false)+" "+std::to_string(s.blue_goals)+"  -  "+std::to_string(s.red_goals)+" "+s.team_label(true));
                if(s.full_time){
                    int passes[2]{s.sub_passes[0],s.sub_passes[1]},attempts[2]{s.sub_shots[0],s.sub_shots[1]};
                    for(int i:s.visible_players){passes[s.is_blue(i)?0:1]+=s.engine.teams[i].tm_pass;attempts[s.is_blue(i)?0:1]+=s.engine.teams[i].tm_shots;}
                    if(s.shootout){passes[0]=s.match_passes[0];passes[1]=s.match_passes[1];attempts[0]=s.match_shots[0];attempts[1]=s.match_shots[1];}
                    text(r,140,330,s.shootout?"Penalties: "+std::to_string(s.penalty_scores[0])+" / "+std::to_string(s.penalty_scores[1]):"Passes: "+std::to_string(passes[0])+" / "+std::to_string(passes[1]));
                    text(r,140,364,s.match_format==OriginalActions::MatchFormat::penalties?"Kicks:  "+std::to_string(s.penalty_results[0].size())+" / "+std::to_string(s.penalty_results[1].size()):"Match shots: "+std::to_string(attempts[0])+" / "+std::to_string(attempts[1]));
                    text(r,140,430,pad_prompts?"R3: new match":"R: new match / Q: replays");
                }else{text(r,140,344,"Teams are changing ends");text(r,140,392,s.interval_minute==90?"Two extra halves of 15 match minutes":"Play resumes automatically");}
            }
            if(s.shootout && !s.full_time){
                color(r,10,20,30);rect(r,24,116,640,62);color(r,244,208,91);
                text(r,36,126,"PENALTIES  "+s.team_label(false)+" "+std::to_string(s.penalty_scores[0])+" : "+std::to_string(s.penalty_scores[1])+" "+s.team_label(true));
                for(int side=0;side<2;++side){std::string kicks=s.team_label(side!=0)+": ";const auto& list=s.penalty_results[side];for(std::size_t i=list.size()>12?list.size()-12:0;i<list.size();++i)kicks+=list[i]?"O ":"X ";text(r,36+side*310,151,kicks);}
            }
            if(graphics.replaying){replay_controls(r,graphics,pad_prompts);return;}
            pause_reference(r,graphics,pad_prompts);
            if(s.team_play)return;
            color(r,211,232,218);hud_text(r,24,620,s.match_text(s.status),912,1.25f);
            color(r,173,189,202);text(r,24,648,s.full_time?(pad_prompts?"R3: new match   L3: match setup":"R: new match   F2: match setup"):graphics.paused?"Match stopped. Resume when ready.":s.cpu_vs_cpu?(pad_prompts?"RB: take control   Start: pause and options":"B: take control   V: camera   P: pause and options"):s.human_keeper_hold()?(pad_prompts?"Left stick: aim delivery":"WASD / arrows: aim delivery"):pad_prompts?"Left stick: move   East / LB / RB: switch   Start: help":"WASD / arrows: move   E: switch   V: camera   P: help");
            return;
        }
        color(r,134,222,150);text(r,696,126,graphics.action_view?(graphics.sequence->goal_review?"ORIGINAL GOAL COLLISIONS":"ORIGINAL KEEPER SEQUENCE"):"ORIGINAL PLAYER MODEL");
        const auto& c=graphics.clips[std::size_t(graphics.clip)];
        color(r,222,232,241);text(r,696,164,graphics.action_view?"Original action timing":c.name);text(r,696,192,graphics.action_view?"Tick "+std::to_string(graphics.sequence->ticks):"Frame "+std::to_string(graphics.frame+1)+" / "+std::to_string(c.frames.size()));
        text(r,696,220,"Kits "+std::to_string(graphics.kit+1)+" / "+std::to_string((graphics.kit+1)%16+1));
        color(r,143,157,174);text(r,696,264,"Left / Right: animation");text(r,696,290,"Up / Down: turn camera");text(r,696,316,"K: next original kit");text(r,696,342,"W: wireframe");text(r,696,368,"P: pause animation");text(r,696,394,"V: stadium / close-up");
        text(r,696,426,"SPACE: attack / outlet");text(r,696,448,"M: pass and control");text(r,696,470,"N: shot beats keeper");
        if(graphics.action_view && graphics.sequence->goal_review){
            text(r,696,498,std::to_string(graphics.sequence->review_stage+1)+" / 5  "+graphics.sequence->goal_review_name(graphics.sequence->review_stage));
            text(r,696,522,graphics.sequence->review_left?"Left goal":"Right goal");
            text(r,696,546,graphics.sequence->engine.goal_events?"Goal detected":"No goal");
        }else if(graphics.action_view){text(r,696,498,graphics.sequence->continuation?"Delivery tick: "+std::to_string(graphics.sequence->distribution_tick):"Release tick: "+std::to_string(graphics.sequence->release_tick));text(r,696,522,"Shots / saves: "+std::to_string(graphics.sequence->shots)+" / "+std::to_string(graphics.sequence->saves));text(r,696,546,"Ball owner: "+std::to_string(graphics.sequence->engine.ball_poss));}
        color(r,134,222,150);text(r,696,572,"T: passing / G,H: goal checks");text(r,696,594,pad_prompts?"Start: play / RB: CPU teams":"Y: play / B: watch CPU teams");
        text(r,696,616,pad_prompts?"L3: match setup":"F2: teams / match setup");
        color(r,211,232,218);text(r,24,624,graphics.action_view?graphics.sequence->status:"SPACE: attack, save and outlet. Left / Right: browse original animations.");
        return;
    }
    if(nav.page==3) { draw_practice(r,practice,pad,pad_prompts); return; }
    if(nav.page==2) { draw_physics(r,lab); return; }
    if(nav.page==0) {
        const SDL_FRect dst{24,116,640,480};
        check(SDL_RenderTexture(r,texture,nullptr,&dst),"Draw original screen");
    } else {
        color(r,24,64,43); rect(r,24,116,640,480);
        // The preserved tables use integer pitch coordinates; no gameplay is simulated here.
        color(r,26,84,49); rect(r,44,156,600,375);
        color(r,158,196,164); rect(r,44,156,600,375,false);
        rect(r,44,250,95,187,false); rect(r,549,250,95,187,false);
        rect(r,44,295,35,97,false); rect(r,609,295,35,97,false);
        check(SDL_RenderLine(r,344,156,344,531),"Halfway line");
        circle(r,344,343.5f,45,false);
        color(r,75,115,81);
        for(int x=1;x<8;++x) check(SDL_RenderLine(r,44+float(x)*75,156,44+float(x)*75,531),"Zone grid");
        for(int y=1;y<4;++y) check(SDL_RenderLine(r,44,156+float(y)*93.75f,644,156+float(y)*93.75f),"Zone grid");
        if(nav.zone<64) {
            int cell=nav.zone%32;
            color(r,44,108,65); rect(r,45+float(cell%8)*75,157+float(cell/8)*93.75f,73,91.75f);
        }
        const auto& zone=tactics[std::size_t(nav.formation)][std::size_t(nav.zone)];
        for(std::size_t i=0;i<zone.size();++i) {
            const float x=44+float(zone[i].x)*600/1280, y=156+float(zone[i].y)*375/800;
            color(r,8,25,21); circle(r,x+2,y+3,12,true);
            color(r,255,220,103); circle(r,x,y,11,true);
            color(r,14,28,23); text(r,x-(i==9?8:4),y-4,std::to_string(i+1));
        }
        color(r,211,232,218); text(r,44,552,"Original outfield-player targets / raw table view");
        text(r,44,573,"Click a grid cell to inspect its formation targets.");
    }
    color(r,222,232,241); text(r,696,122,nav.page==0?"PRESERVED ARTWORK":"PRESERVED TACTICS");
    color(r,134,222,150); text(r,696,152,nav.page==0?screen_names[std::size_t(nav.screen)]:formation_names[std::size_t(nav.formation)]);
    color(r,143,157,174);
    text(r,696,179,nav.page==0?"Screen "+std::to_string(nav.screen+1)+" / 15":"Zone "+std::to_string(nav.zone+1)+" / 70");
    if(nav.page==0) {
        text(r,696,218,"640 x 480 indexed image");
        text(r,696,240,"Original VGA palette");
        text(r,696,262,"Read from EURO.DAT");
        text(r,696,308,"These are backgrounds.");
        text(r,696,330,"Original menu actions");
        text(r,696,352,"are not connected yet.");
    } else {
        text(r,696,218,"10 outfield players");
        text(r,696,240,"70 target sets / formation");
        text(r,696,262,"10 original formations");
        text(r,696,308,"[ / ] Change formation");
        text(r,696,330,"Arrows change zone");
        text(r,696,352,"No match AI is running.");
    }
    button(r,696,412,112,"< PREVIOUS"); button(r,820,412,112,"NEXT >");
    color(r,143,157,174);
    text(r,696,474,"TAB   Switch view");
    text(r,696,496,"F11   Fullscreen");
    text(r,696,518,"ESC   Exit");
    text(r,696,560,"Gamepad: D-pad / shoulders");
    color(r,32,46,63); rect(r,24,612,912,1);
    color(r,143,157,174); text(r,24,632,"Original assets. Native Windows display. Match engine restoration is still in progress.");
}
static FrameTask run_game(std::vector<std::string> arguments) {
    const int argc=int(arguments.size());
    std::vector<char*> pointers;for(auto& arg:arguments)pointers.push_back(arg.data());
    pointers.push_back(nullptr);char** argv=pointers.data();
    // Extra --verify makes version probes noninteractive on older executables too.
    if(argc>=2 && std::string(argv[1])=="--version"){std::cout<<ACTUA_VERSION<<"\n";co_return 0;}
    bool initialized=false;
    bool interactive=true;
    for(int i=1;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg=="--head-to-head-smoke" || arg=="--tournament-save-smoke" || arg=="--commentary-smoke" || arg=="--frontend-smoke" || arg=="--audio-smoke" || arg=="--setpiece-smoke" || arg=="--penalty-smoke" || arg=="--depth-smoke" || arg=="--goals-smoke" || arg=="--control-smoke")interactive=false;
        if(arg=="--startup-smoke" || arg=="--replay-file-smoke" || arg=="--movie-smoke" || arg=="--intro-smoke")interactive=false;
        if(arg=="--setup-smoke" || arg=="--cpu-smoke" || arg=="--team-smoke" || arg=="--verify" || arg=="--smoke-test" || arg=="--physics-smoke" || arg=="--practice-smoke" || arg=="--opponents-smoke" || arg=="--gamepad-smoke" || arg=="--capture" || arg=="--result-capture" || arg=="--graphics-smoke" || arg=="--actions-smoke") interactive=false;
    }
    try {
        const auto opt=options(argc,argv);
        Archive euro(opt.data/"EURO.DAT",opt.data/"EURO.OFF");
        verify(opt.data,euro);
        if(opt.verify) co_return 0;
#ifdef __EMSCRIPTEN__
        SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT,"#canvas");
#endif
        check(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMEPAD),"Initialize SDL");
        initialized=true;
        if(opt.tournament_save_smoke){
            // Exercise this executable's actual production save DIRECTORY, not the
            // permissive build/temp folder. Unique files never replace user progress.
            const auto production=OriginalFrontend::default_save_path();
            const auto probe=production.parent_path()/("save-check-"+std::to_string(SDL_GetPerformanceCounter())+".sav");
            {
                OriginalGraphics game(opt.data);OriginalFrontend menu(opt.data,opt.audio_data,probe,true);menu.open(game);
                const auto key=[&](SDL_Keycode k){SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=k;menu.event(event,game);};
                key(SDLK_O);key(SDLK_RIGHT);key(SDLK_Y);key(SDLK_LEFT);key(SDLK_RETURN);
                key(SDLK_T);key(SDLK_RETURN);
                if(!menu.tournament_error().empty() || !fs::exists(probe))throw std::runtime_error("Production save creation failed: "+menu.tournament_error());
                key(SDLK_Y);
                if(menu.stage()!=OriginalFrontend::Stage::simulating)throw std::runtime_error("Save failure still blocks Sim Next");
                for(int frame=0;frame<400 && menu.stage()==OriginalFrontend::Stage::simulating;++frame)menu.observe(game,.05);
                if(menu.tournament().results.size()!=1 || !menu.tournament_error().empty())throw std::runtime_error("Completed match save failed");
                OriginalTournament saved;saved.load(probe);
                if(saved.results.size()!=1 || !fs::exists(fs::path(probe.string()+".bak")))throw std::runtime_error("Checkpoint replacement/backup failed");
                OriginalFrontend resumed(opt.data,opt.audio_data,probe,true);resumed.open(game);
                SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=SDLK_L;resumed.event(event,game);
                if(resumed.tournament().results.size()!=1)throw std::runtime_error("Resume checkpoint failed");
                if(resumed.preferences().match.wind!=1 || resumed.preferences().music!=75 || game.sequence->next_options.wind!=1)throw std::runtime_error("Production preferences reload failed");
                event.key.key=SDLK_RETURN;resumed.event(event,game);
                if(resumed.stage()!=OriginalFrontend::Stage::lineup)throw std::runtime_error("Next Match still blocked after resume");
                // Observe the next scheduled match actually starting as well.
                resumed.event(event,game);game.sequence->step();
                if(!game.sequence->running || !game.sequence->ticks)throw std::runtime_error("Next fixture did not start");
                // Corrupt only our unique probe, never tournament.sav. The
                // packaged executable must recover and retain its valid backup.
                {std::ofstream damaged(probe,std::ios::binary|std::ios::trunc);damaged<<"interrupted probe";}
                OriginalFrontend recovered(opt.data,opt.audio_data,probe,true);recovered.open(game);
                event.key.key=SDLK_L;recovered.event(event,game);
                if(recovered.stage()!=OriginalFrontend::Stage::tournament || !recovered.tournament().active || recovered.tournament().results.size()!=1)throw std::runtime_error("Production backup recovery failed");
                event.key.key=SDLK_S;recovered.event(event,game);
                saved.load(probe);OriginalTournament previous;previous.load(fs::path(probe.string()+".bak"));
                if(saved.results.size()!=1 || previous.results.size()!=1 || !recovered.tournament_error().empty())throw std::runtime_error("Recovered save destroyed valid backup");
                std::cout<<"Production save directory verified: "<<production.parent_path().string()<<"\nCreate, simulate, replace, backup, reload, resume, next fixture and preferences passed.\n";
                std::cout<<"Damaged checkpoint recovered; valid backup preserved.\n";
            }
            fs::remove(probe);auto backup=probe;backup+=".bak";fs::remove(backup);
            auto prefs=probe;prefs.replace_extension(".prefs");fs::remove(prefs);prefs+=".bak";fs::remove(prefs);
            SDL_Quit();co_return 0;
        }
        using Window=std::unique_ptr<SDL_Window,decltype(&SDL_DestroyWindow)>;
        using Renderer=std::unique_ptr<SDL_Renderer,decltype(&SDL_DestroyRenderer)>;
        using Texture=std::unique_ptr<SDL_Texture,decltype(&SDL_DestroyTexture)>;
#ifdef __EMSCRIPTEN__
        constexpr const char* window_title="UEFA Euro 96 - Browser preview";
#else
        constexpr const char* window_title="UEFA Euro 96 - Windows restoration preview";
#endif
        Window window(SDL_CreateWindow(window_title,presentation_height?1280:1200,presentation_height?720:830,
            SDL_WINDOW_RESIZABLE | (opt.frontend_smoke||opt.intro_smoke||opt.movie_smoke||opt.penalty_smoke||opt.team_smoke||opt.control_smoke||opt.goals_smoke||opt.actions_smoke||opt.graphics_smoke||opt.smoke||opt.physics_smoke||opt.practice_smoke||opt.opponents_smoke||opt.gamepad_smoke||!opt.capture.empty()?SDL_WINDOW_HIDDEN:0)),SDL_DestroyWindow);
        if(!window) throw std::runtime_error(SDL_GetError());
        Renderer renderer(SDL_CreateRenderer(window.get(),nullptr),SDL_DestroyRenderer);
        if(!renderer) throw std::runtime_error(SDL_GetError());
        check(SDL_SetRenderLogicalPresentation(renderer.get(),960,664,SDL_LOGICAL_PRESENTATION_LETTERBOX),"Set logical size");
        SDL_SetRenderVSync(renderer.get(),1);
        Texture texture(SDL_CreateTexture(renderer.get(),SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_STREAMING,640,480),SDL_DestroyTexture);
        if(!texture) throw std::runtime_error(SDL_GetError());
        check(SDL_SetTextureScaleMode(texture.get(),SDL_SCALEMODE_NEAREST),"Set pixel scaling");
        std::array<Tactics,10> tactics;
        for(std::size_t i=0;i<10;++i) tactics[i]=decode_tactics(euro.entry(111+i));
        Navigation nav; nav.screen=opt.screen; nav.page=opt.original3d?4:opt.practice?3:(opt.physics?2:(opt.tactics?1:0));
        BallSandbox lab; FixedStep simulation_clock;
        OriginalGraphics graphics(opt.data);graphics.overview=opt.stadium;
        if(opt.fixed_match_seed)graphics.sequence->match_seed_override=opt.fixed_match_seed;
        else if(interactive || opt.random_match_seed)graphics.sequence->match_seed_override.reset();
        if(opt.action_scene){graphics.action_view=true;graphics.sequence->start_continuation();if(!opt.capture.empty())for(int i=0;i<(opt.result_capture?180:30);++i)graphics.advance(0.05);}
        if(opt.goal_review){graphics.action_view=true;graphics.sequence->start_goal_review();if(!opt.capture.empty())for(int i=0;i<29;++i)graphics.advance(.05);}
        if(opt.original_play){graphics.action_view=true;graphics.sequence->start_play();}
        graphics.sequence->set_match_minutes(opt.match_minutes);
        graphics.sequence->next_home=opt.home;graphics.sequence->next_away=opt.away;
        graphics.sequence->next_home_formation=original_teams[opt.home].formation;
        graphics.sequence->next_away_formation=original_teams[opt.away].formation;
        graphics.sequence->next_format=static_cast<OriginalActions::MatchFormat>(opt.format);graphics.match_camera=opt.camera;MatchSetup setup;SquadMenu squad;
        if(opt.setup)setup.toggle(graphics);
        if(opt.team_play){graphics.action_view=true;graphics.sequence->start_match(opt.cpu_vs_cpu);if(opt.cpu_vs_cpu && !opt.capture.empty())for(int i=0;i<opt.match_ticks;++i)graphics.advance(.05);}
        // Offscreen visual-review fixtures use actual engine play and existing keeper practice.
        if(!opt.hud_capture.empty()){
            auto& s=*graphics.sequence;
            if(opt.hud_capture=="keeper")s.practice_restart(6);
            else{
                s.start_match(true);bool ready=false;
                for(int i=0;i<6000;++i){
                    graphics.advance(.05);const auto& e=s.engine;
                    if(i>60 && !s.restart_mode && !e.match_mode && e.ball_poss>0 && !e.ball_in_hands && s.is_blue(e.ball_poss-1)==(opt.hud_capture!="defend")){ready=true;break;}
                }
                if(!ready)throw std::runtime_error("HUD capture did not reach requested live possession");
                if(opt.hud_capture!="watch")s.set_cpu_control(false);
                if(opt.hud_capture=="charge"){
                    // Stage an isolated stationary possession for this visual fixture.
                    auto& e=s.engine;e.holder_lose_ball();e.reset_shot();e.ball_limbo_on=0;
                    s.selected=1;auto& p=e.teams[s.selected];p.tm_x=900;p.tm_y=400;
                    p.tm_limbo=p.contact=0;p.tm_xdis=p.mface_x=1;p.tm_ydis=p.mface_y=0;
                    e.reset_ideas(&p);e.init_stand_act(&p);e.collect_ball(&p);e.hold_ball(&p);
                    s.set_play_input(0,0);s.set_play_buttons(false,false);
                    for(int i=0;i<12;++i){s.set_play_buttons(true,false);s.step();}
                    if(s.live_charge()<6)throw std::runtime_error("HUD capture did not retain a stationary charge");
                }
                if(opt.hud_capture=="paused"){s.clear_play_input();graphics.paused=true;}
                if(opt.hud_capture=="panels")graphics.match_panels=true;
                if(opt.hud_capture=="replay"){
                    if(!graphics.start_replay())throw std::runtime_error("Replay capture needs recorded frames");
                    graphics.seek_replay(int(graphics.replay_frames()/2));graphics.change_replay_speed(-3);
                }
            }
        }
        auto audio_root=opt.audio_data;
        if(audio_root.empty()){
            const auto base=fs::path(std::u8string(reinterpret_cast<const char8_t*>(SDL_GetBasePath())));
            for(const auto& candidate:{base/"audio",opt.data,opt.data.parent_path()/"local-data"})
                if(fs::exists(candidate/"DIGI/PITCH/PITCH.MAD") || fs::exists(candidate/"FRONTEND/TITLE.IDX")){audio_root=candidate;break;}
        }
        graphics.load_cd_graphics(audio_root);
        graphics.walkoffs_enabled=opt.front_end && opt.frontend_capture.empty(); // Diagnostics have no frontend pause/skip overlay.
        graphics.prematches_enabled=opt.frontend_capture.empty();
        graphics.exits_enabled=opt.frontend_capture.empty(); // Named menu captures show that menu, not its preceding exit.
        std::unique_ptr<OriginalAudio> audio;
        std::unique_ptr<OriginalCommentary> commentary;
        std::unique_ptr<OriginalMusic> music;
        std::unique_ptr<OriginalFrontend> front;
        OriginalMovie movie(audio_root,interactive || opt.movie_smoke || opt.intro_smoke);
        if(interactive || opt.audio_smoke || opt.frontend_smoke || opt.movie_smoke || opt.intro_smoke){
            try{audio=std::make_unique<OriginalAudio>(audio_root);SDL_Log("%s",audio->status().c_str());}
            catch(const std::exception& error){SDL_Log("Original audio unavailable: %s",error.what());}
            try{commentary=std::make_unique<OriginalCommentary>(audio_root);}
            catch(const std::exception& error){SDL_Log("Original commentary unavailable: %s",error.what());}
            if(opt.front_end)try{music=std::make_unique<OriginalMusic>(audio_root);}
            catch(const std::exception& error){SDL_Log("Original music unavailable: %s",error.what());}
        }
        if(opt.front_end){
            front=std::make_unique<OriginalFrontend>(opt.data,audio_root,opt.replay_file_smoke?fs::temp_directory_path()/("Actua96-replay-smoke-"+std::to_string(SDL_GetTicksNS()))/"tournament.sav":fs::path{},interactive && opt.frontend_capture.empty(),opt.intro_smoke || opt.movie_smoke || opt.frontend_capture=="intro" || opt.frontend_capture=="movie");front->menu_fades=opt.frontend_capture.empty() && opt.capture.empty() && !opt.frontend_smoke && !opt.movie_smoke && !opt.intro_smoke;front->open(graphics);
            if(!opt.frontend_capture.empty() && opt.frontend_capture!="title" && opt.frontend_capture!="movie" && opt.frontend_capture!="intro"){
                const auto key=[&](SDL_Keycode k){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;front->event(e,graphics);};
                key(SDLK_RETURN);
                if(opt.frontend_capture!="selection")key(SDLK_RETURN);
                if(opt.frontend_capture=="interval" || opt.frontend_capture=="result"){
                    key(SDLK_RETURN);auto& s=*graphics.sequence;s.blue_goals=2;s.red_goals=1;
                    if(opt.frontend_capture=="interval"){s.interval_ticks=100;s.interval_minute=45;}
                    else s.finish_match();
                    front->observe(graphics,0);
                }
            }
        }
        if(!graphics.menu_ui)graphics.menu_ui=std::make_shared<OriginalMenuArt>(Archive(opt.data/"EURO.DAT",opt.data/"EURO.OFF"));
        // Startup presentation is optional media; a damaged or absent film must not block play.
        // Euro96 enters its frontend directly. The Actua film path is diagnostic only.
        if(front && opt.intro_smoke && std::filesystem::exists(audio_root/"VIDEO/INTRO.AMV")){
            try{movie.start(true);}catch(const std::exception& error){front->movie_error="INTRO UNAVAILABLE";SDL_Log("CD intro: %s",error.what());}
        }
        if(opt.frontend_capture=="intro"){movie.start(true);movie.seek(24);movie.toggle_pause();}
        if(opt.frontend_capture=="movie"){movie.start();movie.seek(70);movie.toggle_pause();}
        PlayerPractice practice; FixedStep practice_clock;
        if(opt.opponents) practice.set_opponents(true);
        if(opt.result_capture) {
            for(int i=0;i<8000 && !practice.match_finished;++i)practice.step();
            if(!practice.match_finished)throw std::runtime_error("Result capture did not reach full time");
        } else if(opt.practice && !opt.capture.empty()) {
            if(opt.opponents) {
                practice.engine.teams[1].tm_x=1180;practice.engine.hold_ball(&practice.engine.teams[1]);
                practice.aim_at(1280,400);practice.shoot();
            } else practice.pass();
            for(int i=0;i<12;++i)practice.step();
        }
        auto previous_time=SDL_GetTicksNS();
        bool focused=true;
        bool live_key_shot=false,live_key_pass=false;
        if(opt.physics && !opt.capture.empty()) { lab.curve=1; lab.launch(); for(int i=0;i<24;++i) lab.step(); }
        SDL_JoystickID virtual_id=0,local_second_id=0;SDL_Joystick* virtual_pad=nullptr;SDL_Joystick* local_second_pad=nullptr;
        if(opt.intro_smoke || opt.movie_smoke || opt.frontend_smoke || opt.penalty_smoke || opt.gamepad_smoke || opt.control_smoke || opt.cpu_smoke || opt.depth_smoke) {
            SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");
            SDL_VirtualJoystickDesc desc{};SDL_INIT_INTERFACE(&desc);desc.type=SDL_JOYSTICK_TYPE_GAMEPAD;
            desc.naxes=SDL_GAMEPAD_AXIS_COUNT;desc.nbuttons=SDL_GAMEPAD_BUTTON_COUNT;
            desc.axis_mask=(1u<<SDL_GAMEPAD_AXIS_COUNT)-1;desc.button_mask=(1u<<SDL_GAMEPAD_BUTTON_COUNT)-1;
            desc.name="Xbox controller (virtual test)";desc.vendor_id=0x045e;desc.product_id=0x028e;
            virtual_id=SDL_AttachVirtualJoystick(&desc);check(virtual_id!=0,"Attach gamepad smoke device");
            virtual_pad=SDL_OpenJoystick(virtual_id);check(virtual_pad!=nullptr,"Open gamepad smoke device");
        }
        const auto attach_local_second=[&](){SDL_VirtualJoystickDesc d{};SDL_INIT_INTERFACE(&d);d.type=SDL_JOYSTICK_TYPE_GAMEPAD;d.naxes=6;d.nbuttons=15;d.name="Second local test controller";local_second_id=SDL_AttachVirtualJoystick(&d);check(local_second_id!=0,"Attach second controller");local_second_pad=SDL_OpenJoystick(local_second_id);check(local_second_pad!=nullptr,"Open second controller");};
        if(opt.head_to_head_smoke)attach_local_second();
        GamepadInput pads(opt.head_to_head_smoke);bool pad_prompts=pads.connected();
        bool running=true, fullscreen=false,sound_muted=false;
        int uploaded=-1, frames=0,penalty_smoke_stage=0,penalty_smoke_age=0;
        bool setpiece_smoke_released=false;
        int local_pause_tick=0,local_first_selected=0,local_second_selected=0;
        SDL_JoystickID penalty_detached_id=0;
#ifdef __EMSCRIPTEN__
        // Creating SDL's WebGL canvas can queue a transient blur during startup.
        // The launch click owns focus; subsequent real blur events still pause.
        EM_ASM({ Module.canvas.focus(); });
        SDL_FlushEvent(SDL_EVENT_WINDOW_FOCUS_LOST);
#endif
        while(running) {
            if(opt.intro_smoke || opt.movie_smoke || opt.frontend_smoke || opt.penalty_smoke || opt.gamepad_smoke || opt.control_smoke || opt.depth_smoke) SDL_UpdateJoysticks();
            pads.set_local_match(graphics.sequence->head_to_head || (graphics.saved_replay_active() && pads.local_match()));
            if(front)front->connected_controllers=pads.count();
            SDL_Event event;
            while(SDL_PollEvent(&event)) {
                const auto pad_event=pads.event(event,focused);
                if(front)front->connected_controllers=pads.count();
                if(pad_event==PadEvent::joined){graphics.sequence->clear_play_input();continue;}
                if(pad_event==PadEvent::activity) pad_prompts=true;
                if(pad_event==PadEvent::disconnected) {
                    pad_prompts=false;practice.set_input(0,0);practice.set_tackle_held(false);
                    if(nav.page==3 && !practice.match_finished) {practice.paused=true;practice.status="Controller disconnected. Reconnect + Start, or P to resume.";}
                    if(nav.page==2) lab.paused=true;
                    if(nav.page==4){graphics.paused=true;graphics.exit.paused=true;graphics.prematch.paused=true;graphics.walkoff.paused=true;graphics.sequence->clear_play_input();}
                }
                if(event.type==SDL_EVENT_KEY_DOWN || event.type==SDL_EVENT_MOUSE_BUTTON_DOWN) pad_prompts=false;
                if(event.type==SDL_EVENT_QUIT) running=false;
#ifndef __EMSCRIPTEN__
                if(event.type==SDL_EVENT_KEY_DOWN && !event.key.repeat && event.key.key==SDLK_F12){
                    presentation_height=presentation_height==0?720:presentation_height==720?1080:0;
                    if(!fullscreen)SDL_SetWindowSize(window.get(),presentation_height?1280:1200,presentation_height?720:830);
                    continue;
                }
#endif
                if(event.type==SDL_EVENT_WINDOW_FOCUS_LOST) { live_key_shot=live_key_pass=false;focused=false; simulation_clock.reset(); practice_clock.reset(); graphics.exit.paused=true;graphics.prematch.paused=true;graphics.walkoff.paused=true;graphics.sequence->clear_play_input();if(graphics.sequence->play)graphics.paused=true; }
                if(event.type==SDL_EVENT_WINDOW_FOCUS_GAINED) { focused=true; simulation_clock.reset(); practice_clock.reset(); }
                if(movie.active()){
                    if((pad_event==PadEvent::disconnected || event.type==SDL_EVENT_WINDOW_FOCUS_LOST) && !movie.paused())movie.toggle_pause();
                    if(event.type==SDL_EVENT_KEY_DOWN && !event.key.repeat && focused){
                        const auto k=event.key.key;
                        if(k==SDLK_ESCAPE || k==SDLK_SPACE || k==SDLK_RETURN)movie.stop();
                        else if(k==SDLK_P)movie.toggle_pause();
                        else if(k==SDLK_LEFT || k==SDLK_RIGHT)movie.seek(movie.position()+(k==SDLK_LEFT?-5:5));
                        else if(k==SDLK_R)movie.seek(0);
                        else if(k==SDLK_F8 && front)front->audio_shortcut(true);
                        else if(k==SDLK_F11){fullscreen=!fullscreen;SDL_SetWindowFullscreen(window.get(),fullscreen);}
                    }else if(event.type==SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button==SDL_BUTTON_LEFT && focused){movie.stop();
                    }else if(event.type==SDL_EVENT_GAMEPAD_BUTTON_DOWN && focused && (pads.local_match()?pads.player_for(event.gbutton.which)>=0:event.gbutton.which==pads.active_id())){
                        const auto b=event.gbutton.button;
                        if(b==SDL_GAMEPAD_BUTTON_SOUTH || b==SDL_GAMEPAD_BUTTON_BACK || b==SDL_GAMEPAD_BUTTON_EAST)movie.stop();
                        else if(b==SDL_GAMEPAD_BUTTON_START)movie.toggle_pause();
                        else if(b==SDL_GAMEPAD_BUTTON_NORTH)movie.seek(0);
                        else if(b==SDL_GAMEPAD_BUTTON_LEFT_SHOULDER || b==SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)movie.seek(movie.position()+(b==SDL_GAMEPAD_BUTTON_LEFT_SHOULDER?-5:5));
                    }
                    continue;
                }
                if(event.type==SDL_EVENT_KEY_DOWN || event.type==SDL_EVENT_KEY_UP){
                    const bool down=event.type==SDL_EVENT_KEY_DOWN && focused;
                    if(event.key.key==SDLK_X)live_key_shot=down;
                    if(event.key.key==SDLK_SPACE)live_key_pass=down;
                }
                if(front && focused && nav.page==4 && (front->visible() || (!squad.open && !graphics.replaying))){
                    SDL_Event front_event=event;
                    if(event.type==SDL_EVENT_MOUSE_BUTTON_DOWN && front->visible())check(SDL_ConvertEventToRenderCoordinates(renderer.get(),&front_event),"Front-end pointer coordinates");
                    const bool pad_event_ok=(event.type!=SDL_EVENT_GAMEPAD_BUTTON_DOWN && event.type!=SDL_EVENT_GAMEPAD_BUTTON_UP) || (pads.local_match()?pads.player_for(event.gbutton.which)>=0:event.gbutton.which==pads.active_id());
                    if(pad_event_ok && front->event(front_event,graphics)){pads.set_local_match(graphics.sequence->head_to_head || (graphics.saved_replay_active() && pads.local_match()));live_key_shot=live_key_pass=false;if(front->quit)running=false;continue;}
                }
                if(event.type==SDL_EVENT_KEY_DOWN && !event.key.repeat) {
                    if(graphics.sequence->head_to_head && !graphics.replaying && (!front || !front->visible()) && !setup.open &&
                       (event.key.key==SDLK_X || event.key.key==SDLK_SPACE || event.key.key==SDLK_C || event.key.key==SDLK_E || event.key.key==SDLK_Q || event.key.key==SDLK_Y || event.key.key==SDLK_B || event.key.key==SDLK_F5))continue;
                    if(event.key.key==SDLK_F9 && (commentary || front)){if(front)front->audio_shortcut(false);else commentary->cycle_volume();continue;}
                    if(event.key.key==SDLK_F8 && (audio || music || commentary || front)){if(front)front->audio_shortcut(true);else sound_muted=!sound_muted;if(audio && !front)audio->set_muted(sound_muted);continue;}
                    if(nav.page==4){
                        if(!setup.open && !squad.open && graphics.sequence->team_play){
                            if(event.key.key==SDLK_F6){graphics.match_panels=!graphics.match_panels;continue;}
                            if(event.key.key==SDLK_TAB || event.key.key==SDLK_F7){graphics.show_radar=!graphics.show_radar;continue;}
                        }
                        if(event.key.key==SDLK_F3 && !setup.open){squad.toggle(graphics);continue;}
                        if(squad.open){switch(event.key.key){
                            case SDLK_UP:squad.row=(squad.row+2)%3;break;
                            case SDLK_DOWN:squad.row=(squad.row+1)%3;break;
                            case SDLK_LEFT:squad.change(-1);break;case SDLK_RIGHT:squad.change(1);break;
                            case SDLK_RETURN:squad.confirm(graphics);break;case SDLK_ESCAPE:squad.toggle(graphics);break;
                            default:break;}continue;}
                        if(graphics.replaying){
                            if(event.key.key==SDLK_S && front && !graphics.saved_replay_active())front->save_replay(graphics);
                            else if(event.key.key==SDLK_SPACE || event.key.key==SDLK_Q || event.key.key==SDLK_F4)graphics.finish_replay();
                            else if(event.key.key==SDLK_P)graphics.toggle_replay_pause();
                            else if(event.key.key==SDLK_V)graphics.change_camera();
                            else if(event.key.key==SDLK_LEFT)graphics.step_replay(-1);
                            else if(event.key.key==SDLK_RIGHT)graphics.step_replay(1);
                            else if(event.key.key==SDLK_PAGEUP)graphics.step_replay(-20);
                            else if(event.key.key==SDLK_PAGEDOWN)graphics.step_replay(20);
                            else if(event.key.key==SDLK_HOME)graphics.seek_replay(0);
                            else if(event.key.key==SDLK_END)graphics.seek_replay(int(graphics.replay_frames())-1);
                            else if(event.key.key==SDLK_UP)graphics.change_replay_speed(1);
                            else if(event.key.key==SDLK_DOWN)graphics.change_replay_speed(-1);
                            else if(event.key.key==SDLK_L)graphics.toggle_replay_loop();
                            else if(event.key.key==SDLK_ESCAPE)graphics.finish_replay();
                            continue;
                        }
                        if(event.key.key==SDLK_F4 && !setup.open){graphics.start_replay();continue;}
                        if(event.key.key==SDLK_F5 && !setup.open){graphics.action_view=true;graphics.sequence->practice_restart(graphics.sequence->restart_practice+1);graphics.paused=false;continue;}
                        if(event.key.key==SDLK_F2){setup.toggle(graphics);continue;}
                        if(setup.open){
                            switch(event.key.key){
                                case SDLK_UP:setup.row=(setup.row+6)%7;break;
                                case SDLK_DOWN:setup.row=(setup.row+1)%7;break;
                                case SDLK_LEFT:setup.change(graphics,-1);break;
                                case SDLK_RIGHT:setup.change(graphics,1);break;
                                case SDLK_RETURN:setup.start(graphics,true);break;
                                case SDLK_Y:setup.start(graphics,false);break;
                                case SDLK_ESCAPE:setup.toggle(graphics);break;
                                default:break;
                            }continue;
                        }
                        if(event.key.key==SDLK_V && graphics.sequence->team_play){graphics.change_camera();continue;}
                        if(event.key.key==SDLK_B && focused){
                            if(graphics.action_view && graphics.sequence->team_play)graphics.sequence->set_cpu_control(!graphics.sequence->cpu_vs_cpu);
                            else{graphics.action_view=true;graphics.overview=false;graphics.paused=false;graphics.sequence->start_match(true);}continue;
                        }
                        if(event.key.key==SDLK_L && graphics.sequence->full_match){auto& s=*graphics.sequence;const int lengths[]={5,10,20,40,90};int next=5;for(int i=0;i<4;++i)if(s.match_minutes==lengths[i])next=lengths[i+1];s.set_match_minutes(next);continue;}
                        if(event.key.key==SDLK_Y){graphics.action_view=true;graphics.overview=false;graphics.paused=false;graphics.sequence->start_match();continue;}
                        if(event.key.key==SDLK_T){graphics.action_view=true;graphics.overview=false;graphics.paused=false;graphics.sequence->start_play();continue;}
                        if(graphics.action_view && graphics.sequence->play){
                            if(!focused)continue;
                            switch(event.key.key){
                                case SDLK_SPACE:if(!graphics.paused && (!graphics.sequence->team_play || graphics.sequence->human_keeper_hold()))graphics.sequence->request_play_kick(OriginalActions::PlayKick::ground);continue;
                                case SDLK_C:if(!graphics.paused)graphics.sequence->request_play_kick(OriginalActions::PlayKick::loft);continue;
                                case SDLK_X:if(!graphics.paused && (!graphics.sequence->team_play || graphics.sequence->human_keeper_hold()))graphics.sequence->request_play_kick(OriginalActions::PlayKick::shot);continue;
                                case SDLK_E:if(!graphics.paused)graphics.sequence->switch_play_player();continue;
                                case SDLK_R:graphics.sequence->restart_play();graphics.paused=false;continue;
                                case SDLK_Q:graphics.sequence->start_continuation();graphics.paused=false;continue;
                                case SDLK_P:graphics.sequence->clear_play_input();graphics.paused=!graphics.paused;continue;
                                case SDLK_W:case SDLK_A:case SDLK_S:case SDLK_D:
                                case SDLK_LEFT:case SDLK_RIGHT:case SDLK_UP:case SDLK_DOWN:case SDLK_V:continue;
                                default:break;
                            }
                        }
                        switch(event.key.key){
                            case SDLK_SPACE:graphics.action_view=true;graphics.paused=false;graphics.sequence->start_continuation();continue;
                            case SDLK_M:graphics.action_view=true;graphics.paused=false;graphics.sequence->start(true);continue;
                            case SDLK_N:graphics.action_view=true;graphics.paused=false;graphics.sequence->start_attack(400,400);continue;
                            case SDLK_G:graphics.action_view=true;graphics.overview=false;graphics.paused=false;graphics.sequence->start_goal_review();continue;
                            case SDLK_H:graphics.action_view=true;graphics.overview=false;graphics.paused=false;graphics.sequence->start_goal_review(0,true);continue;
                            case SDLK_R:graphics.sequence->start_continuation();graphics.paused=false;continue;
                            case SDLK_LEFT:graphics.action_view=false;graphics.change_clip(-1);continue;
                            case SDLK_RIGHT:graphics.action_view=false;graphics.change_clip(1);continue;
                            case SDLK_UP:graphics.angle+=0.2f;continue;
                            case SDLK_DOWN:graphics.angle-=0.2f;continue;
                            case SDLK_K:graphics.change_kit(1);continue;
                            case SDLK_W:graphics.wire=!graphics.wire;continue;
                            case SDLK_V:graphics.overview=!graphics.overview;continue;
                            default:break;
                        }
                    }
                    switch(event.key.key) {
                        case SDLK_ESCAPE: running=false; break;
                        case SDLK_TAB: nav.toggle(); break;
                        case SDLK_1: nav.page=0; break;
                        case SDLK_2: nav.page=1; break;
                        case SDLK_3: nav.page=2; break;
                        case SDLK_5: nav.page=4;break;
                        case SDLK_4: nav.page=3; break;
                        case SDLK_O: if(nav.page==3) practice.set_opponents(!practice.opponents); break;
                        case SDLK_E: if(nav.page==3) practice.switch_player(); break;
                        case SDLK_X: if(nav.page==3) practice.action(); break;
                        case SDLK_SPACE: if(nav.page==2) lab.launch(); else if(nav.page==3) practice.pass(); break;
                        case SDLK_R: if(nav.page==2) lab.reset(); else if(nav.page==3) practice.reset(); break;
                        case SDLK_P: if(nav.page==4) graphics.paused=!graphics.paused; else if(nav.page==2) lab.paused=!lab.paused; else if(nav.page==3) practice.toggle_pause(); break;
                        case SDLK_V: if(nav.page==2) lab.preset=(lab.preset+1)%3; break;
                        case SDLK_C: if(nav.page==2) lab.curve=lab.curve==0?1:(lab.curve==1?-1:0); else if(nav.page==3) practice.pass(true); break;
                        case SDLK_W: if(nav.page==2) lab.toggle_wind(); break;
                        case SDLK_RIGHT: if(nav.page==2) lab.turn(-5); else nav.move(1); break;
                        case SDLK_LEFT: if(nav.page==2) lab.turn(5); else nav.move(-1); break;
                        case SDLK_UP: if(nav.page==2) lab.change_power(1); else nav.move(-1); break;
                        case SDLK_DOWN: if(nav.page==2) lab.change_power(-1); else nav.move(1); break;
                        case SDLK_LEFTBRACKET: nav.change_formation(-1); break;
                        case SDLK_RIGHTBRACKET: nav.change_formation(1); break;
                        case SDLK_F11:
                            fullscreen=!fullscreen;
                            check(SDL_SetWindowFullscreen(window.get(),fullscreen),"Toggle fullscreen");
                            break;
                        default: break;
                    }
                }
                if(event.type==SDL_EVENT_GAMEPAD_BUTTON_DOWN && focused && (pads.local_match()?pads.player_for(event.gbutton.which)>=0:event.gbutton.which==pads.active_id())) {
                    if(nav.page==4){
                        if(squad.open){switch(event.gbutton.button){
                            case SDL_GAMEPAD_BUTTON_DPAD_UP:squad.row=(squad.row+2)%3;break;
                            case SDL_GAMEPAD_BUTTON_DPAD_DOWN:squad.row=(squad.row+1)%3;break;
                            case SDL_GAMEPAD_BUTTON_DPAD_LEFT:squad.change(-1);break;case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:squad.change(1);break;
                            case SDL_GAMEPAD_BUTTON_SOUTH:squad.confirm(graphics);break;case SDL_GAMEPAD_BUTTON_BACK:squad.toggle(graphics);break;
                            default:break;}continue;}
                        if(graphics.replaying){
                            if(event.gbutton.button==SDL_GAMEPAD_BUTTON_EAST && front && !graphics.saved_replay_active())front->save_replay(graphics);
                            else if(event.gbutton.button==SDL_GAMEPAD_BUTTON_SOUTH || event.gbutton.button==SDL_GAMEPAD_BUTTON_BACK)graphics.finish_replay();
                            else if(event.gbutton.button==SDL_GAMEPAD_BUTTON_START)graphics.toggle_replay_pause();
                            else if(event.gbutton.button==SDL_GAMEPAD_BUTTON_RIGHT_STICK)graphics.change_camera();
                            else if(event.gbutton.button==SDL_GAMEPAD_BUTTON_DPAD_LEFT)graphics.step_replay(-1);
                            else if(event.gbutton.button==SDL_GAMEPAD_BUTTON_DPAD_RIGHT)graphics.step_replay(1);
                            else if(event.gbutton.button==SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)graphics.step_replay(-20);
                            else if(event.gbutton.button==SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)graphics.step_replay(20);
                            else if(event.gbutton.button==SDL_GAMEPAD_BUTTON_DPAD_UP)graphics.change_replay_speed(1);
                            else if(event.gbutton.button==SDL_GAMEPAD_BUTTON_DPAD_DOWN)graphics.change_replay_speed(-1);
                            else if(event.gbutton.button==SDL_GAMEPAD_BUTTON_WEST)graphics.toggle_replay_loop();
                            else if(event.gbutton.button==SDL_GAMEPAD_BUTTON_NORTH)graphics.seek_replay(0);
                            continue;
                        }
                        if(!graphics.paused && !setup.open && graphics.sequence->team_play && !graphics.sequence->full_time && event.gbutton.button==SDL_GAMEPAD_BUTTON_RIGHT_STICK){graphics.match_panels=!graphics.match_panels;continue;}
                        if(graphics.paused && !setup.open && graphics.sequence->full_match){
                            if(event.gbutton.button==SDL_GAMEPAD_BUTTON_NORTH){if(graphics.sequence->head_to_head)squad.away=pads.player_for(event.gbutton.which)==1;squad.toggle(graphics);continue;}
                            if(event.gbutton.button==SDL_GAMEPAD_BUTTON_WEST){graphics.start_replay();continue;}
                        }
                        if(event.gbutton.button==SDL_GAMEPAD_BUTTON_LEFT_STICK){setup.toggle(graphics);continue;}
                        if(setup.open){
                            switch(event.gbutton.button){
                                case SDL_GAMEPAD_BUTTON_DPAD_UP:setup.row=(setup.row+6)%7;break;
                                case SDL_GAMEPAD_BUTTON_DPAD_DOWN:setup.row=(setup.row+1)%7;break;
                                case SDL_GAMEPAD_BUTTON_DPAD_LEFT:setup.change(graphics,-1);break;
                                case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:setup.change(graphics,1);break;
                                case SDL_GAMEPAD_BUTTON_SOUTH:setup.start(graphics,true);break;
                                case SDL_GAMEPAD_BUTTON_NORTH:setup.start(graphics,false);break;
                                case SDL_GAMEPAD_BUTTON_BACK:setup.toggle(graphics);break;
                                default:break;
                            }continue;
                        }
                        if(event.gbutton.button==SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER){
                            if(graphics.action_view && graphics.sequence->play){
                                // Both shoulders select players in human play. RB may take
                                // over a watched exhibition, never silently give control away.
                                if(!graphics.paused){
                                    if(graphics.sequence->cpu_vs_cpu)graphics.sequence->set_cpu_control(false);
                                    else{auto scope=graphics.sequence->input_scope(graphics.sequence->head_to_head?pads.player_for(event.gbutton.which):0);graphics.sequence->switch_play_player();}
                                }
                            }else{graphics.action_view=true;graphics.overview=false;graphics.paused=false;graphics.sequence->start_match(true);}continue;
                        }
                        if(event.gbutton.button==SDL_GAMEPAD_BUTTON_START){
                            if(!graphics.action_view || !graphics.sequence->play){graphics.action_view=true;graphics.overview=false;graphics.paused=false;graphics.sequence->start_match();}
                            else{graphics.sequence->clear_play_input();graphics.paused=!graphics.paused;}continue;
                        }
                        if(graphics.action_view && graphics.sequence->play){
                            // Back is menu navigation, never a request to replace a match.
                            if(event.gbutton.button==SDL_GAMEPAD_BUTTON_BACK)continue;
                            if(event.gbutton.button==SDL_GAMEPAD_BUTTON_RIGHT_STICK && (graphics.paused || graphics.sequence->full_time)){const bool finished=graphics.sequence->full_time;graphics.sequence->restart_play();if(finished)graphics.paused=false;continue;}
                            auto scope=graphics.sequence->input_scope(graphics.sequence->head_to_head?pads.player_for(event.gbutton.which):0);
                            if(!graphics.paused)switch(event.gbutton.button){
                                case SDL_GAMEPAD_BUTTON_SOUTH:if(!graphics.sequence->team_play || graphics.sequence->human_keeper_hold())graphics.sequence->request_play_kick(OriginalActions::PlayKick::ground);break;
                                case SDL_GAMEPAD_BUTTON_NORTH:graphics.sequence->request_play_kick(OriginalActions::PlayKick::loft);break;
                                case SDL_GAMEPAD_BUTTON_WEST:if(!graphics.sequence->team_play || graphics.sequence->human_keeper_hold())graphics.sequence->request_play_kick(OriginalActions::PlayKick::shot);break;
                                case SDL_GAMEPAD_BUTTON_EAST:case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:graphics.sequence->switch_play_player();break;
                                default:break;
                            }
                            continue;
                        }
                    }
                    if(nav.page==3 && pads.practice_button(static_cast<SDL_GamepadButton>(event.gbutton.button),practice)) continue;
                    if(event.gbutton.button==SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER){nav.toggle();continue;}
                    switch(event.gbutton.button) {
                        case SDL_GAMEPAD_BUTTON_DPAD_LEFT: if(nav.page==2) lab.turn(5); else nav.move(-1); break;
                        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: if(nav.page==2) lab.turn(-5); else nav.move(1); break;
                        case SDL_GAMEPAD_BUTTON_DPAD_UP: if(nav.page==2) lab.change_power(1); break;
                        case SDL_GAMEPAD_BUTTON_DPAD_DOWN: if(nav.page==2) lab.change_power(-1); break;
                        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: nav.change_formation(-1); break;
                        case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: nav.change_formation(1); break;
                        case SDL_GAMEPAD_BUTTON_SOUTH: if(nav.page==2) lab.launch(); else if(nav.page==3) practice.pass(); else nav.toggle(); break;
                        case SDL_GAMEPAD_BUTTON_NORTH: if(nav.page==2) lab.preset=(lab.preset+1)%3; else if(nav.page==3) practice.pass(true); break;
                        case SDL_GAMEPAD_BUTTON_WEST: if(nav.page==3) practice.action(); break;
                        case SDL_GAMEPAD_BUTTON_BACK: if(nav.page==3) practice.set_opponents(!practice.opponents); break;
                        case SDL_GAMEPAD_BUTTON_START: if(nav.page==2) lab.paused=!lab.paused; else if(nav.page==3) practice.toggle_pause(); break;
                        case SDL_GAMEPAD_BUTTON_EAST: if(nav.page==3) practice.switch_player(); else nav.page=3; break;
                        default: break;
                    }
                }
                if(event.type==SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button==SDL_BUTTON_LEFT) {
                    if(setup.open || squad.open)continue;
                    check(SDL_ConvertEventToRenderCoordinates(renderer.get(),&event),"Convert pointer coordinates");
                    float x=event.button.x,y=event.button.y;
                    if(graphics.replaying){
                        if(focused && nav.page==4 && x>=replay_bar_left && x<=replay_bar_left+replay_bar_width && y>=replay_bar_top-8 && y<=replay_bar_top+18)
                            graphics.seek_replay(int(std::lround((x-replay_bar_left)/replay_bar_width*(graphics.replay_frames()-1))));
                        continue;
                    }
                    if(!(nav.page==4 && (graphics.clean_match_view() || graphics.sequence->team_play)) && y>=68 && y<98) {
                        if(x>=24 && x<208) nav.page=0;
                        if(x>=220 && x<404) nav.page=1;
                        if(x>=416 && x<600) nav.page=2;
                        if(x>=612 && x<796) nav.page=3;
                        if(x>=804 && x<936) nav.page=4;
                    }
                    if(nav.page<2 && y>=412 && y<442) {
                        if(x>=696 && x<808) nav.move(-1);
                        if(x>=820 && x<932) nav.move(1);
                    }
                    if(nav.page==2) {
                        if(x>=24 && x<664 && y>=156 && y<556) lab.aim_at((x-24)*2,(y-156)*2);
                        if(x>=696 && x<928 && y>=382 && y<412) lab.launch();
                        if(x>=696 && x<808 && y>=422 && y<452) lab.reset();
                        if(x>=816 && x<928 && y>=422 && y<452) lab.paused=!lab.paused;
                    }
                    if(nav.page==3) {
                        if(practice.match_finished && x>=200 && x<488 && y>=386 && y<416) {practice.reset();continue;}
                        if(x>=14 && x<674 && y>=156 && y<556) practice.aim_at((x-24)*2,(y-156)*2);
                        if(x>=696 && x<928) {
                            if(y>=310 && y<340) practice.pass();
                            if(y>=350 && y<380) practice.pass(true);
                            if(y>=390 && y<420) practice.action();
                            if(y>=430 && y<460) practice.set_opponents(!practice.opponents);
                        }
                    }
                    if(nav.page==1 && x>=44 && x<644 && y>=156 && y<531)
                        nav.zone=(nav.zone>=32 && nav.zone<64?32:0)+int((y-156)/93.75f)*8+int((x-44)/75);
                }
            }
            if(uploaded!=nav.screen) {
                const auto rgba=decode_screen(euro,std::size_t(nav.screen));
                check(SDL_UpdateTexture(texture.get(),nullptr,rgba.data(),640*4),"Upload preserved screen");
                uploaded=nav.screen;
            }
            const auto now=SDL_GetTicksNS();
            const double elapsed=double(now-previous_time)/1e9; previous_time=now;
            if(front && (front->movie_requested || front->intro_requested)){const bool intro=front->intro_requested;front->movie_requested=front->intro_requested=false;try{movie.start(intro);}catch(const std::exception& error){front->movie_error="MOVIE UNAVAILABLE";SDL_Log("CD movie: %s",error.what());}}
            if(front && front->trophy_requested>=0){const int film=front->trophy_requested;front->trophy_requested=-1;try{movie.start_trophy(film);}catch(const std::exception& error){front->movie_error="TROPHY FILM UNAVAILABLE";SDL_Log("Euro96 trophy: %s",error.what());}}
            const auto keys=SDL_GetKeyboardState(nullptr);
            float touch_x=float(keys[SDL_SCANCODE_L])-float(keys[SDL_SCANCODE_J]);
            float touch_y=float(keys[SDL_SCANCODE_K])-float(keys[SDL_SCANCODE_I]);
            const auto pad_move=pads.movement();
            if(pad_move.x || pad_move.y) {touch_x=pad_move.x;touch_y=pad_move.y;}
            lab.set_aftertouch(touch_x,touch_y);
            const int steps=simulation_clock.advance(elapsed,nav.page==2 && focused && !lab.paused && !lab.outside);
            if(!opt.physics_smoke && opt.capture.empty()) for(int i=0;i<steps;++i) lab.step();
            if(opt.physics_smoke && frames>0) lab.step();
            float move_x=float(keys[SDL_SCANCODE_D]||keys[SDL_SCANCODE_RIGHT])-float(keys[SDL_SCANCODE_A]||keys[SDL_SCANCODE_LEFT]);
            float move_y=float(keys[SDL_SCANCODE_S]||keys[SDL_SCANCODE_DOWN])-float(keys[SDL_SCANCODE_W]||keys[SDL_SCANCODE_UP]);
            if(pad_move.x || pad_move.y) {move_x=pad_move.x;move_y=pad_move.y;}
            if(graphics.action_view && graphics.sequence->play && !graphics.replaying){
                if(pads.local_match() && !pads.local_ready()){
                    graphics.paused=true;graphics.prematch.paused=graphics.walkoff.paused=graphics.exit.paused=true;graphics.sequence->clear_play_input();
                }
                bool actions_released=!keys[SDL_SCANCODE_X] && !keys[SDL_SCANCODE_SPACE] && !keys[SDL_SCANCODE_C];
                for(auto b:{SDL_GAMEPAD_BUTTON_WEST,SDL_GAMEPAD_BUTTON_SOUTH,SDL_GAMEPAD_BUTTON_NORTH}){
                    if(pads.local_match())actions_released=actions_released && !pads.player_held(0,b) && !pads.player_held(1,b);
                    else actions_released=actions_released && !pads.held(b);
                }
                if(graphics.opening_input_guard && actions_released)graphics.opening_input_guard=false;
                if(nav.page==4 && focused && !graphics.paused && !graphics.prematch.active() && !graphics.walkoff.active() && !graphics.opening_input_guard){
                    if(graphics.sequence->head_to_head){
                        for(int side=0;side<2;++side){
                            auto scope=graphics.sequence->input_scope(side);const auto direction=pads.player_movement(side);
                            const bool shot=pads.player_held(side,SDL_GAMEPAD_BUTTON_WEST),pass=pads.player_held(side,SDL_GAMEPAD_BUTTON_SOUTH),loft=pads.player_held(side,SDL_GAMEPAD_BUTTON_NORTH);
                            graphics.sequence->set_play_input(direction.x,direction.y);graphics.sequence->set_play_buttons(shot,pass);
                            graphics.sequence->set_penalty_input(direction.y,shot);graphics.sequence->set_restart_input(direction.x,direction.y,shot,pass,loft);
                        }
                    }else{
                    graphics.sequence->set_play_input(move_x,move_y);
                    if(graphics.sequence->team_play)graphics.sequence->set_play_buttons(live_key_shot || pads.held(SDL_GAMEPAD_BUTTON_WEST),live_key_pass || pads.held(SDL_GAMEPAD_BUTTON_SOUTH));
                    graphics.sequence->set_penalty_input(move_y,keys[SDL_SCANCODE_X] || pads.held(SDL_GAMEPAD_BUTTON_WEST));
                    graphics.sequence->set_restart_input(move_x,move_y,keys[SDL_SCANCODE_X] || pads.held(SDL_GAMEPAD_BUTTON_WEST),keys[SDL_SCANCODE_SPACE] || pads.held(SDL_GAMEPAD_BUTTON_SOUTH),keys[SDL_SCANCODE_C] || pads.held(SDL_GAMEPAD_BUTTON_NORTH));
                    }
                }
                else graphics.sequence->clear_play_input();
            }
            if(nav.page==4 && focused && opt.capture.empty() && !opt.frontend_smoke && !opt.actions_smoke && !opt.goals_smoke && !opt.control_smoke && !opt.team_smoke && !opt.penalty_smoke)graphics.advance(elapsed);
            if(opt.frontend_smoke || opt.actions_smoke || opt.goals_smoke || opt.control_smoke || opt.team_smoke || opt.penalty_smoke)graphics.advance(0.05);
            practice.set_input(move_x,move_y);
            if(pad_prompts && focused && nav.page==3 && move_x==0 && move_y==0 && !practice.paused) pads.aim_player(practice);
            const bool tackle_down=keys[SDL_SCANCODE_X] || pads.held(SDL_GAMEPAD_BUTTON_WEST);
            practice.set_tackle_held(tackle_down && focused && nav.page==3);
            const int practice_steps=practice_clock.advance(elapsed,nav.page==3 && focused && !practice.paused && !practice.outside && !practice.match_finished);
            if(!opt.practice_smoke && !opt.opponents_smoke && !opt.gamepad_smoke && opt.capture.empty()) for(int i=0;i<practice_steps;++i) practice.step();
            if((opt.practice_smoke || opt.opponents_smoke || opt.gamepad_smoke) && frames>0) practice.step();
            if(front){const auto& prefs=front->preferences();sound_muted=prefs.muted;
                if(audio){audio->set_muted(sound_muted);audio->set_levels(prefs.effects,prefs.crowd);}
                if(music)music->set_volume(prefs.music);
                if(commentary){
                    if(commentary->language()!=prefs.commentary_language && !commentary->set_language(prefs.commentary_language))front->commentary_unavailable(commentary->language());
                    commentary->set_volume(prefs.commentary);
                }
            }
            if(commentary)commentary->update(*graphics.sequence,elapsed,nav.page==4 && focused && !sound_muted && graphics.action_view && graphics.sequence->full_match && !graphics.replaying && !setup.open && !squad.open && ((!graphics.paused && (!graphics.prematch.active() || !graphics.prematch.paused) && (!graphics.walkoff.active() || !graphics.walkoff.paused)) || (front && front->ending())));
            else graphics.sequence->engine.commentary_count=0;
            if(audio)audio->set_duck(commentary && commentary->speaking());
            if(music)music->set_duck(commentary && commentary->speaking());
            if(audio)audio->update(*graphics.sequence,graphics.camera_pose(),nav.page==4 && graphics.action_view && focused && !graphics.paused && (!graphics.prematch.active() || !graphics.prematch.paused) && (!graphics.walkoff.active() || !graphics.walkoff.paused) && !graphics.replaying && !setup.open && !squad.open,front && nav.page==4 && front->ending() && focused);
            if(front && nav.page==4)front->observe(graphics,focused?elapsed:0);
            if(music){music->select(front && nav.page==4?front->music(*graphics.sequence):OriginalMusic::Track::none);music->update(focused && !sound_muted && !movie.active() && !graphics.exit.active());}
            draw(renderer.get(),texture.get(),nav,tactics,lab,practice,pads,opt.hud_capture.empty()?(graphics.sequence->head_to_head || pad_prompts):false,graphics);
            if(audio && !front && nav.page==4 && (graphics.paused || setup.open)){color(renderer.get(),10,17,27);rect(renderer.get(),24,644,912,20);color(renderer.get(),143,157,174);text(renderer.get(),24,648,(sound_muted?std::string("F8 SOUND OFF"):std::string("F8 SOUND ON"))+"    F9 COMMENTARY "+(commentary && commentary->loaded()?std::to_string(commentary->volume())+"%":"UNAVAILABLE"));}
            if(nav.page==4 && graphics.sequence->head_to_head && !movie.active() && (!front || !front->visible())){
                const float banner_y=graphics.clean_match_view()?4.f:100.f;
                color(renderer.get(),10,20,30);rect(renderer.get(),80,banner_y,800,16);color(renderer.get(),244,208,91);
                const auto& s=*graphics.sequence;
                hud_text(renderer.get(),92,banner_y+4,pads.local_ready()?"P1 "+s.player_label(s.selected_for(0))+"   /   P2 "+s.player_label(s.selected_for(1)):pads.local_status(),776,1.0f);
            }
            if(nav.page==4){setup.draw(renderer.get(),graphics);squad.draw(renderer.get(),graphics);if(front)front->draw(renderer.get(),graphics,opt.frontend_capture.empty()?(graphics.sequence->head_to_head || pad_prompts):false,sound_muted,commentary?commentary->volume():0);}
            if(movie.active())try{
                movie.update((opt.movie_smoke || opt.intro_smoke)?.05:elapsed,focused,!sound_muted,front?front->preferences().music:100);
                movie.draw(renderer.get());
            }catch(const std::exception& error){movie.stop();if(front)front->movie_error="MOVIE UNAVAILABLE";SDL_Log("CD movie: %s",error.what());}
            if(!opt.capture.empty() && !opt.depth_smoke && !opt.penalty_smoke) {
                auto surface=SDL_RenderReadPixels(renderer.get(),nullptr);
                if(!surface) throw std::runtime_error(SDL_GetError());
                auto path=opt.capture.u8string();
                const bool saved=SDL_SaveBMP(surface,reinterpret_cast<const char*>(path.c_str()));
                SDL_DestroySurface(surface);
                check(saved,"Save capture"); running=false;
            }
            if(opt.head_to_head_smoke && (frames==1 || frames==2 || frames==16 || frames==25 || frames==46)){
                if(const char* folder=SDL_getenv("ACTUA96_LOCAL_CAPTURE_DIR")){
                    fs::create_directories(folder);auto* pixels=SDL_RenderReadPixels(renderer.get(),nullptr);check(pixels!=nullptr,"Read local-match capture");
                    const auto path=fs::path(folder)/(std::to_string(frames)+".png");const bool saved=SDL_SavePNG(pixels,path.string().c_str());SDL_DestroySurface(pixels);check(saved,"Save local-match capture");
                }
            }
            check(SDL_RenderPresent(renderer.get()),"Present");
            ++frames;
            if(opt.startup_smoke){
                using Stage=OriginalFrontend::Stage;
                const auto expect=[](bool ok,const char* why){if(!ok)throw std::runtime_error(why);};
                const auto key=[](SDL_Keycode k){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;check(SDL_PushEvent(&e),"Startup key");};
                const auto button=[&](SDL_GamepadButton b,bool down){check(SDL_SetJoystickVirtualButton(virtual_pad,b,down),"Startup controller");};
                expect(!movie.active(),"Euro96 startup must never play an Actua film");
                if(frames<=7)expect(front->stage()==Stage::title && !front->intro_requested && !front->movie_requested && front->movie_error.empty(),"Legacy film shortcuts must leave Euro96 title unchanged");
                if(frames==1)key(SDLK_I);
                if(frames==2)key(SDLK_M);
                if(frames==3)button(SDL_GAMEPAD_BUTTON_NORTH,true);
                if(frames==4)button(SDL_GAMEPAD_BUTTON_NORTH,false);
                if(frames==5)button(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,true);
                if(frames==6)button(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,false);
                if(frames==7)for(int i=0;i<4;++i)key(SDLK_DOWN);
                if(frames==8)key(SDLK_RETURN);
                if(frames==9){expect(front->stage()==Stage::options,"Title navigation reaches options");key(SDLK_ESCAPE);}
                if(frames==10){front->open(graphics);button(SDL_GAMEPAD_BUTTON_SOUTH,true);}
                if(frames==11)button(SDL_GAMEPAD_BUTTON_SOUTH,false);
                if(frames==12){expect(front->stage()==Stage::selection,"Euro96 controller entry reaches exhibition");std::cout<<"Euro96 direct startup, legacy shortcut isolation and menu navigation passed.\n";running=false;}
            }
            if(opt.intro_smoke){
                const auto expect=[](bool ok,const char* why){if(!ok)throw std::runtime_error(why);};
                const auto key=[](SDL_Keycode k){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;SDL_PushEvent(&e);};
                const auto button=[&](SDL_GamepadButton b,bool down){check(SDL_SetJoystickVirtualButton(virtual_pad,b,down),"Intro button");};
                const auto focus=[](Uint32 type){SDL_Event e{};e.type=type;SDL_PushEvent(&e);};
                if(frames==1){expect(movie.active() && movie.intro() && movie.frames()==399,"Startup intro entry");focus(SDL_EVENT_WINDOW_FOCUS_LOST);focus(SDL_EVENT_WINDOW_FOCUS_GAINED);}
                if(frames==2){expect(movie.paused(),"Same-frame focus loss must pause intro");key(SDLK_P);}
                if(frames==3){expect(!movie.paused(),"Intro keyboard resume");key(SDLK_RETURN);}
                if(frames==4){expect(!movie.active() && front->stage()==OriginalFrontend::Stage::title,"Skip must stay on title");button(SDL_GAMEPAD_BUTTON_NORTH,true);}
                if(frames==5)button(SDL_GAMEPAD_BUTTON_NORTH,false);
                if(frames==6){expect(movie.active() && movie.intro(),"Gamepad intro replay");button(SDL_GAMEPAD_BUTTON_START,true);}
                if(frames==7)button(SDL_GAMEPAD_BUTTON_START,false);
                if(frames==8){expect(movie.paused(),"Intro gamepad pause");button(SDL_GAMEPAD_BUTTON_SOUTH,true);}
                if(frames==9)button(SDL_GAMEPAD_BUTTON_SOUTH,false);
                if(frames==10){expect(!movie.active() && front->stage()==OriginalFrontend::Stage::title,"Gamepad skip stays on title");key(SDLK_I);}
                if(frames==11){expect(movie.active() && movie.intro(),"Keyboard intro replay");movie.seek(movie.duration());}
                if(frames==14){expect(!movie.active() && front->stage()==OriginalFrontend::Stage::title,"Intro natural completion");if(fs::exists(audio_root/"VIDEO/MAKING.AMV"))key(SDLK_M);}
                if(frames==15){if(fs::exists(audio_root/"VIDEO/MAKING.AMV"))expect(movie.active() && !movie.intro() && movie.frames()==2869,"Making-of remains independent");SDL_Event e{};e.type=SDL_EVENT_MOUSE_BUTTON_DOWN;e.button.button=SDL_BUTTON_LEFT;SDL_PushEvent(&e);}
                if(frames==16){expect(!movie.active() && front->stage()==OriginalFrontend::Stage::title,"Mouse skip stays on title");key(SDLK_I);}
                if(frames==17){expect(movie.active(),"Intro reentry for disconnect");SDL_CloseJoystick(virtual_pad);virtual_pad=nullptr;check(SDL_DetachVirtualJoystick(virtual_id),"Intro detach");virtual_id=0;}
                if(frames==18){expect(movie.paused(),"Controller disconnect pauses intro");key(SDLK_P);}
                if(frames==19){expect(!movie.paused(),"Keyboard resumes after controller removal");key(SDLK_ESCAPE);}
                if(frames==20){expect(!movie.active() && front->stage()==OriginalFrontend::Stage::title,"Escape skip stays on title");std::cout<<"Original intro startup, replay, keyboard, gamepad, focus and completion passed.\n";running=false;}
            }
            if(opt.movie_smoke){
                const auto key=[](SDL_Keycode k){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;SDL_PushEvent(&e);};
                const auto button=[&](SDL_GamepadButton b,bool down){check(SDL_SetJoystickVirtualButton(virtual_pad,b,down),"Movie button");};
                if(frames==1)key(SDLK_M);
                if(frames==2){if(!movie.active())throw std::runtime_error("Movie title entry");key(SDLK_P);}
                if(frames==3){if(!movie.paused())throw std::runtime_error("Movie keyboard pause");key(SDLK_RIGHT);}
                if(frames==4){if(movie.position()<5)throw std::runtime_error("Movie keyboard seek");button(SDL_GAMEPAD_BUTTON_NORTH,true);}
                if(frames==5)button(SDL_GAMEPAD_BUTTON_NORTH,false);
                if(frames==6){if(movie.position()!=0)throw std::runtime_error("Movie gamepad restart");button(SDL_GAMEPAD_BUTTON_START,true);}
                if(frames==7)button(SDL_GAMEPAD_BUTTON_START,false);
                if(frames==8){if(movie.paused() || movie.position()==0)throw std::runtime_error("Movie gamepad resume");SDL_Event e{};e.type=SDL_EVENT_WINDOW_FOCUS_LOST;SDL_PushEvent(&e);}
                if(frames==9){if(!movie.paused())throw std::runtime_error("Movie focus pause");SDL_Event e{};e.type=SDL_EVENT_WINDOW_FOCUS_GAINED;SDL_PushEvent(&e);}
                if(frames==10){if(!movie.paused())throw std::runtime_error("Movie focus must not auto-resume");button(SDL_GAMEPAD_BUTTON_SOUTH,true);}
                if(frames==11)button(SDL_GAMEPAD_BUTTON_SOUTH,false);
                if(frames==12){if(movie.active() || front->stage()!=OriginalFrontend::Stage::title)throw std::runtime_error("Movie return to title");button(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,true);}
                if(frames==13)button(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,false);
                if(frames==14){if(!movie.active())throw std::runtime_error("Movie pad title entry");movie.seek(movie.duration());}
                if(frames==18){if(movie.active() || front->stage()!=OriginalFrontend::Stage::title)throw std::runtime_error("Movie natural completion");std::cout<<"CD movie title, keyboard, gamepad, focus and return checks passed.\n";running=false;}
            }
            if(opt.replay_file_smoke){
                using Stage=OriginalFrontend::Stage;
                const auto expect=[](bool ok,const char* why){if(!ok)throw std::runtime_error(why);};
                const auto key=[&](SDL_Keycode k){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;check(SDL_PushEvent(&e),"Replay file key");};
                const auto button=[&](SDL_GamepadButton b,bool down){check(SDL_SetJoystickVirtualButton(virtual_pad,b,down),"Replay file pad");};
                if(frames==1){graphics.prematches_enabled=false;key(SDLK_RETURN);}
                if(frames==2)key(SDLK_RETURN);
                if(frames==3)key(SDLK_RETURN);
                if(frames==4){for(int n=0;n<400;++n)graphics.advance(.05);graphics.paused=true;key(SDLK_F4);}
                if(frames==5){expect(graphics.replaying,"Replay opened");key(SDLK_S);}
                if(frames==6){expect(graphics.replay_message=="REPLAY SAVED","Keyboard replay save");button(SDL_GAMEPAD_BUTTON_EAST,true);}
                if(frames==7)button(SDL_GAMEPAD_BUTTON_EAST,false);
                if(frames==8){expect(replay_library(front->save_path().parent_path()/"replays").size()==2,"Controller replay save and unique files");key(SDLK_ESCAPE);}
                if(frames==9){expect(!graphics.replaying && graphics.paused,"Match remains paused");button(SDL_GAMEPAD_BUTTON_EAST,true);}
                if(frames==10){button(SDL_GAMEPAD_BUTTON_EAST,false);expect(front->stage()==Stage::replays && front->replays().size()==2,"Controller library opens");button(SDL_GAMEPAD_BUTTON_SOUTH,true);}
                if(frames==11){expect(graphics.saved_replay_active(),"Saved clip plays");button(SDL_GAMEPAD_BUTTON_SOUTH,false);key(SDLK_END);}
                if(frames==12){expect(graphics.paused && graphics.replay_position()==int(graphics.replay_frames())-1,"Saved clip seek");key(SDLK_V);}
                if(frames==13)button(SDL_GAMEPAD_BUTTON_BACK,true);
                if(frames==14){button(SDL_GAMEPAD_BUTTON_BACK,false);expect(front->stage()==Stage::replays && !graphics.saved_replay_active(),"Return to library");}
                if(frames==15)key(SDLK_ESCAPE);
                if(frames==16){expect(front->stage()==Stage::hidden && graphics.paused && graphics.sequence->ticks>=400,"Return to paused match");key(SDLK_F10);}
                if(frames==17){expect(front->stage()==Stage::replays,"Keyboard library");key(SDLK_RETURN);}
                if(frames==18){expect(graphics.saved_replay_active(),"Keyboard play saved clip");key(SDLK_SPACE);}
                if(frames==19){expect(front->stage()==Stage::replays,"Keyboard exit saved clip");key(SDLK_RETURN);}
                if(frames==20){expect(graphics.saved_replay_active() && !graphics.paused,"Saved clip playing");SDL_Event e{};e.type=SDL_EVENT_WINDOW_FOCUS_LOST;check(SDL_PushEvent(&e),"Replay focus loss");}
                if(frames==21){expect(graphics.paused,"Saved clip focus pause");SDL_Event e{};e.type=SDL_EVENT_WINDOW_FOCUS_GAINED;check(SDL_PushEvent(&e),"Replay focus return");}
                if(frames==22){expect(graphics.paused,"Focus return retains pause");button(SDL_GAMEPAD_BUTTON_START,true);}
                if(frames==23){button(SDL_GAMEPAD_BUTTON_START,false);expect(!graphics.paused,"Controller resumes saved clip");}
                if(frames==24){SDL_CloseJoystick(virtual_pad);virtual_pad=nullptr;check(SDL_DetachVirtualJoystick(virtual_id),"Replay controller disconnect");virtual_id=0;}
                if(frames==25){expect(graphics.paused,"Controller disconnect pauses saved clip");std::cout<<"Replay file keyboard/controller save, unique clips, library, seek, camera, focus/disconnect and paused-match return passed.\n";running=false;}
            }
            if(opt.head_to_head_smoke){
                auto& s=*graphics.sequence;auto& e=s.engine;
                auto key=[](SDL_Keycode k){SDL_Event ev{};ev.type=SDL_EVENT_KEY_DOWN;ev.key.key=k;check(SDL_PushEvent(&ev),"Local match key");};
                auto button=[](SDL_Joystick* pad,SDL_GamepadButton b,bool down){check(SDL_SetJoystickVirtualButton(pad,b,down),"Local controller button");};
                auto fixture=[&](bool away_ball){
                    s.start_match(false);s.restart_mode=s.restart_ticks=0;e.match_mode=e.set_piece_on=e.dead_ball_cnt=e.game_action=e.ball_out_of_play=0;e.reset_shot();e.holder_lose_ball();
                    for(int i=0;i<22;++i){auto& p=e.teams[i];p.guy_on=0;p.control=0;p.tm_x=i<11?400.f:880.f;p.tm_y=400;e.reset_ideas(&p);e.init_stand_act(&p);}
                    for(int i:{1,2,12,13}){auto& p=e.teams[i];p.guy_on=1;p.tm_xdis=p.mface_x=i<11?1.f:-1.f;p.tm_ydis=p.mface_y=0;p.tm_y=i%11==1?400.f:500.f;p.tm_rate=p.tm_pow=p.tm_cont=p.tm_flair=p.tm_vis=p.tm_ac=96;p.tm_mcspd=1.5f;p.tm_jump=40;}
                    s.selected=1;{auto scope=s.input_scope(1);s.selected=12;}
                    auto& owner=e.teams[away_ball?12:1];if(away_ball)owner.tm_x=330;e.ball_inair=0;e.ballz=2;e.collect_ball(&owner);e.hold_ball(&owner);
                    graphics.paused=false;graphics.opening_input_guard=false;
                };
                if(frames==1)key(SDLK_RETURN);
                if(frames==2){if(front->connected_controllers!=2)throw std::runtime_error("Local setup needs two pads");key(SDLK_H);}
                if(frames==3){if(front->stage()!=OriginalFrontend::Stage::lineup || !s.next_head_to_head)throw std::runtime_error("Head-to-head lineup choice");key(SDLK_RETURN);}
                if(frames==4){if(!s.head_to_head || s.cpu_vs_cpu || !pads.local_ready())throw std::runtime_error("Local match launch and device binding");key(SDLK_RETURN);}
                if(frames==8){fixture(false);SDL_SetJoystickVirtualAxis(virtual_pad,0,32767);SDL_SetJoystickVirtualAxis(local_second_pad,0,-32768);}
                if(frames==18){
                    if(e.teams[1].tm_x<=400 || e.teams[12].tm_x>=880 || e.teams[1].control!=1 || e.teams[12].control!=2)throw std::runtime_error("Simultaneous local SDL movement");
                    SDL_SetJoystickVirtualAxis(virtual_pad,0,0);SDL_SetJoystickVirtualAxis(local_second_pad,0,0);
                    local_first_selected=s.selected_for(0);local_second_selected=s.selected_for(1);button(local_second_pad,SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,true);
                }
                if(frames==19){if(s.selected_for(0)!=local_first_selected || s.selected_for(1)==local_second_selected)throw std::runtime_error("Independent local player switch");button(local_second_pad,SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,false);}
                if(frames==20)button(local_second_pad,SDL_GAMEPAD_BUTTON_START,true);
                if(frames==21){if(!graphics.paused)throw std::runtime_error("P2 pauses match");local_pause_tick=s.ticks;button(local_second_pad,SDL_GAMEPAD_BUTTON_START,false);button(virtual_pad,SDL_GAMEPAD_BUTTON_SOUTH,true);}
                if(frames==24){if(s.ticks!=local_pause_tick)throw std::runtime_error("Pause must freeze shared match");SDL_CloseJoystick(local_second_pad);local_second_pad=nullptr;SDL_DetachVirtualJoystick(local_second_id);local_second_id=0;}
                if(frames==26)button(virtual_pad,SDL_GAMEPAD_BUTTON_START,true);
                if(frames==27){if(!graphics.paused || pads.local_ready() || s.ticks!=local_pause_tick)throw std::runtime_error("Missing P2 must prevent resume: paused="+std::to_string(graphics.paused)+" ready="+std::to_string(pads.local_ready())+" ticks="+std::to_string(s.ticks)+" expected="+std::to_string(local_pause_tick)+" replay="+std::to_string(graphics.replaying)+" ids="+std::to_string(pads.player_id(0))+","+std::to_string(pads.player_id(1)));button(virtual_pad,SDL_GAMEPAD_BUTTON_START,false);button(virtual_pad,SDL_GAMEPAD_BUTTON_SOUTH,false);attach_local_second();}
                if(frames==29)button(local_second_pad,SDL_GAMEPAD_BUTTON_START,true);
                if(frames==30){if(!pads.local_ready() || !graphics.paused || pads.player_for(local_second_id)!=1)throw std::runtime_error("Replacement joins P2 without auto-resume");button(local_second_pad,SDL_GAMEPAD_BUTTON_START,false);}
                if(frames==32)button(local_second_pad,SDL_GAMEPAD_BUTTON_START,true);
                if(frames==33){if(graphics.paused)throw std::runtime_error("Reconnected P2 resumes");button(local_second_pad,SDL_GAMEPAD_BUTTON_START,false);}
                if(frames==40)fixture(true);
                if(frames==42)button(local_second_pad,SDL_GAMEPAD_BUTTON_WEST,true);
                if(frames==50){auto scope=s.input_scope(1);if(s.live_charge()!=8 || e.ball_poss!=13)throw std::runtime_error("P2 held shot uses independent charge");button(local_second_pad,SDL_GAMEPAD_BUTTON_WEST,false);}
                if(frames==88){if(e.teams[12].tm_shots!=1)throw std::runtime_error("P2 actual shot contact");s.interval_ticks=1;s.interval_minute=45;e.match_half=1;}
                if(frames==89){if(!s.is_blue(s.selected_for(0)) || s.is_blue(s.selected_for(1)) || s.selected_for(s.human_for(e.setp_taker-1))!=e.setp_taker-1 || pads.player_for(virtual_id)!=0 || pads.player_for(local_second_id)!=1)throw std::runtime_error("Half-time keeps controller-to-team assignments and selects the kickoff taker");}
                if(frames==90){SDL_Event ev{};ev.type=SDL_EVENT_WINDOW_FOCUS_LOST;SDL_PushEvent(&ev);}
                if(frames==91){if(!graphics.paused)throw std::runtime_error("Local focus loss pauses both players");SDL_Event ev{};ev.type=SDL_EVENT_WINDOW_FOCUS_GAINED;SDL_PushEvent(&ev);}
                if(frames==92){if(!graphics.paused)throw std::runtime_error("Focus return must not resume local match");button(virtual_pad,SDL_GAMEPAD_BUTTON_START,true);}
                if(frames==93){if(graphics.paused)throw std::runtime_error("P1 resumes after focus return");button(virtual_pad,SDL_GAMEPAD_BUTTON_START,false);}
                if(frames==96){std::cout<<"Local head-to-head SDL setup, two-pad movement/switching, pause, disconnect, replacement, P2 shot and half-time passed.\n";running=false;}
            }
            if(opt.frontend_smoke && !opt.startup_smoke && !opt.replay_file_smoke && !opt.head_to_head_smoke){
                using Stage=OriginalFrontend::Stage;auto& s=*graphics.sequence;
                const auto key=[](SDL_Keycode k){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;check(SDL_PushEvent(&e),"Front-end key injection");};
                if(frames==1){if(front->stage()!=Stage::title)throw std::runtime_error("Missing title");check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_SOUTH,true),"Title confirm");}
                if(frames==2)check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_SOUTH,false),"Title release");
                if(frames==3){if(front->stage()!=Stage::selection)throw std::runtime_error("Gamepad title transition");key(SDLK_RIGHT);}
                if(frames==4)key(SDLK_Y);
                if(frames==5){if(front->stage()!=Stage::lineup || !graphics.paused)throw std::runtime_error("Pre-match review");key(SDLK_RETURN);}
                if(frames==6 && graphics.prematch.active()){if(s.ticks)throw std::runtime_error("Opening advanced live kickoff");check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_SOUTH,true),"Opening skip");}
                if(frames==6 && (front->visible() || !s.full_match || s.cpu_vs_cpu))throw std::runtime_error("Human kick-off transition");
                if(frames==8 && graphics.has_cd_stadium() && (graphics.prematch.active() || !graphics.opening_input_guard || !s.full_match))throw std::runtime_error("Held opening skip must not become gameplay input");
                if(frames==9)check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_SOUTH,false),"Opening skip release");
                if(frames==10 && graphics.opening_input_guard)throw std::runtime_error("Opening input re-arms after release");
                if(opt.commentary_smoke){
                    if(frames==6 && (!commentary || !commentary->ready() || !commentary->loaded()))throw std::runtime_error("Commentary device/assets unavailable");
                    if(frames==7)s.engine.PlayCommentaryMessage(22);
                    if(frames==8){if(!commentary->speaking())throw std::runtime_error("Goal commentary not playing");key(SDLK_F9);}
                    if(frames==9){if(commentary->volume()!=50)throw std::runtime_error("Commentary volume key failed");key(SDLK_F8);}
                    if(frames==10){if(commentary->speaking() || commentary->pending())throw std::runtime_error("Master mute did not flush speech");key(SDLK_F8);}
                    if(frames==11){key(SDLK_F9);key(SDLK_F9);}
                    if(frames==12){if(commentary->volume()!=0)throw std::runtime_error("Independent speech mute failed");key(SDLK_F9);}
                    if(frames==13 && commentary->volume()!=100)throw std::runtime_error("Speech volume wrap failed");
                }
                // Exercise the real controller/event route, including paused play:
                // the former Back shortcut replaced exhibitions with a preview.
                static int back_guard_ticks=0;
                if(frames==14){back_guard_ticks=s.ticks;check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_BACK,true),"Live match Back");}
                if(frames==15){
                    if(front->visible() || !s.full_match || !s.team_play || !s.play || graphics.paused || s.ticks<back_guard_ticks)throw std::runtime_error("Back replaced or paused live match");
                    check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_BACK,false),"Live Back release");
                    check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_START,true),"Match pause");
                }
                if(frames==16){
                    if(!graphics.paused)throw std::runtime_error("Start must pause match");back_guard_ticks=s.ticks;
                    check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_START,false),"Pause release");
                    check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_BACK,true),"Paused match Back");
                }
                if(frames==17){
                    if(front->visible() || !s.full_match || !graphics.paused || s.ticks!=back_guard_ticks)throw std::runtime_error("Back replaced or resumed paused match");
                    check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_BACK,false),"Paused Back release");
                    check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_START,true),"Match resume");
                }
                if(frames==18){
                    if(graphics.paused || !s.full_match)throw std::runtime_error("Start must resume preserved match");
                    check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_START,false),"Resume release");
                    std::cout<<"Controller Back preserves live and paused matches; Start pause/resume passed.\n";
                }
                if(frames==30){s.interval_ticks=100;s.interval_minute=45;}
                if(frames==31){if(front->stage()!=Stage::interval || !graphics.paused)throw std::runtime_error("Half-time presentation");if(graphics.exit.active())key(SDLK_RETURN);key(SDLK_RETURN);}
                if(frames==33 && (front->visible() || graphics.paused || s.engine.match_half!=1))throw std::runtime_error("Half-time continuation");
                if(frames==35)s.finish_match();
                if(frames==36){if(front->stage()!=Stage::result)throw std::runtime_error("Result presentation");if(graphics.exit.active())key(SDLK_RETURN);key(SDLK_F8);}
                if(frames==37){if(!audio || !audio->muted())throw std::runtime_error("Menu mute");if(music && music->uses_retail_theme() && music->track()!=OriginalMusic::Track::menu)throw std::runtime_error("Euro result must resume frontend theme");key(SDLK_F8);}
                if(frames==38)key(SDLK_R);
                if(frames==39){if(front->stage()!=Stage::lineup)throw std::runtime_error("Rematch preview");key(SDLK_ESCAPE);}
                if(frames==40){if(front->stage()!=Stage::selection || !music || !music->ready() || music->track()!=OriginalMusic::Track::menu)throw std::runtime_error("Return to menu music");key(SDLK_O);}
                if(frames==41){if(front->stage()!=Stage::options)throw std::runtime_error("Options shortcut");key(SDLK_Y);key(SDLK_LEFT);}
                if(frames==42){if(music->volume()!=75)throw std::runtime_error("Music level integration");key(SDLK_DOWN);key(SDLK_LEFT);}
                if(frames==43){if(audio->effects_volume()!=75)throw std::runtime_error("Effects level integration");key(SDLK_DOWN);key(SDLK_LEFT);}
                if(frames==44){if(audio->crowd_volume()!=75)throw std::runtime_error("Crowd level integration");key(SDLK_DOWN);key(SDLK_LEFT);}
                if(frames==45){if(commentary && commentary->volume()!=front->preferences().commentary)throw std::runtime_error("Commentary level integration");key(SDLK_DOWN);key(SDLK_RIGHT);}
                if(frames==46){if(!audio->muted())throw std::runtime_error("Options master mute");key(SDLK_F8);}
                const bool french=fs::exists(audio_root/"COMMENTARY/FRENCH.BIN");
                if(frames==47){if(audio->muted() || music->volume()!=75 || audio->crowd_volume()!=75)throw std::runtime_error("Unmute preserves channel levels");if(french){key(SDLK_DOWN);key(SDLK_RIGHT);}else key(SDLK_RETURN);}
                if(frames==48){if(!french){if(front->stage()!=Stage::selection)throw std::runtime_error("Options return");running=false;}
                    else {if(!commentary || commentary->language()!=1 || front->preferences().commentary_language!=1)throw std::runtime_error("French voice keyboard integration");check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_DPAD_LEFT,true),"Voice language pad switch");}}
                if(frames==49){check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_DPAD_LEFT,false),"Voice language pad release");if(commentary->language()!=0 || front->preferences().commentary_language!=0)throw std::runtime_error("English voice gamepad integration");key(SDLK_RETURN);}
                if(frames==50){if(front->stage()!=Stage::selection)throw std::runtime_error("Options return");std::cout<<"French/English voice keyboard and gamepad integration passed.\n";if(graphics.has_cd_stadium())key(SDLK_Y);else running=false;}
                if(frames==51)key(SDLK_RETURN);
                if(frames==52){if(graphics.prematch.active())key(SDLK_RETURN);s.interval_ticks=100;s.interval_minute=45;}
                if(frames==53){if(!graphics.exit.active())throw std::runtime_error("Native half-time tunnel entry");SDL_Event e{};e.type=SDL_EVENT_WINDOW_FOCUS_LOST;SDL_PushEvent(&e);e.type=SDL_EVENT_WINDOW_FOCUS_GAINED;SDL_PushEvent(&e);}
                if(frames==54){if(!graphics.exit.paused)throw std::runtime_error("Tunnel must remain paused after same-frame focus return");check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_START,true),"Tunnel resume");}
                if(frames==55){if(graphics.exit.paused)throw std::runtime_error("Tunnel gamepad resume");check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_START,false),"Tunnel resume release");check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_SOUTH,true),"Tunnel skip");}
                if(frames==56){if(graphics.exit.active() || front->stage()!=Stage::interval)throw std::runtime_error("Tunnel skip must open team talk");check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_SOUTH,false),"Tunnel skip release");key(SDLK_RETURN);}
                if(frames==57){if(front->visible() || s.engine.match_half!=1)throw std::runtime_error("Tunnel continue must reach second half");std::cout<<"Native tunnel focus, controller pause and skip passed.\n";running=false;}
            }
            if(opt.setpiece_smoke){
                auto& s=*graphics.sequence;auto& e=s.engine;
                const auto button=[&](bool down){check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_WEST,down),"Set-piece button");};
                if(frames>2200)throw std::runtime_error("Set-piece input smoke timed out: fixture="+std::to_string(s.restart_practice)+" phase="+std::to_string(penalty_smoke_stage)+" mode="+std::to_string(e.match_mode)+" released="+std::to_string(setpiece_smoke_released));
                ++penalty_smoke_age;
                if(penalty_smoke_stage==0){SDL_Event ev{};ev.type=SDL_EVENT_KEY_DOWN;ev.key.key=SDLK_F5;check(SDL_PushEvent(&ev),"Practice shortcut");penalty_smoke_stage=1;}
                else if(penalty_smoke_stage==1 && ((s.human_restart() && e.dead_ball_cnt<=1) || s.human_keeper_hold())){button(true);penalty_smoke_stage=2;penalty_smoke_age=0;setpiece_smoke_released=false;}
                else if(penalty_smoke_stage==2 && penalty_smoke_age==8){
                    if(s.restart_practice<4 && e.setp_hgt!=8)throw std::runtime_error("Set-piece gamepad charge failed");
                    if(s.restart_practice==1 && !opt.capture.empty()){
                        auto* pixels=SDL_RenderReadPixels(renderer.get(),nullptr);check(pixels!=nullptr,"Set-piece HUD capture");check(SDL_SaveBMP(pixels,opt.capture.string().c_str()),"Save set-piece HUD");SDL_DestroySurface(pixels);
                    }
                    button(false);penalty_smoke_stage=3;penalty_smoke_age=0;
                }else if(penalty_smoke_stage==3){
                    // Observe the tested restart ending, even if subsequent play
                    // awards another human restart during the settling period.
                    if(!e.match_mode)setpiece_smoke_released=true;
                    if(penalty_smoke_age>100 && setpiece_smoke_released){
                        std::cout<<"Set-piece fixture "<<s.restart_practice<<" released; subsequent mode="<<int(e.match_mode)<<".\n";
                        if(s.restart_practice==6){if(s.distributions<1)throw std::runtime_error("Keeper gamepad punt failed");running=false;}
                        else penalty_smoke_stage=0;
                    }
                }
            }
            if(opt.penalty_smoke && !opt.setpiece_smoke){
                auto& s=*graphics.sequence;auto& e=s.engine;
                const auto button=[&](bool down){check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_WEST,down),"Penalty button");};
                const auto key=[&](SDL_Keycode code){SDL_Event ev{};ev.type=SDL_EVENT_KEY_DOWN;ev.key.key=code;check(SDL_PushEvent(&ev),"Penalty key");};
                if(frames>2500)throw std::runtime_error("Penalty input smoke timed out");
                ++penalty_smoke_age;
                switch(penalty_smoke_stage){
                case 0:if(s.penalty_human_taker() && s.penalty_aiming()){button(true);++penalty_smoke_stage;penalty_smoke_age=0;}break;
                case 1:if(penalty_smoke_age==8){if(e.setp_hgt!=8)throw std::runtime_error("Gamepad penalty charge failed");key(SDLK_P);++penalty_smoke_stage;penalty_smoke_age=0;}break;
                case 2:if(penalty_smoke_age==3){if(!graphics.paused || e.f1_down)throw std::runtime_error("Penalty pause must cancel charge");key(SDLK_P);++penalty_smoke_stage;penalty_smoke_age=0;}break;
                case 3:if(penalty_smoke_age==4){if(e.f1_down || !s.penalty_aiming())throw std::runtime_error("Held resume fired penalty");button(false);++penalty_smoke_stage;penalty_smoke_age=0;}break;
                case 4:if(penalty_smoke_age==2){button(true);++penalty_smoke_stage;penalty_smoke_age=0;}break;
                case 5:if(penalty_smoke_age==12){
                    if(e.setp_hgt!=12)throw std::runtime_error("Penalty rearmed charge failed");
                    if(!opt.capture.empty()){
                        auto pixels=SDL_RenderReadPixels(renderer.get(),nullptr);check(pixels!=nullptr,"Read penalty UI");
                        const auto path=opt.capture.u8string();const bool saved=SDL_SaveBMP(pixels,reinterpret_cast<const char*>(path.c_str()));SDL_DestroySurface(pixels);check(saved,"Save penalty UI");
                    }
                    SDL_Event ev{};ev.type=SDL_EVENT_WINDOW_FOCUS_LOST;check(SDL_PushEvent(&ev),"Penalty focus loss");
                    ++penalty_smoke_stage;penalty_smoke_age=0;
                }break;
                case 6:{
                    if(!graphics.paused || focused || e.f1_down)throw std::runtime_error("Focus loss must cancel penalty charge and pause");
                    SDL_Event ev{};ev.type=SDL_EVENT_WINDOW_FOCUS_GAINED;check(SDL_PushEvent(&ev),"Penalty focus gain");key(SDLK_P);
                    ++penalty_smoke_stage;penalty_smoke_age=0;break;
                }
                case 7:if(penalty_smoke_age==4){if(e.f1_down || !s.penalty_aiming())throw std::runtime_error("Focus resume held button fired penalty");button(false);++penalty_smoke_stage;penalty_smoke_age=0;}break;
                case 8:if(penalty_smoke_age==2){button(true);++penalty_smoke_stage;penalty_smoke_age=0;}break;
                case 9:if(penalty_smoke_age==12){if(e.setp_hgt!=12)throw std::runtime_error("Charge after focus return failed");button(false);++penalty_smoke_stage;penalty_smoke_age=0;}break;
                case 10:if(s.penalty_released){
                    if(e.ball_poss || e.ball_limbo_on || s.releases<1)throw std::runtime_error("Human gamepad shot did not release");
                    penalty_detached_id=virtual_id;
                    SDL_CloseJoystick(virtual_pad);virtual_pad=nullptr;check(SDL_DetachVirtualJoystick(virtual_id),"Penalty disconnect");virtual_id=0;
                    ++penalty_smoke_stage;penalty_smoke_age=0;
                }break;
                case 11:if(penalty_smoke_age==2){if(!graphics.paused || pads.active_id()==penalty_detached_id)throw std::runtime_error("Penalty disconnect must pause and remove the active device");running=false;}break;
                }
            }
            if(opt.depth_smoke){
                const auto key=[&](SDL_Keycode code){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=code;check(SDL_PushEvent(&e),"Depth key");};
                const auto button=[&](SDL_GamepadButton code,bool down){check(SDL_SetJoystickVirtualButton(virtual_pad,int(code),down),"Depth button");};
                auto& s=*graphics.sequence;
                if(frames==1)key(SDLK_F3);
                if(frames==2){if(!squad.open || !graphics.paused)throw std::runtime_error("Squad pause failed");key(SDLK_DOWN);}
                if(frames==3){key(SDLK_RETURN);
                    if(!opt.capture.empty()){auto* image=SDL_RenderReadPixels(renderer.get(),nullptr);check(image!=nullptr,"Squad capture");check(SDL_SavePNG(image,opt.capture.string().c_str()),"Save squad capture");SDL_DestroySurface(image);}
                }
                if(frames==4){if(s.pending_subs[0].shirt!=13 || s.engine.teams[1].shirt!=2)throw std::runtime_error("Keyboard substitution queue failed");key(SDLK_F3);}
                if(frames==5){if(squad.open || graphics.paused)throw std::runtime_error("Squad resume failed");}
                if(frames==40){s.engine.match_mode=0;s.restart_ticks=1;}
                if(frames==42){if(s.engine.teams[1].shirt!=13 || s.substitutions[0]!=1)throw std::runtime_error("Queued substitution did not apply at stoppage");key(SDLK_P);}
                if(frames==43)button(SDL_GAMEPAD_BUTTON_NORTH,true);
                if(frames==44)button(SDL_GAMEPAD_BUTTON_NORTH,false);
                if(frames==45){if(!squad.open || !graphics.paused)throw std::runtime_error("Gamepad squad entry failed");button(SDL_GAMEPAD_BUTTON_DPAD_DOWN,true);}
                if(frames==46)button(SDL_GAMEPAD_BUTTON_DPAD_DOWN,false);
                if(frames==47)button(SDL_GAMEPAD_BUTTON_DPAD_RIGHT,true);
                if(frames==48)button(SDL_GAMEPAD_BUTTON_DPAD_RIGHT,false);
                if(frames==49)button(SDL_GAMEPAD_BUTTON_SOUTH,true);
                if(frames==50)button(SDL_GAMEPAD_BUTTON_SOUTH,false);
                if(frames==51){if(s.pending_subs[0].shirt!=14)throw std::runtime_error("Gamepad substitution queue failed");button(SDL_GAMEPAD_BUTTON_BACK,true);}
                if(frames==52)button(SDL_GAMEPAD_BUTTON_BACK,false);
                if(frames==53){if(squad.open || !graphics.paused)throw std::runtime_error("Squad must retain previous pause state");button(SDL_GAMEPAD_BUTTON_WEST,true);}
                if(frames==54)button(SDL_GAMEPAD_BUTTON_WEST,false);
                if(frames==55){if(!graphics.replaying)throw std::runtime_error("Gamepad replay start failed");button(SDL_GAMEPAD_BUTTON_SOUTH,true);}
                if(frames==56)button(SDL_GAMEPAD_BUTTON_SOUTH,false);
                if(frames==57){if(graphics.replaying || !graphics.paused)throw std::runtime_error("Replay skip must retain pause");key(SDLK_F4);}
                if(frames==58 && !graphics.replaying)throw std::runtime_error("Keyboard replay failed");
                if(frames==59)key(SDLK_P);
                if(frames==60)key(SDLK_END);
                if(frames==61){if(!graphics.paused || graphics.replay_position()!=int(graphics.replay_frames())-1)throw std::runtime_error("Replay end seek failed");key(SDLK_LEFT);}
                if(frames==62){if(graphics.replay_position()!=int(graphics.replay_frames())-2)throw std::runtime_error("Replay backward frame failed");key(SDLK_DOWN);}
                if(frames==63){if(graphics.replay_speed()!=4)throw std::runtime_error("Keyboard replay speed failed");button(SDL_GAMEPAD_BUTTON_DPAD_DOWN,true);}
                if(frames==64)button(SDL_GAMEPAD_BUTTON_DPAD_DOWN,false);
                if(frames==65){if(graphics.replay_speed()!=3)throw std::runtime_error("Gamepad replay speed failed");button(SDL_GAMEPAD_BUTTON_WEST,true);}
                if(frames==66)button(SDL_GAMEPAD_BUTTON_WEST,false);
                if(frames==67){if(!graphics.replay_looping())throw std::runtime_error("Gamepad replay loop failed");button(SDL_GAMEPAD_BUTTON_NORTH,true);}
                if(frames==68)button(SDL_GAMEPAD_BUTTON_NORTH,false);
                if(frames==69){if(graphics.replay_position()!=0 || !graphics.paused)throw std::runtime_error("Gamepad replay seek failed");button(SDL_GAMEPAD_BUTTON_RIGHT_STICK,true);}
                if(frames==70)button(SDL_GAMEPAD_BUTTON_RIGHT_STICK,false);
                if(frames==71){
                    if(graphics.match_camera!=1)throw std::runtime_error("Replay camera failed");
                    SDL_Event click{};click.type=SDL_EVENT_MOUSE_BUTTON_DOWN;click.button.button=SDL_BUTTON_LEFT;click.button.windowID=SDL_GetWindowID(window.get());
                    check(SDL_RenderCoordinatesToWindow(renderer.get(),replay_bar_left+replay_bar_width/2,replay_bar_top+5,&click.button.x,&click.button.y),"Replay timeline coordinates");
                    SDL_PushEvent(&click);
                }
                if(frames==72){if(graphics.replay_position()!=int(std::lround((graphics.replay_frames()-1)/2.0)))throw std::runtime_error("Replay timeline click failed: frame "+std::to_string(graphics.replay_position())+" / "+std::to_string(graphics.replay_frames()));key(SDLK_SPACE);}
                if(frames==73){if(graphics.replaying || !graphics.paused || graphics.match_camera!=0)throw std::runtime_error("Replay exit must restore pause and camera");running=false;}
            }
            if(opt.gamepad_smoke) {
                const auto press=[&](SDL_GamepadButton button,bool down){check(SDL_SetJoystickVirtualButton(virtual_pad,button,down),"Virtual gamepad button");};
                if(frames==1) check(SDL_SetJoystickVirtualAxis(virtual_pad,SDL_GAMEPAD_AXIS_LEFTX,32767),"Virtual stick movement");
                if(frames==10) {
                    if(practice.engine.teams[1].tm_x<=510 || !pad_prompts) throw std::runtime_error("Gamepad movement/prompts failed");
                    press(SDL_GAMEPAD_BUTTON_SOUTH,true); // Keep movement held through reception.
                }
                if(frames==11) press(SDL_GAMEPAD_BUTTON_SOUTH,false);
                if(frames==45) {
                    if(practice.engine.ball_poss!=3) throw std::runtime_error("Gamepad pass was not received");
                    const auto& receiver=practice.engine.teams[2];
                    if(std::hypot(receiver.go_txdis,receiver.go_tydis)<4) throw std::runtime_error("Held gamepad input did not carry through reception");
                    check(SDL_SetJoystickVirtualAxis(virtual_pad,SDL_GAMEPAD_AXIS_LEFTX,0),"Release virtual stick");
                    check(SDL_SetJoystickVirtualAxis(virtual_pad,SDL_GAMEPAD_AXIS_RIGHTY,-32768),"Virtual shot aim");press(SDL_GAMEPAD_BUTTON_WEST,true);
                }
                if(frames==46) press(SDL_GAMEPAD_BUTTON_WEST,false);
                if(frames==47) {
                    if(practice.engine.ballydis>=0 || practice.engine.teams[2].tm_shots!=1) throw std::runtime_error("Gamepad aimed shot failed");
                    press(SDL_GAMEPAD_BUTTON_START,true);
                }
                if(frames==48) {
                    if(!practice.paused) throw std::runtime_error("Gamepad pause failed");
                    press(SDL_GAMEPAD_BUTTON_START,false);press(SDL_GAMEPAD_BUTTON_RIGHT_STICK,true);
                }
                if(frames==49) {
                    if(practice.paused || practice.engine.ball_poss!=2) throw std::runtime_error("Gamepad reset failed");
                    press(SDL_GAMEPAD_BUTTON_RIGHT_STICK,false);press(SDL_GAMEPAD_BUTTON_BACK,true);
                }
                if(frames==50) press(SDL_GAMEPAD_BUTTON_BACK,false);
                if(frames==51) {
                    if(!practice.opponents) throw std::runtime_error("Gamepad mode toggle failed");
                    SDL_CloseJoystick(virtual_pad);virtual_pad=nullptr;
                    check(SDL_DetachVirtualJoystick(virtual_id),"Unplug virtual controller");virtual_id=0;
                }
                if(frames==53) {
                    if(!practice.paused || pad_prompts) throw std::runtime_error("Gamepad disconnect did not pause");
                    running=false;
                }
            }
            if(opt.opponents_smoke) {
                if(frames==100) {
                    if(practice.engine.ball_poss<12 || practice.red_wins==0)
                        throw std::runtime_error("Opponent pressure did not win possession");
                    // Place a reachable holder to verify the real X event wins a tackle.
                    practice.reset();practice.engine.holder_lose_ball();
                    auto& red=practice.engine.teams[12];red.tm_x=520;red.tm_y=400;red.tm_xdis=-1;red.tm_ydis=0;
                    practice.engine.collect_ball(&red);
                    SDL_Event injected{};injected.type=SDL_EVENT_KEY_DOWN;injected.key.key=SDLK_X;
                    check(SDL_PushEvent(&injected),"Inject tackle");
                }
                if(frames==101) {
                    if(practice.engine.ball_poss!=2 || practice.blue_wins!=1)
                        throw std::runtime_error("Tackle input failed");
                }
                if(frames==101 || frames==102 || frames==103) {
                    if(frames==102 && practice.opponents) throw std::runtime_error("Opponents toggle off failed");
                    if(frames==103 && !practice.opponents) throw std::runtime_error("Opponents toggle on failed");
                    SDL_Event injected{};injected.type=SDL_EVENT_KEY_DOWN;injected.key.key=frames==103?SDLK_R:SDLK_O;
                    check(SDL_PushEvent(&injected),"Inject drill reset/toggle");
                }
                if(frames==104) {
                    if(!practice.opponents || practice.engine.ball_poss!=2 || practice.blue_wins || practice.red_wins)
                        throw std::runtime_error("Opponent reset failed");
                    practice.engine.teams[1].tm_x=1180;practice.engine.teams[1].tm_y=400;
                    practice.engine.hold_ball(&practice.engine.teams[1]);
                    practice.keepers[1].y=364;practice.aim_at(1280,424);
                    SDL_Event injected{};injected.type=SDL_EVENT_KEY_DOWN;injected.key.key=SDLK_X;
                    check(SDL_PushEvent(&injected),"Inject scoring shot");
                }
                if(frames==120 && (practice.blue_score!=1 || practice.restart_ticks<=0))
                    throw std::runtime_error("Scoring shot or goal celebration failed");
                if(frames==155) {
                    if(practice.blue_score!=1 || practice.restart_ticks || practice.engine.ball_poss!=13)
                        throw std::runtime_error("Automatic conceding-team kickoff failed");
                    // Continue through a real AI pass and receiving run while rendering.
                    practice.reset();practice.engine.holder_lose_ball();
                    auto& red=practice.engine.teams[12];red.tm_x=900;red.tm_y=400;red.tm_xdis=-1;red.tm_ydis=0;
                    practice.engine.teams[13].tm_x=670;practice.engine.teams[13].tm_y=400;
                    practice.engine.teams[1].tm_x=1000;practice.engine.teams[1].tm_y=400;
                    practice.engine.teams[2].tm_x=1050;practice.engine.teams[2].tm_y=650;
                    practice.engine.collect_ball(&red);
                }
                if(frames==166) {
                    const auto& teammate=practice.engine.teams[2];
                    if(std::hypot(teammate.tm_x-1050,teammate.tm_y-650)<10 || practice.engine.teams[1].tm_x!=1000)
                        throw std::runtime_error("Native teammate defence/manual-control failed");
                }
                if(frames==235) {
                    if(!practice.red_passes || !practice.red_completed || practice.selected!=1)
                        throw std::runtime_error("Native opponent pass/reception failed");
                    practice.reset();practice.engine.holder_lose_ball();
                    practice.engine.ballx=60;practice.engine.bally=400;practice.engine.ballz=2;
                    practice.engine.ballxdis=-12;practice.engine.ballydis=practice.engine.ballzdis=0;
                    practice.engine.last_touch=13;
                }
                if(frames==242 && (practice.keeper_holding!=0 || practice.keepers[0].saves!=1))
                    throw std::runtime_error("Native keeper catch failed");
                if(frames==270) {
                    if(practice.keeper_holding>=0 || practice.engine.ballxdis<=0)
                        throw std::runtime_error("Native keeper release failed");
                    practice.reset();practice.engine.holder_lose_ball();
                    auto& red=practice.engine.teams[12];red.tm_x=100;red.tm_y=400;red.tm_xdis=-1;red.tm_ydis=0;
                    practice.engine.teams[1].tm_x=800;practice.engine.teams[1].tm_y=150;
                    practice.engine.teams[2].tm_x=950;practice.engine.teams[2].tm_y=650;
                    practice.engine.collect_ball(&red);red.tm_poss=20;practice.keepers[0].y=364;
                }
                if(frames==285 && (practice.red_score!=1 || !practice.restart_ticks || practice.engine.teams[12].tm_shots!=1))
                    throw std::runtime_error("Native deliberate red goal failed");
                if(frames==325) {
                    if(practice.red_score!=1 || practice.restart_ticks || practice.engine.ball_poss!=2)
                        throw std::runtime_error("Native blue kickoff after red goal failed");
                    practice.match_elapsed_ticks=PlayerPractice::match_duration_ticks-1;
                }
                if(frames==327) {
                    if(!practice.match_finished || practice.match_seconds_left()!=0 || practice.match_result()!="RED WINS")
                        throw std::runtime_error("Native full-time result failed");
                    SDL_Event injected{};injected.type=SDL_EVENT_KEY_DOWN;injected.key.key=SDLK_R;
                    check(SDL_PushEvent(&injected),"Inject rematch");
                }
                if(frames==329) {
                    if(practice.match_finished || practice.red_score || practice.blue_score || practice.match_seconds_left()!=180)
                        throw std::runtime_error("Native rematch failed");
                    practice.match_elapsed_ticks=PlayerPractice::match_duration_ticks-1;
                }
                if(frames==331) {
                    if(!practice.match_finished)throw std::runtime_error("Native second full time failed");
                    SDL_Event injected{};injected.type=SDL_EVENT_MOUSE_BUTTON_DOWN;
                    injected.button.button=SDL_BUTTON_LEFT;injected.button.windowID=SDL_GetWindowID(window.get());
                    check(SDL_RenderCoordinatesToWindow(renderer.get(),344,400,&injected.button.x,&injected.button.y),"Map rematch click");
                    check(SDL_PushEvent(&injected),"Inject rematch click");
                }
                if(frames==333) {
                    if(practice.match_finished || practice.match_seconds_left()!=180)
                        throw std::runtime_error("Native mouse rematch failed");
                    running=false;
                }
            }
            if(opt.cpu_smoke){
                const auto button=[&](SDL_GamepadButton code,bool down){check(SDL_SetJoystickVirtualButton(virtual_pad,int(code),down),"Setup virtual button" );};
                const auto key=[&](SDL_Keycode code){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=code;check(SDL_PushEvent(&e),"Inject CPU key");};
                auto& s=*graphics.sequence;static int shoulder_selected=0;
                if(frames==1)key(SDLK_B);
                if(frames==2 && (!s.cpu_vs_cpu || s.visible_players.size()!=22))throw std::runtime_error("CPU entry failed");
                if(frames==6)key(SDLK_F6);
                if(frames==7){if(!graphics.match_panels)throw std::runtime_error("Match panels keyboard toggle failed");key(SDLK_F6);}
                if(frames==8){if(graphics.match_panels)throw std::runtime_error("Match panels did not close");key(SDLK_TAB);}
                if(frames==9){if(graphics.show_radar)throw std::runtime_error("Original radar toggle failed");key(SDLK_TAB);}
                if(frames==10){if(!graphics.show_radar)throw std::runtime_error("Radar did not return");button(SDL_GAMEPAD_BUTTON_RIGHT_STICK,true);}
                if(frames==11 && !graphics.match_panels)throw std::runtime_error("Gamepad match panels failed");
                if(frames==12)button(SDL_GAMEPAD_BUTTON_RIGHT_STICK,false);
                if(frames==13)button(SDL_GAMEPAD_BUTTON_RIGHT_STICK,true);
                if(frames==14){if(graphics.match_panels)throw std::runtime_error("Gamepad panels did not close");button(SDL_GAMEPAD_BUTTON_RIGHT_STICK,false);}
                if(frames==15){
                    SDL_Event click{};click.type=SDL_EVENT_MOUSE_BUTTON_DOWN;click.button.button=SDL_BUTTON_LEFT;click.button.windowID=SDL_GetWindowID(window.get());
                    check(SDL_RenderCoordinatesToWindow(renderer.get(),100,80,&click.button.x,&click.button.y),"Map hidden toolbar click");check(SDL_PushEvent(&click),"Inject hidden toolbar click");
                }
                if(frames==16 && nav.page!=4)throw std::runtime_error("Hidden restoration tabs consumed a match click");
                if(frames==40)key(SDLK_B);
                if(frames==41 && s.cpu_vs_cpu)throw std::runtime_error("CPU takeover failed");
                if(frames==42){s.start_team_play(false);s.full_match=true;shoulder_selected=s.selected;button(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,true);}
                if(frames==44){
                    if(s.cpu_vs_cpu || s.selected==shoulder_selected)throw std::runtime_error("RB must switch players without handing the human team to CPU");
                    check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,false),"Release player switch");
                    key(SDLK_P);
                }
                if(frames==45){if(!graphics.paused)throw std::runtime_error("Human pause failed");shoulder_selected=s.selected;button(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,true);}
                if(frames==46){if(!graphics.paused || s.cpu_vs_cpu || s.selected!=shoulder_selected)throw std::runtime_error("Paused RB must preserve human control and selected player");button(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,false);key(SDLK_B);key(SDLK_R);}
                if(frames==47){if(!s.cpu_vs_cpu || graphics.paused || s.ticks>1)throw std::runtime_error("Explicit CPU reset failed");button(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,true);}
                if(frames==48){if(s.cpu_vs_cpu)throw std::runtime_error("RB can take control from spectator mode");button(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,false);key(SDLK_B);std::cout<<"RB switches human players, preserves pause, and takes control from spectator mode.\n";}
                if(frames==150){
                    for(int i:s.visible_players)if(s.engine.teams[i].control)throw std::runtime_error("Human slot in CPU teams");
                    if(graphics.scene_counts.home_players!=11 || graphics.scene_counts.away_players!=11 || graphics.scene_counts.keepers!=2 || !graphics.polygon_count())throw std::runtime_error("CPU rendering failed");
                    s.start_team_play(true);s.full_match=true;s.engine.time_factor=5;s.engine.match_time.min=45;
                }
                if(frames==151 && !s.interval_ticks)throw std::runtime_error("Half-time display transition failed");
                if(frames==252){
                    if(s.engine.match_half!=1 || s.blue_slot()!=11)throw std::runtime_error("Second-half transition failed");
                    s.restart_mode=s.restart_ticks=0;s.engine.match_mode=0;s.engine.dead_ball_cnt=s.engine.game_action=0;
                    s.engine.reset_ball();s.engine.match_time.min=90;
                }
                if(frames==253){if(!s.full_time || s.running)throw std::runtime_error("Result display transition failed");check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_RIGHT_STICK,true),"Result rematch press");}
                if(frames==254)check(SDL_SetJoystickVirtualButton(virtual_pad,SDL_GAMEPAD_BUTTON_RIGHT_STICK,false),"Result rematch release");
                if(frames==256){if(!s.full_match || s.full_time || s.engine.match_half || !s.cpu_vs_cpu)throw std::runtime_error("Result controller rematch failed");key(SDLK_Q);}
                if(frames==258){if(s.cpu_vs_cpu || s.team_play || !s.continuation)throw std::runtime_error("CPU replay exit failed");if(!opt.setup_smoke)running=false;else key(SDLK_F2);}
                if(opt.setup_smoke){
                    if(frames==259){if(!setup.open || !graphics.paused)throw std::runtime_error("Setup must pause");key(SDLK_RIGHT);}
                    if(frames==260){if(s.next_home!=5)throw std::runtime_error("Team selector failed");key(SDLK_DOWN);key(SDLK_DOWN);key(SDLK_RIGHT);}
                    if(frames==261){if(s.next_home_formation!=6)throw std::runtime_error("Independent formation selector failed");key(SDLK_RETURN);}
                    if(frames==262){if(setup.open || !s.cpu_vs_cpu || s.home_team!=5 || s.home_formation!=6 || !s.national_teams)throw std::runtime_error("Setup CPU start failed");button(SDL_GAMEPAD_BUTTON_LEFT_STICK,true);}
                    if(frames==263)button(SDL_GAMEPAD_BUTTON_LEFT_STICK,false);
                    if(frames==264){if(!setup.open)throw std::runtime_error("Controller setup open failed");button(SDL_GAMEPAD_BUTTON_DPAD_DOWN,true);}
                    if(frames==265)button(SDL_GAMEPAD_BUTTON_DPAD_DOWN,false);
                    if(frames==266)button(SDL_GAMEPAD_BUTTON_DPAD_RIGHT,true);
                    if(frames==267)button(SDL_GAMEPAD_BUTTON_DPAD_RIGHT,false);
                    if(frames==268){if(s.next_away_formation!=3)throw std::runtime_error("Controller formation selector failed");button(SDL_GAMEPAD_BUTTON_NORTH,true);}
                    if(frames==269)button(SDL_GAMEPAD_BUTTON_NORTH,false);
                    if(frames==270){if(setup.open || s.cpu_vs_cpu || !s.full_match || s.away_formation!=3)throw std::runtime_error("Controller human start failed");key(SDLK_V);}
                    if(frames==271){if(graphics.match_camera!=1)throw std::runtime_error("Live camera cycle failed");key(SDLK_F2);}
                    if(frames==272){key(SDLK_DOWN);key(SDLK_DOWN);key(SDLK_DOWN);key(SDLK_RIGHT);}
                    if(frames==273){if(s.next_format!=OriginalActions::MatchFormat::knockout)throw std::runtime_error("Knockout format selector failed");key(SDLK_RIGHT);}
                    if(frames==274){if(s.next_format!=OriginalActions::MatchFormat::golden)throw std::runtime_error("Golden goal selector failed");key(SDLK_RIGHT);}
                    if(frames==275){if(s.next_format!=OriginalActions::MatchFormat::penalties)throw std::runtime_error("Penalty format selector failed");key(SDLK_RETURN);}
                    if(frames==276){if(!s.shootout || !s.cpu_vs_cpu || s.blue_goals || s.red_goals)throw std::runtime_error("Penalty-only setup launch failed");button(SDL_GAMEPAD_BUTTON_LEFT_STICK,true);}
                    if(frames==277)button(SDL_GAMEPAD_BUTTON_LEFT_STICK,false);
                    if(frames==278)button(SDL_GAMEPAD_BUTTON_DPAD_RIGHT,true);
                    if(frames==279)button(SDL_GAMEPAD_BUTTON_DPAD_RIGHT,false);
                    if(frames==280){if(s.next_format!=OriginalActions::MatchFormat::regulation)throw std::runtime_error("Gamepad format wrap failed");button(SDL_GAMEPAD_BUTTON_NORTH,true);}
                    if(frames==281)button(SDL_GAMEPAD_BUTTON_NORTH,false);
                    if(frames==282){if(s.shootout || s.cpu_vs_cpu || !s.full_match || s.match_format!=OriginalActions::MatchFormat::regulation)throw std::runtime_error("Format reset leaked shootout state");running=false;}
                }
            }
            if(opt.audio_smoke){
                if(!audio || !audio->device_ready())throw std::runtime_error("Audio smoke could not open dummy stream");
                const auto key=[](SDL_Keycode k){SDL_Event event{};event.type=SDL_EVENT_KEY_DOWN;event.key.key=k;check(SDL_PushEvent(&event),"Audio key injection");};
                if(frames==10)key(SDLK_F8);
                if(frames==11){if(!audio->muted() || audio->voice_count())throw std::runtime_error("F8 mute failed");key(SDLK_F8);}
                if(frames==12 && audio->muted())throw std::runtime_error("F8 unmute failed");
                if(frames==101 && audio->voice_count())throw std::runtime_error("Pause did not stop audio");
                if(frames==200 && !audio->stats().pitch[2])throw std::runtime_error("Match restart whistle not connected");
            }
            if(opt.team_smoke && !opt.cpu_smoke && !opt.depth_smoke){
                const auto key=[&](SDL_Keycode code){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=code;check(SDL_PushEvent(&e),"Inject original team key");};
                auto& s=*graphics.sequence;
                if(frames==1)key(SDLK_Y);
                if(frames==2 && (!s.team_play || s.visible_players.size()!=22))throw std::runtime_error("Team entry failed");
                if(frames==3)key(SDLK_SPACE);
                if(frames==40)key(SDLK_E);
                if(frames==50)key(SDLK_X);
                if(frames==100)key(SDLK_P);
                if(frames==101){if(!graphics.paused)throw std::runtime_error("Team pause failed");key(SDLK_R);}
                if(frames==102 && (!s.team_play || graphics.paused || s.ticks>1))throw std::runtime_error("Team reset failed");
                if(frames==200){
                    if(graphics.scene_counts.home_players!=11 || graphics.scene_counts.away_players!=11 || graphics.scene_counts.keepers!=2 || !graphics.polygon_count())throw std::runtime_error("Team rendering failed");
                    key(SDLK_Q);
                }
                if(frames==202){if(s.team_play || !s.continuation)throw std::runtime_error("Team replay return failed");running=false;}
            }
            if(opt.control_smoke){
                const auto key=[&](SDL_Keycode code){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=code;check(SDL_PushEvent(&e),"Inject original control key");};
                const auto button=[&](SDL_GamepadButton code,bool down){check(SDL_SetJoystickVirtualButton(virtual_pad,int(code),down),"Original control virtual button");};
                const auto axis=[&](Sint16 value){check(SDL_SetJoystickVirtualAxis(virtual_pad,SDL_GAMEPAD_AXIS_LEFTX,value),"Original control virtual movement");};
                auto& s=*graphics.sequence;
                if(frames==1)key(SDLK_T);
                if(frames==2){if(!s.play || !graphics.action_view)throw std::runtime_error("Take-control key failed");axis(22000);}
                if(frames==15){if(s.engine.teams[1].tm_x<940 || s.engine.ball_poss!=2)throw std::runtime_error("Original gamepad dribble failed");axis(0);}
                if(frames==20 || frames==85 || frames==145)key(SDLK_R);
                if(frames==22)button(SDL_GAMEPAD_BUTTON_SOUTH,true);
                if(frames==23)button(SDL_GAMEPAD_BUTTON_SOUTH,false);
                if(frames==80 && (s.receptions!=1 || s.selected!=2 || s.engine.ball_poss!=3))throw std::runtime_error("Original gamepad ground pass failed");
                if(frames==87)button(SDL_GAMEPAD_BUTTON_NORTH,true);
                if(frames==88)button(SDL_GAMEPAD_BUTTON_NORTH,false);
                if(frames==140 && (s.receptions!=1 || s.selected!=2 || s.engine.ball_poss!=3))throw std::runtime_error("Original gamepad lofted pass failed");
                if(frames==147)button(SDL_GAMEPAD_BUTTON_WEST,true);
                if(frames==148)button(SDL_GAMEPAD_BUTTON_WEST,false);
                if(frames==200 && s.shots!=1)throw std::runtime_error("Original gamepad shot failed");
                if(frames==205)button(SDL_GAMEPAD_BUTTON_START,true);
                if(frames==206)button(SDL_GAMEPAD_BUTTON_START,false);
                if(frames==207){if(!graphics.paused)throw std::runtime_error("Original gamepad pause failed");button(SDL_GAMEPAD_BUTTON_RIGHT_STICK,true);}
                if(frames==208)button(SDL_GAMEPAD_BUTTON_RIGHT_STICK,false);
                if(frames==210){if(s.ticks || !graphics.paused || s.engine.ball_poss!=2)throw std::runtime_error("Original paused reset failed");button(SDL_GAMEPAD_BUTTON_START,true);}
                if(frames==211)button(SDL_GAMEPAD_BUTTON_START,false);
                if(frames==212)key(SDLK_E);
                if(frames==214){if(s.selected!=2)throw std::runtime_error("Original keyboard switch failed");axis(22000);}
                if(frames==224 && s.engine.teams[2].tm_x<=1040)throw std::runtime_error("Original off-ball movement failed");
                if(frames==225){SDL_CloseJoystick(virtual_pad);virtual_pad=nullptr;check(SDL_DetachVirtualJoystick(virtual_id),"Disconnect original controller");virtual_id=0;}
                if(frames==227 && !graphics.paused)throw std::runtime_error("Original controller disconnect must pause");
                if(frames==228)key(SDLK_Q);
                if(frames==232){
                    if(s.play || !s.continuation || !graphics.polygon_count())throw std::runtime_error("Return to original replay failed");
                    s.start_match(false);s.full_match=false;s.restart_mode=s.restart_ticks=0;auto& e=s.engine;
                    e.match_mode=e.dead_ball_cnt=e.set_piece_on=e.game_action=e.ball_out_of_play=0;e.holder_lose_ball();
                    for(auto& p:e.teams){p.guy_on=0;p.tm_x=p.tm_y=80;}
                    auto& p=e.teams[1];p.guy_on=1;p.tm_x=950;p.tm_y=400;p.tm_xdis=1;p.tm_ydis=0;p.control=1;
                    e.reset_ideas(&p);e.init_stand_act(&p);s.selected=1;e.collect_ball(&p);e.hold_ball(&p);
                    graphics.paused=false;live_key_shot=live_key_pass=false;
                    SDL_Event up{};up.type=SDL_EVENT_KEY_UP;up.key.key=SDLK_X;check(SDL_PushEvent(&up),"Clear previous shot key");
                }
                if(frames==234)key(SDLK_X);
                if(frames==240){
                    if(s.live_charge()<5 || s.engine.ball_poss!=2)throw std::runtime_error("Keyboard stationary charge did not hold possession");
                    SDL_Event up{};up.type=SDL_EVENT_KEY_UP;up.key.key=SDLK_X;check(SDL_PushEvent(&up),"Release original power shot");
                }
                if(frames==265){
                    if(!s.engine.teams[1].tm_shots || s.engine.after_touch_on)throw std::runtime_error("Keyboard charged drive did not release correctly");
                    SDL_VirtualJoystickDesc desc{};desc.version=sizeof(desc);desc.type=SDL_JOYSTICK_TYPE_GAMEPAD;desc.naxes=6;desc.nbuttons=15;desc.name="Human controls reconnect";
                    virtual_id=SDL_AttachVirtualJoystick(&desc);check(virtual_id!=0,"Reconnect human controller");virtual_pad=SDL_OpenJoystick(virtual_id);check(virtual_pad!=nullptr,"Open human controller");
                    s.start_match(false);s.full_match=false;s.restart_mode=s.restart_ticks=0;auto& e=s.engine;
                    e.match_mode=e.dead_ball_cnt=e.set_piece_on=e.game_action=e.ball_out_of_play=0;e.holder_lose_ball();
                    for(auto& p:e.teams){p.guy_on=0;p.tm_x=p.tm_y=80;}
                    auto& p=e.teams[1];p.guy_on=1;p.tm_x=950;p.tm_y=400;p.tm_xdis=1;p.tm_ydis=0;p.control=1;
                    e.reset_ideas(&p);e.init_stand_act(&p);s.selected=1;e.collect_ball(&p);e.hold_ball(&p);
                }
                if(frames==268)button(SDL_GAMEPAD_BUTTON_WEST,true);
                if(frames==274){if(s.live_charge()<5)throw std::runtime_error("Controller charge differs from keyboard");button(SDL_GAMEPAD_BUTTON_WEST,false);}
                if(frames==300){
                    if(!s.engine.teams[1].tm_shots || s.engine.after_touch_on)throw std::runtime_error("Controller power shot did not release");
                    std::cout<<"Keyboard and reconnected gamepad original hold/release controls passed.\n";
                }
                if(frames==301 || frames==351){
                    s.start_match(false);s.full_match=false;s.restart_mode=s.restart_ticks=0;auto& e=s.engine;
                    e.match_mode=e.dead_ball_cnt=e.set_piece_on=e.game_action=e.ball_out_of_play=0;e.reset_shot();e.holder_lose_ball();
                    for(auto& p:e.teams){p.guy_on=0;p.tm_x=p.tm_y=80;}
                    auto& p=e.teams[1];p.guy_on=1;p.tm_x=950;p.tm_y=400;p.tm_xdis=p.mface_x=1;p.tm_ydis=p.mface_y=0;p.control=1;
                    p.tm_vis=p.tm_cont=p.tm_ac=p.tm_pow=p.tm_flair=p.tm_rate=96;p.tm_mcspd=1.5f;p.tm_jump=40;
                    e.reset_ideas(&p);e.init_stand_act(&p);s.selected=1;
                    auto& mate=e.teams[2];mate.guy_on=1;mate.tm_x=1200;mate.tm_y=400;e.init_stand_act(&mate);
                    e.ballx=980;e.bally=400;e.ballz=8;e.ballxdis=-2;e.ballydis=0;e.ballzdis=-2;e.ball_inair=1;e.last_touch=3;e.ball_released=0;
                }
                if(frames==302){key(SDLK_X);key(SDLK_SPACE);}
                if(frames==342 || frames==392){
                    const auto& p=s.engine.teams[1];
                    if(p.tm_pass!=1 || p.tm_shots)throw std::runtime_error("Keyboard/gamepad both-button first-time pass failed");
                    if(frames==342){for(auto k:{SDLK_X,SDLK_SPACE}){SDL_Event up{};up.type=SDL_EVENT_KEY_UP;up.key.key=k;check(SDL_PushEvent(&up),"Release first-time pass keys");}}
                    else{button(SDL_GAMEPAD_BUTTON_WEST,false);button(SDL_GAMEPAD_BUTTON_SOUTH,false);}
                }
                if(frames==352){button(SDL_GAMEPAD_BUTTON_WEST,true);button(SDL_GAMEPAD_BUTTON_SOUTH,true);}
                if(frames==395){
                    std::cout<<"Keyboard and gamepad both-button first-time pass contacts passed.\n";running=false;
                }
            }
            if(opt.goals_smoke){
                if(frames==1){SDL_Event injected{};injected.type=SDL_EVENT_KEY_DOWN;injected.key.key=SDLK_G;check(SDL_PushEvent(&injected),"Inject goal review");}
                if(frames==2 && (!graphics.action_view || !graphics.sequence->goal_review))throw std::runtime_error("Goal review key dispatch failed");
                if(graphics.sequence->goal_review && !graphics.sequence->running){
                    if(graphics.sequence->review_stage!=4 || graphics.sequence->engine.goal_events!=1 || !graphics.polygon_count())throw std::runtime_error("Goal review smoke failed");
                    if(graphics.sequence->review_left)running=false;
                    else{SDL_Event injected{};injected.type=SDL_EVENT_KEY_DOWN;injected.key.key=SDLK_H;check(SDL_PushEvent(&injected),"Inject opposite goal review");}
                }
                if(frames>850)throw std::runtime_error("Goal review did not finish both batches");
            }
            if(opt.actions_smoke && !graphics.sequence->running) {
                if(graphics.sequence->releases!=3 || graphics.sequence->first_touches!=2 || graphics.sequence->shots!=1 || graphics.sequence->saves!=1 || graphics.sequence->engine.ball_poss!=13 || graphics.sequence->distributions!=1 || graphics.sequence->receptions!=1 || !graphics.polygon_count())throw std::runtime_error("Original keeper continuation smoke failed");
                running=false;
            }
            if(opt.graphics_smoke) {
                if(frames==1 && graphics.polygon_count()==0)throw std::runtime_error("Original graphics produced no polygons");
                if(frames==2){SDL_Event injected{};injected.type=SDL_EVENT_KEY_DOWN;injected.key.key=SDLK_RIGHT;check(SDL_PushEvent(&injected),"Inject original animation selection");}
                if(frames==4){if(graphics.clip!=41)throw std::runtime_error("Original animation navigation failed");running=false;}
            }
            if(opt.practice_smoke) {
                if(frames==1 || frames==45 || frames==90) {
                    if(frames==45 && practice.engine.ball_poss!=3) throw std::runtime_error("Practice pass was not received");
                    if(frames==90 && (practice.engine.ball_poss!=2 || practice.passes_received!=2)) throw std::runtime_error("Practice return pass failed");
                    SDL_Event injected{};injected.type=SDL_EVENT_KEY_DOWN;injected.key.key=frames==90?SDLK_X:SDLK_SPACE;
                    check(SDL_PushEvent(&injected),"Inject practice action");
                }
                if(frames==100) {
                    if(nav.page!=3 || practice.engine.ball_poss || practice.engine.teams[1].tm_shots!=1 || practice.engine.ballz<=2)
                        throw std::runtime_error("Practice shot input/render failed");
                    running=false;
                }
            }
            if(opt.physics_smoke) {
                if(frames==1) { SDL_Event injected{}; injected.type=SDL_EVENT_KEY_DOWN; injected.key.key=SDLK_SPACE; check(SDL_PushEvent(&injected),"Inject kick"); }
                if(frames==60) {
                    if(nav.page!=2 || lab.ticks==0 || lab.engine.ballx<=640 || lab.trail_size<2)
                        throw std::runtime_error("Physics input/render smoke failed");
                    running=false;
                }
            }
            if(opt.smoke) {
                // Exercise actual SDL event dispatch across every background, then both views.
                if(frames<15) {
                    SDL_Event injected{}; injected.type=SDL_EVENT_KEY_DOWN; injected.key.key=SDLK_RIGHT;
                    check(SDL_PushEvent(&injected),"Inject navigation test");
                } else if(frames==15) {
                    SDL_Event injected{}; injected.type=SDL_EVENT_KEY_DOWN; injected.key.key=SDLK_TAB;
                    check(SDL_PushEvent(&injected),"Inject tab test");
                } else {
                    if(nav.screen!=Navigation::wrap(opt.screen+14,15) || nav.page!=((opt.practice?3:(opt.physics?2:(opt.tactics?1:0)))+1)%4)
                        throw std::runtime_error("Native event navigation did not reach expected state");
                    running=false;
                }
            }
#ifdef __EMSCRIPTEN__
            if(frames%20==0){
                const auto& s=*graphics.sequence;
                EM_ASM({ Module.gameState = ({frames:$0, ticks:$1, selected:$2,
                    ballX:$3, ballY:$4, homeGoals:$5, awayGoals:$6, paused:!!$7, menuStage:$8, presentationHeight:$9}); },
                    frames,s.ticks,s.selected,s.engine.ballx,s.engine.bally,
                    s.blue_goals,s.red_goals,graphics.paused,front?int(front->stage()):0,presentation_height);
            }
#endif
            co_await std::suspend_always{};
        }
        pads.close();
        if(local_second_pad)SDL_CloseJoystick(local_second_pad);
        if(local_second_id)SDL_DetachVirtualJoystick(local_second_id);
        if(virtual_pad) SDL_CloseJoystick(virtual_pad);
        if(virtual_id) SDL_DetachVirtualJoystick(virtual_id);
        front.reset();music.reset();commentary.reset();audio.reset();
        texture.reset(); renderer.reset(); window.reset();
        SDL_Quit();
        std::cout<<"Native render completed: "<<frames<<" frames.\n";
        co_return 0;
    } catch(const std::exception& e) {
        std::cerr<<"Euro96: "<<e.what()<<"\n";
        if(interactive) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"UEFA Euro 96 restoration",e.what(),nullptr);
        if(initialized) SDL_Quit();
        co_return 1;
    }
}


#ifdef __EMSCRIPTEN__
static void browser_frame(void* context){
    auto* task=static_cast<FrameTask*>(context);
    if(task->step())return;
    const int code=task->result();
    emscripten_cancel_main_loop();
    delete task;
    emscripten_force_exit(code);
}
#endif
int main(int argc,char** argv){
    std::vector<std::string> arguments(argv,argv+argc);
#ifdef __EMSCRIPTEN__
    auto* task=new FrameTask(run_game(std::move(arguments)));
    emscripten_set_main_loop_arg(browser_frame,task,0,false);
    // Initialize audio during the launch gesture; subsequent frames use rAF.
    browser_frame(task);
    emscripten_exit_with_live_runtime();
#else
    auto task=run_game(std::move(arguments));
    while(task.step())SDL_Delay(8);
    return task.result();
#endif
}
