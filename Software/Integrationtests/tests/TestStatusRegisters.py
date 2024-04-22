import re
from tests.TestBase import TestBase


class TestStatusRegisters(TestBase):
    def query_stb(self):
        resp = self.vna.query("*STB?")
        self.assertTrue(re.match(r"^\d+$", resp))
        value = int(resp)
        self.assertTrue(value >= 0 and value <= 255)
        return value

    def test_invalid_command(self):
        status = self.vna.get_status()
        self.assertEqual(status, 0)
        self.vna.default_check_cmds = False
        self.vna.cmd("INVALID:COMMAND")
        status = self.vna.get_status()
        self.assertEqual(status & 0x3C, 0x20)
        status = self.vna.get_status()
        self.assertEqual(status, 0)

    def test_invalid_query(self):
        status = self.vna.get_status()
        self.assertEqual(status, 0)
        self.vna.default_check_cmds = False
        self.vna.cmd("INVALID:QUERY?")  # send as cmd to avoid timeout
        status = self.vna.get_status()
        self.assertTrue(status & 0x20)  # expect CME
        status = self.vna.get_status()
        self.assertEqual(status, 0)

    def test_stb(self):
        self.vna.default_check_cmds = False
        self.vna.cmd("*SRE 0")
        status = self.vna.get_status()
        if status & 0x20:
            self.skipTest("Skipping test: *SRE, *SRE?, *STB? not implemented")
        self.vna.cmd("*RST")
        self.vna.cmd("VNA:ACQ:SINGLE TRUE")
        self.vna.cmd("*WAI")
        status = self.vna.get_status()
        self.assertEqual(status, 0)
        self.vna.cmd("OPC")              # should set OPC
        self.vna.cmd(f"*ESE {0x21:d}")   # mask is CME|OPC
        self.assertEqual(self.query_stb() & 0x60, 0x20)  # expect !MSS, ESB
        self.assertEqual(self.query_stb() & 0x60, 0x20)  # shouldn't clear
        self.vna.cmd(f"*SRE {0x20:d}")   # unmask ESB
        self.assertEqual(self.query_stb() & 0x60, 0x60)  # expect MSS, ESB
        self.vna.cmd(f"*ESE {0x20:d}")   # mask is CME only
        self.assertEqual(self.query_stb() & 0x60, 0)     # expect !MSS, !ESB
        self.vna.cmd("INVALID:COMMAND")  # should set CME
        self.assertEqual(self.query_stb() & 0x60, 0x60)  # expect MSS, ESB
        status = self.get_status()
        self.assertEqual(status, 0x21)   # expect CMD|OPC, clears
        self.assertEqual(self.query_stb() & 0x60, 0)     # expect !MSS, !ESB
