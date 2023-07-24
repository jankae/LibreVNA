#!/usr/bin/env python3

import time
from libreVNA import libreVNA

# Create the control instance
vna = libreVNA('localhost', 19542)

# Quick connection check (should print "LibreVNA-GUI")
print(vna.query("*IDN?"))

vna.cmd(":VNA:DEEMB:CLEAR")
vna.cmd(":VNA:DEEMB:NEW PORT_EXTENSION")
vna.cmd(":VNA:DEEMB:NEW 2XTHRU")
vna.cmd(":VNA:DEEMB:NEW MATCHING_NETWORK")
vna.cmd(":VNA:DEEMB:NEW IMPEDANCE_RENORMALIZATION")

print("Number of total deembedding options:")
num_options = int(vna.query(":VNA:DEEMB:NUM?"))
print(num_options)

for i in range(1, num_options+1):
	option_name = vna.query(":VNA:DEEMB:TYPE? "+str(i))
	print("Option "+str(i)+": "+option_name)
	
# edit port extension
vna.cmd(":VNA:DEEMB:1:PORT 2")
vna.cmd(":VNA:DEEMB:1:DELAY 0.00002")
vna.cmd(":VNA:DEEMB:1:DCLOSS 1")
vna.cmd(":VNA:DEEMB:1:LOSS 3")
vna.cmd(":VNA:DEEMB:1:FREQUENCY 5000000000")

vna.cmd(":VNA:DEEMB:3:PORT 3")
vna.cmd(":VNA:DEEMB:3:CLEAR")
vna.cmd(":VNA:DEEMB:3:ADD FALSE")
vna.cmd(":VNA:DEEMB:3:NEW ParallelC")
vna.cmd(":VNA:DEEMB:3:NEW SeriesR")
vna.cmd(":VNA:DEEMB:3:NEW ParallelL")
vna.cmd(":VNA:DEEMB:3:NEW SeriesL")
vna.cmd(":VNA:DEEMB:3:NEW touchstone_shunt")

vna.cmd(":VNA:DEEMB:3:1:VALUE 0.0001")
vna.cmd(":VNA:DEEMB:3:2:VALUE 0.00002")
vna.cmd(":VNA:DEEMB:3:3:VALUE 0.000003")
vna.cmd(":VNA:DEEMB:3:4:VALUE 0.000004")

vna.cmd(":VNA:DEEMB:3:5:FILE TEST.S2P")

vna.cmd(":VNA:DEEMB:4:IMPedance 75")

