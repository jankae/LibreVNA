#ifndef APPWINDOW_H
#define APPWINDOW_H

#include "Device/virtualdevice.h"
#include "Traces/traceplot.h"
#include "Traces/tracemodel.h"
#include "Traces/Marker/markermodel.h"
#include "averaging.h"
#include "Device/devicelog.h"
#include "preferences.h"
#include "scpi.h"
#include "tcpserver.h"
#include "Device/manualcontroldialog.h"

#include <QWidget>
#include <QMainWindow>
#include <QGridLayout>
#include <QComboBox>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QCheckBox>
#include <QLabel>
#include <QCommandLineParser>
#include <QProgressDialog>

namespace Ui {
class MainWindow;
}

class VNA;
class Generator;
class SpectrumAnalyzer;
class ModeHandler;

class AppWindow : public QMainWindow
{
    Q_OBJECT
public:
    AppWindow(QWidget *parent = nullptr);
    ~AppWindow();

    Ui::MainWindow *getUi() const;
    QStackedWidget *getCentral() const;
    ModeHandler* getModeHandler() const;
    VirtualDevice *getDevice();

    const QString& getAppVersion() const;
    const QString& getAppGitHash() const;

    static bool showGUI();

    SCPI* getSCPI();

public slots:
    void setModeStatus(QString msg);

protected:
    void closeEvent(QCloseEvent *event) override;
private slots:
    bool ConnectToDevice(QString serial = QString());
    void DisconnectDevice();
    int UpdateDeviceList();
    void StartManualControl();
    void UpdateReferenceToolbar();
    void UpdateReference();
    void UpdateAcquisitionFrequencies();
    void StartFirmwareUpdateDialog();
    void DeviceNeedsUpdate(int reported, int expected);
    void DeviceStatusUpdated(VirtualDevice::Status status);
    void DeviceInfoUpdated();
    void SourceCalibrationDialog();
    void ReceiverCalibrationDialog();
    void FrequencyCalibrationDialog();
    nlohmann::json SaveSetup();
    void SaveSetup(QString filename);
    void LoadSetup(QString filename);
    void LoadSetup(nlohmann::json j);
private:

    enum class DeviceStatusBar {
        Connected,
        Disconnected,
    };

    void DeviceConnectionLost();

    void SetupMenu();
    void SetupStatusBar();
    void UpdateStatusBar(DeviceStatusBar status);
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

    ModeHandler *modeHandler;
    VirtualDevice *vdevice;
    DeviceLog deviceLog;
    QString deviceSerial;
    QActionGroup *deviceActionGroup;

    ManualControlDialog *manual;

    // Status bar widgets
    QLabel lConnectionStatus;
    QLabel lDeviceInfo;

    QLabel lModeInfo;
    QLabel lSetupName;
    // Error flag labels
    QLabel lADCOverload;
    QLabel lUnlevel;
    QLabel lUnlock;

    Ui::MainWindow *ui;
    QCommandLineParser parser;

    SCPI scpi;
    TCPServer *server;

    QString appVersion;
    QString appGitHash;
};

#endif // APPWINDOW_H
