#include "eyediagramdialog.h"
#include "ui_eyediagramdialog.h"
#include "Util/prbs.h"
#include "Traces/Math/tdr.h"
#include "Util/util.h"
#include "preferences.h"
#include "Traces/fftcomplex.h"
#include "Traces/traceaxis.h"
#include "unit.h"

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
    setAttribute(Qt::WA_DeleteOnClose);

    workingBuffer = &eyeBuffer[0];
    finishedBuffer = &eyeBuffer[1];

    updating = false;
    firstUpdate = true;

    trace = nullptr;

    tdr = new Math::TDR();

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

    connect(this, &EyeDiagramDialog::calculationStatus, ui->status, &QLabel::setText, Qt::QueuedConnection);
    connect(ui->update, &QPushButton::clicked, this, &EyeDiagramDialog::triggerUpdate);
    connect(this, &EyeDiagramDialog::updateDone, ui->widget, qOverload<>(&QWidget::update));

    connect(ui->traceSelector, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
        trace = qvariant_cast<Trace*>(ui->traceSelector->itemData(ui->traceSelector->currentIndex()));
        tdr->assignInput(trace);
        ui->update->setEnabled(true);
    });

    connect(tdr, &Math::TDR::outputSamplesChanged, [=](){
        if(ui->updateOnTraceChange->isChecked() || firstUpdate) {
            triggerUpdate();
            firstUpdate = false;
        }
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
    while(updating) {
        std::this_thread::sleep_for(20ms);
    }
    delete tdr;
    delete ui;
}

unsigned int EyeDiagramDialog::getCalculatedPixelsX()
{
    return finishedBuffer->size();
}

unsigned int EyeDiagramDialog::getCalculatedPixelsY()
{
    if(getCalculatedPixelsX() > 0) {
        return (*finishedBuffer)[0].size();
    } else {
        return 0;
    }
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

double EyeDiagramDialog::displayedTime()
{
    return 2 * 1.0/ui->datarate->value();
}

double EyeDiagramDialog::minGraphVoltage()
{
    auto highlevel = ui->highLevel->value();
    auto lowlevel = ui->lowLevel->value();
    auto eyeRange = highlevel - lowlevel;
    return lowlevel - eyeRange * yOverrange;
}

double EyeDiagramDialog::maxGraphVoltage()
{
    auto highlevel = ui->highLevel->value();
    auto lowlevel = ui->lowLevel->value();
    auto eyeRange = highlevel - lowlevel;
    return highlevel + eyeRange * yOverrange;
}

bool EyeDiagramDialog::triggerUpdate()
{
    update(ui->widget->eyeWidth(), ui->widget->eyeHeight());
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
    emit calculationStatus("Starting calculation...");
    if(!trace) {
        emit calculationStatus("No trace assigned");
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
    bool linearEdge = ui->fallrisetype->currentIndex() == 0;
    unsigned int patternbits = ui->patternLength->currentIndex() + 2;
    unsigned int cycles = ui->displayedCycles->value() + 1; // first cycle will not be displayed

    // sanity check values
    if(datarate >= trace->getSample(trace->numSamples() - 1).x) {
        emit calculationStatus("Data rate too high");
        updating = false;
        return;
    }
    if(datarate <= 0) {
        emit calculationStatus("Data rate too low");
        updating = false;
        return;
    }
    if(risetime > 0.3 * 1.0 / datarate) {
        emit calculationStatus("Rise time too high");
        updating = false;
        return;
    }
    if(falltime > 0.3 * 1.0 / datarate) {
        emit calculationStatus("Fall time too high");
        updating = false;
        return;
    }
    if(jitter > 0.3 * 1.0 / datarate) {
        emit calculationStatus("Jitter too high");
        updating = false;
        return;
    }

    qDebug() << "Eye calculation: input values okay";

    // resize working buffer
    workingBuffer->clear();
    workingBuffer->resize(width);
    for(auto& y : *workingBuffer) {
        y.resize(height, 0.0);
    }

    emit calculationStatus("Generating PRBS sequence...");

    // calculate timestep
    double timestep = displayedTime() / (width);

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
    std::vector<std::complex<double>> inVec(width * cycles, 0.0);

    unsigned int bitcnt = 1;
    double transitionTime = -10; // assume that we start with a settled input, last transition was "long" ago
    for(unsigned int i=0;i<inVec.size();i++) {
        double time = i*timestep;
        double voltage;
        if(time >= transitionTime) {
            // currently within a bit transition
            double edgeTime = 0;
            double expTimeConstant;
            if(!currentBit && nextBit) {
                edgeTime = risetime;
            } else if(currentBit && !nextBit) {
                edgeTime = falltime;
            }
            if(linearEdge) {
                // edge is modeled as linear rise/fall
                // increase slightly to account for typical 10/90% fall/rise time
                edgeTime *= 1.25;
            } else {
                // edge is modeled as exponential rise/fall. Adjust time constant to match
                // selected rise/fall time (with 10-90% signal rise/fall within specified time)
                expTimeConstant = edgeTime / 2.197224577;
                edgeTime = 6 * expTimeConstant; // after six time constants, 99.7% of signal movement has happened
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
                double from = currentBit ? highlevel : lowlevel;
                double to = nextBit ? highlevel : lowlevel;
                if(linearEdge) {
                    double edgeRatio = timeSinceEdge / edgeTime;
                    voltage = from * (1.0 - edgeRatio) + to * edgeRatio;
                } else {
                    voltage = from + (1.0 - exp(-timeSinceEdge/expTimeConstant)) * (to - from);
                }
            }
        } else {
            // still before the next edge
            voltage = currentBit ? highlevel : lowlevel;
        }
        voltage += dist_noise(mt_noise);
        inVec[i] = voltage;
    }

    // input voltage vector fully assembled
    qDebug() << "Eye calculation: input data generated";

    emit calculationStatus("Extracting impulse response...");

    // calculate impulse response of trace
    double eyeTimeShift = 0;
    std::vector<std::complex<double>> impulseVec;
    // determine how long the impulse response is
    auto samples = tdr->numSamples();
    if(samples == 0) {
        // TDR calculation not yet done, unable to update
        updating = false;
        emit calculationStatus("No time-domain data from trace");
        return;
    }
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

    unsigned long convolutedSize = length / timestep;
    if(convolutedSize > inVec.size()) {
        // impulse response is longer than what we display, truncate
        convolutedSize = inVec.size();
    }
    impulseVec.resize(convolutedSize);
    /*
     *  we can't use the impulse response directly because we most likely need samples inbetween
     * the calculated values. Interpolation is available but if our sample spacing here is much
     * wider than the impulse response data, we might miss peaks (or severely miscalculate their
     * amplitude.
     * Instead, the step response is interpolated and the impulse response determined by deriving
     * it from the interpolated step response data. As the step response is the integrated imulse
     * response data, we can't miss narrow peaks that way.
     */
    double lastStepResponse = 0.0;
    for(unsigned long i=0;i<convolutedSize;i++) {
        auto x = i*timestep;
        auto step = tdr->getInterpolatedStepResponse(x);
        impulseVec[i] = step - lastStepResponse;
        lastStepResponse = step;
    }

    eyeTimeShift += (risetime + falltime) * 1.25 / 4;
    eyeTimeShift += 0.5 / datarate;
    int eyeXshift = eyeTimeShift / timestep;

    qDebug() << "Eye calculation: TDR calculation done";

    emit calculationStatus("Performing convolution...");

    unsigned int highestIntensity = 0;

    qDebug() << "Convolve via FFT start";
    std::vector<std::complex<double>> outVec;
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

    emit calculationStatus("Creating intensity bitmap...");

    // got the input data and the convolution data, calculate output
    int lastyBin;
    for(unsigned int i=width;i<inVec.size();i++) {
        double voltage = outVec[i].real();
        int yBin = Util::Scale<double>(voltage, minGraphVoltage(), maxGraphVoltage(), height-1, 0);
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
    }

    qDebug() << "Eye calculation: Convolution done";

    // normalize intensity
    for(auto &y : *workingBuffer) {
        for(auto &v : y) {
            v /= highestIntensity;
        }
    }
    // switch buffers
    auto buf = finishedBuffer;
    finishedBuffer = workingBuffer;
    workingBuffer = buf;
    updating = false;

    emit calculationStatus("Eye calculation complete");
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

unsigned int EyeDiagramPlot::eyeWidth()
{
    return width() - leftSpace() - rightSpace();
}

unsigned int EyeDiagramPlot::eyeHeight()
{
    return height() - topSpace() - bottomSpace();
}

unsigned int EyeDiagramPlot::leftSpace()
{
    auto &pref = Preferences::getInstance();
    return pref.Graphs.fontSizeAxis * 5.5;
}

unsigned int EyeDiagramPlot::bottomSpace()
{
    auto &pref = Preferences::getInstance();
    return  pref.Graphs.fontSizeAxis * 3;
}

void EyeDiagramPlot::paintEvent(QPaintEvent *event)
{
    if(!dialog) {
        return;
    }

    auto &pref = Preferences::getInstance();
    int plotAreaLeft = leftSpace();
    int plotAreaWidth = width() - leftSpace() - rightSpace();
    int plotAreaTop = topSpace();
    int plotAreaHeight = height() - topSpace() - bottomSpace();

    QPainter p(this);
    p.setBackground(QBrush(pref.Graphs.Color.background));
    p.fillRect(0, 0, width(), height(), QBrush(pref.Graphs.Color.background));

    auto pen = QPen(pref.Graphs.Color.axis, 0);
    pen.setCosmetic(true);
    p.setPen(pen);
    auto plotRect = QRect(plotAreaLeft, plotAreaTop, plotAreaWidth + 1, plotAreaHeight + 1);
    p.drawRect(plotRect);

    // Y axis
    QString labelY = "Voltage";
    auto font = p.font();
    font.setPixelSize(pref.Graphs.fontSizeAxis);
    p.setFont(font);
    p.setPen(QPen(pref.Graphs.Color.axis, 1));
    p.save();
    p.translate(0, height()-bottomSpace());
    p.rotate(-90);
    p.drawText(QRect(0, 0, height()-bottomSpace(), pref.Graphs.fontSizeAxis*1.5), Qt::AlignHCenter, labelY);
    p.restore();

    XAxis axis;
    axis.set(XAxis::Type::Time, false, true, dialog->minGraphVoltage(), dialog->maxGraphVoltage(), 10);
    // draw ticks
    if(axis.getTicks().size() > 0) {
        // this only works for evenly distributed ticks:
        auto max = qMax(abs(axis.getTicks().front()), abs(axis.getTicks().back()));
        double step;
        if(axis.getTicks().size() >= 2) {
            step = abs(axis.getTicks()[0] - axis.getTicks()[1]);
        } else {
            // only one tick, set arbitrary number of digits
            step = max / 1000;
        }
        int significantDigits = floor(log10(max)) - floor(log10(step)) + 1;

        for(unsigned int j = 0; j < axis.getTicks().size(); j++) {
            auto yCoord = axis.transform(axis.getTicks()[j], plotAreaTop + plotAreaHeight, plotAreaTop);
            p.setPen(QPen(pref.Graphs.Color.axis, 1));
            // draw tickmark on axis
            auto tickStart = plotAreaLeft;
            auto tickLen = -2;
            p.drawLine(tickStart, yCoord, tickStart + tickLen, yCoord);
            QString unit = "";
            QString prefix = " ";
            if(pref.Graphs.showUnits) {
                unit = "V";
                prefix = "um ";
            }
            auto tickValue = Unit::ToString(axis.getTicks()[j], unit, prefix, significantDigits);
            p.drawText(QRectF(0, yCoord - pref.Graphs.fontSizeAxis/2 - 2, tickStart + 2 * tickLen, pref.Graphs.fontSizeAxis), Qt::AlignRight, tickValue);

            // tick lines
            if(yCoord == plotAreaTop || yCoord == plotAreaTop + plotAreaHeight) {
                // skip tick lines right on the plot borders
                continue;
            }
            // only draw tick lines for primary axis
            if (pref.Graphs.Color.Ticks.Background.enabled) {
                if (j%2)
                {
                    int yCoordTop = axis.transform(axis.getTicks()[j], plotAreaTop, plotAreaTop + plotAreaHeight);
                    int yCoordBot = axis.transform(axis.getTicks()[j-1], plotAreaTop, plotAreaTop + plotAreaHeight);
                    if(yCoordTop > yCoordBot) {
                        auto buf = yCoordBot;
                        yCoordBot = yCoordTop;
                        yCoordTop = buf;
                    }
                    p.setBrush(pref.Graphs.Color.Ticks.Background.background);
                    p.setPen(pref.Graphs.Color.Ticks.Background.background);
                    auto rect = QRect(plotAreaLeft+1, yCoordTop+1, plotAreaWidth-2, yCoordBot-yCoordTop-2);
                    p.drawRect(rect);
                }
            }
            p.setPen(QPen(pref.Graphs.Color.Ticks.divisions, 0.5, Qt::DashLine));
            p.drawLine(plotAreaLeft, yCoord, plotAreaLeft + plotAreaWidth, yCoord);
        }
    }

    // use the XY-plot axes for tick calculation
    axis.set(XAxis::Type::Time, false, true, 0, dialog->displayedTime(), 10);

    // X axis name
    p.drawText(QRect(plotAreaLeft, height()-pref.Graphs.fontSizeAxis*1.5, plotAreaWidth, pref.Graphs.fontSizeAxis*1.5), Qt::AlignHCenter, axis.TypeToName());

    // draw X axis ticks
    if(axis.getTicks().size() >= 1) {
        // draw X ticks
        int significantDigits;
        // this only works for evenly distributed ticks:
        auto max = qMax(abs(axis.getTicks().front()), abs(axis.getTicks().back()));
        double step;
        if(axis.getTicks().size() >= 2) {
            step = abs(axis.getTicks()[0] - axis.getTicks()[1]);
        } else {
            // only one tick, set arbitrary number of digits
            step = max / 1000;
        }
        significantDigits = floor(log10(max)) - floor(log10(step)) + 1;
        QString prefixes = "fpnum kMG";
        QString unit = "";
        if(pref.Graphs.showUnits) {
            unit = axis.Unit();
        }
        QString commonPrefix = QString();
        int lastTickLabelEnd = 0;
        for(auto t : axis.getTicks()) {
            auto xCoord = axis.transform(t, plotAreaLeft, plotAreaLeft + plotAreaWidth);
            p.setPen(QPen(pref.Graphs.Color.axis, 1));
            p.drawLine(xCoord, plotAreaTop + plotAreaHeight, xCoord, plotAreaTop + plotAreaHeight + 2);
            if(xCoord != plotAreaLeft && xCoord != plotAreaLeft + plotAreaWidth) {
                p.setPen(QPen(pref.Graphs.Color.Ticks.divisions, 0.5, Qt::DashLine));
                p.drawLine(xCoord, plotAreaTop, xCoord, plotAreaTop + plotAreaHeight);
            }
            if(xCoord - 40 <= lastTickLabelEnd) {
                // would overlap previous tick label, skip
                continue;
            }
            auto tickValue = Unit::ToString(t, unit, prefixes, significantDigits);
            p.setPen(QPen(pref.Graphs.Color.axis, 1));
            QRect bounding;
            p.drawText(QRect(xCoord - pref.Graphs.fontSizeAxis*2, plotAreaTop + plotAreaHeight + 5, pref.Graphs.fontSizeAxis*4,
                             pref.Graphs.fontSizeAxis), Qt::AlignHCenter, tickValue, &bounding);
            lastTickLabelEnd = bounding.x() + bounding.width();
        }
    }

    if(dialog->getCalculatedPixelsX() == 0 || dialog->getCalculatedPixelsY() == 0) {
        // no eye data
        return;
    }

    // eye data is normally calculated to match the displayed pixels in this widget.
    // But the window size mighe have been adjusted since the last eye calculation.
    // Use scale factors until the eye data is updated
    double xScale = (double) plotAreaWidth / dialog->getCalculatedPixelsX();
    double yScale = (double) plotAreaHeight / dialog->getCalculatedPixelsY();

    for(unsigned int i=0;i<plotAreaWidth;i++) {
        for(unsigned int j=0;j<plotAreaHeight;j++) {
            auto value = dialog->getIntensity(i / xScale, j / yScale);
            if(isnan(value) || value == 0) {
                // do not paint, just leave the background shining through
                continue;
            }
            auto pen = QPen(Util::getIntensityGradeColor(value));
            pen.setCosmetic(true);
            p.setPen(pen);
            p.drawPoint(plotAreaLeft + i + 1, plotAreaTop + j + 1);
        }
    }
}
