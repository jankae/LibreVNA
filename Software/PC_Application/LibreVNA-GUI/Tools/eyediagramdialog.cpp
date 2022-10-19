#include "eyediagramdialog.h"
#include "ui_eyediagramdialog.h"
#include "Util/prbs.h"
#include "Traces/Math/tdr.h"
#include "Util/util.h"
#include "preferences.h"
#include "Traces/fftcomplex.h"

#include <random>
#include <thread>
#include <chrono>

#include <QPainter>

using namespace std::chrono_literals;

EyeDiagramDialog::EyeDiagramDialog(TraceModel &model) :
    QDialog(nullptr),
    ui(new Ui::EyeDiagramDialog)
{
    ui->setupUi(this);

    workingBuffer = &eyeBuffer[0];
    finishedBuffer = &eyeBuffer[1];

    updating = false;

    trace = nullptr;
    ui->update->setEnabled(false);

    ui->datarate->setUnit("bps");
    ui->datarate->setPrefixes(" kMG");
    ui->datarate->setPrecision(3);

    ui->risetime->setUnit("s");
    ui->risetime->setPrefixes("pnum ");
    ui->risetime->setPrecision(3);

    ui->falltime->setUnit("s");
    ui->falltime->setPrefixes("pnum ");
    ui->falltime->setPrecision(3);

    ui->highLevel->setUnit("V");
    ui->highLevel->setPrefixes("m ");
    ui->highLevel->setPrecision(3);

    ui->lowLevel->setUnit("V");
    ui->lowLevel->setPrefixes("m ");
    ui->lowLevel->setPrecision(3);

    ui->noise->setUnit("V");
    ui->noise->setPrefixes("um ");
    ui->noise->setPrecision(3);

    ui->jitter->setUnit("s");
    ui->jitter->setPrefixes("pnum ");
    ui->jitter->setPrecision(3);

    ui->datarate->setValue(100000000);
    ui->risetime->setValue(0.000000001);
    ui->falltime->setValue(0.000000001);
    ui->highLevel->setValue(1);
    ui->lowLevel->setValue(0);
    ui->noise->setValue(0.01);
    ui->jitter->setValue(0.0000000001);

    ui->displayedCycles->setValue(200);

    ui->widget->setDialog(this);

    connect(this, &EyeDiagramDialog::updatePercent, ui->progress, &QProgressBar::setValue, Qt::QueuedConnection);
    connect(ui->update, &QPushButton::clicked, this, &EyeDiagramDialog::triggerUpdate);
    connect(this, &EyeDiagramDialog::updateDone, ui->widget, qOverload<>(&QWidget::update));

    connect(ui->traceSelector, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
        trace = qvariant_cast<Trace*>(ui->traceSelector->itemData(ui->traceSelector->currentIndex()));
        ui->update->setEnabled(true);
    });

    // find applicable traces
    for(auto t : model.getTraces()) {
        if(t->getDataType() != Trace::DataType::Frequency) {
            // wrong domain
            continue;
        }
        if(t->isReflection()) {
            // can't work with reflection measurements
            continue;
        }
        if(t->numSamples() < 100) {
            // not enough samples
            continue;
        }
        auto start = t->getSample(0).x;
        auto stop = t->getSample(t->numSamples() - 1).x;
        if(stop - start < start * 100) {
            // span/start is not suitable for step response TDR
            continue;
        }
        // can use this trace
        ui->traceSelector->addItem(t->name(), QVariant::fromValue<Trace*>(t));
    }
}

EyeDiagramDialog::~EyeDiagramDialog()
{
    delete ui;
}

