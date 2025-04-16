#include "spectrumanalyzer.h"

#include "unit.h"
#include "CustomWidgets/toggleswitch.h"
#include "Traces/tracemodel.h"
#include "tracewidgetsa.h"
#include "Traces/tracesmithchart.h"
#include "Traces/tracexyplot.h"
#include "Traces/traceimportdialog.h"
#include "CustomWidgets/tilewidget.h"
#include "CustomWidgets/siunitedit.h"
#include "Traces/Marker/markerwidget.h"
#include "Tools/impedancematchdialog.h"
#include "ui_main.h"
#include "preferences.h"
#include "Generator/signalgenwidget.h"

#include <QDockWidget>
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
      firstPointTime(0),
      central(new QScrollArea),
      tiles(new TileWidget(traceModel, window))
{
    central->setWidget(tiles);
    central->setWidgetResizable(true);
    changingSettings = false;
    averages = 1;
    singleSweep = false;
    settings = {};
    normalize.active = false;
    normalize.measuring = false;
    normalize.points = 0;
    normalize.dialog.reset();
    normalize.f_start = 0;
    normalize.f_stop = 0;
    normalize.points = 0;
    normalize.Level = nullptr;
    normalize.measure = nullptr;
    normalize.enable = nullptr;

    configurationTimer.setSingleShot(true);
    connect(&configurationTimer, &QTimer::timeout, this, [=](){
        ConfigureDevice();
    });

    traceModel.setSource(TraceModel::DataSource::SA);

    // Create default traces
    preset();

    // Create menu entries and connections
    // Sweep toolbar
    auto tb_sweep = new QToolBar("Sweep");

    auto bRun = new QPushButton("Run/Stop");
    bRun->setToolTip("Pause/continue sweep");
    bRun->setCheckable(true);
    running = true;
    connect(bRun, &QPushButton::toggled, [=](){
        if(bRun->isChecked()) {
            Run();
        } else {
            Stop();
        }
    });
    connect(this, &SpectrumAnalyzer::sweepStopped, [=](){
        bRun->blockSignals(true);
        bRun->setChecked(false);
        bRun->setIcon(bRun->style()->standardIcon(QStyle::SP_MediaPause));
        bRun->blockSignals(false);
    });
    connect(this, &SpectrumAnalyzer::sweepStarted, [=](){
        bRun->blockSignals(true);
        bRun->setChecked(true);
        bRun->setIcon(bRun->style()->standardIcon(QStyle::SP_MediaPlay));
        bRun->blockSignals(false);
    });
    tb_sweep->addWidget(bRun);

    auto bSingle = new QPushButton("Single");
    bSingle->setToolTip("Single sweep");
    bSingle->setCheckable(true);
    connect(bSingle, &QPushButton::toggled, this, &SpectrumAnalyzer::SetSingleSweep);
    connect(this, &SpectrumAnalyzer::singleSweepChanged, bSingle, &QPushButton::setChecked);
    tb_sweep->addWidget(bSingle);

    auto eStart = new SIUnitEdit("Hz", " kMG", 6);
    // calculate width required with expected string length
    auto width = QFontMetrics(eStart->font()).horizontalAdvance("3.00000GHz") + 15;
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

    auto eBandwidth = new SIUnitEdit("Hz", " kM", 3);
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
       SetWindow((DeviceDriver::SASettings::Window) index);
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
       SetDetector((DeviceDriver::SASettings::Detector) index);
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
    auto bResetAvg = new QPushButton("Reset");
    connect(bResetAvg, &QPushButton::clicked, this, [=](){
        average.reset(DeviceDriver::SApoints());
        UpdateAverageCount();
    });
    tb_acq->addWidget(bResetAvg);

    window->addToolBar(tb_acq);
    toolbars.insert(tb_acq);

    // Tracking generator toolbar
    tb_trackgen = new QToolBar("Tracking Generator");
    auto cbTrackGenEnable = new QCheckBox("Tracking Generator");
    connect(cbTrackGenEnable, &QCheckBox::toggled, this, &SpectrumAnalyzer::SetTGEnabled);
    connect(this, &SpectrumAnalyzer::TGStateChanged, cbTrackGenEnable, &QCheckBox::setChecked);
    tb_trackgen->addWidget(cbTrackGenEnable);

    cbTrackGenPort = new QComboBox();
    cbTrackGenPort->addItem("Port 1");
    cbTrackGenPort->addItem("Port 2");
    cbTrackGenPort->setCurrentIndex(0);
    connect(cbTrackGenPort, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int index) {
        SetTGPort(index + 1);
    });
    connect(this, &SpectrumAnalyzer::TGPortChanged, this, [=](int port) {
        cbTrackGenPort->setCurrentIndex(port - 1);
    });
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

    tgOffset = new SIUnitEdit("Hz", " kMG", 6);
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

    auto importAction = new QAction("CSV");
    connect(importAction, &QAction::triggered, traceWidget, &TraceWidgetSA::importDialog);
    importActions.push_back(importAction);

    auto exportAction = new QAction("CSV");
    connect(exportAction, &QAction::triggered, traceWidget, &TraceWidgetSA::exportDialog);
    exportActions.push_back(exportAction);

    auto markerWidget = new MarkerWidget(*markerModel);

    auto markerDock = new QDockWidget("Marker");
    markerDock->setWidget(markerWidget);
    window->addDockWidget(Qt::BottomDockWidgetArea, markerDock);
    docks.insert(markerDock);

    // Set initial GUI state
    deviceInfoUpdated();

    SetupSCPI();

    // Set initial TG settings
    SetTGLevel(-20.0);
    SetTGOffset(0);
    SetTGEnabled(false);

    // Set initial sweep settings
    auto& pref = Preferences::getInstance();

    if(pref.Acquisition.useMedianAveraging) {
        average.setMode(Averaging::Mode::Median);
    } else {
        average.setMode(Averaging::Mode::Mean);
    }

    if(pref.Startup.RememberSweepSettings) {
        LoadSweepSettings();
    } else {
        settings.freqStart = pref.Startup.SA.start;
        settings.freqStop = pref.Startup.SA.stop;
        ConstrainAndUpdateFrequencies();
        SetRBW(pref.Startup.SA.RBW);
        SetAveraging(pref.Startup.SA.averaging);
        SetWindow((DeviceDriver::SASettings::Window) pref.Startup.SA.window);
        SetDetector((DeviceDriver::SASettings::Detector) pref.Startup.SA.detector);
    }

    finalize(central);
}

