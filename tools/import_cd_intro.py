"""Recover VFULL_S.GDV without dropping its valid early-ended delta frames.

Method 2's end tag retains the unpainted tail. FFmpeg rejects that tail.
Expand only those end tags to explicit unchanged-pixel runs in a temporary
GDV, then demand exact agreement between FFmpeg and a bounded format reader.
The ISO and extracted original are never modified. No decoder ships at runtime.
Format research: https://multimedia.cx/gdv-format.txt (method 2 end tag).
"""
from pathlib import Path
import argparse, hashlib, io, json, struct, subprocess, tempfile
from PIL import Image

def require(condition, message):
    if not condition:
        raise ValueError(message)

def method2(data, pixels):
    """Return equivalent full-span commands and decoded pixels, preserving LZ overlap."""
    memory = bytearray(bytes(c for c in range(256) for _ in range(16)) + pixels)
    cursor, position, ended, commands = 0, 4096, False, []
    def take(count):
        nonlocal cursor
        require(cursor + count <= len(data), 'Truncated method-2 operand')
        value = data[cursor:cursor+count]; cursor += count
        return value
    while cursor < len(data) and position < len(memory) and not ended:
        control = take(1)[0]
        for shift in (6, 4, 2, 0):
            tag = (control >> shift) & 3
            if tag == 3:
                ended = True
                break
            operand = take(2 if tag == 1 else 1)
            count = 1 if tag == 0 else (operand[0] & 15) + 3 if tag == 1 else operand[0] + 2
            require(position + count <= len(memory), 'Method-2 run outside image')
            if tag == 0:
                memory[position] = operand[0]
            elif tag == 1:
                distance = 4096 - ((operand[1] << 4) | (operand[0] >> 4))
                for index in range(count):
                    memory[position+index] = memory[position+index-distance]
            commands.append((tag, operand)); position += count
            if position == len(memory):
                break
    missing = len(memory) - position
    require(not missing or ended, 'Missing method-2 end tag')
    # Authored records have at most two unused alignment bytes after their end.
    require(len(data)-cursor <= 2, f'Unexpected method-2 trailing data: {data[cursor:].hex()} ended={ended} remaining={missing}')
    while position < len(memory):
        count = min(257, len(memory)-position)
        if count == 1:
            commands.append((0, bytes([memory[position]])))
        else:
            commands.append((2, bytes([count-2])))
        position += count
    commands.append((3, b''))
    encoded = bytearray()
    for start in range(0, len(commands), 4):
        group = commands[start:start+4]
        encoded.append(sum(tag << (6-2*i) for i, (tag, _) in enumerate(group)))
        for _, operand in group:
            encoded.extend(operand)
    return bytes(encoded), bytes(memory[4096:]), missing

def recover(original):
    require(len(original) >= 792, 'Truncated intro header')
    header = struct.unpack_from('<I10H', original)
    require(header[:7] == (0x29111994, 10, 399, 12, 1, 22050, 1) and header[9:] == (320,240), 'Unsupported intro metadata')
    palette = original[24:792]; pixels = bytes(76800)
    normalized = bytearray(original[:792]); pcm = bytearray(); raw = bytearray(); early = []
    offset = 792
    for frame in range(399):
        require(offset+1837+8 <= len(original), 'Truncated intro record')
        audio = original[offset:offset+1837]; offset += 1837
        magic, size, flags = struct.unpack_from('<HHI', original, offset); offset += 8
        require(magic == 0x1305 and offset+size <= len(original), 'Invalid intro frame')
        payload = original[offset:offset+size]; offset += size
        if flags == 2:
            fixed, pixels, missing = method2(payload, pixels)
            if missing:
                early.append({'frame': frame, 'unchanged_tail_pixels': missing})
                payload = fixed
        elif flags == 1:
            require(size == 768, 'Invalid intro palette'); palette = payload; pixels = bytes(76800)
        else:
            require(flags == 3 and size == 0, 'Unsupported intro coding method')
        require(max(palette) <= 63, 'Invalid VGA palette')
        pcm.extend(audio); raw.extend(pixels)
        raw.extend(bytes(c for i in range(256) for c in (palette[i*3+2]*4,palette[i*3+1]*4,palette[i*3]*4,255)))
        normalized.extend(audio); normalized.extend(struct.pack('<HHI', magic,len(payload),flags)); normalized.extend(payload)
    require(offset == len(original), 'Unexpected intro trailing records')
    return bytes(normalized), bytes(raw), bytes(pcm), early

