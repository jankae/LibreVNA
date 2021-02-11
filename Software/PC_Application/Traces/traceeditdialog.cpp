#include "traceeditdialog.h"
#include "ui_traceeditdialog.h"
#include <QColorDialog>
#include <QFileDialog>
#include "Math/tracematheditdialog.h"

TraceEditDialog::TraceEditDialog(Trace &t, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TraceEditDialog),
    trace(t)
{
    ui->setupUi(this);
    ui->vFactor->setPrecision(3);
    ui->name->setText(t.name());
    ui->color->setColor(trace.color());
    ui->vFactor->setValue(t.velocityFactor());
    connect(ui->color, &ColorPickerButton::colorChanged, [=](const QColor& color){
       trace.setColor(color);
    });

    ui->GSource->setId(ui->bLive, 0);
    ui->GSource->setId(ui->bFile, 1);

    if(t.isCalibration() || (t.isFromFile() && t.getFilename().endsWith(".csv"))) {
        // prevent editing imported calibration traces (and csv files for now)
        ui->bLive->setEnabled(false);
        ui->bFile->setEnabled(false);
        ui->CLiveType->setEnabled(false);
        ui->CLiveParam->setEnabled(false);
    }

    if(t.isFromFile() && !t.getFilename().endsWith(".csv")) {
        ui->bFile->click();
        ui->touchstoneImport->setFile(t.getFilename());
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
            if(trace.getFileParameter() < touchstone.ports()*touchstone.ports()) {
                ui->CParameter->setCurrentIndex(trace.getFileParameter());
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
    case Trace::LiveParameter::S11:
    case Trace::LiveParameter::S12:
    case Trace::LiveParameter::S21:
    case Trace::LiveParameter::S22:
        VNAtrace = true;
        ui->CLiveParam->addItem("S11");
        ui->CLiveParam->addItem("S12");
        ui->CLiveParam->addItem("S21");
        ui->CLiveParam->addItem("S22");
        break;
    case Trace::LiveParameter::Port1:
    case Trace::LiveParameter::Port2:
        ui->CLiveParam->addItem("Port 1");
        ui->CLiveParam->addItem("Port 2");
        VNAtrace = false;
        break;
    }

    switch(t.liveParameter()) {
    case Trace::LiveParameter::S11: ui->CLiveParam->setCurrentIndex(0); break;
    case Trace::LiveParameter::S12: ui->CLiveParam->setCurrentIndex(1); break;
    case Trace::LiveParameter::S21: ui->CLiveParam->setCurrentIndex(2); break;
    case Trace::LiveParameter::S22: ui->CLiveParam->setCurrentIndex(3); break;
    case Trace::LiveParameter::Port1: ui->CLiveParam->setCurrentIndex(0); break;
    case Trace::LiveParameter::Port2: ui->CLiveParam->setCurrentIndex(1); break;
    }

    connect(ui->GSource, qOverload<int>(&QButtonGroup::buttonClicked), updateFileStatus);
    connect(ui->touchstoneImport, &TouchstoneImport::statusChanged, updateFileStatus);
    connect(ui->touchstoneImport, &TouchstoneImport::filenameChanged, updateFileStatus);
    connect(ui->mathSetup, &QPushButton::clicked, [&](){
        auto dialog = new TraceMathEditDialog(t);
        dialog->show();
    });

    updateFileStatus();
}

TraceEditDialog::~TraceEditDialog()
{
    delete ui;
}

void TraceEditDialog::on_buttonBox_accepted()
{
    trace.setName(ui->name->text());
    trace.setVelocityFactor(ui->vFactor->value());
    if(!trace.isCalibration()) {
        // only apply changes if it is not a calibration trace
        if (ui->bFile->isChecked()) {
            auto t = ui->touchstoneImport->getTouchstone();
            trace.fillFromTouchstone(t, ui->CParameter->currentIndex());
        } else {
            Trace::LivedataType type = Trace::LivedataType::Overwrite;
            Trace::LiveParameter param = Trace::LiveParameter::S11;
            switch(ui->CLiveType->currentIndex()) {
            case 0: type = Trace::LivedataType::Overwrite; break;
            case 1: type = Trace::LivedataType::MaxHold; break;
            case 2: type = Trace::LivedataType::MinHold; break;
            }
            if(VNAtrace) {
                switch(ui->CLiveParam->currentIndex()) {
                case 0: param = Trace::LiveParameter::S11; break;
                case 1: param = Trace::LiveParameter::S12; break;
                case 2: param = Trace::LiveParameter::S21; break;
                case 3: param = Trace::LiveParameter::S22; break;
                }
            } else {
                switch(ui->CLiveParam->currentIndex()) {
                case 0: param = Trace::LiveParameter::Port1; break;
                case 1: param = Trace::LiveParameter::Port2; break;
                }
            }
            trace.fromLivedata(type, param);
        }
    }
    delete this;
}
