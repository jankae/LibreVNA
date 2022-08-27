#ifndef CALIBRATIONMEASUREMENT_H
#define CALIBRATIONMEASUREMENT_H

#include "calstandard.h"
#include "Device/virtualdevice.h"

#include <QDateTime>

class Calibration2;

namespace CalibrationMeasurement {

class Base : public Savable
{
public:
    Base(Calibration2 *cal);

    enum class Type {
        Open,
        Short,
        Load,
        Through,
        Last,
    };

    bool setFirstSupportedStandard();
    bool setStandard(CalStandard::Virtual *standard);

    QString getStatistics();

    virtual double minFreq() = 0;
    virtual double maxFreq() = 0;
    virtual unsigned int numPoints() = 0;

    static std::vector<Type> availableTypes();
    static QString TypeToString(Type type);
    static Type TypeFromString(QString s);
    virtual std::set<CalStandard::Virtual::Type> supportedStandards() = 0;
    virtual Type getType() = 0;
    virtual void clearPoints() = 0;
    virtual void addPoint(const VirtualDevice::VNAMeasurement &m) = 0;

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    static bool canMeasureSimultaneously(std::vector<Base*> measurements);
protected:
    CalStandard::Virtual *standard;
    QDateTime timestamp;
    Calibration2 *cal;
};

class OnePort : public Base
{
public:
    OnePort(Calibration2 *cal) :
        Base(cal),
        port(0) {}

    virtual double minFreq() override;
    virtual double maxFreq() override;
    virtual unsigned int numPoints() override {return points.size();}

    virtual void clearPoints();
    virtual void addPoint(const VirtualDevice::VNAMeasurement &m);

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    std::complex<double> getMeasured(double frequency);
    std::complex<double> getActual(double frequency);

    int getPort() const;

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
public:
    Open(Calibration2 *cal) :
        OnePort(cal){setFirstSupportedStandard();}

    virtual std::set<CalStandard::Virtual::Type> supportedStandards() override {return {CalStandard::Virtual::Type::Open};}
    virtual Type getType() override {return Type::Open;}
};

class Short : public OnePort
{
public:
    Short(Calibration2 *cal) :
        OnePort(cal){setFirstSupportedStandard();}
    virtual std::set<CalStandard::Virtual::Type> supportedStandards() override {return {CalStandard::Virtual::Type::Short};}
    virtual Type getType() override {return Type::Short;}
};

class Load : public OnePort
{
public:
    Load(Calibration2 *cal) :
        OnePort(cal){setFirstSupportedStandard();}
    virtual std::set<CalStandard::Virtual::Type> supportedStandards() override {return {CalStandard::Virtual::Type::Load};}
    virtual Type getType() override {return Type::Load;}
};

class TwoPort : public Base
{
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

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    Sparam getMeasured(double frequency);
    Sparam getActual(double frequency);

    int getPort1() const;
    int getPort2() const;

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
public:
    Through(Calibration2 *cal) :
        TwoPort(cal){setFirstSupportedStandard();}
    virtual std::set<CalStandard::Virtual::Type> supportedStandards() override {return {CalStandard::Virtual::Type::Through};}
    virtual Type getType() override {return Type::Through;}
};

}
#endif // CALIBRATIONMEASUREMENT_H
