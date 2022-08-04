#include "impedancerenormalization.h"

#include "ui_impedancenormalizationdialog.h"
#include "Tools/parameters.h"
#include "appwindow.h"

#include <complex>

using namespace std;

ImpedanceRenormalization::ImpedanceRenormalization()
    : DeembeddingOption(),
      impedance(50.0)
{

}

void ImpedanceRenormalization::transformDatapoint(VirtualDevice::VNAMeasurement &p)
{
    //p.S = Sparam(ABCDparam(p.S, p.Z0), impedance);
    // TODO
    p.Z0 = impedance;
}

nlohmann::json ImpedanceRenormalization::toJSON()
{
    nlohmann::json j;
    j["impedance"] = impedance;
    return j;
}

void ImpedanceRenormalization::fromJSON(nlohmann::json j)
{
    impedance = j.value("impedance", impedance);
}

void ImpedanceRenormalization::edit()
{
    auto dialog = new QDialog();
    ui = new Ui::ImpedanceRenormalizationDialog();
    ui->setupUi(dialog);
    connect(dialog, &QDialog::finished, [=](){
        delete ui;
    });

    // set initial values
    ui->impedance->setUnit("Ω");
    ui->impedance->setPrecision(3);
    ui->impedance->setValue(impedance);

    connect(ui->impedance, &SIUnitEdit::valueChanged, [&](double newval){
       impedance = newval;
    });

    if(AppWindow::showGUI()) {
        dialog->show();
    }
}
