#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include "Util/qpointervariant.h"
#include "savable.h"

#include "Device/compounddevice.h"

#include <QDialog>
#include <QVariant>
#include <exception>

enum GraphDomainChangeBehavior {
    RemoveChangedTraces = 0,
    AdjustGraphs = 1,
    AdjustGrahpsIfOnlyTrace = 2,
};

Q_DECLARE_METATYPE(GraphDomainChangeBehavior)

enum GraphLimitIndication {
    DontShowAnything = 0,
    PassFailText = 1,
    Overlay = 2,
};

Q_DECLARE_METATYPE(GraphLimitIndication)

enum MarkerSortOrder {
    PrefMarkerSortXCoord = 0,
    PrefMarkerSortNumber = 1,
    PrefMarkerSortTimestamp = 2,
};

Q_DECLARE_METATYPE(MarkerSortOrder)

enum MarkerSymbolStyle {
    FilledNumberInside = 0,
    FilledNumberAbove = 1,
    EmptyNumberAbove = 2,
};

Q_DECLARE_METATYPE(MarkerSymbolStyle);


class Preferences : public Savable {
public:
    static Preferences& getInstance() {
        return instance;
    }
    Preferences(const Preferences&) = delete;
    ~Preferences();
    void load();
    void store();
    void edit();
    void setDefault();

    void manualTCPport() { TCPoverride = true; }

    struct {
        bool ConnectToFirstDevice;
        bool RememberSweepSettings;
        bool UseSetupFile;
        bool AutosaveSetupFile;
        QString SetupFile;
        struct {
            QString type;
            double f_start;
            double f_stop;
            bool logSweep;
            double f_excitation;

            double dbm_start;
            double dbm_stop;
            double dbm_freq;

            int points;
            double bandwidth;
            int averaging;
        } DefaultSweep;
        struct {
            double frequency;
            double level;
        } Generator;
        struct {
            double start;
            double stop;
            double RBW;
            int window;
            int detector;
            int averaging;
            bool signalID;
        } SA;
    } Startup;
    struct {
        bool alwaysExciteAllPorts;
        bool suppressPeaks;
        bool adjustPowerLevel;
        bool harmonicMixing;
        bool allowSegmentedSweep;
        bool useDFTinSAmode;
        double RBWLimitForDFT;
        bool useMedianAveraging;

        // advanced, hardware specific settings
        double IF1;
        int ADCprescaler;
        int DFTPhaseInc;

        // Full span settings
        bool fullSpanManual;
        double fullSpanStart;
        double fullSpanStop;
        bool fullSpanCalibratedRange;
    } Acquisition;
    struct {
        bool showUnits;
        struct {
            QColor background;
            QColor axis;
            struct {
                QColor divisions;
                struct {
                    bool enabled;
                    QColor background;
                } Background;
            } Ticks;
        } Color;
        GraphDomainChangeBehavior domainChangeBehavior;
        GraphLimitIndication limitIndication;

        double lineWidth;
        int fontSizeAxis;
        int fontSizeMarkerData;
        int fontSizeTraceNames;
        int fontSizeCursorOverlay;

        bool enablePanAndZoom;
        double zoomFactor;

        struct {
            bool triangle;
            int triangleSize;
            bool line;
            bool hide;
            double hidePercent;
        } SweepIndicator;
    } Graphs;
    struct {
        struct {
            bool showDataOnGraphs;
            bool showAllData;
        } defaultBehavior;
        bool interpolatePoints;
        MarkerSortOrder sortOrder;
        MarkerSymbolStyle symbolStyle;
    } Marker;
    struct {
        bool enabled;
        int port;
    } SCPIServer;

    bool TCPoverride; // in case of manual port specification via command line

    QString compoundDeviceJSON;
    std::vector<CompoundDevice*> compoundDevices;

    void fromJSON(nlohmann::json j) override;
    nlohmann::json toJSON() override;

    void nonTrivialParsing();
    void nonTrivialWriting();

private:
    Preferences() :
     TCPoverride(false) {
        qDebug() << "Pref constructor: " << &compoundDeviceJSON;
    }
    static Preferences instance;

