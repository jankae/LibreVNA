#include "impedancerenormalization.h"

#include "ui_impedancenormalizationdialog.h"
#include "Tools/parameters.h"
#include "appwindow.h"
#include "Util/util.h"

#include <complex>

using namespace std;

ImpedanceRenormalization::ImpedanceRenormalization()
    : DeembeddingOption("ZNORMalization"),
      impedance(50.0)
{
    add(new SCPICommand("IMPedance", [=](QStringList params) -> QString {
        double new_value;
        if(!SCPI::paramToDouble(params, 0, new_value)) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        impedance = new_value;
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList params) -> QString {
        Q_UNUSED(params);
        return QString::number(impedance);
    }));
}

std::set<unsigned int> ImpedanceRenormalization::getAffectedPorts()
{
    set<unsigned int> ret;
    for(unsigned int i=1;i<=DeviceDriver::getInfo(DeviceDriver::getActiveDriver()).Limits.VNA.ports;i++) {
        ret.insert(i);
    }
    return ret;
}

void ImpedanceRenormalization::transformDatapoint(DeviceDriver::VNAMeasurement &p)
{
    auto S = p.toSparam();
    auto Z = Zparam(S, p.Z0);
    auto S_renorm = Sparam(Z, impedance);
    p.fromSparam(S_renorm);
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
