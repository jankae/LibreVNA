#ifndef CALDEVICE_H
#define CALDEVICE_H

#include "usbdevice.h"
#include "touchstone.h"

#include <QString>
#include <QObject>
#include <thread>

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

    QString getDateTimeUTC();

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

        Coefficient *getOpen(int port);
        Coefficient *getShort(int port);
        Coefficient *getLoad(int port);
        Coefficient *getThrough(int port1, int port2);

        std::map<int, Coefficient*> opens;
        std::map<int, Coefficient*> shorts;
        std::map<int, Coefficient*> loads;
        std::map<int, Coefficient*> throughs;

        int portsToThroughIndex(int port1, int port2);
        void portsFromThroughIndex(int &port1, int &port2, int index);

        void createEmptyCoefficients();
        bool isEmpty();
    };

    // Extracts the coefficients from the device. This is done with a dedicated thread.
    // Do not call any other functions until the update is finished. Process can be
    // monitored through the updateCoefficientsPercent and updateCoefficientsDone signals
    void loadCoefficientSets(QStringList names = QStringList(), QList<int> ports = {}, bool fast=true);

    void abortCoefficientLoading();
    // Writes coefficient sets to the device. This will only write modified files to save
    // time. This is done with a dedicated thread.
    // Do not call any other functions until the update is finished. Process can be
    // monitored through the updateCoefficientsPercent and updateCoefficientsDone signals
    void saveCoefficientSets();
    std::vector<CoefficientSet> getCoefficientSets() const;

    void addCoefficientSet(QString name);

    QStringList getCoefficientSetNames();

    bool hasModifiedCoefficients();

    bool coefficientTransferActive() { return transferActive; }

public slots:
    void factoryUpdateDialog();

signals:
    void updateCoefficientsPercent(int percent);
    // emitted when all coefficients have been received and it is safe to call all functions again
    void updateCoefficientsDone(bool success);

    void disconnected();

private:
    void loadCoefficientSetsThreadSlow(QStringList names, QList<int> ports);
    void loadCoefficientSetsThreadFast(QStringList names, QList<int> ports);
    void saveCoefficientSetsThread();

    USBDevice *usb;
    QString firmware;
    int numPorts;
    std::thread *loadThread;
    bool abortLoading;
    bool transferActive;

    float firmware_major_minor;

    std::vector<CoefficientSet> coeffSets;
};

#endif // CALDEVICE_H
