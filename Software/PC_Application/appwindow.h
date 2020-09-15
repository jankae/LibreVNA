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

namespace Ui {
class MainWindow;
}

class AppWindow : public QMainWindow
{
    Q_OBJECT
public:
    AppWindow(QWidget *parent = nullptr);


    Ui::MainWindow *getUi() const;
    QStackedWidget *getCentral() const;
    Device *getDevice() const;

    Preferences &getPreferenceRef();

protected:
    void closeEvent(QCloseEvent *event) override;
private slots:
    void ConnectToDevice(QString serial = QString());
    void DisconnectDevice();
    int UpdateDeviceList();
    void StartManualControl();
    void UpdateReference();

private:
    void DeviceConnectionLost();
    void CreateToolbars();

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

    // Status bar widgets
    QLabel lConnectionStatus;
    QLabel lDeviceInfo;

    Ui::MainWindow *ui;
};

#endif // VNA_H
