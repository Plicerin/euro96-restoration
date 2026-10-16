"""Losslessly import all 32 Euro96 trophy/credits films with shared PNG/PCM blobs."""
import argparse, hashlib, io, json, struct, subprocess, tempfile
from pathlib import Path
from PIL import Image
from import_cd_movie import run
from import_cd_graphics import ROOT

def recover(iso):
    exe=run('7z','e','-so',str(iso),'EURO96.EXE')
    if hashlib.sha256(exe).hexdigest()!='2a473a002a294c1bb28ea39e94a36afc34497dd7e44f0ebfeda0ea21f652b62d':raise ValueError('Unsupported retail executable')
    base=0x12f854;names=[]
    for offset in struct.unpack_from('<32I',exe,base+0x7e50):names.append(exe[base+offset:].split(b'\0')[0].decode().upper())
    assert names[:2]==['BULHOUTS.GDV','CROHOUTS.GDV'] and names[16:18]==['BULAOUTS.GDV','CROAOUTS.GDV']
    target=ROOT/'local-data/VIDEO/EURO_CUP.BIN';target.parent.mkdir(parents=True,exist_ok=True)
    pending=target.with_suffix('.tmp');rows=[];dedup={};entries=[]
    with tempfile.TemporaryDirectory(prefix='euro96-cup-') as tmp, open(pending,'wb') as out:
        out.write(b'EU96CUP1'+struct.pack('<I',32)+bytes(32*8))
        def blob(data):
            digest=hashlib.sha256(data).digest()
            if digest not in dedup:dedup[digest]=(out.tell(),len(data));out.write(data)
            return dedup[digest]
        for film,name in enumerate(names):
            original=run('7z','e','-so',str(iso),'GDV/'+name);source=Path(tmp)/'film.gdv';source.write_bytes(original)
            probe=json.loads(run('ffprobe','-v','error','-show_streams','-of','json',str(source)))
            video,audio=probe['streams'];frames=int(video['nb_frames'])
            assert (video['codec_name'],video['width'],video['height'],video['r_frame_rate'],frames)==('gdv',320,240,'12/1',983)
            assert (audio['codec_name'],audio['sample_rate'],audio['channels'])==('pcm_u8','22050',2)
            pcm=run('ffmpeg','-v','error','-xerror','-i',str(source),'-map','0:a','-f','u8','-acodec','pcm_u8','pipe:1')
            assert len(pcm)==frames*1837*2
            audio_offset,audio_size=blob(pcm);images=[];samples={};rawhash=hashlib.sha256()
            with open(Path(tmp)/'decode.log','wb+') as errors:
                proc=subprocess.Popen(['ffmpeg','-v','error','-xerror','-i',str(source),'-map','0:v','-an','-fps_mode','passthrough','-f','rawvideo','-pix_fmt','pal8','pipe:1'],stdout=subprocess.PIPE,stderr=errors)
                try:
                    for frame in range(frames):
                        raw=proc.stdout.read(320*240+1024)
                        if len(raw)!=320*240+1024:raise ValueError(f'Truncated film {film} frame {frame}')
                        rawhash.update(raw);im=Image.frombytes('P',(320,240),raw[:320*240]);pal=raw[320*240:]
                        im.putpalette(bytes(c for n in range(256) for c in pal[n*4:n*4+3][::-1]))
                        encoded=io.BytesIO();im.save(encoded,format='PNG',compress_level=3);images.append(blob(encoded.getvalue()))
                        if frame in (0,180,982):
                            h=1469598103934665603
                            for b in im.convert('RGBA').tobytes():h=((h^b)*1099511628211)&((1<<64)-1)
                            samples[str(frame)]=str(h)
                    if proc.stdout.read(1) or proc.wait()!=0:raise ValueError('GDV decode failed')
                    errors.seek(0)
                    if errors.read().strip():raise ValueError('GDV decoder errors')
                finally:
                    proc.stdout.close()
                    if proc.poll() is None:proc.kill();proc.wait()
            entries.append(out.tell());out.write(struct.pack('<IIQ',frames,audio_size,audio_offset))
            for offset,size in images:out.write(struct.pack('<QI',offset,size))
            rows.append(dict(id=film,file='GDV/'+name,sha256=hashlib.sha256(original).hexdigest(),frames=frames,pcm_bytes=audio_size,pcm_sha256=hashlib.sha256(pcm).hexdigest(),decoded_pal8_sha256=rawhash.hexdigest(),samples=samples))
            print(f'{film+1}/32 {name}: {out.tell()//1024//1024} MiB shared pack',flush=True)
        out.seek(12);out.write(struct.pack('<32Q',*entries))
    pending.replace(target)
    (ROOT/'docs/euro96-trophy.json').write_text(json.dumps(dict(films=rows,bytes=target.stat().st_size,sha256=hashlib.sha256(target.read_bytes()).hexdigest(),decoder=run('ffmpeg','-version').decode().splitlines()[0],audio='Original unsigned 8-bit stereo 22050 Hz; no downmix',routing='Retail filename table 0x1376a4; home 0..15, alternate 16..31. Native champion event uses the final match kit clash selection.'),indent=2)+'\n')
    (ROOT/'docs/euro96-trophy-samples.txt').write_text(''.join(f'{r["id"]} {f} {v}\n' for r in rows for f,v in r['samples'].items()))

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('iso',type=Path);recover(p.parse_args().iso.resolve(strict=True))
