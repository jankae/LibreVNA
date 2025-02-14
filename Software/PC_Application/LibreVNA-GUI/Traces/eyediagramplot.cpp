#include "eyediagramplot.h"

#include "ui_eyediagrameditdialog.h"
#include "unit.h"
#include "Util/prbs.h"
#include "Util/util.h"
#include "fftcomplex.h"
#include "preferences.h"
#include "appwindow.h"

#include <random>
#include <thread>
#include <chrono>

#include <QFileDialog>
#include <QPainter>
#include <QPushButton>

using namespace std::chrono_literals;

EyeDiagramPlot::EyeDiagramPlot(TraceModel &model, QWidget *parent)
    : TracePlot(model, parent),
      trace(nullptr),
      xSamples(200),
      datarate(100000000),
      highlevel(1.0),
      lowlevel(0.0),
      bitsPerSymbol(1),
      risetime(0.000000001),
      falltime(0.000000001),
      noise(0.01),
      jitter(0.0000000001),
      linearEdge(true),
      patternbits(9),
      cycles(200),
      traceBlurring(2)
{
    plotAreaTop = 0;
    plotAreaLeft = 0;
    plotAreaWidth = 0;
    plotAreaBottom = 0;

    tdr = new Math::TDR;

    calcData = &data[0];
    displayData = &data[1];

    xAxis.set(XAxis::Type::Time, false, true, 0, 0.000001, 10, true);
    yAxis.set(YAxis::Type::Real, false, true, -1, 1, 10, true);
    initializeTraceInfo();

    destructing = false;
    thread = new EyeThread(*this);
    thread->start(EyeThread::Priority::LowestPriority);

    connect(tdr, &Math::TDR::outputSamplesChanged, this, &EyeDiagramPlot::triggerUpdate);

    replot();
}

EyeDiagramPlot::~EyeDiagramPlot()
{
    // tell thread to exit
    destructing = true;
    semphr.release();
    thread->wait();
    delete thread;
    delete tdr;
}

void EyeDiagramPlot::enableTrace(Trace *t, bool enabled)
{
    bool already_enabled = trace == t;
    if(already_enabled == enabled) {
        // ignore, the requested condition is already fulfilled
        return;
    }
    if(enabled) {
        // only one trace at a time is allowed, disable all others
        for(auto t : traces) {
            if(t.second) {
                enableTrace(t.first, false);
                break;
            }
        }
    }
    TracePlot::enableTrace(t, enabled);
    if(enabled) {
        trace = t;
        tdr->assignInput(trace->getLastMath());
        connect(trace, &Trace::lastMathChanged, this, [=](){
            tdr->assignInput(trace->getLastMath());
        });
    } else {
        if(trace) {
            disconnect(trace, &Trace::lastMathChanged, this, nullptr);
            tdr->removeInput();
            std::lock_guard<std::mutex> calc(calcMutex);
            displayData->clear();
            calcData->clear();
        }
        trace = nullptr;
    }
}

void EyeDiagramPlot::replot()
{
    if(xAxis.getAutorange()) {
        xAxis.set(xAxis.getType(), false, true, 0, calculatedTime(), 10, false);
    }
    if(yAxis.getAutorange()) {
        yAxis.set(yAxis.getType(), false, true, minDisplayVoltage(), maxDisplayVoltage(), 10, false);
    }
    TracePlot::replot();
}

void EyeDiagramPlot::move(const QPoint &vect)
{
    if(!xAxis.getLog()) {
        // can only move axis in linear mode
        // calculate amount of movement
        double distance = xAxis.inverseTransform(vect.x(), 0, plotAreaWidth) - xAxis.getRangeMin();
        xAxis.set(xAxis.getType(), false, false, xAxis.getRangeMin() - distance, xAxis.getRangeMax() - distance, xAxis.getDivs(), xAxis.getAutoDivs());
    }
    if(!yAxis.getLog()) {
        // can only move axis in linear mode
        // calculate amount of movement
        double distance = yAxis.inverseTransform(vect.y(), 0, plotAreaTop - plotAreaBottom) - yAxis.getRangeMin();
        yAxis.set(yAxis.getType(), false, false, yAxis.getRangeMin() - distance, yAxis.getRangeMax() - distance, yAxis.getDivs(), yAxis.getAutoDivs());
    }
    replot();
}

