#include "twothru.h"
#include "CustomWidgets/informationbox.h"
#include "ui_twothrudialog.h"
#include "Traces/fftcomplex.h"
#include <QDebug>
#include "unit.h"

using namespace std;

TwoThru::TwoThru()
{
    Z0 = 50.0;
}

void TwoThru::transformDatapoint(Protocol::Datapoint &p)
{
    // correct measurement
    if(points.size() > 0) {
        auto S11 = complex<double>(p.real_S11, p.imag_S11);
        auto S12 = complex<double>(p.real_S12, p.imag_S12);
        auto S21 = complex<double>(p.real_S21, p.imag_S21);
        auto S22 = complex<double>(p.real_S22, p.imag_S22);
        Sparam S(S11, S12, S21, S22);
        Tparam meas(S);

        Tparam inv1, inv2;
        if(p.frequency < points.front().freq) {
            inv1 = points.front().inverseP1;
            inv2 = points.front().inverseP2;
        } else if(p.frequency > points.back().freq) {
            inv1 = points.back().inverseP1;
            inv2 = points.back().inverseP2;
        } else {
            // find correct measurement point
            auto point = lower_bound(points.begin(), points.end(), p.frequency, [](Point p, uint64_t freq) -> bool {
                return p.freq < freq;
            });
            if(point->freq == p.frequency) {
                inv1 = point->inverseP1;
                inv2 = point->inverseP2;
            } else {
                // need to interpolate
                auto high = point;
                point--;
                auto low = point;
                double alpha = (p.frequency - low->freq) / (high->freq - low->freq);
                inv1 = low->inverseP1 * (1 - alpha) + high->inverseP1 * alpha;
                inv2 = low->inverseP2 * (1 - alpha) + high->inverseP2 * alpha;
            }
        }
        // perform correction
        Tparam corrected = inv1*meas*inv2;
        // transform back into S parameters
        S = Sparam(corrected);
        p.real_S11 = real(S.m11);
        p.imag_S11 = imag(S.m11);
        p.real_S12 = real(S.m12);
        p.imag_S12 = imag(S.m12);
        p.real_S21 = real(S.m21);
        p.imag_S21 = imag(S.m21);
        p.real_S22 = real(S.m22);
        p.imag_S22 = imag(S.m22);
    }
}

void TwoThru::startMeasurement()
{
    emit triggerMeasurement();
}

void TwoThru::updateGUI()
{
    if(measurements2xthru.size() > 0) {
        ui->l2xthru->setText(QString::number(measurements2xthru.size())+" points from "
                             +Unit::ToString(measurements2xthru.front().frequency, "Hz", " kMG", 4)+" to "
                             +Unit::ToString(measurements2xthru.back().frequency, "Hz", " kMG", 4));
    } else {
        ui->l2xthru->setText("Not available");
    }

    if(measurementsDUT.size() > 0) {
        ui->lDUT->setText(QString::number(measurementsDUT.size())+" points from "
                             +Unit::ToString(measurementsDUT.front().frequency, "Hz", " kMG", 4)+" to "
                             +Unit::ToString(measurementsDUT.back().frequency, "Hz", " kMG", 4));
    } else {
        ui->lDUT->setText("Not available");
    }

    if(points.size() > 0) {
        ui->lPoints->setText(QString::number(points.size())+" points from "
                             +Unit::ToString(points.front().freq, "Hz", " kMG", 4)+" to "
                             +Unit::ToString(points.back().freq, "Hz", " kMG", 4));
    } else {
        ui->lPoints->setText("No values calculated, not de-embedding");
    }

    if (measurementsDUT.size() > 0 && measurements2xthru.size() > 0) {
        // correction using both measurements is available
        ui->Z0->setEnabled(true);
        ui->bCalc->setEnabled(true);
    } else if(measurements2xthru.size() > 0) {
        // simpler correction using only 2xthru measurement available
        ui->Z0->setEnabled(false);
        ui->bCalc->setEnabled(true);
    } else {
        // no correction available
        ui->Z0->setEnabled(false);
        ui->bCalc->setEnabled(false);
    }
}

void TwoThru::measurementCompleted(std::vector<Protocol::Datapoint> m)
{
    if (measuring2xthru) {
        measurements2xthru = m;
    } else if(measuringDUT) {
        measurementsDUT = m;
    }
    updateGUI();
}

