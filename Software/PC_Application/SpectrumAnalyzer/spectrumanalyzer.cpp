#include "spectrumanalyzer.h"

#include "unit.h"
#include "CustomWidgets/toggleswitch.h"
#include "Device/manualcontroldialog.h"
#include "Traces/tracemodel.h"
#include "tracewidgetsa.h"
#include "Traces/tracesmithchart.h"
#include "Traces/tracexyplot.h"
#include "Traces/traceimportdialog.h"
#include "CustomWidgets/tilewidget.h"
#include "CustomWidgets/siunitedit.h"
#include "Traces/Marker/markerwidget.h"
#include "Tools/impedancematchdialog.h"
#include "Calibration/calibrationtracedialog.h"
#include "ui_main.h"
#include "Device/firmwareupdatedialog.h"
#include "preferences.h"
#include "Generator/signalgenwidget.h"
#include "modehandler.h"

#include <QDockWidget>
#include <QDesktopWidget>
#include <QApplication>
#include <QActionGroup>
#include "CustomWidgets/informationbox.h"
#include <QDebug>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <math.h>
#include <QToolBar>
#include <QMenu>
#include <QToolButton>
#include <QActionGroup>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSettings>
#include <algorithm>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <iostream>
#include <fstream>
#include <QDateTime>

