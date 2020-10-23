#ifndef SPECTRUMANALYZER_H
#define SPECTRUMANALYZER_H

#include <QObject>
#include <QWidget>
#include "appwindow.h"
#include "mode.h"
#include "CustomWidgets/tilewidget.h"
#include <QComboBox>
#include <QCheckBox>

class SpectrumAnalyzer : public Mode
{
    Q_OBJECT
public:
    SpectrumAnalyzer(AppWindow *window);

    void deactivate() override;
    void initializeDevice() override;
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
    void SetAveraging(unsigned int averages);

signals:

private:
    void UpdateAverageCount();
    void SettingsChanged();
    void ConstrainAndUpdateFrequencies();
    void LoadSweepSettings();
    void StoreSweepSettings();

    Protocol::SpectrumAnalyzerSettings  settings;
    unsigned int averages;
    TraceModel traceModel;
    TraceMarkerModel *markerModel;
    Averaging average;

    TileWidget *central;
    QCheckBox *cbSignalID;
    QComboBox *cbWindowType, *cbDetector;
    QLabel *lAverages;

signals:
    void dataChanged();
    void startFreqChanged(double freq);
    void stopFreqChanged(double freq);
    void centerFreqChanged(double freq);
    void spanChanged(double span);
    void RBWChanged(double RBW);

    void averagingChanged(unsigned int averages);
};

#endif // VNA_H
