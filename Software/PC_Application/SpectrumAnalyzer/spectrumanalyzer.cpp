#include "spectrumanalyzer.h"
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
#include "unit.h"
#include "CustomWidgets/toggleswitch.h"
#include "Device/manualcontroldialog.h"
#include "Traces/tracemodel.h"
#include "Traces/tracewidget.h"
#include "Traces/tracesmithchart.h"
#include "Traces/tracexyplot.h"
#include "Traces/traceimportdialog.h"
#include "CustomWidgets/tilewidget.h"
#include "CustomWidgets/siunitedit.h"
#include <QDockWidget>
#include "Traces/markerwidget.h"
#include "Tools/impedancematchdialog.h"
#include "Calibration/calibrationtracedialog.h"
#include "ui_main.h"
#include "Device/firmwareupdatedialog.h"
#include "preferences.h"
#include "Generator/signalgenwidget.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QActionGroup>

SpectrumAnalyzer::SpectrumAnalyzer(AppWindow *window)
    : Mode(window, "Spectrum Analyzer"),
      central(new TileWidget(traceModel, window))
{
    averages = 1;
    settings = {};

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
    traceXY->setYAxis(0, TraceXYPlot::YAxisType::Magnitude, false, false, -120,0,10);
    traceXY->setYAxis(1, TraceXYPlot::YAxisType::Disabled, false, true, 0,0,1);

    central->setPlot(traceXY);

    // Create menu entries and connections
    // Sweep toolbar
    auto tb_sweep = new QToolBar("Sweep");
    auto eStart = new SIUnitEdit("Hz", " kMG", 6);
    eStart->setFixedWidth(100);
    eStart->setToolTip("Start frequency");
    connect(eStart, &SIUnitEdit::valueChanged, this, &SpectrumAnalyzer::SetStartFreq);
    connect(this, &SpectrumAnalyzer::startFreqChanged, eStart, &SIUnitEdit::setValueQuiet);
    tb_sweep->addWidget(new QLabel("Start:"));
    tb_sweep->addWidget(eStart);

    auto eCenter = new SIUnitEdit("Hz", " kMG", 6);
    eCenter->setFixedWidth(100);
    eCenter->setToolTip("Center frequency");
    connect(eCenter, &SIUnitEdit::valueChanged, this, &SpectrumAnalyzer::SetCenterFreq);
    connect(this, &SpectrumAnalyzer::centerFreqChanged, eCenter, &SIUnitEdit::setValueQuiet);
    tb_sweep->addWidget(new QLabel("Center:"));
    tb_sweep->addWidget(eCenter);

    auto eStop = new SIUnitEdit("Hz", " kMG", 6);
    eStop->setFixedWidth(100);
    eStop->setToolTip("Stop frequency");
    connect(eStop, &SIUnitEdit::valueChanged, this, &SpectrumAnalyzer::SetStopFreq);
    connect(this, &SpectrumAnalyzer::stopFreqChanged, eStop, &SIUnitEdit::setValueQuiet);
    tb_sweep->addWidget(new QLabel("Stop:"));
    tb_sweep->addWidget(eStop);

    auto eSpan = new SIUnitEdit("Hz", " kMG", 6);
    eSpan->setFixedWidth(100);
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
       settings.WindowType = index;
       SettingsChanged();
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
       settings.Detector = index;
       SettingsChanged();
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
    connect(cbSignalID, &QCheckBox::toggled, [=](bool enabled) {
        settings.SignalID = enabled;
        SettingsChanged();
    });
    tb_acq->addWidget(cbSignalID);

    window->addToolBar(tb_acq);
    toolbars.insert(tb_acq);


    markerModel = new TraceMarkerModel(traceModel);

    auto tracesDock = new QDockWidget("Traces");
    tracesDock->setWidget(new TraceWidget(traceModel, window, true));
    window->addDockWidget(Qt::LeftDockWidgetArea, tracesDock);
    docks.insert(tracesDock);


    auto markerWidget = new MarkerWidget(*markerModel);

    auto markerDock = new QDockWidget("Marker");
    markerDock->setWidget(markerWidget);
    window->addDockWidget(Qt::BottomDockWidgetArea, markerDock);
    docks.insert(markerDock);

    qRegisterMetaType<Protocol::SpectrumAnalyzerResult>("SpectrumResult");

    // Set initial sweep settings
    auto pref = Preferences::getInstance();
    if(pref.Startup.RememberSweepSettings) {
        LoadSweepSettings();
    } else {
        settings.f_start = pref.Startup.SA.start;
        settings.f_stop = pref.Startup.SA.stop;
        ConstrainAndUpdateFrequencies();
        SetRBW(pref.Startup.SA.RBW);
        SetAveraging(pref.Startup.SA.averaging);
        settings.pointNum = 1001;
        cbWindowType->setCurrentIndex(pref.Startup.SA.window);
        cbDetector->setCurrentIndex(pref.Startup.SA.detector);
        cbSignalID->setChecked(pref.Startup.SA.signalID);
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
    window->getDevice()->Configure(settings);
}

using namespace std;

void SpectrumAnalyzer::NewDatapoint(Protocol::SpectrumAnalyzerResult d)
{
    // TODO level adjustment in device
    d.port1 /= pow(10.0, 7.5);
    d.port2 /= pow(10.0, 7.5);
    d = average.process(d);
    traceModel.addSAData(d);
    emit dataChanged();
    if(d.pointNum == settings.pointNum - 1) {
        UpdateAverageCount();
        markerModel->updateMarkers();
    }
}

void SpectrumAnalyzer::SettingsChanged()
{
    if(settings.f_stop - settings.f_start >= 1000) {
        settings.pointNum = 1001;
    } else {
        settings.pointNum = settings.f_stop - settings.f_start + 1;
    }

    if(window->getDevice()) {
        window->getDevice()->Configure(settings);
    }
    average.reset(settings.pointNum);
    UpdateAverageCount();
    traceModel.clearVNAData();
    emit traceModel.SpanChanged(settings.f_start, settings.f_stop);
}

void SpectrumAnalyzer::StartImpedanceMatching()
{
    auto dialog = new ImpedanceMatchDialog(*markerModel);
    dialog->show();
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
    if (freq > old_span / 2) {
        settings.f_start = freq - old_span / 2;
        settings.f_stop = freq + old_span / 2;
    } else {
        settings.f_start = 0;
        settings.f_stop = 2 * freq;
    }
    ConstrainAndUpdateFrequencies();
}

void SpectrumAnalyzer::SetSpan(double span)
{
    auto old_center = (settings.f_start + settings.f_stop) / 2;
    if(old_center > span / 2) {
        settings.f_start = old_center - span / 2;
    } else {
        settings.f_start = 0;
    }
    settings.f_stop = old_center + span / 2;
    ConstrainAndUpdateFrequencies();
}

void SpectrumAnalyzer::SetFullSpan()
{
    settings.f_start = Device::Limits().minFreq;
    settings.f_stop = Device::Limits().maxFreq;
    ConstrainAndUpdateFrequencies();
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

void SpectrumAnalyzer::SetRBW(double bandwidth)
{
    if(bandwidth > Device::Limits().maxRBW) {
        bandwidth = Device::Limits().maxRBW;
    } else if(bandwidth < Device::Limits().minRBW) {
        bandwidth = Device::Limits().minRBW;
    }
    settings.RBW = bandwidth;
    emit RBWChanged(settings.RBW);
    SettingsChanged();
}

void SpectrumAnalyzer::SetAveraging(unsigned int averages)
{
    this->averages = averages;
    average.setAverages(averages);
    emit averagingChanged(averages);
    SettingsChanged();
}

void SpectrumAnalyzer::UpdateAverageCount()
{
    lAverages->setText(QString::number(average.getLevel()) + "/");
}

void SpectrumAnalyzer::ConstrainAndUpdateFrequencies()
{
    if(settings.f_stop > Device::Limits().maxFreq) {
        settings.f_stop = Device::Limits().maxFreq;
    }
    if(settings.f_start > settings.f_stop) {
        settings.f_start = settings.f_stop;
    }
    if(settings.f_start < Device::Limits().minFreq) {
        settings.f_start = Device::Limits().minFreq;
    }
    emit startFreqChanged(settings.f_start);
    emit stopFreqChanged(settings.f_stop);
    emit spanChanged(settings.f_stop - settings.f_start);
    emit centerFreqChanged((settings.f_stop + settings.f_start)/2);
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
    cbWindowType->setCurrentIndex(s.value("SAWindow", pref.Startup.SA.window).toInt());
    cbDetector->setCurrentIndex(s.value("SADetector", pref.Startup.SA.detector).toInt());
    SetAveraging(s.value("SAAveraging", pref.Startup.SA.averaging).toInt());
    cbSignalID->setChecked(s.value("SASignalID", pref.Startup.SA.signalID).toBool());
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