SpectrumAnalyzer::SpectrumAnalyzer(AppWindow *window, QString name)
    : Mode(window, name, "SA"),
      central(new TileWidget(traceModel, window))
{
    averages = 1;
    singleSweep = false;
    settings = {};
    normalize.active = false;
    normalize.measuring = false;
    normalize.points = 0;
    normalize.dialog.reset();

    traceModel.setSource(TraceModel::DataSource::SA);

    // Create default traces
    auto tPort1 = new Trace("Port1", Qt::yellow);
    tPort1->fromLivedata(Trace::LivedataType::Overwrite, Trace::LiveParameter::Port1);
    traceModel.addTrace(tPort1);
    auto tPort2 = new Trace("Port2", Qt::blue);
    tPort2->fromLivedata(Trace::LivedataType::Overwrite, Trace::LiveParameter::Port2);
    traceModel.addTrace(tPort2);

    auto traceXY = new TraceXYPlot(traceModel);
    traceXY->enableTrace(tPort1, true);
    traceXY->enableTrace(tPort2, true);
    traceXY->setYAxis(0, YAxis::Type::Magnitude, false, false, -120,0,10);
    traceXY->setYAxis(1, YAxis::Type::Disabled, false, true, 0,0,1);

    central->setPlot(traceXY);

    // Create menu entries and connections
    // Sweep toolbar
    auto tb_sweep = new QToolBar("Sweep");

    auto bSingle = new QPushButton("Single");
    bSingle->setToolTip("Single sweep");
    bSingle->setCheckable(true);
    connect(bSingle, &QPushButton::toggled, this, &SpectrumAnalyzer::SetSingleSweep);
    connect(this, &SpectrumAnalyzer::singleSweepChanged, bSingle, &QPushButton::setChecked);
    tb_sweep->addWidget(bSingle);

    auto eStart = new SIUnitEdit("Hz", " kMG", 6);
    // calculate width required with expected string length
    auto width = QFontMetrics(eStart->font()).width("3.00000GHz") + 15;
    eStart->setFixedWidth(width);
    eStart->setToolTip("Start frequency");
    connect(eStart, &SIUnitEdit::valueChanged, this, &SpectrumAnalyzer::SetStartFreq);
    connect(this, &SpectrumAnalyzer::startFreqChanged, eStart, &SIUnitEdit::setValueQuiet);
    tb_sweep->addWidget(new QLabel("Start:"));
    tb_sweep->addWidget(eStart);

    auto eCenter = new SIUnitEdit("Hz", " kMG", 6);
    eCenter->setFixedWidth(width);
    eCenter->setToolTip("Center frequency");
    connect(eCenter, &SIUnitEdit::valueChanged, this, &SpectrumAnalyzer::SetCenterFreq);
    connect(this, &SpectrumAnalyzer::centerFreqChanged, eCenter, &SIUnitEdit::setValueQuiet);
    tb_sweep->addWidget(new QLabel("Center:"));
    tb_sweep->addWidget(eCenter);

    auto eStop = new SIUnitEdit("Hz", " kMG", 6);
    eStop->setFixedWidth(width);
    eStop->setToolTip("Stop frequency");
    connect(eStop, &SIUnitEdit::valueChanged, this, &SpectrumAnalyzer::SetStopFreq);
    connect(this, &SpectrumAnalyzer::stopFreqChanged, eStop, &SIUnitEdit::setValueQuiet);
    tb_sweep->addWidget(new QLabel("Stop:"));
    tb_sweep->addWidget(eStop);

    auto eSpan = new SIUnitEdit("Hz", " kMG", 6);
    eSpan->setFixedWidth(width);
    eSpan->setToolTip("Span");
    connect(eSpan, &SIUnitEdit::valueChanged, this, &SpectrumAnalyzer::SetSpan);
    connect(this, &SpectrumAnalyzer::spanChanged, eSpan, &SIUnitEdit::setValueQuiet);
    tb_sweep->addWidget(new QLabel("Span:"));
    tb_sweep->addWidget(eSpan);

    auto bFull = new QPushButton(QIcon::fromTheme("zoom-fit-best", QIcon(":/icons/zoom-fit.png")), "");
    bFull->setToolTip("Full span");
    connect(bFull, &QPushButton::clicked, this, &SpectrumAnalyzer::SetFullSpan);
    tb_sweep->addWidget(bFull);

    auto bZoomIn = new QPushButton(QIcon::fromTheme("zoom-in", QIcon(":/icons/zoom-in.png")), "");
    bZoomIn->setToolTip("Zoom in");
    connect(bZoomIn, &QPushButton::clicked, this, &SpectrumAnalyzer::SpanZoomIn);
    tb_sweep->addWidget(bZoomIn);

    auto bZoomOut = new QPushButton(QIcon::fromTheme("zoom-out", QIcon(":/icons/zoom-out.png")), "");
    bZoomOut->setToolTip("Zoom out");
    connect(bZoomOut, &QPushButton::clicked, this, &SpectrumAnalyzer::SpanZoomOut);
    tb_sweep->addWidget(bZoomOut);

    auto bZero = new QPushButton("0");
    bZero->setToolTip("Zero span");
    bZero->setMaximumWidth(28);
    bZero->setMaximumHeight(24);
    connect(bZero, &QPushButton::clicked, this, &SpectrumAnalyzer::SetZeroSpan);
    tb_sweep->addWidget(bZero);

    window->addToolBar(tb_sweep);
    toolbars.insert(tb_sweep);

    // Acquisition toolbar
    auto tb_acq = new QToolBar("Acquisition");

    auto eBandwidth = new SIUnitEdit("Hz", " k", 3);
    eBandwidth->setFixedWidth(70);
    eBandwidth->setToolTip("RBW");
    connect(eBandwidth, &SIUnitEdit::valueChanged, this, &SpectrumAnalyzer::SetRBW);
    connect(this, &SpectrumAnalyzer::RBWChanged, eBandwidth, &SIUnitEdit::setValueQuiet);
    tb_acq->addWidget(new QLabel("RBW:"));
    tb_acq->addWidget(eBandwidth);

    tb_acq->addWidget(new QLabel("Window:"));
    cbWindowType = new QComboBox();
    cbWindowType->addItem("None");
    cbWindowType->addItem("Kaiser");
    cbWindowType->addItem("Hann");
    cbWindowType->addItem("Flat Top");
    cbWindowType->setCurrentIndex(1);
    connect(cbWindowType, qOverload<int>(&QComboBox::currentIndexChanged), [=](int index) {
       SetWindow((Window) index);
    });
    tb_acq->addWidget(cbWindowType);

    tb_acq->addWidget(new QLabel("Detector:"));
    cbDetector = new QComboBox();
    cbDetector->addItem("+Peak");
    cbDetector->addItem("-Peak");
    cbDetector->addItem("Sample");
    cbDetector->addItem("Normal");
    cbDetector->addItem("Average");
    cbDetector->setCurrentIndex(0);
    connect(cbDetector, qOverload<int>(&QComboBox::currentIndexChanged), [=](int index) {
       SetDetector((Detector) index);
    });
    tb_acq->addWidget(cbDetector);

    tb_acq->addWidget(new QLabel("Averaging:"));
    lAverages = new QLabel("0/");
    tb_acq->addWidget(lAverages);
    auto sbAverages = new QSpinBox;
    sbAverages->setRange(1, 100);
    sbAverages->setRange(1, 99);
    sbAverages->setFixedWidth(40);
    connect(sbAverages, qOverload<int>(&QSpinBox::valueChanged), this, &SpectrumAnalyzer::SetAveraging);
    connect(this, &SpectrumAnalyzer::averagingChanged, sbAverages, &QSpinBox::setValue);
    tb_acq->addWidget(sbAverages);

    cbSignalID = new QCheckBox("Signal ID");
    connect(cbSignalID, &QCheckBox::toggled, this, &SpectrumAnalyzer::SetSignalID);
    tb_acq->addWidget(cbSignalID);

    window->addToolBar(tb_acq);
    toolbars.insert(tb_acq);

    // Tracking generator toolbar
    auto tb_trackgen = new QToolBar("Tracking Generator");
    auto cbTrackGenEnable = new QCheckBox("Tracking Generator");
    connect(cbTrackGenEnable, &QCheckBox::toggled, this, &SpectrumAnalyzer::SetTGEnabled);
    connect(this, &SpectrumAnalyzer::TGStateChanged, cbTrackGenEnable, &QCheckBox::setChecked);
    tb_trackgen->addWidget(cbTrackGenEnable);

    auto cbTrackGenPort = new QComboBox();
    cbTrackGenPort->addItem("Port 1");
    cbTrackGenPort->addItem("Port 2");
    cbTrackGenPort->setCurrentIndex(0);
    connect(cbTrackGenPort, qOverload<int>(&QComboBox::currentIndexChanged), this, &SpectrumAnalyzer::SetTGPort);
    connect(this, &SpectrumAnalyzer::TGPortChanged, cbTrackGenPort, qOverload<int>(&QComboBox::setCurrentIndex));
    tb_trackgen->addWidget(cbTrackGenPort);

    auto dbm = new QDoubleSpinBox();
    dbm->setFixedWidth(95);
    dbm->setRange(-100.0, 100.0);
    dbm->setSingleStep(0.25);
    dbm->setSuffix("dbm");
    dbm->setToolTip("Level");
    dbm->setKeyboardTracking(false);
    connect(dbm, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SpectrumAnalyzer::SetTGLevel);
    connect(this, &SpectrumAnalyzer::TGLevelChanged, dbm, &QDoubleSpinBox::setValue);
    tb_trackgen->addWidget(new QLabel("Level:"));
    tb_trackgen->addWidget(dbm);

    auto tgOffset = new SIUnitEdit("Hz", " kMG", 6);
    tgOffset->setFixedWidth(width);
    tgOffset->setToolTip("Tracking generator offset");
    connect(tgOffset, &SIUnitEdit::valueChanged, this, &SpectrumAnalyzer::SetTGOffset);
    connect(this, &SpectrumAnalyzer::TGOffsetChanged, tgOffset, &SIUnitEdit::setValueQuiet);
    tb_trackgen->addWidget(new QLabel("Offset:"));
    tb_trackgen->addWidget(tgOffset);

    normalize.enable = new QCheckBox("Normalize");
    tb_trackgen->addWidget(normalize.enable);
    connect(normalize.enable, &QCheckBox::toggled, this, &SpectrumAnalyzer::EnableNormalization);
    normalize.Level = new SIUnitEdit("dBm", " ", 3);
    normalize.Level->setFixedWidth(width);
    normalize.Level->setValue(0);
    normalize.Level->setToolTip("Level to normalize to");
    tb_trackgen->addWidget(new QLabel("To:"));
    tb_trackgen->addWidget(normalize.Level);
    normalize.measure = new QPushButton("Measure");
    normalize.measure->setToolTip("Perform normalization measurement");
    connect(normalize.measure, &QPushButton::clicked, this, &SpectrumAnalyzer::MeasureNormalization);
    tb_trackgen->addWidget(normalize.measure);

    window->addToolBar(tb_trackgen);
    toolbars.insert(tb_trackgen);

    markerModel = new MarkerModel(traceModel, this);

    auto tracesDock = new QDockWidget("Traces");
    traceWidget = new TraceWidgetSA(traceModel, window);
    tracesDock->setWidget(traceWidget);
    window->addDockWidget(Qt::LeftDockWidgetArea, tracesDock);
    docks.insert(tracesDock);


    auto markerWidget = new MarkerWidget(*markerModel);

    auto markerDock = new QDockWidget("Marker");
    markerDock->setWidget(markerWidget);
    window->addDockWidget(Qt::BottomDockWidgetArea, markerDock);
    docks.insert(markerDock);

    SetupSCPI();

    // Set initial TG settings
    SetTGLevel(-20.0);
    SetTGOffset(0);
    SetTGEnabled(false);

    // Set initial sweep settings
    auto pref = Preferences::getInstance();

    if(pref.Acquisition.useMedianAveraging) {
        average.setMode(Averaging::Mode::Median);
    } else {
        average.setMode(Averaging::Mode::Mean);
    }

    if(pref.Startup.RememberSweepSettings) {
        LoadSweepSettings();
    } else {
        settings.f_start = pref.Startup.SA.start;
        settings.f_stop = pref.Startup.SA.stop;
        ConstrainAndUpdateFrequencies();
        SetRBW(pref.Startup.SA.RBW);
        SetAveraging(pref.Startup.SA.averaging);
        settings.pointNum = 1001;
        SetWindow((Window) pref.Startup.SA.window);
        SetDetector((Detector) pref.Startup.SA.detector);
        SetSignalID(pref.Startup.SA.signalID);
    }

    finalize(central);
}