void EyeDiagramPlot::zoom(const QPoint &center, double factor, bool horizontally, bool vertically)
{
    if(horizontally && !xAxis.getLog()) {
        // can only zoom axis in linear mode
        // calculate center point
        double cp = xAxis.inverseTransform(center.x(), plotAreaLeft, plotAreaLeft + plotAreaWidth);
        double min = ((xAxis.getRangeMin() - cp) * factor) + cp;
        double max = ((xAxis.getRangeMax() - cp) * factor) + cp;
        xAxis.set(xAxis.getType(), false, false, min, max, xAxis.getDivs(), xAxis.getAutoDivs());
    }
    if(vertically) {
        // can only move axis in linear mode
        // calculate center point
        double cp = yAxis.inverseTransform(center.y(), plotAreaBottom, plotAreaTop);
        double min = ((yAxis.getRangeMin() - cp) * factor) + cp;
        double max = ((yAxis.getRangeMax() - cp) * factor) + cp;
        yAxis.set(yAxis.getType(), false, false, min, max, yAxis.getDivs(), yAxis.getAutoDivs());
    }
    replot();
}

void EyeDiagramPlot::setAuto(bool horizontally, bool vertically)
{
    if(horizontally) {
        xAxis.set(xAxis.getType(), xAxis.getLog(), true, xAxis.getRangeMin(), xAxis.getRangeMax(), xAxis.getDivs(), xAxis.getAutoDivs());
    }
    if(vertically) {
        yAxis.set(yAxis.getType(), yAxis.getLog(), true, yAxis.getRangeMin(), yAxis.getRangeMax(), yAxis.getDivs(), yAxis.getAutoDivs());
    }
    replot();
}

void EyeDiagramPlot::fromJSON(nlohmann::json j)
{
    auto jX = j["XAxis"];
    bool xAuto = jX.value("autorange", xAxis.getAutorange());
    double xMin = jX.value("min", xAxis.getRangeMin());
    double xMax = jX.value("max", xAxis.getRangeMax());
    double xDivs = jX.value("divs", xAxis.getDivs());
    // older formats specified the spacing instead of the number of divisions
    if(jX.contains("div")) {
        xDivs = (xMax - xMin) / jX.value("div", (xMax - xMin) / xDivs);
    }
    auto xautodivs = jX.value("autoDivs", false);
    xAxis.set(xAxis.getType(), false, xAuto, xMin, xMax, xDivs, xautodivs);

    auto jY = j["YAxis"];
    bool yAuto = jY.value("autorange", yAxis.getAutorange());
    double yMin = jY.value("min", yAxis.getRangeMin());
    double yMax = jY.value("max", yAxis.getRangeMax());
    double yDivs = jY.value("divs", yAxis.getDivs());
    // older formats specified the spacing instead of the number of divisions
    if(jY.contains("div")) {
        yDivs = (yMax - yMin) / jY.value("div", (yMax - yMin) / yDivs);
    }
    auto yautodivs = jY.value("autoDivs", false);
    yAxis.set(yAxis.getType(), false, yAuto, yMin, yMax, yDivs, yautodivs);

    datarate = j.value("datarate", datarate);
    risetime = j.value("risetime", risetime);
    falltime = j.value("falltime", falltime);
    linearEdge = j.value("linearEdge", linearEdge);
    highlevel = j.value("highlevel", highlevel);
    lowlevel = j.value("lowlevel", lowlevel);
    bitsPerSymbol = j.value("bitPerSymbol", bitsPerSymbol);
    noise = j.value("noise", noise);
    jitter = j.value("jitter", jitter);
    patternbits = j.value("patternBits", patternbits);
    cycles = j.value("cycles", cycles);
    xSamples = j.value("xSamples", xSamples);
    traceBlurring = j.value("traceBlurring", traceBlurring);

    for(unsigned int hash : j["traces"]) {
        // attempt to find the traces with this hash
        bool found = false;
        for(auto t : model.getTraces()) {
            if(t->toHash() == hash) {
                enableTrace(t, true);
                found = true;
                break;
            }
        }
        if(!found) {
            qWarning() << "Unable to find trace with hash" << hash;
        }
    }
}

