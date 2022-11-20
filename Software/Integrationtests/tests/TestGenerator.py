from tests.TestBase import TestBase

class TestGenerator(TestBase):
    def test_Generator(self):
        self.vna.cmd(":DEV:MODE GEN")
        self.assertEqual(self.vna.query(":DEV:MODE?"), "GEN")

        self.vna.cmd(":GEN:FREQ 2000000000")
        self.assertEqual(float(self.vna.query(":GEN:FREQ?")), 2000000000)

        self.vna.cmd(":GEN:LVL -12.34")
        self.assertEqual(float(self.vna.query(":GEN:LVL?")), -12.34)

        self.assertEqual(self.vna.query(":GEN:PORT?"), "0")
        self.vna.cmd(":GEN:PORT 2")
        self.assertEqual(self.vna.query(":GEN:PORT?"), "2")
