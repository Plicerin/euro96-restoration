"""Validate the pinned retail relocation audit and reject damaged evidence."""
from pathlib import Path
import hashlib, sys, unittest
from verify_retail_board_limits import relocation_audit, EXE_SHA


class BoardEvidence(unittest.TestCase):
    def test_pinned_layout_and_positive_control(self):
        report=relocation_audit(self.exe)
        self.assertEqual(report['fixups'],36631)
        self.assertEqual(report['relocated_handler_references'],[])
        self.assertEqual(len(report['relocated_board_state_references']),14)
        self.assertEqual(report['direct_branch_candidates']['0xe107c']['call_candidates'],
                         ['0xb7543','0xb75c4','0xeb007'])

    def reject_change(self, offset, value):
        damaged=bytearray(self.exe);damaged[offset]=value
        with self.assertRaises(ValueError):relocation_audit(damaged)

    def test_wrong_embedded_header(self):self.reject_change(0x292d4,0)
    def test_page_map_gap(self):self.reject_change(0x293e2,2)
    def test_unknown_fixup_form(self):self.reject_change(0x292d4+0x7a1+1,255)
    def test_mismatched_relocated_operand(self):self.reject_change(0xd16de,0)
    def test_missing_live_goal_caller(self):self.reject_change(0xb7543,0x90)


if __name__=='__main__':
    if len(sys.argv)!=2:raise SystemExit('Expected verified retail EURO96.EXE path')
    BoardEvidence.exe=Path(sys.argv.pop()).read_bytes()
    if hashlib.sha256(BoardEvidence.exe).hexdigest()!=EXE_SHA:raise SystemExit('Unverified retail executable')
    unittest.main()