    const std::vector<Savable::SettingDescription> descr = {{
        {&Startup.ConnectToFirstDevice, "Startup.ConnectToFirstDevice", true},
        {&Startup.RememberSweepSettings, "Startup.RememberSweepSettings", false},
        {&Startup.UseSetupFile, "Startup.UseSetupFile", false},
        {&Startup.SetupFile, "Startup.SetupFile", ""},
        {&Startup.AutosaveSetupFile, "Startup.AutosaveSetupFile", false},
        {&Startup.DefaultSweep.type, "Startup.DefaultSweep.type", "Frequency"},
        {&Startup.DefaultSweep.f_start, "Startup.DefaultSweep.start", 1000000.0},
        {&Startup.DefaultSweep.f_stop, "Startup.DefaultSweep.stop", 6000000000.0},
        {&Startup.DefaultSweep.logSweep, "Startup.DefaultSweep.logSweep", false},
        {&Startup.DefaultSweep.f_excitation, "Startup.DefaultSweep.excitation", -10.00},
        {&Startup.DefaultSweep.dbm_start, "Startup.DefaultSweep.dbm_start", -30.00},
        {&Startup.DefaultSweep.dbm_stop, "Startup.DefaultSweep.dbm_stop", -10.0},
        {&Startup.DefaultSweep.dbm_freq, "Startup.DefaultSweep.dbm_freq", 1000000000.0},
        {&Startup.DefaultSweep.points, "Startup.DefaultSweep.points", 501},
        {&Startup.DefaultSweep.bandwidth, "Startup.DefaultSweep.bandwidth", 1000.0},
        {&Startup.DefaultSweep.averaging, "Startup.DefaultSweep.averaging", 1},
        {&Startup.Generator.frequency, "Startup.Generator.frequency", 1000000000.0},
        {&Startup.Generator.level, "Startup.Generator.level", -10.00},
        {&Startup.SA.start, "Startup.SA.start", 950000000.0},
        {&Startup.SA.stop, "Startup.SA.stop", 1050000000.0},
        {&Startup.SA.RBW, "Startup.SA.RBW", 10000.0},
        {&Startup.SA.window, "Startup.SA.window", 1},
        {&Startup.SA.detector, "Startup.SA.detector", 0},
        {&Startup.SA.averaging, "Startup.SA.averaging", 1},
        {&Startup.SA.signalID, "Startup.SA.signalID", true},
        {&Acquisition.alwaysExciteAllPorts, "Acquisition.alwaysExciteBothPorts", true},
        {&Acquisition.suppressPeaks, "Acquisition.suppressPeaks", true},
        {&Acquisition.adjustPowerLevel, "Acquisition.adjustPowerLevel", false},
        {&Acquisition.harmonicMixing, "Acquisition.harmonicMixing", false},
        {&Acquisition.allowSegmentedSweep, "Acquisition.allowSegmentedSweep", false},
        {&Acquisition.useDFTinSAmode, "Acquisition.useDFTinSAmode", true},
        {&Acquisition.RBWLimitForDFT, "Acquisition.RBWLimitForDFT", 3000.0},
        {&Acquisition.useMedianAveraging, "Acquisition.useMedianAveraging", false},
        {&Acquisition.IF1, "Acquisition.IF1", 62000000},
        {&Acquisition.ADCprescaler, "Acquisition.ADCprescaler", 128},
        {&Acquisition.DFTPhaseInc, "Acquisition.DFTPhaseInc", 1280},
        {&Acquisition.fullSpanManual, "Acquisition.fullSpanManual", false},
        {&Acquisition.fullSpanStart, "Acquisition.fullSpanStart", 0.0},
        {&Acquisition.fullSpanStop, "Acquisition.fullSpanStop", 6000000000.0},
        {&Acquisition.fullSpanCalibratedRange, "Acquisition.fullSpanCalibratedRange", false},
        {&Graphs.showUnits, "Graphs.showUnits", true},
        {&Graphs.Color.background, "Graphs.Color.background", QColor(Qt::black)},
        {&Graphs.Color.axis, "Graphs.Color.axis", QColor(Qt::white)},
        {&Graphs.Color.Ticks.Background.enabled, "Graphs.Color.Ticks.Background.enabled", true},
        {&Graphs.Color.Ticks.Background.background, "Graphs.Color.Ticks.Background.background", QColor(20, 20, 20)},
        {&Graphs.Color.Ticks.divisions, "Graphs.Color.Ticks.divisions", QColor(Qt::gray)},
        {&Graphs.domainChangeBehavior, "Graphs.domainChangeBehavior", GraphDomainChangeBehavior::AdjustGraphs},
        {&Graphs.limitIndication, "Graphs.limitIndication", GraphLimitIndication::PassFailText},
        {&Graphs.lineWidth, "Graphs.lineWidth", 1.0},
        {&Graphs.fontSizeAxis, "Graphs.fontSizeAxis", 10},
        {&Graphs.fontSizeCursorOverlay, "Graphs.fontSizeCursorOverlay", 12},
        {&Graphs.fontSizeMarkerData, "Graphs.fontSizeMarkerData", 12},
        {&Graphs.fontSizeTraceNames, "Graphs.fontSizeTraceNames", 12},
        {&Graphs.enablePanAndZoom, "Graphs.enablePanAndZoom", true},
        {&Graphs.zoomFactor, "Graphs.zoomFactor", 0.9},
        {&Graphs.SweepIndicator.triangle, "Graphs.SweepIndicator.triangle", true},
        {&Graphs.SweepIndicator.triangleSize, "Graphs.SweepIndicator.triangleSize", 5},
        {&Graphs.SweepIndicator.line, "Graphs.SweepIndicator.line", false},
        {&Graphs.SweepIndicator.hide, "Graphs.SweepIndicator.hide", false},
        {&Graphs.SweepIndicator.hidePercent, "Graphs.SweepIndicator.hidePercent", 3.0},
        {&Marker.defaultBehavior.showDataOnGraphs, "Marker.defaultBehavior.ShowDataOnGraphs", true},
        {&Marker.defaultBehavior.showAllData, "Marker.defaultBehavior.ShowAllData", false},
        {&Marker.interpolatePoints, "Marker.interpolatePoints", false},
        {&Marker.sortOrder, "Marker.sortOrder", MarkerSortOrder::PrefMarkerSortXCoord},
        {&Marker.symbolStyle, "Marker.symbolStyle", MarkerSymbolStyle::EmptyNumberAbove},
        {&SCPIServer.enabled, "SCPIServer.enabled", true},
        {&SCPIServer.port, "SCPIServer.port", 19542},
        {&compoundDeviceJSON, "compoundDeviceJSON", "[]"},
    }};
};

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(Preferences *pref, QWidget *parent = nullptr);
    ~PreferencesDialog();

private:
    void setInitialGUIState();
    void updateFromGUI();
    Ui::PreferencesDialog *ui;
    Preferences *p;
};

#endif // PREFERENCESDIALOG_H
