"""Check sequence metadata and WAVs against independently recorded CD evidence."""
import hashlib,json,struct,unittest
from import_euro96 import ROOT,CODES
from test_euro96_audio import records


class Sequences(unittest.TestCase):
    def test_original_pcm_and_fixture_lookup(self):
        data=(ROOT/'local-data/COMMENTARY/EURO_SEQUENCES.BIN').read_bytes()
        self.assertEqual(data[:8],b'AC96SEQ1')
        fixtures=[tuple(data[12+i*3:15+i*3]) for i in range(31)]
        self.assertEqual([f[0] for f in fixtures],[1,2,3,4,5,6,7,8,10,9,12,11,13,14,15,16,18,20,19,17,22,24,21,23,25,26,27,28,29,30,31])
        self.assertEqual(fixtures[0],(1,4,14))
        self.assertEqual(fixtures[9],(9,14,7))
        self.assertTrue(all(f[1:]==(255,255) for f in fixtures[24:]))
        clips=records(data[:12]+data[105:])
        inventory=json.loads((ROOT/'docs/euro96-cd-audio-inspection.json').read_text())
        banks={b['path']:b['entries'] for b in inventory['audio'] if 'entries' in b}
        expected={}
        for entry in banks['DIGI/PHRASES/E/E_STT.MAD']:
            expected[99,int(entry['name'][5:7])]=entry['sha256']
        for team,code in enumerate(CODES):
            entries={e['name'].lower():e['sha256'] for e in banks[f'DIGI/TEAMS/E/E_{code}.MAD']}
            for category,suffix in [(100,'ttz'),(101,'ttc'),(102,'ttg'),(103,'ttv')]:expected[category,team+1]=entries[f'e_{code.lower()}{suffix}.wav']
        self.assertEqual({k:hashlib.sha256(wav).hexdigest() for k,wav in clips.items()},expected)
        self.assertEqual(len(clips),94);self.assertNotIn((99,20),clips)
        report=json.loads((ROOT/'docs/euro96-sequences-import.json').read_text())
        self.assertEqual(report['outputs']['COMMENTARY/EURO_SEQUENCES.BIN']['sha256'],hashlib.sha256(data).hexdigest())

    def test_retail_qualifier_resolution(self):
        report=json.loads((ROOT/'docs/euro96-sequences-import.json').read_text())
        self.assertEqual(report['qualifier_table_file_offset'],0x139ad8)
        self.assertEqual(report['retail_qualifier_positions'],[0,1,4,5,8,9,12,13])
        quarters=report['fixtures'][24:28]
        self.assertEqual([f['retail_slots'] for f in quarters],[[19,16],[18,17],[20,23],[21,22]])
        self.assertEqual([f['resolved_retail_slots'] for f in quarters],[[5,0],[4,1],[8,13],[9,12]])
        self.assertEqual([f['source_slots'] for f in quarters],[[0,5],[4,1],[8,13],[12,9]])
        self.assertEqual([f['number'] for f in quarters],[25,26,27,28])
        for fixture in report['fixtures']:
            self.assertEqual(sorted(fixture['resolved_retail_slots']),sorted(fixture['source_slots']))
        anchors={r['file_offset']:r['bytes'] for r in report['instruction_evidence']}
        self.assertEqual(anchors[0x808a2],'0fb68084a20000')
        self.assertEqual(anchors[0x80939],'80c210')
        self.assertEqual(anchors[0x80940],'8890849a0400')


if __name__=='__main__':unittest.main()
