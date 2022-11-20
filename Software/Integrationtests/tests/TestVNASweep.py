from tests.TestBase import TestBase
import time

class TestVNASweep(TestBase):
    def waitSweepTimeout(self, timeout = 1):
        self.assertEqual(self.vna.query(":VNA:ACQ:FIN?"), "FALSE")
        stoptime = time.time() + timeout
        while self.vna.query(":VNA:ACQ:FIN?") == "FALSE":
            if time.time() > stoptime:
                raise AssertionError("Sweep timed out")
            
    def test_sweep_frequency(self):
        self.vna.cmd(":DEV:MODE VNA")
        self.vna.cmd(":VNA:SWEEP FREQUENCY")
        self.vna.cmd(":VNA:STIM:LVL -10")
        self.vna.cmd(":VNA:ACQ:IFBW 10000")
        self.vna.cmd(":VNA:ACQ:AVG 1")
        self.vna.cmd(":VNA:ACQ:POINTS 501")
        self.vna.cmd(":VNA:FREQuency:START 1000000")
        self.vna.cmd(":VNA:FREQuency:STOP 6000000000")
        self.waitSweepTimeout(2)
        
        S11 = self.vna.parse_VNA_trace_data(self.vna.query(":VNA:TRACE:DATA? S11"))
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
        self.waitSweepTimeout(2)
        
        S11 = self.vna.parse_VNA_trace_data(self.vna.query(":VNA:TRACE:DATA? S11"))
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
        self.waitSweepTimeout(2)
        
        S11 = self.vna.parse_VNA_trace_data(self.vna.query(":VNA:TRACE:DATA? S11"))
        self.assertEqual(S11[0][0], -30)
        self.assertEqual(S11[-1][0], -10)
               
    def test_fast_single_sweeps(self):
        self.vna.cmd(":DEV:MODE VNA")
        self.vna.cmd(":VNA:SWEEP FREQUENCY")
        self.vna.cmd(":VNA:STIM:LVL -10")
        self.vna.cmd(":VNA:ACQ:IFBW 50000")
        self.vna.cmd(":VNA:ACQ:AVG 1")
        self.vna.cmd(":VNA:ACQ:POINTS 501")
        self.vna.cmd(":VNA:FREQuency:START 1000000")
        self.vna.cmd(":VNA:FREQuency:STOP 6000000000")
        
        for i in range(10):
            # Change something irrelevant (to force reconfiguration of device)
            self.vna.cmd(":VNA:FREQuency:START "+str(1000000+i))
            self.waitSweepTimeout(2)