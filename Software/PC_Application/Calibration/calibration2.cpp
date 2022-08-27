#include "calibration2.h"
#include "ui_calibrationdialogui.h"

#include <QDialog>
#include <QMenu>

using namespace std;

Calibration2::Calibration2()
{

}

void Calibration2::edit()
{
    auto d = new QDialog();
    auto ui = new Ui::CalibrationDialog;
    ui->setupUi(d);

    QObject::connect(ui->bDelete, &QPushButton::clicked, [=](){
        auto row = ui->table->currentRow();
        if(row >= 0) {
            delete measurements[row];
            measurements.erase(measurements.begin() + row);
//            updateMeasurementTable();
        }
    });

    QObject::connect(ui->bMoveUp, &QPushButton::clicked, [=](){
        auto row = ui->table->currentRow();
        if(row >= 1) {
            swap(measurements[row], measurements[row-1]);
            ui->table->selectRow(row-1);
//            updateMeasurementTable();
        }
    });

    QObject::connect(ui->bMoveDown, &QPushButton::clicked, [=](){
        auto row = ui->table->currentRow();
        if(row >= 1) {
            swap(measurements[row], measurements[row-1]);
            ui->table->selectRow(row+1);
//            updateMeasurementTable();
        }
    });

//    connect(ui->table, &QTableWidget::currentRowChanged, this, &CalibrationDialog::updateTableEditButtons);

    auto addMenu = new QMenu();
    for(auto t : CalibrationMeasurement::Base::availableTypes()) {
        auto action = new QAction(CalibrationMeasurement::Base::TypeToString(t));
        QObject::connect(action, &QAction::triggered, [=](){
            auto newMeas = newMeasurement(t);
            if(newMeas) {
                measurements.push_back(newMeas);
//                updateMeasurementTable();
            }
        });
        addMenu->addAction(action);
    }

    ui->bAdd->setMenu(addMenu);

//    updateMeasurementTable();

    d->show();
}

CalibrationMeasurement::Base *Calibration2::newMeasurement(CalibrationMeasurement::Base::Type type)
{
    CalibrationMeasurement::Base *m = nullptr;
    switch(type) {
    case CalibrationMeasurement::Base::Type::Open: m = new CalibrationMeasurement::Open(this); break;
    case CalibrationMeasurement::Base::Type::Short: m = new CalibrationMeasurement::Short(this); break;
    case CalibrationMeasurement::Base::Type::Load: m = new CalibrationMeasurement::Load(this); break;
    case CalibrationMeasurement::Base::Type::Through: m = new CalibrationMeasurement::Through(this); break;
    }
    return m;
}

Calkit &Calibration2::getKit()
{
    return kit;
}
