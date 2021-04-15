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

# Simple generator demo

# switch to generator
vna.cmd(":DEV:MODE GEN")

# set the output level
vna.cmd(":GEN:LVL -20")

# set initial frequency and enable port 1
print("Generating signal with 1GHz at port 1")
vna.cmd(":GEN:FREQ 1000000000")
vna.cmd(":GEN:PORT 1")

try:
    while True:
        time.sleep(2)
        print("Setting frequency to 1.5GHz")
        vna.cmd(":GEN:FREQ 1500000000")
        time.sleep(2)
        print("Setting frequency to 1.0GHz")
        vna.cmd(":GEN:FREQ 1000000000")
except KeyboardInterrupt:
    # turn off generator
    vna.cmd(":GEN:PORT 0")
    exit(0)

