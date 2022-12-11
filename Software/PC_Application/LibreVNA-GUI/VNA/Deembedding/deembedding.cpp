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

    delete measurementDialog;
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
    connect(measurementDialog, &QDialog::finished, [=](){
        delete ui;
    });

    // add the trace selector
    auto traceChooser = new SparamTraceSelector(tm, option->getAffectedPorts());
    ui->horizontalLayout_2->insertWidget(0, traceChooser, 1);

    connect(traceChooser, &SparamTraceSelector::selectionValid, ui->buttonBox, &QDialogButtonBox::setEnabled);

    connect(ui->bMeasure, &QPushButton::clicked, [=](){
        ui->bMeasure->setEnabled(false);
        traceChooser->setEnabled(false);
        ui->buttonBox->setEnabled(false);
        measuring = true;
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){
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

Deembedding::Deembedding(TraceModel &tm)
    : measuringOption(nullptr),
      tm(tm),
      measuring(false),
      measurementDialog(nullptr),
      measurementUI(nullptr),
      sweepPoints(0)
{

}

void Deembedding::Deembed(VirtualDevice::VNAMeasurement &d)
{
    // figure out the point in one sweep based on the incomig pointNums
    static unsigned lastPointNum;
    if (d.pointNum == 0) {
        sweepPoints = lastPointNum;
    } else if(d.pointNum > sweepPoints) {
        sweepPoints = d.pointNum;
    }
    lastPointNum = d.pointNum;

    for(auto it = options.begin();it != options.end();it++) {
        if (measuring && measuringOption == *it) {
            // this option needs a measurement
            if (d.pointNum == 0) {
                if(measurements.size() == 0) {
                    // this is the first point of the measurement
                    measurements.push_back(d);
                } else {
                    // this is the first point of the next sweep, measurement complete
                    measuring = false;
                    measurementCompleted();
                }
            } else if(measurements.size() > 0) {
                // in the middle of the measurement, add point
                measurements.push_back(d);
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
    emit optionAdded();
}

void Deembedding::swapOptions(unsigned int index)
{
    if(index + 1 >= options.size()) {
        return;
    }
    std::swap(options[index], options[index+1]);
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

nlohmann::json Deembedding::toJSON()
{
    nlohmann::json list;
    for(auto m : options) {
        nlohmann::json jm;
        jm["operation"] = DeembeddingOption::getName(m->getType()).toStdString();
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
            if(DeembeddingOption::getName((DeembeddingOption::Type) i) == operation) {
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