void SpectrumAnalyzer::deactivate()
{
    StoreSweepSettings();
    Mode::deactivate();
}

void SpectrumAnalyzer::initializeDevice()
{
    connect(window->getDevice(), &Device::SpectrumResultReceived, this, &SpectrumAnalyzer::NewDatapoint, Qt::UniqueConnection);

    // Configure initial state of device
    SettingsChanged();
}

nlohmann::json SpectrumAnalyzer::toJSON()
{
    nlohmann::json j;
    // save current sweep/acquisition settings
    nlohmann::json sweep;
    nlohmann::json freq;
    freq["start"] = settings.f_start;
    freq["stop"] = settings.f_stop;
    sweep["frequency"] = freq;
    sweep["single"] = singleSweep;
    nlohmann::json acq;
    acq["RBW"] = settings.RBW;
    acq["window"] = WindowToString((Window) settings.WindowType).toStdString();
    acq["detector"] = DetectorToString((Detector) settings.Detector).toStdString();
    acq["signal ID"] = settings.SignalID ? true : false;
    sweep["acquisition"] = acq;
    nlohmann::json tracking;
    tracking["enabled"] = settings.trackingGenerator ? true : false;
    tracking["port"] = settings.trackingGeneratorPort ? 2 : 1;
    tracking["offset"] = settings.trackingGeneratorOffset;
    tracking["power"] = (double) settings.trackingPower / 100.0; // convert to dBm
    sweep["trackingGenerator"] = tracking;

    if(normalize.active) {
        nlohmann::json norm;
        norm["start"] = normalize.f_start;
        norm["stop"] = normalize.f_stop;
        norm["points"] = normalize.points;
        norm["level"] = normalize.Level->value();
        norm["port1"] = normalize.port1Correction;
        norm["port2"] = normalize.port2Correction;
        sweep["normalization"] = norm;
    }
    j["sweep"] = sweep;

    j["traces"] = traceModel.toJSON();
    j["tiles"] = central->toJSON();
    j["markers"] = markerModel->toJSON();
    return j;
}

