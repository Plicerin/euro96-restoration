#include "original_actions.hpp"
#include "action_constants.hpp"
#include <algorithm>
#include <cmath>
#include <string_view>
namespace actua {
using namespace original::action_constants;
OriginalPlayer conditioned_stats(const OriginalPlayer& source,int fitness){
    auto stats=source;
    // EURO_INF fitness adjustments bridged into restored match ratings. Clamp
    // before FOOTBALL's +28 conversion to avoid unsigned stat wraparound.
    const int missing=100-std::clamp(fitness,0,100);
    for(int* v:{&stats.pace,&stats.power,&stats.stamina})*v=std::max(0,int(*v-missing*.5));
    for(int* v:{&stats.accuracy,&stats.flair})*v=std::max(0,int(*v-missing*.1));
    return stats;
}
OriginalPlayer OriginalActions::match_stats(bool away,int shirt)const{return conditioned_stats(original_teams[away?away_team:home_team].players.at(shirt-1),match_fitness[away][shirt-1]);}
OriginalActions::Reports OriginalActions::player_reports()const{
    auto reports=departed_reports;
    for(int i=0;i<22;++i){const auto& p=engine.teams[i];if(p.shirt<1 || p.shirt>20)continue;
        auto& r=reports[!is_blue(i)][p.shirt-1];r.injury+=p.tm_inj;r.bookings+=p.tm_book;
    }
    return reports;
}
float OriginalActions::pace_remaining(int index)const{
    const auto& p=engine.teams.at(index);
    if(!national_teams)return 1;
    const auto stats=match_stats(!is_blue(index),p.shirt);
    return float(p.tm_rate)/(stats.pace+28);
}
bool OriginalActions::bench_available(bool away,int shirt)const{
    // Original in-match substitution menu offers squad slots 12 through 15.
    return national_teams && shirt>=1 && shirt<=20 && match_available[away][shirt-1] && !used_squad[away][shirt-1] && std::find(match_order[away].begin()+11,match_order[away].begin()+15,shirt)!=match_order[away].begin()+15;
}
bool OriginalActions::queue_substitution(bool away,int slot,int shirt){
    if(!full_match || full_time || shootout || slot<0 || slot>10 || substitutions[away]>=3 || !bench_available(away,shirt))return false;
    const int index=slot+(away?11-blue_slot():blue_slot());
    if(!engine.teams[index].guy_on)return false;
    pending_subs[away]={slot,shirt};
    sub_notice=team_label(away)+" substitution queued for next stoppage";return true;
}
void OriginalActions::apply_substitutions(){
    if(!national_teams)return;
    for(int side=0;side<2;++side){
        const auto sub=pending_subs[side];if(sub.slot<0)continue;
        pending_subs[side]={};
        const int index=sub.slot+(side?11-blue_slot():blue_slot());auto& p=engine.teams[index];
        if(!p.guy_on || !bench_available(side!=0,sub.shirt) || substitutions[side]>=3)continue;
        for(int i=0;i<15;++i)departed_figures[side][i]+=player_figures(p)[i];
        departed_reports[side][p.shirt-1]={int(p.tm_inj),int(p.tm_book)};
        const auto old=p;const auto outgoing=player_label(index);
        sub_passes[side]+=p.tm_pass;sub_shots[side]+=p.tm_shots;
        // process_subs/init_player_stats: same field slot, new stats/shirt. Fresh
        // per-player action/stat state prevents a substitute inheriting bookings/time.
        p={};p.tm_player=short(index+1);p.shirt=short(sub.shirt);p.guy_on=p.draw_me=1;
        p.tm_x=old.tm_x;p.tm_y=old.tm_y;p.tm_xdis=p.mface_x=old.tm_xdis;p.tm_ydis=p.mface_y=old.tm_ydis;
        apply_original_stats(p,match_stats(side!=0,sub.shirt));engine.init_stand_act(&p);
        used_squad[side][sub.shirt-1]=true;++substitutions[side];
        sub_notice=outgoing+" off / "+player_label(index)+" on";
        if(walkoff_events_enabled)walkoff_events.push_back({index,old,p,true,sub_notice});
        notice("SUBSTITUTION",sub_notice);++generation;
    }
}
void OriginalActions::update_match_depth(){
    auto& e=engine;refresh_setpiece_takers();
    if(match_notice.ticks>0)--match_notice.ticks;
    if(card_notice_ticks>0)--card_notice_ticks;
    if(e.card_is_out>0)--e.card_is_out;
    if(e.card_events!=seen_card_events){
        seen_card_events=e.card_events;const int i=e.card_player-1;
        card_notice=(e.teams[i].tm_book>=5?"RED CARD - ":"YELLOW CARD - ")+player_label(i);card_notice_ticks=100;
        notice(e.teams[i].tm_book>=5?"RED CARD, SENT OFF":"BOOKED, YELLOW CARD",player_label(i),e.teams[i].tm_book>=5?44:45);
        if(walkoff_events_enabled && e.teams[i].tm_book>=5)walkoff_events.push_back({i,e.teams[i],{},false,player_label(i)});
        if(!e.teams[selected].guy_on && !cpu_vs_cpu)set_cpu_control(false);
    }
    for(int i=0;i<22;++i)if(e.teams[i].guy_on && national_teams){
        const auto stats=match_stats(!is_blue(i),e.teams[i].shirt);
        e.apply_stamina(&e.teams[i],stats.pace+28);
    }
    // Original movement and animation routines; native timing keeps officials
    // ticking during dead balls and faces the offender while a card is shown.
    if(card_notice_ticks && e.card_player){e.refs[0].target=e.card_player;e.init_refs_anim(&e.refs[0],MC_STAND);e.at_target(&e.refs[0]);}
    else if(!restart_ticks && !e.match_mode && !e.just_scored){e.refs[0].target=0;e.follow_play(&e.refs[0]);}
    else if(e.refs[0].act==1)e.goto_target(&e.refs[0]);
    else {e.init_refs_anim(&e.refs[0],MC_STAND);if(e.refs[0].act==3 || e.refs[0].act==4)e.at_target(&e.refs[0]);}
    for(int i=1;i<3;++i){if((restart_ticks || e.match_mode) && e.refs[i].act==1)e.goto_target(&e.refs[i]);else e.lman_follow(&e.refs[i]);}
    for(auto& ref:e.refs)e.anim_officials(&ref);
    // Native CPU team management; original game did not supply this menu policy.
    if(national_teams && !full_time && !shootout)for(int side=0;side<2;++side){
        if(((!side || head_to_head) && !cpu_vs_cpu) || substitutions[side]>=3 || pending_subs[side].slot>=0 || e.match_time.min<60+substitutions[side]*10)continue;
        int slot=1;float lowest=2;const int start=side?11-blue_slot():blue_slot();
        for(int j=1;j<11;++j)if(e.teams[start+j].guy_on && pace_remaining(start+j)<lowest){lowest=pace_remaining(start+j);slot=j;}
        const auto& squad=original_teams[side?away_team:home_team].players;
        const std::string_view role=squad[e.teams[start+slot].shirt-1].position;
        int incoming=0;
        for(int bench=0;bench<4;++bench){const int shirt=bench_shirt(side!=0,bench);if(bench_available(side!=0,shirt) && std::string_view(squad[shirt-1].position)!="GK"){
            if(!incoming)incoming=shirt;
            if(std::string_view(squad[shirt-1].position)==role){incoming=shirt;break;}
        }
        }
        if(incoming)queue_substitution(side!=0,slot,incoming);
    }
}
void OriginalActions::step_celebration(){
    auto& e=engine;e.user_controlled=0;
    for(int i:visible_players){auto& p=e.teams[i];if(!p.guy_on)continue;
        p.control=0;e.process_anims(&p);
        if(p.tm_act<=1 && p.int_move!=16)e.someone_has_scored(&p);
        e.intelligence(&p);
        if(!p.tm_limbo)e.dispatch_action(&p);
        e.process_dir(&p);
    }
    // Original process_ball speaks the score at the just_scored countdown edge.
    // A golden goal goes straight to the final result rather than another kickoff.
    if(e.just_scored>0 && !--e.just_scored && full_match && !shootout && e.golden_goal!=2)
        e.PlayScoreSample(e.team_a_goals,e.team_b_goals);
}
}

