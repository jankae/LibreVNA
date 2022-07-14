#ifndef VNA_H
#define VNA_H

#include "appwindow.h"
#include "mode.h"
#include "CustomWidgets/tilewidget.h"
#include "Device/device.h"
#include "Deembedding/deembedding.h"
#include "scpi.h"
#include "Traces/tracewidget.h"

#include <QObject>
#include <QWidget>
#include <functional>

class VNA : public Mode
{
    Q_OBJECT
public:   
    VNA(AppWindow *window, QString name = "Vector Network Analyzer");

    void deactivate() override;
    void initializeDevice() override;
    void deviceDisconnected() override;
    void shutdown() override;

    virtual Type getType() override { return Type::VNA;}

    // Only save/load user changeable stuff, no need to save the widgets/mode name etc.
    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    void updateGraphColors();
    void setAveragingMode(Averaging::Mode mode) override;

    enum class SweepType {
        Frequency = 0,
        Power = 1,
        Last,
    };

    static QString SweepTypeToString(SweepType sw);
    static SweepType SweepTypeFromString(QString s);

    class Settings {
    public:
        Settings()
            : sweepType(SweepType::Frequency)
            , Freq({.start=1000000, .stop=6000000000, .excitation_power=-10, .logSweep=false})
            , Power({.start=-40, .stop=-10, .frequency=1000000000})
            , npoints(501), bandwidth(1000), excitingPort1(true), excitingPort2(true)
            , segments(1), activeSegment(0){}
        SweepType sweepType;
        struct {
            double start;
            double stop;
            double excitation_power;
            bool logSweep;
        } Freq;
        struct {
            double start;
            double stop;
            double frequency;
        } Power;
        int npoints;
        double bandwidth;
        bool excitingPort1;
        bool excitingPort2;
        // if the number of points is higher than supported by the hardware, the sweep has to be segmented into multiple parts
        int segments;
        int activeSegment;
        bool zerospan;
        double firstPointTime; // timestamp of the first point in the sweep, only use when zerospan is used
    };

public slots:
    bool LoadCalibration(QString filename);

private slots:
    void NewDatapoint(Protocol::Datapoint d);
    void StartImpedanceMatching();
    // Sweep control
    void SetSweepType(SweepType sw);
    void SetStartFreq(double freq);
    void SetStopFreq(double freq);
    void SetCenterFreq(double freq);
    void SetSpan(double span);
    void SetFullSpan();
    void SetZeroSpan();
    void SpanZoomIn();
    void SpanZoomOut();

    void SetLogSweep(bool log);
    // Acquisition control
    void SetSourceLevel(double level);
    // Power sweep settings
    void SetStartPower(double level);
    void SetStopPower(double level);
    void SetPowerSweepFrequency(double freq);

    void SetPoints(unsigned int points);
    void SetIFBandwidth(double bandwidth);
    void SetAveraging(unsigned int averages);
    void ExcitationRequired(bool port1, bool port2);
    // Calibration
    void DisableCalibration(bool force = false);
    void ApplyCalibration(Calibration::Type type);
    void StartCalibrationMeasurements(std::set<Calibration::Measurement> m);


signals:
    void CalibrationMeasurementsComplete(std::set<Calibration::Measurement> m);

private:
    bool CalibrationMeasurementActive() { return calWaitFirst || calMeasuring; }
    void SetupSCPI();
    void UpdateAverageCount();
    void SettingsChanged(bool resetTraces = true, std::function<void (Device::TransmissionResult)> cb = nullptr);
    void ConstrainAndUpdateFrequencies();
    void LoadSweepSettings();
    void StoreSweepSettings();
    void StopSweep();
    void StartCalibrationDialog(Calibration::Type type = Calibration::Type::None);
    void UpdateCalWidget();
private slots:
    void EnableDeembedding(bool enable);
    void UpdateStatusbar();
    void SetSingleSweep(bool single);
private:
    Settings settings;
    unsigned int averages;
    TraceModel traceModel;
    TraceWidget *traceWidget;
    MarkerModel *markerModel;
    Averaging average;
    bool singleSweep;

    // Calibration
    Calibration cal;
    bool changingSettings;
    bool calValid;
    bool calEdited;
    std::set<Calibration::Measurement> calMeasurements;
    bool calMeasuring;
    bool calWaitFirst;
    QProgressDialog calDialog;
    Calibration::InterpolationType getCalInterpolation();
    QString getCalStyle();
    QString getCalToolTip();


    QMenu *defaultCalMenu;
    QAction *assignDefaultCal, *removeDefaultCal;
    QAction *saveCal;

    Deembedding deembedding;
    QAction *enableDeembeddingAction;
    bool deembedding_active;

    // Status Labels
    QLabel *lAverages;
    QLabel *calLabel;

    TileWidget *central;

signals:
    void dataChanged();
    void sweepTypeChanged(SweepType sw);
    void startFreqChanged(double freq);
    void stopFreqChanged(double freq);
    void centerFreqChanged(double freq);
    void spanChanged(double span);
    void logSweepChanged(bool log);
    void singleSweepChanged(bool single);

    void sourceLevelChanged(double level);
    void pointsChanged(unsigned int points);
    void IFBandwidthChanged(double bandwidth);
    void averagingChanged(unsigned int averages);

    void startPowerChanged(double level);
    void stopPowerChanged(double level);
    void powerSweepFrequencyChanged(double freq);

    void CalibrationDisabled();
    void CalibrationApplied(Calibration::Type type);
};

#endif // VNA_H
