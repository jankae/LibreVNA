#!/usr/bin/env python3

import math

# Adapt these constants to your window requirements
NUMBER_OF_COEFFICIENTS = 128
BITS_PER_COEFFICIENT = 16

# If set to true, the coefficients will be scaled to include the window amplitude correction factor.
# They will also be scaled to 1/8 to have enough headroom for the correction.
INCLUDE_AMPLITUDE_CORRECTION = True

# Don't change anything below this line

class Window:
    def __init__(self, name, function, correction):
        self.name = name
        self.function = function
        self.correction = correction
    
    def StartFile(self):
        self.file = open(self.name+".txt", "w")

    def AddCoefficient(self, normalized_index):
        if not hasattr(self, 'file'):
            self.StartFile()
        value = self.function(normalized_index)
        if INCLUDE_AMPLITUDE_CORRECTION:
            value = value * self.correction / 8.0
        value = int(value * (2 ** (BITS_PER_COEFFICIENT-1)))
        # prevent overflow
        if value >= (2 ** (BITS_PER_COEFFICIENT-1)):
            value = value - 1
        output = "{0:b}".format(value)
        self.file.write(output.zfill(BITS_PER_COEFFICIENT)+"\n")

def calc_hann(i):
    return math.sin(math.pi * i) ** 2   

WindowList = []
WindowList.append(Window("Hann", calc_hann, 2.0))

for i in range(NUMBER_OF_COEFFICIENTS):
    norm_i = (i+0.5) / NUMBER_OF_COEFFICIENTS
    for w in WindowList:
        w.AddCoefficient(norm_i)
