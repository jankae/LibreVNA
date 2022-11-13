from tests.TestBase import TestBase

class TestMode(TestBase):
    def test_Mode(self):
        modes = ["VNA", "GEN", "SA"]
        for m in modes:
            self.vna.cmd(":DEV:MODE "+m)
            self.assertEqual(self.vna.query(":DEV:MODE?"), m)
