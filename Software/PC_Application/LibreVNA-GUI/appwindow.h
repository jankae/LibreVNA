#ifndef APPWINDOW_H
#define APPWINDOW_H

#include "json.hpp"
#include "Traces/traceplot.h"
#include "Traces/tracemodel.h"
#include "Traces/Marker/markermodel.h"
#include "averaging.h"
#include "Device/devicelog.h"
#include "preferences.h"
#include "scpi.h"
#include "tcpserver.h"
#include "Device/devicedriver.h"

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
class Mode;

class AppWindow : public QMainWindow
{
    Q_OBJECT
public:
    AppWindow(QWidget *parent = nullptr);
    ~AppWindow();

    Ui::MainWindow *getUi() const;
    QStackedWidget *getCentral() const;
    ModeHandler* getModeHandler() const;
    DeviceDriver *getDevice();

    const QString& getAppVersion() const;
    const QString& getAppGitHash() const;

    static bool showGUI();

    SCPI* getSCPI();

public slots:
    void setModeStatus(QString msg);

protected:
    void closeEvent(QCloseEvent *event) override;
private slots:
    void SetInitialState();
    bool ConnectToDevice(QString serial = QString(), DeviceDriver *driver = nullptr);
    void DisconnectDevice();
    int UpdateDeviceList();
//    void StartManualControl();
    void UpdateReferenceToolbar();
    void UpdateReference();
    void DeviceStatusUpdated();
    void DeviceFlagsUpdated();
    void DeviceInfoUpdated();
    void SaveSetup(QString filename);
    bool LoadSetup(QString filename);
private:
    nlohmann::json SaveSetup();
    void LoadSetup(nlohmann::json j);

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
    Mode *lastActiveMode;

//    VirtualDevice *vdevice;
    DeviceDriver *device;

    class DeviceEntry {
    public:
        QString toString();
        static DeviceEntry fromString(QString s, std::vector<DeviceDriver*> drivers);
        bool operator==(const DeviceEntry& rhs) {
            return serial == rhs.serial && driver == rhs.driver;
        }

        QString serial;
        DeviceDriver *driver;
    };

    std::vector<DeviceEntry> deviceList;

    DeviceLog deviceLog;
    QString deviceSerial;
    QActionGroup *deviceActionGroup;

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
