#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include "Util/qpointervariant.h"
#include "savable.h"
#include "Traces/traceaxis.h"
#include "CustomWidgets/siunitedit.h"

#include "Device/LibreVNA/Compound/compounddevice.h"

#include <QDialog>
#include <QFileDialog>
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


class Preferences : public QObject, public Savable {
    Q_OBJECT
    friend class PreferencesDialog;
public:
    static Preferences& getInstance() {
        return instance;
    }
    Preferences(const Preferences&) = delete;
    ~Preferences();
    void load();
    void load(std::vector<Savable::SettingDescription> descr);
    void store();
    void store(std::vector<Savable::SettingDescription> descr);
    void edit();
    void setDefault();
    void setDefault(std::vector<Savable::SettingDescription> descr);

    void manualTCPport() { TCPoverride = true; }

    static QFileDialog::Options QFileDialogOptions(QFileDialog::Options option = (QFileDialog::Options) 0x00000000);

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
            double dwellTime;
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
        } SA;
    } Startup;
    struct {
        bool alwaysExciteAllPorts;
        bool allowSegmentedSweep;
        bool useMedianAveraging;

        // Full span settings
        bool fullSpanManual;
        double fullSpanStart;
        double fullSpanStop;
        bool fullSpanCalibratedRange;

        // Math settings
        bool limitDFT;
        double maxDFTrate;
        int groupDelaySamples;
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
        bool limitNaNpasses;

        double lineWidth;
        int fontSizeTitle;
        int fontSizeAxis;
        int fontSizeMarkerData;
        int fontSizeTraceNames;
        int fontSizeCursorOverlay;

        bool enablePanAndZoom;
        double zoomFactor;

        bool enableMasterTicksForYAxis;

        struct {
            bool triangle;
            int triangleSize;
            bool line;
            bool hide;
            double hidePercent;
        } SweepIndicator;

        struct {
            QString transmission;
            QString reflection;
        } defaultGraphs;

        struct {
            double min[(int) YAxis::Type::Last];
            double max[(int) YAxis::Type::Last];
        } defaultAxisLimits;
    } Graphs;
    struct {
        struct {
            bool showDataOnGraphs;
            bool showdB, showdBm, showdBuV, showdBAngle, showRealImag, showImpedance, showVSWR, showResistance, showCapacitance, showInductance, showQualityFactor, showGroupDelay;
            bool showNoise, showPhasenoise, showCenterBandwidth, showCutoff, showInsertionLoss, showTOI, showAvgTone, showAvgModulation, showP1dB, showFlatness, showMaxDeltaNeg, showMaxDeltaPos;
        } defaultBehavior;
        bool interpolatePoints;
        MarkerSortOrder sortOrder;
        MarkerSymbolStyle symbolStyle;
        bool clipToYAxis;
    } Marker;
    struct {
        bool enabled;
        int port;
    } SCPIServer;
    struct {
        struct {
            bool enabled;
            int port;
        } VNARawData;
        struct {
            bool enabled;
            int port;
        } VNACalibratedData;
        struct {
            bool enabled;
            int port;
        } VNADeembeddedData;
        struct {
            bool enabled;
            int port;
        } SARawData;
        struct {
            bool enabled;
            int port;
        } SANormalizedData;
    } StreamingServers;
    struct {
        double USBlogSizeLimit;
        bool saveTraceData;
        bool useNativeDialogs;
    } Debug;

    bool TCPoverride; // in case of manual port specification via command line

    void fromJSON(nlohmann::json j) override;
    nlohmann::json toJSON() override;

    bool set(QString name, QVariant value);
    QVariant get(QString name);

signals:
    void updated();