void SpectrumAnalyzer::deactivate()
{
    StoreSweepSettings();
    configurationTimer.stop();
    Mode::deactivate();
    setOperationPending(false);
}

void SpectrumAnalyzer::initializeDevice()
{
    if(!window->getDevice()->supports(DeviceDriver::Feature::SA)) {
        InformationBox::ShowError("Unsupported", "The connected device does not support spectrum analyzer mode");
        return;
    }
    bool hasTG = window->getDevice()->supports(DeviceDriver::Feature::SATrackingGenerator);
    bool hasTGoffset = window->getDevice()->supports(DeviceDriver::Feature::SATrackingOffset);
    tb_trackgen->setEnabled(hasTG);
    if(hasTG) {
        SetTGEnabled(settings.trackingGenerator);
        if(!hasTGoffset) {
            tgOffset->setValue(0);
            tgOffset->setEnabled(false);
        }
    }
    connect(window->getDevice(), &DeviceDriver::SAmeasurementReceived, this, &SpectrumAnalyzer::NewDatapoint, Qt::UniqueConnection);

    // Configure initial state of device
    ConstrainAndUpdateFrequencies();
    SettingsChanged();
}

void SpectrumAnalyzer::deviceDisconnected()
{
    emit sweepStopped();
}

void SpectrumAnalyzer::resetSettings()
{
    settings.freqStart = DeviceDriver::getInfo(window->getDevice()).Limits.SA.minFreq;
    settings.freqStop = DeviceDriver::getInfo(window->getDevice()).Limits.SA.maxFreq;
    ConstrainAndUpdateFrequencies();
    SetRBW(1000000);
    SetAveraging(1);
    SetWindow(DeviceDriver::SASettings::Window::FlatTop);
    SetDetector(DeviceDriver::SASettings::Detector::PPeak);
    Stop();
}

