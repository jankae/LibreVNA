#include "tdr.h"

#include "Traces/fftcomplex.h"
#include "ui_tdrdialog.h"
#include "ui_tdrexplanationwidget.h"
#include "Util/util.h"
#include "appwindow.h"

#include <chrono>
#include <thread>

#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>

using namespace Math;
using namespace std;

TDR::TDR()
{
    automaticDC = true;
    manualDC = 1.0;
    DCExtrapolationPoints = 5;
    lastDC = 0.0;
    lastDCValid = false;
    stepResponse = true;
    mode = Mode::Lowpass;
    padding = 0;

    destructing = false;
    thread = new TDRThread(*this);
    thread->start(TDRThread::Priority::LowestPriority);

    connect(&window, &WindowFunction::changed, this, &TDR::updateTDR);
}

TDR::~TDR()
{
    // tell thread to exit
    destructing = true;
    semphr.release();
    thread->wait();
    delete thread;
}

TraceMath::DataType TDR::outputType(TraceMath::DataType inputType)
{
    if(inputType == DataType::Frequency) {
        return DataType::Time;
    } else {
        return DataType::Invalid;
    }
}

QString TDR::description()
{
    QString ret = "TDR (";
    if(mode == Mode::Lowpass) {
        ret += "lowpass)";
        if(stepResponse) {
            ret += ", with step response";
        }
    } else {
        ret += "bandpass)";
    }
    ret += ", window: " + window.getDescription();

    return ret;
}

void TDR::edit()
{
    auto d = new QDialog();
    auto ui = new Ui::TDRDialog;
    ui->setupUi(d);
    connect(d, &QDialog::finished, [=](){
        delete ui;
    });
    ui->windowBox->setLayout(new QVBoxLayout);
    ui->windowBox->layout()->addWidget(window.createEditor());

    auto updateEnabledWidgets = [=]() {
        // step response and DC point are only relevant in lowpass mode, hide them otherwise
        bool lowpass = mode == Mode::Lowpass;
        ui->computeStepResponse->setVisible(lowpass);
        ui->groupBox->setVisible(lowpass);
        bool enable = lowpass && stepResponse;
        ui->DCmanual->setEnabled(enable);
        ui->DCautomatic->setEnabled(enable);
        enable &= !automaticDC;
        ui->manualPhase->setEnabled(enable);
        ui->manualMag->setEnabled(enable);
        enable = mode == Mode::Lowpass && stepResponse && automaticDC;
        ui->extrapolationPoints->setEnabled(enable);
    };

    // Shows the DC value in the magnitude/phase fields: the live extrapolated value when
    // automatic extrapolation is active, the manually specified value otherwise.
    // Uses setValueQuiet to not feed the displayed value back into manualDC
    auto updateDCPreview = [=]() {
        auto dc = manualDC;
        if(automaticDC) {
            dataMutex.lock();
            bool valid = lastDCValid;
            dc = lastDC;
            dataMutex.unlock();
            if(!valid) {
                ui->manualMag->clear();
                ui->manualPhase->clear();
                return;
            }
        }
        ui->manualMag->setValueQuiet(Util::SparamTodB(dc));
        ui->manualPhase->setValueQuiet(180.0/M_PI * arg(dc));
    };
    auto previewConnection = connect(this, &TDR::DCValueUpdated, d, updateDCPreview);
    connect(d, &QDialog::finished, this, [=](){
        disconnect(previewConnection);
    });

    connect(ui->mode, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int index){
        mode = (Mode) index;
        updateEnabledWidgets();
        updateDCPreview();
        updateTDR();
    });

    connect(ui->padding, &QSpinBox::valueChanged, this, [=](int value) {
        padding = value;
    });

    connect(ui->computeStepResponse, &QCheckBox::toggled, this, [=](bool computeStep) {
        stepResponse = computeStep;
        updateEnabledWidgets();
        updateDCPreview();
        updateTDR();
    });

    connect(ui->DCmanual, &QRadioButton::toggled, this, [=](bool manual) {
        automaticDC = !manual;
        updateEnabledWidgets();
        updateDCPreview();
        updateTDR();
    });

    if(automaticDC) {
        ui->DCautomatic->setChecked(true);
    } else {
        ui->DCmanual->setChecked(true);
    }
    ui->computeStepResponse->setChecked(stepResponse);
    if(mode == Mode::Bandpass) {
        ui->mode->setCurrentIndex(1);
    }

    ui->padding->setValue(padding);

    ui->extrapolationPoints->setValue(DCExtrapolationPoints);
    connect(ui->extrapolationPoints, &QSpinBox::valueChanged, this, [=](int value) {
        setDCExtrapolationPoints(value);
    });
    ui->manualMag->setUnit("dB");
    ui->manualMag->setPrecision(3);
    ui->manualPhase->setUnit("°");
    ui->manualPhase->setPrecision(4);

    updateEnabledWidgets();
    updateDCPreview();

    connect(ui->manualMag, &SIUnitEdit::valueChanged, this, [=](double newval){
        manualDC = polar(pow(10, newval / 20.0), arg(manualDC));
        updateTDR();
    });
    connect(ui->manualPhase, &SIUnitEdit::valueChanged, this, [=](double newval){
        manualDC = polar(abs(manualDC), newval * M_PI / 180.0);
        updateTDR();
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, d, &QDialog::accept);
    if(AppWindow::showGUI()) {
        d->show();
    }
}

