"""Read the OMF records needed for source restoration (TIS OMF 1.1)."""
from pathlib import Path
import struct

class Reader:
 def __init__(self,b): self.b=b; self.p=0
 def get(self,n):
  if self.p+n>len(self.b): raise ValueError('Truncated OMF record')
  r=self.b[self.p:self.p+n];self.p+=n;return r
 def u(self,n=1): return int.from_bytes(self.get(n),'little')
 def idx(self):
  a=self.u();return ((a&127)<<8 | self.u()) if a&128 else a
 def name(self): return self.get(self.u()).decode('latin1')
 def more(self): return self.p<len(self.b)

class Object:
 def __init__(self,path):
  self.names=[''];self.segments=[None];self.publics=[];self.externs=[''];self.records=[];self.lines=[]
  data=Path(path).read_bytes();p=0
  while p<len(data):
   if p+3>len(data): raise ValueError('Truncated record header')
   t,n=struct.unpack_from('<BH',data,p);end=p+3+n
   if n<1 or end>len(data): raise ValueError('Invalid record length')
   record=data[p:end]
   if record[-1]!=0 and sum(record)&255: raise ValueError('Checksum mismatch')
   b=Reader(record[3:-1]);self.records.append((t,record[3:-1]));size=4 if t&1 else 2
   if t==0x96:
    while b.more(): self.names.append(b.name())
   elif t in (0x98,0x99):
    attr=b.u()
    if not attr>>5: b.get(3)
    length=b.u(size);sn,cn,on=b.idx(),b.idx(),b.idx()
    if length>64*1024*1024: raise ValueError('Unreasonable segment')
    self.segments.append(dict(name=self.names[sn],cls=self.names[cn],length=length,data=bytearray(length),written=bytearray(length)))
   elif t in (0x90,0x91,0xb6,0xb7):
    group,seg=b.idx(),b.idx()
    if seg==0:b.get(2)
    while b.more():
     name=b.name();offset=b.u(size);typ=b.idx()
     self.publics.append(dict(name=name,segment=seg,offset=offset,local=t>=0xb6))
   elif t in (0x8c,0xb4):
    while b.more(): self.externs.append(b.name()); b.idx()
   elif t in (0xa0,0xa1):
    seg=b.idx();offset=b.u(size);body=b.get(len(b.b)-b.p);s=self.segments[seg]
    if offset+len(body)>s['length']:raise ValueError('Data outside segment')
    s['data'][offset:offset+len(body)]=body;s['written'][offset:offset+len(body)]=b'\1'*len(body)
   elif t in (0x94,0x95):
    group,seg=b.idx(),b.idx()
    while b.more():self.lines.append((seg,b.u(2),b.u(size)))
   p=end
 def fixups(self,skip_comdat=False,skip_lidata=False):
  target_threads={};frame_threads={};last=None;skipped=False;out=[]
  for typ,body in self.records:
   b=Reader(body);size=4 if typ&1 else 2
   if typ in (0xa0,0xa1):last=(b.idx(),b.u(size));skipped=False
   elif typ in (0xc2,0xc3) and skip_comdat:last=None;skipped=True
   elif typ in (0xa2,0xa3) and skip_lidata:last=None;skipped=True
   elif typ in (0xa2,0xa3,0xc2,0xc3):raise ValueError('Unsupported data type for fixups')
   elif typ in (0x9c,0x9d):
    while b.more():
     a=b.u()
     if not a&128:
      frame=bool(a&64);method=(a>>2)&7;idx=b.idx() if method<3 else None
      (frame_threads if frame else target_threads)[a&3]=(method,idx)
     else:
      loc=((a&3)<<8)|b.u();kind=(a>>2)&15;fd=b.u()
      if not fd&128:
       fm=(fd>>4)&7
       if fm<3:b.idx()
       elif fm==3:b.get(2)
      if fd&8:tm,ti=target_threads[fd&3]
      else:tm=fd&3;ti=b.idx()
      disp=0 if fd&4 else b.u(size)
      if last is None:
       if skipped:continue
       raise ValueError('Fixup without data')
      out.append(dict(segment=last[0],offset=last[1]+loc,kind=kind,relative=not bool(a&64),target_method=tm,target_index=ti,displacement=disp))
  return out
 def named(self,name):return next(x for x in self.publics if x['name'].split('$')[0] in (name,'W?'+name))
 def value(self,name,fmt):
  s=self.named(name);return struct.unpack_from('<'+fmt,self.segments[s['segment']]['data'],s['offset'])[0]

if __name__=='__main__':
 import sys,json
 obj=Object(sys.argv[1])
 print([(i,s['name'],s['length']) for i,s in enumerate(obj.segments) if s])
 for p in obj.publics:
  if p['segment']!=1:
   s=obj.segments[p['segment']];o=p['offset'];b=s['data'][o:o+8]
   print(p['name'],p['segment'],hex(o),b.hex())
