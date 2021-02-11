#include "deembedding.h"
#include "deembeddingdialog.h"
#include <QDebug>
#include "ui_measurementdialog.h"

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

void Deembedding::setInitialTraceSelections()
{
    // all checkboxes initially set to none
    measurementUI->cS11->blockSignals(true);
    measurementUI->cS12->blockSignals(true);
    measurementUI->cS21->blockSignals(true);
    measurementUI->cS22->blockSignals(true);
    measurementUI->cS11->clear();
    measurementUI->cS12->clear();
    measurementUI->cS21->clear();
    measurementUI->cS22->clear();
    measurementUI->cS11->addItem("None");
    measurementUI->cS12->addItem("None");
    measurementUI->cS21->addItem("None");
    measurementUI->cS22->addItem("None");
    // add applicable traces
    for(auto t : tm.getTraces()) {
        if(t->isReflection()) {
            measurementUI->cS11->addItem(t->name(), QVariant::fromValue<Trace*>(t));
            measurementUI->cS22->addItem(t->name(), QVariant::fromValue<Trace*>(t));
        } else {
            measurementUI->cS12->addItem(t->name(), QVariant::fromValue<Trace*>(t));
            measurementUI->cS21->addItem(t->name(), QVariant::fromValue<Trace*>(t));
        }
    }
    measurementUI->cS11->blockSignals(false);
    measurementUI->cS12->blockSignals(false);
    measurementUI->cS21->blockSignals(false);
    measurementUI->cS22->blockSignals(false);
}

void Deembedding::traceSelectionChanged(QComboBox *w)
{
    vector<QComboBox*> cbs;
    if (measurementUI->cS11->isVisible()) {
        cbs.push_back(measurementUI->cS11);
    }
    if (measurementUI->cS12->isVisible()) {
        cbs.push_back(measurementUI->cS12);
    }
    if (measurementUI->cS21->isVisible()) {
        cbs.push_back(measurementUI->cS21);
    }
    if (measurementUI->cS22->isVisible()) {
        cbs.push_back(measurementUI->cS22);
    }

    // update available traces in combo boxes
    if(w->currentIndex() != 0 && points == 0) {
        // the first trace has been selected, extract frequency info
        Trace *t = qvariant_cast<Trace*>(w->itemData(w->currentIndex()));
        points = t->size();
        if(points > 0) {
            minFreq = t->minX();
            maxFreq = t->maxX();
        }
        // remove all trace options with incompatible frequencies
        for(auto c : cbs) {
            for(int i=1;i<c->count();i++) {
                Trace *t = qvariant_cast<Trace*>(c->itemData(i));
                if(t->size() != points || (points > 0 && (t->minX() != minFreq || t->maxX() != maxFreq))) {
                    // this trace is not available anymore
                    c->removeItem(i);
                    // decrement to check the next index in the next loop iteration
                    i--;
                }
            }
        }
    } else if(w->currentIndex() == 0 && points > 0) {
        measurementUI->buttonBox->setEnabled(false);
        // Check if all trace selections are set for none
        for(auto c : cbs) {
            if(c->currentIndex() != 0) {
                // some trace is still selected, abort
                return;
            }
        }
        // all traces set for none
        points = 0;
        minFreq = 0;
        maxFreq = 0;
        setInitialTraceSelections();
    }
    bool allSelectionsValid = true;
    for(auto c : cbs) {
        if (c->currentIndex() == 0) {
            allSelectionsValid = false;
            break;
        }
    }
    if(allSelectionsValid) {
        measurementUI->buttonBox->setEnabled(true);
    }
}

void Deembedding::startMeasurementDialog(bool S11, bool S12, bool S21, bool S22)
{
    measurements.clear();

    points = 0;
    minFreq = 0.0;
    maxFreq = 0.0;

    measurementDialog = new QDialog;
    auto ui = new Ui_DeembeddingMeasurementDialog;
    measurementUI = ui;
    ui->setupUi(measurementDialog);
    // disable not needed GUI elements
    if(!S11) {
        ui->lS11->setVisible(false);
        ui->cS11->setVisible(false);
    }
    if(!S12) {
        ui->lS12->setVisible(false);
        ui->cS12->setVisible(false);
    }
    if(!S21) {
        ui->lS21->setVisible(false);
        ui->cS21->setVisible(false);
    }
    if(!S22) {
        ui->lS22->setVisible(false);
        ui->cS22->setVisible(false);
    }

    connect(ui->bMeasure, &QPushButton::clicked, [=](){
        ui->bMeasure->setEnabled(false);
        ui->cS11->setEnabled(false);
        ui->cS12->setEnabled(false);
        ui->cS21->setEnabled(false);
        ui->cS22->setEnabled(false);
        ui->buttonBox->setEnabled(false);
        measuring = true;
    });

    connect(ui->cS11, qOverload<int>(&QComboBox::currentIndexChanged), [=](int){
        traceSelectionChanged(ui->cS11);
    });
    connect(ui->cS12, qOverload<int>(&QComboBox::currentIndexChanged), [=](int){
        traceSelectionChanged(ui->cS12);
    });
    connect(ui->cS21, qOverload<int>(&QComboBox::currentIndexChanged), [=](int){
        traceSelectionChanged(ui->cS21);
    });
    connect(ui->cS22, qOverload<int>(&QComboBox::currentIndexChanged), [=](int){
        traceSelectionChanged(ui->cS22);
    });
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){
        // create datapoints from individual traces
        measurements.clear();
        Trace *S11 = nullptr, *S12 = nullptr, *S21 = nullptr, *S22 = nullptr;
        if (ui->cS11->currentIndex() != 0) {
            S11 = qvariant_cast<Trace*>(ui->cS11->itemData(ui->cS11->currentIndex()));
        }
        if (ui->cS12->currentIndex() != 0) {
            S12 = qvariant_cast<Trace*>(ui->cS12->itemData(ui->cS12->currentIndex()));
        }
        if (ui->cS21->currentIndex() != 0) {
            S21 = qvariant_cast<Trace*>(ui->cS21->itemData(ui->cS21->currentIndex()));
        }
        if (ui->cS22->currentIndex() != 0) {
            S22 = qvariant_cast<Trace*>(ui->cS22->itemData(ui->cS22->currentIndex()));
        }
        for(unsigned int i=0;i<points;i++) {
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

    setInitialTraceSelections();

    measurementDialog->show();
}

Deembedding::Deembedding(TraceModel &tm)
    : tm(tm),
      measuring(false),
      points(0)
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

void Deembedding::removeOption(unsigned int index)
{
    if(index < options.size()) {
        delete options[index];
        options.erase(options.begin() + index);
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
