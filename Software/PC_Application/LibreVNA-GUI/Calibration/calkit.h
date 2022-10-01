#ifndef CALKIT_H
#define CALKIT_H

#include "touchstone.h"
#include "Util/qpointervariant.h"
#include "calstandard.h"
#include "savable.h"

#include "LibreCAL/caldevice.h"

#include <string>
#include <complex>
#include <QDir>

class Calkit : public Savable
{
    friend class CalkitDialog;
    friend class LibreCALDialog;
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

    std::vector<CalStandard::Virtual *> getStandards() const;

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

private:
    void clearStandards();
    QString manufacturer, serialnumber, description;
    std::vector<CalStandard::Virtual*> standards;

    const std::vector<Savable::SettingDescription> descr = {{
        {&manufacturer, "Manufacturer", ""},
        {&serialnumber, "Serialnumber", ""},
        {&description, "Description", ""},
    }};
};

#endif // CALKIT_H
