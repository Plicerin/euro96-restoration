"""Reject changed timer evidence instead of silently inventing menu pacing."""
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest
from menu_loop_timing import recover


class MenuTiming(unittest.TestCase):
    source = Path(__file__).resolve().parents[1]/'SOURCE'

    def test_original(self):
        evidence = recover(self.source)
        self.assertEqual((evidence['timer_hz'], evidence['wait_ticks']), (200, 6))
        self.assertEqual(evidence['seconds_per_step'], .03)

    def test_changed_evidence_rejected(self):
        changes = [('NETWORK.C', 'count++;', 'count+=2;'),
                   ('TEST.CPP', 'count<TIMER_SPEED/30', 'count<TIMER_SPEED/60'),
                   ('TEST.CPP', 'TIMER_SPEED,nethandler', '63,nethandler'),
                   ('TIMER.H', 'sosTIMERRegisterEvent(rate,handler,&handle)', 'unverified_timer(rate,handler,&handle)')]
        for name, old, new in changes:
            with self.subTest(name=name, old=old), TemporaryDirectory() as folder:
                target = Path(folder)
                for file in ('DEFINES.H', 'TEST.CPP', 'NETWORK.C', 'TIMER.H'):
                    content = (self.source/file).read_text(encoding='latin1')
                    if file == name:
                        self.assertIn(old, content)
                        content = content.replace(old, new)
                    (target/file).write_text(content, encoding='latin1')
                with self.assertRaises(ValueError):
                    recover(target)


if __name__ == '__main__':
    unittest.main()
