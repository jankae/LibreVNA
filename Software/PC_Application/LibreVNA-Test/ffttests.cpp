#include "ffttests.h"

#include "Traces/fftcomplex.h"

using namespace std;

fftTests::fftTests() {}

static bool compareComplexVectors(const vector<complex<double>> &v1, const vector<complex<double>> &v2) {
    if(v1.size() != v2.size()) {
        return false;
    }
    for(unsigned int i=0;i<v1.size();i++) {
        if(abs(v1[i].real() - v2[i].real()) > 1e-14) {
            return false;
        }
        if(abs(v1[i].imag() - v2[i].imag()) > 1e-14) {
            return false;
        }
    }
    return true;
}

void fftTests::fft()
{
    vector<complex<double>> data{1, 2, 3, 4, 5};
    Fft::transform(data, false);
    vector<complex<double>> expectedResult{15, complex(-2.5, 3.440954801177934), complex(-2.5, 0.812299240582265), complex(-2.5, -0.812299240582265), complex(-2.5, -3.440954801177934)};
    QVERIFY(compareComplexVectors(data, expectedResult));
}

void fftTests::fftAndIfft()
{
    vector<complex<double>> data{1, 2, 3, 4, 5};
    vector<complex<double>> expectedResult{1, 2, 3, 4, 5};
    Fft::transform(data, false);
    Fft::transform(data, true);
    for(auto &d : data) {
        d /= data.size();
    }
    QVERIFY(compareComplexVectors(data, expectedResult));
}

void fftTests::ifftAndFft()
{
    vector<complex<double>> data{1, 2, 3, 4, 5};
    vector<complex<double>> expectedResult{1, 2, 3, 4, 5};
    Fft::transform(data, true);
    for(auto &d : data) {
        d /= data.size();
    }
    Fft::transform(data, false);
    QVERIFY(compareComplexVectors(data, expectedResult));
}

void fftTests::fftAndIfftWithShift()
{

}
