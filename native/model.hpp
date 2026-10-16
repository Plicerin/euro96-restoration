#pragma once
namespace actua {
struct Navigation {
    int page=0, screen=0, formation=5, zone=0;
    static int wrap(int value, int count) { return (value%count+count)%count; }
    void move(int delta) {
        if(page==0) screen=wrap(screen+delta,15);
        else if(page==1) zone=wrap(zone+delta,70);
    }
    void change_formation(int delta) { formation=wrap(formation+delta,10); }
    void toggle() { page=wrap(page+1,5); }
};
}

