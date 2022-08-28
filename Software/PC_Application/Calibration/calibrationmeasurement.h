#ifndef CALIBRATIONMEASUREMENT_H
#define CALIBRATIONMEASUREMENT_H

#include "calstandard.h"
#include "Device/virtualdevice.h"

#include <QDateTime>
#include <QObject>

class Calibration2;

namespace CalibrationMeasurement {

class Base : public QObject, public Savable
{
    Q_OBJECT
public:
    Base(Calibration2 *cal);

    enum class Type {
        Open,
        Short,
        Load,
        Through,
        Last,
    };

    std::vector<CalStandard::Virtual*> supportedStandards();
    bool setFirstSupportedStandard();
    bool setStandard(CalStandard::Virtual *standard);

    QString getStatistics();

    virtual double minFreq() = 0;
    virtual double maxFreq() = 0;
    virtual unsigned int numPoints() = 0;

    static std::vector<Type> availableTypes();
    static QString TypeToString(Type type);
    static Type TypeFromString(QString s);
    virtual std::set<CalStandard::Virtual::Type> supportedStandardTypes() = 0;
    virtual Type getType() = 0;

    virtual void clearPoints() = 0;
    virtual void addPoint(const VirtualDevice::VNAMeasurement &m) = 0;

    virtual QWidget* createStandardWidget();
    virtual QWidget* createSettingsWidget() = 0;

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    static bool canMeasureSimultaneously(std::set<Base *> measurements);
    QDateTime getTimestamp() const;

protected:
signals:
    void standardChanged(CalStandard::Virtual* newStandard);
protected:
    CalStandard::Virtual *standard;
    QDateTime timestamp;
    Calibration2 *cal;
};

class OnePort : public Base
{
    Q_OBJECT
public:
    OnePort(Calibration2 *cal) :
        Base(cal),
        port(0) {}

    virtual double minFreq() override;
    virtual double maxFreq() override;
    virtual unsigned int numPoints() override {return points.size();}

    virtual void clearPoints();
    virtual void addPoint(const VirtualDevice::VNAMeasurement &m);

    virtual QWidget* createSettingsWidget() override;

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    std::complex<double> getMeasured(double frequency);
    std::complex<double> getActual(double frequency);

    int getPort() const;

public slots:
    void setPort(int p);
protected:
signals:
    void portChanged(int p);
protected:
    int port;
    class Point {
    public:
        double frequency;
        std::complex<double> S;
    };
    std::vector<Point> points;
};

class Open : public OnePort
{
    Q_OBJECT
public:
    Open(Calibration2 *cal) :
        OnePort(cal){setFirstSupportedStandard();}

    virtual std::set<CalStandard::Virtual::Type> supportedStandardTypes() override {return {CalStandard::Virtual::Type::Open};}
    virtual Type getType() override {return Type::Open;}
};

class Short : public OnePort
{
    Q_OBJECT
public:
    Short(Calibration2 *cal) :
        OnePort(cal){setFirstSupportedStandard();}
    virtual std::set<CalStandard::Virtual::Type> supportedStandardTypes() override {return {CalStandard::Virtual::Type::Short};}
    virtual Type getType() override {return Type::Short;}
};

class Load : public OnePort
{
    Q_OBJECT
public:
    Load(Calibration2 *cal) :
        OnePort(cal){setFirstSupportedStandard();}
    virtual std::set<CalStandard::Virtual::Type> supportedStandardTypes() override {return {CalStandard::Virtual::Type::Load};}
    virtual Type getType() override {return Type::Load;}
};

class TwoPort : public Base
{
    Q_OBJECT
public:
    TwoPort(Calibration2 *cal) :
        Base(cal),
        port1(0),
        port2(0){}

    virtual double minFreq() override;
    virtual double maxFreq() override;
    virtual unsigned int numPoints() override {return points.size();}

    virtual void clearPoints();
    virtual void addPoint(const VirtualDevice::VNAMeasurement &m);

    virtual QWidget* createSettingsWidget() override;

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    Sparam getMeasured(double frequency);
    Sparam getActual(double frequency);

    int getPort1() const;
    int getPort2() const;

public slots:
    void setPort1(int p);
    void setPort2(int p);
protected:
signals:
    void port1Changed(int p);
    void port2Changed(int p);
protected:
    int port1, port2;
    class Point {
    public:
        double frequency;
        Sparam S;
    };
    std::vector<Point> points;
};

class Through : public TwoPort
{
    Q_OBJECT
public:
    Through(Calibration2 *cal) :
        TwoPort(cal){setFirstSupportedStandard();}
    virtual std::set<CalStandard::Virtual::Type> supportedStandardTypes() override {return {CalStandard::Virtual::Type::Through};}
    virtual Type getType() override {return Type::Through;}
};

}
#endif // CALIBRATIONMEASUREMENT_H
