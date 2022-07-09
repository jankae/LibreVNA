#ifndef CALKIT_H
#define CALKIT_H

#include "touchstone.h"
#include "Util/qpointervariant.h"

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
        this->SOLT = other.SOLT;
        this->TRL = other.TRL;
        this->startDialogWithSOLT = other.startDialogWithSOLT;
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
    void TransformPathsToRelative(QFileInfo d);
    void TransformPathsToAbsolute(QFileInfo d);

    QString manufacturer, serialnumber, description;
    // SOLT standard definitions
    struct {
        using Open = struct {
            double Z0, delay, loss, C0, C1, C2, C3;
            QString file;
            bool useMeasurements;
            int Sparam;
        };
        Open open_m, open_f;
        using Short = struct {
            double Z0, delay, loss, L0, L1, L2, L3;
            QString file;
            bool useMeasurements;
            int Sparam;
        };
        Short short_m, short_f;
        using Load = struct {
            double resistance, Z0, delay, Cparallel, Lseries;
            QString file;
            bool useMeasurements;
            int Sparam;
        };
        Load load_m, load_f;
        bool loadModelCFirst;
        struct {
            double Z0, delay, loss;
            QString file;
            bool useMeasurements;
            int Sparam1, Sparam2;
        } Through;
        bool separate_male_female;
    } SOLT;
    struct {
        struct {
            double Z0;
        } Through;
        struct {
            bool isShort;
        } Reflection;
        struct {
            double delay, minFreq, maxFreq;
        } Line;
    } TRL;
    bool startDialogWithSOLT;

    Touchstone *ts_open_m, *ts_short_m, *ts_load_m;
    Touchstone *ts_open_f, *ts_short_f, *ts_load_f;
    Touchstone *ts_through;
    bool ts_cached;

    const std::vector<Savable::SettingDescription> descr = {{
        {&manufacturer, "Manufacturer", ""},
        {&serialnumber, "Serialnumber", ""},
        {&description, "Description", ""},

        {&SOLT.open_m.Z0, "SOLT.Open.Param.Z0", 50.0},
        {&SOLT.open_m.delay, "SOLT.Open.Param.Delay", 0.0},
        {&SOLT.open_m.loss, "SOLT.Open.Param.Loss", 0.0},
        {&SOLT.open_m.C0, "SOLT.Open.Param.C0", 0.0},
        {&SOLT.open_m.C1, "SOLT.Open.Param.C1", 0.0},
        {&SOLT.open_m.C2, "SOLT.Open.Param.C2", 0.0},
        {&SOLT.open_m.C3, "SOLT.Open.Param.C3", 0.0},
        {&SOLT.open_m.useMeasurements, "SOLT.Open.Measurements.Use", false},
        {&SOLT.open_m.file, "SOLT.Open.Measurements.File", ""},
        {&SOLT.open_m.Sparam, "SOLT.Open.Measurements.Port", 0},
        {&SOLT.open_f.Z0, "SOLT.Open.Param.Z0_Female", 50.0},
        {&SOLT.open_f.delay, "SOLT.Open.Param.Delay_Female", 0.0},
        {&SOLT.open_f.loss, "SOLT.Open.Param.Loss_Female", 0.0},
        {&SOLT.open_f.C0, "SOLT.Open.Param.C0_Female", 0.0},
        {&SOLT.open_f.C1, "SOLT.Open.Param.C1_Female", 0.0},
        {&SOLT.open_f.C2, "SOLT.Open.Param.C2_Female", 0.0},
        {&SOLT.open_f.C3, "SOLT.Open.Param.C3_Female", 0.0},
        {&SOLT.open_f.useMeasurements, "SOLT.Open.Measurements.Use_Female", false},
        {&SOLT.open_f.file, "SOLT.Open.Measurements.File_Female", ""},
        {&SOLT.open_f.Sparam, "SOLT.Open.Measurements.Port_Female", 0},

        {&SOLT.short_m.Z0, "SOLT.Short.Param.Z0", 50.0},
        {&SOLT.short_m.delay, "SOLT.Short.Param.Delay", 0.0},
        {&SOLT.short_m.loss, "SOLT.Short.Param.Loss", 0.0},
        {&SOLT.short_m.L0, "SOLT.Short.Param.L0", 0.0},
        {&SOLT.short_m.L1, "SOLT.Short.Param.L1", 0.0},
        {&SOLT.short_m.L2, "SOLT.Short.Param.L2", 0.0},
        {&SOLT.short_m.L3, "SOLT.Short.Param.L3", 0.0},
        {&SOLT.short_m.useMeasurements, "SOLT.Short.Measurements.Use", false},
        {&SOLT.short_m.file, "SOLT.Short.Measurements.File", ""},
        {&SOLT.short_m.Sparam, "SOLT.Short.Measurements.Port", 0},
        {&SOLT.short_f.Z0, "SOLT.Short.Param.Z0_Female", 50.0},
        {&SOLT.short_f.delay, "SOLT.Short.Param.Delay_Female", 0.0},
        {&SOLT.short_f.loss, "SOLT.Short.Param.Loss_Female", 0.0},
        {&SOLT.short_f.L0, "SOLT.Short.Param.L0_Female", 0.0},
        {&SOLT.short_f.L1, "SOLT.Short.Param.L1_Female", 0.0},
        {&SOLT.short_f.L2, "SOLT.Short.Param.L2_Female", 0.0},
        {&SOLT.short_f.L3, "SOLT.Short.Param.L3_Female", 0.0},
        {&SOLT.short_f.useMeasurements, "SOLT.Short.Measurements.Use_Female", false},
        {&SOLT.short_f.file, "SOLT.Short.Measurements.File_Female", ""},
        {&SOLT.short_f.Sparam, "SOLT.Short.Measurements.Port_Female", 0},

        {&SOLT.loadModelCFirst, "SOLT.loadModelCFirst", true},
        {&SOLT.load_m.resistance, "SOLT.Load.Param.Resistance", 50.0},
        {&SOLT.load_m.Z0, "SOLT.Load.Param.Z0", 50.0},
        {&SOLT.load_m.delay, "SOLT.Load.Param.Delay", 0.0},
        {&SOLT.load_m.Cparallel, "SOLT.Load.Param.C", 0.0},
        {&SOLT.load_m.Lseries, "SOLT.Load.Param.L", 0.0},
        {&SOLT.load_m.useMeasurements, "SOLT.Load.Measurements.Use", false},
        {&SOLT.load_m.file, "SOLT.Load.Measurements.File", ""},
        {&SOLT.load_m.Sparam, "SOLT.Load.Measurements.Port", 0},
        {&SOLT.load_f.resistance, "SOLT.Load.Param.Resistance_Female", 50.0},
        {&SOLT.load_f.Z0, "SOLT.Load.Param.Z0_Female", 50.0},
        {&SOLT.load_f.delay, "SOLT.Load.Param.Delay_Female", 0.0},
        {&SOLT.load_f.Cparallel, "SOLT.Load.Param.C_Female", 0.0},
        {&SOLT.load_f.Lseries, "SOLT.Load.Param.L_Female", 0.0},
        {&SOLT.load_f.useMeasurements, "SOLT.Load.Measurements.Use_Female", false},
        {&SOLT.load_f.file, "SOLT.Load.Measurements.File_Female", ""},
        {&SOLT.load_f.Sparam, "SOLT.Load.Measurements.Port_Female", 0},

        {&SOLT.Through.Z0, "SOLT.Through.Param.Z0", 50.0},
        {&SOLT.Through.delay, "SOLT.Through.Param.Delay", 0.0},
        {&SOLT.Through.loss, "SOLT.Through.Param.Loss", 0.0},
        {&SOLT.Through.useMeasurements, "SOLT.Through.Measurements.Use", false},
        {&SOLT.Through.file, "SOLT.Through.Measurements.File", ""},
        {&SOLT.Through.Sparam1, "SOLT.Through.Measurements.Port1", 0},
        {&SOLT.Through.Sparam2, "SOLT.Through.Measurements.Port2", 1},

        {&SOLT.separate_male_female, "SOLT.SeparateMaleFemale", false},

        {&TRL.Through.Z0, "TRL.Through.Z0", 50.0},
        {&TRL.Reflection.isShort, "TRL.Reflect.isShort", false},
        {&TRL.Line.delay, "TRL.Line.Delay", 74.0},
        {&TRL.Line.minFreq, "TRL.Line.minFreq", 751000000.0},
        {&TRL.Line.maxFreq, "TRL.Line.maxFreq", 6000000000.0},

        {&startDialogWithSOLT, "StartDialogWithSOLT", true}
    }};

    void clearTouchstoneCache();
    void fillTouchstoneCache();
};

#endif // CALKIT_H