double EyeDiagramDialog::getIntensity(unsigned int x, unsigned int y)
{
    if(finishedBuffer->size() > x) {
        if((*finishedBuffer)[x].size() > y) {
            return (*finishedBuffer)[x][y];
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}

bool EyeDiagramDialog::triggerUpdate()
{
    update(ui->widget->width(), ui->widget->height());
}

bool EyeDiagramDialog::update(unsigned int width, unsigned int height)
{
    if(updating) {
        // already updating, can't start again
        return false;
    }
    updating = true;
    new std::thread(&EyeDiagramDialog::updateThread, this, width, height);
}

void EyeDiagramDialog::updateThread(unsigned int width, unsigned int height)
{
    emit updatePercent(0);
    if(!trace) {
        updating = false;
        return;
    }

    qDebug() << "Starting eye diagram calculation";

    auto datarate = ui->datarate->value();
    auto highlevel = ui->highLevel->value();
    auto lowlevel = ui->lowLevel->value();
    auto risetime = ui->risetime->value();
    auto falltime = ui->falltime->value();
    auto noise = ui->noise->value();
    auto jitter = ui->jitter->value();
    unsigned int patternbits = ui->patternLength->currentIndex() + 2;
    unsigned int cycles = ui->displayedCycles->value() + 1; // first cycle will not be displayed

    // sanity check values
    // TODO

    qDebug() << "Eye calculation: input values okay";

    // resize working buffer
    workingBuffer->clear();
    workingBuffer->resize(width);
    for(auto& y : *workingBuffer) {
        y.resize(height, 0.0);
    }

    // calculate timestep
    double displayedTime = 2 * 1.0/datarate; // always showing two bit periods
    double timestep = displayedTime / (width);

    auto prbs = new PRBS(patternbits);

    bool currentBit = prbs->next();
    bool nextBit = prbs->next();

    // initialize random generator
    std::random_device rd1;
    std::mt19937 mt_noise(rd1());
    std::normal_distribution<> dist_noise(0, noise);

    std::random_device rd2;
    std::mt19937 mt_jitter(rd2());
    std::normal_distribution<> dist_jitter(0, jitter);

    // reserve vector for input data
    std::vector<double> input(width * cycles, 0.0);

    unsigned int bitcnt = 1;
    double transitionTime = -10; // assume that we start with a settled input, last transition was "long" ago
    for(unsigned int i=0;i<input.size();i++) {
        double time = i*timestep;
        double voltage;
        if(time >= transitionTime) {
            // currently within a bit transition
            double edgeTime = 0;
            if(!currentBit && nextBit) {
                edgeTime = risetime;
            } else if(currentBit && !nextBit) {
                edgeTime = falltime;
            }
            if(time >= transitionTime + edgeTime) {
                // bit transition settled
                voltage = nextBit ? highlevel : lowlevel;
                // move on to the next bit
                currentBit = nextBit;
                nextBit = prbs->next();
                transitionTime = bitcnt * 1.0 / datarate + dist_jitter(mt_jitter);
                bitcnt++;
            } else {
                // still within rise or fall time
                double timeSinceEdge = time - transitionTime;
                double edgeRatio = timeSinceEdge / edgeTime;
                double from = currentBit ? highlevel : lowlevel;
                double to = nextBit ? highlevel : lowlevel;
                voltage = from * (1.0 - edgeRatio) + to * edgeRatio;
            }
        } else {
            // still before the next edge
            voltage = currentBit ? highlevel : lowlevel;
        }
        voltage += dist_noise(mt_noise);
        input[i] = voltage;
    }

    // input voltage vector fully assembled
    qDebug() << "Eye calculation: input data generated";

    // calculate impulse response of trace
    auto tdr = new Math::TDR();
    // default configuration of TDR is lowpass with automatic DC, which is exactly what we need

    // TDR calculation happens in background thread, need to wait for emitted signal
    volatile bool TDRdone = false;

    double eyeTimeShift = 0;

    std::vector<double> convolutionData;
    auto conn = connect(tdr, &Math::TDR::outputSamplesChanged, [&](){
        if(!TDRdone) {
            // determine how long the impulse response is
            auto samples = tdr->numSamples();
            auto length = tdr->getSample(samples - 1).x;

            // determine average delay
            auto total_step = tdr->getStepResponse(samples - 1);
            for(unsigned int i=0;i<samples;i++) {
                auto step = tdr->getStepResponse(i);
                if(abs(total_step - step) <= abs(step)) {
                    // mid point reached
                    eyeTimeShift = tdr->getSample(i).x;
                    break;
                }
            }

            auto scale = timestep / (length / (samples - 1));
            unsigned long convolutedSize = length / timestep;
            if(convolutedSize > input.size()) {
                // impulse response is longer than what we display, truncate
                convolutedSize = input.size();
            }
            convolutionData.resize(convolutedSize);
            for(unsigned long i=0;i<convolutedSize;i++) {
                auto x = i*timestep;
                convolutionData[i] = tdr->getInterpolatedSample(x).y.real() * scale;
            }
            TDRdone = true;
        }
    });
    // assigning the trace starts the TDR calculation
    tdr->assignInput(trace);

    // wait for the TDR calculation to be done
    while(!TDRdone) {
        std::this_thread::sleep_for(20ms);
    };
    disconnect(conn);
    delete tdr;

    eyeTimeShift += (risetime + falltime) / 4;
    eyeTimeShift += 0.5 / datarate;
    int eyeXshift = eyeTimeShift / timestep;

    qDebug() << "Eye calculation: TDR calculation done";

    // calculate voltage at top and bottom of diagram for y binning to pixels
    auto eyeRange = highlevel - lowlevel;
    auto topValue = highlevel + eyeRange * yOverrange;
    auto bottomValue = lowlevel - eyeRange * yOverrange;

    unsigned int highestIntensity = 0;

    qDebug() << "Convolve via FFT start";
    std::vector<std::complex<double>> inVec;
    std::vector<std::complex<double>> impulseVec;
    std::vector<std::complex<double>> outVec;
    for(auto i : input) {
        inVec.push_back(i);
    }
    for(auto i : convolutionData) {
        impulseVec.push_back(i);
    }
    impulseVec.resize(inVec.size(), 0.0);
    outVec.resize(inVec.size());
    Fft::convolve(inVec, impulseVec, outVec);
    qDebug() << "Convolve via FFT stop";

    auto addLine = [&](int x0, int y0, int x1, int y1, bool skipFirst = true) {
        bool first = true;
        auto putpixel = [&](int x, int y) {
            if(skipFirst && first) {
                first = false;
                return;
            }
            if(x < 0 || x >= width || y < 0 || y >= height) {
                return;
            }
            auto &bin = (*workingBuffer)[x][y];
            bin++;
            if(bin > highestIntensity) {
                highestIntensity = bin;
            }
        };

        int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2; /* error value e_xy */

        for (;;){  /* loop */
            putpixel (x0,y0);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
            if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
        }
    };

    // got the input data and the convolution data, calculate output
    int lastyBin;
    for(unsigned int i=width;i<input.size();i++) {
//        double voltage = 0;
//        for(unsigned j=0;j<convolutionData.size();j++) {
//            double inputValue = i >= j ? input[i-j] : input[0];
//            voltage += convolutionData[j] * inputValue;
//        }
        double voltage = outVec[i].real();
        int yBin = Util::Scale<double>(voltage, bottomValue, topValue, height-1, 0);
        // increment pixel bin
        if(yBin < 0) {
            yBin = 0;
        } else if(yBin >= height) {
            yBin = height - 1;
        }

        auto xlast = (i-1-eyeXshift)%width;
        auto xnow = (i-eyeXshift)%width;
        if(xnow > xlast && i > width) {
            addLine(xlast, lastyBin, xnow, yBin, xlast > 0);
        }
        lastyBin = yBin;
        emit updatePercent(100 * i / input.size());
    }

    qDebug() << "Eye calculation: Convolution done";

    // normalize intensity
    for(auto &y : *workingBuffer) {
        for(auto &v : y) {
            v /= highestIntensity;
        }
    }
    emit updatePercent(100);
    // switch buffers
    auto buf = finishedBuffer;
    finishedBuffer = workingBuffer;
    workingBuffer = buf;
    updating = false;
    emit updateDone();
}

EyeDiagramPlot::EyeDiagramPlot(QDialog *dialog)
{
    Q_UNUSED(dialog)
}

void EyeDiagramPlot::setDialog(EyeDiagramDialog *dialog)
{
    this->dialog = dialog;
}

void EyeDiagramPlot::paintEvent(QPaintEvent *event)
{
    auto &pref = Preferences::getInstance();
    QPainter p(this);
    p.setBackground(QBrush(pref.Graphs.Color.background));
    p.fillRect(0, 0, width(), height(), QBrush(pref.Graphs.Color.background));
    if(!dialog) {
        return;
    }
    for(unsigned int i=0;i<width();i++) {
        for(unsigned int j=0;j<height();j++) {
            auto value = dialog->getIntensity(i, j);
            if(isnan(value) || value == 0) {
                // do not paint, just leave the background shining through
                continue;
            }
            auto pen = QPen(Util::getIntensityGradeColor(value));
            pen.setCosmetic(true);
            p.setPen(pen);
            p.drawPoint(i, j);
        }
    }
}
