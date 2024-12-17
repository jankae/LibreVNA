import re
from tests.TestBase import TestBase
import time

float_re = re.compile(r'^[-+]?(\d+(\.\d*)?|\.\d+)([eE][-+]\d+)?$')
int_re = re.compile(r'^\d+$')
lowerq_re = re.compile('[a-z?]')

queries = [
    # Limits used to validate other parameters
    ("float", "DEVice:INFo:LIMits:MINFrequency?"),
    ("float", "DEVice:INFo:LIMits:MAXFrequency?"),
    ("float", "DEVice:INFo:LIMits:MINIFBW?"),
    ("float", "DEVice:INFo:LIMits:MAXIFBW?"),
    ("int",   "DEVice:INFo:LIMits:MAXPoints?"),
    ("float", "DEVice:INFo:LIMits:MINPOWer?"),
    ("float", "DEVice:INFo:LIMits:MAXPOWer?"),
    ("float", "DEVice:INFo:LIMits:MINRBW?"),
    ("float", "DEVice:INFo:LIMits:MAXRBW?"),

    # Settable parameters without query arguments
    ("str",   "DEVice:MODE?"),
    ("str",   "DEVice:REFerence:IN?"),
    ("str",   "DEVice:REFerence:OUT?"),
    ("float", "GENerator:FREQuency?"),
    ("float", "GENerator:LVL?"),
    ("int",   "GENerator:PORT?"),
    ("int",   "SA:ACQuisition:AVG?"),
    ("str",   "SA:ACQuisition:DETector?"),
    ("float", "SA:ACQuisition:RBW?"),
    ("bool",  "SA:ACQuisition:RUN?"),
    ("bool",  "SA:ACQuisition:SINGLE?"),
    ("str",   "SA:ACQuisition:WINDow?"),
    ("float", "SA:FREQuency:CENTer?"),
    ("float", "SA:FREQuency:SPAN?"),
    ("float", "SA:FREQuency:START?"),
    ("float", "SA:FREQuency:STOP?"),
    ("bool",  "SA:TRACKing:ENable?"),
    ("float", "SA:TRACKing:LVL?"),
    ("bool",  "SA:TRACKing:NORMalize:ENable?"),
    ("float", "SA:TRACKing:NORMalize:LVL?"),
    ("float", "SA:TRACKing:OFFset?"),
    ("int",   "SA:TRACKing:Port?"),
    ("int",   "VNA:ACQuisition:AVG?"),
    ("float", "VNA:ACQuisition:IFBW?"),
    ("int",   "VNA:ACQuisition:POINTS?"),
    ("bool",  "VNA:ACQuisition:RUN?"),
    ("bool",  "VNA:ACQuisition:SINGLE?"),
    ("str",   "VNA:CALibration:ACTivate?"),
    ("int",   "VNA:DEEMBedding:NUMber?"),
    ("float", "VNA:FREQuency:CENTer?"),
    ("float", "VNA:FREQuency:SPAN?"),
    ("float", "VNA:FREQuency:START?"),
    ("float", "VNA:FREQuency:STOP?"),
    ("float", "VNA:POWer:START?"),
    ("float", "VNA:POWer:STOP?"),
    ("float", "VNA:STIMulus:FREQuency?"),
    ("float", "VNA:STIMulus:LVL?"),
    ("str",   "VNA:SWEEP?"),
]


