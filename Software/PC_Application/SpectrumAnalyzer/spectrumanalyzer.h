#ifndef SPECTRUMANALYZER_H
#define SPECTRUMANALYZER_H

#include "appwindow.h"
#include "mode.h"
#include "CustomWidgets/tilewidget.h"
#include "scpi.h"
#include "Traces/tracewidget.h"

#include <QObject>
#include <QWidget>
#include <QComboBox>
#include <QCheckBox>

class SpectrumAnalyzer : public Mode
{
    Q_OBJECT
public:
    SpectrumAnalyzer(AppWindow *window, QString name = "Spectrum Analyzer");

    void deactivate() override;
    void initializeDevice() override;

    virtual Type getType() override { return Type::SA;}

    // Only save/load user changeable stuff, no need to save the widgets/mode name etc.
    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    void updateGraphColors();
    void setAveragingMode(Averaging::Mode mode) override;


private:
    enum class Window {
        None = 0,
        Kaiser = 1,
        Hann = 2,
        FlatTop = 3,
        Last
    };
    enum class Detector {
        PPeak = 0,
        NPeak = 1,
        Sample = 2,
        Normal = 3,
        Average = 4,
        Last
    };

    static QString WindowToString(Window w);
    static Window WindowFromString(QString s);
    static QString DetectorToString(Detector d);
    static Detector DetectorFromString(QString s);

private slots:
    void NewDatapoint(Protocol::SpectrumAnalyzerResult d);
    // Sweep control
    void SetStartFreq(double freq);
    void SetStopFreq(double freq);
    void SetCenterFreq(double freq);
    void SetSpan(double span);
    void SetFullSpan();
    void SetZeroSpan();
    void SpanZoomIn();
    void SpanZoomOut();
    void SetSingleSweep(bool single);
    // Acquisition control
    void SetRBW(double bandwidth);
    void SetWindow(Window w);
    void SetDetector(Detector d);
    void SetAveraging(unsigned int averages);
    void SetSignalID(bool enabled);
    // TG control
    void SetTGEnabled(bool enabled);
    void SetTGPort(int port);
    void SetTGLevel(double level);
    void SetTGOffset(double offset);
    void MeasureNormalization();
    void AbortNormalization();
    void EnableNormalization(bool enabled);
    void SetNormalizationLevel(double level);

private:
    void SetupSCPI();
    void UpdateAverageCount();
    void SettingsChanged();
    void ConstrainAndUpdateFrequencies();
    void LoadSweepSettings();
    void StoreSweepSettings();

    Protocol::SpectrumAnalyzerSettings  settings;
    bool changingSettings;
    unsigned int averages;
    bool singleSweep;
    double firstPointTime; // timestamp of the first point in the sweep, only use when zerospan is used
    TraceModel traceModel;
    TraceWidget *traceWidget;
    MarkerModel *markerModel;
    Averaging average;

    TileWidget *central;
    QCheckBox *cbSignalID;
    QComboBox *cbWindowType, *cbDetector;
    QLabel *lAverages;

    struct {
        bool active;
        bool measuring;
        // settings when normalize was measured
        double f_start, f_stop, points;
        // correction values to get the ports to 0dBm
        std::vector<double> port1Correction;
        std::vector<double> port2Correction;
        // level to normalize to (additional correction factor)
        SIUnitEdit *Level;

        // GUI elements
        QProgressDialog dialog;
        QPushButton *measure;
        QCheckBox *enable;
    } normalize;

signals:
    void dataChanged();
    void startFreqChanged(double freq);
    void stopFreqChanged(double freq);
    void centerFreqChanged(double freq);
    void spanChanged(double span);
    void singleSweepChanged(bool single);
    void RBWChanged(double RBW);
    void TGStateChanged(bool enabled);
    void TGPortChanged(int port);
    void TGOffsetChanged(double offset);
    void TGLevelChanged(double level);
    void NormalizationLevelChanged(double level);

    void averagingChanged(unsigned int averages);
};

#endif // VNA_H
