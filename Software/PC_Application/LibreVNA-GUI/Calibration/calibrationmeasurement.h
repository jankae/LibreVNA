#ifndef CALIBRATIONMEASUREMENT_H
#define CALIBRATIONMEASUREMENT_H

#include "calstandard.h"
#include "Device/virtualdevice.h"

#include <QDateTime>
#include <QObject>

class Calibration;

namespace CalibrationMeasurement {

class Base : public QObject, public Savable
{
    Q_OBJECT
public:
    Base(Calibration *cal);

    enum class Type {
        Open,
        Short,
        Load,
        SlidingLoad,
        Reflect,
        Through,
        Isolation,
        Line,
        Last,
    };

    std::vector<CalStandard::Virtual*> supportedStandards();
    virtual bool setFirstSupportedStandard();
    virtual bool setStandard(CalStandard::Virtual *standard);

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

    CalStandard::Virtual *getStandard() const;

protected:
signals:
    void standardChanged(CalStandard::Virtual* newStandard);
protected:
    CalStandard::Virtual *standard;
    QDateTime timestamp;
    Calibration *cal;
};

class OnePort : public Base
{
    Q_OBJECT
public:
    OnePort(Calibration *cal) :
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

    class Point {
    public:
        double frequency;
        std::complex<double> S;
    };
    std::vector<Point> getPoints() const;

    std::complex<double> getMeasured(double frequency);
    std::complex<double> getActual(double frequency);

    int getPort() const;

public slots:
    void setPort(int p);
protected:
signals:
    void portChanged(int p);
protected:
    int port; // starts at 1
    std::vector<Point> points;
};

class Open : public OnePort
{
    Q_OBJECT
public:
    Open(Calibration *cal) :
        OnePort(cal){setFirstSupportedStandard();}

    virtual std::set<CalStandard::Virtual::Type> supportedStandardTypes() override {return {CalStandard::Virtual::Type::Open};}
    virtual Type getType() override {return Type::Open;}
};

class Short : public OnePort
{
    Q_OBJECT
public:
    Short(Calibration *cal) :
        OnePort(cal){setFirstSupportedStandard();}
    virtual std::set<CalStandard::Virtual::Type> supportedStandardTypes() override {return {CalStandard::Virtual::Type::Short};}
    virtual Type getType() override {return Type::Short;}
};

class Load : public OnePort
{
    Q_OBJECT
public:
    Load(Calibration *cal) :
        OnePort(cal){setFirstSupportedStandard();}
    virtual std::set<CalStandard::Virtual::Type> supportedStandardTypes() override {return {CalStandard::Virtual::Type::Load};}
    virtual Type getType() override {return Type::Load;}
};

class SlidingLoad : public OnePort
{
    Q_OBJECT
public:
    SlidingLoad(Calibration *cal) :
        OnePort(cal){setFirstSupportedStandard();}

    virtual QWidget* createStandardWidget() override;

    virtual std::set<CalStandard::Virtual::Type> supportedStandardTypes() override {return {};}
    virtual Type getType() override {return Type::SlidingLoad;}
};

class Reflect : public OnePort
{
    Q_OBJECT
public:
    Reflect(Calibration *cal) :
        OnePort(cal){setFirstSupportedStandard();}

    virtual std::set<CalStandard::Virtual::Type> supportedStandardTypes() override {return {CalStandard::Virtual::Type::Open,
                    CalStandard::Virtual::Type::Short, CalStandard::Virtual::Type::Reflect};}
    virtual Type getType() override {return Type::Reflect;}
};

class TwoPort : public Base
{
    Q_OBJECT
public:
    TwoPort(Calibration *cal) :
        Base(cal),
        port1(0),
        port2(0),
        reverseStandard(false){}

    virtual double minFreq() override;
    virtual double maxFreq() override;
    virtual unsigned int numPoints() override {return points.size();}

    virtual void clearPoints();
    virtual void addPoint(const VirtualDevice::VNAMeasurement &m);

    virtual QWidget* createSettingsWidget() override;

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    class Point {
    public:
        double frequency;
        Sparam S;
    };
    std::vector<Point> getPoints() const;

    Sparam getMeasured(double frequency);
    Sparam getActual(double frequency);

    int getPort1() const;
    int getPort2() const;


public slots:
    void setPort1(int p);
    void setPort2(int p);
    void setReverseStandard(bool reverse);
protected:
signals:
    void port1Changed(int p);
    void port2Changed(int p);
    void reverseStandardChanged(bool r);
protected:
    int port1, port2; // starts at 1
    bool reverseStandard; // Set to true if standard is defined with ports swapped
    std::vector<Point> points;
};

class Through : public TwoPort
{
    Q_OBJECT
public:
    Through(Calibration *cal) :
        TwoPort(cal){setFirstSupportedStandard();}
    virtual std::set<CalStandard::Virtual::Type> supportedStandardTypes() override {return {CalStandard::Virtual::Type::Through};}
    virtual Type getType() override {return Type::Through;}
};

class Isolation : public Base
{
public:
    Isolation(Calibration *cal) :
        Base(cal){}

    virtual double minFreq() override;
    virtual double maxFreq() override;
    virtual unsigned int numPoints() override;

    virtual void clearPoints();
    virtual void addPoint(const VirtualDevice::VNAMeasurement &m);

    virtual QWidget* createStandardWidget() override;
    virtual QWidget* createSettingsWidget() override;

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    class Point {
    public:
        double frequency;
        std::vector<std::vector<std::complex<double>>> S;
    };
    std::vector<Point> getPoints() const;

    std::complex<double> getMeasured(double frequency, unsigned int portRcv, unsigned int portSrc);

    virtual std::set<CalStandard::Virtual::Type> supportedStandardTypes() override {return {};}
    virtual Type getType() override {return Type::Isolation;}

protected:
    std::vector<Point> points;
};

class Line : public TwoPort
{
    Q_OBJECT
public:
    Line(Calibration *cal) :
        TwoPort(cal){setFirstSupportedStandard();}
    virtual std::set<CalStandard::Virtual::Type> supportedStandardTypes() override {return {CalStandard::Virtual::Type::Line};}
    virtual Type getType() override {return Type::Line;}
};

}
#endif // CALIBRATIONMEASUREMENT_H
