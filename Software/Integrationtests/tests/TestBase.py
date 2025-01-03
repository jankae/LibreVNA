import unittest
from tests.libreVNA import libreVNA as libreVNA
import tests.definitions as defs
import subprocess
import time
import select
from signal import SIGINT

class TestBase(unittest.TestCase):
    def setUp(self):
        self.gui = subprocess.Popen([defs.GUI_PATH, '-p', '19544', '--reset-preferences', '--no-gui', '-platform', 'offscreen'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        
        # wait for the SCPI server to become available
        timeout = time.time() + 3;
        poll_obj = select.poll()
        poll_obj.register(self.gui.stdout, select.POLLIN)
        while True:
            poll_result = poll_obj.poll(0)
            if poll_result:
                line = self.gui.stdout.readline().decode().strip()
                if "Listening on port 19544" in line:
                    break
            if time.time() >= timeout:
                self.tearDown()
                raise AssertionError("Timed out waiting for SCPI server")

        self.vna = libreVNA('localhost', 19544, timeout=4)
        try:
            self.vna.cmd("*CLS;:DEV:CONN")
        except Exception as e:
            self.tearDown()
            raise e
        if self.vna.query(":DEV:CONN?") == "Not connected":
            self.tearDown()
            raise AssertionError("Not connected")
               
    def tearDown(self):
        # make sure the GUI did not crash during the test
        self.assertEqual(self.gui.poll(), None)
        self.gui.send_signal(SIGINT)
        try:
            self.gui.wait(timeout = 3)
        except subprocess.TimeoutExpired:
            self.gui.kill()
            
        
