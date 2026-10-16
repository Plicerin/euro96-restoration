#include "gamepad.hpp"
#include "player.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace actua;
static int checks=0;
static void expect(bool v,const char* m){++checks;if(!v)throw std::runtime_error(std::string(m)+": "+SDL_GetError());}
static SDL_JoystickID attach(const char* name,Uint16 vendor,Uint16 product){
    SDL_VirtualJoystickDesc desc{};SDL_INIT_INTERFACE(&desc);desc.type=SDL_JOYSTICK_TYPE_GAMEPAD;
    desc.naxes=SDL_GAMEPAD_AXIS_COUNT;desc.nbuttons=SDL_GAMEPAD_BUTTON_COUNT;
    desc.axis_mask=(1u<<SDL_GAMEPAD_AXIS_COUNT)-1;desc.button_mask=(1u<<SDL_GAMEPAD_BUTTON_COUNT)-1;
    desc.name=name;desc.vendor_id=vendor;desc.product_id=product;
    const auto id=SDL_AttachVirtualJoystick(&desc);expect(id!=0,"Attach SDL virtual controller");return id;
}
static bool pump(GamepadInput& pad,PlayerPractice* p=nullptr){
    SDL_UpdateJoysticks();bool disconnected=false;SDL_Event e;
    while(SDL_PollEvent(&e)){
        const auto result=pad.event(e);if(result==PadEvent::disconnected)disconnected=true;
        if(p && e.type==SDL_EVENT_GAMEPAD_BUTTON_DOWN && e.gbutton.which==pad.active_id())
            pad.practice_button(static_cast<SDL_GamepadButton>(e.gbutton.button),*p);
    }
    return disconnected;
}
static void press(GamepadInput& pad,SDL_Joystick* device,SDL_GamepadButton b,PlayerPractice& p){
    expect(SDL_SetJoystickVirtualButton(device,b,true),"Set virtual button");pump(pad,&p);
    expect(SDL_SetJoystickVirtualButton(device,b,false),"Release virtual button");pump(pad,&p);
}
int main(){try{
    expect(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMEPAD),"Initialize SDL input");
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");
    const auto id=attach("Actua virtual Xbox controller",0x045e,0x028e);
    SDL_Joystick* device=SDL_OpenJoystick(id);expect(device!=nullptr,"Open virtual device");
    {
        GamepadInput pad;pump(pad);expect(pad.connected(),"Controller discovered at startup");
        PlayerPractice p;
        SDL_SetJoystickVirtualAxis(device,SDL_GAMEPAD_AXIS_LEFTX,4000);pump(pad);
        expect(pad.movement().x==0,"Stick drift inside deadzone does not move player");
        SDL_SetJoystickVirtualAxis(device,SDL_GAMEPAD_AXIS_LEFTX,32767);pump(pad);
        auto v=pad.movement();expect(v.x>0.999f && v.y==0,"Full stick produces full movement");
        const float start=p.engine.teams[1].tm_x;p.set_input(v.x,v.y);p.step();const float full=p.engine.teams[1].tm_x-start;
        p.reset();SDL_SetJoystickVirtualAxis(device,SDL_GAMEPAD_AXIS_LEFTX,16384);pump(pad);v=pad.movement();
        p.set_input(v.x,v.y);p.step();expect(p.engine.teams[1].tm_x-500>0 && p.engine.teams[1].tm_x-500<full*0.5f,"Partial stick gives controlled slower movement");
        SDL_SetJoystickVirtualAxis(device,SDL_GAMEPAD_AXIS_LEFTX,-32768);SDL_SetJoystickVirtualAxis(device,SDL_GAMEPAD_AXIS_LEFTY,-32768);pump(pad);v=pad.movement();
        expect(std::hypot(v.x,v.y)<=1.00001f && v.x<0 && v.y<0,"Diagonal and negative extremes are bounded");
        SDL_SetJoystickVirtualAxis(device,SDL_GAMEPAD_AXIS_LEFTX,0);SDL_SetJoystickVirtualAxis(device,SDL_GAMEPAD_AXIS_LEFTY,0);
        SDL_SetJoystickVirtualButton(device,SDL_GAMEPAD_BUTTON_DPAD_RIGHT,true);pump(pad);expect(pad.movement().x==1,"D-pad movement works");
        SDL_SetJoystickVirtualButton(device,SDL_GAMEPAD_BUTTON_DPAD_RIGHT,false);pump(pad);expect(pad.movement().x==0,"D-pad release clears movement");
        p.reset();press(pad,device,SDL_GAMEPAD_BUTTON_SOUTH,p);expect(p.selected==2 && !p.engine.ball_poss,"Controller pass releases ball and selects receiver");
        for(int i=0;i<60 && !p.engine.ball_poss;++i)p.step();expect(p.engine.ball_poss==3,"Controller pass can be received");
        // A held analog stick must flow through ground and lofted reception.
        for(auto button:{SDL_GAMEPAD_BUTTON_SOUTH,SDL_GAMEPAD_BUTTON_NORTH}) {
            p.reset();SDL_SetJoystickVirtualAxis(device,SDL_GAMEPAD_AXIS_LEFTX,16384);pump(pad);
            v=pad.movement();p.set_input(v.x,v.y);p.step();press(pad,device,button,p);
            for(int i=0;i<160 && !p.engine.ball_poss;++i){pump(pad);v=pad.movement();p.set_input(v.x,v.y);p.step();}
            expect(p.engine.ball_poss==3,"Held analog input allows pass reception");
            const auto before=p.engine.teams[2];
            PlayerPractice manual=p;manual.set_input(0,0);manual.set_input(v.x,v.y);manual.step();
            p.step();const auto& after=p.engine.teams[2];const auto& expected=manual.engine.teams[2];
            expect(std::hypot(after.tm_x-before.tm_x,after.tm_y-before.tm_y)>0.01f &&
                std::hypot(after.tm_x-expected.tm_x,after.tm_y-expected.tm_y)<0.001f,
                "Held virtual stick moves receiver on first tick after catch");
        }
        SDL_SetJoystickVirtualAxis(device,SDL_GAMEPAD_AXIS_LEFTX,0);pump(pad);
        p.reset();press(pad,device,SDL_GAMEPAD_BUTTON_NORTH,p);expect(p.engine.ball_inair && p.engine.ballzdis>0,"Controller loft button works");
        p.reset();SDL_SetJoystickVirtualAxis(device,SDL_GAMEPAD_AXIS_RIGHTX,0);SDL_SetJoystickVirtualAxis(device,SDL_GAMEPAD_AXIS_RIGHTY,-32768);pump(pad);
        press(pad,device,SDL_GAMEPAD_BUTTON_WEST,p);expect(p.engine.ballydis<-19.9f && std::abs(p.engine.ballxdis)<0.01f,"Right-stick aim applies to the shot event immediately");
        SDL_SetJoystickVirtualAxis(device,SDL_GAMEPAD_AXIS_RIGHTY,0);pump(pad);
        p.reset();press(pad,device,SDL_GAMEPAD_BUTTON_EAST,p);expect(p.selected==2,"Face button switches player");
        press(pad,device,SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,p);expect(p.selected==1,"Shoulder also switches player");
        p.blue_score=2;press(pad,device,SDL_GAMEPAD_BUTTON_RIGHT_STICK,p);expect(p.blue_score==2,"Stick click cannot erase a live game");
        press(pad,device,SDL_GAMEPAD_BUTTON_START,p);expect(p.paused,"Start pauses");
        press(pad,device,SDL_GAMEPAD_BUTTON_RIGHT_STICK,p);expect(!p.paused && p.blue_score==0,"Paused stick-click starts a fresh game");
        press(pad,device,SDL_GAMEPAD_BUTTON_BACK,p);expect(p.opponents,"Back toggles opponent mode");
        p.match_elapsed_ticks=PlayerPractice::match_duration_ticks-1;p.step();
        expect(p.match_finished,"Virtual controller scenario reaches full time");
        press(pad,device,SDL_GAMEPAD_BUTTON_SOUTH,p);expect(p.match_finished,"Pass cannot resume a finished game");
        press(pad,device,SDL_GAMEPAD_BUTTON_START,p);
        expect(!p.match_finished && p.match_seconds_left()==180 && p.opponents,"Start begins a fresh timed rematch");
        for(int i=0;i<200 && p.engine.ball_poss<12;++i)p.step();
        SDL_SetJoystickVirtualButton(device,SDL_GAMEPAD_BUTTON_WEST,true);pump(pad,&p);
        for(int i=0;i<300 && (p.engine.ball_poss==0 || p.engine.ball_poss>=12);++i){
            const auto& blue=p.engine.teams[std::size_t(p.selected)];const float dx=p.engine.ballx-blue.tm_x,dy=p.engine.bally-blue.tm_y,d=std::hypot(dx,dy);
            SDL_SetJoystickVirtualAxis(device,SDL_GAMEPAD_AXIS_LEFTX,static_cast<Sint16>(dx/d*32767));
            SDL_SetJoystickVirtualAxis(device,SDL_GAMEPAD_AXIS_LEFTY,static_cast<Sint16>(dy/d*32767));pump(pad);
            v=pad.movement();p.set_input(v.x,v.y);p.set_tackle_held(pad.held(SDL_GAMEPAD_BUTTON_WEST));p.step();
        }
        expect(p.blue_wins>0,"Held virtual controller tackle recovers possession");
        SDL_SetJoystickVirtualButton(device,SDL_GAMEPAD_BUTTON_WEST,false);pump(pad);
        const auto second=attach("Actua virtual PlayStation controller",0x054c,0x09cc);
        auto* second_device=SDL_OpenJoystick(second);pump(pad);
        SDL_SetJoystickVirtualButton(second_device,SDL_GAMEPAD_BUTTON_EAST,true);pump(pad);
        expect(pad.active_id()==second,"Last controller used owns both buttons and axes");
        expect(pad.label(SDL_GAMEPAD_BUTTON_WEST)=="SQUARE","PlayStation button prompts match the device");
        SDL_CloseJoystick(second_device);SDL_DetachVirtualJoystick(second);
        expect(pump(pad),"Disconnect of active controller is reported for pausing");
        expect(pad.active_id()==id,"Remaining controller remains available");
        SDL_CloseJoystick(device);SDL_DetachVirtualJoystick(id);pump(pad);
        if(!pad.connected())expect(pad.movement().x==0 && !pad.held(SDL_GAMEPAD_BUTTON_WEST),"Disconnect clears stick and tackle input");
        const auto reconnected=attach("Reconnected controller",0x045e,0x028e);pump(pad);
        expect(pad.connected(),"Controller reconnect works without restarting game");SDL_DetachVirtualJoystick(reconnected);pump(pad);
    }
    SDL_Quit();std::cout<<"Passed "<<checks<<" SDL virtual-gamepad checks.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';SDL_Quit();return 1;}}
