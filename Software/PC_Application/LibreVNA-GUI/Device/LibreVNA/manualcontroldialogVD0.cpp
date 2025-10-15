#include "manualcontroldialogVD0.h"

#include "ui_manualcontroldialogVD0.h"
#include "Util/util.h"

#include <QComboBox>
#include <QDebug>
#include <QButtonGroup>
#include <complex>


using namespace std;

ManualControlDialogVD0::ManualControlDialogVD0(LibreVNADriver &dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManualControlDialogVD0),
    dev(dev)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    emit dev.acquireControl();

    ui->SourceLowFrequency->setUnit("Hz");
    ui->SourceLowFrequency->setPrefixes(" kM");
    ui->SourceLowFrequency->setPrecision(6);
    ui->SourceLowFrequency->setValueQuiet(1000000);

    ui->SourceHighFrequency->setUnit("Hz");
    ui->SourceHighFrequency->setPrefixes(" kMG");
    ui->SourceHighFrequency->setPrecision(6);
    ui->SourceHighFrequency->setValueQuiet(1000000000);

    ui->IFHigh->setUnit("Hz");
    ui->IFHigh->setPrefixes(" kM");
    ui->IFHigh->setPrecision(6);

    ui->IFLow->setUnit("Hz");
    ui->IFLow->setPrefixes(" kM");
    ui->IFLow->setPrecision(6);

    ui->LOHighFrequency->setUnit("Hz");
    ui->LOHighFrequency->setPrefixes(" kMG");
    ui->LOHighFrequency->setPrecision(6);

    ui->LOLowFrequency->setUnit("Hz");
    ui->LOLowFrequency->setPrefixes(" kM");
    ui->LOLowFrequency->setPrecision(6);

    auto UpdateLOHigh = [=]() {
        double sourceFreq;
        if (ui->SwitchLowband->isChecked()) {
            sourceFreq = ui->SourceLowFrequency->value();
        } else {
            sourceFreq = ui->SourceHighFrequency->value();
        }
        if (ui->LOHighFreqType->currentIndex() == 0) {
            // fixed IF mode
            ui->LOHighFrequency->setValueQuiet(sourceFreq + ui->IFHigh->value());
        } else {
            // Manual Frequency mode
            ui->IFHigh->setValueQuiet(ui->LOHighFrequency->value() - sourceFreq);
        }
    };
    auto UpdateLOLow = [=]() {
        double sourceFreq;
        if (ui->SwitchLowband->isChecked()) {
            sourceFreq = ui->SourceLowFrequency->value();
        } else {
            sourceFreq = ui->SourceHighFrequency->value();
        }
        if (ui->LOLowFreqType->currentIndex() == 0) {
            // fixed IF mode
            ui->LOLowFrequency->setValueQuiet(sourceFreq + ui->IFLow->value());
        } else {
            // Manual Frequency mode
            ui->IFLow->setValueQuiet(ui->LOLowFrequency->value() - sourceFreq);
        }
    };

    connect(ui->IFHigh, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLOHigh();
    });
    connect(ui->LOHighFrequency, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLOHigh();
    });
    connect(ui->IFLow, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLOLow();
    });
    connect(ui->LOLowFrequency, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLOLow();
    });
    connect(ui->SourceSwitchGroup, qOverload<int, bool>(&QButtonGroup::idToggled), [=](int, bool) {
        UpdateLOHigh();
        UpdateLOLow();
    });
    connect(ui->SourceLowFrequency, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLOHigh();
        UpdateLOLow();
    });
    connect(ui->SourceHighFrequency, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLOHigh();
        UpdateLOLow();
    });

    ui->IFHigh->setValue(233750);
    ui->IFLow->setValue(233750);

    // LO high/low mode switch connections
    connect(ui->LOHighFreqType, qOverload<int>(&QComboBox::activated), this, [=](int index) {
        switch(index) {
        case 0:
            ui->LOHighFrequency->setEnabled(false);
            ui->IFHigh->setEnabled(true);
            break;
        case 1:
            ui->LOHighFrequency->setEnabled(true);
            ui->IFHigh->setEnabled(false);
            break;
        }
    });
    connect(ui->LOLowFreqType, qOverload<int>(&QComboBox::activated), this, [=](int index) {
        switch(index) {
        case 0:
            ui->LOLowFrequency->setEnabled(false);
            ui->IFLow->setEnabled(true);
            break;
        case 1:
            ui->LOLowFrequency->setEnabled(true);
            ui->IFLow->setEnabled(false);
            break;
        }
    });

    // Readonly widgets
    auto MakeReadOnly = [](QWidget* w) {
        w->setAttribute(Qt::WA_TransparentForMouseEvents);
        w->setFocusPolicy(Qt::NoFocus);
    };
    MakeReadOnly(ui->SourceLocked);
    MakeReadOnly(ui->LOLocked);
    MakeReadOnly(ui->port1min);
    MakeReadOnly(ui->port1max);
    MakeReadOnly(ui->port1mag);
    MakeReadOnly(ui->port1phase);
    MakeReadOnly(ui->port1referenced);
    MakeReadOnly(ui->port2min);
    MakeReadOnly(ui->port2max);
    MakeReadOnly(ui->port2mag);
    MakeReadOnly(ui->port2phase);
    MakeReadOnly(ui->port2referenced);
    MakeReadOnly(ui->refmin);
    MakeReadOnly(ui->refmax);
    MakeReadOnly(ui->refmag);
    MakeReadOnly(ui->refphase);

    connect(&dev, &LibreVNADriver::receivedPacket, this, [=](const Protocol::PacketInfo &p){
        if(p.type == Protocol::PacketType::ManualStatus) {
            NewStatus(p.manualStatus);
        }
    }, Qt::QueuedConnection);

    connect(ui->SourceCE, &QCheckBox::toggled, this, [=](bool) { UpdateDevice(); });
    connect(ui->LOHighCE, &QCheckBox::toggled, this, [=](bool) { UpdateDevice(); });
    connect(ui->SourceLowEnable, &QCheckBox::toggled, this, [=](bool) { UpdateDevice(); });
    connect(ui->LOLowEnable, &QCheckBox::toggled, this, [=](bool) { UpdateDevice(); });
    connect(ui->Port1Enable, &QCheckBox::toggled, this, [=](bool) { UpdateDevice(); });
    connect(ui->Port2Enable, &QCheckBox::toggled, this, [=](bool) { UpdateDevice(); });
    connect(ui->RefEnable, &QCheckBox::toggled, this, [=](bool) { UpdateDevice(); });

    connect(ui->SourceLowpass, qOverload<int>(&QComboBox::activated), this, [=](int) { UpdateDevice(); });
    connect(ui->SourceLowPower, qOverload<int>(&QComboBox::activated), this, [=](int) { UpdateDevice(); });
    connect(ui->LOLowPower, qOverload<int>(&QComboBox::activated), this, [=](int) { UpdateDevice(); });
    connect(ui->cbWindow, qOverload<int>(&QComboBox::activated), this, [=](int) { UpdateDevice(); });

    connect(ui->SourceHighFrequency, &SIUnitEdit::valueChanged, this, [=](double) { UpdateDevice(); });
    connect(ui->SourceLowFrequency, &SIUnitEdit::valueChanged, this, [=](double) { UpdateDevice(); });
    connect(ui->LOHighFrequency, &SIUnitEdit::valueChanged, this, [=](double) { UpdateDevice(); });
    connect(ui->IFHigh, &SIUnitEdit::valueChanged, this, [=](double) { UpdateDevice(); });
    connect(ui->LOLowFrequency, &SIUnitEdit::valueChanged, this, [=](double) { UpdateDevice(); });
    connect(ui->IFLow, &SIUnitEdit::valueChanged, this, [=](double) { UpdateDevice(); });

    connect(ui->PortSwitchGroup, qOverload<int, bool>(&QButtonGroup::idToggled), this, [=](int, bool) { UpdateDevice(); });
    connect(ui->SourceSwitchGroup, qOverload<int, bool>(&QButtonGroup::idToggled), this, [=](int, bool) { UpdateDevice(); });
    connect(ui->LOSwitchGroup, qOverload<int, bool>(&QButtonGroup::idToggled), this, [=](int, bool) { UpdateDevice(); });

    connect(ui->Attenuator, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [=](double) { UpdateDevice(); });
    connect(ui->SourceHighPower, qOverload<int>(&QSpinBox::valueChanged), this, [=](int) { UpdateDevice(); });
    connect(ui->Samples, qOverload<int>(&QSpinBox::valueChanged), this, [=](double) { UpdateDevice(); });


    // Create the SCPI commands

    auto addBooleanManualSetting = [=](QString cmd, void(ManualControlDialogVD0::*set)(bool), bool(ManualControlDialogVD0::*get)(void)) {
       commands.push_back(new SCPICommand(cmd, [=](QStringList params) -> QString {
           bool enable;
           if(!SCPI::paramToBool(params, 0, enable)) {
               return SCPI::getResultName(SCPI::Result::Error);
           }
           auto set_fn = std::bind(set, this, std::placeholders::_1);
           set_fn(enable);
           return SCPI::getResultName(SCPI::Result::Empty);
       }, [=](QStringList) -> QString {
           auto get_fn = std::bind(get, this);
           return get_fn() ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
       }));
    };

    auto addDoubleManualSetting = [=](QString cmd, void(ManualControlDialogVD0::*set)(double), double(ManualControlDialogVD0::*get)(void)) {
       commands.push_back(new SCPICommand(cmd, [=](QStringList params) -> QString {
           double value;
           if(!SCPI::paramToDouble(params, 0, value)) {
               return SCPI::getResultName(SCPI::Result::Error);
           }
           auto set_fn = std::bind(set, this, std::placeholders::_1);
           set_fn(value);
           return SCPI::getResultName(SCPI::Result::Empty);
       }, [=](QStringList) -> QString {
           auto get_fn = std::bind(get, this);
           return QString::number(get_fn());
       }));
    };
    auto addIntegerManualSetting = [=](QString cmd, void(ManualControlDialogVD0::*set)(int), int(ManualControlDialogVD0::*get)(void)) {
       commands.push_back(new SCPICommand(cmd, [=](QStringList params) -> QString {
           double value;
           if(!SCPI::paramToDouble(params, 0, value)) {
               return SCPI::getResultName(SCPI::Result::Error);
           }
           auto set_fn = std::bind(set, this, std::placeholders::_1);
           set_fn(value);
           return SCPI::getResultName(SCPI::Result::Empty);
       }, [=](QStringList) -> QString {
           auto get_fn = std::bind(get, this);
           return QString::number(get_fn());
       }));
    };
    auto addIntegerManualSettingWithReturnValue = [=](QString cmd, bool(ManualControlDialogVD0::*set)(int), int(ManualControlDialogVD0::*get)(void)) {
       commands.push_back(new SCPICommand(cmd, [=](QStringList params) -> QString {
           double value;
           if(!SCPI::paramToDouble(params, 0, value)) {
               return SCPI::getResultName(SCPI::Result::Error);
           }
           auto set_fn = std::bind(set, this, std::placeholders::_1);
           if(set_fn(value)) {
               return SCPI::getResultName(SCPI::Result::Empty);
           } else {
               return SCPI::getResultName(SCPI::Result::Error);
           }
       }, [=](QStringList) -> QString {
           auto get_fn = std::bind(get, this);
           return QString::number(get_fn());
       }));
    };
    auto addIntegerManualQuery = [=](QString cmd, int(ManualControlDialogVD0::*get)(void)) {
       commands.push_back(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
           auto get_fn = std::bind(get, this);
           return QString::number(get_fn());
       }));
    };
    auto addDoubleManualQuery = [=](QString cmd, double(ManualControlDialogVD0::*get)(void)) {
       commands.push_back(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
           auto get_fn = std::bind(get, this);
           return QString::number(get_fn());
       }));
    };
    auto addBooleanManualQuery = [=](QString cmd, bool(ManualControlDialogVD0::*get)(void)) {
       commands.push_back(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
           auto get_fn = std::bind(get, this);
           return get_fn() ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
       }));
    };
    auto addComplexManualQuery = [=](QString cmd, std::complex<double>(ManualControlDialogVD0::*get)(void)) {
       commands.push_back(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
           auto get_fn = std::bind(get, this);
           auto res = get_fn();
           return QString::number(res.real())+","+QString::number(res.imag());
       }));
    };

    addBooleanManualSetting("MANual:HSRC_CE", &ManualControlDialogVD0::setHighSourceChipEnable, &ManualControlDialogVD0::getHighSourceChipEnable);
    addBooleanManualQuery("MANual:HSRC_LOCKed", &ManualControlDialogVD0::getHighSourceLocked);
    addIntegerManualSettingWithReturnValue("MANual:HSRC_PWR", &ManualControlDialogVD0::setHighSourcePower, &ManualControlDialogVD0::getHighSourcePower);
    addDoubleManualSetting("MANual:HSRC_FREQ", &ManualControlDialogVD0::setHighSourceFrequency, &ManualControlDialogVD0::getHighSourceFrequency);
    commands.push_back(new SCPICommand("MANual:HSRC_LPF", [=](QStringList params) -> QString {
       long value;
       if(!SCPI::paramToLong(params, 0, value)) {
           return SCPI::getResultName(SCPI::Result::Error);
       }
       switch(value) {
       case 947:
           setHighSourceLPF(ManualControlDialogVD0::LPF::M947);
           break;
       case 1880:
           setHighSourceLPF(ManualControlDialogVD0::LPF::M1880);
           break;
       case 3500:
           setHighSourceLPF(ManualControlDialogVD0::LPF::M3500);
           break;
       case 0:
           setHighSourceLPF(ManualControlDialogVD0::LPF::None);
           break;
       default:
           return SCPI::getResultName(SCPI::Result::Error);
       }
       return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
       auto lpf = getHighSourceLPF();
       switch(lpf) {
       case ManualControlDialogVD0::LPF::M947: return "947";
       case ManualControlDialogVD0::LPF::M1880: return "1880";
       case ManualControlDialogVD0::LPF::M3500: return "3500";
       case ManualControlDialogVD0::LPF::None: return "0";
       default: return SCPI::getResultName(SCPI::Result::Error);
       }
    }));
    addBooleanManualSetting("MANual:LSRC_EN", &ManualControlDialogVD0::setLowSourceEnable, &ManualControlDialogVD0::getLowSourceEnable);
    addIntegerManualSettingWithReturnValue("MANual:LSRC_PWR", &ManualControlDialogVD0::setLowSourcePower, &ManualControlDialogVD0::getLowSourcePower);
    addDoubleManualSetting("MANual:LSRC_FREQ", &ManualControlDialogVD0::setLowSourceFrequency, &ManualControlDialogVD0::getLowSourceFrequency);
    addBooleanManualSetting("MANual:BAND_SW", &ManualControlDialogVD0::setHighband, &ManualControlDialogVD0::getHighband);
    addDoubleManualSetting("MANual:ATTenuator", &ManualControlDialogVD0::setAttenuator, &ManualControlDialogVD0::getAttenuator);
    addIntegerManualSettingWithReturnValue("MANual:PORT_SW", &ManualControlDialogVD0::setPortSwitch, &ManualControlDialogVD0::getPortSwitch);
    addBooleanManualSetting("MANual:LOHigh_CE", &ManualControlDialogVD0::setLOHighChipEnable, &ManualControlDialogVD0::getLOHighChipEnable);
    addBooleanManualQuery("MANual:LOHigh_LOCKed", &ManualControlDialogVD0::getLOHighLocked);
    addDoubleManualSetting("MANual:LOHigh_FREQ", &ManualControlDialogVD0::setLOHighFrequency, &ManualControlDialogVD0::getLOHighFrequency);
    addDoubleManualSetting("MANual:IFHigh_FREQ", &ManualControlDialogVD0::setIFHighFrequency, &ManualControlDialogVD0::getIFHighFrequency);
    addBooleanManualSetting("MANual:LOLow_EN", &ManualControlDialogVD0::setLOLowEnable, &ManualControlDialogVD0::getLOLowEnable);
    addDoubleManualSetting("MANual:LOLow_FREQ", &ManualControlDialogVD0::setLOLowFrequency, &ManualControlDialogVD0::getLOLowFrequency);
    addDoubleManualSetting("MANual:IFLow_FREQ", &ManualControlDialogVD0::setIFLowFrequency, &ManualControlDialogVD0::getIFLowFrequency);
    addBooleanManualSetting("MANual:PORT1_EN", &ManualControlDialogVD0::setPort1Enable, &ManualControlDialogVD0::getPort1Enable);
    addBooleanManualSetting("MANual:PORT2_EN", &ManualControlDialogVD0::setPort2Enable, &ManualControlDialogVD0::getPort2Enable);
    addBooleanManualSetting("MANual:REF_EN", &ManualControlDialogVD0::setRefEnable, &ManualControlDialogVD0::getRefEnable);
    addIntegerManualSetting("MANual:SAMPLES", &ManualControlDialogVD0::setNumSamples, &ManualControlDialogVD0::getNumSamples);
    commands.push_back(new SCPICommand("MANual:WINdow", [=](QStringList params) -> QString {
       if(params.size() < 1) {
           return SCPI::getResultName(SCPI::Result::Error);
       }
       if (params[0] == "NONE") {
           setWindow(ManualControlDialogVD0::Window::None);
       } else if(params[0] == "KAISER") {
           setWindow(ManualControlDialogVD0::Window::Kaiser);
       } else if(params[0] == "HANN") {
           setWindow(ManualControlDialogVD0::Window::Hann);
       } else if(params[0] == "FLATTOP") {
           setWindow(ManualControlDialogVD0::Window::FlatTop);
       } else {
           return "INVALID WINDOW";
       }
       return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
       switch((ManualControlDialogVD0::Window) getWindow()) {
       case ManualControlDialogVD0::Window::None: return "NONE";
       case ManualControlDialogVD0::Window::Kaiser: return "KAISER";
       case ManualControlDialogVD0::Window::Hann: return "HANN";
       case ManualControlDialogVD0::Window::FlatTop: return "FLATTOP";
       default: return SCPI::getResultName(SCPI::Result::Error);
       }
    }));
    addIntegerManualQuery("MANual:PORT1_MIN", &ManualControlDialogVD0::getPort1MinADC);
    addIntegerManualQuery("MANual:PORT1_MAX", &ManualControlDialogVD0::getPort1MaxADC);
    addDoubleManualQuery("MANual:PORT1_MAG", &ManualControlDialogVD0::getPort1Magnitude);
    addDoubleManualQuery("MANual:PORT1_PHAse", &ManualControlDialogVD0::getPort1Phase);
    addComplexManualQuery("MANual:PORT1_REFerenced", &ManualControlDialogVD0::getPort1Referenced);

    addIntegerManualQuery("MANual:PORT2_MIN", &ManualControlDialogVD0::getPort2MinADC);
    addIntegerManualQuery("MANual:PORT2_MAX", &ManualControlDialogVD0::getPort2MaxADC);
    addDoubleManualQuery("MANual:PORT2_MAG", &ManualControlDialogVD0::getPort2Magnitude);
    addDoubleManualQuery("MANual:PORT2_PHAse", &ManualControlDialogVD0::getPort2Phase);
    addComplexManualQuery("MANual:PORT2_REFerenced", &ManualControlDialogVD0::getPort2Referenced);

    addIntegerManualQuery("MANual:REF_MIN", &ManualControlDialogVD0::getRefMinADC);
    addIntegerManualQuery("MANual:REF_MAX", &ManualControlDialogVD0::getRefMaxADC);
    addDoubleManualQuery("MANual:REF_MAG", &ManualControlDialogVD0::getRefMagnitude);
    addDoubleManualQuery("MANual:REF_PHAse", &ManualControlDialogVD0::getRefPhase);

    for(auto c : commands) {
        emit dev.addSCPICommand(c);
    }

    UpdateDevice();
}