def main():
    parser=argparse.ArgumentParser(description=__doc__); parser.add_argument('iso',type=Path); args=parser.parse_args()
    root=Path(__file__).resolve().parents[1]
    def run(*args): return subprocess.run(args,capture_output=True,check=True).stdout
    original=run('7z','e','-so',str(args.iso.resolve(strict=True)),'VIDEO/VFULL_S.GDV')
    normalized, expected, pcm, early = recover(original)
    with tempfile.TemporaryDirectory(prefix='actua-intro-') as temp:
        source=Path(temp)/'full-span.gdv';source.write_bytes(normalized)
        decoded=subprocess.run(['ffmpeg','-v','error','-xerror','-i',str(source),'-map','0:v','-an','-fps_mode','passthrough','-f','rawvideo','-pix_fmt','pal8','pipe:1'],capture_output=True,check=True)
        require(not decoded.stderr.strip(), 'FFmpeg emitted errors')
        require(decoded.stdout == expected, 'Independent full-frame decoder disagreement')
        require(run('ffmpeg','-v','error','-xerror','-i',str(source),'-map','0:a','-f','u8','pipe:1') == pcm, 'Original PCM disagreement')
    target=root/'local-data/VIDEO/INTRO.AMV';target.parent.mkdir(parents=True,exist_ok=True)
    pending=target.with_suffix('.tmp');offsets=[];samples={}
    with pending.open('wb') as out:
        out.write(struct.pack('<8s7I',b'ACTMOV1\0',320,240,12,1,399,22050,len(pcm)))
        out.write(bytes(8*400));out.write(pcm)
        for frame in range(399):
            record=expected[frame*77824:(frame+1)*77824]
            image=Image.frombytes('P',(320,240),record[:76800])
            image.putpalette(bytes(c for i in range(256) for c in record[76800+i*4:76803+i*4][::-1]))
            png=io.BytesIO();image.save(png,format='PNG',compress_level=3)
            offsets.append(out.tell());out.write(png.getvalue())
            if frame in (0,1,24,110,111,144,288,398):
                h=1469598103934665603
                for b in image.convert('RGBA').tobytes():h=((h^b)*1099511628211)&((1<<64)-1)
                samples[str(frame)]=str(h)
        offsets.append(out.tell());out.seek(36);out.write(struct.pack('<400Q',*offsets))
    pending.replace(target)
    sha=lambda b:hashlib.sha256(b).hexdigest()
    report=dict(original='VIDEO/VFULL_S.GDV',source_sha256=sha(original),path='VIDEO/INTRO.AMV',sha256=sha(target.read_bytes()),
        size=target.stat().st_size,frames=399,fps=12,width=320,height=240,pcm_bytes=len(pcm),pcm_sha256=sha(pcm),
        decoded_pal8_sha256=sha(expected),normalized_gdv_sha256=sha(normalized),early_terminated_frames=early,
        rgba_fnv64=samples,verification='All 399 frames agree byte-for-byte with strict FFmpeg decoding of equivalent full-span commands; original PCM agrees exactly.',
        decoder=run('ffmpeg','-version').decode().splitlines()[0])
    (root/'docs/cd-intro-recovery.json').write_text(json.dumps(report,indent=2)+'\n')
    print(f'Imported all 399 intro frames; {len(early)} valid early end tags; {len(pcm)} original PCM bytes. Independent decoding agrees exactly.')

if __name__=='__main__':main()
