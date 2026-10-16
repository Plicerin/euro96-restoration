"""Bounded retail FAP decoding: executable routines at 0x9e60c/0x9e73f.

The first frame uses literal runs followed by repeats of the last literal.
Later frames prefix 1 for literal/skip deltas or 2 for literal/repeat frames.
The file starts with 256 little-endian frame lengths and ends with zero.
"""
import struct


def decode(data, width, height):
    if (width,height) not in ((140,285),(320,240)) or len(data)<1026:
        raise ValueError('Invalid FAP dimensions/header')
    sizes=struct.unpack_from('<256I',data)
    count=next((i for i,n in enumerate(sizes) if not n),256)
    if not 1<=count<256 or any(sizes[count:]) or sum(sizes)+1025!=len(data) or data[-1]:
        raise ValueError('Invalid FAP frame table/terminator')
    pixels=bytearray(width*height);at=1024;frames=[]
    for index,size in enumerate(sizes[:count]):
        frame=data[at:at+size];at+=size;cursor=0;pos=0;kind=2
        if index:
            kind=frame[0];cursor=1
            if kind not in (1,2):raise ValueError('Unsupported FAP frame type')
        while True:
            if cursor>=len(frame):raise ValueError('Truncated FAP run')
            run=frame[cursor];cursor+=1
            if not run:break
            if run>len(frame)-cursor or run>len(pixels)-pos:raise ValueError('FAP literal outside frame')
            pixels[pos:pos+run]=frame[cursor:cursor+run];pos+=run;cursor+=run
            if cursor>=len(frame):raise ValueError('Truncated FAP repeat/skip')
            skip=frame[cursor];cursor+=1
            if skip>len(pixels)-pos:raise ValueError('FAP repeat/skip outside frame')
            if kind==2:pixels[pos:pos+skip]=bytes([pixels[pos-1]])*skip
            pos+=skip
        if cursor!=len(frame):raise ValueError('Trailing FAP frame data')
        if not index and pos!=len(pixels):raise ValueError('Incomplete first FAP frame')
        frames.append(bytes(pixels))
    return frames
