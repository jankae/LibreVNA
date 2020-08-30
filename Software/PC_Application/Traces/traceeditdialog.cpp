#include "traceeditdialog.h"
#include "ui_traceeditdialog.h"
#include <QColorDialog>
#include <QFileDialog>

TraceEditDialog::TraceEditDialog(Trace &t, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TraceEditDialog),
    trace(t)
{
    ui->setupUi(this);
    ui->name->setText(t.name());
    setColor(trace.color());

    ui->GSource->setId(ui->bLive, 0);
    ui->GSource->setId(ui->bFile, 1);

    if(t.isCalibration()) {
        // prevent editing imported calibration traces
        ui->bLive->setEnabled(false);
        ui->bFile->setEnabled(false);
        ui->CLiveType->setEnabled(false);
        ui->CLiveParam->setEnabled(false);
    }

    if(t.isTouchstone()) {
        ui->bFile->click();
        ui->touchstoneImport->setFile(t.getTouchstoneFilename());
    }

    auto updateFileStatus = [this]() {
        // remove all options from paramater combo box
        while(ui->CParameter->count() > 0) {
            ui->CParameter->removeItem(0);
        }
        if (ui->bFile->isChecked() && !ui->touchstoneImport->getStatus())  {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        } else {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            auto touchstone = ui->touchstoneImport->getTouchstone();
            for(unsigned int i=0;i<touchstone.ports();i++) {
                for(unsigned int j=0;j<touchstone.ports();j++) {
                    QString name = "S"+QString::number(i+1)+QString::number(j+1);
                    ui->CParameter->addItem(name);
                }
            }
            if(trace.getTouchstoneParameter() < touchstone.ports()*touchstone.ports()) {
                ui->CParameter->setCurrentIndex(trace.getTouchstoneParameter());
            } else {
                ui->CParameter->setCurrentIndex(0);
            }
        }
    };

    switch(t.liveType()) {
    case Trace::LivedataType::Overwrite: ui->CLiveType->setCurrentIndex(0); break;
    case Trace::LivedataType::MaxHold: ui->CLiveType->setCurrentIndex(1); break;
    case Trace::LivedataType::MinHold: ui->CLiveType->setCurrentIndex(2); break;
    }

    switch(t.liveParameter()) {
    case Trace::LiveParameter::S11: ui->CLiveParam->setCurrentIndex(0); break;
    case Trace::LiveParameter::S12: ui->CLiveParam->setCurrentIndex(1); break;
    case Trace::LiveParameter::S21: ui->CLiveParam->setCurrentIndex(2); break;
    case Trace::LiveParameter::S22: ui->CLiveParam->setCurrentIndex(3); break;
    }

    connect(ui->GSource, qOverload<int>(&QButtonGroup::buttonClicked), updateFileStatus);
    connect(ui->touchstoneImport, &TouchstoneImport::statusChanged, updateFileStatus);
    connect(ui->touchstoneImport, &TouchstoneImport::filenameChanged, updateFileStatus);

    updateFileStatus();
}

TraceEditDialog::~TraceEditDialog()
{
    delete ui;
}

void TraceEditDialog::on_color_clicked()
{
    auto color = QColorDialog::getColor(trace.color(), this, "Select color", QColorDialog::DontUseNativeDialog);
    setColor(color);
}


void TraceEditDialog::on_buttonBox_accepted()
{
    trace.setName(ui->name->text());
    if(!trace.isCalibration()) {
        // only apply changes if it is not a calibration trace
        if (ui->bFile->isChecked()) {
            auto t = ui->touchstoneImport->getTouchstone();
            trace.fillFromTouchstone(t, ui->CParameter->currentIndex(), ui->touchstoneImport->getFilename());
        } else {
            Trace::LivedataType type;
            Trace::LiveParameter param;
            switch(ui->CLiveType->currentIndex()) {
            case 0: type = Trace::LivedataType::Overwrite; break;
            case 1: type = Trace::LivedataType::MaxHold; break;
            case 2: type = Trace::LivedataType::MinHold; break;
            }
            switch(ui->CLiveParam->currentIndex()) {
            case 0: param = Trace::LiveParameter::S11; break;
            case 1: param = Trace::LiveParameter::S12; break;
            case 2: param = Trace::LiveParameter::S21; break;
            case 3: param = Trace::LiveParameter::S22; break;
            }
            trace.fromLivedata(type, param);
        }
    }
    delete this;
}

void TraceEditDialog::setColor(QColor c)
{
    QPalette pal = ui->color->palette();
    pal.setColor(QPalette::Button, c);
    ui->color->setAutoFillBackground(true);
    ui->color->setPalette(pal);
    ui->color->update();
    trace.setColor(c);
}
