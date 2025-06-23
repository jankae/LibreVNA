#include "tracetouchstoneexport.h"

#include "ui_tracetouchstoneexport.h"
#include "touchstone.h"
#include "preferences.h"

#include <QDebug>
#include <QFileDialog>
#include <QPushButton>

TraceTouchstoneExport::TraceTouchstoneExport(TraceModel &model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TraceTouchstoneExport)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
    ui->selector->setModel(&model);
    ui->selector->setPartialSelectionAllowed(true);
    connect(ui->selector, qOverload<>(&TraceSetSelector::selectionChanged), this, &TraceTouchstoneExport::selectionChanged);
    connect(ui->sbPorts, &QSpinBox::valueChanged, this, &TraceTouchstoneExport::setPortNum);
    // restore the last used settings
    auto& pref = Preferences::getInstance();
    auto ports = pref.UISettings.TouchstoneExport.ports;
    setPortNum(ports);
    ui->cFormat->setCurrentIndex(pref.UISettings.TouchstoneExport.formatIndex);
    ui->cUnit->setCurrentIndex(pref.UISettings.TouchstoneExport.unitIndex);

    // attempt to set the traces that were exported last
    QStringList traces = pref.UISettings.TouchstoneExport.exportedTraceNames.split(",");
    if(traces.size() == ports * ports) {
        // got the correct number of traces
        for(unsigned int i=0;i<traces.size();i++) {
            for(auto t : model.getTraces()) {
                if(t->name() == traces[i]) {
                    setTrace(i / ports + 1, i % ports + 1, t);
                    break;
                }
            }
        }
    }

//     unsigned int p;
//     for(p=4;p>=1;p--) {
//         // do we have a trace name which could indicate such a number of ports?
//         for(unsigned int i=1;i<=p;i++) {
//             auto n1 = "S"+QString::number(p)+QString::number(i);
//             auto n2 = "S"+QString::number(i)+QString::number(p);
//             for(auto t : model.getTraces()) {
//                 if(t->name().contains(n1) || t->name().contains(n2)) {
//                     goto traceFound;
//                 }
//             }
//         }
//     }
// traceFound:
//     setPortNum(p);
}

TraceTouchstoneExport::~TraceTouchstoneExport()
{
    delete ui;
}

bool TraceTouchstoneExport::setTrace(int portTo, int portFrom, Trace *t)
{
    return ui->selector->setTrace(portTo, portFrom, t);
}

bool TraceTouchstoneExport::setPortNum(unsigned int ports)
{
    if(ports < 1 || ports > 4) {
        return false;
    }
    if((unsigned int) ui->sbPorts->value() == ports && ui->selector->getPorts() == ports) {
        // already set correctly, nothing to do
        return true;
    }
    ui->sbPorts->setValue(ports);
    ui->selector->setPorts(ports);
    // Attempt to set default traces (this will result in correctly populated
    // 2 port export if the initial 4 traces have not been modified)
    auto traces = ui->selector->getModel()->getTraces();
    for(unsigned int i=1;i<=ports;i++) {
        for(unsigned int j=1;j<=ports;j++) {
            auto name = "S"+QString::number(i)+QString::number(j);
            for(auto t : traces) {
                if(t->name().contains(name)) {
                    // this could be the correct trace
                    setTrace(i, j, t);
                    break;
                }
            }
        }
    }
    return true;
}

void TraceTouchstoneExport::on_buttonBox_accepted()
{
    unsigned int ports = ui->sbPorts->value();
    QString extension = ".s"+QString::number(ports)+"p";
    auto filename = QFileDialog::getSaveFileName(this, "Select file for exporting traces", Preferences::getInstance().UISettings.Paths.data, "Touchstone files (*"+extension+")", nullptr, Preferences::QFileDialogOptions());
    if(filename.length() > 0) {
        Preferences::getInstance().UISettings.Paths.data = QFileInfo(filename).path();
        auto t = Touchstone(ports);
        t.setReferenceImpedance(ui->selector->getReferenceImpedance());
        // add trace points to touchstone
        for(unsigned int s=0;s<ui->selector->getPoints();s++) {
            Touchstone::Datapoint tData;
            for(unsigned int i=1;i<=ports;i++) {
                for(unsigned int j=1;j<=ports;j++) {
                    auto t = ui->selector->getTrace(i, j);
                    if(!t) {
                        // missing trace, set to 0
                        tData.S.push_back(0.0);
                    } else {
                        // extract frequency (will overwrite for each trace but all traces have the same frequency points anyway)
                        tData.frequency = t->sample(s).x;
                        // add S parameter from trace to touchstone
                        tData.S.push_back(t->sample(s).y);
                    }
                }
            }
            t.AddDatapoint(tData);
        }
        Touchstone::Scale unit = Touchstone::Scale::GHz;
        switch(ui->cUnit->currentIndex()) {
        case 0: unit = Touchstone::Scale::Hz; break;
        case 1: unit = Touchstone::Scale::kHz; break;
        case 2: unit = Touchstone::Scale::MHz; break;
        case 3: unit = Touchstone::Scale::GHz; break;
        }
        Touchstone::Format format = Touchstone::Format::RealImaginary;
        switch(ui->cFormat->currentIndex()) {
        case 0: format = Touchstone::Format::DBAngle; break;
        case 1: format = Touchstone::Format::MagnitudeAngle; break;
        case 2: format = Touchstone::Format::RealImaginary; break;
        }

        t.toFile(filename, unit, format);

        // update preferences for next call
        auto& pref = Preferences::getInstance();
        pref.UISettings.TouchstoneExport.ports = ui->sbPorts->value();
        pref.UISettings.TouchstoneExport.formatIndex = ui->cFormat->currentIndex();
        pref.UISettings.TouchstoneExport.unitIndex = ui->cUnit->currentIndex();
        QString traceNames = "";
        for(unsigned int i=0;i<ports*ports;i++) {
            auto t = ui->selector->getTrace(i / ports + 1, i % ports + 1);
            if(t) {
                traceNames += t->name();
            }
            if(i != (ports*ports-1)) {
                // add separator for all but the last trace name
                traceNames += ",";
            }
        }
        pref.UISettings.TouchstoneExport.exportedTraceNames = traceNames;

        delete this;
    }
}

void TraceTouchstoneExport::selectionChanged()
{
    auto valid = ui->selector->selectionValid();
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(valid);
    if(valid) {
        ui->points->setText(QString::number(ui->selector->getPoints()));
        if(ui->selector->getPoints() > 0) {
            ui->lowerFreq->setText(QString::number(ui->selector->getLowerFreq()));
            ui->upperFreq->setText(QString::number(ui->selector->getUpperFreq()));
        }
    } else {
        ui->points->clear();
        ui->lowerFreq->clear();
        ui->upperFreq->clear();
    }
}
