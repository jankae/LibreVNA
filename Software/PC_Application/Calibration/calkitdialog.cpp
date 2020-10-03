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
       ownKit.TRL_line_minfreq = newval;
       updateEntries();
    });
    connect(ui->TRL_line_max, &SIUnitEdit::valueChanged, [=](double newval){
       ownKit.TRL_line_maxfreq = newval;
       updateEntries();
    });
    connect(ui->TRL_line_delay, &QLineEdit::editingFinished, [=](){
        ownKit.TRL_line_delay = ui->TRL_line_delay->text().toDouble();
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
        delete this;
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, [this]() {
        delete this;
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Open), &QPushButton::clicked, [=](){
        auto filename = QFileDialog::getOpenFileName(this, "Open calibration kit coefficients", "", "Calibration kit files (*.calkit)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.length() > 0) {
            ownKit = Calkit::fromFile(filename.toStdString());
            updateEntries();
        }
    });

    connect(ui->buttonBox->button(QDialogButtonBox::Save), &QPushButton::clicked, [=](){
        auto filename = QFileDialog::getSaveFileName(this, "Save calibration kit coefficients", "", "Calibration kit files (*.calkit)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.length() > 0) {
            parseEntries();
            ownKit.toFile(filename.toStdString());
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
    ownKit.open_measurements = ui->open_measurement->isChecked();
    ownKit.short_measurements = ui->short_measurement->isChecked();
    ownKit.load_measurements = ui->load_measurement->isChecked();
    ownKit.through_measurements = ui->through_measurement->isChecked();

    // coefficients
    ownKit.open_Z0 = ui->open_Z0->text().toDouble();
    ownKit.open_delay = ui->open_delay->text().toDouble();
    ownKit.open_loss = ui->open_loss->text().toDouble();
    ownKit.open_C0 = ui->open_C0->text().toDouble();
    ownKit.open_C1 = ui->open_C1->text().toDouble();
    ownKit.open_C2 = ui->open_C2->text().toDouble();
    ownKit.open_C3 = ui->open_C3->text().toDouble();

    ownKit.short_Z0 = ui->short_Z0->text().toDouble();
    ownKit.short_delay = ui->short_delay->text().toDouble();
    ownKit.short_loss = ui->short_loss->text().toDouble();
    ownKit.short_L0 = ui->short_L0->text().toDouble();
    ownKit.short_L1 = ui->short_L1->text().toDouble();
    ownKit.short_L2 = ui->short_L2->text().toDouble();
    ownKit.short_L3 = ui->short_L3->text().toDouble();

    ownKit.load_Z0 = ui->load_Z0->text().toDouble();

    ownKit.through_Z0 = ui->through_Z0->text().toDouble();
    ownKit.through_delay = ui->through_delay->text().toDouble();
    ownKit.through_loss = ui->through_loss->text().toDouble();

    // file
    ownKit.open_file = ui->open_touchstone->getFilename().toStdString();
    ownKit.short_file = ui->short_touchstone->getFilename().toStdString();
    ownKit.load_file = ui->load_touchstone->getFilename().toStdString();
    ownKit.through_file = ui->through_touchstone->getFilename().toStdString();

    ownKit.open_Sparam = ui->open_touchstone->getPorts()[0];
    ownKit.short_Sparam = ui->short_touchstone->getPorts()[0];
    ownKit.load_Sparam = ui->load_touchstone->getPorts()[0];
    ownKit.through_Sparam1 = ui->through_touchstone->getPorts()[0];
    ownKit.through_Sparam2 = ui->through_touchstone->getPorts()[1];

    // TRL
    ownKit.TRL_through_Z0 = ui->TRL_through_Z0->text().toDouble();
    ownKit.TRL_reflection_short = ui->TRL_R_short->isChecked();
    ownKit.TRL_line_delay = ui->TRL_line_delay->text().toDouble();
    ownKit.TRL_line_minfreq = ui->TRL_line_min->value();
    ownKit.TRL_line_maxfreq = ui->TRL_line_max->value();
}

void CalkitDialog::updateEntries()
{
    // Coefficients
    ui->open_Z0->setText(QString::number(ownKit.open_Z0));
    ui->open_delay->setText(QString::number(ownKit.open_delay));
    ui->open_loss->setText(QString::number(ownKit.open_loss));
    ui->open_C0->setText(QString::number(ownKit.open_C0));
    ui->open_C1->setText(QString::number(ownKit.open_C1));
    ui->open_C2->setText(QString::number(ownKit.open_C2));
    ui->open_C3->setText(QString::number(ownKit.open_C3));

    ui->short_Z0->setText(QString::number(ownKit.short_Z0));
    ui->short_delay->setText(QString::number(ownKit.short_delay));
    ui->short_loss->setText(QString::number(ownKit.short_loss));
    ui->short_L0->setText(QString::number(ownKit.short_L0));
    ui->short_L1->setText(QString::number(ownKit.short_L1));
    ui->short_L2->setText(QString::number(ownKit.short_L2));
    ui->short_L3->setText(QString::number(ownKit.short_L3));

    ui->load_Z0->setText(QString::number(ownKit.load_Z0));

    ui->through_Z0->setText(QString::number(ownKit.through_Z0));
    ui->through_delay->setText(QString::number(ownKit.through_delay));
    ui->through_loss->setText(QString::number(ownKit.through_loss));

    // Measurements
    ui->open_touchstone->setFile(QString::fromStdString(ownKit.open_file));
    ui->open_touchstone->selectPort(0, ownKit.open_Sparam);

    ui->short_touchstone->setFile(QString::fromStdString(ownKit.short_file));
    ui->short_touchstone->selectPort(0, ownKit.short_Sparam);

    ui->load_touchstone->setFile(QString::fromStdString(ownKit.load_file));
    ui->load_touchstone->selectPort(0, ownKit.load_Sparam);

    ui->through_touchstone->setFile(QString::fromStdString(ownKit.through_file));
    ui->through_touchstone->selectPort(0, ownKit.through_Sparam1);
    ui->through_touchstone->selectPort(1, ownKit.through_Sparam2);

    // Type
    if (ownKit.open_measurements) {
        ui->open_measurement->click();
    } else {
        ui->open_coefficients->click();
    }

    if (ownKit.short_measurements) {
        ui->short_measurement->click();
    } else {
        ui->short_coefficients->click();
    }

    if (ownKit.load_measurements) {
        ui->load_measurement->click();
    } else {
        ui->load_coefficients->click();
    }

    if (ownKit.through_measurements) {
        ui->through_measurement->click();
    } else {
        ui->through_coefficients->click();
    }

    // TRL
    ui->TRL_through_Z0->setText(QString::number(ownKit.TRL_through_Z0));
    if(ownKit.TRL_reflection_short) {
        ui->TRL_R_short->setChecked(true);
    } else {
        ui->TRL_R_open->setChecked(true);
    }
    ui->TRL_line_delay->setText(QString::number(ownKit.TRL_line_delay));
    ui->TRL_line_min->setValueQuiet(ownKit.TRL_line_minfreq);
    ui->TRL_line_max->setValueQuiet(ownKit.TRL_line_maxfreq);
    // Check if line length is appropriate for frequencies
    auto minDelay = 20.0/(ownKit.TRL_line_minfreq * 360.0) * 1e12;
    auto maxDelay = 160.0/(ownKit.TRL_line_maxfreq * 360.0) * 1e12;
    if(ownKit.TRL_line_delay < minDelay) {
        ui->TRL_line_warning->setText("Line too short, minimum required delay is "+QString::number(minDelay, 'g', 3) + "ps");
    } else if(ownKit.TRL_line_delay > maxDelay) {
        ui->TRL_line_warning->setText("Line too long, maximum allowed delay is "+QString::number(maxDelay, 'g', 3) + "ps");
    } else {
        ui->TRL_line_warning->clear();
    }
}
