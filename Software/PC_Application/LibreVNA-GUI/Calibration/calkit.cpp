#include "calkit.h"

#include "calkitdialog.h"
#include "json.hpp"
#include "CustomWidgets/informationbox.h"
#include "appwindow.h"
#include "Util/app_common.h"

#include <fstream>
#include <iomanip>
#include <QMessageBox>
#include <QDebug>
#include <math.h>

using json = nlohmann::json;
using namespace std;

Calkit::Calkit()
    : SCPINode("KIT")
{
    // set default values
    for(auto e : descr) {
        e.var.setValue(e.def);
    }

    add(new SCPICommand("SAVE", [=](QStringList params) -> QString {
        if(params.size() != 1 ) {
            // no filename given or no calibration active
            return SCPI::getResultName(SCPI::Result::Error);
        }
        toFile(params[0]);
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    add(new SCPICommand("LOAD", nullptr, [=](QStringList params) -> QString {
        if(params.size() != 1) {
            // no filename given or no calibration active
            return SCPI::getResultName(SCPI::Result::False);
        }
        try {
            *this = fromFile(params[0]);
            return SCPI::getResultName(SCPI::Result::True);
        } catch (runtime_error &e) {
            return SCPI::getResultName(SCPI::Result::False);
        }
    }));
}

void Calkit::toFile(QString filename)
{
    if(!filename.endsWith(".calkit")) {
        filename.append(".calkit");
    }

    qDebug() << "Saving calkit to file" << filename;

    ofstream file;
    file.open(filename.toStdString());
    file << setw(4) << toJSON() << endl;
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
    c.clearStandards();
    if(j.contains("standards")) {
        qDebug() << "new JSON format detected";
        c.fromJSON(j);
    } else {
        // older format is used
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
        if(j.contains("SOLT")) {
            qDebug() << "old JSON format detected";
            // calkit file uses json format, parse
            Savable::parseJSON(j, c.descr);
            const std::vector<Savable::SettingDescription> descr_deprecated = {{
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
            }};
            Savable::parseJSON(j, descr_deprecated);
        } else {
            qDebug() << "Legacy format detected";
            // legacy file format, return to beginning of file
            file.clear();
            file.seekg(0);
            SOLT.open_m.useMeasurements = readLine(file).toInt();
            SOLT.short_m.useMeasurements = readLine(file).toInt();
            SOLT.load_m.useMeasurements = readLine(file).toInt();
            SOLT.Through.useMeasurements = readLine(file).toInt();
            SOLT.open_m.Z0 = readLine(file).toDouble();
            SOLT.open_m.delay = readLine(file).toDouble();
            SOLT.open_m.loss = readLine(file).toDouble();
            SOLT.open_m.C0 = readLine(file).toDouble();
            SOLT.open_m.C1 = readLine(file).toDouble();
            SOLT.open_m.C2 = readLine(file).toDouble();
            SOLT.open_m.C3 = readLine(file).toDouble();
            SOLT.short_m.Z0 = readLine(file).toDouble();
            SOLT.short_m.delay = readLine(file).toDouble();
            SOLT.short_m.loss = readLine(file).toDouble();
            SOLT.short_m.L0 = readLine(file).toDouble();
            SOLT.short_m.L1 = readLine(file).toDouble();
            SOLT.short_m.L2 = readLine(file).toDouble();
            SOLT.short_m.L3 = readLine(file).toDouble();
            SOLT.load_m.resistance = readLine(file).toDouble();
            SOLT.Through.Z0 = readLine(file).toDouble();
            SOLT.Through.delay = readLine(file).toDouble();
            SOLT.Through.loss = readLine(file).toDouble();
            if(SOLT.open_m.useMeasurements) {
                SOLT.open_m.file = readLine(file);
                SOLT.open_m.Sparam = readLine(file).toInt();
            }
            if(SOLT.short_m.useMeasurements) {
                SOLT.short_m.file = readLine(file);
                SOLT.short_m.Sparam = readLine(file).toInt();
            }
            if(SOLT.load_m.useMeasurements) {
                SOLT.load_m.file = readLine(file);
                SOLT.load_m.Sparam = readLine(file).toInt();
            }
            if(SOLT.Through.useMeasurements) {
                SOLT.Through.file = readLine(file);
                SOLT.Through.Sparam1 = readLine(file).toInt();
                SOLT.Through.Sparam2 = readLine(file).toInt();
            }
            TRL.Through.Z0 = readLine(file).toDouble();
            TRL.Reflection.isShort = readLine(file).toDouble();
            TRL.Line.delay = readLine(file).toDouble();
            TRL.Line.minFreq = readLine(file).toDouble();
            TRL.Line.maxFreq = readLine(file).toDouble();

            SOLT.separate_male_female = false;
            SOLT.loadModelCFirst = false;
        }

        vector<QString*> filenames = {&SOLT.short_m.file, &SOLT.open_m.file, &SOLT.load_m.file, &SOLT.short_f.file, &SOLT.open_f.file, &SOLT.load_f.file, &SOLT.Through.file};
        QFileInfo d(filename);
        for(auto f : filenames) {
            if(f->isEmpty()) {
                continue;
            }
            if(QFileInfo(*f).isRelative()) {
                auto absDir = QDir(d.dir().path() + "/" + *f);
                QString buf = *f;
                *f = absDir.absolutePath();
            }
        }

        // create standards from old calkit forma
        auto open_m = new CalStandard::Open(SOLT.separate_male_female ? "Default male standard" : "Default standard", SOLT.open_m.Z0, SOLT.open_m.delay, SOLT.open_m.loss, SOLT.open_m.C0, SOLT.open_m.C1, SOLT.open_m.C2, SOLT.open_m.C3);
        if(SOLT.open_m.useMeasurements) {
            auto ts = Touchstone(1);
            ts.fromFile(SOLT.open_m.file.toStdString());
            open_m->setMeasurement(ts, SOLT.open_m.Sparam);
        }
        c.addStandard(open_m);
        if(SOLT.separate_male_female) {
            auto open_f = new CalStandard::Open("Default female standard", SOLT.open_f.Z0, SOLT.open_f.delay, SOLT.open_f.loss, SOLT.open_f.C0, SOLT.open_f.C1, SOLT.open_f.C2, SOLT.open_f.C3);
            if(SOLT.open_f.useMeasurements) {
                auto ts = Touchstone(1);
                ts.fromFile(SOLT.open_f.file.toStdString());
                open_m->setMeasurement(ts, SOLT.open_f.Sparam);
            }
            c.addStandard(open_f);
        }

        auto short_m = new CalStandard::Short(SOLT.separate_male_female ? "Default male standard" : "Default standard", SOLT.short_m.Z0, SOLT.short_m.delay, SOLT.short_m.loss, SOLT.short_m.L0, SOLT.short_m.L1, SOLT.short_m.L2, SOLT.short_m.L3);
        if(SOLT.short_m.useMeasurements) {
            auto ts = Touchstone(1);
            ts.fromFile(SOLT.short_m.file.toStdString());
            short_m->setMeasurement(ts, SOLT.short_m.Sparam);
        }
        c.addStandard(short_m);
        if(SOLT.separate_male_female) {
            auto short_f = new CalStandard::Short("Default female standard", SOLT.short_f.Z0, SOLT.short_f.delay, SOLT.short_f.loss, SOLT.short_f.L0, SOLT.short_f.L1, SOLT.short_f.L2, SOLT.short_f.L3);
            if(SOLT.short_f.useMeasurements) {
                auto ts = Touchstone(1);
                ts.fromFile(SOLT.short_f.file.toStdString());
                short_m->setMeasurement(ts, SOLT.short_f.Sparam);
            }
            c.addStandard(short_f);
        }

        auto load_m = new CalStandard::Load(SOLT.separate_male_female ? "Default male standard" : "Default standard", SOLT.load_m.Z0, SOLT.load_m.delay, 0.0, SOLT.load_m.resistance, SOLT.load_m.Cparallel, SOLT.load_m.Lseries, SOLT.loadModelCFirst);
        if(SOLT.load_m.useMeasurements) {
            auto ts = Touchstone(1);
            ts.fromFile(SOLT.load_m.file.toStdString());
            load_m->setMeasurement(ts, SOLT.load_m.Sparam);
        }
        c.addStandard(load_m);
        if(SOLT.separate_male_female) {
            auto load_f = new CalStandard::Load("Default female standard", SOLT.load_m.Z0, SOLT.load_f.delay, 0.0, SOLT.load_f.resistance, SOLT.load_f.Cparallel, SOLT.load_f.Lseries, SOLT.loadModelCFirst);
            if(SOLT.load_f.useMeasurements) {
                auto ts = Touchstone(1);
                ts.fromFile(SOLT.load_f.file.toStdString());
                load_m->setMeasurement(ts, SOLT.load_f.Sparam);
            }
            c.addStandard(load_f);
        }

        auto through = new CalStandard::Through("Default standard", SOLT.Through.Z0, SOLT.Through.delay, SOLT.Through.loss);
        if(SOLT.Through.useMeasurements) {
            auto ts = Touchstone(2);
            ts.fromFile(SOLT.Through.file.toStdString());
            through->setMeasurement(ts, SOLT.Through.Sparam1, SOLT.Through.Sparam2);
        }
        c.addStandard(through);

        InformationBox::ShowMessage("Loading calkit file", "The file \"" + filename + "\" is stored in a deprecated"
                     " calibration kit format. Future versions of this application might not support"
                     " it anymore. Please save the calibration kit to update to the new format");
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

void Calkit::clearStandards()
{
    for(auto s : standards) {
        delete s;
    }
    standards.clear();
}

std::vector<CalStandard::Virtual *> Calkit::getStandards() const
{
    return standards;
}

void Calkit::addStandard(CalStandard::Virtual *s)
{
    // check for ID collisions. This should never happen but would mess up further
    // usage of the standards. Better to catch this now and throw an error
    for(auto comp : standards) {
        if(comp->getID() == s->getID()) {
            // collision, do not add
            InformationBox::ShowError("Calibration standard ID collision", "New standard \""
                                      +s->getName()+"\" of type \""+CalStandard::Virtual::TypeToString(s->getType())
                                      +"\" has the same ID as already existing standard \""
                                      +comp->getName()+"\" of type \""+CalStandard::Virtual::TypeToString(comp->getType())
                                      +"\" (ID="+QString::number(s->getID())+"). It will be removed from the calibration kit");
            return;
        }
    }
    standards.push_back(s);
}

nlohmann::json Calkit::toJSON()
{
    json j = Savable::createJSON(descr);
    nlohmann::json jstandards;
    for(auto s : standards) {
        nlohmann::json jstandard;
        jstandard["type"] = CalStandard::Virtual::TypeToString(s->getType()).toStdString();
        jstandard["params"] = s->toJSON();
        jstandards.push_back(jstandard);
    }
    j["standards"] = jstandards;
    j["version"] = qlibrevnaApp->applicationVersion().toStdString();
    return j;
}

void Calkit::fromJSON(nlohmann::json j)
{
    clearStandards();
    Savable::parseJSON(j, descr);
    for(auto js : j["standards"]) {
        if(!js.contains("type") || !js.contains("params")) {
            // missing fields
            continue;
        }
        auto type = CalStandard::Virtual::TypeFromString(QString::fromStdString(js.value("type", "")));
        if(type == CalStandard::Virtual::Type::Last) {
            // failed to parse type
            continue;
        }
        auto s = CalStandard::Virtual::create(type);
        s->fromJSON(js["params"]);
        addStandard(s);
    }
}

void Calkit::setIdealDefault()
{
    manufacturer = "LibreVNA";
    description = "Default calibration kit with ideal standards";
    clearStandards();
    addStandard(new CalStandard::Open("Ideal Open Standard", 50.0, 0, 0, 0, 0, 0, 0));
    addStandard(new CalStandard::Short("Ideal Short Standard", 50.0, 0, 0, 0, 0, 0, 0));
    addStandard(new CalStandard::Load("Ideal Load Standard", 50.0, 0, 0, 50.0, 0, 0));
    addStandard(new CalStandard::Through("Ideal Through Standard", 50.0, 0, 0));
}
