#include "calkitdialog.h"
#include "ui_calkitdialog.h"
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
        if(ui->through_measurement->isChecked() && !ui->through_touchstone->getStatus()) {
            ok = false;
        }
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ok);
        ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(ok);
    };

    connect(ui->open_touchstone, &TouchstoneImport::statusChanged, UpdateStatus);
    connect(ui->short_touchstone, &TouchstoneImport::statusChanged, UpdateStatus);
    connect(ui->load_touchstone, &TouchstoneImport::statusChanged, UpdateStatus);
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
            ownKit = Calkit::fromFile(filename);
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
    // type
    ownKit.SOLT.Open.useMeasurements = ui->open_measurement->isChecked();
    ownKit.SOLT.Short.useMeasurements = ui->short_measurement->isChecked();
    ownKit.SOLT.Load.useMeasurements = ui->load_measurement->isChecked();
    ownKit.SOLT.Through.useMeasurements = ui->through_measurement->isChecked();

    // coefficients
    ownKit.SOLT.Open.Z0 = ui->open_Z0->value();
    ownKit.SOLT.Open.delay = ui->open_delay->value();
    ownKit.SOLT.Open.loss = ui->open_loss->value();
    ownKit.SOLT.Open.C0 = ui->open_C0->value();
    ownKit.SOLT.Open.C1 = ui->open_C1->value();
    ownKit.SOLT.Open.C2 = ui->open_C2->value();
    ownKit.SOLT.Open.C3 = ui->open_C3->value();

    ownKit.SOLT.Short.Z0 = ui->short_Z0->value();
    ownKit.SOLT.Short.delay = ui->short_delay->value();
    ownKit.SOLT.Short.loss = ui->short_loss->value();
    ownKit.SOLT.Short.L0 = ui->short_L0->value();
    ownKit.SOLT.Short.L1 = ui->short_L1->value();
    ownKit.SOLT.Short.L2 = ui->short_L2->value();
    ownKit.SOLT.Short.L3 = ui->short_L3->value();

    ownKit.SOLT.Load.Z0 = ui->load_Z0->value();
    ownKit.SOLT.Load.delay = ui->load_delay->value();
    ownKit.SOLT.Load.Cparallel = ui->load_parC->value();
    ownKit.SOLT.Load.Lseries = ui->load_serL->value();

    ownKit.SOLT.Through.Z0 = ui->through_Z0->value();
    ownKit.SOLT.Through.delay = ui->through_delay->value();
    ownKit.SOLT.Through.loss = ui->through_loss->value();

    // file
    ownKit.SOLT.Open.file = ui->open_touchstone->getFilename();
    ownKit.SOLT.Short.file = ui->short_touchstone->getFilename();
    ownKit.SOLT.Load.file = ui->load_touchstone->getFilename();
    ownKit.SOLT.Through.file = ui->through_touchstone->getFilename();

    ownKit.SOLT.Open.Sparam = ui->open_touchstone->getPorts()[0];
    ownKit.SOLT.Short.Sparam = ui->short_touchstone->getPorts()[0];
    ownKit.SOLT.Load.Sparam = ui->load_touchstone->getPorts()[0];
    ownKit.SOLT.Through.Sparam1 = ui->through_touchstone->getPorts()[0];
    ownKit.SOLT.Through.Sparam2 = ui->through_touchstone->getPorts()[1];

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
    // Coefficients
    ui->open_Z0->setValueQuiet(ownKit.SOLT.Open.Z0);
    ui->open_delay->setValueQuiet(ownKit.SOLT.Open.delay);
    ui->open_loss->setValueQuiet(ownKit.SOLT.Open.loss);
    ui->open_C0->setValueQuiet(ownKit.SOLT.Open.C0);
    ui->open_C1->setValueQuiet(ownKit.SOLT.Open.C1);
    ui->open_C2->setValueQuiet(ownKit.SOLT.Open.C2);
    ui->open_C3->setValueQuiet(ownKit.SOLT.Open.C3);

    ui->short_Z0->setValueQuiet(ownKit.SOLT.Short.Z0);
    ui->short_delay->setValueQuiet(ownKit.SOLT.Short.delay);
    ui->short_loss->setValueQuiet(ownKit.SOLT.Short.loss);
    ui->short_L0->setValueQuiet(ownKit.SOLT.Short.L0);
    ui->short_L1->setValueQuiet(ownKit.SOLT.Short.L1);
    ui->short_L2->setValueQuiet(ownKit.SOLT.Short.L2);
    ui->short_L3->setValueQuiet(ownKit.SOLT.Short.L3);

    ui->load_Z0->setValueQuiet(ownKit.SOLT.Load.Z0);
    ui->load_delay->setValueQuiet(ownKit.SOLT.Load.delay);
    ui->load_parC->setValueQuiet(ownKit.SOLT.Load.Cparallel);
    ui->load_serL->setValueQuiet(ownKit.SOLT.Load.Lseries);

    ui->through_Z0->setValueQuiet(ownKit.SOLT.Through.Z0);
    ui->through_delay->setValueQuiet(ownKit.SOLT.Through.delay);
    ui->through_loss->setValueQuiet(ownKit.SOLT.Through.loss);

    // Measurements
    ui->open_touchstone->setFile(ownKit.SOLT.Open.file);
    ui->open_touchstone->selectPort(0, ownKit.SOLT.Open.Sparam);

    ui->short_touchstone->setFile(ownKit.SOLT.Short.file);
    ui->short_touchstone->selectPort(0, ownKit.SOLT.Short.Sparam);

    ui->load_touchstone->setFile(ownKit.SOLT.Load.file);
    ui->load_touchstone->selectPort(0, ownKit.SOLT.Load.Sparam);

    ui->through_touchstone->setFile(ownKit.SOLT.Through.file);
    ui->through_touchstone->selectPort(0, ownKit.SOLT.Through.Sparam1);
    ui->through_touchstone->selectPort(1, ownKit.SOLT.Through.Sparam2);

    // Type
    if (ownKit.SOLT.Open.useMeasurements) {
        ui->open_measurement->click();
    } else {
        ui->open_coefficients->click();
    }

    if (ownKit.SOLT.Short.useMeasurements) {
        ui->short_measurement->click();
    } else {
        ui->short_coefficients->click();
    }

    if (ownKit.SOLT.Load.useMeasurements) {
        ui->load_measurement->click();
    } else {
        ui->load_coefficients->click();
    }

    if (ownKit.SOLT.Through.useMeasurements) {
        ui->through_measurement->click();
    } else {
        ui->through_coefficients->click();
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