nlohmann::json EyeDiagramPlot::toJSON()
{
    nlohmann::json j;
    nlohmann::json jX;
    jX["autorange"] = yAxis.getAutorange();
    jX["min"] = xAxis.getRangeMin();
    jX["max"] = xAxis.getRangeMax();
    jX["divs"] = xAxis.getDivs();
    jX["autoDivs"] = xAxis.getAutoDivs();
    j["XAxis"] = jX;
    nlohmann::json jY;
    jY["autorange"] = yAxis.getAutorange();
    jY["min"] = yAxis.getRangeMin();
    jY["max"] = yAxis.getRangeMax();
    jY["divs"] = yAxis.getDivs();
    jY["autoDivs"] = yAxis.getAutoDivs();
    j["YAxis"] = jY;
    nlohmann::json jtraces;
    for(auto t : traces) {
        if(t.second) {
            jtraces.push_back(t.first->toHash());
        }
    }
    j["traces"] = jtraces;

    j["datarate"] = datarate;
    j["risetime"] = risetime;
    j["falltime"] = falltime;
    j["linearEdge"] = linearEdge;
    j["highlevel"] = highlevel;
    j["lowlevel"] = lowlevel;
    j["bitPerSymbol"] = bitsPerSymbol;
    j["noise"] = noise;
    j["jitter"] = jitter;
    j["patternBits"] = patternbits;
    j["cycles"] = cycles;
    j["xSamples"] = xSamples;
    j["traceBlurring"] = traceBlurring;
    return j;
}

void EyeDiagramPlot::axisSetupDialog()
{
    auto d = new QDialog(this);
    d->setAttribute(Qt::WA_DeleteOnClose);
    auto ui = new Ui::EyeDiagramEditDialog;
    ui->setupUi(d);

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

    ui->Xmin->setUnit("s");
    ui->Xmin->setPrefixes("pnum ");
    ui->Xmin->setPrecision(5);

    ui->Xmax->setUnit("s");
    ui->Xmax->setPrefixes("pnum ");
    ui->Xmax->setPrecision(5);

    ui->Ymin->setUnit("V");
    ui->Ymin->setPrefixes("um ");
    ui->Ymin->setPrecision(4);

    ui->Ymax->setUnit("V");
    ui->Ymax->setPrefixes("um ");
    ui->Ymax->setPrecision(4);

    // set initial values
    ui->datarate->setValue(datarate);
    ui->risetime->setValue(risetime);
    ui->falltime->setValue(falltime);
    ui->highLevel->setValue(highlevel);
    ui->lowLevel->setValue(lowlevel);
    ui->noise->setValue(noise);
    ui->jitter->setValue(jitter);

    ui->signalLevels->setCurrentIndex(bitsPerSymbol - 1);
    ui->patternLength->setCurrentIndex(patternbits - 2);
    ui->fallrisetype->setCurrentIndex(linearEdge ? 0 : 1);

    ui->displayedCycles->setValue(cycles);
    ui->pointsPerCycle->setValue(xSamples);
    ui->traceBlurring->setValue(traceBlurring);

    connect(ui->Xauto, &QCheckBox::toggled, [=](bool checked) {
        ui->Xmin->setEnabled(!checked);
        ui->Xmax->setEnabled(!checked);
        ui->Xdivs->setEnabled(!checked && !ui->XautoDivs->isChecked());
        ui->XautoDivs->setEnabled(!checked);
    });
    connect(ui->XautoDivs, &QCheckBox::toggled, [=](bool checked) {
        ui->Xdivs->setEnabled(!checked);
    });
    ui->Xauto->setChecked(xAxis.getAutorange());
    ui->Xmin->setValue(xAxis.getRangeMin());
    ui->Xmax->setValue(xAxis.getRangeMax());
    ui->Xdivs->setValue(xAxis.getDivs());

    connect(ui->Yauto, &QCheckBox::toggled, [=](bool checked) {
        ui->Ymin->setEnabled(!checked);
        ui->Ymax->setEnabled(!checked);
        ui->Ydivs->setEnabled(!checked && !ui->YautoDivs->isChecked());
        ui->YautoDivs->setEnabled(!checked);
    });
    connect(ui->YautoDivs, &QCheckBox::toggled, [=](bool checked) {
        ui->Ydivs->setEnabled(!checked);
    });
    ui->Yauto->setChecked(yAxis.getAutorange());
    ui->Ymin->setValue(yAxis.getRangeMin());
    ui->Ymax->setValue(yAxis.getRangeMax());
    ui->Ydivs->setValue(yAxis.getDivs());

    auto updateValues = [=](){
        std::lock_guard<std::mutex> guard(calcMutex);
        datarate = ui->datarate->value();
        risetime = ui->risetime->value();
        falltime = ui->falltime->value();
        highlevel = ui->highLevel->value();
        lowlevel = ui->lowLevel->value();
        noise = ui->noise->value();
        jitter = ui->jitter->value();

        bitsPerSymbol = ui->signalLevels->currentIndex() + 1;
        patternbits = ui->patternLength->currentIndex() + 2;
        linearEdge = ui->fallrisetype->currentIndex() == 0;

        cycles = ui->displayedCycles->value();
        xSamples = ui->pointsPerCycle->value();
        traceBlurring = ui->traceBlurring->value();

        xAxis.set(xAxis.getType(), false, ui->Xauto->isChecked(), ui->Xmin->value(), ui->Xmax->value(), ui->Xdivs->value(), ui->XautoDivs->isChecked());
        yAxis.set(yAxis.getType(), false, ui->Yauto->isChecked(), ui->Ymin->value(), ui->Ymax->value(), ui->Ydivs->value(), ui->YautoDivs->isChecked());
    };

    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, [=](){
        triggerUpdate();
        updateValues();
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, [=](){
        triggerUpdate();
        updateValues();
    });

    if(AppWindow::showGUI()) {
        d->show();
    }
}