void TwoThru::edit()
{
    auto dialog = new QDialog();
    ui = new Ui::TwoThruDialog();
    ui->setupUi(dialog);
    ui->Z0->setUnit("Ω");
    ui->Z0->setPrecision(4);
    ui->Z0->setValue(Z0);

    // choice of Z0 does not seem to make any difference, hide from user
    ui->Z0->setVisible(false);
    ui->lZ0->setVisible(false);

    connect(ui->bMeasure, &QPushButton::clicked, [=](){
        measuringDUT = false;
        measuring2xthru = true;
        startMeasurement();
    });

    connect(ui->bMeasureDUT, &QPushButton::clicked, [=](){
        measuringDUT = true;
        measuring2xthru = false;
        startMeasurement();
    });

    connect(ui->bClear, &QPushButton::clicked, [=](){
        measurements2xthru.clear();
        updateGUI();
    });

    connect(ui->bClearDUT, &QPushButton::clicked, [=](){
        measurementsDUT.clear();
        updateGUI();
    });

    connect(ui->bCalc, &QPushButton::clicked, [=](){
        ui->lPoints->setText("Calculating...");
        qApp->processEvents();
        if(measurementsDUT.size() > 0) {
            points = calculateErrorBoxes(measurements2xthru, measurementsDUT, ui->Z0->value());
        } else {
            points = calculateErrorBoxes(measurements2xthru);
        }
        updateGUI();
    });

    updateGUI();

    dialog->show();
}

nlohmann::json TwoThru::toJSON()
{
    nlohmann::json j;
    for(auto p : points) {
        nlohmann::json jp;
        jp["frequency"] = p.freq;
        jp["p1_11_r"] = p.inverseP1.m11.real();
        jp["p1_11_i"] = p.inverseP1.m11.imag();
        jp["p1_12_r"] = p.inverseP1.m12.real();
        jp["p1_12_i"] = p.inverseP1.m12.imag();
        jp["p1_21_r"] = p.inverseP1.m21.real();
        jp["p1_21_i"] = p.inverseP1.m21.imag();
        jp["p1_22_r"] = p.inverseP1.m22.real();
        jp["p1_22_i"] = p.inverseP1.m22.imag();
        jp["p2_11_r"] = p.inverseP2.m11.real();
        jp["p2_11_i"] = p.inverseP2.m11.imag();
        jp["p2_12_r"] = p.inverseP2.m12.real();
        jp["p2_12_i"] = p.inverseP2.m12.imag();
        jp["p2_21_r"] = p.inverseP2.m21.real();
        jp["p2_21_i"] = p.inverseP2.m21.imag();
        jp["p2_22_r"] = p.inverseP2.m22.real();
        jp["p2_22_i"] = p.inverseP2.m22.imag();
        j.push_back(jp);
    }
    return j;
}

void TwoThru::fromJSON(nlohmann::json j)
{
    points.clear();
    for(auto jp : j) {
        Point p;
        p.freq = jp.value("frequency", 0.0);
        p.inverseP1.m11 = complex<double>(jp.value("p1_11_r", 0.0), jp.value("p1_11_i", 0.0));
        p.inverseP1.m12 = complex<double>(jp.value("p1_12_r", 0.0), jp.value("p1_12_i", 0.0));
        p.inverseP1.m21 = complex<double>(jp.value("p1_21_r", 0.0), jp.value("p1_21_i", 0.0));
        p.inverseP1.m22 = complex<double>(jp.value("p1_22_r", 0.0), jp.value("p1_22_i", 0.0));
        p.inverseP2.m11 = complex<double>(jp.value("p2_11_r", 0.0), jp.value("p2_11_i", 0.0));
        p.inverseP2.m12 = complex<double>(jp.value("p2_12_r", 0.0), jp.value("p2_12_i", 0.0));
        p.inverseP2.m21 = complex<double>(jp.value("p2_21_r", 0.0), jp.value("p2_21_i", 0.0));
        p.inverseP2.m22 = complex<double>(jp.value("p2_22_r", 0.0), jp.value("p2_22_i", 0.0));
        points.push_back(p);
    }
}

