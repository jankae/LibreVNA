#ifndef CALKIT_H
#define CALKIT_H

#include "touchstone.h"
#include "Util/qpointervariant.h"
#include "calstandard.h"

#include <string>
#include <complex>
#include <QDir>

class Calkit
{
    friend class CalkitDialog;
public:
    Calkit();
    Calkit(const Calkit&) = default;
    Calkit& operator=(const Calkit& other)
    {
        this->manufacturer = other.manufacturer;
        this->serialnumber = other.serialnumber;
        this->description = other.description;
        this->standards = other.standards;
        return *this;
    }

    class SOLT {
    public:
        std::complex<double> Open;
        std::complex<double> Short;
        std::complex<double> Load;
        std::complex<double> ThroughS11, ThroughS12, ThroughS21, ThroughS22;
    };

    class TRL {
    public:
        bool reflectionIsNegative;
        std::complex<double> ThroughS11, ThroughS12, ThroughS21, ThroughS22;
    };

    void toFile(QString filename);
    static Calkit fromFile(QString filename);
    void edit(std::function<void(void)> updateCal = nullptr);
    bool hasSeparateMaleFemaleStandards();
    SOLT toSOLT(double frequency, bool male_standards = true);
    TRL toTRL(double frequency);
    double minFreqTRL();
    double maxFreqTRL();
    double minFreqSOLT(bool male_standards = true);
    double maxFreqSOLT(bool male_standards = true);
    bool checkIfValid(double min_freq, double max_freq, bool isTRL, bool include_male, bool include_female);
    bool isTRLReflectionShort() const;

private:
    QString manufacturer, serialnumber, description;
    std::vector<CalStandard::Virtual*> standards;

    const std::vector<Savable::SettingDescription> descr = {{
        {&manufacturer, "Manufacturer", ""},
        {&serialnumber, "Serialnumber", ""},
        {&description, "Description", ""},
    }};
};

#endif // CALKIT_H
