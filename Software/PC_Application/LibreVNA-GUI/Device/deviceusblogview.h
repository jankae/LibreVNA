#ifndef DEVICEUSBLOGVIEW_H
#define DEVICEUSBLOGVIEW_H

#include "deviceusblog.h"

#include <QDialog>

namespace Ui {
class DeviceUSBLogView;
}

class DeviceUSBLogView : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceUSBLogView(QWidget *parent = nullptr);
    ~DeviceUSBLogView();

private slots:
    void updateTree();
    void addEntry(const DeviceUSBLog::LogEntry &e);
private:
    Ui::DeviceUSBLogView *ui;
};

#endif // DEVICEUSBLOGVIEW_H
