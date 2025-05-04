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
#include "streamingserver.h"
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

    enum class VNADataType {
        Raw = 0,
        Calibrated = 1,
        Deembedded = 2,
    };

    void addStreamingData(const DeviceDriver::VNAMeasurement &m, VNADataType type, bool is_zerospan);

    enum class SADataType {
        Raw = 0,
        Normalized = 1,
    };

    void addStreamingData(const DeviceDriver::SAMeasurement &m, SADataType type, bool is_zerospan);

public slots:
    void setModeStatus(QString msg);

protected:
    void closeEvent(QCloseEvent *event) override;
private slots:
    void SetInitialState();
    void SetResetState();
    bool ConnectToDevice(QString serial = QString(), DeviceDriver *driver = nullptr);
    void DisconnectDevice();
    int UpdateDeviceList();
//    void StartManualControl();
    void ResetReference();
    void UpdateReferenceToolbar();
    void ReferenceChanged();
    void UpdateReference();
    void DeviceStatusUpdated();
    void DeviceFlagsUpdated();
    void DeviceInfoUpdated();
    void SaveSetup(QString filename);
    bool LoadSetup(QString filename);
    void UpdateImportExportMenus();
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

    // Call whenever the preferences have changed. It stores the updated preferences and applies the changes which do not take effect immediately
    void preferencesChanged();

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

    // Reference change timer
    QTimer referenceTimer;

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
    std::vector<SCPICommand*> temporaryDeviceCommands;
    std::vector<SCPINode*> temporaryDeviceNodes;
    TCPServer *server;
    StreamingServer *streamVNARawData;
    StreamingServer *streamVNACalibratedData;
    StreamingServer *streamVNADeembeddedData;
    StreamingServer *streamSARawData;
    StreamingServer *streamSANormalizedData;

    QString appVersion;
    QString appGitHash;
};

#endif // APPWINDOW_H
