"""Independent CD-inventory checks for the period speech overlay."""
from collections import Counter
import hashlib, json, unittest
from import_euro96 import ROOT
from test_euro96_audio import records


class PeriodProvenance(unittest.TestCase):
    def test_all_unambiguous_original_recordings(self):
        inventory = json.loads((ROOT/'docs/euro96-cd-audio-inspection.json').read_text())
        expected = {}
        for category, bank in [(96, 'HLF'), (97, 'FLL'), (98, 'END'), (104, 'XTM')]:
            rows = next(b['entries'] for b in inventory['audio'] if b['path'] == f'DIGI/PHRASES/E/E_{bank}.MAD')
            counts = Counter(r['name'].lower() for r in rows)
            for row in rows:
                name = row['name'].lower()
                if counts[name] != 1: continue
                number = int(name[5:7])
                if category in (96,97): number += 100*(ord(name[7])-ord('a'))
                expected[category, number] = row['sha256']
        data = (ROOT/'local-data/COMMENTARY/EURO_PERIODS.BIN').read_bytes()
        self.assertEqual(data[:8], b'AC96PER1')
        actual = records(data)
        self.assertEqual({k: hashlib.sha256(wav).hexdigest() for k, wav in actual.items()}, expected)
        self.assertEqual(Counter(c for c, n in actual), {96:112, 97:131, 98:13, 104:26})
        # These holes and duplicate labels are in the independently inventoried
        # MAD directory. Never let a nearby score fill them.
        for score in (11, 21, 31, 32, 33):
            self.assertFalse(any(c == 96 and n % 100 == score for c, n in actual))
        self.assertNotIn((97, 232), actual)
        report = json.loads((ROOT/'docs/euro96-periods-import.json').read_text())
        self.assertEqual(len(report['excluded']), 8)
        self.assertEqual(report['outputs']['COMMENTARY/EURO_PERIODS.BIN']['sha256'], hashlib.sha256(data).hexdigest())


if __name__ == '__main__': unittest.main()