std::vector<TwoThru::Point> TwoThru::calculateErrorBoxes(std::vector<Protocol::Datapoint> data_2xthru)
{
    // calculate error boxes, see https://www.freelists.org/post/si-list/IEEE-P370-Opensource-Deembedding-MATLAB-functions
    // create vectors of S parameters
    vector<complex<double>> S11, S12, S21, S22;
    vector<double> f;

    // remove DC point if present
    if(data_2xthru[0].frequency == 0) {
        data_2xthru.erase(data_2xthru.begin());
    }

    data_2xthru = interpolateEvenFrequencySteps(data_2xthru);

    for(auto m : data_2xthru) {
        if(m.frequency == 0) {
            // ignore possible DC point
            continue;
        }
        S11.push_back(complex<double>(m.real_S11, m.imag_S11));
        S12.push_back(complex<double>(m.real_S12, m.imag_S12));
        S21.push_back(complex<double>(m.real_S21, m.imag_S21));
        S22.push_back(complex<double>(m.real_S22, m.imag_S22));
        f.push_back(m.frequency);
    }
    auto n = f.size();

    auto makeSymmetric = [](const vector<complex<double>> &in) -> vector<complex<double>> {
        auto abs_DC = 2.0 * abs(in[0]) - abs(in[1]);
        auto phase_DC = 2.0 * arg(in[0]) - arg(in[1]);
        auto DC = polar(abs_DC, phase_DC);
        vector<complex<double>> ret;
        ret.push_back(DC);
        // add non-symmetric part
        ret.insert(ret.end(), in.begin(), in.end());
        // add flipped complex conjugate values
        for(auto it = in.rbegin(); it != in.rend(); it++) {
            ret.push_back(conj(*it));
        }
        return ret;
    };

    auto makeRealAndScale = [](vector<complex<double>> &in) {
        for(unsigned int i=0;i<in.size();i++) {
            in[i] = real(in[i]) / in.size();
        }
    };

    // S parameter error boxes
    vector<Sparam> data_side1, data_side2;

    {
        auto p112x = makeSymmetric(S11);
        auto p212x = makeSymmetric(S21);

        // transform into time domain and calculate step responses
        auto t112x = p112x;
        Fft::transform(t112x, true);
        makeRealAndScale(t112x);
        Fft::shift(t112x, false);
        partial_sum(t112x.begin(), t112x.end(), t112x.begin());
        auto t212x = p212x;
        Fft::transform(t212x, true);
        makeRealAndScale(t212x);
        Fft::shift(t212x, false);
        partial_sum(t212x.begin(), t212x.end(), t212x.begin());

        // find the midpoint of the trace
        double threshold = 0.5*real(t212x.back());
        auto mid = lower_bound(t212x.begin(), t212x.end(), threshold, [](complex<double> p, double c) -> bool {
                return real(p) < c;
        }) - t212x.begin();

        // mask step response
        vector<complex<double>> t111xStep(2*n + 1, 0.0);
        copy(t112x.begin() + n, t112x.begin() + mid, t111xStep.begin() + n);
        Fft::shift(t111xStep, true);
        // create impulse response from masked step response
        adjacent_difference(t111xStep.begin(), t111xStep.end(), t111xStep.begin());
        Fft::transform(t111xStep, false);
        auto &p111x = t111xStep;

        // calculate p221x and p211x
        vector<complex<double>> p221x;
        vector<complex<double>> p211x;
        double k = 1.0;
        complex<double> test, last_test;
        for(unsigned int i=0;i<p112x.size();i++) {
            p221x.push_back((p112x[i]-p111x[i])/p212x[i]);
            test = sqrt(p212x[i]*(1.0-p221x[i]*p221x[i]));
            if(i > 0) {
                // according to the octave script, the next line should be if(arg(test) - arg(last_test) > 0)
                // but that leads to 180° degree phase shift and also doesn't make much sense:
                // we want to figure out the correct sign for the root so that no phase jumps occur. The
                // phase difference from one to the next point is allowed to be positive, it just should be smaller
                // than PI/2 (otherwise we got the wrong sign for the root)
                if(abs(arg(test) - arg(last_test)) > M_PI / 2) {
                    k = -k;
                }
                qDebug() << "angle at point" << i << "(" << arg(test) - arg(last_test) << ")";
            }
            last_test = test;
            p211x.push_back(k*test);
        }

        // create S parameter errorbox
        for(unsigned int i=1;i<=n;i++) {
            data_side1.push_back(Sparam(p111x[i], p211x[i], p211x[i], p221x[i]));
        }
    }

    // same thing for error box 2. Variable names get a bit confusing because they are viewed from port 2 (S22 is now called p112x, ...).
    // All variable names follow https://gitlab.com/IEEE-SA/ElecChar/P370/-/blob/master/TG1/IEEEP3702xThru_Octave.m
    {
        auto p112x = makeSymmetric(S22);
        auto p212x = makeSymmetric(S12);

        // transform into time domain and calculate step responses
        auto t112x = p112x;
        Fft::transform(t112x, true);
        makeRealAndScale(t112x);
        Fft::shift(t112x, false);
        partial_sum(t112x.begin(), t112x.end(), t112x.begin());
        auto t212x = p212x;
        Fft::transform(t212x, true);
        makeRealAndScale(t212x);
        Fft::shift(t212x, false);
        partial_sum(t212x.begin(), t212x.end(), t212x.begin());

        // find the midpoint of the trace
        double threshold = 0.5*real(t212x.back());
        auto mid = lower_bound(t212x.begin(), t212x.end(), threshold, [](complex<double> p, double c) -> bool {
                return real(p) < c;
        }) - t212x.begin();

        // mask step response
        vector<complex<double>> t111xStep(2*n + 1, 0.0);
        copy(t112x.begin() + n, t112x.begin() + mid, t111xStep.begin() + n);
        Fft::shift(t111xStep, true);
        // create impulse response from masked step response
        adjacent_difference(t111xStep.begin(), t111xStep.end(), t111xStep.begin());
        Fft::transform(t111xStep, false);
        auto &p111x = t111xStep;

        // calculate p221x and p211x
        vector<complex<double>> p221x;
        vector<complex<double>> p211x;
        double k = 1.0;
        complex<double> test, last_test;
        for(unsigned int i=0;i<p112x.size();i++) {
            p221x.push_back((p112x[i]-p111x[i])/p212x[i]);
            test = sqrt(p212x[i]*(1.0-p221x[i]*p221x[i]));
            if(i > 0) {
                if(abs(arg(test) - arg(last_test)) > M_PI / 2) {
                    k = -k;
                }
            }
            last_test = test;
            p211x.push_back(k*test);
        }

        // create S parameter errorbox
        for(unsigned int i=1;i<=n;i++) {
            data_side2.push_back(Sparam(data_side1[i-1].m22, p211x[i], p211x[i], p111x[i]));
            data_side1[i-1].m22 = p221x[i];
        }
    }

    // got the error boxes, convert to T parameters and invert
    vector<Point> ret;
    for(unsigned int i=0;i<n;i++) {
        Point p;
        p.freq = f[i];
        p.inverseP1 = Tparam(data_side1[i]).inverse();
        p.inverseP2 = Tparam(data_side2[i]).inverse();
        ret.push_back(p);
    }
    return ret;
}

