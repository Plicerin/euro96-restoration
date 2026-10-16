"""Losslessly pack the fully decoded original CD making-of movie for SDL playback.

FFmpeg is an import-time dependency only. No lossy conversion, dropped frames,
replacement audio or runtime external player. Reject any decoder error.
"""
from pathlib import Path
import argparse, hashlib, io, json, struct, subprocess, tempfile
from PIL import Image

def run(*args):
    return subprocess.run(args, capture_output=True, check=True).stdout

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('iso',type=Path)
    args=parser.parse_args()
    root=Path(__file__).resolve().parents[1]
    original=run('7z','e','-so',str(args.iso.resolve(strict=True)),'VIDEO/VWHOLE_S.GDV')
    target=root/'local-data/VIDEO/MAKING.AMV';target.parent.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='actua-movie-import-') as temp:
        source=Path(temp)/'VWHOLE_S.GDV';source.write_bytes(original)
        probe=json.loads(run('ffprobe','-v','error','-show_streams','-of','json',str(source)))
        video,audio=probe['streams']
        assert (video['codec_name'],video['width'],video['height'],video['r_frame_rate'],int(video['nb_frames']))==('gdv',320,240,'12/1',2869)
        assert (audio['codec_name'],audio['sample_rate'],audio['channels'])==('pcm_u8','22050',1)
        pcm=run('ffmpeg','-v','error','-xerror','-i',str(source),'-map','0:a','-f','u8','-acodec','pcm_u8','pipe:1')
        assert len(pcm)==2869*(22050//12)
        pending=target.with_suffix('.tmp')
        offsets=[];samples={};rawhash=hashlib.sha256()
        with open(Path(temp)/'decode.log','wb+') as errors, open(pending,'wb') as out:
            out.write(struct.pack('<8s7I',b'ACTMOV1\0',320,240,12,1,2869,22050,len(pcm)))
            out.write(bytes(8*2870));out.write(pcm)
            proc=subprocess.Popen(['ffmpeg','-v','error','-xerror','-i',str(source),'-map','0:v','-an','-fps_mode','passthrough','-f','rawvideo','-pix_fmt','pal8','pipe:1'],stdout=subprocess.PIPE,stderr=errors)
            try:
                for frame in range(2869):
                    raw=proc.stdout.read(320*240+1024)
                    if len(raw)!=320*240+1024:raise ValueError(f'Truncated decoded frame {frame}')
                    rawhash.update(raw)
                    image=Image.frombytes('P',(320,240),raw[:320*240])
                    palette=raw[320*240:]
                    image.putpalette(bytes(c for n in range(256) for c in palette[n*4:n*4+3][::-1]))
                    encoded=io.BytesIO();image.save(encoded,format='PNG',compress_level=3)
                    offsets.append(out.tell());out.write(encoded.getvalue())
                    if frame in (0,144,840,2160,2868):
                        rgb=image.convert('RGBA').tobytes();h=1469598103934665603
                        for b in rgb:h=((h^b)*1099511628211)&((1<<64)-1)
                        samples[str(frame)]={'rgba_fnv64':str(h),'png_sha256':hashlib.sha256(encoded.getvalue()).hexdigest()}
                if proc.stdout.read(1):raise ValueError('Unexpected extra decoded frame')
                if proc.wait()!=0:raise ValueError('GDV decoder failed')
                errors.seek(0)
                if errors.read().strip():raise ValueError('GDV decoder emitted errors')
            finally:
                proc.stdout.close()
                if proc.poll() is None:proc.kill();proc.wait()
            offsets.append(out.tell());out.seek(36);out.write(struct.pack('<2870Q',*offsets))
        pending.replace(target)
    report=dict(original='VIDEO/VWHOLE_S.GDV',source_sha256=hashlib.sha256(original).hexdigest(),
        path='VIDEO/MAKING.AMV',sha256=hashlib.sha256(target.read_bytes()).hexdigest(),size=target.stat().st_size,
        width=320,height=240,frames=2869,fps=12,pcm_rate=22050,pcm_bytes=len(pcm),pcm_sha256=hashlib.sha256(pcm).hexdigest(),
        decoded_pal8_sha256=rawhash.hexdigest(),samples=samples,
        decoder=run('ffmpeg','-version').decode().splitlines()[0],
        limits='This importer handles VWHOLE_S only. Use import_cd_intro.py for VFULL_S early-end normalization. High-colour S variants remain unsupported.')
    (root/'docs/cd-movie-recovery.json').write_text(json.dumps(report,indent=2)+'\n')
    print(f'Imported {report["frames"]} lossless original frames and {len(pcm)} audio bytes; {report["size"]} bytes packed.')

if __name__=='__main__':main()
