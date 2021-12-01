#include "calkitdialog.h"

#include "ui_calkitdialog.h"
#include "CustomWidgets/informationbox.h"

#include <QPushButton>
#include <QDebug>
#include <QFileDialog>
#include <fstream>
#include <touchstone.h>
#include <QtGlobal>

using namespace std;

CalkitDialog::CalkitDialog(Calkit &c, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalkitDialog),
    open_ok(true),
    short_ok(true),
    load_ok(true),
    through_ok(true),
    editKit(c)
{
    ui->setupUi(this);

    ui->OpenType->setId(ui->open_coefficients, 0);
    ui->OpenType->setId(ui->open_measurement, 1);

    ui->ShortType->setId(ui->short_coefficients, 0);
    ui->ShortType->setId(ui->short_measurement, 1);

    ui->LoadType->setId(ui->load_coefficients, 0);
    ui->LoadType->setId(ui->load_measurement, 1);

    ui->ThroughType->setId(ui->through_coefficients, 0);
    ui->ThroughType->setId(ui->through_measurement, 1);

    ui->open_touchstone->setPorts(1);
    ui->short_touchstone->setPorts(1);
    ui->load_touchstone->setPorts(1);
    ui->through_touchstone->setPorts(2);

    ui->short_Z0->setUnit("Ω");
    ui->open_Z0->setUnit("Ω");
    ui->load_Z0->setUnit("Ω");
    ui->load_parC->setUnit("F");
    ui->load_parC->setPrefixes("fpnum ");
    ui->load_serL->setUnit("H");
    ui->load_serL->setPrefixes("fpnum ");

    // Same setup for female standards
    ui->OpenType_f->setId(ui->open_coefficients_f, 0);
    ui->OpenType_f->setId(ui->open_measurement_f, 1);

    ui->ShortType_f->setId(ui->short_coefficients_f, 0);
    ui->ShortType_f->setId(ui->short_measurement_f, 1);

    ui->LoadType_f->setId(ui->load_coefficients_f, 0);
    ui->LoadType_f->setId(ui->load_measurement_f, 1);

    ui->open_touchstone_f->setPorts(1);
    ui->short_touchstone_f->setPorts(1);
    ui->load_touchstone_f->setPorts(1);

    ui->short_Z0_f->setUnit("Ω");
    ui->open_Z0_f->setUnit("Ω");
    ui->load_Z0_f->setUnit("Ω");
    ui->load_parC_f->setUnit("F");
    ui->load_parC_f->setPrefixes("fpnum ");
    ui->load_serL_f->setUnit("H");
    ui->load_serL_f->setPrefixes("fpnum ");

    ui->through_Z0->setUnit("Ω");

    ui->TRL_through_Z0->setUnit("Ω");
    ui->TRL_line_max->setUnit("Hz");
    ui->TRL_line_max->setPrecision(4);
    ui->TRL_line_max->setPrefixes(" kMG");
    ui->TRL_line_min->setUnit("Hz");
    ui->TRL_line_min->setPrecision(4);
    ui->TRL_line_min->setPrefixes(" kMG");

    editKit.clearTouchstoneCache();
    ownKit = editKit;

    connect(ui->cbStandardDefinition, qOverload<int>(&QComboBox::currentIndexChanged), [=](int index){
        if (index == 0) {
            // common definition, hide tab bars, set all to male tab
            ui->mf_short->setCurrentIndex(0);
            ui->mf_short->tabBar()->hide();
            ui->mf_open->setCurrentIndex(0);
            ui->mf_open->tabBar()->hide();
            ui->mf_load->setCurrentIndex(0);
            ui->mf_load->tabBar()->hide();
        } else {
            // separate definitions for male/female standards
            ui->mf_short->tabBar()->show();
            ui->mf_open->tabBar()->show();
            ui->mf_load->tabBar()->show();
        }
    });

    updateEntries();

    connect(ui->TRL_line_min, &SIUnitEdit::valueChanged, [=](double newval){
       ownKit.TRL.Line.minFreq = newval;
       updateEntries();
    });
    connect(ui->TRL_line_max, &SIUnitEdit::valueChanged, [=](double newval){
       ownKit.TRL.Line.maxFreq = newval;
       updateEntries();
    });
    connect(ui->TRL_line_delay, &QLineEdit::editingFinished, [=](){
        ownKit.TRL.Line.delay = ui->TRL_line_delay->value();
        updateEntries();
    });

    auto UpdateStatus = [=]() {
        bool ok = true;
        if(ui->open_measurement->isChecked() && !ui->open_touchstone->getStatus()) {
            ok = false;
        }
        if(ui->short_measurement->isChecked() && !ui->short_touchstone->getStatus()) {
            ok = false;
        }
        if(ui->load_measurement->isChecked() && !ui->load_touchstone->getStatus()) {
            ok = false;
        }
        if(ui->open_measurement_f->isChecked() && !ui->open_touchstone_f->getStatus()) {
            ok = false;
        }
        if(ui->short_measurement_f->isChecked() && !ui->short_touchstone_f->getStatus()) {
            ok = false;
        }
        if(ui->load_measurement_f->isChecked() && !ui->load_touchstone_f->getStatus()) {
            ok = false;
        }
        if(ui->through_measurement->isChecked() && !ui->through_touchstone->getStatus()) {
            ok = false;
        }
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ok);
        ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(ok);
    };

    connect(ui->open_touchstone, &TouchstoneImport::statusChanged, UpdateStatus);
    connect(ui->short_touchstone, &TouchstoneImport::statusChanged, UpdateStatus);
    connect(ui->load_touchstone, &TouchstoneImport::statusChanged, UpdateStatus);
    connect(ui->open_touchstone_f, &TouchstoneImport::statusChanged, UpdateStatus);
    connect(ui->short_touchstone_f, &TouchstoneImport::statusChanged, UpdateStatus);
    connect(ui->load_touchstone_f, &TouchstoneImport::statusChanged, UpdateStatus);
    connect(ui->through_touchstone, &TouchstoneImport::statusChanged, UpdateStatus);

    connect(ui->OpenType, qOverload<int>(&QButtonGroup::buttonClicked), [=](int) {
        UpdateStatus();
    });
    connect(ui->ShortType, qOverload<int>(&QButtonGroup::buttonClicked), [=](int) {
        UpdateStatus();
    });
    connect(ui->LoadType, qOverload<int>(&QButtonGroup::buttonClicked), [=](int) {
        UpdateStatus();
    });
    connect(ui->OpenType_f, qOverload<int>(&QButtonGroup::buttonClicked), [=](int) {
        UpdateStatus();
    });
    connect(ui->ShortType_f, qOverload<int>(&QButtonGroup::buttonClicked), [=](int) {
        UpdateStatus();
    });
    connect(ui->LoadType_f, qOverload<int>(&QButtonGroup::buttonClicked), [=](int) {
        UpdateStatus();
    });
    connect(ui->ThroughType, qOverload<int>(&QButtonGroup::buttonClicked), [=](int) {
        UpdateStatus();
    });

    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, [this]() {
        parseEntries();
        editKit = ownKit;
        accept();
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, [this]() {
        reject();
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Open), &QPushButton::clicked, [=](){
        auto filename = QFileDialog::getOpenFileName(this, "Open calibration kit coefficients", "", "Calibration kit files (*.calkit)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.length() > 0) {
            try {
                ownKit = Calkit::fromFile(filename);
            } catch (runtime_error e) {
                InformationBox::ShowError("Error", "The calibration kit file could not be parsed (" + QString(e.what()) + ")");
                qWarning() << "Parsing of calibration kit failed while opening calibration file: " << e.what();
            }
            updateEntries();
        }
    });

    connect(ui->buttonBox->button(QDialogButtonBox::Save), &QPushButton::clicked, [=](){
        auto filename = QFileDialog::getSaveFileName(this, "Save calibration kit coefficients", "", "Calibration kit files (*.calkit)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.length() > 0) {
            parseEntries();
            ownKit.toFile(filename);
        }
    });
}

