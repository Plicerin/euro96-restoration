"""Small independent examples for the intro's compressed-frame boundary."""
import unittest
from import_cd_intro import method2, recover

class IntroImportTests(unittest.TestCase):
    def test_early_end_preserves_tail(self):
        # Literal 9, then END: three existing pixels are intentionally untouched.
        fixed, pixels, missing = method2(bytes([0x30,9]), bytes([1,2,3,4]))
        self.assertEqual((pixels,missing),(bytes([9,2,3,4]),3))
        self.assertEqual(method2(fixed,bytes([1,2,3,4]))[1:],(pixels,0))

    def test_overlapping_copy(self):
        # Literal 7 followed by distance -1, length 5.
        _, pixels, missing = method2(bytes([0x10,7,0xf2,0xff]),bytes(6))
        self.assertEqual((pixels,missing),(bytes([7]*6),0))

    def test_preamble(self):
        # Offset -4096 reads the initial run of zero pixels.
        self.assertEqual(method2(bytes([0x40,0,0]),bytes([8]*3))[1],bytes(3))

    def test_skip_and_single_tail(self):
        fixed,pixels,missing=method2(bytes([0xb0,0]),bytes([4,5,6]))
        self.assertEqual((pixels,missing),(bytes([4,5,6]),1))
        self.assertEqual(method2(fixed,bytes([4,5,6]))[1:],(pixels,0))

    def test_rejects_truncated_operands_and_unterminated_frames(self):
        for payload in (b'',b'\x00',b'\x40\xff',b'\x80',b'\x00\x01'):
            with self.assertRaises(ValueError):method2(payload,bytes(10))

    def test_rejects_runs_outside_image(self):
        with self.assertRaises(ValueError):method2(b'\x80\xff',bytes(10))

    def test_rejects_wrong_file(self):
        for payload in (b'',bytes(792)):
            with self.assertRaises(ValueError):recover(payload)

if __name__=='__main__':unittest.main()