void SpectrumAnalyzer::fromJSON(nlohmann::json j)
{
    if(j.is_null()) {
        return;
    }
    if(j.contains("traces")) {
        traceModel.fromJSON(j["traces"]);
    }
    if(j.contains("tiles")) {
        central->fromJSON(j["tiles"]);
    }
    if(j.contains("markers")) {
        markerModel->fromJSON(j["markers"]);
    }
    if(j.contains("sweep")) {
        // restore sweep settings
        auto sweep = j["sweep"];
        if(sweep.contains("frequency")) {
            auto freq = sweep["frequency"];
            SetStartFreq(freq.value("start", settings.f_start));
            SetStopFreq(freq.value("stop", settings.f_start));
        }
        if(sweep.contains("acquisition")) {
            auto acq = sweep["acquisition"];
            SetRBW(acq.value("RBW", settings.RBW));
            auto w = WindowFromString(QString::fromStdString(acq.value("window", "")));
            if(w == Window::Last) {
                // invalid, keep current value
                w = (Window) settings.WindowType;
            }
            SetWindow(w);
            auto d = DetectorFromString(QString::fromStdString(acq.value("detector", "")));
            if(d == Detector::Last) {
                // invalid, keep current value
                d = (Detector) settings.Detector;
            }
            SetDetector(d);
            SetSignalID(acq.value("signal ID", settings.SignalID ? true : false));
        }
        if(sweep.contains("trackingGenerator")) {
            auto tracking = sweep["trackingGenerator"];
            SetTGEnabled(tracking.value("enabled", settings.trackingGenerator ? true : false));
            int port = tracking.value("port", 1);
            // Function expects 0 for port1, 1 for port2
            SetTGPort(port - 1);
            SetTGLevel(tracking.value("power", settings.trackingPower));
            SetTGOffset(tracking.value("offset", settings.trackingGeneratorOffset));
        }
        if(sweep.contains("normalization")) {
            auto norm = sweep["normalization"];
            // restore normalization data
            normalize.port1Correction.clear();
            for(double p1 : norm["port1"]) {
                normalize.port1Correction.push_back(p1);
            }
            normalize.port2Correction.clear();
            for(double p2 : norm["port2"]) {
                normalize.port2Correction.push_back(p2);
            }
            normalize.f_start = norm.value("start", normalize.f_start);
            normalize.f_stop = norm.value("stop", normalize.f_stop);
            normalize.points = norm.value("points", normalize.points);
            normalize.Level->setValue(norm.value("level", normalize.Level->value()));
            if((normalize.port1Correction.size() == normalize.points) && (normalize.port1Correction.size() == normalize.points)) {
                // got the correct number of points
                EnableNormalization(true);
            } else {
                EnableNormalization(false);
            }
        }
        SetSingleSweep(sweep.value("single", singleSweep));
    }
}

using namespace std;

void SpectrumAnalyzer::NewDatapoint(Protocol::SpectrumAnalyzerResult d)
{
    if(window->getModeHandler()->getActiveMode() != this) {
        return;
    }

    if(changingSettings) {
        // already setting new sweep settings, ignore incoming points from old settings
        return;
    }

    if(singleSweep && average.getLevel() == averages) {
        changingSettings = true;
        // single sweep finished
        window->getDevice()->SetIdle([=](Device::TransmissionResult){
            changingSettings = false;
        });
    }

    if(d.pointNum >= settings.pointNum) {
        qWarning() << "Ignoring point with too large point number (" << d.pointNum << ")";
        return;
    }

    d = average.process(d);

    if(settings.f_start == settings.f_stop) {
        // keep track of first point time
        if(d.pointNum == 0) {
            firstPointTime = d.us;
            d.us = 0;
        } else {
            d.us -= firstPointTime;
        }
    }

    if(normalize.measuring) {
        if(average.currentSweep() == averages) {
            // this is the last averaging sweep, use values for normalization
            if(normalize.port1Correction.size() > 0 || d.pointNum == 0) {
                // add measurement
                normalize.port1Correction.push_back(d.port1);
                normalize.port2Correction.push_back(d.port2);
                if(d.pointNum == settings.pointNum - 1) {
                    // this was the last point
                    normalize.measuring = false;
                    normalize.f_start = settings.f_start;
                    normalize.f_stop = settings.f_stop;
                    normalize.points = settings.pointNum;
                    EnableNormalization(true);
                    qDebug() << "Normalization measurement complete";
                }
            }
        }
        int percentage = (((average.currentSweep() - 1) * 100) + (d.pointNum + 1) * 100 / settings.pointNum) / averages;
        normalize.dialog.setValue(percentage);
    }

    if(normalize.active) {
        d.port1 /= normalize.port1Correction[d.pointNum];
        d.port2 /= normalize.port2Correction[d.pointNum];
        double corr = pow(10.0, normalize.Level->value() / 20.0);
        d.port1 *= corr;
        d.port2 *= corr;
    }

    traceModel.addSAData(d, settings);
    emit dataChanged();
    if(d.pointNum == settings.pointNum - 1) {
        UpdateAverageCount();
        markerModel->updateMarkers();
    }
    static unsigned int lastPoint = 0;
    if(d.pointNum > 0 && d.pointNum != lastPoint + 1) {
        qWarning() << "Got point" << d.pointNum << "but last received point was" << lastPoint << "("<<(d.pointNum-lastPoint-1)<<"missed points)";
    }
    lastPoint = d.pointNum;
}

