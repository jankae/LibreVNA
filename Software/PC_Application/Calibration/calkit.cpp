#include "calkit.h"

#include "calkitdialog.h"
#include "json.hpp"
#include "CustomWidgets/informationbox.h"

#include <fstream>
#include <iomanip>
#include <QMessageBox>
#include <QDebug>
#include <math.h>

using json = nlohmann::json;
using namespace std;

Calkit::Calkit()
 : ts_open_m(nullptr),
   ts_short_m(nullptr),
   ts_load_m(nullptr),
   ts_open_f(nullptr),
   ts_short_f(nullptr),
   ts_load_f(nullptr),
   ts_through(nullptr),
   ts_cached(false)
{

    // set default values
    for(auto e : descr) {
        e.var.setValue(e.def);
    }
}

void Calkit::toFile(QString filename)
{
    if(!filename.endsWith(".calkit")) {
        filename.append(".calkit");
    }

    qDebug() << "Saving calkit to file" << filename;

    TransformPathsToRelative(filename);

    json j = Savable::createJSON(descr);
    ofstream file;
    file.open(filename.toStdString());
    file << setw(4) << j << endl;
    file.close();

    TransformPathsToAbsolute(filename);
}

static QString readLine(ifstream &file) {
    string line;
    getline(file, line);
    return QString::fromStdString(line).simplified();
}

Calkit Calkit::fromFile(QString filename)
{
    qDebug() << "Opening calkit to file" << filename;

    auto c = Calkit();
    ifstream file;
    file.open(filename.toStdString());
    if(!file.is_open()) {
        throw runtime_error("Unable to open file");
    }

    json j;
    try {
        file >> j;
    } catch (exception &e) {
        throw runtime_error("JSON parsing error: " + string(e.what()));
    }
    if(j.contains("SOLT")) {
        // older file versions specify Z0 for resistance. Set resistance to Nan to detect missing values later
        c.SOLT.load_m.resistance = std::numeric_limits<double>::quiet_NaN();
        c.SOLT.load_f.resistance = std::numeric_limits<double>::quiet_NaN();

        qDebug() << "JSON format detected";
        // calkit file uses json format, parse
        Savable::parseJSON(j, c.descr);
        // adjust Z0/resistance in case of older calkit file version with missing resistance entries
        if(isnan(c.SOLT.load_f.resistance)) {
            c.SOLT.load_f.resistance = c.SOLT.load_f.Z0;
            c.SOLT.load_f.Z0 = 50.0;
        }
        if(isnan(c.SOLT.load_m.resistance)) {
            c.SOLT.load_m.resistance = c.SOLT.load_m.Z0;
            c.SOLT.load_m.Z0 = 50.0;
        }
    } else {
        qDebug() << "Legacy format detected";
        // legacy file format, return to beginning of file
        file.clear();
        file.seekg(0);
        c.SOLT.open_m.useMeasurements = readLine(file).toInt();
        c.SOLT.short_m.useMeasurements = readLine(file).toInt();
        c.SOLT.load_m.useMeasurements = readLine(file).toInt();
        c.SOLT.Through.useMeasurements = readLine(file).toInt();
        c.SOLT.open_m.Z0 = readLine(file).toDouble();
        c.SOLT.open_m.delay = readLine(file).toDouble();
        c.SOLT.open_m.loss = readLine(file).toDouble();
        c.SOLT.open_m.C0 = readLine(file).toDouble();
        c.SOLT.open_m.C1 = readLine(file).toDouble();
        c.SOLT.open_m.C2 = readLine(file).toDouble();
        c.SOLT.open_m.C3 = readLine(file).toDouble();
        c.SOLT.short_m.Z0 = readLine(file).toDouble();
        c.SOLT.short_m.delay = readLine(file).toDouble();
        c.SOLT.short_m.loss = readLine(file).toDouble();
        c.SOLT.short_m.L0 = readLine(file).toDouble();
        c.SOLT.short_m.L1 = readLine(file).toDouble();
        c.SOLT.short_m.L2 = readLine(file).toDouble();
        c.SOLT.short_m.L3 = readLine(file).toDouble();
        c.SOLT.load_m.resistance = readLine(file).toDouble();
        c.SOLT.Through.Z0 = readLine(file).toDouble();
        c.SOLT.Through.delay = readLine(file).toDouble();
        c.SOLT.Through.loss = readLine(file).toDouble();
        if(c.SOLT.open_m.useMeasurements) {
            c.SOLT.open_m.file = readLine(file);
            c.SOLT.open_m.Sparam = readLine(file).toInt();
        }
        if(c.SOLT.short_m.useMeasurements) {
            c.SOLT.short_m.file = readLine(file);
            c.SOLT.short_m.Sparam = readLine(file).toInt();
        }
        if(c.SOLT.load_m.useMeasurements) {
            c.SOLT.load_m.file = readLine(file);
            c.SOLT.load_m.Sparam = readLine(file).toInt();
        }
        if(c.SOLT.Through.useMeasurements) {
            c.SOLT.Through.file = readLine(file);
            c.SOLT.Through.Sparam1 = readLine(file).toInt();
            c.SOLT.Through.Sparam2 = readLine(file).toInt();
        }
        c.TRL.Through.Z0 = readLine(file).toDouble();
        c.TRL.Reflection.isShort = readLine(file).toDouble();
        c.TRL.Line.delay = readLine(file).toDouble();
        c.TRL.Line.minFreq = readLine(file).toDouble();
        c.TRL.Line.maxFreq = readLine(file).toDouble();

        c.SOLT.separate_male_female = false;

        InformationBox::ShowMessage("Loading calkit file", "The file \"" + filename + "\" is stored in a deprecated"
                     " calibration kit format. Future versions of this application might not support"
                     " it anymore. Please save the calibration kit to update to the new format");
    }
    file.close();

    c.TransformPathsToAbsolute(filename);

    // set default values for non-editable items (for now)
    c.TRL.Through.Z0 = 50.0;
    c.SOLT.Through.Z0 = 50.0;

    return c;
}