nlohmann::json SpectrumAnalyzer::toJSON()
{
    nlohmann::json j;
    // save current sweep/acquisition settings
    nlohmann::json sweep;
    nlohmann::json freq;
    freq["start"] = settings.freqStart;
    freq["stop"] = settings.freqStop;
    sweep["frequency"] = freq;
    sweep["single"] = singleSweep;
    nlohmann::json acq;
    acq["RBW"] = settings.RBW;
    acq["window"] = WindowToString((DeviceDriver::SASettings::Window) settings.window).toStdString();
    acq["detector"] = DetectorToString((DeviceDriver::SASettings::Detector) settings.detector).toStdString();
    sweep["acquisition"] = acq;
    nlohmann::json tracking;
    tracking["enabled"] = settings.trackingGenerator ? true : false;
    tracking["port"] = settings.trackingPort;
    tracking["offset"] = settings.trackingOffset;
    tracking["power"] = settings.trackingPower;
    sweep["trackingGenerator"] = tracking;

    if(normalize.active) {
        nlohmann::json norm;
        norm["start"] = normalize.f_start;
        norm["stop"] = normalize.f_stop;
        norm["points"] = normalize.points;
        norm["level"] = normalize.Level->value();
        nlohmann::json jCorr;
        for(auto m : normalize.portCorrection) {
            jCorr[m.first.toStdString()] = m.second;
        }
        norm["corrections"] = jCorr;
        sweep["normalization"] = norm;
    }
    j["sweep"] = sweep;

    j["traces"] = traceModel.toJSON();
    j["tiles"] = tiles->toJSON();
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
        tiles->fromJSON(j["tiles"]);
    }
    if(j.contains("markers")) {
        markerModel->fromJSON(j["markers"]);
    }
    if(j.contains("sweep")) {
        // restore sweep settings
        auto sweep = j["sweep"];
        if(sweep.contains("frequency")) {
            auto freq = sweep["frequency"];
            SetStartFreq(freq.value("start", settings.freqStart));
            SetStopFreq(freq.value("stop", settings.freqStop));
        }
        if(sweep.contains("acquisition")) {
            auto acq = sweep["acquisition"];
            SetRBW(acq.value("RBW", settings.RBW));
            auto w = WindowFromString(QString::fromStdString(acq.value("window", "")));
            if(w == DeviceDriver::SASettings::Window::Last) {
                // invalid, keep current value
                w = (DeviceDriver::SASettings::Window) settings.window;
            }
            SetWindow(w);
            auto d = DetectorFromString(QString::fromStdString(acq.value("detector", "")));
            if(d == DeviceDriver::SASettings::Detector::Last) {
                // invalid, keep current value
                d = (DeviceDriver::SASettings::Detector) settings.detector;
            }
            SetDetector(d);
        }
        if(sweep.contains("trackingGenerator")) {
            auto tracking = sweep["trackingGenerator"];
            SetTGEnabled(tracking.value("enabled", settings.trackingGenerator ? true : false));
            int port = tracking.value("port", 1);
            SetTGPort(port);
            SetTGLevel(tracking.value("power", settings.trackingPower));
            SetTGOffset(tracking.value("offset", settings.trackingOffset));
        }
        if(sweep.contains("normalization")) {
            auto norm = sweep["normalization"];
            // restore normalization data
            normalize.portCorrection.clear();
            if(norm.contains("corrections")) {
                for(auto& el : norm["corrections"].items()) {
                    normalize.portCorrection[QString::fromStdString(el.key())] = {};
                    for(auto p : el.value()) {
                        normalize.portCorrection[QString::fromStdString(el.key())].push_back(p);
                    }
                }
            }
            normalize.f_start = norm.value("start", normalize.f_start);
            normalize.f_stop = norm.value("stop", normalize.f_stop);
            normalize.points = norm.value("points", normalize.points);
            normalize.Level->setValue(norm.value("level", normalize.Level->value()));
            // check correction vector size
            bool correctSize = true;
            for(auto c : normalize.portCorrection) {
                if(c.second.size() != normalize.points) {
                    correctSize = false;
                    break;
                }
            }
            EnableNormalization(correctSize);
        }
        SetSingleSweep(sweep.value("single", singleSweep));
    }
}

using namespace std;