CalkitDialog::~CalkitDialog()
{
    delete ui;
}

void CalkitDialog::parseEntries()
{
    ownKit.manufacturer = ui->manufacturer->text();
    ownKit.serialnumber = ui->serialnumber->text();
    ownKit.description = ui->description->toPlainText();

    // type
    ownKit.SOLT.open_m.useMeasurements = ui->open_measurement->isChecked();
    ownKit.SOLT.short_m.useMeasurements = ui->short_measurement->isChecked();
    ownKit.SOLT.load_m.useMeasurements = ui->load_measurement->isChecked();
    ownKit.SOLT.open_f.useMeasurements = ui->open_measurement_f->isChecked();
    ownKit.SOLT.short_f.useMeasurements = ui->short_measurement_f->isChecked();
    ownKit.SOLT.load_f.useMeasurements = ui->load_measurement_f->isChecked();
    ownKit.SOLT.Through.useMeasurements = ui->through_measurement->isChecked();

    // coefficients
    ownKit.SOLT.open_m.Z0 = ui->open_Z0->value();
    ownKit.SOLT.open_m.delay = ui->open_delay->value();
    ownKit.SOLT.open_m.loss = ui->open_loss->value();
    ownKit.SOLT.open_m.C0 = ui->open_C0->value();
    ownKit.SOLT.open_m.C1 = ui->open_C1->value();
    ownKit.SOLT.open_m.C2 = ui->open_C2->value();
    ownKit.SOLT.open_m.C3 = ui->open_C3->value();

    ownKit.SOLT.short_m.Z0 = ui->short_Z0->value();
    ownKit.SOLT.short_m.delay = ui->short_delay->value();
    ownKit.SOLT.short_m.loss = ui->short_loss->value();
    ownKit.SOLT.short_m.L0 = ui->short_L0->value();
    ownKit.SOLT.short_m.L1 = ui->short_L1->value();
    ownKit.SOLT.short_m.L2 = ui->short_L2->value();
    ownKit.SOLT.short_m.L3 = ui->short_L3->value();

    ownKit.SOLT.load_m.Z0 = ui->load_Z0->value();
    ownKit.SOLT.load_m.delay = ui->load_delay->value();
    ownKit.SOLT.load_m.Cparallel = ui->load_parC->value();
    ownKit.SOLT.load_m.Lseries = ui->load_serL->value();

    ownKit.SOLT.open_f.Z0 = ui->open_Z0_f->value();
    ownKit.SOLT.open_f.delay = ui->open_delay_f->value();
    ownKit.SOLT.open_f.loss = ui->open_loss_f->value();
    ownKit.SOLT.open_f.C0 = ui->open_C0_f->value();
    ownKit.SOLT.open_f.C1 = ui->open_C1_f->value();
    ownKit.SOLT.open_f.C2 = ui->open_C2_f->value();
    ownKit.SOLT.open_f.C3 = ui->open_C3_f->value();

    ownKit.SOLT.short_f.Z0 = ui->short_Z0_f->value();
    ownKit.SOLT.short_f.delay = ui->short_delay_f->value();
    ownKit.SOLT.short_f.loss = ui->short_loss_f->value();
    ownKit.SOLT.short_f.L0 = ui->short_L0_f->value();
    ownKit.SOLT.short_f.L1 = ui->short_L1_f->value();
    ownKit.SOLT.short_f.L2 = ui->short_L2_f->value();
    ownKit.SOLT.short_f.L3 = ui->short_L3_f->value();

    ownKit.SOLT.load_f.Z0 = ui->load_Z0_f->value();
    ownKit.SOLT.load_f.delay = ui->load_delay_f->value();
    ownKit.SOLT.load_f.Cparallel = ui->load_parC_f->value();
    ownKit.SOLT.load_f.Lseries = ui->load_serL_f->value();

    ownKit.SOLT.Through.Z0 = ui->through_Z0->value();
    ownKit.SOLT.Through.delay = ui->through_delay->value();
    ownKit.SOLT.Through.loss = ui->through_loss->value();

    ownKit.SOLT.separate_male_female = ui->cbStandardDefinition->currentIndex() == 1;

    // file
    ownKit.SOLT.open_m.file = ui->open_touchstone->getFilename();
    ownKit.SOLT.short_m.file = ui->short_touchstone->getFilename();
    ownKit.SOLT.load_m.file = ui->load_touchstone->getFilename();
    ownKit.SOLT.Through.file = ui->through_touchstone->getFilename();

    ownKit.SOLT.open_m.Sparam = ui->open_touchstone->getPorts()[0];
    ownKit.SOLT.short_m.Sparam = ui->short_touchstone->getPorts()[0];
    ownKit.SOLT.load_m.Sparam = ui->load_touchstone->getPorts()[0];
    ownKit.SOLT.Through.Sparam1 = ui->through_touchstone->getPorts()[0];
    ownKit.SOLT.Through.Sparam2 = ui->through_touchstone->getPorts()[1];

    ownKit.SOLT.open_f.file = ui->open_touchstone_f->getFilename();
    ownKit.SOLT.short_f.file = ui->short_touchstone_f->getFilename();
    ownKit.SOLT.load_f.file = ui->load_touchstone_f->getFilename();

    ownKit.SOLT.open_f.Sparam = ui->open_touchstone_f->getPorts()[0];
    ownKit.SOLT.short_f.Sparam = ui->short_touchstone_f->getPorts()[0];
    ownKit.SOLT.load_f.Sparam = ui->load_touchstone_f->getPorts()[0];

    // TRL
    ownKit.TRL.Through.Z0 = ui->TRL_through_Z0->value();
    ownKit.TRL.Reflection.isShort = ui->TRL_R_short->isChecked();
    ownKit.TRL.Line.delay = ui->TRL_line_delay->value();
    ownKit.TRL.Line.minFreq = ui->TRL_line_min->value();
    ownKit.TRL.Line.maxFreq = ui->TRL_line_max->value();

    ownKit.startDialogWithSOLT = ui->tabWidget->currentIndex() == 0;
}

