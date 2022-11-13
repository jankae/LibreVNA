from tests.TestBase import TestBase

class TestConnect(TestBase):
    def test_connection(self):
        self.vna.cmd(":DEV:DISC")
        self.assertEqual(self.vna.query(":DEV:CONN?"), "Not connected")
        self.vna.cmd(":DEV:CONN")
        self.assertNotEqual(self.vna.query(":DEV:CONN?"), "Not connected")
        
    def test_list(self):
        list = self.vna.query(":DEV:LIST?")
        serials = list.split(',')
        self.assertEqual(len(serials), 1)
