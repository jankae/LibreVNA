from tests.TestBase import TestBase
import time

class TestCalibration(TestBase):
    def cal_measure(self, number):
        self.vna.cmd(":VNA:CAL:MEAS "+str(number))
        # wait for the measurement to finish
        timeout = time.time() + 3
        while self.vna.query(":VNA:CAL:BUSY?") == "TRUE":
            if time.time() > timeout:
                raise AssertionError("Calibration measurement timed out")
            time.sleep(0.1)
    
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
        self.assertEqual(self.vna.query(":VNA:CAL:ACT SOLT_1"), "ERROR")

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
        self.assertEqual(self.vna.query(":VNA:CAL:ACT SOLT_1"), "")

        # SOLT_2 and SOLT_12 should still be unavailable
        self.assertEqual(self.vna.query(":VNA:CAL:ACT SOLT_2"), "ERROR")
        self.assertEqual(self.vna.query(":VNA:CAL:ACT SOLT_12"), "ERROR")

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
        self.assertEqual(self.vna.query(":VNA:CAL:ACT SOLT_1"), "")
        self.assertEqual(self.vna.query(":VNA:CAL:ACT SOLT_2"), "")
        
        # SOLT_12 should still be unavailable
        self.assertEqual(self.vna.query(":VNA:CAL:ACT SOLT_12"), "ERROR")

        # Take the final through measurement for SOLT_12
        self.vna.cmd(":VNA:CAL:ADD THROUGH")
        self.vna.cmd(":VNA:CAL:PORT 6 1 2")
        
        self.cal_measure(6)
        
        # SOLT_1, SOLT_2 and SOLT_12 should now be available
        self.assertEqual(self.vna.query(":VNA:CAL:ACT SOLT_1"), "")
        self.assertEqual(self.vna.query(":VNA:CAL:ACT SOLT_2"), "")
        self.assertEqual(self.vna.query(":VNA:CAL:ACT SOLT_12"), "")
