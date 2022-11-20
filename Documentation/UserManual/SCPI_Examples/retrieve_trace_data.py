#!/usr/bin/env python3

import time
from libreVNA import libreVNA

# Create the control instance
vna = libreVNA('localhost', 19542)

# Quick connection check (should print "LibreVNA-GUI")
print(vna.query("*IDN?"))

# Make sure we are connecting to a device (just to be sure, with default settings the LibreVNA-GUI auto-connects)
vna.cmd(":DEV:CONN")
dev = vna.query(":DEV:CONN?")
if dev == "Not connected":
    print("Not connected to any device, aborting")
    exit(-1)
else:
    print("Connected to "+dev)

# Simple trace data extraction

# switch to VNA mode, setup the sweep parameters
print("Setting up the sweep...")
vna.cmd(":DEV:MODE VNA")
vna.cmd(":VNA:SWEEP FREQUENCY")
vna.cmd(":VNA:STIM:LVL -10")
vna.cmd(":VNA:ACQ:IFBW 100")
vna.cmd(":VNA:ACQ:AVG 1")
vna.cmd(":VNA:ACQ:POINTS 501")
vna.cmd(":VNA:FREQuency:START 2000000000")
vna.cmd(":VNA:FREQuency:STOP 3500000000")

# wait for the sweep to finish
print("Waiting for the sweep to finish...")
while vna.query(":VNA:ACQ:FIN?") == "FALSE":
    time.sleep(0.1)

# grab the data of trace S11
print("Reading trace data...")
data = vna.query(":VNA:TRACE:DATA? S11")

# Returned data is just a string containing all the measurement points.
# Parsing the data returns a list containing frequency/complex tuples
S11 = vna.parse_VNA_trace_data(data)

for x in S11:
    print(x)

