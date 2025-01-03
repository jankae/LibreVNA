from tests.TestBase import TestBase
import time

class TestSASweep(TestBase):
    def waitSweepTimeout(self, timeout = 1):
        self.assertEqual(self.vna.query(":SA:ACQ:FIN?"), "FALSE")
        stoptime = time.time() + timeout
        while self.vna.query(":SA:ACQ:FIN?") == "FALSE":
            if time.time() > stoptime:
                raise AssertionError("Sweep timed out")
                
    def assertTrace_dB(self, trace, dB_nominal, dB_deviation):
        for S in trace:
            self.assertLessEqual(S[1], dB_nominal + dB_deviation)
            self.assertGreaterEqual(S[1], dB_nominal - dB_deviation)
            
    def test_sweep_frequency(self):
        self.vna.cmd(":DEV:MODE SA")
        self.vna.cmd(":SA:FREQ:CENT 2000000000")
        self.vna.cmd(":SA:FREQ:SPAN 200000")
        
        self.assertEqual(float(self.vna.query(":SA:FREQ:START?")), 1999900000)
        self.assertEqual(float(self.vna.query(":SA:FREQ:STOP?")), 2000100000)
        
        self.vna.cmd(":SA:ACQ:RBW 10000")
        self.waitSweepTimeout(10)
        
        port1 = self.vna.parse_SA_trace_data(self.vna.query(":SA:TRACE:DATA? PORT1"))
        port2 = self.vna.parse_SA_trace_data(self.vna.query(":SA:TRACE:DATA? PORT2"))
        
        self.assertEqual(port1[0][0], 1999900000)
        self.assertEqual(port1[-1][0], 2000100000)
        self.assertEqual(port2[0][0], 1999900000)
        self.assertEqual(port2[-1][0], 2000100000)
       
        # No signal present, signal level should be very low
        self.assertTrace_dB(port1, -140, 60)
        self.assertTrace_dB(port2, -140, 60)
        
    def test_sweep_zerospan(self):
        self.vna.cmd(":DEV:MODE SA")
        self.vna.cmd(":SA:FREQ:CENT 2000000000")
        self.vna.cmd(":SA:FREQ:SPAN 0")
        
        self.assertEqual(float(self.vna.query(":SA:FREQ:START?")), 2000000000)
        self.assertEqual(float(self.vna.query(":SA:FREQ:STOP?")), 2000000000)
        
        self.vna.cmd(":SA:ACQ:RBW 10000")
        self.waitSweepTimeout(10)
        
        port1 = self.vna.parse_SA_trace_data(self.vna.query(":SA:TRACE:DATA? PORT1"))
        port2 = self.vna.parse_SA_trace_data(self.vna.query(":SA:TRACE:DATA? PORT2"))
        
        self.assertEqual(port1[0][0], 0.0)
        self.assertGreater(port1[-1][0], 4)
        self.assertLess(port1[-1][0], 8)
        self.assertEqual(port2[0][0], 0.0)
        self.assertGreater(port2[-1][0], 4)
        self.assertLess(port2[-1][0], 8)
       
        # No signal present, signal level should be very low
        self.assertTrace_dB(port1, -140, 60)
        self.assertTrace_dB(port2, -140, 60)
        
    def test_tracking_generator(self):
        self.vna.cmd(":DEV:MODE SA")
        self.vna.cmd(":SA:FREQ:CENT 1930000000")
        self.vna.cmd(":SA:FREQ:SPAN 200000")       
        self.vna.cmd(":SA:ACQ:RBW 10000")
        self.vna.cmd(":SA:TRACK:PORT 1")
        self.assertEqual(self.vna.query(":SA:TRACK:PORT?"), "1")
        self.vna.cmd(":SA:TRACK:LVL -20")
        self.vna.cmd(":SA:TRACK:OFFSET 0")
        self.vna.cmd(":SA:TRACK:EN TRUE")
        self.waitSweepTimeout(10)
        
        port1 = self.vna.parse_SA_trace_data(self.vna.query(":SA:TRACE:DATA? PORT1"))
       
        level = port1[0][1]
        self.assertGreater(level, -30)
        self.assertLess(level, -10)
        # check tracking generator signal
        self.assertTrace_dB(port1, level, 5)
        
        # Enable normalization
        self.vna.cmd(":SA:TRACK:NORM:LVL -10")
        self.vna.cmd(":SA:TRACK:NORM:EN TRUE")
        # wait for normalization to finish
        self.waitSweepTimeout(10)
        
        # trigger the next sweep
        self.vna.cmd(":SA:ACQ:SINGLE TRUE")
        self.waitSweepTimeout(10)
        
        # Reported level on port1 should match normalization very closely now
        port1 = self.vna.parse_SA_trace_data(self.vna.query(":SA:TRACE:DATA? PORT1"))
        self.assertTrace_dB(port1, -10, 1)
       
