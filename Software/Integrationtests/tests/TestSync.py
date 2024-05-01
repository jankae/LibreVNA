from tests.TestBase import TestBase
import time


class TestSync(TestBase):
    def test_wai(self):
        self.vna.cmd("*RST")
        self.vna.cmd("VNA:ACQ:SINGLE TRUE")
        self.assertEqual(self.vna.query(":VNA:ACQ:FIN?"), "FALSE")
        self.vna.cmd("*WAI", timeout=3)
        self.assertEqual(self.vna.query(":VNA:ACQ:FIN?"), "TRUE")

    def test_opc_query(self):
        self.vna.cmd("*RST")
        self.vna.cmd("VNA:ACQ:SINGLE TRUE")
        self.assertEqual(self.vna.query(":VNA:ACQ:FIN?"), "FALSE")
        resp = self.vna.query("*OPC?", timeout=3)
        self.assertEqual(resp, "1")
        self.assertEqual(self.vna.query(":VNA:ACQ:FIN?"), "TRUE")

    def test_opc_poll(self):
        self.vna.cmd("*RST")
        self.vna.cmd("VNA:ACQ:SINGLE TRUE")
        self.vna.cmd("*OPC")
        self.assertEqual(self.vna.query(":VNA:ACQ:FIN?"), "FALSE")
        time_limit = time.time() + 4
        while True:
            status = self.vna.get_status()
            if status & 0x01:
                break
            if time.time() >= time_limit:
                raise Exception("Timeout waiting for OPC")
            time.sleep(0.05)
        self.assertEqual(self.vna.query(":VNA:ACQ:FIN?"), "TRUE")
        self.assertEqual(self.vna.get_status(), 0)

    def test_idle_waits(self):
        '''
        Test that *WAI and *OPC? don't hang when device is idle.  Test
        that *OPC query sets the OPC status bit immediately.
        '''
        self.vna.cmd("*RST")
        self.vna.cmd("VNA:ACQ:SINGLE TRUE")
        self.vna.cmd("*WAI", timeout=3)
        self.assertEqual(self.vna.get_status(), 0)
        self.assertEqual(self.vna.query(":VNA:ACQ:FIN?"), "TRUE")
        self.vna.cmd("*WAI")
        resp = self.vna.query("*OPC?")
        self.assertEqual(resp, "1")
        self.assertEqual(self.vna.get_status(), 0)
        self.assertEqual(self.vna.cmd("*OPC"), 0x01)  # should return OPC
        self.assertEqual(self.vna.get_status(), 0)
