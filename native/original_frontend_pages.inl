// Screen composition uses recovered frontend sprites/fonts. Navigation and
// Windows-only pages remain native; match/tournament data remains authoritative.
namespace actua {
namespace {using Ink=OriginalMenuArt::Ink;}
void OriginalFrontend::menu_hit(int x,int y,int w,int h,std::function<void(OriginalGraphics&)> action){menu_hits_.push_back({{float(x),float(y),float(w),float(h)},std::move(action)});}
void OriginalFrontend::menu_button(int x,int y,int w,const std::string& label,SDL_Keycode k,bool selected){
 art_->panel(x,y,w,38);art_->image((selected?menu_art::BUTN_WHTE:menu_art::BUTN_BLUE)+(selected?int(menu_clock_*12)%12:0),x,y);
 art_->text(x+45,y+10,label,selected?Ink::cyan:Ink::white,0,0,w-51);
 menu_hit(x,y,w,38,[this,k](OriginalGraphics& g){key(k,g);});
}
void OriginalFrontend::menu_footer(const std::string& label){
 const int width=(stage_==Stage::lineup || stage_==Stage::tactics)?464:628;
 const bool controls=stage_!=Stage::title;
 art_->panel(6,455,width,25);art_->text(6+(width-(controls?88:0))/2,460,label,Ink::white,0,2,width-(controls?106:20));
 if(controls){const bool options=stage_==Stage::interval || stage_==Stage::result;
  art_->text(width-37,460,options?"OPTIONS":"BACK",Ink::cyan,0,2,78);
  menu_hit(width-80,455,86,25,[this,options](OriginalGraphics& g){if(options)open_options(g);else back(g);});}
}
void OriginalFrontend::menu_formation(const OriginalGraphics& g,int team,int formation,const std::array<int,20>& order,int selected){
 art_->bar(22,106,418);art_->text(231,110,original_teams[team].name,Ink::white,2,2,376);
 art_->image(menu_art::SMLL_GOTH+team,550,2,0,0,team);
 // EURO_SQD::DisplayFormation: tactical row 69 and authored trapezoid.
 const bool live=stage_==Stage::tactics && tactics_return_==Stage::interval;
 const int start=side_?11-g.sequence->blue_slot():g.sequence->blue_slot();
 for(int j=0;j<11;++j){if(live && !g.sequence->engine.teams[start+j].guy_on)continue;float x=235,y=342;int scale=5;
  if(j){const auto p=tactics_[formation][69][j-1];const float depth=std::clamp(float(p.x)/640.0f,0.0f,1.0f);const float span=414-depth*131;
   x=28+depth*65+std::clamp(float((p.y+16)&~31)/800.0f,0.0f,1.0f)*span;y=350-depth*158;scale=std::clamp(int((1-depth)*6),0,5);}
  const int shirt=std::clamp(order[j],1,20);const auto& glyph=menu_art::images[menu_art::FORM_IMGS+scale*20+shirt-1];
  art_->image(menu_art::FORM_IMGS+scale*20+shirt-1,int(x)-glyph.w/2,int(y)-glyph.h);
 }
 for(int j=0;j<20;++j){const int shirt=std::clamp(order[j],1,20);std::string name=original_teams[team].players[shirt-1].name;name=name.substr(name.find(' ')+1);
  const bool on=!live || j>=11 || g.sequence->engine.teams[start+j].guy_on;
  art_->text(513,102+j*18,on?std::to_string(shirt):"OFF",j==selected?Ink::cyan:Ink::white,0,2,28);
  art_->text(535,102+j*18,name,j==selected?Ink::cyan:Ink::white,0,0,103);
 }
}
void OriginalFrontend::draw_page(SDL_Renderer* r,const OriginalGraphics& g,bool pad,bool,int){
 if(!visible() && !g.prematch.active() && !g.walkoff.active())return;
 menu_hits_.clear();hit_stage_=stage_;const auto& s=*g.sequence;
 if(stage_!=Stage::selection && stage_!=Stage::tournament_setup)country_stage_=Stage::hidden;
 const auto text=[&](int x,int y,const std::string& value,Ink ink=Ink::white,int size=0,int align=0,int limit=620){art_->text(x,y,value,ink,size,align,limit);};
 if(g.walkoff.active() || g.prematch.active() || g.exit.active()){
  art_->clear();art_->panel(22,394,596,78);
  const bool paused=g.walkoff.active()?g.walkoff.paused:g.prematch.active()?g.prematch.paused:g.exit.paused;
  const std::string title=paused?"PAUSED":g.walkoff.active()?(g.walkoff.replacement()?"SUBSTITUTION":"SENT OFF"):g.prematch.active()?std::string(original_teams[s.home_team].name)+" V "+original_teams[s.away_team].name:stage_==Stage::interval?"HALF TIME":"FULL TIME";
  text(320,400,title,Ink::white,1,2,565);
  if(g.walkoff.active())text(320,427,g.walkoff.label(),Ink::cyan,0,2,565);
  text(320,450,pad?"A CONTINUE     START PAUSE":"ENTER CONTINUE     P PAUSE",Ink::white,0,2);
  art_->present(r);return;
 }
 check(SDL_SetRenderDrawColor(r,0,0,16,255));check(SDL_RenderClear(r));
 if(stage_==Stage::title){
  const auto entries=title_entries();
  art_->begin(0);art_->panel(354,119,280,int(entries.size())*39+22);
  for(int i=0;i<int(entries.size());++i){const int y=128+i*39;art_->button(362,y,274,entries[i].first,title_choice_==i,menu_clock_);
   menu_hit(360,y,278,38,[this,i,k=entries[i].second](OriginalGraphics& target){title_choice_=i;if(!i)confirm(target);else key(k,target);});}
  const auto status=!movie_error.empty()?movie_error:tournament_error_;
  menu_footer(status.empty()?(pad?"D-PAD SELECT     A CONFIRM     B BACK":"ARROWS SELECT     ENTER CONFIRM     ESC QUIT"):status);
 }else if(stage_==Stage::selection){
  const bool venue_preview=row_==7 && g.next_stadium>=5 && g.next_stadium<=12 && preview_ && preview_->count()>0;
  art_->begin(3);if(!venue_preview)menu_country(row_==1?s.next_away:s.next_home);
  text(12,99,"EXHIBITION",Ink::blue,1);
  text(235,venue_preview?106:159,venue_preview?g.stadium_names[g.next_stadium]:original_teams[row_==1?s.next_away:s.next_home].name,Ink::blue,venue_preview?1:2,2,venue_preview?220:422);
  if(venue_preview)menu_preview(320+g.next_stadium-5,74,129,320,240);
  art_->image(menu_art::GERL_SBAR,28,372);text(132,385,original_teams[s.next_home].name,Ink::blue,1,2,185);text(340,385,original_teams[s.next_away].name,Ink::blue,1,2,185);text(238,385,"V",Ink::dark,1,2);
  menu_hit(28,372,205,48,[this](OriginalGraphics&){row_=0;});menu_hit(241,372,205,48,[this](OriginalGraphics&){row_=1;});
  const std::string labels[]={"HOME","AWAY","HOME FORMATION","AWAY FORMATION","MATCH LENGTH","CAMERA","FORMAT","VENUE"};
  const std::string values[]={original_teams[s.next_home].name,original_teams[s.next_away].name,formation_names[s.next_home_formation],formation_names[s.next_away_formation],std::to_string(s.match_minutes)+" MIN",g.camera_name(),OriginalActions::format_name(s.next_format),g.stadium_available(g.next_stadium)?g.stadium_names[g.next_stadium]:"SOURCE"};
  for(int i=0;i<8;++i){const int y=96+i*39;text(476,y,labels[i],i==row_?Ink::cyan:Ink::white,0,0,158);text(486,y+18,values[i],Ink::white,0,0,143);
   if(i==row_)text(471,y+17,">",Ink::cyan);
   menu_hit(472,y,78,38,[this,i](OriginalGraphics& target){row_=i;change(target,-1);});menu_hit(550,y,90,38,[this,i](OriginalGraphics& target){row_=i;change(target,1);});}
  menu_button(8,416,195,pad?"A WATCH":"ENTER WATCH",SDLK_RETURN,true);menu_button(219,416,195,pad?"Y PLAY":"Y PLAY HOME",SDLK_Y);menu_button(430,416,202,pad?"X TWO PLAYERS":"H TWO PLAYERS",SDLK_H);
  menu_footer(local_message.empty()?(pad?"D-PAD CHANGE     RB OPTIONS     B RETURN":"ARROWS CHANGE     O OPTIONS     ESC RETURN"):local_message);
 }else if(stage_==Stage::lineup){
  const int team=side_?s.next_away:s.next_home,formation=side_?s.next_away_formation:s.next_home_formation;
  std::array<int,20> order{};for(int i=0;i<20;++i)order[i]=tournament_mode_?tournament_.squad[team][i]:i+1;
  art_->begin(1,team);menu_formation(g,team,formation,order,tournament_mode_?squad_cursor_:-1);
  menu_hit(22,106,418,38,[this](OriginalGraphics&){side_=1-side_;});
  for(int i=0;i<20;++i)menu_hit(480,100+i*18,160,18,[this,i](OriginalGraphics& target){if(tournament_mode_){squad_cursor_=i;squad_select(target);}});
  art_->bar(22,410,230);text(136,415,formation_names[formation],Ink::white,1,2,170);
  menu_button(258,410,194,pad?"A KICK OFF":"ENTER KICK OFF",SDLK_RETURN,true);
  menu_hit(22,410,230,38,[this](OriginalGraphics& target){open_tactics(target);});
  if(tournament_mode_){const auto& p=tournament_.players[team][order[squad_cursor_]-1];text(235,391,"FITNESS "+std::to_string(p.fitness)+"   BAN "+std::to_string(p.ban)+"   YELLOW "+std::to_string(p.yellows),Ink::white,0,2,432);}
  text(235,371,!local_message.empty()?local_message:!squad_message_.empty()?squad_message_:s.next_head_to_head?"P1 HOME / P2 AWAY":cpu_?"COMPUTER V COMPUTER":"YOU PLAY "+std::string(original_teams[s.next_home].name),Ink::blue,0,2,432);
  menu_footer(tournament_mode_?(pad?"Y SWAP    X TACTICS    B BACK":"SPACE SWAP    T TACTICS    ESC BACK"):(pad?"LEFT/RIGHT TEAM    X TACTICS    B BACK":"LEFT/RIGHT TEAM    T TACTICS    ESC BACK"));
 }else if(stage_==Stage::tactics){tactics_draw(r,g,pad);
 }else if(stage_==Stage::options){options_draw(r,pad);
 }else if(stage_==Stage::tournament || stage_==Stage::tournament_setup || stage_==Stage::simulating){tournament_draw(r,g,pad);
 }else if(stage_==Stage::replays){
  art_->begin(0);art_->panel(274,78,360,371);text(454,90,"SAVED REPLAYS",Ink::white,1,2,334);
  const int first=(replay_row_/6)*6;
  if(replay_entries_.empty())text(454,207,"NO SAVED REPLAYS",Ink::white,1,2,332);
  for(int i=first;i<std::min(first+6,int(replay_entries_.size()));++i){const int y=129+(i-first)*43;
   text(287,y,(i==replay_row_?"> ":"  ")+replay_entries_[i].label,i==replay_row_?Ink::cyan:Ink::white,0,0,334);
   text(300,y+19,replay_entries_[i].path.stem().string(),Ink::white,0,0,318);
   menu_hit(280,y,347,41,[this,i](OriginalGraphics&){replay_row_=i;});}
  text(454,390,replay_status_.empty()?std::to_string(replay_entries_.empty()?0:replay_row_+1)+" / "+std::to_string(replay_entries_.size()):replay_status_,Ink::cyan,0,2,334);
  menu_button(280,411,165,pad?"A PLAY":"ENTER PLAY",SDLK_RETURN,true);menu_button(453,411,175,"REFRESH",SDLK_R);
  menu_footer(pad?"D-PAD SELECT     B RETURN":"ARROWS SELECT     PGUP/PGDN PAGE     ESC RETURN");
 }else{
  art_->begin(7);text(235,102,stage_==Stage::interval?s.period_label():"FULL TIME",Ink::blue,0,2,420);
  text(237,126,std::string(original_teams[s.home_team].code)+"   "+std::to_string(s.blue_goals)+" : "+std::to_string(s.red_goals)+"   "+original_teams[s.away_team].code,Ink::blue,1,2,418);
  const auto figures=s.match_figures();const char* labels[]={"TACKLES WON","PASSES COMPLETED","SHOTS","POSSESSION","WORK","CORNERS","DIRECT FREE KICKS","PENALTIES","OFFSIDES","BOOKED","SENT OFF"};
  for(int row=0;row<11;++row){const int y=165+row*21;text(168,y,labels[row],Ink::white,0,2,193);
   for(int side=0;side<2;++side){const auto& f=figures[side];std::string value;
    if(row==0)value=std::to_string(f[1])+"/"+std::to_string(f[0]);else if(row==1)value=std::to_string(f[3])+"/"+std::to_string(f[2]);else if(row==2)value=std::to_string(f[4]);else if(row==3){const int total=figures[0][6]+figures[1][6];value=total?std::to_string(f[6]*100/total)+"%":"-";}else if(row==4)value=std::to_string(f[7]);else value=std::to_string(f[row+4]);
    text(side?298:42,y,value,Ink::cyan,0,2,60);}}
  text(426,173,original_teams[s.home_team].name,Ink::white,0,2,145);text(426,205,"V",Ink::cyan,0,2);text(426,237,original_teams[s.away_team].name,Ink::white,0,2,145);
  text(426,299,s.shootout?"PENALTIES":stage_==Stage::interval?s.period_label():"RESULT",Ink::cyan,0,2,146);
  text(426,337,s.shootout?std::to_string(s.penalty_scores[0])+" : "+std::to_string(s.penalty_scores[1]):stage_==Stage::result?s.result_label():s.cpu_vs_cpu?"CONTINUE IN "+std::to_string(std::max(1,8-int(interval_age_))):"READY",Ink::white,0,2,146);
  if(stage_==Stage::interval){menu_button(18,410,242,"CONTINUE",SDLK_RETURN,true);menu_button(270,410,242,"TACTICS",SDLK_T);}
  else{menu_button(18,410,242,tournament_mode_?"TOURNAMENT":"TEAM SELECT",SDLK_RETURN,true);if(!tournament_mode_)menu_button(270,410,242,"REMATCH",SDLK_R);}
  menu_footer(pad?"A CONTINUE     RB OPTIONS":"ENTER CONTINUE     O OPTIONS");
 }
 art_->present(r);
}
void OriginalFrontend::tactics_draw(SDL_Renderer*,const OriginalGraphics& g,bool pad){
 const auto& s=*g.sequence;const bool live=tactics_return_==Stage::interval;const int team=tactics_team(g);const auto order=tactics_squad(g);
 const auto text=[&](int x,int y,const std::string& value,Ink ink=Ink::white,int size=0,int align=0,int limit=620){art_->text(x,y,value,ink,size,align,limit);};
 const auto surname=[&](int shirt){std::string n=original_teams[team].players[shirt-1].name;return n.substr(n.find(' ')+1);};
 if(tactics_page_==0){
  const int formation=live?(side_?s.away_formation:s.home_formation):(side_?s.next_away_formation:s.next_home_formation);
  art_->begin(1,team);menu_formation(g,team,formation,order,tactics_player_);
  art_->bar(22,410,270);text(157,415,formation_names[formation],Ink::white,1,2,205);
  menu_hit(22,410,135,38,[this](OriginalGraphics& t){tactics_change(t,-1);});menu_hit(157,410,135,38,[this](OriginalGraphics& t){tactics_change(t,1);});
 }else if(tactics_page_==1){
  art_->begin(1,team);art_->bar(22,106,418);text(231,110,std::string(original_teams[team].name)+" TAKERS",Ink::white,1,2,386);
  art_->panel(25,157,416,226);
  const char* labels[]={"PENALTIES","FREE KICKS","CORNERS"};const auto& choices=live?s.match_takers[side_]:s.next_takers[side_];
  for(int row=0;row<3;++row){const int y=174+row*65;const int shirt=choices[row];text(48,y,labels[row],row==tactics_row_?Ink::cyan:Ink::white,1);text(62,y+29,shirt?std::to_string(shirt)+" "+surname(shirt):"AUTOMATIC",Ink::white,0,0,350);
   bool eligible=false;for(int j=1;j<11;++j)if(order[j]==shirt){const int start=side_?11-s.blue_slot():s.blue_slot();eligible=live?s.engine.teams[start+j].guy_on:!tournament_mode_ || tournament_.available(team,shirt);}
   if(shirt && !eligible)text(62,y+47,"OFF PITCH - AUTO",Ink::cyan,0,0,350);
   menu_hit(25,y,208,62,[this,row](OriginalGraphics& t){tactics_row_=row;tactics_change(t,-1);});menu_hit(233,y,208,62,[this,row](OriginalGraphics& t){tactics_row_=row;tactics_change(t,1);});}
  for(int i=0;i<20;++i)text(485,102+i*18,std::to_string(order[i])+" "+surname(order[i]),Ink::white,0,0,150);
  text(230,391,"FREE KICKS: SHOOTING RANGE",Ink::blue,0,2,420);
 }else{
  art_->begin(5,team);const int shirt=order[tactics_player_];const auto& p=original_teams[team].players[shirt-1];
  menu_preview(team*20+shirt-1,294,114,140,285);
  const int fitness=live?s.match_fitness[side_][shirt-1]:tournament_mode_?tournament_.players[team][shirt-1].fitness:100;const auto fit=conditioned_stats(p,fitness);
  art_->image(menu_art::SMLL_GOTH+team,550,2,0,0,team);art_->image(menu_art::SQUD_BALS+shirt-1,28,100);
  text(60,100,surname(shirt),Ink::white,1,0,210);text(36,138,p.position,Ink::white,0,0,204);text(36,174,"FITNESS  "+std::to_string(fitness),Ink::white);text(36,207,"BASE / MATCH",Ink::cyan);
  const char* labels[]={"PACE","POWER","CONTROL","FLAIR","VISION","ACCURACY","STAMINA","DISCIPLINE"};const int a[]={p.pace,p.power,p.control,p.flair,p.vision,p.accuracy,p.stamina,p.discipline};const int b[]={fit.pace,fit.power,fit.control,fit.flair,fit.vision,fit.accuracy,fit.stamina,fit.discipline};
  art_->panel(19,259,258,194);
  for(int i=0;i<8;++i){text(31,270+i*22,labels[i],Ink::white,0,0,132);text(263,270+i*22,std::to_string(a[i])+" / "+std::to_string(b[i]),Ink::cyan,0,1,94);}
  for(int i=0;i<20;++i)text(485,102+i*18,std::to_string(order[i])+" "+surname(order[i]),i==tactics_player_?Ink::cyan:Ink::white,0,0,150);
 }
 for(int i=0;i<20;++i)menu_hit(480,100+i*18,160,18,[this,i](OriginalGraphics&){tactics_player_=i;if(i>=11 && tactics_page_==0)tactics_page_=2;});
 if(tactics_page_==2){art_->text(367,92,original_teams[team].name,Ink::blue,0,2,166);menu_hit(288,88,170,26,[this](OriginalGraphics& target){tactics_key(SDLK_TAB,target);});}
 else menu_hit(22,106,418,38,[this](OriginalGraphics& target){tactics_key(SDLK_TAB,target);});
 menu_button(tactics_page_?288:310,410,tactics_page_?170:140,"NEXT PAGE",SDLK_Y);
 if(tactics_page_==1)menu_button(288,362,170,"RETURN",SDLK_RETURN);
 menu_footer(!tactics_message_.empty()?tactics_message_:!tactics_editable(g)?"OPPOSITION - VIEW ONLY":pad?"D-PAD CHANGE   Y PAGE   LB/RB TEAM":"ARROWS CHANGE   Y PAGE   TAB TEAM");
}
void OriginalFrontend::options_draw(SDL_Renderer*,bool pad){
 art_->begin(0);art_->panel(265,82,368,367);art_->text(449,91,options_page_?"SOUND OPTIONS":"MATCH OPTIONS",Ink::white,1,2,338);
 const auto& p=preferences_;const auto& m=p.match;const char* winds[]={"OFF","LIGHT","MEDIUM","STRONG"};
 const char* matchLabels[]={"WIND","OFFSIDE","FOULS","BOOKINGS","REF ACCURACY","REF STRICTNESS"};const char* audioLabels[]={"MUSIC","PITCH EFFECTS","CROWD","COMMENTARY","MASTER SOUND","VOICE LANGUAGE"};
 const auto on=[](bool v){return std::string(v?"ON":"OFF");};const std::string values[]={winds[m.wind],on(m.offside),on(m.fouls),on(m.bookings),std::to_string(m.referee_accuracy),std::to_string(m.referee_strictness)};const int levels[]={p.music,p.effects,p.crowd,p.commentary};
 for(int i=0;i<6;++i){const int y=135+i*39;art_->text(280,y,(i==options_row_?"> ":"  ")+std::string(options_page_?audioLabels[i]:matchLabels[i]),i==options_row_?Ink::cyan:Ink::white,0,0,209);
  art_->text(615,y,options_page_?(i<4?std::to_string(levels[i])+"%":i==4?on(!p.muted):p.commentary_language?"FRENCH":"ENGLISH"):values[i],Ink::white,0,1,118);
  menu_hit(272,y-4,169,37,[this,i](OriginalGraphics& target){options_row_=i;options_change(target,-1);});menu_hit(441,y-4,184,37,[this,i](OriginalGraphics& target){options_row_=i;options_change(target,1);});}
 art_->text(449,377,preferences_status_.empty()?(options_page_?"AUDIO CHANGES APPLY NOW":"CHANGES APPLY NEXT KICKOFF"):preferences_status_,Ink::cyan,0,2,339);
 menu_button(275,409,170,"RETURN",SDLK_RETURN);menu_button(452,409,174,"NEXT PAGE",SDLK_Y);
 menu_footer(pad?"D-PAD CHANGE     Y PAGE     B RETURN":"ARROWS CHANGE     Y PAGE     ESC RETURN");
}
void OriginalFrontend::tournament_draw(SDL_Renderer*,const OriginalGraphics& g,bool pad){
 const auto text=[&](int x,int y,const std::string& value,Ink ink=Ink::white,int size=0,int align=0,int limit=620){art_->text(x,y,value,ink,size,align,limit);};
 const auto name=[](int t){return t<0?std::string("TBD"):std::string(original_teams[t].name);};const auto code=[](int t){return t<0?std::string("TBD"):std::string(original_teams[t].code);};
 if(stage_==Stage::tournament_setup){
  art_->begin(3);if(tournament_team_>=0)menu_country(tournament_team_);else country_stage_=Stage::hidden;text(12,99,"EURO 96 CHAMPIONSHIP",Ink::blue,1);text(233,164,tournament_team_<0?"WATCH ALL TEAMS":name(tournament_team_),Ink::blue,2,2,420);
  art_->image(menu_art::GERL_SBAR,28,372);text(237,386,std::to_string(g.sequence->match_minutes)+" MINUTE MATCHES",Ink::blue,1,2,395);
  text(486,123,"YOUR COUNTRY",Ink::white,0,0,148);text(486,153,"LEFT / RIGHT",Ink::cyan);text(486,212,"MATCH LENGTH",Ink::white);text(486,242,"UP / DOWN",Ink::cyan);
  const bool scheduled=std::all_of(OriginalTournament::fixture_venues.begin(),OriginalTournament::fixture_venues.end(),[&](int venue){return g.stadium_available(venue);});
  text(486,301,"VENUES",Ink::white);text(486,331,scheduled?"ORIGINAL SCHEDULE":"SELECTED STADIUM",Ink::cyan,0,0,148);
  menu_hit(40,220,195,140,[this](OriginalGraphics& t){key(SDLK_LEFT,t);});menu_hit(235,220,200,140,[this](OriginalGraphics& t){key(SDLK_RIGHT,t);});menu_hit(28,372,418,42,[this](OriginalGraphics& t){key(SDLK_UP,t);});
  menu_button(28,416,250,"START CHAMPIONSHIP",SDLK_RETURN,true);menu_button(310,416,240,"RETURN",SDLK_ESCAPE);
  menu_footer("NEW CHAMPIONSHIP REPLACES THE CURRENT SAVE");return;
 }
 if(stage_==Stage::simulating){
  art_->begin(10);const auto& s=*g.sequence;
  // NEXT_MATCH uses its complete baked palette, not the small squad mascot's
  // team palette overlay. The home sprite is flipped in EURO_GEN.CPP.
  art_->image(menu_art::LRGE_GOTH+s.home_team,24,146,0,0,-1,true);art_->image(menu_art::LRGE_GOTH+s.away_team,252,146);
  text(118,108,name(s.home_team),Ink::blue,1,2,211);text(346,108,name(s.away_team),Ink::blue,1,2,211);
  text(237,414,std::to_string(s.blue_goals)+" : "+std::to_string(s.red_goals),Ink::white,2,2,190);text(551,161,s.clock_label(),Ink::white,1,2,145);
  text(551,129,"SIMULATING",Ink::cyan,0,2,145);menu_button(475,410,158,"RETURN",SDLK_ESCAPE);menu_footer("MATCH IN PROGRESS");return;
 }
 const int played=int(tournament_.results.size());
 const std::string next_label=tournament_.complete()?(trophy_available_?"TROPHY FILM":"COMPLETE"):tournament_match_?"RESUME":"NEXT MATCH";
 const bool groups=tournament_page_<4;
 if(groups){
  art_->begin(2);
  const int round=tournament_page_?tournament_page_-1:std::min(2,played/8);
  for(int group=0;group<4;++group){const int x=(group%2)*251, y=(group/2)*192;
   const int cols[]={144,173,202,230};const char* labels[]={"W","D","L","PTS"};for(int c=0;c<4;++c)text(x+cols[c],y+117,labels[c],Ink::white,0,2,30);
   for(int pos=0;pos<4;++pos){const auto it=std::find(tournament_.positions.begin(),tournament_.positions.end(),group*4+pos);const int team=int(it-tournament_.positions.begin());const auto& row=tournament_.table[team];
    text(x+33,y+139+pos*22,name(team),team==tournament_.user?Ink::cyan:Ink::white,0,0,103);const int v[]={row.won,row.drew,row.lost,row.points};for(int c=0;c<4;++c)text(x+cols[c],y+139+pos*22,std::to_string(v[c]),Ink::white,0,2,30);}
   int line=0;for(int fixture=round*8;fixture<round*8+8;++fixture){const auto teams=tournament_.fixture(fixture);if(tournament_.draw[teams[0]]/4!=group)continue;
    const std::string score=fixture<played?" "+std::to_string(tournament_.results[fixture].hg)+":"+std::to_string(tournament_.results[fixture].ag)+" ":" V ";
    text(x+137,y+225+line++*21,code(teams[0])+score+code(teams[1]),fixture==played?Ink::cyan:Ink::white,0,2,210);}
   menu_hit(x+20,y+97,236,175,[this](OriginalGraphics&){tournament_page_=(tournament_page_+1)%4;});
  }
  menu_button(513,101,123,tournament_.complete()?(trophy_available_?"TROPHY":"COMPLETE"):tournament_match_?"RESUME":"NEXT",SDLK_RETURN,true);menu_button(513,146,123,"SIM",SDLK_Y);menu_button(513,191,123,tournament_auto_?"AUTO ON":"AUTO OFF",SDLK_A);menu_button(513,236,123,"ROUNDS",SDLK_RIGHT);
  // Right arrow at the round button explicitly opens the bracket.
  menu_hits_.back().action=[this](OriginalGraphics&){tournament_page_=4;};
  menu_button(513,281,123,"FIXTURES",SDLK_RIGHT);menu_hits_.back().action=[this](OriginalGraphics&){tournament_page_=(tournament_page_+1)%4;};
  text(575,326,"ROUND "+std::to_string(round+1),Ink::cyan,0,2,110);
  if(!tournament_.complete()){text(575,357,"NEXT VENUE",Ink::white,0,2,120);text(575,379,OriginalGraphics::stadium_names[tournament_.fixture_venue(played)],Ink::cyan,0,2,120);}
 }else{
  const int screen=tournament_page_+8;art_->begin(screen);
  if(screen==14){art_->image(menu_art::EUCP_IMGS+int(menu_clock_*8)%30,167,129);text(253,112,tournament_.complete()?name(tournament_.champion())+" WINS":"THE FINAL",Ink::blue,1,2,430);}
  const int first=screen==12?24:screen==13?28:30;const int count=screen==12?4:screen==13?2:1;
  for(int n=0;n<count;++n){const int i=first+n;const auto t=tournament_.fixture(i);
   const int y=screen==12?139+n*88:screen==13?180+n*121:396;
   const auto* result=i<played?&tournament_.results[i]:nullptr;
   text(134,y,name(t[0])+(result?"  "+std::to_string(result->hg):""),Ink::white,1,2,174);
   text(322,y,name(t[1])+(result?"  "+std::to_string(result->ag):""),Ink::white,1,2,174);
   if(result)text(226,y+(screen==12?35:39),result->pens?"PENALTIES "+std::to_string(result->hp)+" : "+std::to_string(result->ap):result->extra?"AFTER EXTRA TIME":"FULL TIME",Ink::cyan,0,2,360);
   else text(226,y,"V",Ink::cyan,1,2,20);
  }
  menu_button(475,260,158,next_label,SDLK_RETURN,true);menu_button(475,299,158,"SIM NEXT",SDLK_Y);
  menu_button(475,338,158,tournament_auto_?"AUTO ON":"AUTO OFF",SDLK_A);
  menu_button(475,377,158,"PREVIOUS",SDLK_LEFT);menu_button(475,416,158,"NEXT PAGE",SDLK_RIGHT);
 }
 menu_footer(!movie_error.empty()?movie_error:!tournament_error_.empty()?tournament_error_:tournament_.complete() && trophy_available_?(pad?"A TROPHY FILM     B RETURN":"ENTER TROPHY FILM     ESC RETURN"):save_status_.empty()?(pad?"A NEXT MATCH     Y SIMULATE     B RETURN":"ENTER NEXT MATCH     Y SIMULATE     ESC RETURN"):save_status_);
}
}
