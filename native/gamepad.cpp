#include "gamepad.hpp"
#include "player.hpp"
#include <algorithm>
#include <cmath>
namespace actua {
GamepadInput::GamepadInput(bool virtual_only):virtual_only(virtual_only) {
    int count=0;if(auto ids=SDL_GetGamepads(&count)) {
        for(int i=0;i<count;++i)open(ids[i]);SDL_free(ids);
    }
}
GamepadInput::~GamepadInput(){close();}
void GamepadInput::close(){assigned={};local=false;for(auto p:pads)SDL_CloseGamepad(p);pads.clear();}
void GamepadInput::open(SDL_JoystickID id) {
    if(virtual_only && !SDL_IsJoystickVirtual(id))return;
    if(std::none_of(pads.begin(),pads.end(),[id](auto p){return SDL_GetGamepadID(p)==id;}))
        if(auto p=SDL_OpenGamepad(id))pads.push_back(p);
}
SDL_JoystickID GamepadInput::active_id() const {return pads.empty()?0:SDL_GetGamepadID(pads.front());}
void GamepadInput::activate(SDL_JoystickID id) {
    if(local)return;
    auto found=std::find_if(pads.begin(),pads.end(),[id](auto p){return SDL_GetGamepadID(p)==id;});
    if(found!=pads.end())std::rotate(pads.begin(),found,found+1);
}
PadEvent GamepadInput::event(const SDL_Event& e,bool focused) {
    if(e.type==SDL_EVENT_GAMEPAD_ADDED){open(e.gdevice.which);return PadEvent::none;}
    if(e.type==SDL_EVENT_GAMEPAD_REMOVED){
        const bool active=local?player_for(e.gdevice.which)>=0:e.gdevice.which==active_id();
        for(auto& id:assigned)if(id==e.gdevice.which)id=0;
        std::erase_if(pads,[&](auto p){if(SDL_GetGamepadID(p)!=e.gdevice.which)return false;SDL_CloseGamepad(p);return true;});
        return active?PadEvent::disconnected:PadEvent::none;
    }
    if(!focused)return PadEvent::none;
    if(local){
        if(e.type==SDL_EVENT_GAMEPAD_BUTTON_DOWN){
            if(player_for(e.gbutton.which)>=0)return PadEvent::activity;
            if(e.gbutton.button==SDL_GAMEPAD_BUTTON_START){
                const auto found=std::find_if(pads.begin(),pads.end(),[&](auto p){return SDL_GetGamepadID(p)==e.gbutton.which;});
                if(found!=pads.end())for(auto& id:assigned)if(!id){id=e.gbutton.which;return PadEvent::joined;}
            }
        }
        return PadEvent::none;
    }
    if(e.type==SDL_EVENT_GAMEPAD_BUTTON_DOWN){activate(e.gbutton.which);return e.gbutton.which==active_id()?PadEvent::activity:PadEvent::none;}
    if(e.type==SDL_EVENT_GAMEPAD_AXIS_MOTION && e.gaxis.axis<=SDL_GAMEPAD_AXIS_RIGHTY && std::abs(int(e.gaxis.value))>8000){
        activate(e.gaxis.which);return e.gaxis.which==active_id()?PadEvent::activity:PadEvent::none;
    }
    return PadEvent::none;
}
void GamepadInput::set_local_match(bool enabled){
    if(local==enabled)return;
    local=enabled;assigned={};
    if(enabled)for(int side=0;side<std::min(2,int(pads.size()));++side)assigned[side]=SDL_GetGamepadID(pads[side]);
}
int GamepadInput::player_for(SDL_JoystickID id)const{
    if(!id)return -1;
    for(int side=0;side<2;++side)if(assigned[side]==id)return side;
    return -1;
}
SDL_Gamepad* GamepadInput::player_pad(int side)const{
    const auto id=assigned.at(side);
    for(auto p:pads)if(SDL_GetGamepadID(p)==id)return p;
    return nullptr;
}
bool GamepadInput::player_held(int side,SDL_GamepadButton b)const{
    const auto p=player_pad(side);return p && SDL_GetGamepadButton(p,b);
}
PadVector GamepadInput::player_movement(int side)const{
    const auto p=player_pad(side);if(!p)return {};
    const float x=float(SDL_GetGamepadButton(p,SDL_GAMEPAD_BUTTON_DPAD_RIGHT))-float(SDL_GetGamepadButton(p,SDL_GAMEPAD_BUTTON_DPAD_LEFT));
    const float y=float(SDL_GetGamepadButton(p,SDL_GAMEPAD_BUTTON_DPAD_DOWN))-float(SDL_GetGamepadButton(p,SDL_GAMEPAD_BUTTON_DPAD_UP));
    if(x || y){const auto d=std::hypot(x,y);return {x/d,y/d};}
    return stick(SDL_GetGamepadAxis(p,SDL_GAMEPAD_AXIS_LEFTX),SDL_GetGamepadAxis(p,SDL_GAMEPAD_AXIS_LEFTY));
}
std::string GamepadInput::local_status()const{
    if(local_ready())return "P1 HOME   /   P2 AWAY";
    return std::string(!assigned[0]?"P1":"P2")+" DISCONNECTED - START TO JOIN, THEN START TO RESUME";
}
PadVector GamepadInput::stick(Sint16 x,Sint16 y) {
    const float ax=float(x)/(x<0?32768.0f:32767.0f),ay=float(y)/(y<0?32768.0f:32767.0f);
    const float length=std::hypot(ax,ay);constexpr float deadzone=0.24f;
    if(length<=deadzone)return {};
    const float strength=(std::min(length,1.0f)-deadzone)/(1-deadzone);
    return {ax/length*strength,ay/length*strength};
}
PadVector GamepadInput::axes(SDL_GamepadAxis x,SDL_GamepadAxis y) const {
    return pads.empty()?PadVector{}:stick(SDL_GetGamepadAxis(pads.front(),x),SDL_GetGamepadAxis(pads.front(),y));
}
bool GamepadInput::held(SDL_GamepadButton b) const{return !pads.empty() && SDL_GetGamepadButton(pads.front(),b);}
PadVector GamepadInput::movement() const {
    const float x=float(held(SDL_GAMEPAD_BUTTON_DPAD_RIGHT))-float(held(SDL_GAMEPAD_BUTTON_DPAD_LEFT));
    const float y=float(held(SDL_GAMEPAD_BUTTON_DPAD_DOWN))-float(held(SDL_GAMEPAD_BUTTON_DPAD_UP));
    if(x || y){const float d=std::hypot(x,y);return {x/d,y/d};}
    return axes(SDL_GAMEPAD_AXIS_LEFTX,SDL_GAMEPAD_AXIS_LEFTY);
}
PadVector GamepadInput::aim() const{return axes(SDL_GAMEPAD_AXIS_RIGHTX,SDL_GAMEPAD_AXIS_RIGHTY);}
void GamepadInput::aim_player(PlayerPractice& practice) const {
    const auto v=aim();if(v.x || v.y){const auto& p=practice.engine.teams[std::size_t(practice.selected)];practice.aim_at(p.tm_x+v.x*256,p.tm_y+v.y*256);}
}
bool GamepadInput::practice_button(SDL_GamepadButton b,PlayerPractice& p) const {
    switch(b){
        case SDL_GAMEPAD_BUTTON_SOUTH:p.pass();return true;
        case SDL_GAMEPAD_BUTTON_NORTH:p.pass(true);return true;
        case SDL_GAMEPAD_BUTTON_WEST:aim_player(p);p.action();return true;
        case SDL_GAMEPAD_BUTTON_EAST:
        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:p.switch_player();return true;
        case SDL_GAMEPAD_BUTTON_START:if(p.match_finished)p.reset();else p.toggle_pause();return true;
        case SDL_GAMEPAD_BUTTON_BACK:p.set_opponents(!p.opponents);return true;
        case SDL_GAMEPAD_BUTTON_RIGHT_STICK:if(p.paused || p.match_finished)p.reset();return true;
        default:return false;
    }
}
std::string GamepadInput::label(SDL_GamepadButton button) const {
    const auto type=pads.empty()?SDL_GAMEPAD_TYPE_XBOXONE:SDL_GetGamepadType(pads.front());
    switch(SDL_GetGamepadButtonLabelForType(type,button)){
        case SDL_GAMEPAD_BUTTON_LABEL_A:return "A";case SDL_GAMEPAD_BUTTON_LABEL_B:return "B";
        case SDL_GAMEPAD_BUTTON_LABEL_X:return "X";case SDL_GAMEPAD_BUTTON_LABEL_Y:return "Y";
        case SDL_GAMEPAD_BUTTON_LABEL_CROSS:return "CROSS";case SDL_GAMEPAD_BUTTON_LABEL_CIRCLE:return "CIRCLE";
        case SDL_GAMEPAD_BUTTON_LABEL_SQUARE:return "SQUARE";case SDL_GAMEPAD_BUTTON_LABEL_TRIANGLE:return "TRIANGLE";
        default:return "BUTTON";
    }
}
std::string GamepadInput::name() const {
    if(pads.empty())return "No controller";
    const char* name=SDL_GetGamepadName(pads.front());std::string result=name?name:"Gamepad";
    for(char& c:result)if(static_cast<unsigned char>(c)<32 || static_cast<unsigned char>(c)>126)c='?';
    if(result.size()>29)result.resize(29);return result;
}
std::string GamepadInput::hint(std::string text) const {
    const auto replace=[&](const std::string& from,const std::string& to){
        std::size_t pos=0;while((pos=text.find(from,pos))!=std::string::npos){text.replace(pos,from.size(),to);pos+=to.size();}
    };
    replace("Click toward goal","Aim with right stick");replace(" X", " "+label(SDL_GAMEPAD_BUTTON_WEST));
    replace("Space",label(SDL_GAMEPAD_BUTTON_SOUTH));replace("E switches",label(SDL_GAMEPAD_BUTTON_EAST)+" switches");
    replace("R to reset","pause, then R3 to reset");return text;
}
}