QWidget *TDR::createExplanationWidget()
{
    auto w = new QWidget();
    auto ui = new Ui::TDRExplanationWidget;
    ui->setupUi(w);
    connect(w, &QWidget::destroyed, [=](){
        delete ui;
    });
    return w;
}

nlohmann::json TDR::toJSON()
{
    nlohmann::json j;
    j["bandpass_mode"] = mode == Mode::Bandpass;
    j["window"] = window.toJSON();
    j["padding"] = padding;
    j["DC_extrapolation_points"] = DCExtrapolationPoints;
    if(mode == Mode::Lowpass) {
        j["step_response"] = stepResponse;
        if(stepResponse) {
            j["automatic_DC"] = automaticDC;
            if(!automaticDC) {
                j["manual_DC_real"] = manualDC.real();
                j["manual_DC_imag"] = manualDC.imag();
            }
        }
    }
    return j;
}

void TDR::fromJSON(nlohmann::json j)
{
    if(j.contains("window")) {
        window.fromJSON(j["window"]);
    }
    padding = j.value("padding", 0);
    // older versions always used two points for the extrapolation and this parameter might be missing.
    // The exact implementation of the interpolation is also different, so there is no point in keeping
    // the old value. Just start from the default of 5 and ensure that is is at least 2
    DCExtrapolationPoints = std::max(2, j.value("DC_extrapolation_points", 5));
    if(j.value("bandpass_mode", true)) {
        mode = Mode::Bandpass;
    } else {
        mode = Mode::Lowpass;
        if(j.value("step_response", true)) {
            stepResponse = true;
            if(j.value("automatic_DC", true)) {
                automaticDC = true;
            } else {
                automaticDC = false;
                manualDC = complex<double>(j.value("manual_DC_real", 1.0), j.value("manual_DC_imag", 0.0));
            }
        } else {
            stepResponse = false;
        }
    }
}

void TDR::setMode(Mode m)
{
    if(mode == m) {
        // already set to correct mode
        return;
    }
    mode = m;
    if(input) {
        inputSamplesChanged(0, input->numSamples());
    }
}

void TDR::inputSamplesChanged(unsigned int begin, unsigned int end)
{
    Q_UNUSED(begin);
    Q_UNUSED(end);
    if(input->numSamples() >= 2) {
        // trigger calculation in thread
        semphr.release();
        success();
    } else {
        // not enough input data
        clearOutput();
        warning("Not enough input samples");
    }
}

void TDR::updateTDR()
{
    if(dataType != DataType::Invalid) {
        inputSamplesChanged(0, input->numSamples());
    }
}

void TDR::clearOutput()
{
    dataMutex.lock();
    data.clear();
    dataMutex.unlock();
    updateStepResponse(false);
    emit outputSamplesChanged(0, 0);
}

unsigned int TDR::getUnpaddedInputSize() const
{
    return unpaddedInputSize;
}

const WindowFunction& TDR::getWindow() const
{
    return window;
}

unsigned int TDR::getDCExtrapolationPoints() const
{
    return DCExtrapolationPoints;
}

void TDR::setDCExtrapolationPoints(unsigned int points)
{
    if(points < 2) {
        points = 2;
    }
    if(points == DCExtrapolationPoints) {
        return;
    }
    DCExtrapolationPoints = points;
    updateTDR();
}

std::complex<double> TDR::getDCValue()
{
    QMutexLocker locker(&dataMutex);
    return lastDC;
}

TDR::Mode TDR::getMode() const
{
    return mode;
}

TDRThread::TDRThread(TDR &tdr)
    : tdr(tdr)
{

}

