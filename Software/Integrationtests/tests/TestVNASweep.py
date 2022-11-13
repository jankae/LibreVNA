from tests.TestBase import TestBase
import time

class TestVNASweep(TestBase):
    def test_sweep_frequency(self):
        self.vna.cmd(":DEV:MODE VNA")
        self.vna.cmd(":VNA:SWEEP FREQUENCY")
        self.vna.cmd(":VNA:STIM:LVL -10")
        self.vna.cmd(":VNA:ACQ:IFBW 10000")
        self.vna.cmd(":VNA:ACQ:AVG 1")
        self.vna.cmd(":VNA:ACQ:POINTS 501")
        self.vna.cmd(":VNA:FREQuency:START 1000000")
        self.vna.cmd(":VNA:FREQuency:STOP 6000000000")
        while self.vna.query(":VNA:ACQ:FIN?") == "FALSE":
            time.sleep(0.1)
        
        S11 = self.vna.parse_trace_data(self.vna.query(":VNA:TRACE:DATA? S11"))
        self.assertEqual(S11[0][0], 1000000)
        self.assertEqual(S11[-1][0], 6000000000)
        
    def test_sweep_zerospan(self):
        self.vna.cmd(":DEV:MODE VNA")
        self.vna.cmd(":VNA:SWEEP FREQUENCY")
        self.vna.cmd(":VNA:STIM:LVL -10")
        self.vna.cmd(":VNA:ACQ:IFBW 10000")
        self.vna.cmd(":VNA:ACQ:AVG 1")
        self.vna.cmd(":VNA:ACQ:POINTS 501")
        self.vna.cmd(":VNA:FREQuency:START 500000000")
        self.vna.cmd(":VNA:FREQuency:STOP 1500000000")
        self.vna.cmd(":VNA:FREQuency:ZERO 1500000000")
        while self.vna.query(":VNA:ACQ:FIN?") == "FALSE":
            time.sleep(0.1)
        
        S11 = self.vna.parse_trace_data(self.vna.query(":VNA:TRACE:DATA? S11"))
        self.assertEqual(S11[0][0], 0.0)
        # Sweep should take about 0.125 seconds
        self.assertGreater(S11[-1][0], 0.1)
        self.assertLess(S11[-1][0], 0.5)
        
    def test_sweep_power(self):
        self.vna.cmd(":DEV:MODE VNA")
        self.vna.cmd(":VNA:SWEEP POWER")
        self.vna.cmd(":VNA:STIM:LVL -10")
        self.vna.cmd(":VNA:ACQ:IFBW 10000")
        self.vna.cmd(":VNA:ACQ:AVG 1")
        self.vna.cmd(":VNA:ACQ:POINTS 501")
        self.vna.cmd(":VNA:POWER:START -30")
        self.vna.cmd(":VNA:POWER:STOP -10")
        while self.vna.query(":VNA:ACQ:FIN?") == "FALSE":
            time.sleep(0.1)
        
        S11 = self.vna.parse_trace_data(self.vna.query(":VNA:TRACE:DATA? S11"))
        self.assertEqual(S11[0][0], -30)
        self.assertEqual(S11[-1][0], -10)