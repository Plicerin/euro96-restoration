#include "original_frontend.hpp"
#include <iostream>
#include <stdexcept>
#include <cstring>
using namespace actua;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 // Nominal source interrupt cadence must not depend on monitor refresh.
 for(int fps:{30,60,63,144}){
  OriginalMenuFade fade;fade.page(0);fade.finish();fade.page(1);
  for(int frame=0;frame<fps;++frame)fade.advance(1.0/fps);
  require(fade.step()==50 && fade.shade()==0,"Fade completes at every tested display rate");
 }
 {OriginalMenuFade fade;fade.page(0);fade.finish();fade.page(1);
  for(int tick=0;tick<24;++tick)fade.advance(1.0/63);
  require(fade.outgoing() && fade.step()==24,"Outgoing page remains until source tick 25");
  fade.advance(1.0/63);require(!fade.outgoing() && fade.shade()==255,"Source tick 25 is the black handoff");
  for(int tick=0;tick<25;++tick)fade.advance(1.0/63);
  require(fade.step()==50 && fade.shade()==0,"Source tick 50 fully reveals destination");
 }
 require(argc==4,"Expected source, media and captures");require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMEPAD),SDL_GetError());
 SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");
 const std::filesystem::path folder=argv[3];std::filesystem::create_directories(folder);
 auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* r=SDL_CreateSoftwareRenderer(surface);require(r,SDL_GetError());
 {
  OriginalGraphics g(argv[1]);OriginalFrontend front(argv[1],argv[2]);auto& s=*g.sequence;using Stage=OriginalFrontend::Stage;
  const auto key=[&](SDL_Keycode key,bool repeat=false){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=key;e.key.repeat=repeat;return front.event(e,g);};
  const auto frame=[&](const std::string& name=""){
   front.draw(r,g,false,false);auto* raw=SDL_RenderReadPixels(r,nullptr);require(raw,SDL_GetError());auto* rgba=SDL_ConvertSurface(raw,SDL_PIXELFORMAT_RGBA32);require(rgba,SDL_GetError());
   Bytes bytes;bytes.reserve(rgba->w*rgba->h*3);
   for(int y=0;y<rgba->h;++y){auto* row=static_cast<Uint8*>(rgba->pixels)+y*rgba->pitch;for(int x=0;x<rgba->w;++x)for(int c=0;c<3;++c)bytes.push_back(row[x*4+c]);}
   if(!name.empty())require(SDL_SavePNG(rgba,(folder/(name+".png")).string().c_str()),SDL_GetError());
   SDL_DestroySurface(rgba);SDL_DestroySurface(raw);return bytes;
  };
  const auto energy=[](const Bytes& b){std::uint64_t result=0;for(auto v:b)result+=v;return result;};
  const auto settle=[&](){for(int i=0;i<9;++i){front.observe(g,.1);frame();}};
  front.open(g);frame();settle();const auto title=frame("title");key(SDLK_RETURN);const auto players=s.engine.teams;const auto seed=s.engine.seed;const auto ticks=s.ticks;
  require(front.stage()==Stage::selection && frame()==title && front.menu_fading_out(),"Outgoing title remains visible after selection becomes the destination");
  front.observe(g,.1);front.observe(g,.1);const auto down=frame("outgoing");require(energy(down)>energy(title)/3 && energy(down)<energy(title)*2/3,"Outgoing menu fades down as one image");
  key(SDLK_RETURN,true);require(frame()==down && front.stage()==Stage::selection,"Held-key repeats cannot accept a hidden destination");
  require(!key(SDLK_F11) && frame()==down,"Display shortcuts pass through without skipping transition");
  SDL_Event focus{};focus.type=SDL_EVENT_WINDOW_FOCUS_LOST;front.event(focus,g);front.observe(g,0);require(frame()==down,"Focus loss holds outgoing frame");
  focus.type=SDL_EVENT_WINDOW_FOCUS_GAINED;front.event(focus,g);front.observe(g,0);require(frame()==down,"No hidden elapsed time is added on focus return");
  front.observe(g,.1);front.observe(g,25.0/63-.3);
  require(energy(frame("black"))==0 && !front.menu_fading_out(),"Both pages meet at a fully black boundary after 25 ticks");
  front.observe(g,.1);front.observe(g,.1);const auto up=frame("incoming");settle();const auto selection=frame("selection");
  require(selection!=title && energy(up)>energy(selection)/3 && energy(up)<energy(selection)*2/3,"Destination fades up after outgoing page disappears");
  front.menu_fades=false;require(frame()==selection,"Completed composition exactly matches direct menu rendering");front.menu_fades=true;frame();settle();
  require(s.engine.seed==seed && s.ticks==ticks && !std::memcmp(players.data(),s.engine.teams.data(),sizeof(players)),"Screen transitions do not advance live gameplay");
  key(SDLK_Y);frame();require(front.menu_fading_out() && front.stage()==Stage::lineup,"Lineup begins with retained setup page");
  key(SDLK_RETURN);key(SDLK_RETURN);require(front.stage()==Stage::lineup && !s.full_match,"Queued confirms reveal lineup without starting unseen match before it is drawn");frame();
  key(SDLK_RETURN);require(front.stage()==Stage::hidden && s.full_match,"Second deliberate confirmation starts match");frame();
  std::cout<<"Outgoing/black/incoming pixels, focus, exact settled rendering, keyboard repeat and confirm protection passed.\n";

  // Actual SDL virtual-controller events exercise the same frontend mapping.
  SDL_VirtualJoystickDesc desc{};SDL_INIT_INTERFACE(&desc);desc.type=SDL_JOYSTICK_TYPE_GAMEPAD;desc.naxes=6;desc.nbuttons=15;desc.name="Menu transition contract pad";
  const auto id=SDL_AttachVirtualJoystick(&desc);require(id!=0,SDL_GetError());auto* joystick=SDL_OpenJoystick(id);auto* pad=SDL_OpenGamepad(id);require(joystick && pad,SDL_GetError());
  const auto pump=[&](){SDL_UpdateJoysticks();SDL_Event event;while(SDL_PollEvent(&event))if((event.type==SDL_EVENT_GAMEPAD_BUTTON_DOWN || event.type==SDL_EVENT_GAMEPAD_BUTTON_UP) && event.gbutton.which==id)front.event(event,g);};
  const auto button=[&](SDL_GamepadButton b,bool held){require(SDL_SetJoystickVirtualButton(joystick,b,held),SDL_GetError());pump();};
  pump();front.setup(g);frame();settle();button(SDL_GAMEPAD_BUTTON_NORTH,true);button(SDL_GAMEPAD_BUTTON_NORTH,false);frame();const auto generation=s.generation;
  require(front.menu_fading_out() && front.stage()==Stage::lineup,"Controller enters lineup transition");
  button(SDL_GAMEPAD_BUTTON_SOUTH,true);require(front.stage()==Stage::lineup && s.generation==generation,"Controller confirm reveals without activating unseen kickoff");frame();
  button(SDL_GAMEPAD_BUTTON_SOUTH,false);button(SDL_GAMEPAD_BUTTON_SOUTH,true);require(front.stage()==Stage::hidden && s.generation!=generation,"Fresh controller confirm starts match");button(SDL_GAMEPAD_BUTTON_SOUTH,false);frame();
  SDL_CloseGamepad(pad);SDL_CloseJoystick(joystick);SDL_DetachVirtualJoystick(id);
  std::cout<<"Virtual controller navigation and release/press confirmation passed.\n";

  front.setup(g);frame();settle();const auto click=[&](float x,float y){SDL_Event e{};e.type=SDL_EVENT_MOUSE_BUTTON_DOWN;e.button.button=SDL_BUTTON_LEFT;e.button.x=48+x*1.35f;e.button.y=8+y*1.35f;front.event(e,g);};
  const auto click_generation=s.generation;click(300,435);require(front.menu_fading_out() && front.stage()==Stage::lineup,"Pending pointer transition is guarded before drawing");
  click(300,435);click(300,435);require(s.generation==click_generation && front.stage()==Stage::lineup,"Queued pointer clicks cannot activate unseen kickoff");frame();
  click(300,435);require(s.generation!=click_generation && front.stage()==Stage::hidden,"Deliberate second click starts match");frame();
  front.setup(g);frame();settle();key(SDLK_O);frame();require(front.menu_fading_out(),"Options transition active");
  SDL_Event reset{};reset.type=SDL_EVENT_RENDER_TARGETS_RESET;front.event(reset,g);require(energy(frame())==0 && !front.menu_fading_out(),"Lost target content restarts safely from current page");settle();
  key(SDLK_ESCAPE);frame();key(SDLK_RIGHT);frame(); // Reveal setup without changing a hidden selection.
  std::cout<<"Pointer confirm protection and render-target reset recovery passed.\n";

  // The composed menu must behave identically inside a scaled render target.
  auto* outer=SDL_CreateTexture(r,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_TARGET,1280,720);require(outer,SDL_GetError());require(SDL_SetRenderTarget(r,outer),SDL_GetError());
  require(SDL_SetRenderLogicalPresentation(r,960,664,SDL_LOGICAL_PRESENTATION_LETTERBOX),SDL_GetError());
  front.menu_fades=true;frame();settle();const auto composed=frame("scaled");front.menu_fades=false;require(frame()==composed,"Scaled/letterboxed composed menu matches direct rendering at the same animation time");
  require(SDL_GetRenderTarget(r)==outer,"Frontend restores caller's render target");int w=0,h=0;SDL_RendererLogicalPresentation mode{};require(SDL_GetRenderLogicalPresentation(r,&w,&h,&mode),SDL_GetError());require(w==960 && h==664 && mode==SDL_LOGICAL_PRESENTATION_LETTERBOX,"Frontend preserves logical presentation");
  require(SDL_SetRenderTarget(r,nullptr),SDL_GetError());SDL_DestroyTexture(outer);
  std::cout<<"Scaled composition and caller render-state preservation passed.\n";
 }
 SDL_DestroyRenderer(r);SDL_DestroySurface(surface);SDL_Quit();return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
