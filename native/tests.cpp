#include "archive.hpp"
#include "model.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace actua;
static int checks=0;
static void expect(bool value,const char* why) {
    ++checks;
    if(!value) throw std::runtime_error(why);
}
template<class F> static void rejects(F f,const char* why) {
    bool threw=false; try{f();}catch(const std::exception&){threw=true;}
    expect(threw,why);
}
int main(int argc,char** argv) {
    try {
        expect(argc==2,"Asset directory argument required");
        const std::filesystem::path root=argv[1];
        Archive euro(root/"EURO.DAT",root/"EURO.OFF");
        expect(euro.count()==121,"Preserved EURO archive record count");
        for(std::size_t i=0;i<15;++i) {
            auto rgba=decode_screen(euro,i);
            expect(rgba.size()==640*480*4,"Decoded image size");
            expect(rgba[3]==255 && rgba.back()==255,"Opaque palette conversion");
        }
        for(std::size_t i=0;i<10;++i) {
            const auto packed=euro.entry(111+i);
            const std::string formation=formation_names[i];
            std::string digits=formation; std::erase(digits,'-');
            auto loose=read_file(root/("TAC_"+digits+".TAC"));
            expect(std::equal(loose.begin(),loose.end(),packed.begin(),packed.end()),"Embedded and standalone tactics agree");
            const auto table=decode_tactics(packed);
            for(const auto& zone:table) for(auto point:zone)
                expect(point.x>=0 && point.x<=1280 && point.y>=0 && point.y<=800,"Preserved target coordinate bounds");
        }
        Bytes truncated{0,0,0};
        rejects([&]{Archive a({},truncated);},"Reject truncated offset table");
        Bytes overflow{255,255,255,255,255,255,255,255};
        rejects([&]{Archive a(Bytes(8),overflow);},"Reject overflowing archive range");
        Bytes beyond{4,0,0,0,8,0,0,0};
        rejects([&]{Archive a(Bytes(8),beyond);},"Reject out-of-bounds entry");
        Bytes empty_entry{8,0,0,0,0,0,0,0};
        Archive valid_empty(Bytes(8),empty_entry);
        expect(valid_empty.entry(0).empty(),"Allow empty entry at EOF");
        rejects([&]{euro.entry(121);},"Reject out-of-range entry");
        rejects([&]{decode_screen(euro,15);},"Reject out-of-range screen");
        rejects([&]{decode_tactics(Bytes(5599));},"Reject truncated tactics");
        rejects([&]{read_file(root/"not-present.dat");},"Missing data produces useful failure");
        BYTE source[]{0,2,3,0}, destination[]{9,9,9,9};
        spritecopy(4,destination,source);
        expect(destination[0]==9 && destination[1]==2 && destination[2]==3 && destination[3]==9,"Original transparent sprite copy");
        flipsprcopy(4,destination,source);
        expect(destination[1]==3 && destination[2]==2,"Original reversed sprite copy");
        straightcopy(4,destination,source);
        expect(std::equal(std::begin(source),std::end(source),std::begin(destination)),"Original opaque copy");
        BYTE saved[4]{};
        pseudostore(4,destination,saved);
        expect(std::equal(std::begin(saved),std::end(saved),std::begin(destination)),"Original background store");
        expect(std::abs(calc_dist(3,4)-5)<0.0001f,"Original distance routine");
        expect(std::abs(calc_dist(0,0)-0.1f)<0.0001f,"Original minimum distance behavior");
        Navigation n;
        n.move(-1); expect(n.screen==14,"Screen wraps backward");
        n.move(1); expect(n.screen==0,"Screen wraps forward");
        n.toggle(); n.move(-1); expect(n.zone==69,"Zone wraps backward");
        n.formation=0; n.change_formation(-1); expect(n.formation==9,"Formation wraps backward");
        std::cout<<"Passed "<<checks<<" asset and preserved-routine checks.\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr<<"Test failed: "<<e.what()<<"\n"; return 1;
    }
}

