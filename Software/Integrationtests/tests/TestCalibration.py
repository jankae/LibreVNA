from tests.TestBase import TestBase
from libreCAL import libreCAL
import time
import math

class TestCalibration(TestBase):
    def cal_measure(self, number, timeout = 3):
        self.vna.cmd(":VNA:CAL:MEAS "+str(number))
        # wait for the measurement to finish
        assert self.vna.query(":VNA:CAL:BUSY?") == "TRUE"
        self.vna.cmd("*WAI", timeout=timeout)
        assert self.vna.query(":VNA:CAL:BUSY?") == "FALSE"
    
    def test_dummy_calibration(self):
        # This test just iterates through the calibration steps. As no actual standards
        # are applied to the ports, the calibration result is just random data
        # Set up the sweep first
        self.vna.cmd(":DEV:MODE VNA")
        self.vna.cmd(":VNA:SWEEP FREQUENCY")
        self.vna.cmd(":VNA:STIM:LVL -10")
        self.vna.cmd(":VNA:ACQ:IFBW 50000")
        self.vna.cmd(":VNA:ACQ:AVG 1")
        self.vna.cmd(":VNA:ACQ:POINTS 501")
        self.vna.cmd(":VNA:FREQuency:START 1000000")
        self.vna.cmd(":VNA:FREQuency:STOP 6000000000")
        
        self.vna.cmd(":VNA:CAL:RESET")

        # No measurements yet, activating should fail
        self.vna.cmd(":VNA:CAL:ACT SOLT_1", check=False)
        self.assertTrue(self.vna.get_status() & 0x3C)

        # Load calibration kit
        self.assertEqual(self.vna.query(":VNA:CAL:KIT:LOAD? DUMMY.CALKIT"), "TRUE")

        # Lets take the measurements for port 1 first
        self.vna.cmd(":VNA:CAL:ADD OPEN")
        self.vna.cmd(":VNA:CAL:ADD SHORT")
        self.vna.cmd(":VNA:CAL:ADD LOAD")
        self.vna.cmd(":VNA:CAL:PORT 0 1")
        self.vna.cmd(":VNA:CAL:PORT 1 1")
        self.vna.cmd(":VNA:CAL:PORT 2 1")
        
        self.cal_measure(0)
        self.cal_measure(1)
        self.cal_measure(2)
                           
        # SOLT_1 should now be available
        self.vna.cmd(":VNA:CAL:ACT SOLT_1", check=False)
        self.assertFalse(self.vna.get_status() & 0x3C)

        # SOLT_2 and SOLT_12 should still be unavailable
        self.vna.cmd(":VNA:CAL:ACT SOLT_2", check=False)
        self.assertTrue(self.vna.get_status() & 0x3C)
        self.vna.cmd(":VNA:CAL:ACT SOLT_12", check=False)
        self.assertTrue(self.vna.get_status() & 0x3C)

        # Take measurements for SOLT_2
        self.vna.cmd(":VNA:CAL:ADD OPEN")
        self.vna.cmd(":VNA:CAL:ADD SHORT")
        self.vna.cmd(":VNA:CAL:ADD LOAD")
        self.vna.cmd(":VNA:CAL:PORT 3 2")
        self.vna.cmd(":VNA:CAL:PORT 4 2")
        self.vna.cmd(":VNA:CAL:PORT 5 2")
        
        self.cal_measure(3)
        self.cal_measure(4)
        self.cal_measure(5)

        # SOLT_1 and SOLT_2 should now be available
        self.vna.cmd(":VNA:CAL:ACT SOLT_1", check=False)
        self.assertFalse(self.vna.get_status() & 0x3C)
        self.vna.cmd(":VNA:CAL:ACT SOLT_2", check=False)
        self.assertFalse(self.vna.get_status() & 0x3C)

        # SOLT_12 should still be unavailable
        self.vna.cmd(":VNA:CAL:ACT SOLT_12", check=False)
        self.assertTrue(self.vna.get_status() & 0x3C)

        # Take the final through measurement for SOLT_12
        self.vna.cmd(":VNA:CAL:ADD THROUGH")
        self.vna.cmd(":VNA:CAL:PORT 6 1 2")
        
        self.cal_measure(6)
        
        # SOLT_1, SOLT_2 and SOLT_12 should now be available
        self.vna.cmd(":VNA:CAL:ACT SOLT_1", check=False)
        self.assertFalse(self.vna.get_status() & 0x3C)
        self.vna.cmd(":VNA:CAL:ACT SOLT_2", check=False)
        self.assertFalse(self.vna.get_status() & 0x3C)
        self.vna.cmd(":VNA:CAL:ACT SOLT_12", check=False)
        self.assertFalse(self.vna.get_status() & 0x3C)
        
    def assertTrace_dB(self, trace, dB_nominal, dB_deviation):
        for S in trace:
            dB = 20*math.log10(abs(S[1]))
            self.assertLessEqual(dB, dB_nominal + dB_deviation)
            self.assertGreaterEqual(dB, dB_nominal - dB_deviation)
        
    def test_SOLT_calibration(self):
        # This test performs a 2-port SOLT calibration with a connected LibreCAL
        # Afterwqrds, the calibration is checked with a 6dB attenuator
        # Set up the sweep first
        self.vna.cmd(":DEV:MODE VNA")
        self.vna.cmd(":VNA:SWEEP FREQUENCY")
        self.vna.cmd(":VNA:STIM:LVL -10")
        self.vna.cmd(":VNA:ACQ:IFBW 1000")
        self.vna.cmd(":VNA:ACQ:AVG 1")
        self.vna.cmd(":VNA:ACQ:POINTS 501")
        self.vna.cmd(":VNA:FREQuency:START 50000000")
        self.vna.cmd(":VNA:FREQuency:STOP 6000000000")
        
        self.vna.cmd(":VNA:CAL:RESET")

        # Load calibration file (only for standard and measurement setup, no
        # measurements are included)
        self.assertEqual(self.vna.query(":VNA:CAL:LOAD? LIBRECAL.CAL"), "TRUE")
        
        # Take the measurements
               
        cal = libreCAL()
        # Connections:
        # LibreVNA -> LibreCAL
        # 1 -> 3
        # 2 -> 1
        cal.reset()
        cal.setPort(cal.Standard.OPEN, 1)
        cal.setPort(cal.Standard.OPEN, 3)
        self.cal_measure(0, 15)
        self.cal_measure(3, 15)

        cal.setPort(cal.Standard.SHORT, 1)
        cal.setPort(cal.Standard.SHORT, 3)
        self.cal_measure(1, 15)
        self.cal_measure(4, 15)

        cal.setPort(cal.Standard.LOAD, 1)
        cal.setPort(cal.Standard.LOAD, 3)
        self.cal_measure(2, 15)
        self.cal_measure(5, 15)

        cal.setPort(cal.Standard.THROUGH, 1, 3)
        self.cal_measure(6, 15)
        
        # activate calibration
        self.vna.cmd(":VNA:CAL:ACT SOLT_12", check=False)
        self.assertFalse(self.vna.get_status() & 0x3C)

        # switch in 6dB attenuator
        cal.setPort(cal.Standard.THROUGH, 1, 2)        
        cal.setPort(cal.Standard.THROUGH, 3, 4)        

        # Start measurement and grab data
        self.vna.cmd(":VNA:ACQ:SINGLE TRUE")
        self.assertEqual(self.vna.query(":VNA:ACQ:FIN?"), "FALSE")
        self.vna.cmd("*WAI", timeout=3)
        self.assertEqual(self.vna.query(":VNA:ACQ:FIN?"), "TRUE")

        cal.reset()
        
        # grab trace data
        S11 = self.vna.parse_VNA_trace_data(self.vna.query(":VNA:TRACE:DATA? S11"))
        S12 = self.vna.parse_VNA_trace_data(self.vna.query(":VNA:TRACE:DATA? S12"))
        S21 = self.vna.parse_VNA_trace_data(self.vna.query(":VNA:TRACE:DATA? S21"))
        S22 = self.vna.parse_VNA_trace_data(self.vna.query(":VNA:TRACE:DATA? S22"))
    
        # Attenuation is frequency dependent, use excessively large limits
        # TODO: use smaller limits based on frequency
        self.assertTrace_dB(S12, -13, 5)
        self.assertTrace_dB(S21, -13, 5)
        
        # Reflection should be below -10dB (much lower for most frequencies)
        self.assertTrace_dB(S11, -100, 91)
        self.assertTrace_dB(S22, -100, 91)

