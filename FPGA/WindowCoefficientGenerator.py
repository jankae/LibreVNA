#!/usr/bin/env python3

import math
import numpy as np

# Adapt these constants to your window requirements
NUMBER_OF_COEFFICIENTS = 128
BITS_PER_COEFFICIENT = 16

# If set to true, the coefficients will be scaled to include the window amplitude correction factor.
# They will also be scaled to 1/8 to have enough headroom for the correction.
INCLUDE_AMPLITUDE_CORRECTION = True

# Don't change anything below this line

def bindigits(n, bits):
    s = bin(n & int("1"*bits, 2))[2:]
    return ("{0:0>%s}" % (bits)).format(s)

class Window:
    def __init__(self, name, function, correction):
        self.name = name
        self.function = function
        self.correction = correction
        self.sum = 0
        self.num_coeff = 0
    
    def StartFile(self):
        self.file = open(self.name+".dat", "w")

    def AddCoefficient(self, normalized_index):
        if not hasattr(self, 'file'):
            self.StartFile()
        value = self.function(normalized_index)
        self.sum = self.sum + value
        self.num_coeff = self.num_coeff + 1
        if INCLUDE_AMPLITUDE_CORRECTION:
            value = value * self.correction / 8.0
        value = int(value * (2 ** (BITS_PER_COEFFICIENT-1)))
        # prevent overflow
        if value >= (2 ** (BITS_PER_COEFFICIENT-1)):
            value = value - 1
        output = bindigits(value, BITS_PER_COEFFICIENT)
        self.file.write(output+"\n")

    def CorrectionFactor(self):
        return 1.0 / (self.sum / self.num_coeff)

def calc_hann(i):
    return math.sin(math.pi * i) ** 2   

def calc_kaiser(i):
    return np.kaiser(NUMBER_OF_COEFFICIENTS, 9.4)[int(i * NUMBER_OF_COEFFICIENTS)]

def calc_flattop(i):
    a0 = 0.21557895
    a1 = 0.41663158
    a2 = 0.277263158
    a3 = 0.083578947
    a4 = 0.006947368
    return a0 - a1 * math.cos(2*math.pi*i) + a2 * math.cos(4*math.pi*i) - a3 * math.cos(6*math.pi*i) + a4 * math.cos(8*math.pi*i)

WindowList = []
WindowList.append(Window("Hann", calc_hann, 2.00))
WindowList.append(Window("Kaiser", calc_kaiser, 2.50))
WindowList.append(Window("Flattop", calc_flattop, 4.64))


for i in range(NUMBER_OF_COEFFICIENTS):
    norm_i = (i+0.5) / NUMBER_OF_COEFFICIENTS
    for w in WindowList:
        w.AddCoefficient(norm_i)

for w in WindowList:
    print(w.CorrectionFactor())
    w.file.close();
