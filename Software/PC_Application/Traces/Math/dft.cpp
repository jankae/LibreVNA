#include "dft.h"
#include "tdr.h"
#include "Traces/fftcomplex.h"
#include "unit.h"
#include "ui_dftdialog.h"
#include "ui_dftexplanationwidget.h"
using namespace std;

Math::DFT::DFT()
{
    automaticDC = true;
    DCfreq = 1000000000.0;

    connect(&window, &WindowFunction::changed, this, &DFT::updateDFT);
}

TraceMath::DataType Math::DFT::outputType(TraceMath::DataType inputType)
{
    if(inputType == DataType::Time) {
        return DataType::Frequency;
    } else {
        return DataType::Invalid;
    }
}

QString Math::DFT::description()
{
    QString ret = "DFT (";
    if(automaticDC) {
        ret += "automatic DC)";
    } else {
        ret += "DC:" + Unit::ToString(DCfreq, "Hz", " kMG", 6) + ")";
    }
    ret += ", window: " + window.getDescription();
    return ret;
}

void Math::DFT::edit()
{
    auto d = new QDialog();
    auto ui = new Ui::DFTDialog;
    ui->setupUi(d);
    ui->windowBox->setLayout(new QVBoxLayout);
    ui->windowBox->layout()->addWidget(window.createEditor());

    connect(ui->DCautomatic, &QRadioButton::toggled, [=](bool automatic){
        automaticDC = automatic;
        ui->freq->setEnabled(!automatic);
        updateDFT();
    });

    if(automaticDC) {
        ui->DCautomatic->setChecked(true);
    } else {
        ui->DCmanual->setChecked(true);
    }

    ui->freq->setUnit("Hz");
    ui->freq->setPrecision(6);
    ui->freq->setPrefixes(" kMG");
    ui->freq->setValue(DCfreq);

    connect(ui->freq, &SIUnitEdit::valueChanged, [=](double newval){
        DCfreq = newval;
        updateDFT();
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, d, &QDialog::accept);
    d->show();
}

QWidget *Math::DFT::createExplanationWidget()
{
    auto w = new QWidget();
    auto ui = new Ui::DFTExplanationWidget;
    ui->setupUi(w);
    return w;
}

nlohmann::json Math::DFT::toJSON()
{
    nlohmann::json j;
    j["automatic_DC"] = automaticDC;
    j["window"] = window.toJSON();
    if(!automaticDC) {
        j["DC"] = DCfreq;
    }
    return j;
}

void Math::DFT::fromJSON(nlohmann::json j)
{
    automaticDC = j.value("automatic_DC", true);
    DCfreq = j.value("DC", 1000000000.0);
    if(j.contains("window")) {
        window.fromJSON(j["window"]);
    }
}

void Math::DFT::inputSamplesChanged(unsigned int begin, unsigned int end)
{
    Q_UNUSED(end);
    if(input->rData().size() < 2) {
        // not enough input data
        data.clear();
        emit outputSamplesChanged(0, 0);
        warning("Not enough input samples");
        return;
    }
    // DFT is computationally expensive, only update at the end of sweep -> check if this is the first changed data in the next sweep
    if(begin != 0) {
        // not the end, do nothing
        return;
    }
    double DC = DCfreq;
    TDR *tdr = nullptr;
    if(automaticDC) {
        // find the last operation that transformed from the frequency domain to the time domain
        auto in = input;
        while(in->getInput()->getDataType() != DataType::Frequency) {
            in = input->getInput();
        }
        switch(in->getType()) {
        case Type::TDR: {
            tdr = static_cast<TDR*>(in);
            if(tdr->getMode() == TDR::Mode::Lowpass) {
                DC = 0;
            } else {
                // bandpass mode, assume DC is in the middle of the frequency data
                DC = tdr->getInput()->getSample(tdr->getInput()->numSamples()/2).x;
            }
        }
            break;
        default:
            // unknown, assume DC is in the middle of the frequency data
            DC = in->getInput()->getSample(in->getInput()->numSamples()/2).x;
            break;
        }
    }
    auto samples = input->rData().size();
    auto timeSpacing = input->rData()[1].x - input->rData()[0].x;
    vector<complex<double>> timeDomain(samples);
    for(unsigned int i=0;i<samples;i++) {
        timeDomain.at(i) = input->rData()[i].y;
    }

    Fft::shift(timeDomain, false);
    window.apply(timeDomain);
    Fft::shift(timeDomain, true);
    Fft::transform(timeDomain, false);
    // shift DC bin into the middle
    Fft::shift(timeDomain, false);

    double binSpacing = 1.0 / (timeSpacing * timeDomain.size());
    data.clear();
    int DCbin = timeDomain.size() / 2, startBin = 0;
    if(DC > 0) {
        data.resize(timeDomain.size());
    } else {
        startBin = (timeDomain.size()+1) / 2;
        data.resize(timeDomain.size()/2);
    }

    // reverse effect of frequency domain window function from TDR (if available)
    if(tdr) {
        tdr->getWindow().reverse(timeDomain);
    }

    for(int i = startBin;(unsigned int) i<timeDomain.size();i++) {
        auto freq = (i - DCbin) * binSpacing + DC;
        data[i - startBin].x = round(freq);
        data[i - startBin].y = timeDomain.at(i);
    }
    emit outputSamplesChanged(0, data.size());
    success();
}

void Math::DFT::updateDFT()
{
    if(dataType != DataType::Invalid) {
        inputSamplesChanged(0, input->rData().size());
    }
}
