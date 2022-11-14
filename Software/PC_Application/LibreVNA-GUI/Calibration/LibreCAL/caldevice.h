#ifndef CALDEVICE_H
#define CALDEVICE_H

#include "usbdevice.h"
#include "touchstone.h"

#include <QString>
#include <QObject>

class CalDevice : public QObject
{
    Q_OBJECT
public:
    CalDevice(QString serial);
    ~CalDevice();

    class Standard {
    public:
        enum class Type {
            Open,
            Short,
            Load,
            Through,
            None
        };
        Standard(Type type) : type(type), throughDest(0){}
        Standard(int throughDest) : type(Type::Through), throughDest(throughDest){}
        Type type;
        int throughDest;
    };

    static QString StandardToString(Standard s);
    static Standard StandardFromString(QString s);

    Standard getStandard(int port);
    bool setStandard(int port, Standard s);
    static std::vector<Standard> availableStandards();

    double getTemperature();
    bool stabilized();
    double getHeaterPower();

    QString serial();
    QString getFirmware() const;
    unsigned int getNumPorts() const;

    bool enterBootloader();

    class CoefficientSet {
    public:
        QString name;
        int ports;
        class Coefficient {
        public:
            Coefficient() : t(Touchstone(1)), modified(false) {}
            Touchstone t;
            bool modified;
        };

        std::vector<Coefficient*> opens;
        std::vector<Coefficient*> shorts;
        std::vector<Coefficient*> loads;
        std::vector<Coefficient*> throughs;

        Coefficient *getThrough(int port1, int port2) const;
    };

    // Extracts the coefficients from the device. This is done with a dedicated thread.
    // Do not call any other functions until the update is finished. Process can be
    // monitored through the updateCoefficientsPercent and updateCoefficientsDone signals
    void loadCoefficientSets(QStringList names = QStringList());
    // Writes coefficient sets to the device. This will only write modified files to save
    // time. This is done with a dedicated thread.
    // Do not call any other functions until the update is finished. Process can be
    // monitored through the updateCoefficientsPercent and updateCoefficientsDone signals
    void saveCoefficientSets();
    std::vector<CoefficientSet> getCoefficientSets() const;

    void addCoefficientSet(QString name);

    QStringList getCoefficientSetNames();

    bool hasModifiedCoefficients();

signals:
    void updateCoefficientsPercent(int percent);
    // emitted when all coefficients have been received and it is safe to call all functions again
    void updateCoefficientsDone(bool success);

    void disconnected();

private:
    void loadCoefficientSetsThread(QStringList names = QStringList());
    void saveCoefficientSetsThread();

    USBDevice *usb;
    QString firmware;
    int numPorts;

    std::vector<CoefficientSet> coeffSets;
};

#endif // CALDEVICE_H
