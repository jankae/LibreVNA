#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QWidget>
#include <QMainWindow>
#include <QGridLayout>
#include <QComboBox>
#include <QStackedWidget>
#include "Device/device.h"
#include "Traces/traceplot.h"
#include "Calibration/calibration.h"
#include <QProgressDialog>
#include "Traces/tracemodel.h"
#include "Traces/tracemarkermodel.h"
#include "averaging.h"
#include "Device/devicelog.h"
#include "preferences.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QLabel>
#include <QCommandLineParser>
#include "scpi.h"
#include "tcpserver.h"

namespace Ui {
class MainWindow;
}

class VNA;
class Generator;
class SpectrumAnalyzer;

class AppWindow : public QMainWindow
{
    Q_OBJECT
public:
    AppWindow(QWidget *parent = nullptr);
    ~AppWindow();

    Ui::MainWindow *getUi() const;
    QStackedWidget *getCentral() const;
    Device *getDevice() const;

protected:
    void closeEvent(QCloseEvent *event) override;
private slots:
    bool ConnectToDevice(QString serial = QString());
    void DisconnectDevice();
    int UpdateDeviceList();
    void StartManualControl();
    void UpdateReference();
    void StartFirmwareUpdateDialog();
    void DeviceNeedsUpdate(int reported, int expected);
    void SourceCalibrationDialog();
    void ReceiverCalibrationDialog();
    void FrequencyCalibrationDialog();
    nlohmann::json SaveSetup();
    void LoadSetup(nlohmann::json j);
private:
    void DeviceConnectionLost();
    void CreateToolbars();
    void SetupSCPI();
    void StartTCPServer(int port);
    void StopTCPServer();

    QStackedWidget *central;

    struct {
        struct {
            QComboBox *type;
            QComboBox *outFreq;
        } reference;
    } toolbars;

    Device *device;
    DeviceLog deviceLog;
    QString deviceSerial;
    QActionGroup *deviceActionGroup;

    // Modes
    VNA *vna;
    Generator *generator;
    SpectrumAnalyzer *spectrumAnalyzer;

    // Status bar widgets
    QLabel lConnectionStatus;
    QLabel lDeviceInfo;
    // Error flag labels
    QLabel lADCOverload;
    QLabel lUnlevel;
    QLabel lUnlock;

    Ui::MainWindow *ui;
    QCommandLineParser parser;

    SCPI scpi;
    TCPServer *server;
};

#endif // VNA_H