ManualControlDialogVD0::~ManualControlDialogVD0()
{
    for(auto c : commands) {
        emit dev.removeSCPICommand(c);
    }
    emit dev.releaseControl();
    delete ui;
}

void ManualControlDialogVD0::setHighSourceChipEnable(bool enable)
{
    ui->SourceCE->setChecked(enable);
}

bool ManualControlDialogVD0::getHighSourceChipEnable()
{
    return ui->SourceCE->isChecked();
}

bool ManualControlDialogVD0::getHighSourceLocked()
{
    return ui->SourceLocked->isChecked();
}

bool ManualControlDialogVD0::setHighSourcePower(int dBm)
{
    if(dBm < 0 || dBm > ui->SourceHighPower->maximum()) {
        return false;
    } else {
        ui->SourceHighPower->setValue(dBm);
        return true;
    }
}

int ManualControlDialogVD0::getHighSourcePower()
{
    return ui->SourceHighPower->value();
}

void ManualControlDialogVD0::setHighSourceFrequency(double f)
{
    ui->SourceHighFrequency->setValue(f);
}

double ManualControlDialogVD0::getHighSourceFrequency()
{
    return ui->SourceHighFrequency->value();
}

void ManualControlDialogVD0::setHighSourceLPF(ManualControlDialogVD0::LPF lpf)
{
    switch(lpf) {
    case LPF::M947:
        ui->SourceLowpass->setCurrentIndex(0);
        break;
    case LPF::M1880:
        ui->SourceLowpass->setCurrentIndex(1);
        break;
    case LPF::M3500:
        ui->SourceLowpass->setCurrentIndex(2);
        break;
    case LPF::None:
        ui->SourceLowpass->setCurrentIndex(3);
        break;
    }
}

