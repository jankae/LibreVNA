#ifndef CALKIT_H
#define CALKIT_H

#include <string>
#include <complex>
#include "touchstone.h"
#include "Util/qpointervariant.h"
#include <QDir>

class Calkit
{
    friend class CalkitDialog;
public:
    Calkit();
    Calkit& operator=(const Calkit& other)
    {
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
    void edit(std::function<void(void)> done = nullptr);
    SOLT toSOLT(double frequency);
    TRL toTRL(double frequency);
    double minFreq(bool trl = false);
    double maxFreq(bool trl = false);
    bool isTRLReflectionShort() const;

private:
    void TransformPathsToRelative(QFileInfo d);
    void TransformPathsToAbsolute(QFileInfo d);
    // SOLT standard definitions
    struct {
        struct {
            double Z0, delay, loss, C0, C1, C2, C3;
            QString file;
            bool useMeasurements;
            int Sparam;
        } Open;
        struct {
            double Z0, delay, loss, L0, L1, L2, L3;
            QString file;
            bool useMeasurements;
            int Sparam;
        } Short;
        struct {
            double Z0, delay, Cparallel, Lseries;
            QString file;
            bool useMeasurements;
            int Sparam;
        } Load;
        struct {
            double Z0, delay, loss;
            QString file;
            bool useMeasurements;
            int Sparam1, Sparam2;
        } Through;
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

    Touchstone *ts_open, *ts_short, *ts_load, *ts_through;
    bool ts_cached;

    using JSONDescription = struct _jsondescr {
        QPointerVariant var;
        QString name;
        QVariant def;
    };
    const std::array<JSONDescription, 40> json_descr = {{
        {&SOLT.Open.Z0, "SOLT/Open/Param/Z0", 50.0},
        {&SOLT.Open.delay, "SOLT/Open/Param/Delay", 0.0},
        {&SOLT.Open.loss, "SOLT/Open/Param/Loss", 0.0},
        {&SOLT.Open.C0, "SOLT/Open/Param/C0", 0.0},
        {&SOLT.Open.C1, "SOLT/Open/Param/C1", 0.0},
        {&SOLT.Open.C2, "SOLT/Open/Param/C2", 0.0},
        {&SOLT.Open.C3, "SOLT/Open/Param/C3", 0.0},
        {&SOLT.Open.useMeasurements, "SOLT/Open/Measurements/Use", false},
        {&SOLT.Open.file, "SOLT/Open/Measurements/File", ""},
        {&SOLT.Open.Sparam, "SOLT/Open/Measurements/Port", 0},

        {&SOLT.Short.Z0, "SOLT/Short/Param/Z0", 50.0},
        {&SOLT.Short.delay, "SOLT/Short/Param/Delay", 0.0},
        {&SOLT.Short.loss, "SOLT/Short/Param/Loss", 0.0},
        {&SOLT.Short.L0, "SOLT/Short/Param/L0", 0.0},
        {&SOLT.Short.L1, "SOLT/Short/Param/L1", 0.0},
        {&SOLT.Short.L2, "SOLT/Short/Param/L2", 0.0},
        {&SOLT.Short.L3, "SOLT/Short/Param/L3", 0.0},
        {&SOLT.Short.useMeasurements, "SOLT/Short/Measurements/Use", false},
        {&SOLT.Short.file, "SOLT/Short/Measurements/File", ""},
        {&SOLT.Short.Sparam, "SOLT/Short/Measurements/Port", 0},

        {&SOLT.Load.Z0, "SOLT/Load/Param/Z0", 50.0},
        {&SOLT.Load.delay, "SOLT/Load/Param/Delay", 0.0},
        {&SOLT.Load.Cparallel, "SOLT/Load/Param/C", 0.0},
        {&SOLT.Load.Lseries, "SOLT/Load/Param/L", 0.0},
        {&SOLT.Load.useMeasurements, "SOLT/Load/Measurements/Use", false},
        {&SOLT.Load.file, "SOLT/Load/Measurements/File", ""},
        {&SOLT.Load.Sparam, "SOLT/Load/Measurements/Port", 0},

        {&SOLT.Through.Z0, "SOLT/Through/Param/Z0", 50.0},
        {&SOLT.Through.delay, "SOLT/Through/Param/Delay", 0.0},
        {&SOLT.Through.loss, "SOLT/Through/Param/Loss", 0.0},
        {&SOLT.Through.useMeasurements, "SOLT/Through/Measurements/Use", false},
        {&SOLT.Through.file, "SOLT/Through/Measurements/File", ""},
        {&SOLT.Through.Sparam1, "SOLT/Through/Measurements/Port1", 0},
        {&SOLT.Through.Sparam2, "SOLT/Through/Measurements/Port2", 1},

        {&TRL.Through.Z0, "TRL/Through/Z0", 50.0},
        {&TRL.Reflection.isShort, "TRL/Reflect/isShort", false},
        {&TRL.Line.delay, "TRL/Line/Delay", 74.0},
        {&TRL.Line.minFreq, "TRL/Line/minFreq", 751000000.0},
        {&TRL.Line.maxFreq, "TRL/Line/maxFreq", 6000000000.0},

        {&startDialogWithSOLT, "StartDialogWithSOLT", true}
    }};

    void clearTouchstoneCache();
    void fillTouchstoneCache();
};

#endif // CALKIT_H