void SpectrumAnalyzer::SettingsChanged()
{
    changingSettings = true;
    if(settings.f_stop - settings.f_start >= 1000 || settings.f_stop - settings.f_start <= 0) {
        settings.pointNum = 1001;
    } else {
        settings.pointNum = settings.f_stop - settings.f_start + 1;
    }
    settings.applyReceiverCorrection = 1;
    settings.applySourceCorrection = 1;

    auto pref = Preferences::getInstance();
    if(settings.f_stop > settings.f_start) {
        // non-zerospan, check usability of DFT
        if(!settings.trackingGenerator && pref.Acquisition.useDFTinSAmode && settings.RBW <= pref.Acquisition.RBWLimitForDFT) {
            // Enable DFT if below RBW threshold and TG is not enabled
            settings.UseDFT = 1;
        } else {
            settings.UseDFT = 0;
        }
    } else {
        // zerospan, DFT not usable
        settings.UseDFT = 0;
    }

    if(settings.trackingGenerator && settings.f_stop >= 25000000) {
        // Check point spacing.
        // The highband PLL used as the tracking generator is not able to reach every frequency exactly. This
        // could lead to sharp drops in the spectrum at certain frequencies. If the span is wide enough with
        // respect to the point number, it is ensured that every displayed point has at least one sample with
        // a reachable PLL frequency in it. Display a warning message if this is not the case with the current
        // settings.
        auto pointSpacing = (settings.f_stop - settings.f_start) / (settings.pointNum - 1);
        // The frequency resolution of the PLL is frequency dependent (due to PLL divider).
        // This code assumes some knowledge of the actual hardware and probably should be moved
        // onto the device at some point
        double minSpacing = 25000;
        auto stop = settings.f_stop;
        while(stop <= 3000000000) {
            minSpacing /= 2;
            stop *= 2;
        }
        if(pointSpacing < minSpacing) {
            auto requiredMinSpan = minSpacing * (settings.pointNum - 1);
            auto message = QString() + "Due to PLL limitations, the tracking generator can not reach every frequency exactly. "
                            "With your current span, this could result in the signal not being detected at some bands. A minimum"
                            " span of " + Unit::ToString(requiredMinSpan, "Hz", " kMG") + " is recommended at this stop frequency.";
            InformationBox::ShowMessage("Warning", message, "TrackingGeneratorSpanTooSmallWarning");
        }
    }

    if(normalize.active) {
        // check if normalization is still valid
        if(normalize.f_start != settings.f_start || normalize.f_stop != settings.f_stop || normalize.points != settings.pointNum) {
            // normalization was taken at different settings, disable
            EnableNormalization(false);
            InformationBox::ShowMessage("Information", "Normalization was disabled because the span has been changed");
        }
    }

    if(window->getDevice() && window->getModeHandler()->getActiveMode() == this) {
        window->getDevice()->Configure(settings, [=](Device::TransmissionResult res){
            // device received command
            changingSettings = false;
        });
    }
    average.reset(settings.pointNum);
    UpdateAverageCount();
    traceModel.clearLiveData();
    emit traceModel.SpanChanged(settings.f_start, settings.f_stop);
}

void SpectrumAnalyzer::SetStartFreq(double freq)
{
    settings.f_start = freq;
    if(settings.f_stop < freq) {
        settings.f_stop = freq;
    }
    ConstrainAndUpdateFrequencies();
}

void SpectrumAnalyzer::SetStopFreq(double freq)
{
    settings.f_stop = freq;
    if(settings.f_start > freq) {
        settings.f_start = freq;
    }
    ConstrainAndUpdateFrequencies();
}

void SpectrumAnalyzer::SetCenterFreq(double freq)
{
    auto old_span = settings.f_stop - settings.f_start;
    if (freq - old_span / 2 <= Device::Info(window->getDevice()).limits_minFreq) {
        // would shift start frequency below minimum
        settings.f_start = 0;
        settings.f_stop = 2 * freq;
    } else if(freq + old_span / 2 >= Device::Info(window->getDevice()).limits_maxFreq) {
        // would shift stop frequency above maximum
        settings.f_start = 2 * freq - Device::Info(window->getDevice()).limits_maxFreq;
        settings.f_stop = Device::Info(window->getDevice()).limits_maxFreq;
    } else {
        settings.f_start = freq - old_span / 2;
        settings.f_stop = freq + old_span / 2;
    }
    ConstrainAndUpdateFrequencies();
}

void SpectrumAnalyzer::SetSpan(double span)
{
    auto old_center = (settings.f_start + settings.f_stop) / 2;
    if(old_center < Device::Info(window->getDevice()).limits_minFreq + span / 2) {
        // would shift start frequency below minimum
        settings.f_start = Device::Info(window->getDevice()).limits_minFreq;
        settings.f_stop = Device::Info(window->getDevice()).limits_minFreq + span;
    } else if(old_center > Device::Info(window->getDevice()).limits_maxFreq - span / 2) {
        // would shift stop frequency above maximum
        settings.f_start = Device::Info(window->getDevice()).limits_maxFreq - span;
        settings.f_stop = Device::Info(window->getDevice()).limits_maxFreq;
    } else {
        settings.f_start = old_center - span / 2;
         settings.f_stop = settings.f_start + span;
    }
    ConstrainAndUpdateFrequencies();
}

void SpectrumAnalyzer::SetFullSpan()
{
    settings.f_start = Device::Info(window->getDevice()).limits_minFreq;
    settings.f_stop = Device::Info(window->getDevice()).limits_maxFreq;
    ConstrainAndUpdateFrequencies();
}

void SpectrumAnalyzer::SetZeroSpan()
{
    auto center = (settings.f_start + settings.f_stop) / 2;
    SetStartFreq(center);
    SetStopFreq(center);
}

void SpectrumAnalyzer::SpanZoomIn()
{
    auto center = (settings.f_start + settings.f_stop) / 2;
    auto old_span = settings.f_stop - settings.f_start;
    settings.f_start = center - old_span / 4;
    settings.f_stop = center + old_span / 4;
    ConstrainAndUpdateFrequencies();
}

