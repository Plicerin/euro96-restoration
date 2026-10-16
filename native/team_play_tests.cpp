#include "original_graphics.hpp"
#include "original_actions.hpp"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
using namespace actua;
static void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
    OriginalGraphics g(argv[1]);auto& s=*g.sequence;s.start_team_play();auto& e=s.engine;
    require(s.visible_players.size()==22 && e.ball_poss==9,"Original full roster and initial possession");
    const auto before=e.teams;
    for(int tick=0;tick<40;++tick)s.step();
    int moved=0;for(int i:s.visible_players)if(std::hypot(e.teams[i].tm_x-before[i].tm_x,e.teams[i].tm_y-before[i].tm_y)>2)++moved;
    require(moved>=10,"Tactics and defensive AI must move both teams under neutral user input");
    s.start_team_play();const int previous=s.selected;s.switch_play_player();
    require(s.selected>=1 && s.selected<=10 && s.selected!=previous,"Switch must select another blue outfielder");
    require(e.ball_poss==9,"Selection must not transfer possession");
    // Deterministic contact fixture: original ball_interact can take possession
    // from an opponent without requiring a free ball or fabricated magnet.
    for(int side:{0,1}){
        s.start_team_play();auto& winner=e.teams[side?12:1];auto& holder=e.teams[side?1:12];
        e.holder_lose_ball();winner.tm_x=600;winner.tm_y=400;holder.tm_x=606;holder.tm_y=400;
        holder.tm_xdis=-1;holder.tm_ydis=0;e.collect_ball(&holder);
        e.ballx=600;e.bally=400;e.ballz=2;e.ballxdis=e.ballydis=e.ballzdis=0;
        e.seed=127;e.ball_interact(&winner);
        require(e.ball_poss==winner.tm_player,"Both sides must win low-ball contact from an opponent");
        e.ballx=900;e.bally=400;e.ball_interact(&holder);
        require(e.ball_poss==winner.tm_player,"Distant opponent must not magnetize or steal the ball");
    }
    s.start_team_play();e.holder_lose_ball();s.request_play_kick(OriginalActions::PlayKick::shot);s.step();
    require(e.teams[s.selected].tm_tacks==1 && e.teams[s.selected].tm_act==3,"Shot button without possession must start original tackle");
    for(int i=0;i<35;++i)s.step();require(e.teams[s.selected].tm_act!=3,"Tackle must complete and recover");
    int ai_passes=0,ai_shots=0,collections=0;
    for(int seed=1;seed<=12;++seed){
        s.start_team_play();std::srand(seed);
        if(seed%2){e.holder_lose_ball();e.collect_ball(&e.teams[19]);}
        for(int tick=0;tick<1200 && s.running;++tick){
            if(tick%90==0)s.switch_play_player();
            s.set_play_input(tick%120<60?1.0f:-1.0f,tick%180<90?.3f:-.3f);
            if(tick%50==0)s.request_play_kick(OriginalActions::PlayKick::ground);
            if(tick%101==0)s.request_play_kick(OriginalActions::PlayKick::shot);
            s.set_restart_input(0,0,tick%30<8,false,false);
            s.step();
            require(e.ball_poss>=0 && e.ball_poss<=22,"Possession must stay in enabled roster");
            require(s.selected>=0 && s.selected<=10 && s.is_blue(s.selected),"Possession must never select an opponent");
            for(int i:s.visible_players){const auto& p=e.teams[i];require(std::hypot(p.tm_xdis,p.tm_ydis)>.9f && std::hypot(p.tm_xdis,p.tm_ydis)<1.1f,"Facing must stay normalized");}
        }
        ai_passes+=s.ai_passes;ai_shots+=s.ai_shots;collections+=s.receptions;
        if(argc>2)std::cout<<"seed "<<seed<<" ticks "<<s.ticks<<" AI passes "<<s.ai_passes<<" shots "<<s.ai_shots<<" collections "<<s.receptions<<'\n';
    }
    require(ai_passes>0 && ai_shots>0 && collections>5,"Original opponent AI must pass, shoot and recover possession over seeded play");
    s.restart_play();require(s.team_play && s.running && !s.turnovers,"Team reset must retain mode and clear counters");
    int cpu_passes[2]{},cpu_shots[2]{},cpu_collections=0;
    for(int seed=1;seed<=12;++seed){
        s.start_team_play(true);std::srand(seed);
        if(seed%2){e.holder_lose_ball();e.collect_ball(&e.teams[19]);}
        for(int tick=0;tick<1200 && s.running;++tick){
            s.step();
            require(e.ball_poss>=0 && e.ball_poss<=22,"CPU possession must stay in enabled roster");
            for(int i:s.visible_players)require(!e.teams[i].control,"Every outfielder must remain under CPU control, including after reception");
        }
        for(int i:s.visible_players){cpu_passes[i<11?0:1]+=e.teams[i].tm_pass;cpu_shots[i<11?0:1]+=e.teams[i].tm_shots;}
        cpu_collections+=s.receptions;
    }
    require(cpu_passes[0]>0 && cpu_passes[1]>0 && cpu_shots[0]>0 && cpu_shots[1]>0 && cpu_collections>5,"Both CPU teams must pass, shoot and collect the ball");
    // Identical simulation with and without stray human inputs: no frozen human
    // slot, accidental player switch, or queued kick may influence watch mode.
    std::vector<float> reference;
    for(int run=0;run<2;++run){
        s.start_team_play(true);std::vector<float> trace;
        for(int tick=0;tick<240 && s.running;++tick){
            if(run){s.set_play_input(1,.5f);s.switch_play_player();s.request_play_kick(tick%2?OriginalActions::PlayKick::shot:OriginalActions::PlayKick::ground);}
            s.step();trace.insert(trace.end(),{e.ballx,e.bally,e.ballz,float(e.ball_poss),float(s.selected)});
            for(int i:s.visible_players){auto& p=e.teams[i];trace.insert(trace.end(),{p.tm_x,p.tm_y,float(p.tm_act),float(p.tm_pass),float(p.tm_shots)});}
        }
        if(!run)reference=trace;else require(trace==reference,"Human inputs must not affect CPU play");
    }
    s.start_team_play(true);for(int i=0;i<20;++i)s.step();
    const auto ball_x=e.ballx,ball_y=e.bally;const int poss=e.ball_poss,tick=s.ticks;
    s.set_cpu_control(false);int controlled=0;for(int i:s.visible_players)controlled+=e.teams[i].control!=0;
    require(!s.cpu_vs_cpu && controlled==1 && s.selected>=1 && s.selected<=10,"Takeover must control exactly one blue player");
    require(e.ballx==ball_x && e.bally==ball_y && e.ball_poss==poss && s.ticks==tick,"Takeover must preserve the live ball and clock");
    s.set_cpu_control(true);s.restart_play();require(s.cpu_vs_cpu && s.running && s.ticks==0,"Reset must retain CPU mode");
    s.start_play();require(!s.cpu_vs_cpu,"Leaving team play must clear spectator mode");
    std::cout<<"CPU blue/red passes "<<cpu_passes[0]<<'/'<<cpu_passes[1]<<", shots "<<cpu_shots[0]<<'/'<<cpu_shots[1]<<", collections "<<cpu_collections<<'\n';
    std::cout<<"Original teams, tactics, decisions, challenge contacts and selection passed. AI passes "<<ai_passes<<", shots "<<ai_shots<<", collections "<<collections<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
