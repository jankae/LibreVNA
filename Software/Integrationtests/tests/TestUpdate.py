from tests.TestBase import TestBase
import time
import subprocess

class TestUpdate(TestBase):
    def test_Update(self):
        self.vna.cmd("DEV:UPDATE ../../combined.vnafw", timeout=60)

        reported = self.vna.query("DEV:INF:FWREVISION?")
        major = subprocess.check_output("grep -oP '(?<=FW_MAJOR=)[0-9]+' ../VNA_embedded/Makefile", shell=True).strip()
        minor = subprocess.check_output("grep -oP '(?<=FW_MINOR=)[0-9]+' ../VNA_embedded/Makefile", shell=True).strip()
        patch = subprocess.check_output("grep -oP '(?<=FW_PATCH=)[0-9]+' ../VNA_embedded/Makefile", shell=True).strip()
        expected = major.decode("utf-8") + "." + minor.decode("utf-8") + "." + patch.decode("utf-8")
        self.assertEqual(reported, expected)