void SpectrumAnalyzer::SpanZoomOut()
{
    auto center = (settings.f_start + settings.f_stop) / 2;
    auto old_span = settings.f_stop - settings.f_start;
    if(center > old_span) {
        settings.f_start = center - old_span;
    } else {
        settings.f_start = 0;
    }
    settings.f_stop = center + old_span;
    ConstrainAndUpdateFrequencies();
}

void SpectrumAnalyzer::SetSingleSweep(bool single)
{
    if(singleSweep != single) {
        singleSweep = single;
        emit singleSweepChanged(single);
    }
    SettingsChanged();
}

void SpectrumAnalyzer::SetRBW(double bandwidth)
{
    if(bandwidth > Device::Info(window->getDevice()).limits_maxRBW) {
        bandwidth = Device::Info(window->getDevice()).limits_maxRBW;
    } else if(bandwidth < Device::Info(window->getDevice()).limits_minRBW) {
        bandwidth = Device::Info(window->getDevice()).limits_minRBW;
    }
    settings.RBW = bandwidth;
    emit RBWChanged(settings.RBW);
    SettingsChanged();
}

void SpectrumAnalyzer::SetWindow(SpectrumAnalyzer::Window w)
{
    settings.WindowType = (int) w;
    cbWindowType->setCurrentIndex((int) w);
    SettingsChanged();
}

void SpectrumAnalyzer::SetDetector(SpectrumAnalyzer::Detector d)
{
    settings.Detector = (int) d;
    cbDetector->setCurrentIndex((int) d);
    SettingsChanged();
}

void SpectrumAnalyzer::SetAveraging(unsigned int averages)
{
    this->averages = averages;
    average.setAverages(averages);
    emit averagingChanged(averages);
    SettingsChanged();
}

void SpectrumAnalyzer::SetSignalID(bool enabled)
{
    settings.SignalID = enabled ? 1 : 0;
    cbSignalID->setChecked(enabled);
    SettingsChanged();
}

void SpectrumAnalyzer::SetTGEnabled(bool enabled)
{
    if(enabled != settings.trackingGenerator) {
        settings.trackingGenerator = enabled;
        emit TGStateChanged(enabled);
        SettingsChanged();
    }
    normalize.Level->setEnabled(enabled);
    normalize.enable->setEnabled(enabled);
    normalize.measure->setEnabled(enabled);
    if(!enabled && normalize.active) {
        // disable normalization when TG is turned off
        EnableNormalization(false);
    }
}

void SpectrumAnalyzer::SetTGPort(int port)
{
    if(port < 0 || port > 1) {
        return;
    }
    if(port != settings.trackingGeneratorPort) {
        settings.trackingGeneratorPort = port;
        emit TGPortChanged(port);
        if(settings.trackingGenerator) {
             SettingsChanged();
        }
    }
}

void SpectrumAnalyzer::SetTGLevel(double level)
{
    if(level > Device::Info(window->getDevice()).limits_cdbm_max / 100.0) {
        level = Device::Info(window->getDevice()).limits_cdbm_max / 100.0;
    } else if(level < Device::Info(window->getDevice()).limits_cdbm_min / 100.0) {
        level = Device::Info(window->getDevice()).limits_cdbm_min / 100.0;
    }
    emit TGLevelChanged(level);
    settings.trackingPower = level * 100;
    if(settings.trackingGenerator) {
        SettingsChanged();
    }
}

void SpectrumAnalyzer::SetTGOffset(double offset)
{
    settings.trackingGeneratorOffset = offset;

    ConstrainAndUpdateFrequencies();
    if(settings.trackingGenerator) {
        SettingsChanged();
    }
}

void SpectrumAnalyzer::MeasureNormalization()
{
    normalize.active = false;
    normalize.port1Correction.clear();
    normalize.port2Correction.clear();
    normalize.measuring = true;
    normalize.dialog.setLabelText("Taking normalization measurement...");
    normalize.dialog.setCancelButtonText("Abort");
    normalize.dialog.setWindowTitle("Normalization");
    normalize.dialog.setValue(0);
    normalize.dialog.setWindowModality(Qt::ApplicationModal);
    // always show the dialog
    normalize.dialog.setMinimumDuration(0);
    connect(&normalize.dialog, &QProgressDialog::canceled, this, &SpectrumAnalyzer::AbortNormalization);
    // trigger beginning of next sweep
    SettingsChanged();
}

void SpectrumAnalyzer::AbortNormalization()
{
    EnableNormalization(false);
    normalize.measuring = false;
    normalize.points = 0;
    normalize.dialog.reset();
}

void SpectrumAnalyzer::EnableNormalization(bool enabled)
{
    if(enabled != normalize.active) {
        if(enabled) {
            // check if measurements already taken
            if(normalize.f_start == settings.f_start && normalize.f_stop == settings.f_stop && normalize.points == settings.pointNum) {
                // same settings as with normalization measurement, can enable
                normalize.active = true;
            } else {
                // needs to take measurement first
                MeasureNormalization();
            }
        } else {
            // disabled
            normalize.active = false;
        }
    }
    normalize.enable->blockSignals(true);
    normalize.enable->setChecked(normalize.active);
    normalize.enable->blockSignals(false);
}

void SpectrumAnalyzer::SetNormalizationLevel(double level)
{
    normalize.Level->setValueQuiet(level);
    emit NormalizationLevelChanged(level);
}