ManualControlDialogVD0::LPF ManualControlDialogVD0::getHighSourceLPF()
{
    LPF lpfs[4] = {LPF::M947, LPF::M1880, LPF::M3500, LPF::None};
    return lpfs[ui->SourceLowpass->currentIndex()];
}

void ManualControlDialogVD0::setLowSourceEnable(bool enable)
{
    ui->SourceLowEnable->setChecked(enable);
}

bool ManualControlDialogVD0::getLowSourceEnable()
{
    return ui->SourceLowEnable->isChecked();
}

bool ManualControlDialogVD0::setLowSourcePower(int mA)
{
    switch(mA) {
    case 2:
        ui->SourceLowPower->setCurrentIndex(0);
        break;
    case 4:
        ui->SourceLowPower->setCurrentIndex(1);
        break;
    case 6:
        ui->SourceLowPower->setCurrentIndex(2);
        break;
    case 8:
        ui->SourceLowPower->setCurrentIndex(3);
        break;
    default:
        // invalid power setting
        return false;
    }
    return true;
}

int ManualControlDialogVD0::getLowSourcePower()
{
    int powers[4] = {2,4,6,8};
    return powers[ui->SourceLowPower->currentIndex()];
}

void ManualControlDialogVD0::setLowSourceFrequency(double f)
{
    ui->SourceLowFrequency->setValue(f);
}

