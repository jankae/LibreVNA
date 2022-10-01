#ifndef DEVICELOG_H
#define DEVICELOG_H

#include <QWidget>

namespace Ui {
class DeviceLog;
}

class DeviceLog : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceLog(QWidget *parent = nullptr);
    ~DeviceLog();

public slots:
    void addLine(QString line);
    void clear();

private slots:
    void on_bToFile_clicked();

private:
    Ui::DeviceLog *ui;
};

#endif // DEVICELOG_H
