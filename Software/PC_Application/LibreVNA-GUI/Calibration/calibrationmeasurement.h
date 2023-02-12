#ifndef CALIBRATIONMEASUREMENT_H
#define CALIBRATIONMEASUREMENT_H

#include "calstandard.h"
#include "Device/devicedriver.h"

#include <QDateTime>
#include <QObject>
#include <QTableWidgetItem>

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

    virtual QTableWidgetItem *getStatisticsItem();
    QString getStatistics();

    virtual double minUsableFreq() = 0;
    virtual double maxUsableFreq() = 0;
    virtual double minFreq() = 0;
    virtual double maxFreq() = 0;
    virtual unsigned int numPoints() = 0;
    virtual bool readyForMeasurement() {return false;}
    virtual bool readyForCalculation() {return false;}

    static std::vector<Type> availableTypes();
    static QString TypeToString(Type type);
    static Type TypeFromString(QString s);
    virtual std::set<CalStandard::Virtual::Type> supportedStandardTypes() = 0;
    virtual Type getType() = 0;

    virtual void clearPoints() = 0;
    virtual void addPoint(const DeviceDriver::VNAMeasurement &m) = 0;

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

    virtual QTableWidgetItem *getStatisticsItem() override;

    virtual double minUsableFreq() override;
    virtual double maxUsableFreq() override;
    virtual double minFreq() override {return points.size() > 0 ? points.front().frequency : std::numeric_limits<double>::max();}
    virtual double maxFreq() override {return points.size() > 0 ? points.back().frequency : 0;}
    virtual unsigned int numPoints() override {return points.size();}
    virtual bool readyForMeasurement() override {return standard != nullptr;}
    virtual bool readyForCalculation() override {return standard && points.size() > 0;}

    virtual void clearPoints() override;
    virtual void addPoint(const DeviceDriver::VNAMeasurement &m) override;

    virtual QWidget* createSettingsWidget() override;

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    class Point {
    public:
        double frequency;
        std::complex<double> S;

        Point operator*(double scalar) const {
            Point ret;
            ret.frequency = frequency * scalar;
            ret.S = S * scalar;
            return ret;
        }
        Point operator+(const Point &p) const {
            Point ret;
            ret.frequency = frequency + p.frequency;
            ret.S = S + p.S;
            return ret;
        }
    };
    std::vector<Point> getPoints() const;

    std::complex<double> getMeasured(double frequency);
    std::complex<double> getActual(double frequency);

    int getPort() const;

public slots:
    void setPort(unsigned int p);
protected:
signals:
    void portChanged(int p);
protected:
    unsigned int port; // starts at 1
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

    virtual QTableWidgetItem *getStatisticsItem() override;

    virtual double minUsableFreq() override;
    virtual double maxUsableFreq() override;
    virtual double minFreq() override {return points.size() > 0 ? points.front().frequency : std::numeric_limits<double>::max();}
    virtual double maxFreq() override {return points.size() > 0 ? points.back().frequency : 0;}
    virtual unsigned int numPoints() override {return points.size();}
    virtual bool readyForMeasurement() override {return standard != nullptr;}
    virtual bool readyForCalculation() override {return standard && points.size() > 0;}

    virtual void clearPoints() override;
    virtual void addPoint(const DeviceDriver::VNAMeasurement &m) override;

    virtual QWidget* createSettingsWidget() override;

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    class Point {
    public:
        double frequency;
        Sparam S;

        Point operator*(double scalar) const {
            Point ret;
            ret.frequency = frequency * scalar;
            ret.S = S * scalar;
            return ret;
        }
        Point operator+(const Point &p) const {
            Point ret;
            ret.frequency = frequency + p.frequency;
            ret.S = S + p.S;
            return ret;
        }
    };
    std::vector<Point> getPoints() const;

    Sparam getMeasured(double frequency);
    Sparam getActual(double frequency);

    int getPort1() const;
    int getPort2() const;


public slots:
    void setPort1(unsigned int p);
    void setPort2(unsigned int p);
    void setReverseStandard(bool reverse);
protected:
signals:
    void port1Changed(int p);
    void port2Changed(int p);
    void reverseStandardChanged(bool r);
protected:
    unsigned int port1, port2; // starts at 1
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

    virtual double minUsableFreq() override {return minFreq();}
    virtual double maxUsableFreq() override {return maxFreq();}
    virtual double minFreq() override {return points.size() > 0 ? points.front().frequency : std::numeric_limits<double>::max();}
    virtual double maxFreq() override {return points.size() > 0 ? points.back().frequency : 0;}
    virtual unsigned int numPoints() override;
    virtual bool readyForMeasurement() override {return true;}
    virtual bool readyForCalculation() override {return points.size() > 0;}

    virtual void clearPoints() override;
    virtual void addPoint(const DeviceDriver::VNAMeasurement &m) override;

    virtual QWidget* createStandardWidget() override;
    virtual QWidget* createSettingsWidget() override;

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    class Point {
    public:
        double frequency;
        std::vector<std::vector<std::complex<double>>> S;

        Point operator*(double scalar) const {
            Point ret;
            ret.frequency = frequency * scalar;
            for(const auto &v1 : S) {
                std::vector<std::complex<double>> v;
                for(const auto &s : v1) {
                    v.push_back(s * scalar);
                }
                ret.S.push_back(v);
            }
            return ret;
        }
        Point operator+(const Point &p) const {
            Point ret;
            ret.frequency = frequency + p.frequency;
            if(S.size() != p.S.size()) {
                throw std::runtime_error("Points to not have the same number of measurements");
            }
            for(unsigned int i=0;i<S.size();i++) {
                std::vector<std::complex<double>> v;
                if(S[i].size() != p.S[i].size()) {
                    throw std::runtime_error("Points to not have the same number of measurements");
                }
                for(unsigned int j=0;j<S[i].size();j++) {
                    v.push_back(S[i][j] + p.S[i][j]);
                }
                ret.S.push_back(v);
            }
            return ret;
        }
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
