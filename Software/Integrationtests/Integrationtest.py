import unittest
import glob, os

testmodules = [
    'tests.TestUpdate',     # Must go first because it updates the connected VNA to the firwmare which should be tested
    'tests.TestConnect',
    'tests.TestStatusRegisters',
    'tests.TestMode',
    'tests.TestTemperature',
    'tests.TestSync',
    'tests.TestVNASweep',
    'tests.TestCalibration',
    'tests.TestGenerator',
    'tests.TestSASweep',
    'tests.TestRST',
    ]

suite = unittest.TestSuite()

# Clean up potential error logs from previous test runs
for f in glob.glob("errorlog_*"):
    os.remove(f)
for f in glob.glob("packetlog_*"):
    os.remove(f)

for t in testmodules:
    try:
        # If the module defines a suite() function, call it to get the suite.
        mod = __import__(t, globals(), locals(), ['suite'])
        suitefn = getattr(mod, 'suite')
        suite.addTest(suitefn())
    except (ImportError, AttributeError):
        # else, just load all the test cases from the module.
        suite.addTest(unittest.defaultTestLoader.loadTestsFromName(t))

result = unittest.TextTestRunner(verbosity=2).run(suite)
exit(int(not result.wasSuccessful()))