void SpectrumAnalyzer::NewDatapoint(DeviceDriver::SAMeasurement m)
{   
    if(isActive != true) {
        return;
    }

    if(changingSettings) {
        // already setting new sweep settings, ignore incoming points from old settings
        return;
    }

    if(singleSweep && average.getLevel() == averages) {
        Stop();
        return;
    }

    auto m_avg = average.process(m);
    if(average.settled()) {
        setOperationPending(false);
    }

    if(settings.freqStart == settings.freqStop) {
        // keep track of first point time
        if(m_avg.pointNum == 0) {
            firstPointTime = m_avg.us;
            m_avg.us = 0;
        } else {
            m_avg.us -= firstPointTime;
        }
    }

    window->addStreamingData(m_avg, AppWindow::SADataType::Raw, settings.freqStart == settings.freqStop);

    if(normalize.measuring) {
        if(average.currentSweep() == averages) {
            // this is the last averaging sweep, use values for normalization
            if(normalize.portCorrection.size() > 0 || m_avg.pointNum == 0) {
                // add measurement
                for(auto m : m_avg.measurements) {
                    normalize.portCorrection[m.first].push_back(m.second);
                }
                if(m_avg.pointNum == DeviceDriver::SApoints() - 1) {
                    // this was the last point
                    normalize.measuring = false;
                    normalize.f_start = settings.freqStart;
                    normalize.f_stop = settings.freqStop;
                    normalize.points = DeviceDriver::SApoints();
                    EnableNormalization(true);
                    qDebug() << "Normalization measurement complete";
                }
            }
        }
        int percentage = (((average.currentSweep() - 1) * 100) + (m_avg.pointNum + 1) * 100 / DeviceDriver::SApoints()) / averages;
        normalize.dialog.setValue(percentage);
    }

    if(normalize.active) {
        double corr = pow(10.0, normalize.Level->value() / 20.0);
        for(auto &m : m_avg.measurements) {
            m.second /= normalize.portCorrection[m.first][m_avg.pointNum];
            m.second *= corr;
        }
        window->addStreamingData(m_avg, AppWindow::SADataType::Normalized, settings.freqStart == settings.freqStop);
    }


    traceModel.addSAData(m_avg, settings);
    emit dataChanged();
    if(m_avg.pointNum == DeviceDriver::SApoints() - 1) {
        UpdateAverageCount();
        markerModel->updateMarkers();
    }
    static unsigned int lastPoint = 0;
    if(m_avg.pointNum > 0 && m_avg.pointNum != lastPoint + 1) {
        qWarning() << "Got point" << m_avg.pointNum << "but last received point was" << lastPoint << "("<<(m_avg.pointNum-lastPoint-1)<<"missed points)";
    }
    lastPoint = m_avg.pointNum;
}

void SpectrumAnalyzer::SettingsChanged()
{
    if(!running) {
        // not running, no need for any communication
        return;
    }
    if(isActive && window->getDevice()) {
        setOperationPending(true);
    }
    configurationTimer.start(100);
    ResetLiveTraces();
}

void SpectrumAnalyzer::SetStartFreq(double freq)
{
    settings.freqStart = freq;
    if(settings.freqStop < freq) {
        settings.freqStop = freq;
    }
    ConstrainAndUpdateFrequencies();
}

void SpectrumAnalyzer::SetStopFreq(double freq)
{
    settings.freqStop = freq;
    if(settings.freqStart > freq) {
        settings.freqStart = freq;
    }
    ConstrainAndUpdateFrequencies();
}

void SpectrumAnalyzer::SetCenterFreq(double freq)
{
    auto old_span = settings.freqStop - settings.freqStart;
    if (freq - old_span / 2 <= DeviceDriver::getInfo(window->getDevice()).Limits.SA.minFreq) {
        // would shift start frequency below minimum
        settings.freqStart = 0;
        settings.freqStop = 2 * freq;
    } else if(freq + old_span / 2 >= DeviceDriver::getInfo(window->getDevice()).Limits.SA.maxFreq) {
        // would shift stop frequency above maximum
        settings.freqStart = 2 * freq - DeviceDriver::getInfo(window->getDevice()).Limits.SA.maxFreq;
        settings.freqStop = DeviceDriver::getInfo(window->getDevice()).Limits.SA.maxFreq;
    } else {
        settings.freqStart = freq - old_span / 2;
        settings.freqStop = freq + old_span / 2;
    }
    ConstrainAndUpdateFrequencies();
}

void SpectrumAnalyzer::SetSpan(double span)
{
    auto old_center = (settings.freqStart + settings.freqStop) / 2;
    if(old_center < DeviceDriver::getInfo(window->getDevice()).Limits.SA.minFreq + span / 2) {
        // would shift start frequency below minimum
        settings.freqStart = DeviceDriver::getInfo(window->getDevice()).Limits.SA.minFreq;
        settings.freqStop = DeviceDriver::getInfo(window->getDevice()).Limits.SA.minFreq + span;
    } else if(old_center > DeviceDriver::getInfo(window->getDevice()).Limits.SA.maxFreq - span / 2) {
        // would shift stop frequency above maximum
        settings.freqStart = DeviceDriver::getInfo(window->getDevice()).Limits.SA.maxFreq - span;
        settings.freqStop = DeviceDriver::getInfo(window->getDevice()).Limits.SA.maxFreq;
    } else {
        settings.freqStart = old_center - span / 2;
         settings.freqStop = settings.freqStart + span;
    }
    ConstrainAndUpdateFrequencies();
}