double ManualControlDialogVD0::getLowSourceFrequency()
{
    return ui->SourceLowFrequency->value();
}

void ManualControlDialogVD0::setHighband(bool high)
{
    if(high) {
        ui->SwitchHighband->setChecked(true);
    } else {
        ui->SwitchLowband->setChecked(true);
    }
}

bool ManualControlDialogVD0::getHighband()
{
    return ui->SwitchHighband->isChecked();
}

void ManualControlDialogVD0::setAttenuator(double att)
{
    ui->Attenuator->setValue(att);
}

double ManualControlDialogVD0::getAttenuator()
{
    return ui->Attenuator->value();
}

bool ManualControlDialogVD0::setPortSwitch(int port)
{
    switch(port) {
    case 1:
        ui->Port1Switch->setChecked(true);
        break;
    case 2:
        ui->Port2Switch->setChecked(true);
        break;
    default:
        // invalid port
        return false;
    }
    return true;
}

int ManualControlDialogVD0::getPortSwitch()
{
    if(ui->Port1Switch->isChecked()) {
        return 1;
    } else {
        return 2;
    }
}

void ManualControlDialogVD0::setLOHighChipEnable(bool enable)
{
    ui->LOHighCE->setChecked(enable);
}

bool ManualControlDialogVD0::getLOHighChipEnable()
{
    return ui->LOHighCE->isChecked();
}