std::vector<TwoThru::Point> TwoThru::calculateErrorBoxes(std::vector<Protocol::Datapoint> data_2xthru, std::vector<Protocol::Datapoint> data_fix_dut_fix, double z0)
{
    vector<Point> ret;

    if(data_2xthru.size() != data_fix_dut_fix.size()) {
        InformationBox::ShowMessageBlocking("Unable to calculate", "The DUT and 2xthru measurements do not have the same amount of points, calculation not possible");
        return ret;
    }

    // check if frequencies are the same (measurements must be taken with identical span settings)
    for(unsigned int i=0;i<data_2xthru.size();i++) {
        if(abs((long int)data_2xthru[i].frequency - (long int)data_fix_dut_fix[i].frequency) > (double) data_2xthru[i].frequency / 1e9) {
            InformationBox::ShowMessageBlocking("Unable to calculate", "The DUT and 2xthru measurements do not have identical frequencies for all points, calculation not possible");
            return ret;
        }
    }

    data_2xthru = interpolateEvenFrequencySteps(data_2xthru);
    data_fix_dut_fix = interpolateEvenFrequencySteps(data_fix_dut_fix);

    // Variable names and order of calulation follows https://gitlab.com/IEEE-SA/ElecChar/P370/-/blob/master/TG1/IEEEP370Zc2xThru_Octave.m as close as possible
    vector<Sparam> p;
    vector<double> f;
    for(auto d : data_2xthru) {
        p.push_back(Sparam(complex<double>(d.real_S11, d.imag_S11),
                           complex<double>(d.real_S12, d.imag_S12),
                           complex<double>(d.real_S21, d.imag_S21),
                           complex<double>(d.real_S22, d.imag_S22)));
        f.push_back(d.frequency);
    }
    auto data_2xthru_Sparam = p;
    vector<Sparam> data_fix_dut_fix_Sparam;
    for(auto d : data_fix_dut_fix) {
        data_fix_dut_fix_Sparam.push_back(Sparam(complex<double>(d.real_S11, d.imag_S11),
                           complex<double>(d.real_S12, d.imag_S12),
                           complex<double>(d.real_S21, d.imag_S21),
                           complex<double>(d.real_S22, d.imag_S22)));
    }

    // grabbing S21
    vector<complex<double>> s212x;
    for(auto s : p) {
        s212x.push_back(s.m21);
    }

    // get the attenuation and phase constant per length
    vector<complex<double>> gamma;
    double last_angle = 0.0;
    for(auto s : s212x) {
        // unwrap phase
        double angle = arg(s);
        while(angle - last_angle > M_PI) {
            angle -= 2 * M_PI;
        }
        while(angle - last_angle < -M_PI) {
            angle += 2 * M_PI;
        }
        last_angle = angle;
        double beta_per_length = -angle;
        double alpha_per_length = 20 * log10(abs(s))/-8.686;

        // assume no bandwidth limit (==0)
        gamma.push_back(complex<double>(alpha_per_length, beta_per_length));
    }

    // helper function lambdas
    auto makeSymmetric = [](const vector<complex<double>> &in) -> vector<complex<double>> {
        auto ret = in;
        for(auto it = in.rbegin();it != in.rend();it++) {
            ret.push_back(conj(*it));
        }
        // went one step too far, remove the DC point from the symmetric data
        ret.pop_back();
        return ret;
    };

    auto makeRealAndScale = [](vector<complex<double>> &in) {
        for(unsigned int i=0;i<in.size();i++) {
            in[i] = real(in[i]) / in.size();
        }
    };

    auto DC2 = [=](const vector<complex<double>> &s, const vector<double> &f) -> complex<double> {
        auto simple_filter = [](const vector<double> &f, double f0) -> vector<complex<double>> {
            vector<complex<double>> ret;
            for(auto v : f) {
                ret.push_back(1.0/complex<double>(1.0, pow(v/f0, 4)));
            }
            return ret;
        };

        complex<double> DCpoint = 0.002; // seed for the algorithm
        double err = 1; // error seed
        double allowedError = 1e-10; // allowable error
        long cnt = 0;
        auto df = f[1] - f[0];
        auto n = f.size();
        unsigned int ts = round((-3e-9) / ((2.0/df)/(n*2+1)) + (n*2+1)/2);
        auto Hr = simple_filter(f, f.back()/2);
        while (err > allowedError) {
            vector<complex<double>> f1;
            f1.push_back(DCpoint);
            for(unsigned int i=0;i<n;i++) {
                f1.push_back(s[i] * Hr[i]);
            }
            auto h1 = makeSymmetric(f1);
            Fft::transform(h1, true);
            makeRealAndScale(h1);
            Fft::shift(h1, false);
            partial_sum(h1.begin(), h1.end(), h1.begin());

            vector<complex<double>> f2;
            f2.push_back(DCpoint+0.001);
            for(unsigned int i=0;i<n;i++) {
                f2.push_back(s[i] * Hr[i]);
            }
            auto h2 = makeSymmetric(f2);
            Fft::transform(h2, true);
            makeRealAndScale(h2);
            Fft::shift(h2, false);
            partial_sum(h2.begin(), h2.end(), h2.begin());

            auto m = (h2[ts]-h1[ts])/0.001;
            auto b = h1[ts] - m*DCpoint;
            DCpoint = (0.0 - b) / m;
            err = abs(h1[ts] - 0.0);
            cnt++;
        }

        return DCpoint;
    };

    auto makeTL = [](const vector<complex<double>> &gamma, double l, complex<double> zLine, complex<double> z0) -> vector<Sparam> {
        vector<Sparam> ret;
        for(auto g : gamma) {
            auto s11 = ((zLine*zLine-z0*z0)*sinh(g*l))/((zLine*zLine+z0*z0)*sinh(g*l)+2.0*z0*zLine*cosh(g*l));
            auto s21 = (2.0*z0*zLine)/((zLine*zLine + z0*z0)*sinh(g*l)+2.0*z0*zLine*cosh(g*l));
            ret.push_back(Sparam(s11, s21, s21, s11));
        }
        return ret;
    };

    auto hybrid = [](const vector<Sparam> &errorbox, const vector<Sparam> &data_2xthru, const vector<double> &freq_2xthru) -> vector<Sparam> {
        // taking the errorbox created by peeling and using it only for e00 and e11

        // grab s11 and s22 of errorbox model
        vector<complex<double>> s111x, s221x;
        for(auto s : errorbox) {
            s111x.push_back(s.m11);
            s221x.push_back(s.m22);
        }

        // grab s21 of the 2x thru measurement
        vector<complex<double>> s212x;
        for(auto s : data_2xthru) {
            s212x.push_back(s.m21);
        }
        auto f = freq_2xthru;

        double k = 1.0;
        complex<double> test, last_test;
        vector<complex<double>> s211x;
        vector<Sparam> ret;
        for(unsigned int i=0;i<f.size();i++) {
            test = sqrt(s212x[i]*(1.0-s221x[i]*s221x[i]));
            if(i > 0) {
                if(abs(arg(test) - arg(last_test)) > M_PI / 2) {
                    k = -k;
                }
            }
            last_test = test;
            s211x.push_back(k*test);

            // create the error box and make the s-parameter block
            ret.push_back(Sparam(s111x[i], s211x[i], s211x[i], s221x[i]));
        }
        return ret;
    };

    auto makeErrorbox = [=](vector<Sparam> data_dut, const vector<Sparam> &data_2xthru, const vector<double> &freq_2xthru, const vector<complex<double>> &gamma, complex<double> z0) -> vector<Sparam> {
        auto f = freq_2xthru;
        auto n = f.size();

        vector<complex<double>> s212x;
        // add the DC point
        s212x.push_back(1.0);
        for(auto p : data_2xthru) {
            s212x.push_back(p.m21);
        }
        // extract the mid point from the 2x thru
        auto t212x = makeSymmetric(s212x);
        Fft::transform(t212x, true);
        makeRealAndScale(t212x);
        auto x = max_element(t212x.begin(), t212x.end(), [](complex<double> a, complex<double> b) -> bool {
            return abs(a) < abs(b);
        }) - t212x.begin() + 1;

        // define the relative length
        double l = 1.0/(2*x);

        // peel away the fixture and create the errorbox

        // create the errorbox seed (a perfect transmission line with no delay)
        vector<ABCDparam> abcd_errorbox(n, ABCDparam(Sparam(0.0, 1.0, 1.0, 0.0), z0));


        for(unsigned int i=0;i<x;i++) {
            // INPUTS: data_dut, f, abcd_errorbox, n

            // define the fixture-dut-fixture S-parameters
            vector<complex<double>> s_dut;
            for(auto s : data_dut) {
                s_dut.push_back(s.m11);
            }

            // define the point for extraction
            s_dut.insert(s_dut.begin(), DC2(s_dut, f));
            auto dc11 = makeSymmetric(s_dut);
            Fft::transform(dc11, true);
            makeRealAndScale(dc11);
            Fft::shift(dc11, false);
            partial_sum(dc11.begin(), dc11.end(), dc11.begin());
            auto t11dutStep = dc11;
            vector<complex<double>> z11dutStep;
            for(auto s : t11dutStep) {
                z11dutStep.push_back(-z0 * (s+1.0)/(s-1.0));
            }
            Fft::shift(z11dutStep, true);
            auto zLine = z11dutStep;

            // create the TL
            auto TL = makeTL(gamma, l, zLine[0], z0);

            for(unsigned int i=0;i<n;i++) {
                // peel away the the TL
                auto abcd_TL = ABCDparam(TL[i], z0);
                auto abcd_dut = ABCDparam(data_dut[i], z0);
                abcd_dut = abcd_TL.inverse() * abcd_dut;
                data_dut[i] = Sparam(abcd_dut, z0);
                // add to the errorbox
                abcd_errorbox[i] = abcd_errorbox[i] * abcd_TL;
            }
        }
        vector<Sparam> errorbox;
        for(auto abcd : abcd_errorbox) {
            errorbox.push_back(Sparam(abcd, z0));
        }
        return hybrid(errorbox, data_2xthru, f);
    };

    // make the first error box
    auto data_side1 = makeErrorbox(data_fix_dut_fix_Sparam, data_2xthru_Sparam, f, gamma, z0);

    // reverse the port order of fixture-dut-fixture and 2x thru
    vector<Sparam> data_fix_dut_fix_reversed;
    for(auto s : data_fix_dut_fix_Sparam) {
        data_fix_dut_fix_reversed.push_back(Sparam(s.m22, s.m21, s.m12, s.m11));
    }
    vector<Sparam> data_2xthru_reversed;
    for(auto s : data_2xthru_Sparam) {
        data_2xthru_reversed.push_back(Sparam(s.m22, s.m21, s.m12, s.m11));
    }

    // make the second error box
    auto data_side2 = makeErrorbox(data_fix_dut_fix_reversed, data_2xthru_reversed, f, gamma, z0);

    // got the error boxes, convert to T parameters and invert
    for(unsigned int i=0;i<f.size();i++) {
        Point p;
        p.freq = f[i];
        p.inverseP1 = Tparam(data_side1[i]).inverse();
        // correct port order of error box 2
        auto side2 = Sparam(data_side2[i].m22, data_side2[i].m21, data_side2[i].m12, data_side2[i].m11);
        p.inverseP2 = Tparam(side2).inverse();
        ret.push_back(p);
    }
    return ret;
}

