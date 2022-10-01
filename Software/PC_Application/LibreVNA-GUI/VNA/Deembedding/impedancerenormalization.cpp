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

std::set<int> ImpedanceRenormalization::getAffectedPorts()
{
    set<int> ret;
    for(int i=1;i<=VirtualDevice::getInfo(VirtualDevice::getConnected()).ports;i++) {
        ret.insert(i);
    }
    return ret;
}

void ImpedanceRenormalization::transformDatapoint(VirtualDevice::VNAMeasurement &p)
{
    std::map<QString, std::complex<double>> transformed;
    int ports = 0;
    QString name = "S11";
    while(p.measurements.count(name) > 0) {
        ports++;
        name = "S"+QString::number(ports+1)+QString::number(ports+1);
    }
    for(auto i=1;i<=ports;i++) {
        auto S11name = "S"+QString::number(i)+QString::number(i);
        auto S11 = p.measurements[S11name];
        transformed[S11name] = Sparam(ABCDparam(Sparam(S11, 0.0, 0.0, 1.0), p.Z0), impedance).m11;
        for(auto j=i+1;j<=ports;j++) {
                auto S12name = "S"+QString::number(i)+QString::number(j);
                auto S21name = "S"+QString::number(j)+QString::number(i);
                auto S22name = "S"+QString::number(j)+QString::number(j);
                if(!p.measurements.count(S12name) || !p.measurements.count(S21name) || !p.measurements.count(S22name)) {
                    // not all measurements available, skip this
                    continue;
                }
                auto S12 = p.measurements[S12name];
                auto S21 = p.measurements[S21name];
                auto S22 = p.measurements[S22name];
            auto S_t = Sparam(ABCDparam(Sparam(S11, S12, S21, S22), p.Z0), impedance);
            transformed[S12name] = S_t.m12;
            transformed[S21name] = S_t.m21;
        }
    }
    p.measurements = transformed;
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