bool ManualControlDialogVD0::getLOHighLocked()
{
    return ui->LOLocked->isChecked();
}

void ManualControlDialogVD0::setLOHighFrequency(double f)
{
    ui->LOHighFreqType->setCurrentIndex(1);
    ui->LOHighFrequency->setValue(f);
}

double ManualControlDialogVD0::getLOHighFrequency()
{
    return ui->LOHighFrequency->value();
}

void ManualControlDialogVD0::setIFHighFrequency(double f)
{
    ui->LOHighFreqType->setCurrentIndex(0);
    ui->IFHigh->setValue(f);
}

double ManualControlDialogVD0::getIFHighFrequency()
{
    return ui->IFHigh->value();
}

void ManualControlDialogVD0::setLOLowEnable(bool enable)
{
    ui->LOLowEnable->setChecked(enable);
}

bool ManualControlDialogVD0::getLOLowEnable()
{
    return ui->LOLowEnable->isChecked();
}

void ManualControlDialogVD0::setLOLowFrequency(double f)
{
    ui->LOLowFreqType->setCurrentIndex(1);
    ui->LOLowFrequency->setValue(f);
}

double ManualControlDialogVD0::getLOLowFrequency()
{
    return ui->LOLowFrequency->value();
}

void ManualControlDialogVD0::setIFLowFrequency(double f)
{
    ui->LOLowFreqType->setCurrentIndex(0);
    ui->IFLow->setValue(f);
}

