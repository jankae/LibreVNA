#include "deembedding.h"
#include "deembeddingdialog.h"
#include <QDebug>
#include "ui_measurementdialog.h"
#include "Traces/sparamtraceselector.h"

using namespace std;

void Deembedding::configure()
{
    auto d = new DeembeddingDialog(this);
    d->show();
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

void Deembedding::startMeasurementDialog(bool S11, bool S12, bool S21, bool S22)
{
    measurements.clear();
    measurementDialog = new QDialog;
    auto ui = new Ui_DeembeddingMeasurementDialog;
    measurementUI = ui;
    ui->setupUi(measurementDialog);

    // add the trace selector
    set<unsigned int> skip;
    if(!S11) {
        skip.insert(0);
    }
    if(!S12) {
        skip.insert(1);
    }
    if(!S21) {
        skip.insert(2);
    }
    if(!S22) {
        skip.insert(3);
    }
    auto traceChooser = new SparamTraceSelector(tm, 2, false, skip);
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
        auto t = traceChooser->getTraces();
        auto S11 = t[0];
        auto S12 = t[1];
        auto S21 = t[2];
        auto S22 = t[3];
        for(unsigned int i=0;i<traceChooser->getPoints();i++) {
            Protocol::Datapoint p;
            p.pointNum = i;
            if(S11) {
                auto sample = S11->sample(i);
                p.imag_S11 = sample.y.imag();
                p.real_S11 = sample.y.real();
                p.frequency = sample.x;
            }
            if(S12) {
                auto sample = S12->sample(i);
                p.imag_S12 = sample.y.imag();
                p.real_S12 = sample.y.real();
                p.frequency = sample.x;
            }
            if(S21) {
                auto sample = S21->sample(i);
                p.imag_S21 = sample.y.imag();
                p.real_S21 = sample.y.real();
                p.frequency = sample.x;
            }
            if(S22) {
                auto sample = S22->sample(i);
                p.imag_S22 = sample.y.imag();
                p.real_S22 = sample.y.real();
                p.frequency = sample.x;
            }
            measurements.push_back(p);
        }
        measurementCompleted();
    });

    measurementDialog->show();
}

Deembedding::Deembedding(TraceModel &tm)
    : tm(tm),
      measuring(false)
{

}

void Deembedding::Deembed(Protocol::Datapoint &d)
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

void Deembedding::Deembed(Trace &S11, Trace &S12, Trace &S21, Trace &S22)
{
    auto points = Trace::assembleDatapoints(S11, S12, S21, S22);
    if(points.size()) {
        // succeeded in assembling datapoints
        for(auto &p : points) {
            Deembed(p);
        }
        Trace::fillFromDatapoints(S11, S12, S21, S22, points);
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
    connect(option, &DeembeddingOption::triggerMeasurement, [=](bool S11, bool S12, bool S21, bool S22) {
        measuringOption = option;
        startMeasurementDialog(S11, S12, S21, S22);
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
