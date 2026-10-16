#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include <array>
namespace actua {
class PlayerPractice;
struct PadVector { float x=0,y=0; };
enum class PadEvent { none, activity, disconnected, joined };
class GamepadInput {
public:
    explicit GamepadInput(bool virtual_only=false);
    ~GamepadInput();
    GamepadInput(const GamepadInput&)=delete;
    GamepadInput& operator=(const GamepadInput&)=delete;
    void close();
    PadEvent event(const SDL_Event& event,bool focused=true);
    void set_local_match(bool enabled);
    bool local_match()const{return local;}
    bool local_ready()const{return assigned[0] && assigned[1];}
    int count()const{return int(pads.size());}
    int player_for(SDL_JoystickID id)const;
    SDL_JoystickID player_id(int side)const{return assigned.at(side);}
    PadVector player_movement(int side)const;
    bool player_held(int side,SDL_GamepadButton button)const;
    std::string local_status()const;
    bool connected() const {return !pads.empty();}
    SDL_JoystickID active_id() const;
    PadVector movement() const;
    PadVector aim() const;
    bool held(SDL_GamepadButton button) const;
    void aim_player(PlayerPractice& practice) const;
    bool practice_button(SDL_GamepadButton button,PlayerPractice& practice) const;
    std::string label(SDL_GamepadButton button) const;
    std::string name() const;
    std::string hint(std::string text) const;
    static PadVector stick(Sint16 x,Sint16 y);
private:
    std::vector<SDL_Gamepad*> pads;
    bool virtual_only=false;
    bool local=false;
    std::array<SDL_JoystickID,2> assigned{};
    SDL_Gamepad* player_pad(int side)const;
    void open(SDL_JoystickID id);
    void activate(SDL_JoystickID id);
    PadVector axes(SDL_GamepadAxis x,SDL_GamepadAxis y) const;
};
}
