#ifndef PORTEXTENSION_H
#define PORTEXTENSION_H

#include <QObject>
#include "../VNA_embedded/Application/Communication/Protocol.hpp"
#include <QToolBar>
#include "Calibration/calkit.h"
#include <QMessageBox>

class PortExtension : public QObject
{
    Q_OBJECT
public:
    PortExtension();
    void applyToMeasurement(Protocol::Datapoint& d);
    QToolBar *createToolbar();
    void setCalkit(Calkit *kit);
public slots:
    void edit();

private:
    void startMeasurement();
    class Extension {
    public:
        bool enabled;
        double delay;
        double velocityFactor;
        double DCloss;
        double loss;
        double frequency;
    };
    Extension port1, port2;

    // status variables for automatic measurements
    Calkit *kit;
    bool measuring;
    bool isPort1;
    bool isOpen;
    bool isIdeal;
    std::vector<Protocol::Datapoint> measurements;
    QMessageBox *msgBox;
};

#endif // PORTEXTENSION_H
