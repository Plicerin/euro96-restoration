"""Bounds and provenance checks for the restoration tooling, using synthetic OMF records."""
from pathlib import Path
import struct,tempfile,unittest
from omf import Object
from restore_ball import routine

def record(typ,body):
 data=struct.pack('<BH',typ,len(body)+1)+body
 return data+bytes([(-sum(data))&255])
def name(s):return bytes([len(s)])+s.encode('ascii')
def sample():
 return b''.join([
  record(0x96,name('')+name('_DATA')+name('DATA')),
  record(0x99,b'\x69'+struct.pack('<I',4)+b'\x02\x03\x01'),
  record(0x91,b'\x00\x01'+name('W?example$nb')+struct.pack('<I',0)+b'\0'),
  record(0xa1,b'\x01'+struct.pack('<I',0)+struct.pack('<f',2.2)),
  record(0x8a,b'\0')])
class OMFContracts(unittest.TestCase):
 def test_function_with_comment_after_signature(self):
  source='char go_to_path(int p)\n// Original signature comment.\n\n{\n return p;\n}\n'
  body,line=routine(source,'go_to_path')
  self.assertEqual(line,1);self.assertEqual(body,source.rstrip())
 def parse(self,data):
  with tempfile.TemporaryDirectory() as folder:
   path=Path(folder)/'sample.obj';path.write_bytes(data);return Object(path)
 def test_scalar_from_named_segment(self):
  obj=self.parse(sample());self.assertAlmostEqual(obj.value('example','f'),2.2,places=6)
  self.assertEqual(obj.segments[1]['name'],'_DATA')
  self.assertTrue(all(obj.segments[1]['written']))
 def test_rejects_truncated_record(self):
  with self.assertRaises(ValueError):self.parse(sample()[:-1])
 def test_rejects_bad_checksum(self):
  data=bytearray(sample());data[8]^=1
  with self.assertRaises(ValueError):self.parse(data)
 def test_rejects_out_of_segment_data(self):
  with self.assertRaises(ValueError):self.parse(sample()+record(0xa1,b'\1'+struct.pack('<I',3)+b'ab'))
 def test_rejects_unbounded_segment(self):
  with self.assertRaises(ValueError):self.parse(record(0x96,name('')+name('_DATA')+name('DATA'))+record(0x99,b'\x69'+struct.pack('<I',0xffffffff)+b'\2\3\1'))
 def test_unwritten_bytes_are_not_marked_initialized(self):
  data=record(0x96,name('')+name('_DATA')+name('DATA'))+record(0x99,b'\x69'+struct.pack('<I',4)+b'\2\3\1')
  self.assertEqual(self.parse(data).segments[1]['written'],b'\0'*4)
 def test_missing_data_fixup_is_rejected_even_when_skipping_comdat(self):
  obj=self.parse(record(0x9d,b'\xe4\x00\x56\x01'))
  with self.assertRaises(ValueError):obj.fixups(skip_comdat=True)
 def test_comdat_requires_explicit_skip(self):
  obj=self.parse(record(0xc3,b'')+record(0x9d,b'\xe4\x00\x56\x01'))
  with self.assertRaises(ValueError):obj.fixups()
  self.assertEqual(obj.fixups(skip_comdat=True),[])
 def test_normal_data_after_comdat_is_not_skipped(self):
  obj=self.parse(record(0xc3,b'')+sample()+record(0x9d,b'\xe4\x00\x56\x01'))
  self.assertEqual(len(obj.fixups(skip_comdat=True)),1)
 def test_lidata_requires_its_own_explicit_skip(self):
  obj=self.parse(record(0xa3,b'')+record(0x9d,b'\xe4\x00\x56\x01'))
  with self.assertRaises(ValueError):obj.fixups(skip_comdat=True)
  self.assertEqual(obj.fixups(skip_lidata=True),[])
 def test_initialized_data_after_lidata_is_not_skipped(self):
  obj=self.parse(record(0xa3,b'')+sample()+record(0x9d,b'\xe4\x00\x56\x01'))
  self.assertEqual(len(obj.fixups(skip_lidata=True)),1)
if __name__=='__main__':unittest.main()
