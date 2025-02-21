#include "deembedding.h"

#include "deembeddingdialog.h"
#include "ui_measurementdialog.h"
#include "Traces/sparamtraceselector.h"
#include "appwindow.h"

#include <QDebug>

using namespace std;

void Deembedding::configure()
{
    auto d = new DeembeddingDialog(this);
    if(AppWindow::showGUI()) {
        d->show();
    }
}

void Deembedding::measurementCompleted()
{
    // pass on the measurement result to the option that triggered the measurement
    if (measuringOption) {
        measuringOption->measurementCompleted(measurements);
        measuringOption = nullptr;
    }

    measurementDialog->close();
    measurementDialog->deleteLater();
    measurementDialog = nullptr;
    measurementUI = nullptr;
}

void Deembedding::startMeasurementDialog(DeembeddingOption *option)
{
    measurements.clear();
    measurementDialog = new QDialog;
    auto ui = new Ui_DeembeddingMeasurementDialog;
    measurementUI = ui;
    ui->setupUi(measurementDialog);
    connect(measurementDialog, &QDialog::finished, this, [=](){
        if(measuring) {
            measuring = false;
            emit finishedMeasurement();
        }
        measuringOption = nullptr;
        measurementUI = nullptr;
        delete ui;
    });

    // add the trace selector
    auto traceChooser = new SparamTraceSelector(tm, option->getAffectedPorts());
    ui->horizontalLayout_2->insertWidget(0, traceChooser, 1);

    connect(traceChooser, &SparamTraceSelector::selectionValid, ui->buttonBox, &QDialogButtonBox::setEnabled);

    connect(ui->bMeasure, &QPushButton::clicked, this, [=](){
        ui->bMeasure->setEnabled(false);
        traceChooser->setEnabled(false);
        ui->buttonBox->setEnabled(false);
        measuring = true;
        emit triggerMeasurement();
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [=](){
        // create datapoints from individual traces
        measurements.clear();
        auto points = Trace::assembleDatapoints(traceChooser->getTraces());
        for(auto p : points) {
            measurements.push_back(p);
        }
        measurementCompleted();
    });

    if(AppWindow::showGUI()) {
        measurementDialog->show();
    }
}

void Deembedding::updateSCPINames()
{
    // Need to remove all options from the subnode list first, otherwise
    // name changes wouldn't work due to temporarily name collisions
    for(auto &option : options) {
        remove(option);
    }
    unsigned int i=1;
    for(auto &option : options) {
        option->changeName(QString::number(i));
        add(option);
        i++;
    }
}

Deembedding::Deembedding(TraceModel &tm)
    : SCPINode("DEEMBedding"),
      measuringOption(nullptr),
      tm(tm),
      measuring(false),
      measurementDialog(nullptr),
      measurementUI(nullptr),
      sweepPoints(0)
{
    add(new SCPICommand("NUMber", nullptr, [=](QStringList params) -> QString {
        Q_UNUSED(params);
        return QString::number(options.size());
    }));
    add(new SCPICommand("TYPE", nullptr, [=](QStringList params) -> QString {
        unsigned long long index;
        if(!SCPI::paramToULongLong(params, 0, index)) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        if(index < 1 || index > options.size()) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        return DeembeddingOption::TypeToString(options[index-1]->getType()).replace(" ", "_");
    }));
    add(new SCPICommand("NEW", [=](QStringList params) -> QString {
        if(params.size() < 1) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        auto type = DeembeddingOption::TypeFromString(params[0].replace("_", " "));
        if(type == DeembeddingOption::Type::Last) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        auto option = DeembeddingOption::create(type);
        addOption(option);
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    add(new SCPICommand("CLEAR", [=](QStringList params) -> QString {
        Q_UNUSED(params);
        clear();
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
}

void Deembedding::Deembed(DeviceDriver::VNAMeasurement &d)
{
    for(auto it = options.begin();it != options.end();it++) {
        if (measuring && measuringOption == *it) {
            // this option needs a measurement
            if (d.pointNum == 0) {
                if(measurements.size() == 0) {
                    // this is the first point of the measurement
                    measurements.push_back(d);
                }
            } else if(measurements.size() > 0) {
                // in the middle of the measurement, add point
                measurements.push_back(d);

                if(d.pointNum == sweepPoints - 1) {
                    // this is the last point, measurement complete
                    measuring = false;
                    emit finishedMeasurement();
                    measurementCompleted();
                }
            }

            if(measurementUI) {
                measurementUI->progress->setValue(100 * measurements.size() / sweepPoints);
            }
        }
        (*it)->transformDatapoint(d);
    }
}

void Deembedding::Deembed(std::map<QString, Trace *> traceSet)
{
    auto points = Trace::assembleDatapoints(traceSet);
    if(points.size()) {
        // succeeded in assembling datapoints
        for(auto &p : points) {
            Deembed(p);
        }
        Trace::fillFromDatapoints(traceSet, points, true);
        for(auto t : traceSet) {
            t.second->setDeembeddingActive(true);
        }
    }
}

void Deembedding::removeOption(unsigned int index)
{
    if(index < options.size()) {
        delete options[index];
        options.erase(options.begin() + index);
    }
    updateSCPINames();
    if(options.size() == 0) {
        emit allOptionsCleared();
    }
}

void Deembedding::addOption(DeembeddingOption *option)
{
    options.push_back(option);
    connect(option, &DeembeddingOption::deleted, [=](DeembeddingOption *o){
        // find deleted option and remove from list
        auto pos = find(options.begin(), options.end(), o);
        if(pos != options.end()) {
            options.erase(pos);
        }
    });
    connect(option, &DeembeddingOption::triggerMeasurement, [=]() {
        measuringOption = option;
        startMeasurementDialog(option);
    });
    updateSCPINames();
    emit optionAdded();
}

void Deembedding::swapOptions(unsigned int index)
{
    if(index + 1 >= options.size()) {
        return;
    }
    std::swap(options[index], options[index+1]);
    updateSCPINames();
}

void Deembedding::clear()
{
    while(options.size() > 0) {
        removeOption(0);
    }
}

bool Deembedding::isMeasuring()
{
    return measuring;
}

std::set<unsigned int> Deembedding::getAffectedPorts()
{
    set<unsigned int> ret;
    for(auto o : options) {
        auto affected = o->getAffectedPorts();
        ret.insert(affected.begin(), affected.end());
    }
    return ret;
}

void Deembedding::setPointsInSweepForMeasurement(unsigned int points)
{
    sweepPoints = points;
}

nlohmann::json Deembedding::toJSON()
{
    nlohmann::json list;
    for(auto m : options) {
        nlohmann::json jm;
        jm["operation"] = DeembeddingOption::TypeToString(m->getType()).toStdString();
        jm["settings"] = m->toJSON();
        list.push_back(jm);
    }
    return list;
}

void Deembedding::fromJSON(nlohmann::json j)
{
    // clear all options
    while(options.size() > 0) {
        removeOption(0);
    }
    for(auto jm : j) {
        QString operation = QString::fromStdString(jm.value("operation", ""));
        if(operation.isEmpty()) {
            qWarning() << "Skipping empty de-embedding operation";
            continue;
        }
        // attempt to find the type of operation
        DeembeddingOption::Type type = DeembeddingOption::Type::Last;
        for(unsigned int i=0;i<(int) DeembeddingOption::Type::Last;i++) {
            if(DeembeddingOption::TypeToString((DeembeddingOption::Type) i) == operation) {
                // found the correct operation
                type = (DeembeddingOption::Type) i;
                break;
            }
        }
        if(type == DeembeddingOption::Type::Last) {
            // unable to find this operation
            qWarning() << "Unable to create de-embedding operation:" << operation;
            continue;
        }
        qDebug() << "Creating math operation of type:" << operation;
        auto op = DeembeddingOption::create(type);
        if(jm.contains("settings")) {
            op->fromJSON(jm["settings"]);
        }
        addOption(op);
    }
}