double ManualControlDialogVD0::getIFLowFrequency()
{
    return ui->IFLow->value();
}

void ManualControlDialogVD0::setPort1Enable(bool enable)
{
    ui->Port1Enable->setChecked(enable);
}

bool ManualControlDialogVD0::getPort1Enable()
{
    return ui->Port1Enable->isChecked();
}

void ManualControlDialogVD0::setPort2Enable(bool enable)
{
    ui->Port2Enable->setChecked(enable);
}

bool ManualControlDialogVD0::getPort2Enable()
{
    return ui->Port2Enable->isChecked();
}

void ManualControlDialogVD0::setRefEnable(bool enable)
{
    ui->RefEnable->setChecked(enable);
}

bool ManualControlDialogVD0::getRefEnable()
{
    return ui->RefEnable->isChecked();
}

void ManualControlDialogVD0::setNumSamples(int samples)
{
    ui->Samples->setValue(samples);
}

int ManualControlDialogVD0::getNumSamples()
{
    return ui->Samples->value();
}

void ManualControlDialogVD0::setWindow(ManualControlDialogVD0::Window w)
{
    ui->cbWindow->setCurrentIndex((int) w);
}

ManualControlDialogVD0::Window ManualControlDialogVD0::getWindow()
{
    return (Window) ui->cbWindow->currentIndex();
}

