#ifndef DEVICEUSBLOGVIEW_H
#define DEVICEUSBLOGVIEW_H

#include "devicepacketlog.h"

#include <QDialog>

namespace Ui {
class DevicePacketLogView;
}

class DevicePacketLogView : public QDialog
{
    Q_OBJECT

public:
    explicit DevicePacketLogView(QWidget *parent = nullptr);
    ~DevicePacketLogView();

private slots:
    void updateTree();
    void addEntry(const DevicePacketLog::LogEntry &e);
private:
    Ui::DevicePacketLogView *ui;
};

#endif // DEVICEUSBLOGVIEW_H
