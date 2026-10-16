"""Cross-check audio overlays against the independently recorded disc inventory."""
import hashlib, json, struct, unittest
from pathlib import Path
from collections import Counter
from import_euro96 import ROOT
from inspect_cd_audio import inspect_mad
from import_euro96_audio import source_pass_calls


def records(data):
    count, = struct.unpack_from('<I', data, 8);at=12;out={}
    for _ in range(count):
        c,n,size=struct.unpack_from('<HHI',data,at);at+=8
        if (c,n) in out or not 0<size<=len(data)-at: raise ValueError('Invalid record')
        out[c,n]=data[at:at+size];at+=size
    if at!=len(data): raise ValueError('Trailing data')
    return out


class AudioProvenance(unittest.TestCase):
    def test_retail_and_retained_records(self):
        inventory=json.loads((ROOT/'docs/euro96-cd-audio-inspection.json').read_text())
        banks={b['path'].split('/')[-1][2:-4]:b for b in inventory['audio'] if '/PHRASES/E/' in b['path']}
        report=json.loads((ROOT/'docs/euro96-audio-import.json').read_text())
        data=(ROOT/'local-data/COMMENTARY/EURO_EVENTS.BIN').read_bytes()
        self.assertEqual(data[:8],b'AC96COM2'); clips=records(data)
        original=records((ROOT/'local-data/COMMENTARY/EURO_ENGLISH.BIN').read_bytes())
        mapped={r['source_id']:r for r in report['mapped']}
        self.assertEqual(len(mapped),31)
        rebound=mapped[27]
        self.assertEqual((rebound['event'],rebound['source_bank'],rebound['bank'],rebound['retail_id'],rebound['count']),
                         ('GL_POST','GLF','POS',22,8))
        self.assertEqual(set(rebound['imported_numbers']),set(range(1,9)))
        self.assertTrue(all(clips[27,n]!=original[27,n] for n in range(1,9)))
        for category,row in mapped.items():
            entries=banks[row['bank']]['entries'];counts=Counter(e['name'].lower() for e in entries)
            expected={}
            for e in entries:
                number=int(e['name'][-6:-4])
                if counts[e['name'].lower()]==1 and 1<=number<=row['count']:expected[number]=e['sha256']
            actual={n:hashlib.sha256(wav).hexdigest() for (c,n),wav in clips.items() if c==category}
            self.assertEqual(actual,expected)
        for (c,n),wav in clips.items():
            if c not in mapped and c!=44:self.assertEqual(wav,original[c,n])
        score_entries={e['name']:e['sha256'] for e in banks['SCO']['entries']}
        for (c,n),wav in clips.items():
            if c==44:self.assertEqual(hashlib.sha256(wav).hexdigest(),score_entries[f'e_sco{n%100:02d}{chr(97+n//100)}.wav'])
        self.assertEqual(sum(c==44 for c,n in clips),132)
        for score in [10*h+a for h in range(8) for a in range(6) if h>=a]:
            self.assertEqual({n//100 for c,n in clips if c==44 and n%100==score},{0,1,2,3})
        self.assertNotIn((22,30),clips);self.assertNotIn((29,4),clips)
        self.assertEqual(len(clips),927)

    def test_pass_category_callers(self):
        report=json.loads((ROOT/'docs/euro96-audio-import.json').read_text())
        calls=source_pass_calls()
        self.assertEqual(calls,report['source_pass_callers'])
        self.assertEqual({k:len(v) for k,v in calls.items()},
                         dict(CP_SHORT=1,CP_SETPIECE=2,FP_SHORT=0,FP_LOOSE=0))
        self.assertEqual({r['category'] for r in report['retained'] if r['category']<=35},{14,18,20,21})

    def test_pitch_and_publication_inputs(self):
        inventory=json.loads((ROOT/'docs/euro96-cd-audio-inspection.json').read_text())
        expected=next(b for b in inventory['audio'] if b['path']=='DIGI/PITCH/PITCH.MAD')
        data=(ROOT/'local-data/DIGI/PITCH/EURO_PITCH.MAD').read_bytes()
        self.assertEqual(hashlib.sha256(data).hexdigest(),expected['sha256'])
        self.assertEqual(inspect_mad(data),expected['entries'])
        report=json.loads((ROOT/'docs/euro96-audio-import.json').read_text())
        for name,row in report['outputs'].items():
            data=(ROOT/'local-data'/name).read_bytes()
            self.assertEqual(len(data),row['bytes']);self.assertEqual(hashlib.sha256(data).hexdigest(),row['sha256'])


if __name__=='__main__':unittest.main()
