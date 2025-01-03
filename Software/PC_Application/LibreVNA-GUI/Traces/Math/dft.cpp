#include "dft.h"

#include "tdr.h"
#include "Traces/fftcomplex.h"
#include "unit.h"
#include "ui_dftdialog.h"
#include "ui_dftexplanationwidget.h"
#include "appwindow.h"

#include <chrono>
#include <thread>

#include <QDebug>

using namespace std;

Math::DFT::DFT()
{
    automaticDC = true;
    removePaddingFromTDR = true;
    revertWindowFromTDR = true;
    DCfreq = 1000000000.0;

    destructing = false;
    thread = new DFTThread(*this);
    thread->start(TDRThread::Priority::LowestPriority);

    connect(&window, &WindowFunction::changed, this, &DFT::updateDFT);
}

Math::DFT::~DFT()
{
    // tell thread to exit
    destructing = true;
    semphr.release();
    thread->wait();
    delete thread;
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
    connect(d, &QDialog::finished, [=](){
        delete ui;
    });
    ui->windowBox->setLayout(new QVBoxLayout);
    ui->windowBox->layout()->addWidget(window.createEditor());

    connect(ui->removePadding, &QCheckBox::toggled, this, [=](bool remove){
        removePaddingFromTDR = remove;
    });

    connect(ui->revertWindow, &QCheckBox::toggled, this, [=](bool revert){
        revertWindowFromTDR = revert;
    });

    connect(ui->DCautomatic, &QRadioButton::toggled, this, [=](bool automatic){
        automaticDC = automatic;
        ui->freq->setEnabled(!automatic);
        updateDFT();
    });

    ui->removePadding->setChecked(removePaddingFromTDR);
    ui->revertWindow->setChecked(revertWindowFromTDR);

    if(automaticDC) {
        ui->DCautomatic->setChecked(true);
    } else {
        ui->DCmanual->setChecked(true);
    }

    ui->freq->setUnit("Hz");
    ui->freq->setPrecision(6);
    ui->freq->setPrefixes(" kMG");
    ui->freq->setValue(DCfreq);

    connect(ui->freq, &SIUnitEdit::valueChanged, this, [=](double newval){
        DCfreq = newval;
        updateDFT();
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, d, &QDialog::accept);
    if(AppWindow::showGUI()) {
        d->show();
    }
}

QWidget *Math::DFT::createExplanationWidget()
{
    auto w = new QWidget();
    auto ui = new Ui::DFTExplanationWidget;
    ui->setupUi(w);
    connect(w, &QWidget::destroyed, [=](){
        delete ui;
    });
    return w;
}

nlohmann::json Math::DFT::toJSON()
{
    nlohmann::json j;
    j["automatic_DC"] = automaticDC;
    j["window"] = window.toJSON();
    j["removePadding"] = removePaddingFromTDR;
    j["revertWindow"] = revertWindowFromTDR;
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
    removePaddingFromTDR = j.value("removePadding", true);
    revertWindowFromTDR = j.value("revertWindow", true);
}

void Math::DFT::inputSamplesChanged(unsigned int begin, unsigned int end)
{
    Q_UNUSED(begin);
    Q_UNUSED(end);
    if(input->numSamples() < 2) {
        // not enough input data
        clearOutput();
        warning("Not enough input samples");
        return;
    }
    // trigger calculation in thread
    semphr.release();
    success();
}

void Math::DFT::updateDFT()
{
    if(dataType != DataType::Invalid) {
        inputSamplesChanged(0, input->numSamples());
    }
}

void Math::DFT::clearOutput()
{
    dataMutex.lock();
    data.clear();
    dataMutex.unlock();
    emit outputSamplesChanged(0, 0);
}

Math::DFTThread::DFTThread(Math::DFT &dft)
    : dft(dft)
{

}

void Math::DFTThread::run()
{
    qDebug() << "DFT thread starting";
    using namespace std::chrono;
    auto lastCalc = system_clock::now();
    while(1) {
        dft.semphr.acquire();
        // clear possible additional semaphores
        dft.semphr.tryAcquire(dft.semphr.available());
        if(dft.destructing) {
            // TDR object about to be deleted, exit thread
            qDebug() << "DFT thread exiting";
            return;
        }
        // qDebug() << "DFT thread calculating";
        if(!dft.input) {
            // not connected, skip calculation
            continue;
        }
        auto inputData = dft.input->getData();
        if(!inputData.size()) {
            dft.clearOutput();
            dft.warning("Not enough input samples");
            continue;
        }

        double DC = dft.DCfreq;
        TDR *tdr = nullptr;
        // find the last TDR operation
        auto in = dft.input;
        while(in->getType() != DFT::Type::TDR) {
            in = dft.input->getInput();
            if(!in) {
                break;
            }
        }
        if(in) {
            tdr = static_cast<TDR*>(in);
        }

        if(tdr && dft.automaticDC) {
            if(tdr->getMode() == TDR::Mode::Lowpass) {
                DC = 0;
            } else {
                // bandpass mode, assume DC is in the middle of the frequency data
                DC = tdr->getInput()->getSample(tdr->getInput()->numSamples()/2).x;
            }
        }
        auto samples = inputData.size();
        auto timeSpacing = inputData[1].x - inputData[0].x;
        vector<complex<double>> timeDomain(samples);
        for(unsigned int i=0;i<samples;i++) {
            timeDomain.at(i) = inputData[i].y;
        }

        dft.window.apply(timeDomain);
        Fft::shift(timeDomain, true);
        Fft::transform(timeDomain, false);
        // shift DC bin into the middle
        Fft::shift(timeDomain, false);

        double binSpacing = 1.0 / (timeSpacing * timeDomain.size());

        if(tdr) {
            // split in padding and actual data sections
            unsigned int padding = 0;
            if(timeDomain.size() > tdr->getUnpaddedInputSize()) {
                padding = timeDomain.size() - tdr->getUnpaddedInputSize();
            }
            std::vector<std::complex<double>> pad_front(timeDomain.begin(), timeDomain.begin()+padding/2);
            std::vector<std::complex<double>> data(timeDomain.begin()+padding/2, timeDomain.end()-padding/2);
            std::vector<std::complex<double>> pad_back(timeDomain.end()-padding/2, timeDomain.end());

            if(dft.revertWindowFromTDR) {
                tdr->getWindow().reverse(data);
            }

            if(dft.removePaddingFromTDR) {
                timeDomain = data;
            } else {
                // include padding
                timeDomain = pad_front;
                copy(data.begin(), data.end(), back_inserter(timeDomain));
                copy(pad_back.begin(), pad_back.end(), back_inserter(timeDomain));
            }
        }

        int DCbin = timeDomain.size() / 2, startBin = 0;
        dft.dataMutex.lock();
        if(DC > 0) {
            dft.data.resize(timeDomain.size(), TraceMath::Data());
        } else {
            startBin = (timeDomain.size()+1) / 2;
            dft.data.resize(timeDomain.size()/2, TraceMath::Data());
        }

        for(int i = startBin;(unsigned int) i<timeDomain.size();i++) {
            auto freq = (i - DCbin) * binSpacing + DC;
            dft.data[i - startBin].x = round(freq);
            dft.data[i - startBin].y = timeDomain.at(i);
        }
        auto size = dft.data.size();
        dft.dataMutex.unlock();
        emit dft.outputSamplesChanged(0, size);

        // limit update rate if configured in preferences
        auto &p = Preferences::getInstance();
        if(p.Acquisition.limitDFT) {
            std::this_thread::sleep_until(lastCalc + duration<double>(1.0 / p.Acquisition.maxDFTrate));
            lastCalc = system_clock::now();
        }
    }
}