void Calkit::edit(std::function<void (void)> updateCal)
{
    auto dialog = new CalkitDialog(*this);
    if(updateCal) {
        QObject::connect(dialog, &CalkitDialog::settingsChanged, [=](){
            updateCal();
        });
    }
    dialog->show();
}

bool Calkit::hasSeparateMaleFemaleStandards()
{
    return SOLT.separate_male_female;
}

class Calkit::SOLT Calkit::toSOLT(double frequency, bool male_standards)
{
    auto addTransmissionLine = [](complex<double> termination_reflection, double offset_impedance, double offset_delay, double offset_loss, double frequency) -> complex<double> {
        // nomenclature and formulas from https://loco.lab.asu.edu/loco-memos/edges_reports/report_20130807.pdf
        auto Gamma_T = termination_reflection;
        auto f = frequency;
        auto w = 2.0 * M_PI * frequency;
        auto f_sqrt = sqrt(f / 1e9);

        auto Z_c = complex<double>(offset_impedance + (offset_loss / (2*w)) * f_sqrt, -(offset_loss / (2*w)) * f_sqrt);
        auto gamma_l = complex<double>(offset_loss*offset_delay/(2*offset_impedance)*f_sqrt, w*offset_delay+offset_loss*offset_delay/(2*offset_impedance)*f_sqrt);

        auto Z_r = complex<double>(50.0);

        auto Gamma_1 = (Z_c - Z_r) / (Z_c + Z_r);

        auto Gamma_i = (Gamma_1*(1.0-exp(-2.0*gamma_l)-Gamma_1*Gamma_T)+exp(-2.0*gamma_l)*Gamma_T)
                / (1.0-Gamma_1*(exp(-2.0*gamma_l)*Gamma_1+Gamma_T*(1.0-exp(-2.0*gamma_l))));

        return Gamma_i;
    };

    auto Load = male_standards ? SOLT.load_m : SOLT.load_f;
    auto Short = male_standards ? SOLT.short_m : SOLT.short_f;
    auto Open = male_standards ? SOLT.open_m : SOLT.open_f;
    auto ts_load = male_standards ? ts_load_m : ts_load_f;
    auto ts_short = male_standards ? ts_short_m : ts_short_f;
    auto ts_open = male_standards ? ts_open_m : ts_open_f;

    fillTouchstoneCache();
    class SOLT ref;
    if(Load.useMeasurements) {
        ref.Load = ts_load->interpolate(frequency).S[0];
    } else {
        auto imp_load = complex<double>(Load.resistance, 0);
        // Add parallel capacitor to impedance
        if(Load.Cparallel > 0) {
            auto imp_C = complex<double>(0, -1.0 / (frequency * 2 * M_PI * Load.Cparallel));
            imp_load = (imp_load * imp_C) / (imp_load + imp_C);
        }
        // add series inductor to impedance
        auto imp_L = complex<double>(0, frequency * 2 * M_PI * Load.Lseries);
        imp_load += imp_L;
        ref.Load = (imp_load - complex<double>(50.0)) / (imp_load + complex<double>(50.0));
        ref.Load = addTransmissionLine(ref.Load, Load.Z0, Load.delay*1e-12, 0, frequency);
    }

