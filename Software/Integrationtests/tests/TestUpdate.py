from tests.TestBase import TestBase
import time

class TestUpdate(TestBase):
    def test_Update(self):
        self.vna.cmd("DEV:UPDATE ../../combined.vnafw", timeout=60)