void EyeDiagramPlot::updateContextMenu()
{
    contextmenu->clear();
    auto setup = new QAction("Setup...", contextmenu);
    connect(setup, &QAction::triggered, this, &EyeDiagramPlot::axisSetupDialog);
    contextmenu->addAction(setup);

    contextmenu->addSeparator();
    auto image = new QAction("Save image...", contextmenu);
    contextmenu->addAction(image);
    connect(image, &QAction::triggered, this, [=]() {
        auto filename = QFileDialog::getSaveFileName(nullptr, "Save plot image", "", "PNG image files (*.png)", nullptr, Preferences::QFileDialogOptions());
        if(filename.isEmpty()) {
            // aborted selection
            return;
        }
        if(filename.endsWith(".png")) {
            filename.chop(4);
        }
        filename += ".png";
        grab().save(filename);
    });

    contextmenu->addSection("Traces");
    // Populate context menu
    for(auto t : orderedTraces()) {
        if(!supported(t)) {
            continue;
        }
        auto action = new QAction(t->name(), contextmenu);
        action->setCheckable(true);
        if(traces[t]) {
            action->setChecked(true);
        }
        connect(action, &QAction::toggled, this, [=](bool active) {
            enableTrace(t, active);
        });
        contextmenu->addAction(action);
    }

    finishContextMenu();
}

bool EyeDiagramPlot::positionWithinGraphArea(const QPoint &p)
{
    return p.x() >= plotAreaLeft && p.x() <= plotAreaLeft + plotAreaWidth
            && p.y() >= plotAreaTop && p.y() <= plotAreaBottom;
}