int ManualControlDialogVD0::getPort1MinADC()
{
    return ui->port1min->text().toInt();
}

int ManualControlDialogVD0::getPort1MaxADC()
{
    return ui->port1max->text().toInt();
}

double ManualControlDialogVD0::getPort1Magnitude()
{
    return ui->port1mag->text().toDouble();
}

double ManualControlDialogVD0::getPort1Phase()
{
    return ui->port1phase->text().toDouble();
}

std::complex<double> ManualControlDialogVD0::getPort1Referenced()
{
    return port1referenced;
}

int ManualControlDialogVD0::getPort2MinADC()
{
    return ui->port2min->text().toInt();
}

int ManualControlDialogVD0::getPort2MaxADC()
{
    return ui->port2max->text().toInt();
}

double ManualControlDialogVD0::getPort2Magnitude()
{
    return ui->port2mag->text().toDouble();
}

double ManualControlDialogVD0::getPort2Phase()
{
    return ui->port2phase->text().toDouble();
}

std::complex<double> ManualControlDialogVD0::getPort2Referenced()
{
    return port2referenced;
}

int ManualControlDialogVD0::getRefMinADC()
{
    return ui->refmin->text().toInt();
}

int ManualControlDialogVD0::getRefMaxADC()
{
    return ui->refmax->text().toInt();
}

