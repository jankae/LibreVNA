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

# Capture live data as it is coming in. Stop acquisition for now
vna.cmd(":VNA:ACQ:STOP")

# switch to VNA mode, set up the sweep parameters
print("Setting up the sweep...")
vna.cmd(":DEV:MODE VNA")
vna.cmd(":VNA:SWEEP FREQUENCY")
vna.cmd(":VNA:STIM:LVL -10")
vna.cmd(":VNA:ACQ:IFBW 100")
vna.cmd(":VNA:ACQ:AVG 1")
vna.cmd(":VNA:ACQ:POINTS 501")
vna.cmd(":VNA:FREQuency:START 10000000")
vna.cmd(":VNA:FREQuency:STOP 6000000000")

sweepComplete = False


def callback(data):
    global sweepComplete
    print(data)
    if data["pointNum"] == 500:
        # this was the last point
        vna.remove_live_callback(19000, callback)
        sweepComplete = True


# Set up the connection for the live data
vna.add_live_callback(19000, callback)
print("Starting the sweep...")
vna.cmd(":VNA:ACQ:RUN")

while not sweepComplete:
    time.sleep(0.1)

print("Sweep complete")


