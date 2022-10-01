/*
 * Free FFT and convolution (C++)
 *
 * Copyright (c) 2020 Project Nayuki. (MIT License)
 * https://www.nayuki.io/page/free-small-fft-in-multiple-languages
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */

#pragma once

#include <complex>
#include <vector>

namespace Fft {

    // swap left/right halves of the vector, similar to matlabs fftshift/ifftshift
    void shift(std::vector<std::complex<double> > &vec, bool inverse);

    /*
     * Computes the discrete Fourier transform (DFT) of the given complex vector, storing the result back into the vector.
     * The vector can have any length. This is a wrapper function. The inverse transform does not perform scaling, so it is not a true inverse.
     */
    void transform(std::vector<std::complex<double> > &vec, bool inverse);


    /*
     * Computes the discrete Fourier transform (DFT) of the given complex vector, storing the result back into the vector.
     * The vector's length must be a power of 2. Uses the Cooley-Tukey decimation-in-time radix-2 algorithm.
     */
    void transformRadix2(std::vector<std::complex<double> > &vec, bool inverse);


    /*
     * Computes the discrete Fourier transform (DFT) of the given complex vector, storing the result back into the vector.
     * The vector can have any length. This requires the convolution function, which in turn requires the radix-2 FFT function.
     * Uses Bluestein's chirp z-transform algorithm.
     */
    void transformBluestein(std::vector<std::complex<double> > &vec, bool inverse);


    /*
     * Computes the circular convolution of the given complex vectors. Each vector's length must be the same.
     */
    void convolve(
        const std::vector<std::complex<double> > &xvec,
        const std::vector<std::complex<double> > &yvec,
        std::vector<std::complex<double> > &outvec);

}
