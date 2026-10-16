#include "original_video_board.hpp"
#include "original_graphics.hpp"
#include "original_actions.hpp"
#include "original_menu_overlays.hpp"
#include "original_menu_art.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 require(argc==4,"Source, media and scratch required");const std::filesystem::path media=argv[2],scratch=argv[3];std::filesystem::create_directories(scratch);
 auto bytes=read_file(media/"GRAPHICS/EURO_BOARDS.BIN");
 const auto word=[&](std::size_t at){unsigned n=0;for(int i=0;i<4;++i)n|=unsigned(bytes.at(at+i))<<(8*i);return n;};
 struct Clip{int event,record,count;std::size_t at;};std::vector<Clip> clips;std::size_t at=780;
 for(unsigned i=0;i<word(8);++i){const int event=word(at),record=word(at+4),count=word(at+8);at+=12;clips.push_back({event,record,count,at});at+=std::size_t(count)*10240;}
 require(at==bytes.size(),"Independent pack layout");
 OriginalVideoBoard board(argv[1]);const auto old_idle=board.pixels();board.load_retail(media);board.use_retail(true);
 std::srand(17);const int expected=std::rand();std::srand(17);
 for(int event:{0,2,4,6,7,8}){
  std::vector<Clip> choices;for(const auto& clip:clips)if(clip.event==event)choices.push_back(clip);
  for(std::size_t variant=0;variant<choices.size();++variant){board.play(static_cast<OriginalVideoBoard::Event>(event));require(board.retail_frame(),"Imported event selects retail frames");
   const auto selected=std::find_if(choices.begin(),choices.end(),[&](const Clip& c){return std::equal(board.pixels().begin(),board.pixels().end(),bytes.begin()+c.at);});require(selected!=choices.end(),"Initial frame matches archive");
   const auto c=*selected;for(int f=0;f<c.count;++f){require(std::equal(board.pixels().begin(),board.pixels().end(),bytes.begin()+c.at+std::size_t(f)*10240),"Every decoded frame matches imported pixels");board.advance(.05);}
   auto replay_copy=board;board.advance(.05);replay_copy.advance(.05);require(board.pixels()==replay_copy.pixels(),"Replay copy preserves stream/cursor");
   if(event){for(int i=0;i<410;++i)board.advance(.05);require(board.event()==0 && board.retail_frame(),"Event expires to retail idle");}
  }
 }
 require(std::rand()==expected,"Board variants leave gameplay RNG untouched");
 board.play(OriginalVideoBoard::penalty);require(!board.retail_frame(),"Malformed retail penalty remains original fallback");
 board.use_retail(false);require(board.pixels()==old_idle,"Legacy venues retain original board");
 for(int mode=0;mode<5;++mode){auto bad=bytes;if(mode==0)bad.pop_back();if(mode==1)bad[0]='!';if(mode==2)bad[12]=64;if(mode==3)bad[788]=255;if(mode==4)bad.push_back(0);
  const auto folder=scratch/("bad-"+std::to_string(mode))/"GRAPHICS";std::filesystem::create_directories(folder);
  {std::ofstream f(folder/"EURO_BOARDS.BIN",std::ios::binary);f.write(reinterpret_cast<const char*>(bad.data()),bad.size());}
  bool rejected=false;try{board.load_retail(folder.parent_path());}catch(const std::exception&){rejected=true;}require(rejected,"Malformed pack rejected transactionally");
 }
 require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);require(renderer,SDL_GetError());
 {
  OriginalGraphics g(argv[1]);g.load_cd_graphics(media);g.menu_ui=std::make_shared<OriginalMenuArt>(Archive(std::filesystem::path(argv[1])/"EURO.DAT",std::filesystem::path(argv[1])/"EURO.OFF"));g.action_view=true;auto& s=*g.sequence;auto& e=s.engine;s.start_match(true);s.ticks=10;g.exits_enabled=false;
  board.use_retail(true);board.observe(s);++e.card_events;e.card_player=2;e.teams[1].tm_book=1;board.observe(s);require(board.event()==OriginalVideoBoard::yellow_card,"Booking drives yellow card artwork");
  ++e.card_events;e.teams[1].tm_book=5;board.observe(s);require(board.event()==OriginalVideoBoard::red_card,"Send-off drives red card artwork");
  s.ticks=0;++s.generation;board.observe(s);require(board.event()==0,"New match clears previous card");
  s.ticks=10;
  const auto period=[&](int minute,int expected,bool retail){
   s.interval_ticks=0;board.observe(s);s.interval_ticks=40;s.interval_minute=minute;board.observe(s);
   require(board.event()==expected && board.retail_frame()==retail,"Period selects the correct board and asset family");
   board.advance(.1);const auto revision=board.revision();board.observe(s);
   require(board.revision()==revision,"Repeated interval observations never restart the animation");
  };
  for(bool retail:{true,false}){
   board.use_retail(retail);s.full_time=false;
   const auto seed=e.seed;const auto ticks=s.ticks;
   period(45,OriginalVideoBoard::half_time,retail);
   period(90,OriginalVideoBoard::extra_time,false);
   period(105,OriginalVideoBoard::half_time,retail);
   s.full_time=true;board.observe(s);require(board.event()==OriginalVideoBoard::full_time && !board.retail_frame(),"Final whistle retains verified source fallback");
   require(e.seed==seed && s.ticks==ticks,"Presentation period routing never mutates gameplay");
   s.full_time=false;s.interval_ticks=0;board.observe(s);
  }
  board.use_retail(true);
  for(int stage=0;stage<3;++stage){g.draw(renderer,{48,4,864,648});draw_menu_power(renderer,g,stage*.5f);draw_menu_hint(renderer,g,"P HELP    F6 PANELS    TAB RADAR    V CAMERA");SDL_RenderPresent(renderer);require(SDL_SaveBMP(surface,(scratch/("power-"+std::to_string(stage)+".bmp")).string().c_str()),SDL_GetError());}
 }
 SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();
 std::cout<<"Retail board frames, variants, expiry, replay copying, cards, RNG isolation, malformed packs and power/help captures passed.\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