void SpectrumAnalyzer::SetFullSpan()
{
    auto &pref = Preferences::getInstance();
    if(pref.Acquisition.fullSpanManual) {
        settings.freqStart = pref.Acquisition.fullSpanStart;
        settings.freqStop = pref.Acquisition.fullSpanStop;
    } else {
        settings.freqStart = DeviceDriver::getInfo(window->getDevice()).Limits.SA.minFreq;
        settings.freqStop = DeviceDriver::getInfo(window->getDevice()).Limits.SA.maxFreq;
    }
    ConstrainAndUpdateFrequencies();
}

void SpectrumAnalyzer::SetZeroSpan()
{
    auto center = (settings.freqStart + settings.freqStop) / 2;
    SetStartFreq(center);
    SetStopFreq(center);
}

void SpectrumAnalyzer::SpanZoomIn()
{
    auto center = (settings.freqStart + settings.freqStop) / 2;
    auto old_span = settings.freqStop - settings.freqStart;
    settings.freqStart = center - old_span / 4;
    settings.freqStop = center + old_span / 4;
    ConstrainAndUpdateFrequencies();
}

void SpectrumAnalyzer::SpanZoomOut()
{
    auto center = (settings.freqStart + settings.freqStop) / 2;
    auto old_span = settings.freqStop - settings.freqStart;
    if(center > old_span) {
        settings.freqStart = center - old_span;
    } else {
        settings.freqStart = 0;
    }
    settings.freqStop = center + old_span;
    ConstrainAndUpdateFrequencies();
}

void SpectrumAnalyzer::SetSingleSweep(bool single)
{
    if(singleSweep != single) {
        singleSweep = single;
        if(single) {
            Run();
        }
        emit singleSweepChanged(single);
    } else {
        // if already set to single, a second single command triggers a new sweep
        if(single && !running) {
            Run();
        }
    }
}

void SpectrumAnalyzer::SetRBW(double bandwidth)
{
    if(bandwidth > DeviceDriver::getInfo(window->getDevice()).Limits.SA.maxRBW) {
        bandwidth = DeviceDriver::getInfo(window->getDevice()).Limits.SA.maxRBW;
    } else if(bandwidth < DeviceDriver::getInfo(window->getDevice()).Limits.SA.minRBW) {
        bandwidth = DeviceDriver::getInfo(window->getDevice()).Limits.SA.minRBW;
    }
    settings.RBW = bandwidth;
    emit RBWChanged(settings.RBW);
    SettingsChanged();
}

void SpectrumAnalyzer::SetWindow(DeviceDriver::SASettings::Window w)
{
    settings.window = w;
    cbWindowType->setCurrentIndex((int) w);
    SettingsChanged();
}

void SpectrumAnalyzer::SetDetector(DeviceDriver::SASettings::Detector d)
{
    settings.detector = d;
    cbDetector->setCurrentIndex((int) d);
    SettingsChanged();
}

void SpectrumAnalyzer::SetAveraging(unsigned int averages)
{
    this->averages = averages;
    average.setAverages(averages);
    emit averagingChanged(averages);
    UpdateAverageCount();
    if(isActive && window->getDevice()) {
        setOperationPending(!average.settled());
    }
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
    if(port < 1 || port > cbTrackGenPort->count()) {
        return;
    }
    if(port != settings.trackingPort) {
        settings.trackingPort = port;
        emit TGPortChanged(port);
        if(settings.trackingGenerator) {
             SettingsChanged();
        }
    }
}

void SpectrumAnalyzer::SetTGLevel(double level)
{
    if(level > DeviceDriver::getInfo(window->getDevice()).Limits.SA.maxdBm) {
        level = DeviceDriver::getInfo(window->getDevice()).Limits.SA.maxdBm;
    } else if(level < DeviceDriver::getInfo(window->getDevice()).Limits.SA.mindBm) {
        level = DeviceDriver::getInfo(window->getDevice()).Limits.SA.mindBm;
    }
    emit TGLevelChanged(level);
    settings.trackingPower = level;
    if(settings.trackingGenerator) {
        SettingsChanged();
    }
}