void EyeDiagramPlot::draw(QPainter &p)
{
    auto &pref = Preferences::getInstance();

    auto w = p.window();
    auto yAxisSpace = pref.Graphs.fontSizeAxis * 5.5;
    auto xAxisSpace = pref.Graphs.fontSizeAxis * 3;
    plotAreaLeft = yAxisSpace;
    plotAreaWidth = w.width() - plotAreaLeft - 10;
    plotAreaTop = 10;
    plotAreaBottom = w.height() - xAxisSpace;

    p.setBackground(QBrush(pref.Graphs.Color.background));
    p.fillRect(0, 0, width(), height(), QBrush(pref.Graphs.Color.background));

    auto pen = QPen(pref.Graphs.Color.axis, 0);
    pen.setCosmetic(true);
    p.setPen(pen);
    auto plotRect = QRect(plotAreaLeft, plotAreaTop, plotAreaWidth + 1, plotAreaBottom - plotAreaTop + 1);
    p.drawRect(plotRect);

    // Y axis
    QString labelY = "Voltage";
    auto font = p.font();
    font.setPixelSize(pref.Graphs.fontSizeAxis);
    p.setFont(font);
    p.setPen(QPen(pref.Graphs.Color.axis, 1));
    p.save();
    p.translate(0, w.height()-xAxisSpace);
    p.rotate(-90);
    p.drawText(QRect(0, 0, w.height()-xAxisSpace, pref.Graphs.fontSizeAxis*1.5), Qt::AlignHCenter, labelY);
    p.restore();

    // draw ticks
    if(yAxis.getTicks().size() > 0) {
        // this only works for evenly distributed ticks:
        auto max = qMax(abs(yAxis.getTicks().front()), abs(yAxis.getTicks().back()));
        double step;
        if(yAxis.getTicks().size() >= 2) {
            step = abs(yAxis.getTicks()[0] - yAxis.getTicks()[1]);
        } else {
            // only one tick, set arbitrary number of digits
            step = max / 1000;
        }
        int significantDigits = floor(log10(max)) - floor(log10(step)) + 1;

        for(unsigned int j = 0; j < yAxis.getTicks().size(); j++) {
            auto yCoord = yAxis.transform(yAxis.getTicks()[j], plotAreaBottom, plotAreaTop);
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
            auto tickValue = Unit::ToString(yAxis.getTicks()[j], unit, prefix, significantDigits);
            p.drawText(QRectF(0, yCoord - pref.Graphs.fontSizeAxis/2 - 2, tickStart + 2 * tickLen, pref.Graphs.fontSizeAxis*1.5), Qt::AlignRight, tickValue);

            // tick lines
            if(yCoord == plotAreaTop || yCoord == plotAreaBottom) {
                // skip tick lines right on the plot borders
                continue;
            }
            // only draw tick lines for primary axis
            if (pref.Graphs.Color.Ticks.Background.enabled) {
                if (j%2)
                {
                    int yCoordTop = yAxis.transform(yAxis.getTicks()[j], plotAreaTop, plotAreaBottom);
                    int yCoordBot = yAxis.transform(yAxis.getTicks()[j-1], plotAreaTop, plotAreaBottom);
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

    // X axis name
    p.setPen(QPen(pref.Graphs.Color.axis, 1));
    p.drawText(QRect(plotAreaLeft, w.height()-pref.Graphs.fontSizeAxis*1.5, plotAreaWidth, pref.Graphs.fontSizeAxis*1.5), Qt::AlignHCenter, "Time");

    // draw X axis ticks
    if(xAxis.getTicks().size() >= 1) {
        // draw X ticks
        int significantDigits;
        // this only works for evenly distributed ticks:
        auto max = qMax(abs(xAxis.getTicks().front()), abs(xAxis.getTicks().back()));
        double step;
        if(xAxis.getTicks().size() >= 2) {
            step = abs(xAxis.getTicks()[0] - xAxis.getTicks()[1]);
        } else {
            // only one tick, set arbitrary number of digits
            step = max / 1000;
        }
        significantDigits = floor(log10(max)) - floor(log10(step)) + 1;
        QString prefixes = "fpnum kMG";
        QString unit = "";
        if(pref.Graphs.showUnits) {
            unit = xAxis.Unit();
        }
        int lastTickLabelEnd = 0;
        for(auto t : xAxis.getTicks()) {
            auto xCoord = xAxis.transform(t, plotAreaLeft, plotAreaLeft + plotAreaWidth);
            p.setPen(QPen(pref.Graphs.Color.axis, 1));
            p.drawLine(xCoord, plotAreaBottom, xCoord, plotAreaBottom + 2);
            if(xCoord != plotAreaLeft && xCoord != plotAreaLeft + plotAreaWidth) {
                p.setPen(QPen(pref.Graphs.Color.Ticks.divisions, 0.5, Qt::DashLine));
                p.drawLine(xCoord, plotAreaTop, xCoord, plotAreaBottom);
            }
            if(xCoord - 40 <= lastTickLabelEnd) {
                // would overlap previous tick label, skip
                continue;
            }
            auto tickValue = Unit::ToString(t, unit, prefixes, significantDigits);
            p.setPen(QPen(pref.Graphs.Color.axis, 1));
            QRect bounding;
            p.drawText(QRect(xCoord - pref.Graphs.fontSizeAxis*4, plotAreaBottom + 5, pref.Graphs.fontSizeAxis*8,
                             pref.Graphs.fontSizeAxis*1.5), Qt::AlignHCenter, tickValue, &bounding);
            lastTickLabelEnd = bounding.x() + bounding.width();
        }
    }

    if(displayData->size() >= 2) {
        std::lock_guard<std::mutex> guard(bufferSwitchMutex);
        if((*displayData)[0].y[0] == 0.0 && (*displayData)[0].y[1] == 0.0) {
            qDebug() << "detected null data, displaydata:" << displayData;
        }
        unsigned int pxWidth = plotAreaWidth;
        unsigned int pxHeight = plotAreaBottom - plotAreaTop;
        std::vector<std::vector<unsigned int>> bitmap;
        bitmap.resize(pxWidth);
        for(auto &y : bitmap) {
            y.resize(pxHeight, 0);
        }
        unsigned int highestIntensity = 0;
        unsigned int numTraces = (*displayData)[displayData->size()-1].y.size();

        auto addLine = [&](int x0, int y0, int x1, int y1, bool skipFirst = true) {
            bool first = true;
            auto putpixel = [&](int x, int y) {
                if(skipFirst && first) {
                    first = false;
                    return;
                }
                for(int i=-traceBlurring;i<=traceBlurring;i++) {
                    for(int j=-traceBlurring;j<=traceBlurring;j++) {
                        if(i*i+j*j <= traceBlurring*traceBlurring) {
                            if(x+i < 0 || x+i >= (int) pxWidth || y+j < 0 || y+j >= (int) pxHeight) {
                                return;
                            }
                            auto &bin = bitmap[x+i][y+j];
                            bin++;
                            if(bin > highestIntensity) {
                                highestIntensity = bin;
                            }
                        }
                    }
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

        // Assemble the bitmap
        for(unsigned int i=1;i<displayData->size();i++) {
            int x0 = xAxis.transform((*displayData)[i-1].x, 0, pxWidth);
            int x1 = xAxis.transform((*displayData)[i].x, 0, pxWidth);
            if((x0 < 0 && x1 < 0) || (x0 >= (int) pxWidth && x1 >= (int) pxWidth)) {
                // completely out of the frame
                continue;
            }
            for(unsigned int j=0;j<numTraces;j++) {
                int y0 = yAxis.transform((*displayData)[i-1].y[j], pxHeight, 0);
                int y1 = yAxis.transform((*displayData)[i].y[j], pxHeight, 0);
                addLine(x0, y0, x1, y1, i > 1);
            }
        }

        /*
         * Only a small amount of pixels will have a lot of traces of top of each other.
         * This would result in using mostly the colder colors in the intensity grading.
         *
         * Generate a histogram of pixel usage and create an adjustment curve to evenly
         * distribute all intensity colors
         */
        unsigned int hist[highestIntensity+1];
        memset(hist, 0, sizeof(hist));
        unsigned long total = 0;
        for(unsigned int i=1;i<pxWidth;i++) {
            for(unsigned int j=0;j<pxHeight;j++) {
                hist[bitmap[i][j]]++;
                total++;
            }
        }
        unsigned int sum = 0;
        double correctedCurve[highestIntensity+1];
        correctedCurve[0] = 0.0;
        total -= hist[0];
        for(unsigned int i=1;i<=highestIntensity;i++) {
            sum += hist[i];
            correctedCurve[i] = pow((double) sum / total, 2); // not totally even distribution, x^2 seems to look better
        }

        // draw the bitmap
        pen = QPen();
        pen.setCosmetic(true);
        for(unsigned int i=1;i<pxWidth;i++) {
            for(unsigned int j=0;j<pxHeight;j++) {
                if(bitmap[i][j] > 0) {
                    double value = correctedCurve[bitmap[i][j]];
                    pen.setColor(Util::getIntensityGradeColor(value));
                    p.setPen(pen);
                    p.drawPoint(plotAreaLeft + i + 1, plotAreaTop + j + 1);
                }
            }
        }
    } else {
        qDebug() << "Empty eye data, displaydata:" << displayData;
    }
    if(dropPending && supported(dropTrace)) {
        p.setOpacity(dropOpacity);
        p.setBrush(dropBackgroundColor);
        p.setPen(dropForegroundColor);
        // show drop area over whole plot
        p.drawRect(getDropRect());
        auto font = p.font();
        font.setPixelSize(20);
        p.setFont(font);
        p.setOpacity(1.0);
        p.setPen(dropSection == DropSection::OnPlot ? dropHighlightColor : dropForegroundColor);
        auto text = "Drop here to add\n" + dropTrace->name() + "\nto eye diagram";
        p.drawText(getDropRect(), Qt::AlignCenter, text);
    }
}

bool EyeDiagramPlot::supported(Trace *t)
{
    if(t->outputType() != Trace::DataType::Frequency) {
        // wrong domain
        return false;
    }
    if(t->isReflection()) {
        // can't work with reflection measurements
        return false;
    }
    return true;
}

QString EyeDiagramPlot::mouseText(QPoint pos)
{
    QString ret;
    if(positionWithinGraphArea(pos)) {
        // cursor within plot area
        QPointF coords = pixelToPlotValue(pos);
        int significantDigits = floor(log10(abs(xAxis.getRangeMax()))) - floor(log10((abs(xAxis.getRangeMax() - xAxis.getRangeMin())) / 1000.0)) + 1;
        ret += Unit::ToString(coords.x(), xAxis.Unit(), "fpnum kMG", significantDigits) + "\n";
        auto max = qMax(abs(yAxis.getRangeMax()), abs(yAxis.getRangeMin()));
        auto step = abs(yAxis.getRangeMax() - yAxis.getRangeMin()) / 1000.0;
        significantDigits = floor(log10(max)) - floor(log10(step)) + 1;
        ret += Unit::ToString(coords.y(), "V", yAxis.Prefixes(getModel().getSource()), significantDigits) + "\n";
    }
    return ret;
}

QPoint EyeDiagramPlot::plotValueToPixel(QPointF plotValue)
{
    QPoint p;
    p.setX(xAxis.transform(plotValue.x(), plotAreaLeft, plotAreaLeft + plotAreaWidth));
    p.setY(yAxis.transform(plotValue.y(), plotAreaBottom, plotAreaTop));
    return p;
}

QPointF EyeDiagramPlot::pixelToPlotValue(QPoint pixel)
{
    QPointF p;
    p.setX(xAxis.inverseTransform(pixel.x(), plotAreaLeft, plotAreaLeft + plotAreaWidth));
    p.setY(yAxis.inverseTransform(pixel.y(), plotAreaBottom, plotAreaTop));
    return p;
}

void EyeDiagramPlot::triggerUpdate()
{
    // trigger the thread
    semphr.release();
}

void EyeDiagramPlot::setStatus(QString s)
{
    status = s;
    emit statusChanged(s);
}

double EyeDiagramPlot::calculatedTime()
{
    return 2.0 / datarate;
}

double EyeDiagramPlot::minDisplayVoltage()
{
    auto eyeRange = highlevel - lowlevel;
    return lowlevel - eyeRange * yOverrange;
}

double EyeDiagramPlot::maxDisplayVoltage()
{
    auto eyeRange = highlevel - lowlevel;
    return highlevel + eyeRange * yOverrange;
}

void EyeThread::run()
{
    while(1) {
        eye.semphr.acquire();
        std::lock_guard<std::mutex> calc(eye.calcMutex);
        // clear possible additional semaphores
        eye.semphr.tryAcquire(eye.semphr.available());
        if(eye.destructing) {
            // Eye diagram object about to be deleted, exit thread
            qDebug() << "Eye thread exiting";
            return;
        }
        eye.setStatus("Starting calculation...");
        if(!eye.trace) {
            eye.setStatus("No trace assigned");
            continue;
        }

        qDebug() << "Starting eye diagram calculation";

        // sanity check values
        if(eye.datarate >= eye.trace->getSample(eye.trace->numSamples() - 1).x) {
            eye.setStatus("Data rate too high");
            continue;
        }
        if(eye.datarate <= 0) {
            eye.setStatus("Data rate too low");
            continue;
        }
        if(eye.jitter > 0.3 * 1.0 / eye.datarate) {
            eye.setStatus("Jitter too high");
            continue;
        }

        qDebug() << "Eye calculation: input values okay";

        // calculate timestep
        double timestep = eye.calculatedTime() / eye.xSamples;
        // reserve vector for input data
        std::vector<std::complex<double>> inVec(eye.xSamples * (eye.cycles + 1), 0.0); // needs to calculate one more cycle than required for the display (settling)

        // resize working buffer
        qDebug() << "Clearing old eye data, calcData:" << eye.calcData;
        eye.calcData->clear();
        eye.calcData->resize(eye.xSamples);
        for(auto& s : *eye.calcData) {
            s.y.resize(eye.cycles, 0.0);
        }

        eye.setStatus("Extracting impulse response...");

        // calculate impulse response of trace
        double eyeTimeShift = 0;
        std::vector<std::complex<double>> impulseVec;
        // determine how long the impulse response is
        auto samples = eye.tdr->numSamples();
        if(samples == 0) {
            // TDR calculation not yet done, unable to update
            eye.setStatus("No time-domain data from trace");
            continue;
        }
        auto length = eye.tdr->getSample(samples - 1).x;

        // determine average delay
        auto total_step = eye.tdr->getStepResponse(samples - 1);
        for(unsigned int i=0;i<samples;i++) {
            auto step = eye.tdr->getStepResponse(i);
            if(abs(total_step - step) <= abs(step)) {
                // mid point reached
                eyeTimeShift = eye.tdr->getSample(i).x;
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
            auto step = eye.tdr->getInterpolatedStepResponse(x);
            impulseVec[i] = step - lastStepResponse;
            lastStepResponse = step;
        }

        eyeTimeShift += (eye.risetime + eye.falltime) * 1.25 / 4;
        eyeTimeShift += 0.5 / eye.datarate;
        int eyeXshift = eyeTimeShift / timestep;

        qDebug() << "Eye calculation: TDR calculation done";

        eye.setStatus("Generating PRBS sequence...");

        auto prbs = PRBS(eye.patternbits);

        auto getNextLevel = [&]() -> unsigned int {
            unsigned int level = 0;
            for(unsigned int i=0;i<eye.bitsPerSymbol;i++) {
                level <<= 1;
                if(prbs.next()) {
                    level |= 0x01;
                }
            }
            return level;
        };

        auto levelToVoltage = [=](unsigned int level) -> double {
            unsigned int maxLevel = (0x01 << eye.bitsPerSymbol) - 1;
            return Util::Scale((double) level, 0.0, (double) maxLevel, eye.lowlevel, eye.highlevel);
        };

        unsigned int currentSignal = getNextLevel();
        unsigned int nextSignal = getNextLevel();

        // initialize random generator
        std::random_device rd1;
        std::mt19937 mt_noise(rd1());
        std::normal_distribution<> dist_noise(0, eye.noise);

        std::random_device rd2;
        std::mt19937 mt_jitter(rd2());
        std::normal_distribution<> dist_jitter(0, eye.jitter);

        unsigned int bitcnt = 1;
        double transitionTime = -10; // assume that we start with a settled input, last transition was "long" ago
        for(unsigned int i=0;i<inVec.size();i++) {
            double time = (i+eyeXshift)*timestep;
            double voltage;
            if(time >= transitionTime) {
                // last transition is over,
                // schedule the next transition
                voltage = levelToVoltage(nextSignal);
                // move on to the next bit
                currentSignal = nextSignal;
                nextSignal = getNextLevel();
                transitionTime = bitcnt * 1.0 / eye.datarate + dist_jitter(mt_jitter);
                bitcnt++;
            } else {
                // still before the next edge
                voltage = levelToVoltage(currentSignal);
            }
            inVec[i] = voltage;
        }

        // add fall/rise time
        for(unsigned int i=1;i<inVec.size();i++) {
            double last = inVec[i-1].real();
            double next = inVec[i].real();
            if(last == next) {
                // no change, nothing to do
                continue;
            }
            if(eye.linearEdge) {
                if(next > last) {
                    // rising edge
                    double max_rise = abs(eye.highlevel - eye.lowlevel) * timestep / (eye.risetime * 1.25);
                    if(next - last > max_rise) {
                        next = last + max_rise;
                    }
                } else {
                    // falling edge
                    double max_fall = abs(eye.highlevel - eye.lowlevel) * timestep / (eye.falltime * 1.25);
                    if(next - last < -max_fall) {
                        next = last - max_fall;
                    }
                }
            } else {
                // exponential edge
                // edge is modeled as exponential rise/fall. Adjust time constant to match
                // selected rise/fall time (with 10-90% signal rise/fall within specified time)
                auto expTimeConstant = (next > last ? eye.risetime : eye.falltime) / 2.197224577;
                if(expTimeConstant > 0) {
                    next = last + (1.0 - exp(-timestep/expTimeConstant)) * (next - last);
                }
            }
            inVec[i] = next;
        }

        // add noise
        for(auto &v : inVec) {
            v += dist_noise(mt_noise);
        }

        // input voltage vector fully assembled
        qDebug() << "Eye calculation: input data generated";

        eye.setStatus("Performing convolution...");

        qDebug() << "Convolve via FFT start";
        std::vector<std::complex<double>> outVec;
        impulseVec.resize(inVec.size(), 0.0);
        outVec.resize(inVec.size());
        Fft::convolve(inVec, impulseVec, outVec);
        qDebug() << "Convolve via FFT stop";

        // fill data from outVec
        for(unsigned int i=0;i<eye.xSamples;i++) {
            (*eye.calcData).at(i).x = i * timestep;
        }
        for(unsigned int i=eye.xSamples;i<inVec.size();i++) {
            unsigned int x = i % eye.xSamples;
            unsigned int y = i / eye.xSamples - 1;
            (*eye.calcData).at(x).y.at(y) = outVec[i].real();
        }

        qDebug() << "Eye calculation: Convolution done";

        {
            std::lock_guard<std::mutex> guard(eye.bufferSwitchMutex);
            // switch buffers
            qDebug() << "Switching diplay buffers, calcData:" << eye.calcData;
            auto buf = eye.displayData;
            eye.displayData = eye.calcData;
            eye.calcData = buf;
            if((*eye.displayData)[0].y[0] == 0.0 && (*eye.displayData)[0].y[1] == 0.0) {
                qDebug() << "detected null after eye calculation";
            }
            qDebug() << "Buffer switch complete, displayData:" << eye.displayData;
        }

        eye.setStatus("Eye calculation complete");
        eye.replot();
    }
}
