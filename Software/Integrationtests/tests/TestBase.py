import unittest
from tests.libreVNA import libreVNA as libreVNA
import tests.definitions as defs
import subprocess
import time
import select
import os
from signal import SIGINT

class TestBase(unittest.TestCase):
    def setUp(self):
        f = open("log.txt", "w")
        self.gui = subprocess.Popen([defs.GUI_PATH, '-p', '19544', '--reset-preferences', '--no-gui', '-platform', 'offscreen'], stdout=f, stderr=subprocess.STDOUT)
        
        # wait for the SCPI server to become available
        timeout = time.time() + 3;
        fread = open("log.txt", "r")
        while True:
            text = fread.read()
            if text and "Listening on port 19544" in text:
                break
            if time.time() >= timeout:
                self.tearDown()
                raise AssertionError("Timed out waiting for SCPI server")

        self.vna = libreVNA('localhost', 19544, timeout=4)
        try:
            self.vna.cmd("*CLS")
        except Exception as e:
            self.tearDown()
            raise e
        if self.vna.query(":DEV:CONN?") == "Not connected":
            self.tearDown()
            raise AssertionError("Not connected")


    def tearDown(self):
        if hasattr(self._outcome, 'errors'):
            # Python 3.4 - 3.10  (These two methods have no side effects)
            result = self.defaultTestResult()
            self._feedErrorsToResult(result, self._outcome.errors)
        else:
            # Python 3.11+
            result = self._outcome.result
        ok = all(test != self for test, text in result.errors + result.failures)

        if not ok:
            try:
                # Try to grab the packet log and save it
                packetlog = self.vna.query(":DEV:PACKETLOG?", timeout=5)
                f = open("packetlog_"+time.strftime("%Y%m%d%H%M%S.vnalog"), "w")
                f.write(packetlog)
                f.close()
            except:
                pass

        # make sure the GUI did not crash during the test
        crashed = self.gui.poll() is not None
        self.gui.send_signal(SIGINT)
        try:
            self.gui.wait(timeout = 3)
        except subprocess.TimeoutExpired:
            self.gui.kill()
            

        if ok and not crashed:
            # remove log file
            os.remove("log.txt")
        else:
            os.rename("log.txt", "errorlog_"+time.strftime("%Y%m%d%H%M%S.txt"))

        if(crashed):
            raise Exception("GUI crashed")

        
