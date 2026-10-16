#pragma once
#include "ball_original.hpp"
#include <array>
#include <cstdint>
namespace actua {
struct BallPoint { float x,y,z; };
// Native test controls. Launch presets are diagnostic inputs, not restored kick animations.
class BallSandbox {
public:
    original::BallEngine engine;
    float aim=0, power=14;
    int preset=1, curve=0;
    bool paused=false, outside=false;
    std::uint64_t ticks=0;
    std::array<BallPoint,240> trail{};
    std::size_t trail_size=0;
    BallSandbox();
    void reset();
    void launch();
    void step();
    void aim_at(float x,float y);
    void turn(float degrees);
    void change_power(float amount);
    void set_aftertouch(float x,float y);
    void toggle_wind();
    const char* preset_name() const;
};
// Fixed 20 Hz simulation, bounded catch-up after a stall; no step while inactive.
class FixedStep {
    double remainder=0;
public:
    int advance(double seconds,bool active);
    void reset() { remainder=0; }
};
}
