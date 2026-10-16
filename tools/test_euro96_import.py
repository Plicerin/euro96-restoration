"""Boundaries for the retail decoder and independent imported-media provenance checks."""
import hashlib
import json
import struct
import unittest
from import_euro96 import ROOT, unmangle, read_names
from preview_identity import player

class DecoderTests(unittest.TestCase):
    def test_tokens(self):
        for encoded, expected in [
            (b'\x03abc\x80\0', b'abcabc'),
            (b'\x03abc\xc0\x00\0', b'abcabca'),
            (b'\x03abc\xe0\x00\x01\0', b'abcabcabc'),
            (b'\x01A\x60\0', b'AAAA'),
            (b'\x02AB\x70\0', b'ABABAB'),
            (b'\x02\xfe\xff\x40\0', b'\xfe\xff\x00\x01\x02'),
            (b'\x04\xfe\xff\xff\xff\x50\0', b'\xfe\xff\xff\xff\x00\x00\x01\x00'),
        ]:
            self.assertEqual(unmangle(encoded),expected)

    def test_rejected(self):
        for encoded in [b'',b'\x03ab',b'\x80\0',b'\x60\0',b'\x01a\x40\0',b'\x02ab\x50\0',b'\0x']:
            with self.assertRaises(ValueError): unmangle(encoded)
        with self.assertRaises(ValueError): unmangle(b'\x01a\x6f\0',limit=5)

    def test_provenance(self):
        report=json.loads((ROOT/'docs/euro96-import.json').read_text())
        inventory=json.loads((ROOT/'docs/euro96-cd-audio-inspection.json').read_text())
        names=read_names((ROOT/'local-data/COMMENTARY/EURO_NAMES.BIN').read_bytes())
        for path,row in report['outputs'].items():
            data=(ROOT/'local-data'/path).read_bytes()
            self.assertEqual(len(data),row['bytes']);self.assertEqual(hashlib.sha256(data).hexdigest(),row['sha256'])
        # The pre-import CD inventory is independent of the pack generation.
        original_hashes=set()
        def visit(value):
            if isinstance(value,dict):
                if 'sha256' in value:original_hashes.add(value['sha256'])
                for v in value.values():visit(v)
            elif isinstance(value,list):
                for v in value:visit(v)
        visit(inventory)
        for row in report['matches']:
            for clip in row['clips']:
                data=names[128+row['team']*20+row['shirt']-1,clip['tone']]
                self.assertEqual(hashlib.sha256(data).hexdigest(),clip['sha256'])
                self.assertIn(clip['sha256'],original_hashes)
        self.assertEqual(report['retail_name_players'],319)
        self.assertEqual(report['total_name_players'],320)
        self.assertEqual(len(names),960)
        self.assertEqual(sum(len(p['clips']) for p in report['matches']),959)
        self.assertEqual(report['appearance_matches'],320)
        self.assertFalse(report['unmatched'])
        # Match source order against the separate, already verified preview audit.
        previews=json.loads((ROOT/'docs/euro96-previews.json').read_text())
        source=(ROOT/'SOURCE/SCRIPT.96').read_text(encoding='cp850')
        roster=[p for line in source[source.index('PLYR_NMES'):source.index('TEAM_NMES')].splitlines() if (p:=player(line))]
        appearance=(ROOT/'local-data/GRAPHICS/EURO96.BIN').read_bytes()[-320:]
        for i,row in enumerate(report['matches']):
            self.assertEqual((row['team'],row['shirt']),(i//20,i%20+1))
            self.assertEqual(row['name'],roster[i]['name'])
            self.assertEqual(row['digi'],previews['matches'][i]['retail_id'])
            self.assertEqual(row['retail_name'],previews['matches'][i]['retail_name'])
            if row['match_method']=='verified_identity':
                self.assertEqual(row['source_identity']['fingerprint'],row['retail_identity']['fingerprint'])
                self.assertIsNotNone(row['source_identity']['fingerprint'])
            self.assertEqual(appearance[i],int(row['skin']=='BLK'))
        # The sole missing Euro tone is demonstrably absent in the CD directory;
        # its preserved compatibility WAV stays attached to the same source player.
        rus=next(b for b in inventory['audio'] if b['path'].endswith('/E_RUS.MAD'))
        self.assertNotIn('e_rus17x.wav',{e['name'].lower() for e in rus['entries']})
        base=read_names((ROOT/'local-data/COMMENTARY/NAMES.BIN').read_bytes())
        self.assertEqual(names[128+11*20+16,1],base[128+11*20+16,1])

if __name__=='__main__':unittest.main()