void SpectrumAnalyzer::SetTGOffset(double offset)
{
    settings.trackingOffset = offset;

    ConstrainAndUpdateFrequencies();
    if(settings.trackingGenerator) {
        SettingsChanged();
    }
}

void SpectrumAnalyzer::MeasureNormalization()
{
    if(!window->getDevice()) {
        return;
    }
    normalize.active = false;
    normalize.portCorrection.clear();
    for(auto m : window->getDevice()->availableSAMeasurements()) {
        normalize.portCorrection[m] = {};
    }
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
    ClearNormalization();
    normalize.dialog.reset();
}

void SpectrumAnalyzer::EnableNormalization(bool enabled)
{
    if(enabled != normalize.active) {
        if(enabled) {
            // check if measurements already taken
            if(normalize.f_start == settings.freqStart && normalize.f_stop == settings.freqStop
                    && normalize.points == DeviceDriver::SApoints()) {
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

void SpectrumAnalyzer::ClearNormalization()
{
    EnableNormalization(false);
    normalize.active = false;
    normalize.measuring = false;
    normalize.points = 0;
    normalize.portCorrection.clear();
    normalize.f_start = 0;
    normalize.f_stop = 0;
}

void SpectrumAnalyzer::SetNormalizationLevel(double level)
{
    normalize.Level->setValueQuiet(level);
    emit NormalizationLevelChanged(level);
}

void SpectrumAnalyzer::Run()
{
    running = true;
    ConfigureDevice();
}

void SpectrumAnalyzer::Stop()
{
    running = false;
    ConfigureDevice();
    setOperationPending(false);
}

void SpectrumAnalyzer::ConfigureDevice()
{
    configurationTimer.stop();
    if(running) {
        changingSettings = true;

        if(window->getDevice() && isActive) {
            window->getDevice()->setSA(settings, [=](bool){
                // device received command
                changingSettings = false;

                if(normalize.active) {
                    // check if normalization is still valid
                    if(normalize.f_start != settings.freqStart || normalize.f_stop != settings.freqStop || normalize.points != DeviceDriver::SApoints()) {
                        // normalization was taken at different settings, disable
                        EnableNormalization(false);
                        InformationBox::ShowMessage("Information", "Normalization was disabled because the span has been changed");
                    }
                }
            });
            emit sweepStarted();
        } else {
            // no device, unable to start sweep
            emit sweepStopped();
            changingSettings = false;
        }
        average.reset(DeviceDriver::SApoints());
        UpdateAverageCount();
        traceModel.clearLiveData();
        emit traceModel.SpanChanged(settings.freqStart, settings.freqStop);
    } else {
        if(window->getDevice()) {
            changingSettings = true;
            // single sweep finished
            window->getDevice()->setIdle([=](bool){
                emit sweepStopped();
                changingSettings = false;
            });
        } else {
            emit sweepStopped();
            changingSettings = false;
        }
    }
}

void SpectrumAnalyzer::ResetLiveTraces()
{
    average.reset(DeviceDriver::SApoints());
    traceModel.clearLiveData();
    UpdateAverageCount();
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
        return QString::number(settings.freqStop - settings.freqStart, 'f', 0);
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
        return QString::number(settings.freqStart, 'f', 0);
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
        return QString::number((settings.freqStart + settings.freqStop)/2, 'f', 0);
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
        return QString::number(settings.freqStop, 'f', 0);
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
    scpi_acq->add(new SCPICommand("RUN", [=](QStringList) -> QString {
            Run();
            return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        return running ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
    }));
    scpi_acq->add(new SCPICommand("STOP", [=](QStringList) -> QString {
            Stop();
            return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
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
            SetWindow(DeviceDriver::SASettings::Window::None);
        } else if(params[0] == "KAISER") {
            SetWindow(DeviceDriver::SASettings::Window::Kaiser);
        } else if(params[0] == "HANN") {
            SetWindow(DeviceDriver::SASettings::Window::Hann);
        } else if(params[0] == "FLATTOP") {
            SetWindow(DeviceDriver::SASettings::Window::FlatTop);
        } else {
            return "INVALID WINDOW";
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        switch((DeviceDriver::SASettings::Window) settings.window) {
        case DeviceDriver::SASettings::Window::None: return "NONE";
        case DeviceDriver::SASettings::Window::Kaiser: return "KAISER";
        case DeviceDriver::SASettings::Window::Hann: return "HANN";
        case DeviceDriver::SASettings::Window::FlatTop: return "FLATTOP";
        default: return SCPI::getResultName(SCPI::Result::Error);
        }
    }));
    scpi_acq->add(new SCPICommand("DETector", [=](QStringList params) -> QString {
        if (params.size() != 1) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        if (params[0] == "+PEAK") {
            SetDetector(DeviceDriver::SASettings::Detector::PPeak);
        } else if(params[0] == "-PEAK") {
            SetDetector(DeviceDriver::SASettings::Detector::NPeak);
        } else if(params[0] == "NORMAL") {
            SetDetector(DeviceDriver::SASettings::Detector::Normal);
        } else if(params[0] == "SAMPLE") {
            SetDetector(DeviceDriver::SASettings::Detector::Sample);
        } else if(params[0] == "AVERAGE") {
            SetDetector(DeviceDriver::SASettings::Detector::Average);
        } else {
            return "INVALID MDOE";
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        switch((DeviceDriver::SASettings::Detector) settings.detector) {
        case DeviceDriver::SASettings::Detector::PPeak: return "+PEAK";
        case DeviceDriver::SASettings::Detector::NPeak: return "-PEAK";
        case DeviceDriver::SASettings::Detector::Normal: return "NORMAL";
        case DeviceDriver::SASettings::Detector::Sample: return "SAMPLE";
        case DeviceDriver::SASettings::Detector::Average: return "AVERAGE";
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
        return tiles->allLimitsPassing() ? "PASS" : "FAIL";
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
        unsigned long long newval;
        if(!SCPI::paramToULongLong(params, 0, newval)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else if(newval > 0 && newval <= DeviceDriver::getInfo(window->getDevice()).Limits.SA.ports){
            SetTGPort(newval);
            return SCPI::getResultName(SCPI::Result::Empty);
        } else {
            // invalid port number
            return SCPI::getResultName(SCPI::Result::Error);
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        return QString::number(settings.trackingPort);
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
        return QString::number(settings.trackingPower);
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
        return QString::number(settings.trackingOffset);
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
    if(settings.freqStop > DeviceDriver::getInfo(window->getDevice()).Limits.SA.maxFreq) {
        settings.freqStop = DeviceDriver::getInfo(window->getDevice()).Limits.SA.maxFreq;
    }
    if(settings.freqStart > settings.freqStop) {
        settings.freqStart = settings.freqStop;
    }
    if(settings.freqStart < DeviceDriver::getInfo(window->getDevice()).Limits.SA.minFreq) {
        settings.freqStart = DeviceDriver::getInfo(window->getDevice()).Limits.SA.minFreq;
    }

    bool trackingOffset_limited = false;
    if(settings.freqStop + settings.trackingOffset > DeviceDriver::getInfo(window->getDevice()).Limits.SA.maxFreq) {
        trackingOffset_limited = true;
        settings.trackingOffset = DeviceDriver::getInfo(window->getDevice()).Limits.SA.maxFreq - settings.freqStop;
    }
    if(settings.freqStart + settings.trackingOffset < DeviceDriver::getInfo(window->getDevice()).Limits.SA.minFreq) {
        trackingOffset_limited = true;
        settings.trackingOffset = DeviceDriver::getInfo(window->getDevice()).Limits.SA.minFreq - settings.freqStart;
    }
    if(trackingOffset_limited) {
        InformationBox::ShowMessage("Warning", "The selected tracking generator offset is not reachable for all frequencies with the current span. "
                                    "The tracking generator offset has been constrained according to the selected start and stop frequencies");
    }
    emit startFreqChanged(settings.freqStart);
    emit stopFreqChanged(settings.freqStop);
    emit spanChanged(settings.freqStop - settings.freqStart);
    emit centerFreqChanged((settings.freqStop + settings.freqStart)/2);
    emit TGOffsetChanged(settings.trackingOffset);
    SettingsChanged();
}

void SpectrumAnalyzer::LoadSweepSettings()
{
    QSettings s;
    auto& pref = Preferences::getInstance();
    settings.freqStart = s.value("SAStart", pref.Startup.SA.start).toULongLong();
    settings.freqStop = s.value("SAStop", pref.Startup.SA.stop).toULongLong();
    ConstrainAndUpdateFrequencies();
    SetRBW(s.value("SARBW", pref.Startup.SA.RBW).toUInt());
    SetWindow((DeviceDriver::SASettings::Window) s.value("SAWindow", pref.Startup.SA.window).toInt());
    SetDetector((DeviceDriver::SASettings::Detector) s.value("SADetector", pref.Startup.SA.detector).toInt());
    SetAveraging(s.value("SAAveraging", pref.Startup.SA.averaging).toInt());
}

void SpectrumAnalyzer::StoreSweepSettings()
{
    QSettings s;
    s.setValue("SAStart", static_cast<unsigned long long>(settings.freqStart));
    s.setValue("SAStop", static_cast<unsigned long long>(settings.freqStop));
    s.setValue("SARBW", settings.RBW);
    s.setValue("SAWindow", (int) settings.window);
    s.setValue("SADetector", (int) settings.detector);
    s.setValue("SAAveraging", averages);
}

void SpectrumAnalyzer::createDefaultTracesAndGraphs(int ports)
{
    tiles->clear();
    auto traceXY = new TraceXYPlot(traceModel);
    traceXY->setYAxis(0, YAxis::Type::Magnitude, false, false, -120,0,12,false);
    traceXY->setYAxis(1, YAxis::Type::Disabled, false, true, 0, 0, 10, false);
    traceXY->updateSpan(settings.freqStart, settings.freqStop);

    tiles->setPlot(traceXY);

    QColor defaultColors[] = {Qt::yellow, Qt::blue, Qt::red, Qt::green, Qt::gray, Qt::cyan, Qt::magenta, Qt::white};

    for(int i=0;i<ports;i++) {
        QString param = "PORT"+QString::number(i+1);
        auto trace = new Trace(param, defaultColors[i], param);
        traceModel.addTrace(trace);
        traceXY->enableTrace(trace, true);
    }
}

void SpectrumAnalyzer::setAveragingMode(Averaging::Mode mode)
{
    average.setMode(mode);
}

void SpectrumAnalyzer::preset()
{
    for(auto t : traceModel.getTraces()) {
        if(Trace::isSAParameter(t->name())) {
            traceModel.removeTrace(t);
        }
    }
    // Create default traces
    createDefaultTracesAndGraphs(DeviceDriver::getInfo(window->getDevice()).Limits.SA.ports);
}

void SpectrumAnalyzer::deviceInfoUpdated()
{
    // new device connected, throw away normalization
    ClearNormalization();
    auto tgPort = cbTrackGenPort->currentIndex();
    cbTrackGenPort->clear();
    for(unsigned int i=0;i<DeviceDriver::getInfo(window->getDevice()).Limits.SA.ports;i++) {
        cbTrackGenPort->addItem("Port "+QString::number(i+1));
    }
    SetTGPort(tgPort);
}

QString SpectrumAnalyzer::WindowToString(DeviceDriver::SASettings::Window w)
{
    switch(w) {
    case DeviceDriver::SASettings::Window::None: return "None";
    case DeviceDriver::SASettings::Window::Kaiser: return "Kaiser";
    case DeviceDriver::SASettings::Window::Hann: return "Hann";
    case DeviceDriver::SASettings::Window::FlatTop: return "FlatTop";
    default: return "Unknown";
    }
}

DeviceDriver::SASettings::Window SpectrumAnalyzer::WindowFromString(QString s)
{
    for(int i=0;i<(int)DeviceDriver::SASettings::Window::Last;i++) {
        if(WindowToString((DeviceDriver::SASettings::Window) i) == s) {
            return (DeviceDriver::SASettings::Window) i;
        }
    }
    // not found
    return DeviceDriver::SASettings::Window::Last;
}

QString SpectrumAnalyzer::DetectorToString(DeviceDriver::SASettings::Detector d)
{
    switch(d) {
    case DeviceDriver::SASettings::Detector::PPeak: return "+Peak";
    case DeviceDriver::SASettings::Detector::NPeak: return "-Peak";
    case DeviceDriver::SASettings::Detector::Sample: return "Sample";
    case DeviceDriver::SASettings::Detector::Normal: return "Normal";
    case DeviceDriver::SASettings::Detector::Average: return "Average";
    default: return "Unknown";
    }
}

DeviceDriver::SASettings::Detector SpectrumAnalyzer::DetectorFromString(QString s)
{
    for(int i=0;i<(int)DeviceDriver::SASettings::Detector::Last;i++) {
        if(DetectorToString((DeviceDriver::SASettings::Detector) i) == s) {
            return (DeviceDriver::SASettings::Detector) i;
        }
    }
    // not found
    return DeviceDriver::SASettings::Detector::Last;
}
