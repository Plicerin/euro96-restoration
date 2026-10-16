#include "original_actions.hpp"
#include "action_constants.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <atomic>
#include <chrono>
#include <random>
namespace actua {
using namespace original::action_constants;
namespace {
unsigned int fresh_match_seed(){
    // Independent of the gameplay CRT RNG: menus/audio cannot consume seeds.
    // An odd stride plus a bijective mix avoids repeats until 32-bit wraparound.
    static std::atomic<unsigned int> counter{[]{
        auto seed=static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        try{seed^=std::random_device{}();}catch(const std::exception&){ /* Clock fallback keeps startup available. */ }
        return seed;
    }()};
    auto seed=counter.fetch_add(0x9e3779b9u,std::memory_order_relaxed);
    seed^=seed>>16;seed*=0x7feb352du;seed^=seed>>15;seed*=0x846ca68bu;seed^=seed>>16;
    return seed;
}
}
void OriginalActions::change_team(bool away,int delta){
    int& team=away?next_away:next_home;team=(team+delta+16)%16;
    if(team==(away?next_home:next_away))team=(team+delta+16)%16;
    (away?next_away_formation:next_home_formation)=original_teams[team].formation;next_takers[away]={};
}
std::string OriginalActions::team_label(bool away) const {
    return national_teams?original_teams[away?away_team:home_team].code:away?"RED":"BLUE";
}
std::string OriginalActions::player_label(int index) const {
    if(index<0 || index>=22)return "free";
    const bool away=!is_blue(index);const int shirt=engine.teams[index].shirt;
    if(!national_teams)return team_label(away)+" "+std::to_string(shirt);
    const std::string name=original_teams[away?away_team:home_team].players[shirt-1].name;
    return team_label(away)+" "+name.substr(name.find(' ')+1);
}
std::string OriginalActions::match_text(std::string value) const {
    if(!national_teams)return value;
    for(const char* word:{"Blue","blue","BLUE","Red","red","RED"}){
        std::size_t at=0;const std::string target=team_label(word[0]=='R'||word[0]=='r');
        while((at=value.find(word,at))!=std::string::npos){
            const auto count=std::char_traits<char>::length(word);
            if((at && std::isalpha(static_cast<unsigned char>(value[at-1]))) || (at+count<value.size() && std::isalpha(static_cast<unsigned char>(value[at+count])))){at+=count;continue;}
            value.replace(at,count,target);at+=target.size();
        }
    }
    return value;
}
void OriginalActions::load_match_tactics(){
    if(!national_teams)return;
    const auto& a=formations[blue_slot()?away_formation:home_formation];
    const auto& b=formations[blue_slot()?home_formation:away_formation];
    for(int z=0;z<70;++z)for(int p=0;p<10;++p){
        engine.match_tactics1[z][p][0]=a[z][p].x;engine.match_tactics1[z][p][1]=a[z][p].y;
        engine.match_tactics2[z][p][0]=b[z][p].x;engine.match_tactics2[z][p][1]=b[z][p].y;
    }
}
int OriginalActions::preferred_taker(bool away,int role)const{
    if(!national_teams || role<0 || role>=3)return 0;
    const int shirt=match_takers[away][role],start=away?11-blue_slot():blue_slot();
    if(!shirt)return 0;
    for(int j=1;j<11;++j)if(engine.teams[start+j].guy_on && engine.teams[start+j].shirt==shirt)return start+j+1;
    return 0;
}
void OriginalActions::refresh_setpiece_takers(){
    if(!national_teams)return;
    const bool aAway=blue_slot()!=0;
    engine.penalty_taker_a=preferred_taker(aAway,0);engine.penalty_taker_b=preferred_taker(!aAway,0);
    engine.fkick_taker_a=preferred_taker(aAway,1);engine.fkick_taker_b=preferred_taker(!aAway,1);
    engine.corn_kicker_a=preferred_taker(aAway,2);engine.corn_kicker_b=preferred_taker(!aAway,2);
}
bool OriginalActions::set_interval_formation(bool away,int formation){
    if(!national_teams || !full_match || full_time || shootout || !interval_ticks || formation<0 || formation>=10)return false;
    (away?away_formation:home_formation)=formation;load_match_tactics();return true;
}
void OriginalActions::set_match_minutes(int minutes){
    if(minutes!=5 && minutes!=10 && minutes!=20 && minutes!=40 && minutes!=90)
        throw std::invalid_argument("Match length must be 5, 10, 20, 40 or 90 minutes");
    match_minutes=minutes;
}
void OriginalActions::start_match(bool cpu){
    if(next_home<0 || next_home>=16 || next_away<0 || next_away>=16 || next_home==next_away || next_home_formation<0 || next_home_formation>=10 || next_away_formation<0 || next_away_formation>=10)
        throw std::invalid_argument("Invalid match teams or formations");
    if(!next_options.valid())throw std::invalid_argument("Invalid match options");
    const int prepared_fixture=next_tournament_fixture;
    const bool prepared_original_draw=next_original_draw;
    start_team_play(cpu);match_options=next_options;departed_reports={};match_takers=next_takers;
    tournament_fixture=prepared_fixture>=0 && prepared_fixture<31?prepared_fixture:-1;
    original_draw=tournament_fixture>=0 && prepared_original_draw;
    for(int side=0;side<2;++side)for(int j=0;j<20;++j){
        match_order[side][j]=next_squad_enabled?next_order[side][j]:j+1;
        match_fitness[side][j]=next_squad_enabled?next_fitness[side][j]:100;
        match_available[side][j]=next_squad_enabled?next_available[side][j]:1;
    }
    engine.tournament_conditions=next_squad_enabled;
    next_squad_enabled=false;
    head_to_head=next_head_to_head && !cpu;
    full_match=true;match_format=next_format;auto& e=engine;
    e.need_result=match_format!=MatchFormat::regulation;e.golden_goal=match_format==MatchFormat::golden;
    if(rosters_ready){
        national_teams=true;home_team=next_home;away_team=next_away;
        home_formation=next_home_formation;away_formation=next_away_formation;
        load_match_tactics();
        for(int i=0;i<22;++i){e.teams[i].shirt=short(match_order[i/11][i%11]);apply_original_stats(e.teams[i],match_stats(i>=11,e.teams[i].shirt));e.init_stand_act(&e.teams[i]);}
    }
    e.say_names=1; // Original FOOTBALL.init_match enables player-name calls.
    e.practice=0;e.time_factor=match_minutes;
    e.ref_accuracy=short(match_options.referee_accuracy);e.ref_strictness=short(match_options.referee_strictness);
    e.EUROmatch_info.freekicks=match_options.fouls;e.EUROmatch_info.bookings=match_options.bookings;e.can_be_offside=1;
    // FOOTBALL.init_match: three strengths, fixed +X wind (the original final override).
    e.wind_on=short(match_options.wind!=0);e.wind_speed=float(match_options.wind*30);e.wind_x=1;e.wind_y=0;
    e.refs[0].x=float(e.cntspot_x);e.refs[0].y=float(e.cntspot_y)-80;
    e.refs[1].x=e.refs[2].x=float(e.cntspot_x);
    e.refs[1].y=-16;e.refs[2].y=float(e.pitch_wid)+16;
    for(int side=0;side<2;++side)for(int i=0;i<11;++i)used_squad[side][match_order[side][i]-1]=true;
    for(auto& ref:e.refs){ref.anim=-1;e.init_refs_anim(&ref,MC_STAND);}
    restart_mode=CENTRE_A;prepare_restart();
    status=head_to_head?"Head to head - P1 "+team_label(false)+" / P2 "+team_label(true):"First half - blue kickoff / blue attacks right";
    if(match_format==MatchFormat::penalties){e.swap_teams();load_match_tactics();start_shootout();}
}
std::string OriginalActions::clock_label() const {
    char time[32];std::snprintf(time,sizeof(time),"%02d:%02d",engine.match_time.min,int(engine.match_time.sec));
    return (shootout?std::string(full_time?"FT PEN":"PEN"):std::string(full_time?"FT ":interval_ticks?"HT ":engine.match_half==3?"ET2 ":engine.match_half==2?"ET1 ":engine.match_half?"2H ":"1H ")+time);
}
std::string OriginalActions::match_restart_label() const {
    std::string label=restart_name(restart_mode);
    if(blue_slot()){
        if(label.starts_with("Blue"))label.replace(0,4,"Red");
        else if(label.starts_with("Red"))label.replace(0,3,"Blue");
    }
    return label;
}
bool OriginalActions::step_match_rules(){
    auto& e=engine;
    if(interval_ticks){
        if(!--interval_ticks){
            std::swap(e.players_off_a,e.players_off_b);e.swap_teams();selected=(selected+11)%22;other_input.selected=(other_input.selected+11)%22;load_match_tactics();
            e.match_time.min=interval_minute;e.match_time.sec=0;e.injury_time=0;
            // Rosters are end-relative. The previous red team is now A and takes centre.
            restart_mode=CENTRE_A;prepare_restart();
            status=period_label()+" - "+match_text(match_restart_label());
        }
        return true;
    }
    if(e.golden_goal==2 && !e.just_scored){finish_match();return true;}
    if(restart_ticks || e.ball_in_goal || e.match_mode)return false;
    e.stop_clock=0;e.match_clock();
    const int ended=e.match_half<2?e.regulation_period_end():e.extra_period_end();
    if(ended){
        e.play_fx(ended==1 || ended==3?3:4,e.refs[0].x,e.refs[0].y);
        if(ended==1)e.PlayCommentaryMessage(42); // AUDIO.H TI_HALFTIME, once at the period boundary.
        clear_play_input();e.stop_clock=1;e.reset_ball();e.ball_limbo_on=e.ball_in_hands=0;
        if(ended==1 || ended==3 || (ended==2 && e.need_result && blue_goals==red_goals)){
            interval_minute=ended==1?45:ended==2?90:105;interval_ticks=100;
            if(ended==2)e.PlayCommentaryMessage(43); // Retail FLL dispatcher selects XTM for this tied interval.
            status=ended==2?"Level after 90 - extra time":ended==3?"Extra-time interval - changing ends":"Half-time - changing ends";
        }else if(ended==4 && blue_goals==red_goals)start_shootout();
        else finish_match();
        return true;
    }
    return false;
}
void OriginalActions::accept_rule_restart(){
    auto& e=engine;
    if(e.rule_events==seen_rule_events)return;
    seen_rule_events=e.rule_events;restart_mode=e.match_mode;e.match_mode=0;restart_ticks=25;
    e.ball_out_of_play=1;e.stop_clock=1;
    notice(e.offside_now?"OFFSIDE":"FREE KICK",match_text(match_restart_label()),46);
    status=std::string(e.offside_now?"Offside - ":"Foul - ")+match_restart_label();clear_play_input();
}
void OriginalActions::start_team_play(bool cpu){
    if(!tactics_ready)throw std::runtime_error("Original tactics have not been loaded");
    reset();team_play=play=running=true;
    match_seed=match_seed_override?*match_seed_override:fresh_match_seed();
    std::srand(match_seed);auto& e=engine;
    std::printf("Match start seed: %u (%s)\n",match_seed,match_seed_override?"fixed":"fresh");
    e.practice=1;e.close_in_number=2;e.auto_users_a=e.auto_users_b=0;
    e.team_a_on=e.team_b_on=1;e.goal_kicker_a=1;e.goal_kicker_b=12;
    e.analogue=1;e.ballx=float(e.cntspot_x);e.bally=float(e.cntspot_y);e.last_touch=2;e.want_pass=0;
    for(int z=0;z<70;++z)for(int p=0;p<10;++p){
        e.match_tactics1[z][p][0]=e.match_tactics2[z][p][0]=team_tactics[z][p].x;
        e.match_tactics1[z][p][1]=e.match_tactics2[z][p][1]=team_tactics[z][p].y;
    }
    visible_players.clear();e.get_ball_zone();
    for(int i=0;i<22;++i){
        auto& p=e.teams[i];p={};p.tm_player=short(i+1);p.tm_dist=10000;
        p.guy_on=p.draw_me=1;p.tm_rate=p.tm_pow=p.tm_cont=p.tm_flair=p.tm_ac=96;p.tm_vis=120;
        p.tm_stam=p.tm_disc=96;p.tm_mcspd=float(p.tm_flair+p.tm_rate)/128;p.tm_jump=40;
        p.tm_xdis=p.mface_x=i<11?1.0f:-1.0f;p.face_dir=i<11?0:4;
        p.shirt=short(i<11?i+1:i-10);
        const int side=i<11?1:2,slot=i<11?i:i-11,zone=i<11?e.ball_zone1+32:e.ball_zone2;
        if(i==0 || i==11){p.tm_x=float(KP_OFFLINE);p.tm_y=float(e.cntspot_y)-1;}
        else e.get_target(char(side),zone,i<11?e.zone1_x:e.zone2_x,i<11?e.zone1_y:e.zone2_y,slot,p.tm_x,p.tm_y,side==1?e.match_tactics1:e.match_tactics2);
        if(side==2){p.tm_x=e.pitch_len-p.tm_x;p.tm_y=e.pitch_wid-p.tm_y;}
        e.init_stand_act(&p);visible_players.push_back(i);
    }
    selected=8;auto& p=e.teams[selected];p.tm_x=float(e.cntspot_x)-30;p.tm_y=float(e.cntspot_y);p.control=1;
    e.ball_poss=0;e.collect_ball(&p);e.hold_ball(&p);e.player_distances();
    e.keeper_boxes();set_cpu_control(cpu);
}
void OriginalActions::set_cpu_control(bool enabled){
    if(!team_play || full_time || shootout || head_to_head)return;
    cpu_vs_cpu=enabled;clear_play_input();auto& e=engine;
    selection_wait=manual_selection_ticks=0;seen_selection_events=e.selection_events;
    for(auto& p:e.teams)p.control=0;
    e.user_controlled=0;e.uf1=e.uf2=e.um=0;e.ux=e.uy=0;e.f1_down=e.f2_down=0;
    if(!enabled){
        const int first=blue_slot()+1;
        if(e.ball_poss>first && e.ball_poss<=first+10)selected=e.ball_poss-1;
        else{
            e.player_distances();float best=1e9f;
            for(int i=first;i<first+10;++i)if(e.teams[i].guy_on && e.teams[i].tm_dist<best){best=e.teams[i].tm_dist;selected=i;}
        }
        e.teams[selected].control=1;
    }
    status=std::string(enabled?"CPU controls both teams - ":"You control blue - ")+(blue_slot()?"blue attacks left":"blue attacks right");
}
const char* OriginalActions::restart_name(int mode){
    switch(mode){
    case CENTRE_A:return "Blue kickoff";case CENTRE_B:return "Red kickoff";
    case THROW_IN_A:return "Blue throw-in";case THROW_IN_B:return "Red throw-in";
    case CORNER_TL:case CORNER_BL:return "Red corner";
    case CORNER_TR:case CORNER_BR:return "Blue corner";
    case GOAL_KICK_TL:case GOAL_KICK_BL:return "Blue goal kick";
    case GOAL_KICK_TR:case GOAL_KICK_BR:return "Red goal kick";
    case IF_KICK_A:return "Blue indirect free kick";case IF_KICK_B:return "Red indirect free kick";
    case DF_KICK_A:return "Blue direct free kick";case DF_KICK_B:return "Red direct free kick";
    case PEN_KICK_A:return "Blue penalty";case PEN_KICK_B:return "Red penalty";
    default:return "Live play";
    }
}
void OriginalActions::prepare_restart(){
    record_restart_statistics(restart_mode);
    auto& e=engine;++restarts;++generation;clear_play_input();restart_started=false;
    if(full_match && !shootout)apply_substitutions();
    refresh_setpiece_takers();
    e.just_scored=0;
    const int previous_touch=e.last_touch;
    // BALL.CPP respot_ball preserves this shot classification before reset_ball.
    e.fired_a_shot=e.shot_pending && restart_mode>=GOAL_KICK_TL && restart_mode<=GOAL_KICK_BR;
    e.reset_ball();e.ball_out_of_play=e.ball_limbo_on=e.ball_in_hands=e.just_thrown=0;
    if(restart_mode>=IF_KICK_A && restart_mode<=PEN_KICK_B)e.last_touch=previous_touch;
    e.match_mode=char(restart_mode);e.support_me=e.already_there=0;
    e.must_pass=e.must_punt=e.must_shoot=0;e.want_pass=0;
    e.play_advantage=0;e.ball_released=0;e.clear_wall();e.reset_all_ideas();e.in_range=0;
    for(auto& p:e.teams){p.tm_limbo=p.tm_leave=p.tm_strike=p.contact=p.tm_stopped=p.tm_off=p.tm_wall=0;e.init_stand_act(&p);}
    if(full_match){e.get_ball_zone();e.ball_zone1=std::clamp(e.ball_zone1,0,31);e.ball_zone2=std::clamp(e.ball_zone2,0,31);}
    // Resolve a persistent shirt to its current physical slot after substitutions.
    // Keep shootout rotation and the original long-range free-kick policy.
    if(!shootout && restart_mode>=DF_KICK_A && restart_mode<=PEN_KICK_B){
        const bool physicalB=restart_mode==DF_KICK_B || restart_mode==PEN_KICK_B;
        const bool away=physicalB!=(blue_slot()!=0);const int role=restart_mode>=PEN_KICK_A?0:1;
        if(match_takers[away][role]){const int preferred=preferred_taker(away,role);
            const int player=preferred?preferred:e.get_taker(physicalB?13:2);
            if(role==0)e.penalty_taker=player;else e.fkick_taker=player;
        }
    }
    if(restart_mode==CENTRE_A || restart_mode==CENTRE_B){e.play_fx(2,e.refs[0].x,e.refs[0].y);e.init_centre();e.set_piece_on=SETP_CENTRE;}
    else if(restart_mode==THROW_IN_A || restart_mode==THROW_IN_B){
        e.get_ball_zone();e.ball_zone1=std::clamp(e.ball_zone1,0,31);e.ball_zone2=std::clamp(e.ball_zone2,0,31);
        e.init_throw();e.set_piece_on=SETP_THROW;
    }else if(restart_mode>=GOAL_KICK_TL && restart_mode<=GOAL_KICK_BR){e.init_gkick();e.set_piece_on=SETP_GKICK;}
    else if(restart_mode==IF_KICK_A || restart_mode==IF_KICK_B){e.dead_ball_cnt=50;e.game_action=1;e.init_fkick();e.set_piece_on=SETP_IFKICK;}
    else if(restart_mode==DF_KICK_A || restart_mode==DF_KICK_B){e.dead_ball_cnt=50;e.game_action=1;e.init_dfkick();e.set_piece_on=SETP_DFKICK;}
    else if(restart_mode==PEN_KICK_A || restart_mode==PEN_KICK_B){e.dead_ball_cnt=50;e.game_action=2;e.init_penalty();e.set_piece_on=SETP_PENALTY;}
    else{e.init_corner();e.set_piece_on=SETP_CORNER;}
    e.can_be_offside=e.set_piece_on!=SETP_THROW && e.set_piece_on!=SETP_CORNER && e.set_piece_on!=SETP_GKICK;
    e.setp_taker=e.main_man;e.ref_wait=80;
    // Native transition places actors at original targets; original actions release the ball.
    for(int i:visible_players){
        auto& p=e.teams[i];if(!p.guy_on)continue;float x,y;
        if(i==0 || i==11){x=i?e.pitch_len-float(KP_OFFLINE):float(KP_OFFLINE);y=float(e.cntspot_y)-1;
            if(e.men_in_wall && ((restart_mode==DF_KICK_A && i==11)||(restart_mode==DF_KICK_B && i==0))){x=e.keeper_pos_x;y=e.keeper_pos_y;}
            if(e.setp_taker==i+1){x=float(e.taker_x);y=float(e.taker_y);}}
        else{
            const bool home=i<11;
            e.get_target(home?1:2,home?e.ball_zone1:e.ball_zone2,home?e.zone1_x:e.zone2_x,home?e.zone1_y:e.zone2_y,home?i:i-11,x,y,home?e.match_tactics1:e.match_tactics2);
            if(!home){x=e.pitch_len-x;y=e.pitch_wid-y;}
        }
        p.tm_x=x;p.tm_y=y;p.tm_z=0;
        float dx=e.ballx-x,dy=e.bally-y,d=std::hypot(dx,dy);
        p.tm_xdis=p.mface_x=d>.01f?dx/d:(i<11?1.0f:-1.0f);p.tm_ydis=p.mface_y=d>.01f?dy/d:0;
        e.init_stand_act(&p);p.control=0;
    }
    e.player_distances();e.keeper_boxes();e.prev_ballx=e.ballx;e.prev_bally=e.bally;e.prev_ballz=e.ballz;
    if(e.set_piece_on==SETP_CENTRE && !cpu_vs_cpu){
        // Centre kicks are released by the original automatic restart routine,
        // so human_restart() deliberately excludes them. Assign the visible
        // selection now, before the opening/interval camera returns to play.
        for(int side=0;side<(head_to_head?2:1);++side){auto scope=input_scope(side);
            if(is_controlled_team(e.setp_taker-1))selected=e.setp_taker-1;
            else{
                float best=1e9f;
                for(int i=control_slot()+1;i<control_slot()+11;++i)
                    if(e.teams[i].guy_on && e.teams[i].tm_dist<best){best=e.teams[i].tm_dist;selected=i;}
            }
            e.users[side].plr=short(selected+1);e.users[side].chng=false;
            manual_selection_ticks=0;selection_wait=11;seen_selection_events=e.selection_events;
        }
        // Keep actor control neutral until the scripted centre pass releases;
        // live selection then runs on its first eligible tick, without a delay.
        e.user_taker=0;
    }
    auto restart_scope=input_scope(head_to_head && !is_blue(e.setp_taker-1)?1:0);
    if(human_restart()){
        selected=e.setp_taker-1;e.user_taker=input_side+1;
        e.setp_kick_x=e.teams[selected].tm_xdis;e.setp_kick_y=e.teams[selected].tm_ydis;
        e.setp_power=e.setp_hgt=e.setp_bzd=0;e.setp_byd=e.cntspot_y;
        e.f1_down=e.f2_down=e.uf1=e.uf2=e.spec_kick_type=0;
    }
    status=match_restart_label()+" - setting up";
}
void OriginalActions::step_restart(){
    auto& e=engine;auto restart_scope=input_scope(head_to_head && !is_blue(e.setp_taker-1)?1:0);if(!human_restart())clear_play_input();
    for(int i:visible_players)e.process_anims(&e.teams[i]);
    if(e.dead_ball_cnt>1){--e.dead_ball_cnt;return;}
    auto& taker=e.teams[e.setp_taker-1];e.user_controlled=0;
    if(human_restart()){
        selected=e.setp_taker-1;e.user_controlled=taker.control=char(input_side+1);e.refs[0].act=2;
        if(!restart_started){
            if(e.set_piece_on==SETP_THROW){e.ball_in_hands=1;e.init_throw_act(&taker);}
            e.collect_ball(&taker);restart_started=true;
        }
        e.ux=restart_x;e.uy=restart_y;e.um=std::hypot(e.ux,e.uy)>.24f;
        e.uf1=restart_shot;e.uf2=!restart_shot && (restart_pass || restart_loft);
        e.player_distances();e.get_opp_near_ball(e.ball_poss);
        if(e.set_piece_on==SETP_THROW){
            if(!e.um){e.ux=taker.tm_xdis;e.uy=taker.tm_ydis;e.um=1;}
            // Original outward direction selects a high throw and flips it inward.
            if(restart_loft)e.uy=e.bally>e.cntspot_y?std::abs(e.uy):-std::abs(e.uy);
            e.user_throw(&taker);
        }else if(e.set_piece_on==SETP_PENALTY)e.user_pen_kick(&taker,1);
        else {
            if(restart_loft)restart_high_pass=true;
            if(!e.uf1 && !e.uf2 && e.f2_down && restart_high_pass)e.taker_pass_h(taker.tm_xdis,taker.tm_ydis);
            else e.new_user_spec_kick(&taker);
        }
        e.process_dir(&taker);
        if(e.match_mode){status=restart_help();return;}
        e.set_piece_on=0;e.stop_clock=0;e.offside_now=0;e.clear_wall();e.user_taker=0;
        taker.tm_leave=-1;restart_mode=0;play_kick=PlayKick::none;play_x=play_y=0;
        status="Play resumed - your original set-piece release";return;
    }
    if(e.support_me)for(int i:visible_players)if(i+1!=e.setp_taker){
        auto& p=e.teams[i];p.control=0;e.intelligence(&p);
        if(p.tm_act==0 && !p.int_cnt && !p.tm_stopped)e.find_zonal_target(&p,p.tm_x,p.tm_y);
        else e.dispatch_action(&p);
        e.process_dir(&p);
    }
    if(e.set_piece_on==SETP_THROW){
        e.ball_in_hands=1;e.init_throw_act(&taker);e.collect_ball(&taker);
    }else{
        if(e.ball_poss!=taker.tm_player)e.collect_ball(&taker);
        --e.ref_wait;e.player_distances();e.get_opp_near_ball(e.ball_poss);e.decide_set_kick();
        if(e.support_me)return;
    }
    taker.tm_leave=-1;e.dead_ball_cnt=0;e.match_mode=0;e.game_action=0;e.set_piece_on=0;
    e.offside_now=0;e.clear_wall();e.stop_clock=0;
    e.already_there=0;e.user_taker=0;restart_mode=0;
    status="Play resumed - original set-piece release";
}
void OriginalActions::step_team_play(){
    if(shootout){step_shootout();return;}
    ++ticks;auto& e=engine;++e.logic_cnt;e.prediction=0;e.af_randomize();
    if(full_match){accept_rule_restart();update_match_depth();}
    if(full_match && step_match_rules())return;
    if(e.match_mode){step_restart();return;}
    if(e.just_thrown>0)--e.just_thrown;
    e.prev_ballx=e.ballx;e.prev_bally=e.bally;e.prev_ballz=e.ballz;
    for(int side=0;side<(head_to_head?2:1);++side){auto scope=input_scope(side);e.users_dir[side].x=cpu_vs_cpu?0:play_x;e.users_dir[side].y=cpu_vs_cpu?0:play_y;}
    e.update_ball_limbo();
    if(!e.ball_poss && !e.ball_limbo_on){
        if(e.shot_acknowledged)--e.shot_acknowledged;
        if(e.shot_pending){if(!++e.shot_pending)e.shot_pending=1;}else e.real_shot=0;
        e.ball_trajectory();e.ball_collision();
    }
    if(restart_ticks){if(full_match && e.just_scored)step_celebration();if(!--restart_ticks)prepare_restart();return;}
    const bool throwing=e.ball_poss && e.teams[e.ball_poss-1].tm_act==11;
    if(e.ball_in_goal || (!throwing && !e.just_thrown && !e.ball_limbo_on && (e.ballx<0 || e.ballx>=e.pitch_len || e.bally<0 || e.bally>=e.pitch_wid))){
        if(e.ball_in_goal){
            e.check_golden_goal();e.record_goal_score(e.goal_end);blue_goals=e.team_a_goals;red_goals=e.team_b_goals;
            restart_mode=e.last_goal==1?CENTRE_B:CENTRE_A;restart_ticks=full_match?ORIGINAL_SCORE_WAIT:60;
            if(full_match){e.goal_scorer=std::clamp(((e.goal_end>0 && e.last_touch==12) || (e.goal_end<0 && e.last_touch==1)) && e.pre_kp_touch?e.pre_kp_touch:e.last_touch,1,22);e.just_scored=ORIGINAL_SCORE_WAIT;e.user_scorer=0;e.reset_all_ideas();}
        }else{restart_mode=e.bounds_rules();restart_ticks=25;}
        if(e.ball_in_goal)notice("GOAL",player_label(e.goal_scorer-1),29);
        e.ball_out_of_play=1;
        status=std::string(e.ball_in_goal?"Goal - ":"Ball out - ")+match_restart_label();clear_play_input();return;
    }
    if(full_match && !shootout && e.ball_poss>=1 && e.ball_poss<=22)++possession_ticks[!is_blue(e.ball_poss-1)];
    e.get_ball_zone();e.ball_zone1=std::clamp(e.ball_zone1,0,31);e.ball_zone2=std::clamp(e.ball_zone2,0,31);
    e.keeper_boxes();e.player_distances();e.predict_ball();e.get_nearest();e.get_opp_near_ball(e.ball_poss);e.process_comments();
    if(full_match){
        e.update_rule_flags();accept_rule_restart();if(restart_ticks)return;

    }
    e.ball_speed=int(std::sqrt(e.ballxdis*e.ballxdis+e.ballydis*e.ballydis+e.ballzdis*e.ballzdis));
    std::array<PlayKick,2> commands{};std::array<int,2> command_players{};
    for(int side=0;side<(head_to_head?2:1);++side){auto scope=input_scope(side);update_auto_selection();commands[side]=play_kick;play_kick=PlayKick::none;command_players[side]=selected;}
    for(int side=0;side<2;++side)for(int slot=0;slot<=10;++slot){
        const int i=slot+(((ticks+side)&1)?0:11);auto& p=e.teams[i];
        if(!p.guy_on)continue;
        auto player_scope=input_scope(head_to_head && !is_blue(i)?1:0);
        const auto command=commands[input_side];const int command_player=command_players[input_side];
        p.control=char(!cpu_vs_cpu && i==selected && p.tm_act!=11?input_side+1:0);e.user_controlled=p.control;e.uf1=e.uf2=e.um=0;e.ux=e.uy=0;e.f1_down=e.f2_down=0;e.game_action=0;
        if(p.control){e.ux=play_x;e.uy=play_y;e.um=play_x!=0 || play_y!=0;
            if(live_controls){
                if(live_player!=selected){live_f1=live_f2=0;live_player=selected;}
                e.uf1=live_shot;e.uf2=live_pass;e.f1_down=live_f1;e.f2_down=live_f2;
            }
        }
        e.process_anims(&p);
        if(p.tm_limbo || (e.ball_limbo_on && e.ball_limbo_p==p.tm_player))continue;
        const int previous=e.ball_poss,previous_saves=p.tm_kpsaves;
        const bool held=(i==0 || i==11) && e.ball_in_hands && previous==p.tm_player;
        e.ball_interact(&p);saves+=p.tm_kpsaves-previous_saves;
        if(e.ball_poss==p.tm_player && previous!=p.tm_player){
            ++receptions;receive_tick=ticks;
            if(previous && (previous<12)!=(p.tm_player<12))++turnovers;
            if(i!=0 && i!=11 && is_controlled_team(i) && !cpu_vs_cpu){
                if(selected!=i){live_f1=live_f2=0;live_player=i;}
                e.teams[selected].control=0;selected=i;p.control=char(input_side+1);e.user_controlled=input_side+1;
                e.ux=play_x;e.uy=play_y;e.um=play_x!=0 || play_y!=0;
                if(live_controls){e.uf1=live_shot;e.uf2=live_pass;e.f1_down=live_f1;e.f2_down=live_f2;}
            }
            status=is_blue(i)?"Blue possession":cpu_vs_cpu?"Red possession":"Red possession - X: tackle";
        }
        if(full_match){
            e.player_num=p.tm_player;if(match_options.offside)e.offside_rule(&p);else p.tm_off=0;
            if(e.rule_events==seen_rule_events)e.player_ints(&p);
            accept_rule_restart();if(restart_ticks)return;
        }
        if(!cpu_vs_cpu && is_controlled_team(i) && (i==0 || i==11) && e.ball_poss==p.tm_player && (e.ball_in_hands || p.tm_act==PICKUP_ACT)){
            e.user_controlled=p.control=char(input_side+1);selected=i;
            const bool busy=e.keeper_busy(&p)!=0;
            if(!busy && (p.tm_act<=1 || p.tm_act==12)){
                // kphold_action normally selects STANDB after the recovery
                // countdown. Human delivery bypasses its CPU decisions, but
                // must still finish that original animation transition.
                if(p.tm_act==12){p.dir_mode=3;if(p.tm_anim!=MC_STANDB)e.init_anim(&p,MC_STANDB);}
                if(std::hypot(play_x,play_y)>.05f){p.tm_xdis=play_x;p.tm_ydis=play_y;}
                if(i==command_player && command!=PlayKick::none){
                    if(command==PlayKick::shot){e.must_punt=1;e.make_punt(&p);}
                    else {e.chip_pass=command==PlayKick::loft;e.must_pass=1;
                        const int receiver=e.pass_decide(&p);
                        if(!receiver){
                            // Original directional pass fallback and original keeper delivery animations.
                            e.spec_kick_type=SPACT_FPASS;e.setp_kick_x=p.tm_xdis;e.setp_kick_y=p.tm_ydis;
                            e.pass_type=e.chip_pass?10:9;
                        }else if(e.chip_pass && e.pass_type==9)e.pass_type=10;
                        e.make_pass(&p,receiver);e.chip_pass=0;}
                }
            }else {
                if(i==command_player && command!=PlayKick::none)play_kick=command;
                if(p.tm_act!=0)e.dispatch_action(&p);
            }
            // PICKUP_ACT sets ball_in_hands only when its countdown finishes.
            if(e.ball_poss==p.tm_player && e.ball_in_hands)e.hold_ball(&p);
        }else if(p.control && (i!=0 && i!=11 || e.ball_poss==p.tm_player || p.contact>0 || p.tm_limbo)){
            if(p.int_cnt && p.int_move==I_KICKED && !--p.int_cnt)e.reset_ideas(&p);
            if(live_controls && p.tm_act<=1 && !e.ball_limbo_on && command==PlayKick::none){
                if(e.ball_poss==p.tm_player){
                    e.set_pos_flags(&p);p.tm_srng=e.in_shot_rng;
                    e.spec_kick_type=0;e.drive_shot=0;
                    e.user_got_ball(&p);
                }else if(!e.ball_poss){
                    // Original predictor selects the feasible strike/control animation.
                    if((e.uf1 || e.uf2) && p.tm_strike<=0)e.go_to_path(p.tm_player);
                }else if(!is_controlled_team(e.ball_poss-1))e.user_opp_has_ball(&p);
            }
            if(i==command_player && command!=PlayKick::none && p.tm_act<=1 && !e.ball_limbo_on){
                if(e.ball_poss!=p.tm_player){if(command==PlayKick::shot)e.user_init_tackle(&p);}
                else if(command==PlayKick::shot){if(e.shoot_decide(&p)){e.drive_shot=1;e.make_shoot(&p);}}
                else{
                    e.chip_pass=command==PlayKick::loft;const int receiver=e.pass_decide(&p);
                    if(receiver){if(e.chip_pass && e.pass_type==5)e.pass_type=-1;
                        if(!e.chip_pass || e.pass_type==-1)e.make_pass(&p,receiver);}
                    e.chip_pass=0;
                }
            }
            if(p.tm_act<=1 && !(p.int_move==1 && p.tm_strike>0)){
                if(p.tm_act==1 && !e.um)e.init_stand_act(&p);
                else if(p.tm_act==1)e.user_run(&p);
                else if(e.um)e.user_stand(&p);
            }else e.dispatch_action(&p);
        }else{
            const int passes=p.tm_pass,attempts=p.tm_shots;p.control=0;e.user_controlled=0;e.intelligence(&p);
            if(p.tm_act==0 && !p.int_cnt && !p.tm_stopped)e.find_zonal_target(&p,p.tm_x,p.tm_y);
            else e.dispatch_action(&p);
            ai_passes+=p.tm_pass-passes;ai_shots+=p.tm_shots-attempts;
        }
        if(live_controls && p.control && i==selected){
            live_f1=e.f1_down;live_f2=e.f2_down;
            if(e.ball_poss!=p.tm_player){live_f1=live_f2=0;}
        }
        if(previous && !e.ball_poss){++releases;release_tick=ticks;if(e.real_shot)++shots;}
        if(held && !e.ball_poss){++distributions;distribution_tick=ticks;}
        e.player_comment(&p);
        if(!p.tm_limbo && !(e.ball_limbo_on && e.ball_limbo_p==p.tm_player))e.process_dir(&p);
        if(!std::isfinite(p.tm_x)||!std::isfinite(p.tm_y)||!std::isfinite(p.tm_xdis)||!std::isfinite(p.tm_ydis))throw std::runtime_error("Non-finite original team player");
    }
    if(!std::isfinite(e.ballx)||!std::isfinite(e.bally)||!std::isfinite(e.ballz))throw std::runtime_error("Non-finite original team ball");
}
}
