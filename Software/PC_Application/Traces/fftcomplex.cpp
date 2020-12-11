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

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include "fftcomplex.h"
#include <algorithm>

using std::complex;
using std::size_t;
using std::uintmax_t;
using std::vector;


// Private function prototypes
static size_t reverseBits(size_t val, int width);


void Fft::transform(vector<complex<double> > &vec, bool inverse) {
    size_t n = vec.size();
    if (n == 0)
        return;
    else if ((n & (n - 1)) == 0)  // Is power of 2
        transformRadix2(vec, inverse);
    else  // More complicated algorithm for arbitrary sizes
        transformBluestein(vec, inverse);
}


void Fft::transformRadix2(vector<complex<double> > &vec, bool inverse) {
    // Length variables
    size_t n = vec.size();
    int levels = 0;  // Compute levels = floor(log2(n))
    for (size_t temp = n; temp > 1U; temp >>= 1)
        levels++;
    if (static_cast<size_t>(1U) << levels != n)
        throw std::domain_error("Length is not a power of 2");

    // Trigonometric table
    vector<complex<double> > expTable(n / 2);
    for (size_t i = 0; i < n / 2; i++)
        expTable[i] = std::polar(1.0, (inverse ? 2 : -2) * M_PI * i / n);

    // Bit-reversed addressing permutation
    for (size_t i = 0; i < n; i++) {
        size_t j = reverseBits(i, levels);
        if (j > i)
            std::swap(vec[i], vec[j]);
    }

    // Cooley-Tukey decimation-in-time radix-2 FFT
    for (size_t size = 2; size <= n; size *= 2) {
        size_t halfsize = size / 2;
        size_t tablestep = n / size;
        for (size_t i = 0; i < n; i += size) {
            for (size_t j = i, k = 0; j < i + halfsize; j++, k += tablestep) {
                complex<double> temp = vec[j + halfsize] * expTable[k];
                vec[j + halfsize] = vec[j] - temp;
                vec[j] += temp;
            }
        }
        if (size == n)  // Prevent overflow in 'size *= 2'
            break;
    }
}


void Fft::transformBluestein(vector<complex<double> > &vec, bool inverse) {
    // Find a power-of-2 convolution length m such that m >= n * 2 + 1
    size_t n = vec.size();
    size_t m = 1;
    while (m / 2 <= n) {
        if (m > SIZE_MAX / 2)
            throw std::length_error("Vector too large");
        m *= 2;
    }

    // Trigonometric table
    vector<complex<double> > expTable(n);
    for (size_t i = 0; i < n; i++) {
        uintmax_t temp = static_cast<uintmax_t>(i) * i;
        temp %= static_cast<uintmax_t>(n) * 2;
        double angle = (inverse ? M_PI : -M_PI) * temp / n;
        expTable[i] = std::polar(1.0, angle);
    }

    // Temporary vectors and preprocessing
    vector<complex<double> > avec(m);
    for (size_t i = 0; i < n; i++)
        avec[i] = vec[i] * expTable[i];
    vector<complex<double> > bvec(m);
    bvec[0] = expTable[0];
    for (size_t i = 1; i < n; i++)
        bvec[i] = bvec[m - i] = std::conj(expTable[i]);

    // Convolution
    vector<complex<double> > cvec(m);
    convolve(avec, bvec, cvec);

    // Postprocessing
    for (size_t i = 0; i < n; i++)
        vec[i] = cvec[i] * expTable[i];
}


void Fft::convolve(
        const vector<complex<double> > &xvec,
        const vector<complex<double> > &yvec,
        vector<complex<double> > &outvec) {

    size_t n = xvec.size();
    if (n != yvec.size() || n != outvec.size())
        throw std::domain_error("Mismatched lengths");
    vector<complex<double> > xv = xvec;
    vector<complex<double> > yv = yvec;
    transform(xv, false);
    transform(yv, false);
    for (size_t i = 0; i < n; i++)
        xv[i] *= yv[i];
    transform(xv, true);
    for (size_t i = 0; i < n; i++)  // Scaling (because this FFT implementation omits it)
        outvec[i] = xv[i] / static_cast<double>(n);
}


static size_t reverseBits(size_t val, int width) {
    size_t result = 0;
    for (int i = 0; i < width; i++, val >>= 1)
        result = (result << 1) | (val & 1U);
    return result;
}

void Fft::shift(std::vector<std::complex<double> > &vec, bool inverse)
{
    int rotate_len = vec.size() / 2;
    if((vec.size() & 0x01) != 0) {
        // odd size, behavior depends on whether this is an inverse shift
        if(!inverse) {
            rotate_len++;
        }
    }
    std::rotate(vec.begin(), vec.begin() + rotate_len, vec.end());
}
