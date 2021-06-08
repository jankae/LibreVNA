#include "calkit.h"

#include <fstream>
#include <iomanip>
#include "calkitdialog.h"
#include <math.h>
#include "json.hpp"
#include <QMessageBox>
#include <QDebug>

using json = nlohmann::json;
using namespace std;

Calkit::Calkit()
 : ts_open(nullptr),
   ts_short(nullptr),
   ts_load(nullptr),
   ts_through(nullptr),
   ts_cached(false)
{

    // set default values
    for(auto e : json_descr) {
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

    json j;
    for(auto e : json_descr) {
        auto list = e.name.split("/");
        auto *json_entry = &j;
        while(list.size() > 0) {
            json_entry = &(*json_entry)[list.takeFirst().toStdString()];
        }
        // json library does not now about QVariant, handle used cases
        auto val = e.var.value();
        switch(static_cast<QMetaType::Type>(val.type())) {
        case QMetaType::Double: *json_entry = val.toDouble(); break;
        case QMetaType::Int: *json_entry = val.toInt(); break;
        case QMetaType::Bool: *json_entry = val.toBool(); break;
        case QMetaType::QString: *json_entry = val.toString().toStdString(); break;
        default:
            throw runtime_error("Unimplemented metatype");
        }
    }
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
    file >> j;
    if(j.contains("SOLT")) {
        qDebug() << "JSON format detected";
        // calkit file uses json format, parse
        for(auto e : c.json_descr) {
            auto list = e.name.split("/");
            auto *json_entry = &j;
            bool entry_exists = true;
            while(list.size() > 0) {
                auto key = list.takeFirst().toStdString();
                if((*json_entry).contains(key)) {
                    json_entry = &(*json_entry)[key];
                } else {
                    entry_exists = false;
                    break;
                }
            }
            if(!entry_exists) {
                // missing entry in json file, nothing to do (default values already set in constructor)
                qWarning() << "Entry" << e.name << "not present in file, assuming default value";
                continue;
            }
            // json library does not now about QVariant, handle used cases
            auto val = e.var.value();
            switch(static_cast<QMetaType::Type>(val.type())) {
            case QMetaType::Double: e.var.setValue((*json_entry).get<double>()); break;
            case QMetaType::Int: e.var.setValue((*json_entry).get<int>()); break;
            case QMetaType::Bool: e.var.setValue((*json_entry).get<bool>()); break;
            case QMetaType::QString: {
                auto s = QString::fromStdString((*json_entry).get<string>());
                e.var.setValue(s);
            }

                break;
            default:
                throw runtime_error("Unimplemented metatype");
            }
        }
    } else {
        qDebug() << "Legacy format detected";
        // legacy file format, return to beginning of file
        file.clear();
        file.seekg(0);
        c.SOLT.Open.useMeasurements = readLine(file).toInt();
        c.SOLT.Short.useMeasurements = readLine(file).toInt();
        c.SOLT.Load.useMeasurements = readLine(file).toInt();
        c.SOLT.Through.useMeasurements = readLine(file).toInt();
        c.SOLT.Open.Z0 = readLine(file).toDouble();
        c.SOLT.Open.delay = readLine(file).toDouble();
        c.SOLT.Open.loss = readLine(file).toDouble();
        c.SOLT.Open.C0 = readLine(file).toDouble();
        c.SOLT.Open.C1 = readLine(file).toDouble();
        c.SOLT.Open.C2 = readLine(file).toDouble();
        c.SOLT.Open.C3 = readLine(file).toDouble();
        c.SOLT.Short.Z0 = readLine(file).toDouble();
        c.SOLT.Short.delay = readLine(file).toDouble();
        c.SOLT.Short.loss = readLine(file).toDouble();
        c.SOLT.Short.L0 = readLine(file).toDouble();
        c.SOLT.Short.L1 = readLine(file).toDouble();
        c.SOLT.Short.L2 = readLine(file).toDouble();
        c.SOLT.Short.L3 = readLine(file).toDouble();
        c.SOLT.Load.Z0 = readLine(file).toDouble();
        c.SOLT.Through.Z0 = readLine(file).toDouble();
        c.SOLT.Through.delay = readLine(file).toDouble();
        c.SOLT.Through.loss = readLine(file).toDouble();
        if(c.SOLT.Open.useMeasurements) {
            c.SOLT.Open.file = readLine(file);
            c.SOLT.Open.Sparam = readLine(file).toInt();
        }
        if(c.SOLT.Short.useMeasurements) {
            c.SOLT.Short.file = readLine(file);
            c.SOLT.Short.Sparam = readLine(file).toInt();
        }
        if(c.SOLT.Load.useMeasurements) {
            c.SOLT.Load.file = readLine(file);
            c.SOLT.Load.Sparam = readLine(file).toInt();
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

        auto msg = new QMessageBox();
        msg->setWindowTitle("Loading calkit file");
        msg->setText("The file \"" + filename + "\" is stored in a deprecated"
                     " calibration kit format. Future versions of this application might not support"
                     " it anymore. Please save the calibration kit to update to the new format");
        msg->setStandardButtons(QMessageBox::Ok);
        msg->show();
    }
    file.close();

    c.TransformPathsToAbsolute(filename);

    // set default values for non-editable items (for now)
    c.TRL.Through.Z0 = 50.0;
    c.SOLT.Short.Z0 = 50.0;
    c.SOLT.Open.Z0 = 50.0;
    c.SOLT.Through.Z0 = 50.0;

    return c;
}

void Calkit::edit(std::function<void (void)> done)
{
    auto dialog = new CalkitDialog(*this);
    if(done) {
        QObject::connect(dialog, &QDialog::accepted, [=](){
            done();
        });
    }
    dialog->show();
}

class Calkit::SOLT Calkit::toSOLT(double frequency)
{
    fillTouchstoneCache();
    class SOLT ref;
    if(SOLT.Load.useMeasurements) {
        ref.Load = ts_load->interpolate(frequency).S[0];
    } else {
        auto imp_load = complex<double>(SOLT.Load.Z0, 0);
        // Add parallel capacitor to impedance
        if(SOLT.Load.Cparallel > 0) {
            auto imp_C = complex<double>(0, -1.0 / (frequency * 2 * M_PI * SOLT.Load.Cparallel));
            imp_load = (imp_load * imp_C) / (imp_load + imp_C);
        }
        // add series inductor to impedance
        auto imp_L = complex<double>(0, frequency * 2 * M_PI * SOLT.Load.Lseries);
        imp_load += imp_L;
        ref.Load = (imp_load - complex<double>(50.0)) / (imp_load + complex<double>(50.0));
        // apply phaseshift due to delay
        double load_phaseshift = -2 * M_PI * frequency * 2 * SOLT.Load.delay * 1e-12;
        ref.Load *= polar<double>(1.0, load_phaseshift);
    }

    if(SOLT.Open.useMeasurements) {
        ref.Open = ts_open->interpolate(frequency).S[0];
    } else {
        // calculate fringing capacitance for open
        double Cfringing = SOLT.Open.C0 * 1e-15 + SOLT.Open.C1 * 1e-27 * frequency + SOLT.Open.C2 * 1e-36 * pow(frequency, 2) + SOLT.Open.C3 * 1e-45 * pow(frequency, 3);
        // convert to impedance
        if (Cfringing == 0) {
            // special case to avoid issues with infinity
            ref.Open = complex<double>(1.0, 0);
        } else {
            auto imp_open = complex<double>(0, -1.0 / (frequency * 2 * M_PI * Cfringing));
            ref.Open = (imp_open - complex<double>(50.0)) / (imp_open + complex<double>(50.0));
        }
        // transform the delay into a phase shift for the given frequency
        double open_phaseshift = -2 * M_PI * frequency * 2 * SOLT.Open.delay * 1e-12;
        double open_att_db = SOLT.Open.loss * 1e9 * 4.3429 * 2 * SOLT.Open.delay * 1e-12 / SOLT.Open.Z0 * sqrt(frequency / 1e9);
        double open_att = pow(10.0, -open_att_db / 10.0);
        auto open_correction = polar<double>(open_att, open_phaseshift);
        ref.Open *= open_correction;
    }

    if(SOLT.Short.useMeasurements) {
        ref.Short = ts_short->interpolate(frequency).S[0];
    } else {
        // calculate inductance for short
        double Lseries = SOLT.Short.L0 * 1e-12 + SOLT.Short.L1 * 1e-24 * frequency + SOLT.Short.L2 * 1e-33 * pow(frequency, 2) + SOLT.Short.L3 * 1e-42 * pow(frequency, 3);
        // convert to impedance
        auto imp_short = complex<double>(0, frequency * 2 * M_PI * Lseries);
        ref.Short =  (imp_short - complex<double>(50.0)) / (imp_short + complex<double>(50.0));
        // transform the delay into a phase shift for the given frequency
        double short_phaseshift = -2 * M_PI * frequency * 2 * SOLT.Short.delay * 1e-12;
        double short_att_db = SOLT.Short.loss * 1e9 * 4.3429 * 2 * SOLT.Short.delay * 1e-12 / SOLT.Short.Z0 * sqrt(frequency / 1e9);;
        double short_att = pow(10.0, -short_att_db / 10.0);
        auto short_correction = polar<double>(short_att, short_phaseshift);
        ref.Short *= short_correction;
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

double Calkit::minFreq(bool trl)
{
    if(trl) {
        return TRL.Line.minFreq;
    } else {
        fillTouchstoneCache();
        double min = 0;
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
}

double Calkit::maxFreq(bool trl)
{
    if(trl) {
        return TRL.Line.maxFreq;
    } else {
        fillTouchstoneCache();
        double max = std::numeric_limits<double>::max();
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
}

bool Calkit::isTRLReflectionShort() const
{
    return TRL.Reflection.isShort;
}

void Calkit::TransformPathsToRelative(QFileInfo d)
{
    vector<QString*> filenames = {&SOLT.Short.file, &SOLT.Open.file, &SOLT.Load.file, &SOLT.Through.file};
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
    vector<QString*> filenames = {&SOLT.Short.file, &SOLT.Open.file, &SOLT.Load.file, &SOLT.Through.file};
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
    delete ts_open;
    ts_open = nullptr;
    delete ts_short;
    ts_short = nullptr;
    delete ts_load;
    ts_load = nullptr;
    delete ts_through;
    ts_through = nullptr;
    ts_cached = false;
}

void Calkit::fillTouchstoneCache()
{
    if(ts_cached) {
        return;
    }
    if(SOLT.Open.useMeasurements) {
        ts_open = new Touchstone(1);
        *ts_open = Touchstone::fromFile(SOLT.Open.file.toStdString());
        ts_open->reduceTo1Port(SOLT.Open.Sparam);
    }
    if(SOLT.Short.useMeasurements) {
        ts_short = new Touchstone(1);
        *ts_short = Touchstone::fromFile(SOLT.Short.file.toStdString());
        ts_short->reduceTo1Port(SOLT.Short.Sparam);
    }
    if(SOLT.Load.useMeasurements) {
        ts_load = new Touchstone(1);
        *ts_load = Touchstone::fromFile(SOLT.Load.file.toStdString());
        ts_load->reduceTo1Port(SOLT.Load.Sparam);
    }
    if(SOLT.Through.useMeasurements) {
        ts_through = new Touchstone(2);
        *ts_through = Touchstone::fromFile(SOLT.Through.file.toStdString());
        ts_through->reduceTo2Port(SOLT.Through.Sparam1, SOLT.Through.Sparam2);
    }
    ts_cached = true;
}
