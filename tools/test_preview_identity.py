"""Real roster reconciliation and rejection of misleading identity candidates."""
import copy
import unittest
from pathlib import Path
from preview_identity import player,match,normal

ROOT=Path(__file__).resolve().parents[1]

class PreviewIdentityTests(unittest.TestCase):
    def setUp(self):
        self.source=player('t:"NENO" club:"Vitoria Setubal" dob:27/01/62 posn:GK pace:64 pow:65 cntl:57 flar:66 visn:66 acc:76 stam:58 disc:60 name:"POR12"')
        self.retail=player('t:"Adelino Barros NENO" club:"Vitória Setúbal" dob:27/01/62 [POS_gk] pace:64 pow:65 cntl:57 flair:66 visn:66 accr:76 stam:58 disc:60 digi:"POR12" anim:"PORGO2" [FRAPAL]',True)

    def test_full_name_with_independent_identity(self):
        found,method=match(self.source,[self.retail])
        self.assertEqual((found['animation'],found['palette'],method),('PORGO2','FRA','verified_identity'))

    def test_dos_accents(self):
        self.assertEqual(normal(b'J\x81rgen'.decode('cp850')),normal('Jurgen'))
        self.assertEqual(normal(b'CA\xa5IZARES'.decode('cp850')),normal('Canizares'))

    def test_id_alone_never_suffices(self):
        for field in range(len(self.retail['fingerprint'])):
            wrong=copy.deepcopy(self.retail);values=list(wrong['fingerprint']);values[field]='different';wrong['fingerprint']=tuple(values)
            with self.subTest(field=field):self.assertEqual(match(self.source,[wrong]),(None,None))
        wrong=copy.deepcopy(self.retail);wrong['code']='BUL12'
        self.assertEqual(match(self.source,[wrong]),(None,None))

    def test_missing_metadata_or_conflicting_assignments(self):
        wrong=copy.deepcopy(self.source);wrong['fingerprint']=None
        self.assertEqual(match(wrong,[self.retail]),(None,None))
        wrong=copy.deepcopy(self.retail);wrong['animation']='PORGO1'
        self.assertEqual(match(self.source,[self.retail,wrong]),(None,None))
        wrong=copy.deepcopy(self.retail);wrong['name']='Another person'
        self.assertEqual(match(self.source,[self.retail,wrong]),(None,None))

    def test_duplicate_main_and_reserve_name(self):
        self.source['name']=self.retail['name']
        reserve=copy.deepcopy(self.retail);reserve['code']='POR22'
        values=list(reserve['fingerprint']);values[0]='POR22';reserve['fingerprint']=tuple(values)
        found,method=match(self.source,[reserve,self.retail])
        self.assertEqual((found['code'],method),('POR12','verified_identity'))

    def test_all_preserved_roster_entries(self):
        # The checked-in report records the verified, pinned-disc import. Every
        # non-name match must carry all independent identity checks, including Neno.
        import json
        report=json.loads((ROOT/'docs/euro96-previews.json').read_text())
        self.assertEqual(len(report['matches']),320);self.assertEqual(report['missing_players'],[])
        for row in report['matches']:
            if row['method']=='verified_identity':self.assertTrue(row['identity_fields_equal'])
        neno=report['matches'][191]
        self.assertEqual((neno['name'],neno['retail_name'],neno['animation']),('NENO','Adelino Barros NENO','PORGO2'))

if __name__=='__main__':unittest.main()
