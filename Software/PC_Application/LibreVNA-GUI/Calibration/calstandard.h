#ifndef CALSTANDARD_H
#define CALSTANDARD_H

#include "savable.h"
#include "touchstone.h"
#include "Tools/parameters.h"

#include <complex>
#include <functional>

namespace CalStandard
{

class Virtual : public QObject, public Savable
{
    Q_OBJECT
public:
    Virtual(QString name = "");
    virtual ~Virtual(){emit deleted();}

    enum class Type {
        Open,
        Short,
        Load,
        Reflect,
        Through,
        Line,
        Last
    };

    static Virtual* create(Type type);

    static std::vector<Type> availableTypes();
    static QString TypeToString(Type type);
    static Type TypeFromString(QString s);

    virtual Type getType() = 0;
    double minFrequency() {return minFreq;}
    double maxFrequency() {return maxFreq;}

    virtual void edit(std::function<void(void)> finishedCallback) = 0;
    virtual QString getDescription();

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    unsigned long long getID();

    QString getName() const;
    void setName(const QString &value);

signals:
    void deleted();

protected:
    QString name;
    double minFreq;
    double maxFreq;
    unsigned long long id;
};

class OnePort : public Virtual
{
public:
    OnePort(QString name = "") :
        Virtual(name),
        touchstone(nullptr){}

    virtual std::complex<double> toS11(double freq) = 0;

    void setMeasurement(const Touchstone &ts, int port = 0);
    void clearMeasurement();

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

protected:
    Touchstone *touchstone;
};

class Calkit;

class Open : public OnePort
{
public:
    Open();
    Open(QString name, double Z0, double delay, double loss, double C0, double C1, double C2, double C3)
        : OnePort(name), Z0(Z0), delay(delay), loss(loss), C0(C0), C1(C1), C2(C2), C3(C3){}

    virtual std::complex<double> toS11(double freq) override;
    virtual void edit(std::function<void(void)> finishedCallback = nullptr) override;
    virtual Type getType() override {return Type::Open;}
    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;
private:
    double Z0, delay, loss, C0, C1, C2, C3;
};

class Short : public OnePort
{
public:
    Short();
    Short(QString name, double Z0, double delay, double loss, double L0, double L1, double L2, double L3)
        : OnePort(name), Z0(Z0), delay(delay), loss(loss), L0(L0), L1(L1), L2(L2), L3(L3){}

    virtual std::complex<double> toS11(double freq) override;
    virtual void edit(std::function<void(void)> finishedCallback = nullptr) override;
    virtual Type getType() override {return Type::Short;}
    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;
private:
    double Z0, delay, loss, L0, L1, L2, L3;
};

class Load : public OnePort
{
public:
    Load();
    Load(QString name, double Z0, double delay, double loss, double resistance, double Cparallel, double Lseries, bool Cfirst = true)
        : OnePort(name), Z0(Z0), delay(delay), loss(loss), resistance(resistance), Cparallel(Cparallel), Lseries(Lseries), Cfirst(Cfirst){}

    virtual std::complex<double> toS11(double freq) override;
    virtual void edit(std::function<void(void)> finishedCallback = nullptr) override;
    virtual Type getType() override {return Type::Load;}
    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;
private:
    double Z0, delay, loss, resistance, Cparallel, Lseries;
    bool Cfirst;
};

class Reflect : public OnePort
{
public:
    Reflect();
    Reflect(QString name, bool isShort)
        : OnePort(name), isShort(isShort){}

    virtual std::complex<double> toS11(double freq) override;
    virtual void edit(std::function<void(void)> finishedCallback = nullptr) override;
    virtual Type getType() override {return Type::Reflect;}
    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;
    bool getIsShort() const;

private:
    bool isShort;
};


class TwoPort : public Virtual
{
public:
    TwoPort(QString name = "") :
        Virtual(name),
        touchstone(nullptr){}

    virtual Sparam toSparam(double freq) = 0;

    void setMeasurement(const Touchstone &ts, int port1 = 0, int port2 = 1);
    void clearMeasurement();

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

protected:
    Touchstone *touchstone;
};

class Through : public TwoPort
{
public:
    Through();
    Through(QString name, double Z0, double delay, double loss)
        : TwoPort(name), Z0(Z0), delay(delay), loss(loss){}

    virtual Sparam toSparam(double freq) override;
    virtual void edit(std::function<void(void)> finishedCallback = nullptr) override;
    virtual Type getType() override {return Type::Through;}
    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;
private:
    double Z0, delay, loss;
};

class Line : public TwoPort
{
public:
    Line();
    Line(QString name, double Z0, double delay)
        : TwoPort(name), Z0(Z0), delay(delay){}

    virtual Sparam toSparam(double freq) override;
    virtual void edit(std::function<void(void)> finishedCallback = nullptr) override;
    virtual Type getType() override {return Type::Line;}
    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;
private:
    void setDelay(double delay);
    double Z0, delay;
};

}

#endif // CALSTANDARD_H
