#include "calkitdialog.h"

#include "ui_calkitdialog.h"
#include "CustomWidgets/informationbox.h"
#include "preferences.h"

#include <QPushButton>
#include <QDebug>
#include <QFileDialog>
#include <fstream>
#include <touchstone.h>
#include <QtGlobal>
#include <QAction>
#include <QMenu>

using namespace std;

CalkitDialog::CalkitDialog(Calkit &c, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalkitDialog),
    kit(c)
{
    ui->setupUi(this);

    updateEntries();

    connect(ui->bDelete, &QPushButton::clicked, [=](){
        auto row = ui->list->currentRow();
        if(row >= 0) {
            delete kit.standards[row];
            kit.standards.erase(kit.standards.begin() + row);
            updateStandardList();
        }
    });

    connect(ui->bMoveUp, &QPushButton::clicked, [=](){
        auto row = ui->list->currentRow();
        if(row >= 1) {
            swap(kit.standards[row], kit.standards[row-1]);
            ui->list->setCurrentRow(row-1);
            updateStandardList();
        }
    });

    connect(ui->bMoveDown, &QPushButton::clicked, [=](){
        auto row = ui->list->currentRow();
        if(row < ui->list->count() - 1) {
            swap(kit.standards[row], kit.standards[row+1]);
            ui->list->setCurrentRow(row+1);
            updateStandardList();
        }
    });

    connect(ui->list, &QListWidget::currentRowChanged, this, &CalkitDialog::updateListEditButtons);

    auto addMenu = new QMenu();
    for(auto t : CalStandard::Virtual::availableTypes()) {
        auto action = new QAction(CalStandard::Virtual::TypeToString(t));
        connect(action, &QAction::triggered, [=](){
            auto newStandard = CalStandard::Virtual::create(t);
            if(newStandard) {
                kit.addStandard(newStandard);
                updateStandardList();
                // start the edit dialog of the newly created standard
                kit.standards.back()->edit(bind(&CalkitDialog::updateStandardList, this));
            }
        });
        addMenu->addAction(action);
    }

    ui->bAdd->setMenu(addMenu);

    updateStandardList();

    connect(ui->list, &QListWidget::doubleClicked, [=](const QModelIndex &index){
        if(!index.isValid()) {
            return;
        }
        kit.standards[index.row()]->edit(bind(&CalkitDialog::updateStandardList, this));
    });

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, [this]() {
        parseEntries();
        emit settingsChanged();
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, [this]() {
        parseEntries();
        emit settingsChanged();
        accept();
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Open), &QPushButton::clicked, [=](){
        auto filename = QFileDialog::getOpenFileName(this, "Open calibration kit coefficients", "", "Calibration kit files (*.calkit)", nullptr, Preferences::QFileDialogOptions());
        if(filename.length() > 0) {
            try {
                kit = Calkit::fromFile(filename);
            } catch (runtime_error &e) {
                InformationBox::ShowError("Error", "The calibration kit file could not be parsed (" + QString(e.what()) + ")");
                qWarning() << "Parsing of calibration kit failed while opening calibration file: " << e.what();
            }
            updateEntries();
            updateStandardList();
        }
    });

    connect(ui->buttonBox->button(QDialogButtonBox::Save), &QPushButton::clicked, [=](){
        auto filename = QFileDialog::getSaveFileName(this, "Save calibration kit coefficients", "", "Calibration kit files (*.calkit)", nullptr, Preferences::QFileDialogOptions());
        if(filename.length() > 0) {
            parseEntries();
            kit.toFile(filename);
        }
    });
}

CalkitDialog::~CalkitDialog()
{
    delete ui;
}

void CalkitDialog::updateListEditButtons()
{
    ui->bDelete->setEnabled(ui->list->currentRow() >= 0);
    ui->bMoveUp->setEnabled(ui->list->currentRow() >= 1);
    ui->bMoveDown->setEnabled(ui->list->currentRow() >= 0 && ui->list->currentRow() < ui->list->count() - 1);
}

void CalkitDialog::parseEntries()
{
    kit.manufacturer = ui->manufacturer->text();
    kit.serialnumber = ui->serialnumber->text();
    kit.description = ui->description->toPlainText();
}

void CalkitDialog::updateEntries()
{
    ui->manufacturer->setText(kit.manufacturer);
    ui->serialnumber->setText(kit.serialnumber);
    ui->description->setPlainText(kit.description);
}

void CalkitDialog::updateStandardList()
{
    auto row = ui->list->currentRow();
    ui->list->clear();
    for(auto s : kit.standards) {
        ui->list->addItem(s->getDescription());
    }
    if(row >= 0) {
        if(row < ui->list->count()) {
            ui->list->setCurrentRow(row);
        } else if(ui->list->count() > 0) {
            ui->list->setCurrentRow(ui->list->count() - 1);
        }
    }
    updateListEditButtons();
}