    if(Open.useMeasurements) {
        ref.Open = ts_open->interpolate(frequency).S[0];
    } else {
        // calculate fringing capacitance for open
        double Cfringing = Open.C0 * 1e-15 + Open.C1 * 1e-27 * frequency + Open.C2 * 1e-36 * pow(frequency, 2) + Open.C3 * 1e-45 * pow(frequency, 3);
        // convert to impedance
        if (Cfringing == 0) {
            // special case to avoid issues with infinity
            ref.Open = complex<double>(1.0, 0);
        } else {
            auto imp_open = complex<double>(0, -1.0 / (frequency * 2 * M_PI * Cfringing));
            ref.Open = (imp_open - complex<double>(50.0)) / (imp_open + complex<double>(50.0));
        }
        ref.Open = addTransmissionLine(ref.Open, Open.Z0, Open.delay*1e-12, Open.loss*1e9, frequency);
    }

    if(Short.useMeasurements) {
        ref.Short = ts_short->interpolate(frequency).S[0];
    } else {
        // calculate inductance for short
        double Lseries = Short.L0 * 1e-12 + Short.L1 * 1e-24 * frequency + Short.L2 * 1e-33 * pow(frequency, 2) + Short.L3 * 1e-42 * pow(frequency, 3);
        // convert to impedance
        auto imp_short = complex<double>(0, frequency * 2 * M_PI * Lseries);
        ref.Short =  (imp_short - complex<double>(50.0)) / (imp_short + complex<double>(50.0));
        ref.Short = addTransmissionLine(ref.Short, Short.Z0, Short.delay*1e-12, Short.loss*1e9, frequency);
    }

    if(SOLT.Through.useMeasurements) {
        auto interp = ts_through->interpolate(frequency);
        ref.ThroughS11 = interp.S[0];
        ref.ThroughS12 = interp.S[1];
        ref.ThroughS21 = interp.S[2];
        ref.ThroughS22 = interp.S[3];
    } else {
        // calculate effect of through
        double through_phaseshift = -2 * M_PI * frequency * SOLT.Through.delay * 1e-12;
        double through_att_db = SOLT.Through.loss * 1e9 * 4.3429 * SOLT.Through.delay * 1e-12 / SOLT.Through.Z0 * sqrt(frequency / 1e9);;
        double through_att = pow(10.0, -through_att_db / 10.0);
        ref.ThroughS12 = polar<double>(through_att, through_phaseshift);
        // Assume symmetric and perfectly matched through for other parameters
        ref.ThroughS21 = ref.ThroughS12;
        ref.ThroughS11 = 0.0;
        ref.ThroughS22 = 0.0;
    }

    return ref;
}

class Calkit::TRL Calkit::toTRL(double)
{
    class TRL trl;
    // reflection coefficent sign depends on whether an open or short is used
    trl.reflectionIsNegative = TRL.Reflection.isShort;
    // assume ideal through for now
    trl.ThroughS11 = 0.0;
    trl.ThroughS12 = 1.0;
    trl.ThroughS21 = 1.0;
    trl.ThroughS22 = 0.0;
    return trl;
}

double Calkit::minFreqTRL()
{
    return TRL.Line.minFreq;
}

double Calkit::maxFreqTRL()
{
    return TRL.Line.maxFreq;
}

double Calkit::minFreqSOLT(bool male_standards)
{
    fillTouchstoneCache();
    double min = 0;
    auto ts_load = male_standards ? ts_load_m : ts_load_f;
    auto ts_short = male_standards ? ts_short_m : ts_short_f;
    auto ts_open = male_standards ? ts_open_m : ts_open_f;
    array<Touchstone*, 4> ts_list = {ts_open, ts_short, ts_load, ts_through};
    // find the highest minimum frequency in all measurement files
    for(auto ts : ts_list) {
        if(!ts) {
            // this calibration standard is defined by coefficients, no minimum frequency
            continue;
        }
        if(ts->minFreq() > min) {
            min = ts->minFreq();
        }
    }
    return min;
}

double Calkit::maxFreqSOLT(bool male_standards)
{
    fillTouchstoneCache();
    double max = std::numeric_limits<double>::max();
    auto ts_load = male_standards ? ts_load_m : ts_load_f;
    auto ts_short = male_standards ? ts_short_m : ts_short_f;
    auto ts_open = male_standards ? ts_open_m : ts_open_f;
    array<Touchstone*, 4> ts_list = {ts_open, ts_short, ts_load, ts_through};
    // find the highest minimum frequency in all measurement files
    for(auto ts : ts_list) {
        if(!ts) {
            // this calibration standard is defined by coefficients, no minimum frequency
            continue;
        }
        if(ts->maxFreq() < max) {
            max = ts->maxFreq();
        }
    }
    return max;
}

