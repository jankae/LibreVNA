#include "tracetouchstoneexport.h"
#include "ui_tracetouchstoneexport.h"
#include <QDebug>
#include <QFileDialog>
#include "touchstone.h"
#include <QPushButton>

TraceTouchstoneExport::TraceTouchstoneExport(TraceModel &model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TraceTouchstoneExport),
    model(model),
    ports(0),
    freqsSet(false)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
    ui->gbTraces->setLayout(new QGridLayout);
    on_sbPorts_valueChanged(ui->sbPorts->value());
}

TraceTouchstoneExport::~TraceTouchstoneExport()
{
    delete ui;
}

bool TraceTouchstoneExport::setTrace(int portFrom, int portTo, Trace *t)
{
    if(t->getDataType() != Trace::DataType::Frequency) {
        // can only add frequency traces
        return false;
    }
    if(portFrom < 0 || portFrom >= ui->sbPorts->value() || portTo < 0 || portTo >= ui->sbPorts->value()) {
        // invalid port selection
        return false;
    }
    auto c = cTraces[portTo][portFrom];
    if(t) {
        for(int i=1;i<c->count();i++) {
            if(t == qvariant_cast<Trace*>(c->itemData(i))) {
                // select this trace
                c->setCurrentIndex(i);
                return true;
            }
        }
        // requested trace is not an option
        return false;
    } else {
        // select 'none' option
        c->setCurrentIndex(0);
        return true;
    }
}

bool TraceTouchstoneExport::setPortNum(int ports)
{
    if(ports < 1 || ports > 4) {
        return false;
    }
    ui->sbPorts->setValue(ports);
    return true;
}

void TraceTouchstoneExport::on_buttonBox_accepted()
{
    auto filename = QFileDialog::getSaveFileName(this, "Select file for exporting traces", "", "Touchstone files (*.s1p *.s2p *.s3p *.s4p)", nullptr, QFileDialog::DontUseNativeDialog);
    if(filename.length() > 0) {
        auto ports = ui->sbPorts->value();
        auto t = Touchstone(ports);
        // add trace points to touchstone
        for(unsigned int s=0;s<points;s++) {
            Touchstone::Datapoint tData;
            for(int i=0;i<ports;i++) {
                for(int j=0;j<ports;j++) {
                    if(cTraces[i][j]->currentIndex() == 0) {
                        // missing trace, set to 0
                        tData.S.push_back(0.0);
                    } else {
                        Trace *t = qvariant_cast<Trace*>(cTraces[i][j]->itemData(cTraces[i][j]->currentIndex()));
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

        t.toFile(filename.toStdString(), unit, format);
        delete this;
    }
}

void TraceTouchstoneExport::on_sbPorts_valueChanged(int ports)
{
    QGridLayout *layout = static_cast<QGridLayout*>(ui->gbTraces->layout());
    if((unsigned) ports != this->ports) {
        this->ports = ports;
        // remove the previous widgets
        QLayoutItem *child;
        while ((child = layout->takeAt(0)) != 0)  {
            delete child->widget();
            delete child;
        }
        cTraces.clear();
        for(int i=0;i<ports;i++) {
            cTraces.push_back(std::vector<QComboBox*>());
            for(int j=0;j<ports;j++) {
                auto l = new QLabel("S"+QString::number(i+1)+QString::number(j+1)+":");
                auto c = new QComboBox();
                cTraces[i].push_back(c);
                connect(c, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) {
                   selectionChanged(c);
                });
                layout->addWidget(l, i, j*2);
                layout->addWidget(c, i, j*2 + 1);
            }
        }
    }
    auto availableTraces = model.getTraces();
    for(int i=0;i<ports;i++) {
        for(int j=0;j<ports;j++) {
            auto c = cTraces[i][j];
            c->blockSignals(true);
            c->clear();
            // create possible trace selections
            c->addItem("None");
            for(auto t : availableTraces) {
                if(t->getDataType() != Trace::DataType::Frequency) {
                    // can only add frequency traces
                    continue;
                }
                if(i == j && !t->isReflection()) {
                    // can not add through measurement at reflection port
                    continue;
                } else if(i != j && t->isReflection()) {
                    // can not add reflection measurement at through port
                    continue;
                }
                c->addItem(t->name(), QVariant::fromValue<Trace*>(t));
            }
            c->blockSignals(false);
        }
    }
}

void TraceTouchstoneExport::selectionChanged(QComboBox *w)
{
    if(w->currentIndex() != 0 && !freqsSet) {
        // the first trace has been selected, extract frequency info
        Trace *t = qvariant_cast<Trace*>(w->itemData(w->currentIndex()));
        points = t->size();
        ui->points->setText(QString::number(points));
        if(points > 0) {
            lowerFreq = t->minX();
            upperFreq = t->maxX();
            ui->lowerFreq->setText(QString::number(lowerFreq));
            ui->upperFreq->setText(QString::number(upperFreq));
        }
        freqsSet = true;
        ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
        // remove all trace options with incompatible frequencies
        for(auto v1 : cTraces) {
            for(auto c : v1) {
                for(int i=1;i<c->count();i++) {
                    Trace *t = qvariant_cast<Trace*>(c->itemData(i));
                    if(t->size() != points || (points > 0 && (t->minX() != lowerFreq || t->maxX() != upperFreq))) {
                        // this trace is not available anymore
                        c->removeItem(i);
                        // decrement to check the next index in the next loop iteration
                        i--;
                    }
                }
            }
        }
    } else if(w->currentIndex() == 0 && freqsSet) {
        // Check if all trace selections are set for none
        for(auto v1 : cTraces) {
            for(auto c : v1) {
                if(c->currentIndex() != 0) {
                    // some trace is still selected, abort
                    return;
                }
            }
        }
        // all traces set for none
        freqsSet = false;
        ui->points->clear();
        ui->lowerFreq->clear();
        ui->upperFreq->clear();
        ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
        on_sbPorts_valueChanged(ui->sbPorts->value());
    }
}
