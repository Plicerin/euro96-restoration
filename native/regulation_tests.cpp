#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "action_constants.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <cstdlib>
using namespace actua;
using namespace actua::original::action_constants;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
    OriginalGraphics g(argv[1]);auto& s=*g.sequence;auto& e=s.engine;
    auto live=[&](){
        s.start_team_play(true);s.full_match=true;e.time_factor=5;e.EUROmatch_info.freekicks=true;
        e.ref_strictness=e.ref_accuracy=128;e.can_be_offside=1;
        e.refs[0].x=640;e.refs[0].y=400;e.refs[1].y=-16;e.refs[2].y=816;
    };
    for(int minutes:{5,10,20,40,90}){
        s.set_match_minutes(minutes);s.start_match(true);
        require(e.time_factor==minutes && e.match_mode==CENTRE_A,"Match length/start kickoff");
        const float old=e.match_time.sec;for(int i=0;i<20;++i)s.step();
        require(e.match_time.sec==old,"Clock must stop during kickoff setup");
        e.match_mode=e.stop_clock=e.just_scored=0;
        for(int i=0;i<20;++i)e.match_clock();
        require(std::abs(e.match_time.sec-90.0f/minutes)<.002f,"Original match-clock scaling");
    }
    bool invalid=false;try{s.set_match_minutes(0);}catch(const std::invalid_argument&){invalid=true;}require(invalid,"Reject zero match length");
    s.set_match_minutes(5);live();e.holder_lose_ball();e.reset_shot();e.match_time.min=45;
    e.teams[1].tm_pass=17;e.teams[12].tm_pass=9;s.blue_goals=e.team_a_goals=2;s.red_goals=e.team_b_goals=1;
    s.step();require(s.interval_ticks==100 && e.match_half==0,"Half-time must stop play");
    const float half_ball=e.ballx;for(int i=0;i<99;++i)s.step();require(e.ballx==half_ball,"Ball must freeze through interval");
    s.step();require(e.match_half==1 && s.blue_slot()==11 && e.match_mode==CENTRE_A,"Switch ends and red second-half kickoff");
    require(e.teams[12].tm_pass==17 && e.teams[1].tm_pass==9 && s.blue_goals==2 && s.red_goals==1,"Stats and score follow team identity");
    for(int i=0;i<22;++i)require(e.teams[i].tm_player==i+1,"Swapped player slots must be renumbered");
    s.set_cpu_control(false);require(s.selected>11,"Second-half takeover must select blue");
    s.restart_mode=e.match_mode=e.dead_ball_cnt=e.game_action=0;s.switch_play_player();require(s.selected>11,"Second-half player switching must stay blue");
    e.match_time.min=90;e.match_time.sec=0;e.reset_shot();s.step();require(s.full_time && !s.running,"Full-time must end regulation match");
    const int end=s.ticks;for(int i=0;i<20;++i)s.step();require(s.ticks==end,"Results must freeze simulation");
    s.restart_play();require(s.full_match && !s.full_time && !e.match_half && !s.blue_goals && !s.red_goals && !s.cpu_vs_cpu,"Rematch resets match but preserves control mode");
    // Both goal directions after the original roster exchange.
    for(bool left:{false,true}){
        live();e.swap_teams();e.holder_lose_ball();e.ballx=left?2.0f:1278.0f;e.bally=400;e.ballz=12;
        e.ballxdis=left?-8.0f:8.0f;e.ballydis=e.ballzdis=0;e.ball_inair=1;s.step();
        require(s.blue_goals==int(left) && s.red_goals==int(!left),"Second-half scoring identity");
    }
    // Each foul branch must reach a real animation release for the right team.
    for(int fouler:{2,13})for(int kind=0;kind<4;++kind){
        live();e.holder_lose_ball();auto& p=e.teams[fouler-1];
        p.tm_x=kind==2?(fouler<12?100.0f:1180.0f):kind==3?(fouler<12?300.0f:980.0f):640.0f;p.tm_y=400;
        e.ballx=p.tm_x;e.bally=p.tm_y;e.player_distances();e.get_ball_zone();
        e.man_down=4;e.init_foul(fouler,kind!=0,true);
        const int mode=kind==0?(fouler<12?IF_KICK_B:IF_KICK_A):kind!=2?(fouler<12?DF_KICK_B:DF_KICK_A):(fouler<12?PEN_KICK_B:PEN_KICK_A);
        require(e.match_mode==mode && e.foul_events==1,"Original foul award / box penalty");
        int taker=0;bool release=false,saw_wall=false;
        for(int i=0;i<700;++i){s.step();if(e.match_mode)taker=e.setp_taker;saw_wall|=e.men_in_wall>0;if(taker && (e.teams[taker-1].tm_pass || e.teams[taker-1].tm_shots || e.ball_limbo_on)){release=true;break;}}
        std::cout<<"foul "<<mode<<" taker "<<taker<<" release "<<release<<'\n';
        require(release && ((taker<12)!=(fouler<12)),"Foul restart must release for opponents");
        if(kind==3)require(saw_wall,"Close direct free kick must position original defensive wall");
    }
    // Original linesman decision: flagged attacker interferes, indirect kick.
    for(bool away:{false,true}){
        live();e.holder_lose_ball();auto& p=e.teams[away?13:2];
        p.tm_x=away?240.0f:1040.0f;p.tm_y=400;p.tm_dist=10;p.tm_off=-1;
        e.defense_a=400;e.defense_b=880;e.refs[1].x=400;e.refs[2].x=880;
        e.ballx=away?250.0f:1030.0f;e.bally=400;e.ball_released=away?-40:40;e.last_touch=away?13:2;e.seed=0;
        e.offside_rule(&p);
        require(e.offside_events==1 && e.match_mode==(away?IF_KICK_A:IF_KICK_B),"Offside must award indirect kick to opponents");
        live();auto& on=e.teams[away?13:2];on.tm_x=640;on.tm_off=0;e.can_be_offside=0;e.offside_rule(&on);
        require(!e.rule_events && !on.tm_off,"Restart offside exemption / onside player");
    }
    // Tackle contact uses source knockdown and foul visibility, then recovers.
    live();e.holder_lose_ball();for(auto& p:e.teams){p.tm_x=50;p.tm_y=50;p.tm_act=5;}
    auto& a=e.teams[1];auto& b=e.teams[12];a.tm_x=b.tm_x=640;a.tm_y=b.tm_y=400;
    a.tm_act=3;a.go_cnt=30;a.go_txdis=4;a.go_tydis=0;b.tm_act=0;e.player_num=2;e.seed=0;e.player_ints(&a);
    require(b.tm_act==5 && e.rule_events==1,"Tackle contact must knock down and award foul");
    for(int i=0;i<100;++i){e.process_anims(&b);if(!b.tm_limbo)e.dispatch_action(&b);}
    require(b.tm_act==0,"Knocked-down player must recover");
    // Original whistle gate waits for a pending shot.
    live();e.match_time.min=90;e.match_half=1;e.shot_pending=1;
    require(!e.regulation_period_end(),"Full-time must wait for crucial shot");e.shot_pending=0;require(e.regulation_period_end()==2,"Whistle after shot resolves");
    int goals=0,fouls=0,offsides=0;
    g.action_view=true;
    for(int seed=1;seed<=12;++seed){
        s.start_match(true);std::srand(seed);int stuck=0;bool saw_half=false;
        for(int tick=0;tick<20000 && s.running;++tick){
            g.advance(.05);saw_half|=e.match_half==1;
            for(int i:s.visible_players){const auto sample=g.presentation.player(e,i,.5f);for(float v:sample.pose)require(std::isfinite(v),"Finite full-match animation pose");}
            if(s.restart_mode)++stuck;else stuck=0;
            if(stuck>=1600){std::cerr<<"stuck seed "<<seed<<" mode "<<s.restart_mode<<" owner "<<e.ball_poss<<" piece "<<int(e.set_piece_on)<<'\n';}
            require(stuck<1600,"Set-piece deadlock in full CPU match");
            require(std::isfinite(e.ballx)&&std::isfinite(e.bally)&&std::isfinite(e.ballz),"Finite full-match ball");
            for(auto& p:e.teams)require(std::isfinite(p.tm_x)&&std::isfinite(p.tm_y)&&std::isfinite(p.tm_z),"Finite full-match players");
        }
        std::cout<<"seed "<<seed<<" score "<<s.blue_goals<<':'<<s.red_goals<<" ticks "<<s.ticks<<" clock "<<s.clock_label()<<" fouls "<<e.foul_events<<" offside "<<e.offside_events<<" restarts "<<s.restarts<<'\n';
        require(s.full_time && saw_half,"CPU match must reach both half-time and full-time");
        goals+=s.blue_goals+s.red_goals;fouls+=e.foul_events;offsides+=e.offside_events;
    }
    require(fouls && offsides,"Unscripted CPU matches must exercise fouls and offsides");
    std::cout<<"Twelve complete CPU matches: goals "<<goals<<", fouls "<<fouls<<", offsides "<<offsides<<'\n';
    return 0;
}catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 1;}}