void SpectrumAnalyzer::SetupSCPI()
{
    auto scpi_freq = new SCPINode("FREQuency");
    SCPINode::add(scpi_freq);
    scpi_freq->add(new SCPICommand("SPAN", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetSpan(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.f_stop - settings.f_start, 'f', 0);
    }));
    scpi_freq->add(new SCPICommand("START", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetStartFreq(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.f_start, 'f', 0);
    }));
    scpi_freq->add(new SCPICommand("CENTer", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetCenterFreq(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number((settings.f_start + settings.f_stop)/2, 'f', 0);
    }));
    scpi_freq->add(new SCPICommand("STOP", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetStopFreq(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.f_stop, 'f', 0);
    }));
    scpi_freq->add(new SCPICommand("FULL", [=](QStringList params) -> QString {
        Q_UNUSED(params)
        SetFullSpan();
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    scpi_freq->add(new SCPICommand("ZERO", [=](QStringList params) -> QString {
        Q_UNUSED(params)
        SetZeroSpan();
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    auto scpi_acq = new SCPINode("ACQuisition");
    SCPINode::add(scpi_acq);
    scpi_acq->add(new SCPICommand("RBW", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetRBW(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.RBW);
    }));
    scpi_acq->add(new SCPICommand("WINDow", [=](QStringList params) -> QString {
        if (params.size() != 1) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        if (params[0] == "NONE") {
            SetWindow(Window::None);
        } else if(params[0] == "KAISER") {
            SetWindow(Window::Kaiser);
        } else if(params[0] == "HANN") {
            SetWindow(Window::Hann);
        } else if(params[0] == "FLATTOP") {
            SetWindow(Window::FlatTop);
        } else {
            return "INVALID WINDOW";
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        switch((Window) settings.WindowType) {
        case Window::None: return "NONE";
        case Window::Kaiser: return "KAISER";
        case Window::Hann: return "HANN";
        case Window::FlatTop: return "FLATTOP";
        default: return SCPI::getResultName(SCPI::Result::Error);
        }
    }));
    scpi_acq->add(new SCPICommand("DETector", [=](QStringList params) -> QString {
        if (params.size() != 1) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        if (params[0] == "+PEAK") {
            SetDetector(Detector::PPeak);
        } else if(params[0] == "-PEAK") {
            SetDetector(Detector::NPeak);
        } else if(params[0] == "NORMAL") {
            SetDetector(Detector::Normal);
        } else if(params[0] == "SAMPLE") {
            SetDetector(Detector::Sample);
        } else if(params[0] == "AVERAGE") {
            SetDetector(Detector::Average);
        } else {
            return "INVALID MDOE";
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        switch((Detector) settings.Detector) {
        case Detector::PPeak: return "+PEAK";
        case Detector::NPeak: return "-PEAK";
        case Detector::Normal: return "NORMAL";
        case Detector::Sample: return "SAMPLE";
        case Detector::Average: return "AVERAGE";
        default: return SCPI::getResultName(SCPI::Result::Error);
        }
    }));
    scpi_acq->add(new SCPICommand("AVG", [=](QStringList params) -> QString {
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetAveraging(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(averages);
    }));
    scpi_acq->add(new SCPICommand("AVGLEVel", nullptr, [=](QStringList) -> QString {
        return QString::number(average.getLevel());
    }));
    scpi_acq->add(new SCPICommand("FINished", nullptr, [=](QStringList) -> QString {
        return average.getLevel() == averages ? "TRUE" : "FALSE";
    }));
    scpi_acq->add(new SCPICommand("LIMit", nullptr, [=](QStringList) -> QString {
        return central->allLimitsPassing() ? "PASS" : "FAIL";
    }));
    scpi_acq->add(new SCPICommand("SIGid", [=](QStringList params) -> QString {
        if (params.size() != 1) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        if(params[0] == "1" || params[0] == "TRUE") {
            SetSignalID(true);
        } else if(params[0] == "0" || params[0] == "FALSE") {
            SetSignalID(false);
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        return settings.SignalID ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
    }));
    scpi_acq->add(new SCPICommand("SINGLE", [=](QStringList params) -> QString {
        bool single;
        if(!SCPI::paramToBool(params, 0, single)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetSingleSweep(single);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return singleSweep ? SCPI::getResultName(SCPI::Result::True): SCPI::getResultName(SCPI::Result::False);
    }));
    auto scpi_tg = new SCPINode("TRACKing");
    SCPINode::add(scpi_tg);
    scpi_tg->add(new SCPICommand("ENable", [=](QStringList params) -> QString {
        if (params.size() != 1) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        if(params[0] == "1" || params[0] == "TRUE") {
            SetTGEnabled(true);
        } else if(params[0] == "0" || params[0] == "FALSE") {
            SetTGEnabled(false);
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        return settings.trackingGenerator ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
    }));
    scpi_tg->add(new SCPICommand("Port", [=](QStringList params) -> QString {
        if (params.size() != 1) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        if(params[0] == "1") {
            SetTGPort(0);
        } else if(params[0] == "2") {
            SetTGPort(1);
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        return settings.trackingGeneratorPort ? "2" : "1";
    }));
    scpi_tg->add(new SCPICommand("LVL", [=](QStringList params) -> QString {
        double newval;
        if(!SCPI::paramToDouble(params, 0, newval)) {

            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetTGLevel(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.trackingPower / 100.0);
    }));
    scpi_tg->add(new SCPICommand("OFFset", [=](QStringList params) -> QString {
        long newval;
        if(!SCPI::paramToLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetTGOffset(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(settings.trackingGeneratorOffset);
    }));
    auto scpi_norm = new SCPINode("NORMalize");
    scpi_tg->add(scpi_norm);
    scpi_norm->add(new SCPICommand("ENable", [=](QStringList params) -> QString {
        if (params.size() != 1) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        if(params[0] == "1" || params[0] == "TRUE") {
            EnableNormalization(true);
        } else if(params[0] == "0" || params[0] == "FALSE") {
            EnableNormalization(false);
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        return normalize.active ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
    }));
    scpi_norm->add(new SCPICommand("MEASure", [=](QStringList params) -> QString {
        Q_UNUSED(params)
        MeasureNormalization();
        return "";
    }, nullptr));
    scpi_norm->add(new SCPICommand("LVL", [=](QStringList params) -> QString {
        double newval;
        if(!SCPI::paramToDouble(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            SetNormalizationLevel(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        }
    }, [=](QStringList) -> QString {
        return QString::number(normalize.Level->value());
    }));
    SCPINode::add(traceWidget);
}

void SpectrumAnalyzer::UpdateAverageCount()
{
    lAverages->setText(QString::number(average.getLevel()) + "/");
}

void SpectrumAnalyzer::ConstrainAndUpdateFrequencies()
{
    if(settings.f_stop > Device::Info(window->getDevice()).limits_maxFreq) {
        settings.f_stop = Device::Info(window->getDevice()).limits_maxFreq;
    }
    if(settings.f_start > settings.f_stop) {
        settings.f_start = settings.f_stop;
    }
    if(settings.f_start < Device::Info(window->getDevice()).limits_minFreq) {
        settings.f_start = Device::Info(window->getDevice()).limits_minFreq;
    }

    bool trackingOffset_limited = false;
    if(settings.f_stop + settings.trackingGeneratorOffset > Device::Info(window->getDevice()).limits_maxFreq) {
        trackingOffset_limited = true;
        settings.trackingGeneratorOffset = Device::Info(window->getDevice()).limits_maxFreq - settings.f_stop;
    }
    if(settings.f_start + settings.trackingGeneratorOffset < Device::Info(window->getDevice()).limits_minFreq) {
        trackingOffset_limited = true;
        settings.trackingGeneratorOffset = Device::Info(window->getDevice()).limits_minFreq - settings.f_start;
    }
    if(trackingOffset_limited) {
        InformationBox::ShowMessage("Warning", "The selected tracking generator offset is not reachable for all frequencies with the current span. "
                                    "The tracking generator offset has been constrained according to the selected start and stop frequencies");
    }
    emit startFreqChanged(settings.f_start);
    emit stopFreqChanged(settings.f_stop);
    emit spanChanged(settings.f_stop - settings.f_start);
    emit centerFreqChanged((settings.f_stop + settings.f_start)/2);
    emit TGOffsetChanged(settings.trackingGeneratorOffset);
    SettingsChanged();
}

void SpectrumAnalyzer::LoadSweepSettings()
{
    QSettings s;
    auto pref = Preferences::getInstance();
    settings.f_start = s.value("SAStart", pref.Startup.SA.start).toULongLong();
    settings.f_stop = s.value("SAStop", pref.Startup.SA.stop).toULongLong();
    ConstrainAndUpdateFrequencies();
    SetRBW(s.value("SARBW", pref.Startup.SA.RBW).toUInt());
    settings.pointNum = 1001;
    SetWindow((Window) s.value("SAWindow", pref.Startup.SA.window).toInt());
    SetDetector((Detector) s.value("SADetector", pref.Startup.SA.detector).toInt());
    SetSignalID(s.value("SASignalID", pref.Startup.SA.signalID).toBool());
    SetAveraging(s.value("SAAveraging", pref.Startup.SA.averaging).toInt());
}

void SpectrumAnalyzer::StoreSweepSettings()
{
    QSettings s;
    s.setValue("SAStart", static_cast<unsigned long long>(settings.f_start));
    s.setValue("SAStop", static_cast<unsigned long long>(settings.f_stop));
    s.setValue("SARBW", settings.RBW);
    s.setValue("SAWindow", settings.WindowType);
    s.setValue("SADetector", settings.Detector);
    s.setValue("SAAveraging", averages);
    s.setValue("SASignalID", static_cast<bool>(settings.SignalID));
}

void SpectrumAnalyzer::setAveragingMode(Averaging::Mode mode)
{
    average.setMode(mode);
}

QString SpectrumAnalyzer::WindowToString(SpectrumAnalyzer::Window w)
{
    switch(w) {
    case Window::None: return "None";
    case Window::Kaiser: return "Kaiser";
    case Window::Hann: return "Hann";
    case Window::FlatTop: return "FlatTop";
    default: return "Unknown";
    }
}

SpectrumAnalyzer::Window SpectrumAnalyzer::WindowFromString(QString s)
{
    for(int i=0;i<(int)Window::Last;i++) {
        if(WindowToString((Window) i) == s) {
            return (Window) i;
        }
    }
    // not found
    return Window::Last;
}

QString SpectrumAnalyzer::DetectorToString(SpectrumAnalyzer::Detector d)
{
    switch(d) {
    case Detector::PPeak: return "+Peak";
    case Detector::NPeak: return "-Peak";
    case Detector::Sample: return "Sample";
    case Detector::Normal: return "Normal";
    case Detector::Average: return "Average";
    default: return "Unknown";
    }
}

SpectrumAnalyzer::Detector SpectrumAnalyzer::DetectorFromString(QString s)
{
    for(int i=0;i<(int)Detector::Last;i++) {
        if(DetectorToString((Detector) i) == s) {
            return (Detector) i;
        }
    }
    // not found
    return Detector::Last;
}
