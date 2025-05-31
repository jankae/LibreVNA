from tests.TestBase import TestBase
import time
import subprocess

class TestUpdate(TestBase):
    def test_Update(self):
        # first update: actually update to the firmware version we want to test
        self.vna.cmd("DEV:UPDATE ../../combined.vnafw", timeout=60)
        # second update: check that we still have a working firmware update with this version
        self.vna.cmd("DEV:UPDATE ../../combined.vnafw", timeout=60)
        
        reported = self.vna.query("DEV:INF:FWREVISION?")
        major = subprocess.check_output("grep -oP '(?<=FW_MAJOR=)[0-9]+' ../VNA_embedded/Makefile", shell=True).strip()
        minor = subprocess.check_output("grep -oP '(?<=FW_MINOR=)[0-9]+' ../VNA_embedded/Makefile", shell=True).strip()
        patch = subprocess.check_output("grep -oP '(?<=FW_PATCH=)[0-9]+' ../VNA_embedded/Makefile", shell=True).strip()
        expected = major.decode("utf-8") + "." + minor.decode("utf-8") + "." + patch.decode("utf-8")
        self.assertEqual(reported, expected)