bool Calkit::checkIfValid(double min_freq, double max_freq, bool isTRL, bool include_male, bool include_female)
{
    auto min_supported = std::numeric_limits<double>::min();
    auto max_supported = std::numeric_limits<double>::max();
    if(isTRL) {
        min_supported = TRL.Line.minFreq;
        max_supported = TRL.Line.maxFreq;
    } else {
        if(include_male) {
            auto min_male = minFreqSOLT(true);
            auto max_male = maxFreqSOLT(true);
            if(min_male > min_supported) {
                min_supported = min_male;
            }
            if(max_male > max_supported) {
                max_supported = max_male;
            }
        }
        if(include_female) {
            auto min_female = minFreqSOLT(false);
            auto max_female = maxFreqSOLT(false);
            if(min_female > min_supported) {
                min_supported = min_female;
            }
            if(max_female > max_supported) {
                max_supported = max_female;
            }
        }
    }
    if(min_supported <= min_freq && max_supported >= max_freq) {
        return true;
    } else {
        return false;
    }
}

bool Calkit::isTRLReflectionShort() const
{
    return TRL.Reflection.isShort;
}

void Calkit::TransformPathsToRelative(QFileInfo d)
{
    vector<QString*> filenames = {&SOLT.short_m.file, &SOLT.open_m.file, &SOLT.load_m.file, &SOLT.short_f.file, &SOLT.open_f.file, &SOLT.load_f.file, &SOLT.Through.file};
    for(auto f : filenames) {
        if(f->isEmpty()) {
            continue;
        }
        if(QFileInfo(*f).isAbsolute()) {
            QString buf = *f;
            *f = d.dir().relativeFilePath(*f);
            qDebug() << "Transformed" << buf << "to" << *f << "(to relative)";
        }
    }
}

void Calkit::TransformPathsToAbsolute(QFileInfo d)
{
    vector<QString*> filenames = {&SOLT.short_m.file, &SOLT.open_m.file, &SOLT.load_m.file, &SOLT.short_f.file, &SOLT.open_f.file, &SOLT.load_f.file, &SOLT.Through.file};
    for(auto f : filenames) {
        if(f->isEmpty()) {
            continue;
        }
        if(QFileInfo(*f).isRelative()) {
            auto absDir = QDir(d.dir().path() + "/" + *f);
            QString buf = *f;
            *f = absDir.absolutePath();
            qDebug() << "Transformed" << buf << "to" << *f << "(to absolute)";
        }
    }
}

void Calkit::clearTouchstoneCache()
{
    delete ts_open_m;
    ts_open_m = nullptr;
    delete ts_short_m;
    ts_short_m = nullptr;
    delete ts_load_m;
    ts_load_m = nullptr;
    delete ts_open_f;
    ts_open_f = nullptr;
    delete ts_short_f;
    ts_short_f = nullptr;
    delete ts_load_f;
    ts_load_f = nullptr;
    delete ts_through;
    ts_through = nullptr;
    ts_cached = false;
}

void Calkit::fillTouchstoneCache()
{
    if(ts_cached) {
        return;
    }
    if(SOLT.open_m.useMeasurements) {
        ts_open_m = new Touchstone(1);
        *ts_open_m = Touchstone::fromFile(SOLT.open_m.file.toStdString());
        ts_open_m->reduceTo1Port(SOLT.open_m.Sparam);
    }
    if(SOLT.short_m.useMeasurements) {
        ts_short_m = new Touchstone(1);
        *ts_short_m = Touchstone::fromFile(SOLT.short_m.file.toStdString());
        ts_short_m->reduceTo1Port(SOLT.short_m.Sparam);
    }
    if(SOLT.load_m.useMeasurements) {
        ts_load_m = new Touchstone(1);
        *ts_load_m = Touchstone::fromFile(SOLT.load_m.file.toStdString());
        ts_load_m->reduceTo1Port(SOLT.load_m.Sparam);
    }
    if(SOLT.open_f.useMeasurements) {
        ts_open_f = new Touchstone(1);
        *ts_open_f = Touchstone::fromFile(SOLT.open_f.file.toStdString());
        ts_open_f->reduceTo1Port(SOLT.open_f.Sparam);
    }
    if(SOLT.short_f.useMeasurements) {
        ts_short_f = new Touchstone(1);
        *ts_short_f = Touchstone::fromFile(SOLT.short_f.file.toStdString());
        ts_short_f->reduceTo1Port(SOLT.short_f.Sparam);
    }
    if(SOLT.load_f.useMeasurements) {
        ts_load_f = new Touchstone(1);
        *ts_load_f = Touchstone::fromFile(SOLT.load_f.file.toStdString());
        ts_load_f->reduceTo1Port(SOLT.load_f.Sparam);
    }
    if(SOLT.Through.useMeasurements) {
        ts_through = new Touchstone(2);
        *ts_through = Touchstone::fromFile(SOLT.Through.file.toStdString());
        ts_through->reduceTo2Port(SOLT.Through.Sparam1, SOLT.Through.Sparam2);
    }
    ts_cached = true;
}