double ManualControlDialogVD0::getRefMagnitude()
{
    return ui->refmag->text().toDouble();
}

double ManualControlDialogVD0::getRefPhase()
{
    return ui->refphase->text().toDouble();
}

void ManualControlDialogVD0::NewStatus(Protocol::ManualStatus status)
{
    // ADC values
    ui->port1min->setText(QString::number(status.VD0.port1min));
    ui->port1max->setText(QString::number(status.VD0.port1max));
    auto port1 = complex<double>(status.VD0.port1real, status.VD0.port1imag);
    ui->port1mag->setText(QString::number(abs(port1)));
    ui->port1phase->setText(QString::number(arg(port1)*180/M_PI));

    ui->port2min->setText(QString::number(status.VD0.port2min));
    ui->port2max->setText(QString::number(status.VD0.port2max));
    auto port2 = complex<double>(status.VD0.port2real, status.VD0.port2imag);
    ui->port2mag->setText(QString::number(abs(port2)));
    ui->port2phase->setText(QString::number(arg(port2)*180/M_PI));

    ui->refmin->setText(QString::number(status.VD0.refmin));
    ui->refmax->setText(QString::number(status.VD0.refmax));
    auto ref = complex<double>(status.VD0.refreal, status.VD0.refimag);
    ui->refmag->setText(QString::number(abs(ref)));
    ui->refphase->setText(QString::number(arg(ref)*180/M_PI));

    port1referenced = port1 / ref;
    port2referenced = port2 / ref;
    auto port1db = Util::SparamTodB(port1referenced);
    auto port2db = Util::SparamTodB(port2referenced);

    ui->port1referenced->setText(QString::number(port1db, 'f', 1) + "db@" + QString::number(arg(port1referenced)*180/M_PI, 'f', 0) + "°");
    ui->port2referenced->setText(QString::number(port2db, 'f', 1) + "db@" + QString::number(arg(port2referenced)*180/M_PI, 'f', 0) + "°");

    // PLL state
    ui->SourceLocked->setChecked(status.VD0.source_locked);
    ui->LOLocked->setChecked(status.VD0.LO_locked);
}

void ManualControlDialogVD0::UpdateDevice()
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::ManualControl;
    auto &m = p.manual.VD0;
    // Source highband
    m.SourceHighCE = ui->SourceCE->isChecked();
    m.SourceHighPower = ui->SourceHighPower->value();
    m.SourceHighFrequency = ui->SourceHighFrequency->value();
    m.SourceHighLowpass = ui->SourceLowpass->currentIndex();
    // Source lowband
    m.SourceLowEN = ui->SourceLowEnable->isChecked();
    m.SourceLowPower = ui->SourceLowPower->currentIndex();
    m.SourceLowFrequency = ui->SourceLowFrequency->value();
    // Source signal path
    m.SourceHighband = ui->SwitchHighband->isChecked();
    m.PortSwitch = ui->Port2Switch->isChecked();
    m.attenuator = -ui->Attenuator->value() / 0.25;
    // LO High
    m.LOHighCE = ui->LOHighCE->isChecked();
    m.LOHighFrequency = ui->LOHighFrequency->value();
    // LOLow
    m.LOLowEN = ui->LOLowEnable->isChecked();
    m.LOLowFrequency = ui->LOLowFrequency->value();
    m.LOLowPower = ui->LOLowPower->currentIndex();
    // LO signal path
    m.LOHighband = ui->LOSwitchHighband->isChecked();
    // Acquisition
    m.Port1EN = ui->Port1Enable->isChecked();
    m.Port2EN = ui->Port2Enable->isChecked();
    m.RefEN = ui->RefEnable->isChecked();
    m.Samples = ui->Samples->value();
    m.WindowType = ui->cbWindow->currentIndex();

    qDebug() << "Updating manual control state";

    dev.SendPacket(p);
}