void CalkitDialog::updateEntries()
{
    ui->manufacturer->setText(ownKit.manufacturer);
    ui->serialnumber->setText(ownKit.serialnumber);
    ui->description->setPlainText(ownKit.description);

    // Coefficients
    ui->open_Z0->setValueQuiet(ownKit.SOLT.open_m.Z0);
    ui->open_delay->setValueQuiet(ownKit.SOLT.open_m.delay);
    ui->open_loss->setValueQuiet(ownKit.SOLT.open_m.loss);
    ui->open_C0->setValueQuiet(ownKit.SOLT.open_m.C0);
    ui->open_C1->setValueQuiet(ownKit.SOLT.open_m.C1);
    ui->open_C2->setValueQuiet(ownKit.SOLT.open_m.C2);
    ui->open_C3->setValueQuiet(ownKit.SOLT.open_m.C3);

    ui->short_Z0->setValueQuiet(ownKit.SOLT.short_m.Z0);
    ui->short_delay->setValueQuiet(ownKit.SOLT.short_m.delay);
    ui->short_loss->setValueQuiet(ownKit.SOLT.short_m.loss);
    ui->short_L0->setValueQuiet(ownKit.SOLT.short_m.L0);
    ui->short_L1->setValueQuiet(ownKit.SOLT.short_m.L1);
    ui->short_L2->setValueQuiet(ownKit.SOLT.short_m.L2);
    ui->short_L3->setValueQuiet(ownKit.SOLT.short_m.L3);

    ui->load_Z0->setValueQuiet(ownKit.SOLT.load_m.Z0);
    ui->load_delay->setValueQuiet(ownKit.SOLT.load_m.delay);
    ui->load_parC->setValueQuiet(ownKit.SOLT.load_m.Cparallel);
    ui->load_serL->setValueQuiet(ownKit.SOLT.load_m.Lseries);

    ui->open_Z0_f->setValueQuiet(ownKit.SOLT.open_f.Z0);
    ui->open_delay_f->setValueQuiet(ownKit.SOLT.open_f.delay);
    ui->open_loss_f->setValueQuiet(ownKit.SOLT.open_f.loss);
    ui->open_C0_f->setValueQuiet(ownKit.SOLT.open_f.C0);
    ui->open_C1_f->setValueQuiet(ownKit.SOLT.open_f.C1);
    ui->open_C2_f->setValueQuiet(ownKit.SOLT.open_f.C2);
    ui->open_C3_f->setValueQuiet(ownKit.SOLT.open_f.C3);

    ui->short_Z0_f->setValueQuiet(ownKit.SOLT.short_f.Z0);
    ui->short_delay_f->setValueQuiet(ownKit.SOLT.short_f.delay);
    ui->short_loss_f->setValueQuiet(ownKit.SOLT.short_f.loss);
    ui->short_L0_f->setValueQuiet(ownKit.SOLT.short_f.L0);
    ui->short_L1_f->setValueQuiet(ownKit.SOLT.short_f.L1);
    ui->short_L2_f->setValueQuiet(ownKit.SOLT.short_f.L2);
    ui->short_L3_f->setValueQuiet(ownKit.SOLT.short_f.L3);

    ui->load_Z0_f->setValueQuiet(ownKit.SOLT.load_f.Z0);
    ui->load_delay_f->setValueQuiet(ownKit.SOLT.load_f.delay);
    ui->load_parC_f->setValueQuiet(ownKit.SOLT.load_f.Cparallel);
    ui->load_serL_f->setValueQuiet(ownKit.SOLT.load_f.Lseries);

    ui->through_Z0->setValueQuiet(ownKit.SOLT.Through.Z0);
    ui->through_delay->setValueQuiet(ownKit.SOLT.Through.delay);
    ui->through_loss->setValueQuiet(ownKit.SOLT.Through.loss);

    // Measurements
    ui->open_touchstone->setFile(ownKit.SOLT.open_m.file);
    ui->open_touchstone->selectPort(0, ownKit.SOLT.open_m.Sparam);

    ui->short_touchstone->setFile(ownKit.SOLT.short_m.file);
    ui->short_touchstone->selectPort(0, ownKit.SOLT.short_m.Sparam);

    ui->load_touchstone->setFile(ownKit.SOLT.load_m.file);
    ui->load_touchstone->selectPort(0, ownKit.SOLT.load_m.Sparam);

    ui->open_touchstone_f->setFile(ownKit.SOLT.open_f.file);
    ui->open_touchstone_f->selectPort(0, ownKit.SOLT.open_f.Sparam);

    ui->short_touchstone_f->setFile(ownKit.SOLT.short_f.file);
    ui->short_touchstone_f->selectPort(0, ownKit.SOLT.short_f.Sparam);

    ui->load_touchstone_f->setFile(ownKit.SOLT.load_f.file);
    ui->load_touchstone_f->selectPort(0, ownKit.SOLT.load_f.Sparam);

    ui->through_touchstone->setFile(ownKit.SOLT.Through.file);
    ui->through_touchstone->selectPort(0, ownKit.SOLT.Through.Sparam1);
    ui->through_touchstone->selectPort(1, ownKit.SOLT.Through.Sparam2);

    // Type
    if (ownKit.SOLT.open_m.useMeasurements) {
        ui->open_measurement->click();
    } else {
        ui->open_coefficients->click();
    }

    if (ownKit.SOLT.short_m.useMeasurements) {
        ui->short_measurement->click();
    } else {
        ui->short_coefficients->click();
    }

    if (ownKit.SOLT.load_m.useMeasurements) {
        ui->load_measurement->click();
    } else {
        ui->load_coefficients->click();
    }

    if (ownKit.SOLT.open_f.useMeasurements) {
        ui->open_measurement_f->click();
    } else {
        ui->open_coefficients_f->click();
    }

    if (ownKit.SOLT.short_f.useMeasurements) {
        ui->short_measurement_f->click();
    } else {
        ui->short_coefficients_f->click();
    }

    if (ownKit.SOLT.load_f.useMeasurements) {
        ui->load_measurement_f->click();
    } else {
        ui->load_coefficients_f->click();
    }

    if (ownKit.SOLT.Through.useMeasurements) {
        ui->through_measurement->click();
    } else {
        ui->through_coefficients->click();
    }

    if (ownKit.SOLT.separate_male_female) {
        ui->cbStandardDefinition->setCurrentIndex(1);
    } else {
        ui->cbStandardDefinition->setCurrentIndex(0);
    }

    // TRL
    ui->TRL_through_Z0->setValueQuiet(ownKit.TRL.Through.Z0);
    if(ownKit.TRL.Reflection.isShort) {
        ui->TRL_R_short->setChecked(true);
    } else {
        ui->TRL_R_open->setChecked(true);
    }
    ui->TRL_line_delay->setValueQuiet(ownKit.TRL.Line.delay);
    ui->TRL_line_min->setValueQuiet(ownKit.TRL.Line.minFreq);
    ui->TRL_line_max->setValueQuiet(ownKit.TRL.Line.maxFreq);
    // Check if line length is appropriate for frequencies
    auto minDelay = 20.0/(ownKit.TRL.Line.minFreq * 360.0) * 1e12;
    auto maxDelay = 160.0/(ownKit.TRL.Line.maxFreq * 360.0) * 1e12;
    if(ownKit.TRL.Line.delay < minDelay) {
        ui->TRL_line_warning->setText("Line too short, minimum required delay is "+QString::number(minDelay, 'g', 3) + "ps");
    } else if(ownKit.TRL.Line.delay > maxDelay) {
        ui->TRL_line_warning->setText("Line too long, maximum allowed delay is "+QString::number(maxDelay, 'g', 3) + "ps");
    } else {
        ui->TRL_line_warning->clear();
    }

    if (ownKit.startDialogWithSOLT) {
        ui->tabWidget->setCurrentIndex(0);
    } else {
        ui->tabWidget->setCurrentIndex(1);
    }
}
