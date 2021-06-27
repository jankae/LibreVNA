#include "tdr.h"

#include "Traces/fftcomplex.h"
#include "ui_tdrdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include "ui_tdrexplanationwidget.h"
#include "Util/util.h"

using namespace Math;
using namespace std;

TDR::TDR()
{
    automaticDC = true;
    manualDC = 1.0;
    stepResponse = true;
    mode = Mode::Lowpass;

    connect(&window, &WindowFunction::changed, this, &TDR::updateTDR);
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
    ui->windowBox->setLayout(new QVBoxLayout);
    ui->windowBox->layout()->addWidget(window.createEditor());

    auto updateEnabledWidgets = [=]() {
        bool enable = mode == Mode::Lowpass;
        ui->computeStepResponse->setEnabled(enable);
        enable &= stepResponse;
        ui->DCmanual->setEnabled(enable);
        ui->DCautomatic->setEnabled(enable);
        enable &= !automaticDC;
        ui->manualPhase->setEnabled(enable);
        ui->manualMag->setEnabled(enable);
    };

    connect(ui->mode, qOverload<int>(&QComboBox::currentIndexChanged), [=](int index){
        mode = (Mode) index;
        updateEnabledWidgets();
        updateTDR();
    });

    connect(ui->computeStepResponse, &QCheckBox::toggled, [=](bool computeStep) {
       stepResponse = computeStep;
        updateEnabledWidgets();
       updateTDR();
    });

    connect(ui->DCmanual, &QRadioButton::toggled, [=](bool manual) {
        automaticDC = !manual;
        updateEnabledWidgets();
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

    ui->manualMag->setUnit("dBm");
    ui->manualMag->setPrecision(3);
    ui->manualMag->setValue(Util::SparamTodB(manualDC));
    ui->manualPhase->setUnit("°");
    ui->manualPhase->setPrecision(4);
    ui->manualPhase->setValue(180.0/M_PI * arg(manualDC));

    connect(ui->manualMag, &SIUnitEdit::valueChanged, [=](double newval){
        manualDC = polar(pow(10, newval / 20.0), arg(manualDC));
        updateTDR();
    });
    connect(ui->manualPhase, &SIUnitEdit::valueChanged, [=](double newval){
        manualDC = polar(abs(manualDC), newval * M_PI / 180.0);
        updateTDR();
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, d, &QDialog::accept);
    d->show();
}

QWidget *TDR::createExplanationWidget()
{
    auto w = new QWidget();
    auto ui = new Ui::TDRExplanationWidget;
    ui->setupUi(w);
    return w;
}

nlohmann::json TDR::toJSON()
{
    nlohmann::json j;
    j["bandpass_mode"] = mode == Mode::Bandpass;
    j["window"] = window.toJSON();
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
        inputSamplesChanged(0, input->rData().size());
    }
}

void TDR::inputSamplesChanged(unsigned int begin, unsigned int end)
{
    Q_UNUSED(end);
    if(input->rData().size() >= 2) {
        // TDR is computationally expensive, only update at the end of sweep -> check if this is the first changed data in the next sweep
        if(begin != 0) {
            // not the end, do nothing
            return;
        }
        vector<complex<double>> frequencyDomain;
        auto stepSize = (input->rData().back().x - input->rData().front().x) / (input->rData().size() - 1);
        if(mode == Mode::Lowpass) {
            if(stepResponse) {
                auto steps = input->rData().size();
                auto firstStep = input->rData().front().x;
                // frequency points need to be evenly spaced all the way to DC
                if(firstStep == 0) {
                    // zero as first step would result in infinite number of points, skip and start with second
                    firstStep = input->rData()[1].x;
                    steps--;
                }
                if(firstStep * steps != input->rData().back().x) {
                    // data is not available with correct frequency spacing, calculate required steps
                    steps = input->rData().back().x / firstStep;
                    stepSize = firstStep;
                }
                frequencyDomain.resize(2 * steps + 1);
                // copy frequencies, use the flipped conjugate for negative part
                for(unsigned int i = 1;i<=steps;i++) {
                    auto S = input->getInterpolatedSample(stepSize * i).y;
                    frequencyDomain[steps - i] = conj(S);
                    frequencyDomain[steps + i] = S;
                }
                if(automaticDC) {
                    // use simple extrapolation from lowest two points to extract DC value
                    auto abs_DC = 2.0 * abs(frequencyDomain[steps + 1]) - abs(frequencyDomain[steps + 2]);
                    auto phase_DC = 2.0 * arg(frequencyDomain[steps + 1]) - arg(frequencyDomain[steps + 2]);
                    frequencyDomain[steps] = polar(abs_DC, phase_DC);
                } else {
                    frequencyDomain[steps] = manualDC;
                }
            } else {
                auto steps = input->rData().size();
                unsigned int offset = 0;
                if(input->rData().front().x == 0) {
                    // DC measurement is inaccurate, skip
                    steps--;
                    offset++;
                }
                // no step response required, can use frequency values as they are. No extra extrapolated DC value here -> 2 values less than with step response
                frequencyDomain.resize(2 * steps - 1);
                frequencyDomain[steps - 1] = input->rData()[offset].y;
                for(unsigned int i = 1;i<steps;i++) {
                    auto S = input->rData()[i + offset].y;
                    frequencyDomain[steps - i - 1] = conj(S);
                    frequencyDomain[steps + i - 1] = S;
                }
            }
        } else {
            // bandpass mode
            // Can use input data directly, no need to extend with complex conjugate
            frequencyDomain.resize(input->rData().size());
            for(unsigned int i=0;i<input->rData().size();i++) {
                frequencyDomain[i] = input->rData()[i].y;
            }
        }

        window.apply(frequencyDomain);
        Fft::shift(frequencyDomain, true);

        auto fft_bins = frequencyDomain.size();
        const double fs = 1.0 / (stepSize * fft_bins);

        Fft::transform(frequencyDomain, true);

        data.clear();
        data.resize(fft_bins);

        for(unsigned int i = 0;i<fft_bins;i++) {
            data[i].x = fs * i;
            data[i].y = frequencyDomain[i] / (double) fft_bins;
        }
        if(stepResponse && mode == Mode::Lowpass) {
            updateStepResponse(true);
        } else {
            updateStepResponse(false);
        }
        emit outputSamplesChanged(0, data.size());
        success();
    } else {
        // not enough input data
        data.clear();
        updateStepResponse(false);
        emit outputSamplesChanged(0, 0);
        warning("Not enough input samples");
    }
}

void TDR::updateTDR()
{
    if(dataType != DataType::Invalid) {
        inputSamplesChanged(0, input->rData().size());
    }
}

const WindowFunction& TDR::getWindow() const
{
    return window;
}

TDR::Mode TDR::getMode() const
{
    return mode;
}
