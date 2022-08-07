#!/usr/bin/env python3

import math

# Adapt these constants to the step attenuator
BITS_OF_ATTENUATION = 7
LSB_ATTENUATOR_DB = 0.25
NUMBER_LINEAR_ATTENUATION_STEPS = 128

file = open("AMdepth.dat", "w")

def bindigits(n, bits):
    s = bin(n & int("1"*bits, 2))[2:]
    return ("{0:0>%s}" % (bits)).format(s)

for i in range(NUMBER_LINEAR_ATTENUATION_STEPS):
	# calculate percentage of attenuation
	percent = float(i) / (NUMBER_LINEAR_ATTENUATION_STEPS - 1)
	# calculate attenuation in dB
	if percent < 1:
		required_dB = -20*math.log10(1 - percent)
	else:
		required_dB = 999
	# round to attenuator steps
	attenuator_dB = round(required_dB / LSB_ATTENUATOR_DB) * LSB_ATTENUATOR_DB
	# convert to digital attenuator value
	attenuator_step = int(attenuator_dB / LSB_ATTENUATOR_DB)
	# constrain
	if attenuator_step < 0:
		attenuator_step = 0
	elif attenuator_step >= pow(2, BITS_OF_ATTENUATION):
		attenuator_step = pow(2, BITS_OF_ATTENUATION) - 1
	output = bindigits(attenuator_step, BITS_OF_ATTENUATION)
	file.write(output+"\n")
	
	# Calculate actual attenuation
	actual_dB = attenuator_step * LSB_ATTENUATOR_DB
	actual_percent = 1 - pow(10, -actual_dB/20)
	
	percent_desired = round(percent*100, 2)
	percent_actual = round(actual_percent*100, 2)
	
	print(f'Target modulation: {percent_desired:.2f}, target dB: {required_dB:.2f}, attenuator setting: {attenuator_step}, achieved dB: {actual_dB}, achieved modulation: {percent_actual:.2f}')

file.close()
