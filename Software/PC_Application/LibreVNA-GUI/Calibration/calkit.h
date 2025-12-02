#ifndef CALKIT_H
#define CALKIT_H

#include "touchstone.h"
#include "Util/qpointervariant.h"
#include "calstandard.h"
#include "savable.h"
#include "scpi.h"

#include "LibreCAL/caldevice.h"

#include <string>
#include <complex>
#include <QDir>

class Calkit : public Savable, public SCPINode
{
    friend class CalkitDialog;
    friend class LibreCALDialog;
public:
    Calkit();
    Calkit(const Calkit&) = delete;
    Calkit& operator= (const Calkit&) = delete;

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
    bool fromFile(QString filename);
    void edit(std::function<void(void)> updateCal = nullptr);

    std::vector<CalStandard::Virtual *> getStandards() const;
    void addStandard(CalStandard::Virtual* s);
    void removeStandard(CalStandard::Virtual* s);

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    void setIdealDefault();

private:
    void clearStandards();
    void updateSCPINames();
    QString manufacturer, serialnumber, description;
    QString filename;
    SCPINode scpi_std;
    std::vector<CalStandard::Virtual*> standards;

    const std::vector<Savable::SettingDescription> descr = {{
        {&manufacturer, "Manufacturer", ""},
        {&serialnumber, "Serialnumber", ""},
        {&description, "Description", ""},
    }};
};

#endif // CALKIT_H