std::vector<Protocol::Datapoint> TwoThru::interpolateEvenFrequencySteps(std::vector<Protocol::Datapoint> input)
{
    vector<Protocol::Datapoint> ret;
    if(input.size() > 1) {
        int size = input.size();
        double freqStep = 0.0;
        if(input.front().frequency == 0) {
            freqStep = input[1].frequency;
            size--;
        } else {
            freqStep = input[0].frequency;
        }
        if(freqStep * size == input.back().frequency) {
            // already correct spacing, no interpolation necessary
            for(auto d : input) {
                if(d.frequency == 0) {
                    continue;
                }
                ret.push_back(d);
            }
        } else {
            // needs to interpolate
            double freq = freqStep;
            while(freq <= input.back().frequency) {
                Protocol::Datapoint interp;
                auto it = lower_bound(input.begin(), input.end(), freq, [](const Protocol::Datapoint &lhs, const double f) -> bool {
                    return lhs.frequency < f;
                });
                if(it->frequency == freq) {
                    interp = *it;
                } else {
                    // no exact match, needs to interpolate
                    auto high = *it;
                    it--;
                    auto low = *it;
                    double alpha = (freq - low.frequency) / (high.frequency - low.frequency);
                    interp.real_S11 = low.real_S11 * (1.0 - alpha) + high.real_S11 * alpha;
                    interp.imag_S11 = low.imag_S11 * (1.0 - alpha) + high.imag_S11 * alpha;
                    interp.real_S12 = low.real_S12 * (1.0 - alpha) + high.real_S12 * alpha;
                    interp.imag_S12 = low.imag_S12 * (1.0 - alpha) + high.imag_S12 * alpha;
                    interp.real_S21 = low.real_S21 * (1.0 - alpha) + high.real_S21 * alpha;
                    interp.imag_S21 = low.imag_S21 * (1.0 - alpha) + high.imag_S21 * alpha;
                    interp.real_S22 = low.real_S22 * (1.0 - alpha) + high.real_S22 * alpha;
                    interp.imag_S22 = low.imag_S22 * (1.0 - alpha) + high.imag_S22 * alpha;
                }
                interp.frequency = freq;
                ret.push_back(interp);
                freq += freqStep;
            }
        }
    }
    return ret;
}
