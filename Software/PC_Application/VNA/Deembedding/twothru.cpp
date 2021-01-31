#include "twothru.h"
#include "CustomWidgets/informationbox.h"
#include "ui_twothrudialog.h"
#include "Traces/fftcomplex.h"

using namespace std;

TwoThru::TwoThru()
{
    measuring = false;
}

void TwoThru::transformDatapoint(Protocol::Datapoint &p)
{
    auto S11 = complex<double>(p.real_S11, p.imag_S11);
    auto S12 = complex<double>(p.real_S12, p.imag_S12);
    auto S21 = complex<double>(p.real_S21, p.imag_S21);
    auto S22 = complex<double>(p.real_S22, p.imag_S22);
    Sparam S(S11, S12, S21, S22);
    Tparam meas(S);
    if(measuring) {
        if(measurements.size() > 0 && p.pointNum == 0) {
            // complete sweep measured, exit measurement mode
            measuring = false;
            // calculate error boxes, see https://www.freelists.org/post/si-list/IEEE-P370-Opensource-Deembedding-MATLAB-functions
            // create vectors of S parameters
            vector<complex<double>> S11, S12, S21, S22;
            vector<double> f;
            for(auto m : measurements) {
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
                        if(arg(test) - arg(last_test) > 0) {
                            k = -k;
                        }
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
                        if(arg(test) - arg(last_test) > 0) {
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
            for(unsigned int i=0;i<n;i++) {
                Point p;
                p.freq = f[i];
                p.inverseP1 = Tparam(data_side1[i]).inverse();
                p.inverseP2 = Tparam(data_side2[i]).inverse();
                points.push_back(p);
            }
            measurements.clear();

            if(msgBox) {
                msgBox->accept();
                msgBox = nullptr;
            }
            updateLabel();
        } else if(measurements.size() > 0 || p.pointNum == 0) {
            measurements.push_back(p);
        }
    }

    // correct measurement
    if(points.size() > 0) {
        if(p.frequency != 0 && (p.frequency < points.front().freq || p.frequency > points.back().freq)) {
            // No exact match, measurement no longer valid
            points.clear();
            InformationBox::ShowMessage("Warning", "2xThru measurement cleared because it no longer matches the selected span");
            return;
        }
        // find correct measurement point
        auto point = lower_bound(points.begin(), points.end(), p.frequency, [](Point p, uint64_t freq) -> bool {
            return p.freq < freq;
        });
        Tparam inv1, inv2;
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
        // perform correction
        Tparam corrected = inv1*meas*inv2;
        // transform back into S parameters
        Sparam S(corrected);
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
    points.clear();
    measurements.clear();
    updateLabel();
    msgBox = new QMessageBox(QMessageBox::Information, "2xThru", "Taking measurement...", QMessageBox::Cancel);
    connect(msgBox, &QMessageBox::rejected, [=]() {
        measuring = false;
        points.clear();
        measurements.clear();
        updateLabel();
    });
    msgBox->show();
    measuring = true;
}

void TwoThru::updateLabel()
{
    if(points.size() > 0) {
        ui->lInfo->setText("Got "+QString::number(points.size())+" points");
    } else {
        ui->lInfo->setText("No measurement, not deembedding");
    }
}

void TwoThru::edit()
{
    auto dialog = new QDialog();
    ui = new Ui::TwoThruDialog();
    ui->setupUi(dialog);

    connect(ui->bMeasure, &QPushButton::clicked, this, &TwoThru::startMeasurement);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);

    updateLabel();

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