namespace actua {
std::array<int,15> OriginalActions::player_figures(const original::match_player& p){return {p.tm_tacks,p.tm_twon,p.tm_pass,p.tm_pwon,p.tm_shots,p.tm_saved,p.tm_posst,p.tm_work,p.tm_kpsaves,0,0,0,0,p.tm_book%5?1:0,p.tm_book>=5?1:0};}
OriginalActions::Figures OriginalActions::match_figures()const{
 if(shootout)return shootout_figures;
 auto result=departed_figures;
 for(int side=0;side<2;++side)for(int j=0;j<15;++j)result[side][j]+=restart_figures[side][j];
 for(int i=0;i<22;++i){auto values=player_figures(engine.teams[i]);for(int j=0;j<15;++j)result[!is_blue(i)][j]+=values[j];}
 for(int side=0;side<2;++side)result[side][6]=possession_ticks[side];
 return result;
}
void OriginalActions::record_restart_statistics(int mode){
 if(!full_match || shootout)return;
 int slot=-1,column=-1;
 if(mode>=1 && mode<=4){slot=(mode==1 || mode==3)?11:0;column=9;}
 else if(mode==15 || mode==16){slot=mode==15?0:11;column=10;}
 else if(mode==17 || mode==18){slot=mode==17?0:11;column=11;}
 else if((mode==13 || mode==14) && engine.offside_now){slot=mode==13?11:0;column=12;}
 if(slot>=0)++restart_figures[!is_blue(slot)][column];
}
}