void TDRThread::run()
{
    qDebug() << "TDR thread starting";
    using namespace std::chrono;
    auto lastCalc = system_clock::now();
    while(1) {
        tdr.semphr.acquire();
        // clear possible additional semaphores
        tdr.semphr.tryAcquire(tdr.semphr.available());
        if(tdr.destructing) {
            // TDR object about to be deleted, exit thread
            qDebug() << "TDR thread exiting";
            return;
        }
        // qDebug() << "TDR thread calculating";
        // perform calculation
        if(!tdr.input) {
            // not connected, skip calculation
            continue;
        }
        auto inputData = tdr.input->getData();
        if(inputData.size() < 2) {
            // empty input data, clear output data
            tdr.clearOutput();
            tdr.warning("Not enough input samples");
            continue;
        }
        vector<complex<double>> frequencyDomain;
        auto stepSize = (inputData.back().x - inputData.front().x) / (inputData.size() - 1);
        if(tdr.mode == TDR::Mode::Lowpass) {
            auto steps = inputData.size();
            auto firstStep = inputData.front().x;
            // frequency points need to be evenly spaced all the way to DC
            if(firstStep == 0) {
                // zero as first step would result in infinite number of points, skip and start with second
                firstStep = inputData[1].x;
                steps--;
            }
            if(firstStep * steps != inputData.back().x) {
                // data is not available with correct frequency spacing, calculate required steps
                steps = inputData.back().x / firstStep;
                stepSize = firstStep;
            }
            frequencyDomain.resize(2 * steps + 1);
            // copy frequencies, use the flipped conjugate for negative part
            for(unsigned int i = 1;i<=steps;i++) {
                auto S = TraceMath::interpolatedSample(inputData, stepSize * i);
                frequencyDomain[steps - i] = conj(S);
                frequencyDomain[steps + i] = S;
            }
            complex<double> DC;
            if(tdr.automaticDC) {
                // linear least-squares fit of magnitude and (unwrapped) phase over the lowest measured
                // input points (at their actual frequencies), evaluated at DC. A possibly existing
                // sample at exactly 0Hz is not used
                vector<double> freqs, mags, phases;
                double lastPhase = 0;
                for(const auto &sample : inputData) {
                    if(freqs.size() >= tdr.DCExtrapolationPoints) {
                        break;
                    }
                    if(sample.x <= 0) {
                        continue;
                    }
                    double phase = arg(sample.y);
                    if(!phases.empty()) {
                        // unwrap phase
                        phase = lastPhase + remainder(phase - lastPhase, 2 * M_PI);
                    }
                    lastPhase = phase;
                    freqs.push_back(sample.x);
                    mags.push_back(abs(sample.y));
                    phases.push_back(phase);
                }
                if(freqs.size() < 2) {
                    // not enough points for an extrapolation, use the lowest point
                    DC = frequencyDomain[steps + 1];
                } else {
                    // least-squares linear fit over magnitude and phase
                    auto n = freqs.size();
                    double meanF = 0, meanMag = 0, meanPhase = 0;
                    for(unsigned int i=0;i<n;i++) {
                        meanF += freqs[i];
                        meanMag += mags[i];
                        meanPhase += phases[i];
                    }
                    meanF /= n;
                    meanMag /= n;
                    meanPhase /= n;
                    double sFF = 0, sFMag = 0, sFPhase = 0;
                    for(unsigned int i=0;i<n;i++) {
                        auto dF = freqs[i] - meanF;
                        sFF += dF * dF;
                        sFMag += dF * (mags[i] - meanMag);
                        sFPhase += dF * (phases[i] - meanPhase);
                    }
                    // extrapolate to DC
                    double abs_DC = meanMag - sFMag / sFF * meanF;
                    double phase_DC = meanPhase - sFPhase / sFF * meanF;
                    DC = polar(abs_DC, phase_DC);
                }
            } else {
                DC = tdr.manualDC;
            }
            frequencyDomain[steps] = DC;
            tdr.dataMutex.lock();
            tdr.lastDC = DC;
            tdr.lastDCValid = true;
            tdr.dataMutex.unlock();
            emit tdr.DCValueUpdated();
        } else {
            // bandpass mode
            // Can use input data directly, no need to extend with complex conjugate
            frequencyDomain.resize(inputData.size());
            for(unsigned int i=0;i<inputData.size();i++) {
                frequencyDomain[i] = inputData[i].y;
            }
        }

        tdr.window.apply(frequencyDomain);
        tdr.unpaddedInputSize = frequencyDomain.size();
        if(tdr.padding > 0) {
            frequencyDomain.insert(frequencyDomain.begin(), tdr.padding/2, 0);
            frequencyDomain.insert(frequencyDomain.end(), tdr.padding/2, 0);
        }
        Fft::shift(frequencyDomain, true);

        int fft_bins = frequencyDomain.size();
        const double fs = 1.0 / (stepSize * fft_bins);

        Fft::transform(frequencyDomain, true);
        Fft::shift(frequencyDomain, false);

        tdr.dataMutex.lock();
        tdr.data.resize(fft_bins, TraceMath::Data());
        for(int i = 0;i<fft_bins;i++) {
            tdr.data[i].x = fs * (i - fft_bins / 2);
            tdr.data[i].y = frequencyDomain[i] / (double) fft_bins;
        }
        auto size = tdr.data.size();
        tdr.dataMutex.unlock();
        if(tdr.stepResponse && tdr.mode == TDR::Mode::Lowpass) {
            tdr.updateStepResponse(true);
        } else {
            tdr.updateStepResponse(false);
        }
        emit tdr.outputSamplesChanged(0, size);

        // limit update rate if configured in preferences
        auto &p = Preferences::getInstance();
        if(p.Acquisition.limitDFT) {
            std::this_thread::sleep_until(lastCalc + duration<double>(1.0 / p.Acquisition.maxDFTrate));
            lastCalc = system_clock::now();
        }
    }
}
