#ifndef TOUCHSTONE_H
#define TOUCHSTONE_H

#include "savable.h"

#include <complex>
#include <vector>
#include <string>
#include <QString>

class Touchstone : public Savable
{
public:
    enum class Scale {
        Hz,
        kHz,
        MHz,
        GHz,
    };

    enum class Format {
        DBAngle,
        MagnitudeAngle,
        RealImaginary,
    };

    class Datapoint {
    public:
        double frequency;
        std::vector<std::complex<double>> S;
    };

    Touchstone(unsigned int m_ports);
    virtual ~Touchstone(){};
    void AddDatapoint(Datapoint p);
    void toFile(QString filename, Scale unit = Scale::GHz, Format format = Format::RealImaginary);
    std::stringstream toString(Scale unit = Scale::GHz, Format format = Format::RealImaginary);
    static Touchstone fromFile(std::string filename);
    double minFreq();
    double maxFreq();
    unsigned int points() { return m_datapoints.size(); };
    Datapoint point(int index) { return m_datapoints.at(index); };
    Datapoint interpolate(double frequency);
    // remove all paramaters except the ones regarding port1 and port2 (port cnt starts at 0)
    void reduceTo2Port(unsigned int port1, unsigned int port2);
    // remove all paramaters except the ones from port (port cnt starts at 0)
    void reduceTo1Port(unsigned int port);
    unsigned int ports() { return m_ports; }
    QString getFilename() const;

    virtual nlohmann::json toJSON();
    virtual void fromJSON(nlohmann::json j);

    double getReferenceImpedance() const;
    void setReferenceImpedance(double value);

private:
    unsigned int m_ports;
    double referenceImpedance;
    std::vector<Datapoint> m_datapoints;
    QString filename;
};

#endif // TOUCHSTONE_H
