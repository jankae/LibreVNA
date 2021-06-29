#ifndef SPECTRUMANALYZER_H
#define SPECTRUMANALYZER_H

#include <QObject>
#include <QWidget>
#include "appwindow.h"
#include "mode.h"
#include "CustomWidgets/tilewidget.h"
#include <QComboBox>
#include <QCheckBox>
#include "scpi.h"
#include "Traces/tracewidget.h"

class SpectrumAnalyzer : public Mode, public SCPINode
{
    Q_OBJECT
public:
    SpectrumAnalyzer(AppWindow *window);

    void deactivate() override;
    void initializeDevice() override;

    // Only save/load user changeable stuff, no need to save the widgets/mode name etc.
    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    void updateGraphColors();


private:
    enum class Window {
        None = 0,
        Kaiser = 1,
        Hann = 2,
        FlatTop = 3
    };
    enum class Detector {
        PPeak = 0,
        NPeak = 1,
        Sample = 2,
        Normal = 3,
        Average = 4,
    };

private slots:
    void NewDatapoint(Protocol::SpectrumAnalyzerResult d);
    void StartImpedanceMatching();
    // Sweep control
    void SetStartFreq(double freq);
    void SetStopFreq(double freq);
    void SetCenterFreq(double freq);
    void SetSpan(double span);
    void SetFullSpan();
    void SpanZoomIn();
    void SpanZoomOut();
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
    unsigned int averages;
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
    void RBWChanged(double RBW);
    void TGStateChanged(bool enabled);
    void TGPortChanged(int port);
    void TGOffsetChanged(double offset);
    void TGLevelChanged(double level);
    void NormalizationLevelChanged(double level);

    void averagingChanged(unsigned int averages);
    void graphColorsChanged();
};

#endif // VNA_H