private:
    Preferences() :
     TCPoverride(false) {}

    void nonTrivialParsing();
    void nonTrivialWriting();

    static Preferences instance;

    // TODO remove settings that have been moved to LibreVNADriver
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
        {&Startup.DefaultSweep.dwellTime, "Startup.DefaultSweep.dwellTime", 0.0},
        {&Startup.Generator.frequency, "Startup.Generator.frequency", 1000000000.0},
        {&Startup.Generator.level, "Startup.Generator.level", -10.00},
        {&Startup.SA.start, "Startup.SA.start", 950000000.0},
        {&Startup.SA.stop, "Startup.SA.stop", 1050000000.0},
        {&Startup.SA.RBW, "Startup.SA.RBW", 10000.0},
        {&Startup.SA.window, "Startup.SA.window", 1},
        {&Startup.SA.detector, "Startup.SA.detector", 0},
        {&Startup.SA.averaging, "Startup.SA.averaging", 1},
        {&Acquisition.alwaysExciteAllPorts, "Acquisition.alwaysExciteBothPorts", true},
        {&Acquisition.allowSegmentedSweep, "Acquisition.allowSegmentedSweep", true},
        {&Acquisition.useMedianAveraging, "Acquisition.useMedianAveraging", false},
        {&Acquisition.fullSpanManual, "Acquisition.fullSpanManual", false},
        {&Acquisition.fullSpanStart, "Acquisition.fullSpanStart", 0.0},
        {&Acquisition.fullSpanStop, "Acquisition.fullSpanStop", 6000000000.0},
        {&Acquisition.fullSpanCalibratedRange, "Acquisition.fullSpanCalibratedRange", false},
        {&Acquisition.limitDFT, "Acquisition.limitDFT", true},
        {&Acquisition.maxDFTrate, "Acquisition.maxDFTrate", 1.0},
        {&Acquisition.groupDelaySamples, "Acquisition.groupDelaySamples", 5},
        {&Graphs.showUnits, "Graphs.showUnits", true},
        {&Graphs.Color.background, "Graphs.Color.background", QColor(Qt::black)},
        {&Graphs.Color.axis, "Graphs.Color.axis", QColor(Qt::white)},
        {&Graphs.Color.Ticks.Background.enabled, "Graphs.Color.Ticks.Background.enabled", true},
        {&Graphs.Color.Ticks.Background.background, "Graphs.Color.Ticks.Background.background", QColor(20, 20, 20)},
        {&Graphs.Color.Ticks.divisions, "Graphs.Color.Ticks.divisions", QColor(Qt::gray)},
        {&Graphs.domainChangeBehavior, "Graphs.domainChangeBehavior", GraphDomainChangeBehavior::AdjustGraphs},
        {&Graphs.limitIndication, "Graphs.limitIndication", GraphLimitIndication::PassFailText},
        {&Graphs.limitNaNpasses, "Graphs.limitNaNpasses", false},
        {&Graphs.lineWidth, "Graphs.lineWidth", 1.0},
        {&Graphs.fontSizeTitle, "Graphs.fontSizeTitle", 18},
        {&Graphs.fontSizeAxis, "Graphs.fontSizeAxis", 10},
        {&Graphs.fontSizeCursorOverlay, "Graphs.fontSizeCursorOverlay", 12},
        {&Graphs.fontSizeMarkerData, "Graphs.fontSizeMarkerData", 12},
        {&Graphs.fontSizeTraceNames, "Graphs.fontSizeTraceNames", 12},
        {&Graphs.enablePanAndZoom, "Graphs.enablePanAndZoom", true},
        {&Graphs.zoomFactor, "Graphs.zoomFactor", 0.9},
        {&Graphs.enableMasterTicksForYAxis, "Graphs.enableMasterTicksForYAxis", false},
        {&Graphs.SweepIndicator.triangle, "Graphs.SweepIndicator.triangle", true},
        {&Graphs.SweepIndicator.triangleSize, "Graphs.SweepIndicator.triangleSize", 5},
        {&Graphs.SweepIndicator.line, "Graphs.SweepIndicator.line", false},
        {&Graphs.SweepIndicator.hide, "Graphs.SweepIndicator.hide", false},
        {&Graphs.SweepIndicator.hidePercent, "Graphs.SweepIndicator.hidePercent", 3.0},
        {&Graphs.defaultGraphs.transmission, "Graphs.defaultGraphs.transmission", "XY Plot"},
        {&Graphs.defaultGraphs.reflection, "Graphs.defaultGraphs.reflection", "Smith Chart"},

        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::Magnitude], "Graphs.defaultAxisLimits.Magnitude.max", 20.0},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::Magnitude], "Graphs.defaultAxisLimits.Magnitude.min", -120.0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::MagnitudedBuV], "Graphs.defaultAxisLimits.MagnitudedBuV.max", 128.0},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::MagnitudedBuV], "Graphs.defaultAxisLimits.MagnitudedBuV.min", -13.0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::MagnitudeLinear], "Graphs.defaultAxisLimits.MagnitudeLinear.max", 1.0},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::MagnitudeLinear], "Graphs.defaultAxisLimits.MagnitudeLinear.min", 0.0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::Phase], "Graphs.defaultAxisLimits.Phase.max",180.0},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::Phase], "Graphs.defaultAxisLimits.Phase.min", -180.0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::UnwrappedPhase], "Graphs.defaultAxisLimits.UnwrappedPhase.max", 0.0},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::UnwrappedPhase], "Graphs.defaultAxisLimits.UnwrappedPhase.min", -360.0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::VSWR], "Graphs.defaultAxisLimits.VSWR.max", 10.0},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::VSWR], "Graphs.defaultAxisLimits.VSWR.min", 1.0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::Real], "Graphs.defaultAxisLimits.Real.max", 1.0},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::Real], "Graphs.defaultAxisLimits.Real.min", -1.0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::Imaginary], "Graphs.defaultAxisLimits.Imaginary.max", 1.0},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::Imaginary], "Graphs.defaultAxisLimits.Imaginary.min", -1.0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::AbsImpedance], "Graphs.defaultAxisLimits.AbsImpedance.max", 100.0},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::AbsImpedance], "Graphs.defaultAxisLimits.AbsImpedance.min", 0.0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::SeriesR], "Graphs.defaultAxisLimits.SeriesR.max", 100.0},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::SeriesR], "Graphs.defaultAxisLimits.SeriesR.min", 0.0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::Reactance], "Graphs.defaultAxisLimits.Reactance.max", 100.0},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::Reactance], "Graphs.defaultAxisLimits.Reactance.min", 0.0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::Capacitance], "Graphs.defaultAxisLimits.Capacitance.max", 10e-6},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::Capacitance], "Graphs.defaultAxisLimits.Capacitance.min", 0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::Inductance], "Graphs.defaultAxisLimits.Inductance.max", 1e-3},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::Inductance], "Graphs.defaultAxisLimits.Inductance.min", 0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::QualityFactor], "Graphs.defaultAxisLimits.QualityFactor.max", 100.0},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::QualityFactor], "Graphs.defaultAxisLimits.QualityFactor.min", 0.0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::GroupDelay], "Graphs.defaultAxisLimits.GroupDelay.max", 1e-6},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::GroupDelay], "Graphs.defaultAxisLimits.GroupDelay.min", 0.0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::ImpulseReal], "Graphs.defaultAxisLimits.ImpulseReal.max", 1.0},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::ImpulseReal], "Graphs.defaultAxisLimits.ImpulseReal.min", -1.0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::ImpulseMag], "Graphs.defaultAxisLimits.ImpulseMag.max", 0.0},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::ImpulseMag], "Graphs.defaultAxisLimits.ImpulseMag.min", -100.0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::Step], "Graphs.defaultAxisLimits.Step.max", 1.0},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::Step], "Graphs.defaultAxisLimits.Step.min", -1.0},
        {&Graphs.defaultAxisLimits.max[(int) YAxis::Type::Impedance], "Graphs.defaultAxisLimits.Impedance.max", 100.0},
        {&Graphs.defaultAxisLimits.min[(int) YAxis::Type::Impedance], "Graphs.defaultAxisLimits.Impedance.min", 0.0},

        {&Marker.defaultBehavior.showDataOnGraphs, "Marker.defaultBehavior.ShowDataOnGraphs", true},
        {&Marker.defaultBehavior.showdB, "Marker.defaultBehavior.showdB", true},
        {&Marker.defaultBehavior.showdBm, "Marker.defaultBehavior.showdBm", true},
        {&Marker.defaultBehavior.showdBuV, "Marker.defaultBehavior.showdBuV", true},
        {&Marker.defaultBehavior.showdBAngle, "Marker.defaultBehavior.showdBAngle", true},
        {&Marker.defaultBehavior.showRealImag, "Marker.defaultBehavior.showRealImag", true},
        {&Marker.defaultBehavior.showVSWR, "Marker.defaultBehavior.showVSWR", true},
        {&Marker.defaultBehavior.showImpedance, "Marker.defaultBehavior.showImpedance", true},
        {&Marker.defaultBehavior.showResistance, "Marker.defaultBehavior.showResistance", true},
        {&Marker.defaultBehavior.showCapacitance, "Marker.defaultBehavior.showCapacitance", true},
        {&Marker.defaultBehavior.showInductance, "Marker.defaultBehavior.showInductance", true},
        {&Marker.defaultBehavior.showQualityFactor, "Marker.defaultBehavior.showQualityFactor", true},
        {&Marker.defaultBehavior.showGroupDelay, "Marker.defaultBehavior.showGroupDelay", true},
        {&Marker.defaultBehavior.showNoise, "Marker.defaultBehavior.showNoise", true},
        {&Marker.defaultBehavior.showPhasenoise, "Marker.defaultBehavior.showPhasenoise", true},
        {&Marker.defaultBehavior.showCenterBandwidth, "Marker.defaultBehavior.showCenterBandwidth", true},
        {&Marker.defaultBehavior.showCutoff, "Marker.defaultBehavior.showCutoff", true},
        {&Marker.defaultBehavior.showInsertionLoss, "Marker.defaultBehavior.showInsertionLoss", true},
        {&Marker.defaultBehavior.showTOI, "Marker.defaultBehavior.showTOI", true},
        {&Marker.defaultBehavior.showAvgTone, "Marker.defaultBehavior.showAvgTone", true},
        {&Marker.defaultBehavior.showAvgModulation, "Marker.defaultBehavior.showAvgModulation", true},
        {&Marker.defaultBehavior.showP1dB, "Marker.defaultBehavior.showP1dB", true},
        {&Marker.defaultBehavior.showFlatness, "Marker.defaultBehavior.showNonUniformity", true},
        {&Marker.defaultBehavior.showMaxDeltaNeg, "Marker.defaultBehavior.showMaxDeltaNeg", true},
        {&Marker.defaultBehavior.showMaxDeltaPos, "Marker.defaultBehavior.showMaxDeltaPos", true},
        {&Marker.interpolatePoints, "Marker.interpolatePoints", false},
        {&Marker.sortOrder, "Marker.sortOrder", MarkerSortOrder::PrefMarkerSortXCoord},
        {&Marker.symbolStyle, "Marker.symbolStyle", MarkerSymbolStyle::FilledNumberAbove},
        {&Marker.clipToYAxis, "Marker.clipToYAxis", true},
        {&SCPIServer.enabled, "SCPIServer.enabled", true},
        {&SCPIServer.port, "SCPIServer.port", 19542},
        {&StreamingServers.VNARawData.enabled, "StreamingServers.VNARawData.enabled", false},
        {&StreamingServers.VNARawData.port, "StreamingServers.VNARawData.port", 19000},
        {&StreamingServers.VNACalibratedData.enabled, "StreamingServers.VNACalibratedData.enabled", false},
        {&StreamingServers.VNACalibratedData.port, "StreamingServers.VNACalibratedData.port", 19001},
        {&StreamingServers.VNADeembeddedData.enabled, "StreamingServers.VNADeembeddedData.enabled", false},
        {&StreamingServers.VNADeembeddedData.port, "StreamingServers.VNADeembeddedData.port", 19002},
        {&StreamingServers.SARawData.enabled, "StreamingServers.sARawData.enabled", false},
        {&StreamingServers.SARawData.port, "StreamingServers.sARawData.port", 19100},
        {&StreamingServers.SANormalizedData.enabled, "StreamingServers.SANormalizedData.enabled", false},
        {&StreamingServers.SANormalizedData.port, "StreamingServers.SANormalizedData.port", 19101},
        {&Debug.USBlogSizeLimit, "Debug.USBlogSizeLimit", 10000000.0},
        {&Debug.saveTraceData, "Debug.saveTraceData", false},
        {&Debug.useNativeDialogs, "Debug.useNativeDialogs", true},
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
    std::map<YAxis::Type, SIUnitEdit*> graphAxisLimitsMaxEntries;
    std::map<YAxis::Type, SIUnitEdit*> graphAxisLimitsMinEntries;
};

#endif // PREFERENCESDIALOG_H