class TestRST(TestBase):
    def query_settings(self) -> dict:
        result = dict()
        for qtype, query in queries:
            resp = self.vna.query(query)
            if qtype == "float":
                self.assertTrue(float_re.match(resp),
                                f"Expected float from {query}; got: '{resp}'")
                value = float(resp)
            elif qtype == "int":
                self.assertTrue(int_re.match(resp),
                                f"Expected int from {query}; got: '{resp}'")
                value = int(resp)
            elif qtype == "bool":
                self.assertTrue(resp == "TRUE" or resp == "FALSE",
                                f"Expected bool from {query}; got: '{resp}'")
                value = True if resp == "TRUE" else False
            elif qtype == "str":
                value = resp
            else:
                assert False, "invalid type in table"

            query = re.sub(lowerq_re, r'', query)
            result[query] = value

        return result

    def validate_settings(self, settings):
        # Copy limits into local vars
        f_min = settings["DEV:INF:LIM:MINF"]
        f_max = settings["DEV:INF:LIM:MAXF"]
        ifbw_min = settings["DEV:INF:LIM:MINIFBW"]
        ifbw_max = settings["DEV:INF:LIM:MAXIFBW"]
        points_max = settings["DEV:INF:LIM:MAXP"]
        pwr_min = settings["DEV:INF:LIM:MINPOW"]
        pwr_max = settings["DEV:INF:LIM:MAXPOW"]
        rbw_min = settings["DEV:INF:LIM:MINRBW"]
        rbw_max = settings["DEV:INF:LIM:MAXRBW"]

        # Validate select settings
        self.assertEqual(settings["DEV:MODE"], "VNA")
        self.assertEqual(settings["DEV:REF:IN"], "INT")
        self.assertEqual(settings["DEV:REF:OUT"], "OFF")    # can't source pwr

        f = settings["GEN:FREQ"]
        self.assertGreaterEqual(f, f_min)
        self.assertLessEqual(f, f_max)

        pwr = settings["GEN:LVL"]
        self.assertGreaterEqual(pwr, pwr_min)
        self.assertLessEqual(pwr, pwr_max)

        self.assertEqual(settings["SA:ACQ:AVG"], 1)

        rbw = settings["SA:ACQ:RBW"]
        self.assertGreaterEqual(rbw, rbw_min)
        self.assertLessEqual(rbw, rbw_max)

        f_center = settings["SA:FREQ:CENT"]
        f_span = settings["SA:FREQ:SPAN"]
        f_start = settings["SA:FREQ:START"]
        f_stop = settings["SA:FREQ:STOP"]
        self.assertGreaterEqual(f_start, f_min)
        self.assertLessEqual(f_start, f_stop)
        self.assertLessEqual(f_stop, f_max)
        f_granularity = (f_max - f_min) / points_max
        self.assertTrue(abs(f_stop - f_start - f_span) < f_granularity)
        self.assertTrue(abs((f_start + f_stop) / 2 - f_center) < f_granularity)

        self.assertFalse(settings["SA:TRACK:EN"])
        pwr = settings["SA:TRACK:LVL"]
        self.assertGreaterEqual(pwr, pwr_min)
        self.assertLessEqual(pwr, pwr_max)

        pwr = settings["SA:TRACK:NORM:LVL"]
        self.assertGreaterEqual(pwr, pwr_min)
        self.assertLessEqual(pwr, pwr_max)

        self.assertGreaterEqual(settings["SA:TRACK:P"], 1)

        ifbw = settings["VNA:ACQ:IFBW"]
        self.assertGreaterEqual(ifbw, ifbw_min)
        self.assertLessEqual(ifbw, ifbw_max)

        points = settings["VNA:ACQ:POINTS"]
        self.assertGreaterEqual(points, 1)
        self.assertLessEqual(points, points_max)

        # TODO-check: In standard SCPI, the instrument does not source
        # power from its ports after *RST.  Automation program enables
        # the output only after completing its setup.
        #self.assertFalse(settings["VNA:ACQ:RUN"])   # can't source pwr
        self.assertEqual(settings["VNA:DEEMB:NUM"], 0)

        f_center = settings["VNA:FREQ:CENT"]
        f_span = settings["VNA:FREQ:SPAN"]
        f_start = settings["VNA:FREQ:START"]
        f_stop = settings["VNA:FREQ:STOP"]
        self.assertGreaterEqual(f_start, f_min)
        self.assertLessEqual(f_start, f_stop)
        self.assertLessEqual(f_stop, f_max)
        self.assertTrue(abs(f_stop - f_start - f_span) < f_granularity)
        self.assertTrue(abs((f_start + f_stop) / 2 - f_center) < f_granularity)

        pwr_start = settings["VNA:POW:START"]
        pwr_stop = settings["VNA:POW:STOP"]
        self.assertGreaterEqual(pwr_start, pwr_min)
        self.assertLess(pwr_start, pwr_stop)
        self.assertLessEqual(pwr_stop, pwr_max)

        f = settings["VNA:STIM:FREQ"]
        self.assertGreaterEqual(f, f_min)
        self.assertLessEqual(f, f_max)

        pwr = settings["VNA:STIM:LVL"]
        self.assertGreaterEqual(pwr, pwr_min)
        self.assertLessEqual(pwr, pwr_max)

        self.assertEqual(settings["VNA:SWEEP"], "FREQUENCY")

    def test_rst_basic(self):
        self.vna.cmd("*RST")
        settings = self.query_settings()
        self.validate_settings(settings)

    def test_rst_hard(self):
        self.vna.cmd("*RST")
        settings1 = self.query_settings()
        self.validate_settings(settings1)

        # Get limits.
        f_min = settings1["DEV:INF:LIM:MINF"]
        f_max = settings1["DEV:INF:LIM:MAXF"]
        f_1_3 = (2 * f_min + f_max) / 3
        f_1_2 = (f_min + f_max) / 2
        f_2_3 = (f_min + 2 * f_max) / 3
        ifbw_min = settings1["DEV:INF:LIM:MINIFBW"]
        ifbw_max = settings1["DEV:INF:LIM:MAXIFBW"]
        ifbw_1_2 = (ifbw_min + ifbw_max) / 2
        points_max = settings1["DEV:INF:LIM:MAXP"]
        pwr_min = settings1["DEV:INF:LIM:MINPOW"]
        pwr_max = settings1["DEV:INF:LIM:MAXPOW"]
        pwr_1_3 = (2 * pwr_min + pwr_max) / 3
        pwr_2_3 = (pwr_min + 2 * pwr_max) / 3
        pwr_1_2 = (pwr_min + pwr_max) / 2
        rbw_min = settings1["DEV:INF:LIM:MINRBW"]
        rbw_max = settings1["DEV:INF:LIM:MAXRBW"]
        rbw_1_2 = (rbw_max + rbw_max) / 2

        # Change parameters.
        self.vna.cmd("DEV:MODE SA")
        self.vna.cmd("DEV:REF:IN AUTO")
        self.vna.cmd("DEV:REF:OUT 10")
        self.vna.cmd(f"GEN:FREQ {f_1_2}")
        self.vna.cmd(f"GEN:LVL {pwr_1_2}")
        self.vna.cmd("GEN:PORT 2")
        self.vna.cmd("SA:ACQ:AVG 3")
        self.vna.cmd("SA:ACQ:DET -PEAK")
        self.vna.cmd(f"SA:ACQ:RBW {rbw_1_2}")
        self.vna.cmd("SA:ACQ:SINGLE TRUE")
        self.vna.cmd("SA:ACQ:WIND HANN")
        self.vna.cmd(f"SA:FREQ:START {f_1_3} STOP {f_2_3}")
        self.vna.cmd("SA:TRACK:EN TRUE")
        self.vna.cmd(f"SA:TRACK:LVL {pwr_1_2}")
        self.vna.cmd("SA:TRACK:NORM:EN TRUE")
        self.vna.cmd(f"SA:TRACK:NORM:LVL {pwr_1_3}")
        self.vna.cmd("SA:TRACK:OFF 1.0e+6;PORT 2")
        self.vna.cmd("VNA:ACQ:AVG 10")
        self.vna.cmd(f"VNA:ACQ:IFBW {ifbw_1_2}")
        self.vna.cmd(f"VNA:ACQ:POINTS 100")
        self.vna.cmd("VNA:ACQ:SINGLE TRUE")
        self.vna.cmd(f"VNA:FREQ:START {f_1_2};STOP {f_max}")
        self.vna.cmd(f"VNA:POW:START {pwr_1_3};STOP {pwr_2_3}")
        self.vna.cmd(f"VNA:STIM:FREQ {f_1_3}")
        self.vna.cmd(f"VNA:STIM:LVL {pwr_min}")
        self.vna.cmd("VNA:SWEEP POWER")

        # Reset and verify all settings revert.
        self.vna.cmd("*RST")
        
        settings2 = self.query_settings()
        for key, value in settings1.items():
            self.assertEqual(value, settings2[key])
