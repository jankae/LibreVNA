#ifndef TOUCHSTONEIMPORT_H
#define TOUCHSTONEIMPORT_H

#include "touchstone.h"

#include <QWidget>
#include <QButtonGroup>

namespace Ui {
class TouchstoneImport;
}

class TouchstoneImport : public QWidget
{
    Q_OBJECT

public:
    explicit TouchstoneImport(QWidget *parent = nullptr, int ports = 0);
    ~TouchstoneImport();

    bool getStatus();
    Touchstone getTouchstone();
    void setPorts(int ports);
    QString getFilename();
    void selectPort(int destination, int source);
    std::vector<int> getPorts();

signals:
    void statusChanged(bool status);
    void filenameChanged(QString name);

public slots:
    void setFile(QString filename);

private slots:
    void on_browse_clicked();

private:
    void evaluateFile();
    void preventCollisionWithGroup(QButtonGroup *group, int id);
    Ui::TouchstoneImport *ui;
    int required_ports;
    Touchstone touchstone;
    bool status;
};

#endif // TOUCHSTONEIMPORT_H
