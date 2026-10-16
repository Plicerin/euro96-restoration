#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "original_audio.hpp"
#include "graphics_data.hpp"
#include "action_constants.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace actua;
static void require(bool v,const char* m){if(!v)throw std::runtime_error(m);}
static std::uint64_t hash(std::span<const unsigned char> b){std::uint64_t h=1469598103934665603ull;for(auto c:b){h^=c;h*=1099511628211ull;}return h;}
int main(int argc,char** argv){try{
 require(argc==4,"source, CD media and capture folder required");SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
 const std::filesystem::path folder=argv[3];std::filesystem::create_directories(folder);
 OriginalCdKits kits;kits.load(argv[2]);
 require(kits.away_kit(4,6)==17,"Original England/Germany table selects blue alternate");
 require(kits.away_kit(5,12)==16,"Original France/Scotland table selects white alternate");
 require(kits.away_kit(4,12)==12,"England/Scotland retains original Scotland kit");
 require(kits.away_kit(1,6)==6,"Unmatched Croatia table never fabricates a clash entry");
 require(kits.alternate_skin(4,19),"CD Paul Ince appearance follows Euro roster identity, not CD slot");
 require(!kits.alternate_skin(4,9),"CD Alan Shearer default appearance");
 for(int side:{2,13})for(bool mirror:{false,true}){
  const auto normal=OriginalGraphics::numbered_player_faces(side,mirror,14,false),alternate=OriginalGraphics::numbered_player_faces(side,mirror,14,true);
  require(normal!=alternate && normal[82]==alternate[82],"Original alternate appearance changes mesh materials while preserving substitute number");
 }
 require(OriginalGraphics::player_faces(1,false,false,true)==recovered::player_fg1,"Original goalkeeper appearance exception");
 OriginalVideoBoard board(argv[1]);const auto initial=hash(board.pixels());
 for(int e=1;e<=6;++e){board.play(static_cast<OriginalVideoBoard::Event>(e));auto start=hash(board.pixels());bool changed=false;
  for(int t=0;t<100;++t){board.advance(.05);changed|=hash(board.pixels())!=start;}
  require(changed,"Every recovered stadium clip animates");
  for(int t=0;t<500;++t)board.advance(.05);
  require(board.event()==0 && hash(board.pixels())==initial,"Original event count returns video board to idle");
 }
 std::array<unsigned char,2> tiny{};bool rejected=false;try{const Bytes bad{3,1,2,3,0,0};OriginalVideoBoard::delta(bad,0,tiny);}catch(const std::exception&){rejected=true;}
 require(rejected,"Malformed video run cannot overwrite output");
 rejected=false;try{const Bytes bad{1,1};OriginalVideoBoard::delta(bad,0,tiny);}catch(const std::exception&){rejected=true;}
 require(rejected,"Truncated video skip rejected");
 OriginalGraphics graphics(argv[1]);graphics.load_cd_graphics(argv[2]);graphics.action_view=true;auto& s=*graphics.sequence;auto& e=s.engine;
 s.next_home=4;s.next_away=6;s.start_match(true);board.observe(s);++e.foul_events;board.observe(s);require(board.event()==2,"Foul counter drives board");
 ++e.goal_events;board.observe(s);require(board.event()==6,"Goal counter drives board");
 s.interval_ticks=40;s.interval_minute=45;board.observe(s);require(board.event()==4,"Half-time board");s.interval_ticks=0;board.observe(s);
 s.interval_ticks=40;s.interval_minute=90;board.observe(s);require(board.event()==1,"Extra-time board");s.full_time=true;board.observe(s);require(board.event()==3,"Full-time board");
 s.start_match(true);board.observe(s);require(board.event()==0,"New match resets stadium event");e.match_mode=17;board.observe(s);require(board.event()==5,"Penalty board");
 OriginalCrowd crowd;original::PlayerEngine scenario=e;scenario.match_mode=0;scenario.just_scored=0;scenario.shot_pending=0;scenario.ballx=scenario.cntspot_x;scenario.ball_poss=2;
 require(crowd.update(scenario)==std::array<int,2>{1,1},"Midfield crowd calm");scenario.ballx=1100;
 for(int i=0;i<180;++i)crowd.update(scenario);
 require(crowd.update(scenario)==std::array<int,2>{4,2},"Established attack excites attackers and worries defenders");
 scenario.ball_in_hands=1;scenario.ball_poss=12;require(crowd.update(scenario)==std::array<int,2>{1,1},"Keeper catch calms both crowds");
 scenario.ball_in_hands=0;scenario.last_touch=2;crowd.shot_missed=-2;require(crowd.update(scenario)==std::array<int,2>{6,9},"Bad miss produces boo/laugh moods");crowd.shot_missed=1;require(crowd.update(scenario)==std::array<int,2>{8,2},"Near miss follows original attacking-side reaction");
 const auto seed=e.seed;const auto possession=e.possession;OriginalAudio audio(argv[2],false);audio.update(s,graphics.camera_pose(),true);
 std::vector<float> pcm(48000*2);audio.mix(pcm.data(),48000);require(std::any_of(pcm.begin(),pcm.end(),[](float v){return std::abs(v)>.001f;}),"Recovered crowd ambience emits PCM");
 require(e.seed==seed && e.possession==possession,"Crowd integration never mutates gameplay possession or RNG");
 audio.set_muted(true);audio.mix(pcm.data(),48000);require(std::all_of(pcm.begin(),pcm.end(),[](float v){return v==0;}),"Crowd layers obey mute");
 auto* surface=SDL_CreateSurface(960,700,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 s.start_match(true);graphics.draw(renderer);SDL_RenderPresent(renderer);
 for(int i=0;i<24;++i)for(int c=0;c<3;++c)require(graphics.palette_color(56+i)[c]==kits.part(17,4)[i*3+c],"Actual away renderer uses clash-selected alternate palette");
 for(int i=0;i<8;++i)for(int c=0;c<3;++c)require(graphics.palette_color(88+i)[c]==kits.part(6,5)[i*3+c],"Alternate kit preserves Germany skin palette");
 require(SDL_SaveBMP(surface,(folder/"batch-england-germany.bmp").string().c_str()),SDL_GetError());
 for(int t=0;t<100;++t)graphics.advance(.05);graphics.paused=true;const int live_ticks=s.ticks;const auto live_seed=e.seed;const float ball=e.ballx;
 require(graphics.start_replay(),"Replay history available");
 for(int view=0;view<5;++view){graphics.match_camera=view;graphics.change_camera(0);graphics.draw(renderer);SDL_RenderPresent(renderer);const auto pose=graphics.camera_pose();
  require(std::isfinite(pose.x) && std::isfinite(pose.y) && std::isfinite(pose.z) && graphics.polygon_count()>0,"Every original replay view renders finite geometry");
  require(SDL_SaveBMP(surface,(folder/("batch-replay-"+std::to_string(view)+".bmp")).string().c_str()),SDL_GetError());
  for(int t=0;t<4;++t)graphics.advance(.05);
 }
 graphics.finish_replay();require(graphics.match_camera==0 && graphics.paused,"Replay restores live camera and pause");require(s.ticks==live_ticks && e.seed==live_seed && e.ballx==ball,"Replay camera changes do not advance live gameplay");
 graphics.draw(renderer);SDL_RenderPresent(renderer);const auto frame=hash({static_cast<unsigned char*>(surface->pixels),std::size_t(surface->pitch*surface->h)});graphics.advance(.1);graphics.draw(renderer);SDL_RenderPresent(renderer);
 require(frame==hash({static_cast<unsigned char*>(surface->pixels),std::size_t(surface->pitch*surface->h)}),"Paused stadium, players and camera remain stable");
 // Produce a contact sheet directly from the verified decoder/palettes.
 auto* sheet=SDL_CreateSurface(128*7,80,SDL_PIXELFORMAT_RGBA32);Archive archive(std::filesystem::path(argv[1])/"EUROREND.DAT",std::filesystem::path(argv[1])/"EUROREND.OFF");
 for(int event=0;event<7;++event){board.play(static_cast<OriginalVideoBoard::Event>(event));for(int i=0;i<20;++i)board.advance(.055);Bytes pal(archive.entry(0).begin(),archive.entry(0).end());std::copy(board.colors().begin(),board.colors().end(),pal.begin()+720);
  for(int y=0;y<80;++y)for(int x=0;x<128;++x){const int index=board.pixels()[y*128+x];auto* pixel=static_cast<unsigned char*>(sheet->pixels)+y*sheet->pitch+(x+event*128)*4;for(int c=0;c<3;++c)pixel[c]=pal[index*3+c]*4;pixel[3]=255;}
 }
 SDL_SaveBMP(sheet,(folder/"batch-stadium-clips.bmp").string().c_str());SDL_DestroySurface(sheet);
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();std::cout<<"Presentation batch: alternate kits, matched appearances, 6 stadium clips, 5 replay views, original crowd moods passed\n";return 0;
}catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';SDL_Quit();return 1;}}
