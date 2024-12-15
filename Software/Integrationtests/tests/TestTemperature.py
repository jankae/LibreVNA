from tests.TestBase import TestBase

class TestConnect(TestBase):
    def test_temperature(self):
        res = self.vna.query(":DEV:INF:TEMP?")
        self.assertEqual(len(res.split("/")), 3)
        
        

