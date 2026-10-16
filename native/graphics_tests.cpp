#include "original_graphics.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace actua;
int main(int argc,char** argv){try{
    if(argc!=2)throw std::runtime_error("Data path required");
    if(!SDL_Init(SDL_INIT_VIDEO))throw std::runtime_error(SDL_GetError());
    auto* surface=SDL_CreateSurface(960,664,SDL_PIXELFORMAT_RGBA32);auto* renderer=SDL_CreateSoftwareRenderer(surface);
    if(!renderer)throw std::runtime_error(SDL_GetError());
    std::size_t frames=0;
    {
        OriginalGraphics g(argv[1]);if(g.clips.size()!=95)throw std::runtime_error("Expected all preserved motion captures");
        for(std::size_t c=0;c<g.clips.size();++c){
            g.clip=int(c);
            for(std::size_t f=0;f<g.clips[c].frames.size();++f){g.frame=int(f);g.angle=float(f%8)*0.785398f;g.draw(renderer);++frames;}
        }
        g.clip=40;g.frame=0;g.angle=0.4f;
        for(int k=0;k<16;++k){g.change_kit(k?1:0);g.draw(renderer);if(!g.polygon_count())throw std::runtime_error("Empty textured player");}
        g.overview=true;g.draw(renderer);if(g.polygon_count()<100)throw std::runtime_error("Empty stadium");
        g.overview=false;g.wire=true;g.draw(renderer);if(g.polygon_count()<20)throw std::runtime_error("Empty original wireframe");
        g.paused=true;auto before=g.frame;g.advance(1);if(g.frame!=before)throw std::runtime_error("Animation pause failed");
        g.paused=false;g.advance(.05);if(g.frame==before)throw std::runtime_error("Animation advance failed");
        g.clip=0;g.change_clip(-1);if(g.clip!=94 || g.frame)throw std::runtime_error("Animation wrap failed");
    }
    SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();
    std::cout<<"Rendered "<<frames<<" original motion frames, 16 kits, wireframe and stadium.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';SDL_Quit();return 1;}}
