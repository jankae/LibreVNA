#include "calkit.h"

#include "calkitdialog.h"
#include "json.hpp"
#include "CustomWidgets/informationbox.h"
#include "appwindow.h"

#include <fstream>
#include <iomanip>
#include <QMessageBox>
#include <QDebug>
#include <math.h>

using json = nlohmann::json;
using namespace std;

Calkit::Calkit()
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

    json j = Savable::createJSON(descr);
    ofstream file;
    file.open(filename.toStdString());
    file << setw(4) << j << endl;
    file.close();
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
//        // older file versions specify Z0 for resistance. Set resistance to Nan to detect missing values later
//        c.SOLT.load_m.resistance = std::numeric_limits<double>::quiet_NaN();
//        c.SOLT.load_f.resistance = std::numeric_limits<double>::quiet_NaN();

//        qDebug() << "JSON format detected";
//        // calkit file uses json format, parse
//        Savable::parseJSON(j, c.descr);
//        auto jSOLT = j["SOLT"];
//        if (!jSOLT.contains("loadModelCFirst")) {
//            // older version which did not allow the user to choose the load model. CFirst seems to be the more
//            // used standard so it is the default for newer calkits. However, old calkits used LFirst so we need
//            // to keep that to not mess with older calkit files
//            c.SOLT.loadModelCFirst = false;
//        }
//        // adjust Z0/resistance in case of older calkit file version with missing resistance entries
//        if(isnan(c.SOLT.load_f.resistance)) {
//            c.SOLT.load_f.resistance = c.SOLT.load_f.Z0;
//            c.SOLT.load_f.Z0 = 50.0;
//        }
//        if(isnan(c.SOLT.load_m.resistance)) {
//            c.SOLT.load_m.resistance = c.SOLT.load_m.Z0;
//            c.SOLT.load_m.Z0 = 50.0;
//        }
    } else {
//        qDebug() << "Legacy format detected";
//        // legacy file format, return to beginning of file
//        file.clear();
//        file.seekg(0);
//        c.SOLT.open_m.useMeasurements = readLine(file).toInt();
//        c.SOLT.short_m.useMeasurements = readLine(file).toInt();
//        c.SOLT.load_m.useMeasurements = readLine(file).toInt();
//        c.SOLT.Through.useMeasurements = readLine(file).toInt();
//        c.SOLT.open_m.Z0 = readLine(file).toDouble();
//        c.SOLT.open_m.delay = readLine(file).toDouble();
//        c.SOLT.open_m.loss = readLine(file).toDouble();
//        c.SOLT.open_m.C0 = readLine(file).toDouble();
//        c.SOLT.open_m.C1 = readLine(file).toDouble();
//        c.SOLT.open_m.C2 = readLine(file).toDouble();
//        c.SOLT.open_m.C3 = readLine(file).toDouble();
//        c.SOLT.short_m.Z0 = readLine(file).toDouble();
//        c.SOLT.short_m.delay = readLine(file).toDouble();
//        c.SOLT.short_m.loss = readLine(file).toDouble();
//        c.SOLT.short_m.L0 = readLine(file).toDouble();
//        c.SOLT.short_m.L1 = readLine(file).toDouble();
//        c.SOLT.short_m.L2 = readLine(file).toDouble();
//        c.SOLT.short_m.L3 = readLine(file).toDouble();
//        c.SOLT.load_m.resistance = readLine(file).toDouble();
//        c.SOLT.Through.Z0 = readLine(file).toDouble();
//        c.SOLT.Through.delay = readLine(file).toDouble();
//        c.SOLT.Through.loss = readLine(file).toDouble();
//        if(c.SOLT.open_m.useMeasurements) {
//            c.SOLT.open_m.file = readLine(file);
//            c.SOLT.open_m.Sparam = readLine(file).toInt();
//        }
//        if(c.SOLT.short_m.useMeasurements) {
//            c.SOLT.short_m.file = readLine(file);
//            c.SOLT.short_m.Sparam = readLine(file).toInt();
//        }
//        if(c.SOLT.load_m.useMeasurements) {
//            c.SOLT.load_m.file = readLine(file);
//            c.SOLT.load_m.Sparam = readLine(file).toInt();
//        }
//        if(c.SOLT.Through.useMeasurements) {
//            c.SOLT.Through.file = readLine(file);
//            c.SOLT.Through.Sparam1 = readLine(file).toInt();
//            c.SOLT.Through.Sparam2 = readLine(file).toInt();
//        }
//        c.TRL.Through.Z0 = readLine(file).toDouble();
//        c.TRL.Reflection.isShort = readLine(file).toDouble();
//        c.TRL.Line.delay = readLine(file).toDouble();
//        c.TRL.Line.minFreq = readLine(file).toDouble();
//        c.TRL.Line.maxFreq = readLine(file).toDouble();

//        c.SOLT.separate_male_female = false;

//        InformationBox::ShowMessage("Loading calkit file", "The file \"" + filename + "\" is stored in a deprecated"
//                     " calibration kit format. Future versions of this application might not support"
//                     " it anymore. Please save the calibration kit to update to the new format");
    }
    file.close();

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
    if(AppWindow::showGUI()) {
        dialog->show();
    }
}

bool Calkit::hasSeparateMaleFemaleStandards()
{
    return true; // TODO delete function
}

class Calkit::SOLT Calkit::toSOLT(double frequency, bool male_standards)
{
    return SOLT(); // TODO delete function
}

class Calkit::TRL Calkit::toTRL(double)
{
    return TRL(); // TODO delete function
}

double Calkit::minFreqTRL()
{
    return 0; // TODO delete function
}

double Calkit::maxFreqTRL()
{
    return std::numeric_limits<double>::max(); // TODO delete function
}

double Calkit::minFreqSOLT(bool male_standards)
{
    return 0; // TODO delete function
}

double Calkit::maxFreqSOLT(bool male_standards)
{
    return std::numeric_limits<double>::max(); // TODO delete function
}

bool Calkit::checkIfValid(double min_freq, double max_freq, bool isTRL, bool include_male, bool include_female)
{
    return true; // TODO delete function
}

bool Calkit::isTRLReflectionShort() const
{
    return true; // TODO delete function
}
