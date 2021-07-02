#!/usr/bin/env python3

import time
from libreVNA import libreVNA

def MeasureNoiseAndPrint(condition):
    # Allow signal to settle
    time.sleep(1)

    p1min = int(vna.query("PORT1_MIN?"))
    p1max = int(vna.query("PORT1_MAX?"))
    p2min = int(vna.query("PORT2_MIN?"))
    p2max = int(vna.query("PORT2_MAX?"))
    refmin = int(vna.query("REF_MIN?"))
    refmax = int(vna.query("REF_MAX?"))

    p1pp = p1max - p1min
    p2pp = p2max - p2min
    refpp = refmax - refmin
    print(condition + "&{:d}&{:d}&{:d}\\\\".format(p1pp, p2pp, refpp))

# Create the control instance
vna = libreVNA('localhost', 19542)

# Make sure we are connecting to a device (just to be sure, with default settings the LibreVNA-GUI auto-connects)
vna.cmd(":DEV:CONN")
dev = vna.query(":DEV:CONN?")
if dev == "Not connected":
    print("Not connected to any device, aborting")
    exit(-1)

# Enter manual control mode
vna.cmd(":MAN:STA")

# By default everything is off, measure ADC noise
time.sleep(0.5)
vna.cmd("HSRC_FREQ 100000000")

print("\\begin{longtable}{p{.5\\textwidth} | p{.1\\textwidth} | p{.1\\textwidth} | p{.1\\textwidth}  }")
print("\\textbf{Condition} & \\textbf{Port 1} & \\textbf{Port 2} & \\textbf{Ref}\\\\")
print("\\hline")

MeasureNoiseAndPrint("Everything off")

vna.cmd("PORT1_EN TRUE")
vna.cmd("PORT2_EN TRUE")
vna.cmd("REF_EN TRUE")
MeasureNoiseAndPrint("Mixers and ADC driver enabled")

vna.cmd("LO2_EN TRUE")
MeasureNoiseAndPrint("LO2 enabled")

vna.cmd("LO1_CE TRUE")
vna.cmd("LO1_RFEN TRUE")
MeasureNoiseAndPrint("LO2 and LO1 enabled")

vna.cmd("AMP_EN TRUE")
MeasureNoiseAndPrint("LO2, LO1 and amplifier enabled")

vna.cmd("HSRC_CE TRUE")
vna.cmd("HSRC_RFEN TRUE")
MeasureNoiseAndPrint("LO2, LO1, amplifier and source at port 1 enabled")

vna.cmd("PORT_SW 2")
MeasureNoiseAndPrint("LO2, LO1, amplifier and source at port 2 enabled")

print("\\end{longtable}")

vna.cmd("STOP")

exit(0)

