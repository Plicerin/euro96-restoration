#include "original_frontend.hpp"
#include "original_movie.hpp"
#include <iostream>
#include <fstream>
#include <limits>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::uint64_t hash(const Bytes& b){std::uint64_t h=1469598103934665603ull;for(auto v:b){h^=v;h*=1099511628211ull;}return h;}
static std::uint64_t hash(SDL_Surface* image){auto* rgba=SDL_ConvertSurface(image,SDL_PIXELFORMAT_RGBA32);require(rgba,SDL_GetError());Bytes b;for(int y=0;y<rgba->h;++y){auto* row=static_cast<unsigned char*>(rgba->pixels)+y*rgba->pitch;b.insert(b.end(),row,row+rgba->w*4);}SDL_DestroySurface(rgba);return hash(b);}
int main(int argc,char** argv){try{
 require(argc==5,"Expected source/media/docs/scratch");require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO),SDL_GetError());
 const std::filesystem::path media=argv[2],docs=argv[3],scratch=argv[4];std::filesystem::create_directories(scratch);
 {
 OriginalPreview preview(media);require(preview.count()==109,"Verified preview catalog");
 std::ifstream checks(docs/"euro96-preview-samples.txt");int id,frame,checked=0;std::uint64_t expected;
 while(checks>>id>>frame>>expected){require(preview.select_entry(id),"Select FAP");require(hash(preview.frames().at(frame))==expected,"Independent Python/C++ FAP pixel agreement");++checked;}
 require(checked==327,"All FAP samples checked");int available=0;
 for(int i=0;i<328;++i)if(preview.select(i)){require(preview.width()==(i<320?140:320),"Slot dimensions");++available;}
 require(available==328,"All 320 verified identities and eight venues");
 preview.select(320);auto before=preview.pixels();preview.advance(.1);require(preview.frame()==2 && before!=preview.pixels(),"Preview advances original frames");
 preview.advance(0);preview.advance(std::numeric_limits<double>::quiet_NaN());require(preview.frame()==2,"Focus/invalid time holds preview");
 for(int i=0;i<240;++i)preview.advance(1.0/20.4);require(preview.frame()==2,"Preview loops without drift");preview.select(-1);require(preview.pixels().empty(),"Missing identity clears prior actor");
 Bytes bad(1026);for(int mode=0;mode<5;++mode){bad.assign(1026,0);bad[0]=1;if(mode==1)bad[4]=255;if(mode==2)bad[1024]=255;if(mode==3)bad.back()=1;if(mode==4)bad.resize(1024);bool rejected=false;try{OriginalPreview::decode(bad,140,285);}catch(...){rejected=true;}require(rejected,"Malformed FAP rejected");}
 OriginalMovie movie(media);std::ifstream film_checks(docs/"euro96-trophy-samples.txt");checked=0;
 for(int film=0;film<32;++film){movie.start_trophy(film);require(movie.frames()==983 && movie.channels()==2 && movie.audio_bytes()==983*1837*2,"Original stereo trophy metadata");
  for(int n=0;n<983;++n){auto* image=movie.read_frame(n);require(image->w==320 && image->h==240,"Every trophy frame decodes");
   if(n==0 || n==180 || n==982){require(bool(film_checks>>id>>frame>>expected) && id==film && frame==n,"Trophy sample coverage");require(hash(image)==expected,"Trophy pixels agree with independent importer");++checked;}
   if(film==4 && n==180)SDL_SavePNG(image,(scratch/"england-trophy.png").string().c_str());SDL_DestroySurface(image);
  }
 }
 require(checked==96,"All trophy samples checked");movie.seek(5);movie.update(.02,true,true,80);require(movie.queued_audio()>0,"Stereo PCM queues");
 movie.update(.02,false,true,80);require(movie.paused() && movie.queued_audio()==0,"Focus loss holds film and clears stereo PCM");movie.toggle_pause();movie.update(.02,true,false,80);require(movie.queued_audio()==0,"Mute silences stereo film");
 movie.seek(999);movie.update(.2,true,true,80);require(!movie.active(),"Trophy natural end releases audio");movie.start_trophy(0);movie.stop();require(!movie.active(),"Trophy skip");
 const auto broken=scratch/"bad";std::filesystem::create_directories(broken/"VIDEO");Bytes b(268,0);std::copy_n(reinterpret_cast<const unsigned char*>("EU96CUP1"),8,b.begin());b[8]=32;b[12]=255;b[13]=255;
 {std::ofstream out(broken/"VIDEO/EURO_CUP.BIN",std::ios::binary);out.write(reinterpret_cast<const char*>(b.data()),b.size());}OriginalMovie malformed(broken,false);bool rejected=false;try{malformed.start_trophy(0);}catch(...){rejected=true;}require(rejected && !malformed.active(),"Malformed trophy offsets rejected");
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 OriginalGraphics g(argv[1]);g.load_cd_graphics(media);g.prematches_enabled=false;g.exits_enabled=false;const auto save=scratch/"preview-tournament.sav";OriginalFrontend front(argv[1],media,save);front.menu_fades=false;front.open(g);
 const auto key=[&](SDL_Keycode k){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;front.event(e,g);};
 const auto draw=[&](const char* name){front.draw(renderer,g,false,false);auto* capture=SDL_RenderReadPixels(renderer,nullptr);require(capture,SDL_GetError());require(SDL_SavePNG(capture,(scratch/name).string().c_str()),SDL_GetError());SDL_DestroySurface(capture);};
 key(SDLK_RETURN);for(int i=0;i<7;++i)key(SDLK_DOWN);g.next_stadium=5;draw("venue.png");auto pixels=front.menu_art().pixels();front.observe(g,.2);draw("venue-animated.png");require(pixels!=front.menu_art().pixels(),"Venue page animation is wired");
 key(SDLK_Y);g.sequence->next_home=4;key(SDLK_T);key(SDLK_Y);key(SDLK_Y);draw("player.png");pixels=front.menu_art().pixels();front.observe(g,.2);draw("player-animated.png");require(pixels!=front.menu_art().pixels(),"Player page animation is wired");
 int selected=0;
 for(const auto [team,slot]:std::array<std::pair<int,int>,3>{{{9,11},{6,8},{15,19}}}){
  g.sequence->next_home=team;while(selected!=slot){key(SDLK_DOWN);selected=(selected+1)%20;}
  draw(team==9?"neno.png":team==6?"klinsmann.png":"osman.png");require(preview.select(team*20+slot),"Recovered player is available");
  const auto& expected_pixels=preview.pixels();const auto& actual=front.menu_art().pixels();
  for(int y=0;y<285;++y)for(int x=0;x<140;++x)for(int c=0;c<4;++c)require(actual[((114+y)*640+294+x)*4+c]==expected_pixels[(y*140+x)*4+c],"Recovered identity draws its own verified preview in the real menu");
 }
 OriginalTournament tournament;tournament.reset(-1);for(int i=0;i<30;++i){auto f=tournament.next();tournament.record({f[0],f[1],2,1});}tournament.save(save);
 front.open(g);key(SDLK_L);key(SDLK_Y);require(front.stage()==OriginalFrontend::Stage::simulating,"Final simulation starts");g.sequence->blue_goals=2;g.sequence->red_goals=1;g.sequence->finish_match();front.observe(g,.01);
 require(front.tournament().complete() && front.stage()==OriginalFrontend::Stage::tournament,"Final result recorded");const auto final=front.tournament().results.back();const auto film=g.trophy_film(final.home,final.away,front.tournament().champion());require(film>=0 && front.trophy_requested==film,"Final selects champion and actual final kit");front.trophy_requested=-1;
 const auto count=front.tournament().results.size();const auto tick=g.sequence->ticks;key(SDLK_RETURN);require(front.trophy_requested==film && front.tournament().results.size()==count && g.sequence->ticks==tick,"Trophy replay does not play or record another match");
 draw("champion.png");front.trophy_requested=-1;front.open(g);key(SDLK_L);require(front.trophy_requested<0,"Save resume does not force another film");key(SDLK_RETURN);require(front.trophy_requested==film,"Completed save offers trophy replay");
 front.trophy_requested=-1;tournament.save(save);front.open(g);key(SDLK_L);key(SDLK_RETURN);key(SDLK_RETURN);
 g.sequence->blue_goals=1;g.sequence->red_goals=2;g.sequence->finish_match();front.observe(g,.01);
 require(front.stage()==OriginalFrontend::Stage::result && front.trophy_requested<0,"Watched final shows result before trophy");
 key(SDLK_RETURN);require(front.stage()==OriginalFrontend::Stage::tournament && front.trophy_requested>=0 && front.trophy_requested%16==front.tournament().champion(),"Leaving watched final opens winning country's trophy film");
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);
 std::cout<<"109 FAPs / 327 independent samples, 328 mapped slots, 31456 trophy frames / 96 samples, stereo/pause/seek/end, corrupt media, venue/player menus and championship routing passed.\n";
 }
 SDL_Quit();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
