#!/usr/bin/env python3

import os
import binascii

FPGA_BITSTREAM = "FPGA/VNA/top.bin"
MCU_FW = ["Software/VNA_embedded/Debug/VNA_embedded.bin", "Software/VNA_embedded/Release/VNA_embedded.bin", "Software/VNA_embedded/build/VNA_embedded.bin"]

HEADER_SIZE = 24

f = open("combined.vnafw", "wb")
f.write(bytes("VNA!", 'utf-8'))

bitstream = open(FPGA_BITSTREAM, "rb")
latest_modification = 0
newest_mcu = ""
for path in MCU_FW:
    try:
        modified_time = os.path.getmtime(path)
        if modified_time >= latest_modification:
            latest_modification = modified_time
            newest_mcu = path
    except:
        pass
if latest_modification == 0:
    print("Couldn't find MCU firmware file")
    exit(-1)
print("Using "+newest_mcu+" as MCU firmware")
firmware = open(newest_mcu, "rb")

size_FPGA = os.path.getsize(FPGA_BITSTREAM)
size_MCU = os.path.getsize(newest_mcu)
print("Got FPGA bitstream of size "+str(size_FPGA))
print("Got MCU firmware of size "+str(size_MCU))

#Create header
# Start address of FPGA bitstream
f.write((HEADER_SIZE).to_bytes(4, byteorder='little'))
# Size of FPGA bitstream
f.write(size_FPGA.to_bytes(4, byteorder='little'))
# Start address of MCU firmware
f.write((HEADER_SIZE + size_FPGA).to_bytes(4, byteorder='little'))
# Size of MCU firmware
f.write(size_MCU.to_bytes(4, byteorder='little'))

# Calculate CRC
def CRC32_from_file(filename, initial_CRC):
	buf = open(filename,'rb').read()
	buf = (binascii.crc32(buf, initial_CRC) & 0xFFFFFFFF)
	return buf

print("Calculating CRC...", end="")
CRC = CRC32_from_file(FPGA_BITSTREAM, 0xFFFFFFFF)
CRC = CRC32_from_file(newest_mcu, CRC)
print(":"+hex(CRC))
f.write(CRC.to_bytes(4, byteorder='little'))

# Check that we used the correct header size
if f.tell() != HEADER_SIZE:
    print("Incorrect header size (defined as "+str(HEADER_SIZE)+" but actual header is of size "+str(f.tell())+")")
    exit(-1)

f.write(bitstream.read())
f.write(firmware.read())

if f.tell() % 256 != 0:
    padding = 256 - f.tell() % 256
    f.write(bytearray(padding))

print("Created combined firmware file of size "+str(f.tell()))

