import unittest

testmodules = [
    'tests.TestConnect',
    'tests.TestMode',
    'tests.TestVNASweep',
    'tests.TestCalibration',
    'tests.TestGenerator',
    'tests.TestSASweep',
    ]

suite = unittest.TestSuite()

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

