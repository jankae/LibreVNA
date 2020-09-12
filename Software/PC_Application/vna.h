#ifndef VNA_H
#define VNA_H

#include <QWidget>
#include <QMainWindow>
#include <QGridLayout>
#include <QComboBox>
#include <QStackedWidget>
#include "Device/device.h"
#include "Traces/traceplot.h"
#include "Calibration/calibration.h"
#include <QProgressDialog>
#include "Menu/menuaction.h"
#include "Traces/tracemodel.h"
#include "Traces/tracemarkermodel.h"
#include "averaging.h"
#include "Device/devicelog.h"
#include "preferences.h"
#include <QButtonGroup>
#include <QCheckBox>

namespace Ui {
class MainWindow;
}

class VNA : public QMainWindow
{
    Q_OBJECT
public:
    VNA(QWidget *parent = nullptr);
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    static constexpr double minFreq = 0;
    static constexpr double maxFreq = 6000000000;
    static constexpr Protocol::SweepSettings defaultSweep = {
        .f_start = 1000000,
        .f_stop = (uint64_t) maxFreq,
        .points = 501,
        .if_bandwidth = 1000,
        .cdbm_excitation = 0,
    };
private slots:
    void NewDatapoint(Protocol::Datapoint d);
    void ConnectToDevice(QString serial = QString());
    void DisconnectDevice();
    int UpdateDeviceList();
    void StartManualControl();
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
    void SetSourceLevel(double level);
    void SetPoints(unsigned int points);
    void SetIFBandwidth(double bandwidth);
    void SetAveraging(unsigned int averages);
    // Calibration
    void DisableCalibration(bool force = false);
    void ApplyCalibration(Calibration::Type type);
    void StartCalibrationMeasurement(Calibration::Measurement m);
    void UpdateReference();

signals:
    void CalibrationMeasurementComplete(Calibration::Measurement m);

private:
    void UpdateStatusPanel();
    void SettingsChanged();
    void DeviceConnectionLost();
    void CreateToolbars();
    void ConstrainAndUpdateFrequencies();
    void LoadSweepSettings();
    void StoreSweepSettings();

    class GUIMode {
        friend class VNA;
    public:
        GUIMode(VNA *vna, QString name, QWidget *centralWidget);;
        void addHiddenElement(QAction* a) {
            hiddenActions.insert(a);
        }
        void addHiddenElement(QToolBar* a) {
            hiddenToolbars.insert(a);
        }
        void addHiddenElement(QDockWidget* a) {
            hiddenDocks.insert(a);
        }
        void activate();
        void deactivate();
        QString getName() const;
        static GUIMode *getActiveMode();

    private:
        static GUIMode *activeMode;
        static QWidget *cornerWidget;
        static QButtonGroup *modeButtonGroup;
        std::set<QAction*> hiddenActions;
        std::set<QToolBar*> hiddenToolbars;
        std::set<QDockWidget*> hiddenDocks;
        VNA *vna;
        const QString name;
        QWidget *central;
    };
    GUIMode *modeVNA, *modeSGen;

    QStackedWidget *central;

    struct {
        struct {
            QComboBox *type;
            QCheckBox *automatic;
            QCheckBox *outputEnabled;
            QComboBox *outFreq;
        } reference;
    } toolbars;

    Preferences pref;

    Device *device;
    DeviceLog deviceLog;
    QString deviceSerial;
    QActionGroup *deviceActionGroup;
    Protocol::SweepSettings settings;
    unsigned int averages;
    TraceModel traceModel;
    TraceMarkerModel *markerModel;
    Averaging average;

    // Calibration
    Calibration cal;
    bool calValid;
    Calibration::Measurement calMeasurement;
    bool calMeasuring;
    bool calWaitFirst;
    QProgressDialog calDialog;

    // Status Labels
    QLabel lStart, lCenter, lStop, lSpan, lPoints, lBandwidth;
    QLabel lCalibration;
    QLabel lAverages;

    // Status bar widgets
    QLabel lConnectionStatus;
    QLabel lDeviceInfo;

    Ui::MainWindow *ui;
signals:
    void dataChanged();
    void startFreqChanged(double freq);
    void stopFreqChanged(double freq);
    void centerFreqChanged(double freq);
    void spanChanged(double span);

    void sourceLevelChanged(double level);
    void pointsChanged(unsigned int points);
    void IFBandwidthChanged(double bandwidth);
    void averagingChanged(unsigned int averages);

    void CalibrationDisabled();
    void CalibrationApplied(Calibration::Type type);
};

#endif // VNA_H
