#include "manualcontroldialogV1.h"

#include "ui_manualcontroldialogV1.h"
#include "Util/util.h"

#include <QComboBox>
#include <QDebug>
#include <QButtonGroup>
#include <complex>


using namespace std;

ManualControlDialogV1::ManualControlDialogV1(LibreVNADriver &dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManualControlDialogV1),
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

    ui->IF1->setUnit("Hz");
    ui->IF1->setPrefixes(" kM");
    ui->IF1->setPrecision(6);

    ui->LO1Frequency->setUnit("Hz");
    ui->LO1Frequency->setPrefixes(" kMG");
    ui->LO1Frequency->setPrecision(6);

    ui->IF2->setUnit("Hz");
    ui->IF2->setPrefixes(" kM");
    ui->IF2->setPrecision(6);

    ui->LO2Frequency->setUnit("Hz");
    ui->LO2Frequency->setPrefixes(" kM");
    ui->LO2Frequency->setPrecision(6);

    auto UpdateLO1 = [=]() {
        double sourceFreq;
        if (ui->SwitchLowband->isChecked()) {
            sourceFreq = ui->SourceLowFrequency->value();
        } else {
            sourceFreq = ui->SourceHighFrequency->value();
        }
        if (ui->LO1FreqType->currentIndex() == 0) {
            // fixed IF mode
            ui->LO1Frequency->setValueQuiet(sourceFreq + ui->IF1->value());
        } else {
            // Manual Frequency mode
            ui->IF1->setValueQuiet(ui->LO1Frequency->value() - sourceFreq);
        }
    };
    auto UpdateLO2 = [=]() {
        double IF1 = ui->IF1->value();
        if (ui->LO2FreqType->currentIndex() == 0) {
            // fixed IF mode
            ui->LO2Frequency->setValueQuiet(IF1 + ui->IF2->value());
        } else {
            // Manual Frequency mode
            ui->IF2->setValueQuiet(ui->LO2Frequency->value() - IF1);
        }
    };

    connect(ui->IF1, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLO1();
        UpdateLO2();
    });
    connect(ui->LO1Frequency, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLO1();
        UpdateLO2();
    });
    connect(ui->IF2, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLO2();
    });
    connect(ui->LO2Frequency, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLO2();
    });
    connect(ui->SourceSwitchGroup, qOverload<int, bool>(&QButtonGroup::idToggled), [=](int, bool) {
        UpdateLO1();
        UpdateLO2();
    });
    connect(ui->SourceLowFrequency, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLO1();
        UpdateLO2();
    });
    connect(ui->SourceHighFrequency, &SIUnitEdit::valueChanged, [=](double) {
        UpdateLO1();
        UpdateLO2();
    });

    ui->IF1->setValue(60000000);
    ui->IF2->setValue(250000);

    // LO1/LO2 mode switch connections
    connect(ui->LO1FreqType, qOverload<int>(&QComboBox::activated), [=](int index) {
        switch(index) {
        case 0:
            ui->LO1Frequency->setEnabled(false);
            ui->IF1->setEnabled(true);
            break;
        case 1:
            ui->LO1Frequency->setEnabled(true);
            ui->IF1->setEnabled(false);
            break;
        }
    });
    connect(ui->LO2FreqType, qOverload<int>(&QComboBox::activated), [=](int index) {
        switch(index) {
        case 0:
            ui->LO2Frequency->setEnabled(false);
            ui->IF2->setEnabled(true);
            break;
        case 1:
            ui->LO2Frequency->setEnabled(true);
            ui->IF2->setEnabled(false);
            break;
        }
    });

    // Readonly widgets
    auto MakeReadOnly = [](QWidget* w) {
        w->setAttribute(Qt::WA_TransparentForMouseEvents);
        w->setFocusPolicy(Qt::NoFocus);
    };
    MakeReadOnly(ui->SourceLocked);
    MakeReadOnly(ui->LO1locked);
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

    connect(ui->SourceCE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceRFEN, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LO1CE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LO1RFEN, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceLowEnable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->AmplifierEnable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LO2EN, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->Port1Enable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->Port2Enable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->RefEnable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });

    connect(ui->SourceHighPower, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->SourceLowpass, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });
    connect(ui->SourceLowPower, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });

    connect(ui->SourceHighFrequency, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->SourceLowFrequency, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->LO1Frequency, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->IF1, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->LO2Frequency, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->IF2, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });

    connect(ui->PortSwitchGroup, qOverload<int, bool>(&QButtonGroup::idToggled), [=](int, bool) { UpdateDevice(); });
    connect(ui->SourceSwitchGroup, qOverload<int, bool>(&QButtonGroup::idToggled), [=](int, bool) { UpdateDevice(); });

    connect(ui->Attenuator, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double) { UpdateDevice(); });
    connect(ui->Samples, qOverload<int>(&QSpinBox::valueChanged), [=](double) { UpdateDevice(); });
    connect(ui->cbWindow, qOverload<int>(&QComboBox::activated), [=](int) { UpdateDevice(); });

    // Create the SCPI commands

    auto addBooleanManualSetting = [=](QString cmd, void(ManualControlDialogV1::*set)(bool), bool(ManualControlDialogV1::*get)(void)) {
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

    auto addDoubleManualSetting = [=](QString cmd, void(ManualControlDialogV1::*set)(double), double(ManualControlDialogV1::*get)(void)) {
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
    auto addIntegerManualSetting = [=](QString cmd, void(ManualControlDialogV1::*set)(int), int(ManualControlDialogV1::*get)(void)) {
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
    auto addIntegerManualSettingWithReturnValue = [=](QString cmd, bool(ManualControlDialogV1::*set)(int), int(ManualControlDialogV1::*get)(void)) {
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
    auto addIntegerManualQuery = [=](QString cmd, int(ManualControlDialogV1::*get)(void)) {
       commands.push_back(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
           auto get_fn = std::bind(get, this);
           return QString::number(get_fn());
       }));
    };
    auto addDoubleManualQuery = [=](QString cmd, double(ManualControlDialogV1::*get)(void)) {
       commands.push_back(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
           auto get_fn = std::bind(get, this);
           return QString::number(get_fn());
       }));
    };
    auto addBooleanManualQuery = [=](QString cmd, bool(ManualControlDialogV1::*get)(void)) {
       commands.push_back(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
           auto get_fn = std::bind(get, this);
           return get_fn() ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
       }));
    };
    auto addComplexManualQuery = [=](QString cmd, std::complex<double>(ManualControlDialogV1::*get)(void)) {
       commands.push_back(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
           auto get_fn = std::bind(get, this);
           auto res = get_fn();
           return QString::number(res.real())+","+QString::number(res.imag());
       }));
    };

    addBooleanManualSetting("MANual:HSRC_CE", &ManualControlDialogV1::setHighSourceChipEnable, &ManualControlDialogV1::getHighSourceChipEnable);
    addBooleanManualSetting("MANual:HSRC_RFEN", &ManualControlDialogV1::setHighSourceRFEnable, &ManualControlDialogV1::getHighSourceRFEnable);
    addBooleanManualQuery("MANual:HSRC_LOCKed", &ManualControlDialogV1::getHighSourceLocked);
    addIntegerManualSettingWithReturnValue("MANual:HSRC_PWR", &ManualControlDialogV1::setHighSourcePower, &ManualControlDialogV1::getHighSourcePower);
    addDoubleManualSetting("MANual:HSRC_FREQ", &ManualControlDialogV1::setHighSourceFrequency, &ManualControlDialogV1::getHighSourceFrequency);
    commands.push_back(new SCPICommand("MANual:HSRC_LPF", [=](QStringList params) -> QString {
       long value;
       if(!SCPI::paramToLong(params, 0, value)) {
           return SCPI::getResultName(SCPI::Result::Error);
       }
       switch(value) {
       case 947:
           setHighSourceLPF(ManualControlDialogV1::LPF::M947);
           break;
       case 1880:
           setHighSourceLPF(ManualControlDialogV1::LPF::M1880);
           break;
       case 3500:
           setHighSourceLPF(ManualControlDialogV1::LPF::M3500);
           break;
       case 0:
           setHighSourceLPF(ManualControlDialogV1::LPF::None);
           break;
       default:
           return SCPI::getResultName(SCPI::Result::Error);
       }
       return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
       auto lpf = getHighSourceLPF();
       switch(lpf) {
       case ManualControlDialogV1::LPF::M947: return "947";
       case ManualControlDialogV1::LPF::M1880: return "1880";
       case ManualControlDialogV1::LPF::M3500: return "3500";
       case ManualControlDialogV1::LPF::None: return "0";
       default: return SCPI::getResultName(SCPI::Result::Error);
       }
    }));
    addBooleanManualSetting("MANual:LSRC_EN", &ManualControlDialogV1::setLowSourceEnable, &ManualControlDialogV1::getLowSourceEnable);
    addIntegerManualSettingWithReturnValue("MANual:LSRC_PWR", &ManualControlDialogV1::setLowSourcePower, &ManualControlDialogV1::getLowSourcePower);
    addDoubleManualSetting("MANual:LSRC_FREQ", &ManualControlDialogV1::setLowSourceFrequency, &ManualControlDialogV1::getLowSourceFrequency);
    addBooleanManualSetting("MANual:BAND_SW", &ManualControlDialogV1::setHighband, &ManualControlDialogV1::getHighband);
    addDoubleManualSetting("MANual:ATTenuator", &ManualControlDialogV1::setAttenuator, &ManualControlDialogV1::getAttenuator);
    addBooleanManualSetting("MANual:AMP_EN", &ManualControlDialogV1::setAmplifierEnable, &ManualControlDialogV1::getAmplifierEnable);
    addIntegerManualSettingWithReturnValue("MANual:PORT_SW", &ManualControlDialogV1::setPortSwitch, &ManualControlDialogV1::getPortSwitch);
    addBooleanManualSetting("MANual:LO1_CE", &ManualControlDialogV1::setLO1ChipEnable, &ManualControlDialogV1::getLO1ChipEnable);
    addBooleanManualSetting("MANual:LO1_RFEN", &ManualControlDialogV1::setLO1RFEnable, &ManualControlDialogV1::getLO1RFEnable);
    addBooleanManualQuery("MANual:LO1_LOCKed", &ManualControlDialogV1::getLO1Locked);
    addDoubleManualSetting("MANual:LO1_FREQ", &ManualControlDialogV1::setLO1Frequency, &ManualControlDialogV1::getLO1Frequency);
    addDoubleManualSetting("MANual:IF1_FREQ", &ManualControlDialogV1::setIF1Frequency, &ManualControlDialogV1::getIF1Frequency);
    addBooleanManualSetting("MANual:LO2_EN", &ManualControlDialogV1::setLO2Enable, &ManualControlDialogV1::getLO2Enable);
    addDoubleManualSetting("MANual:LO2_FREQ", &ManualControlDialogV1::setLO2Frequency, &ManualControlDialogV1::getLO2Frequency);
    addDoubleManualSetting("MANual:IF2_FREQ", &ManualControlDialogV1::setIF2Frequency, &ManualControlDialogV1::getIF2Frequency);
    addBooleanManualSetting("MANual:PORT1_EN", &ManualControlDialogV1::setPort1Enable, &ManualControlDialogV1::getPort1Enable);
    addBooleanManualSetting("MANual:PORT2_EN", &ManualControlDialogV1::setPort2Enable, &ManualControlDialogV1::getPort2Enable);
    addBooleanManualSetting("MANual:REF_EN", &ManualControlDialogV1::setRefEnable, &ManualControlDialogV1::getRefEnable);
    addIntegerManualSetting("MANual:SAMPLES", &ManualControlDialogV1::setNumSamples, &ManualControlDialogV1::getNumSamples);
    commands.push_back(new SCPICommand("MANual:WINdow", [=](QStringList params) -> QString {
       if(params.size() < 1) {
           return SCPI::getResultName(SCPI::Result::Error);
       }
       if (params[0] == "NONE") {
           setWindow(ManualControlDialogV1::Window::None);
       } else if(params[0] == "KAISER") {
           setWindow(ManualControlDialogV1::Window::Kaiser);
       } else if(params[0] == "HANN") {
           setWindow(ManualControlDialogV1::Window::Hann);
       } else if(params[0] == "FLATTOP") {
           setWindow(ManualControlDialogV1::Window::FlatTop);
       } else {
           return "INVALID WINDOW";
       }
       return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
       switch((ManualControlDialogV1::Window) getWindow()) {
       case ManualControlDialogV1::Window::None: return "NONE";
       case ManualControlDialogV1::Window::Kaiser: return "KAISER";
       case ManualControlDialogV1::Window::Hann: return "HANN";
       case ManualControlDialogV1::Window::FlatTop: return "FLATTOP";
       default: return SCPI::getResultName(SCPI::Result::Error);
       }
    }));
    addIntegerManualQuery("MANual:PORT1_MIN", &ManualControlDialogV1::getPort1MinADC);
    addIntegerManualQuery("MANual:PORT1_MAX", &ManualControlDialogV1::getPort1MaxADC);
    addDoubleManualQuery("MANual:PORT1_MAG", &ManualControlDialogV1::getPort1Magnitude);
    addDoubleManualQuery("MANual:PORT1_PHAse", &ManualControlDialogV1::getPort1Phase);
    addComplexManualQuery("MANual:PORT1_REFerenced", &ManualControlDialogV1::getPort1Referenced);

    addIntegerManualQuery("MANual:PORT2_MIN", &ManualControlDialogV1::getPort2MinADC);
    addIntegerManualQuery("MANual:PORT2_MAX", &ManualControlDialogV1::getPort2MaxADC);
    addDoubleManualQuery("MANual:PORT2_MAG", &ManualControlDialogV1::getPort2Magnitude);
    addDoubleManualQuery("MANual:PORT2_PHAse", &ManualControlDialogV1::getPort2Phase);
    addComplexManualQuery("MANual:PORT2_REFerenced", &ManualControlDialogV1::getPort2Referenced);

    addIntegerManualQuery("MANual:REF_MIN", &ManualControlDialogV1::getRefMinADC);
    addIntegerManualQuery("MANual:REF_MAX", &ManualControlDialogV1::getRefMaxADC);
    addDoubleManualQuery("MANual:REF_MAG", &ManualControlDialogV1::getRefMagnitude);
    addDoubleManualQuery("MANual:REF_PHAse", &ManualControlDialogV1::getRefPhase);

    for(auto c : commands) {
        emit dev.addSCPICommand(c);
    }

    UpdateDevice();
}

ManualControlDialogV1::~ManualControlDialogV1()
{
    for(auto c : commands) {
        emit dev.removeSCPICommand(c);
    }
    emit dev.releaseControl();
    delete ui;
}

void ManualControlDialogV1::setHighSourceChipEnable(bool enable)
{
    ui->SourceCE->setChecked(enable);
}

bool ManualControlDialogV1::getHighSourceChipEnable()
{
    return ui->SourceCE->isChecked();
}

void ManualControlDialogV1::setHighSourceRFEnable(bool enable)
{
    ui->SourceRFEN->setChecked(enable);
}

bool ManualControlDialogV1::getHighSourceRFEnable()
{
    return ui->SourceRFEN->isChecked();
}

bool ManualControlDialogV1::getHighSourceLocked()
{
    return ui->SourceLocked->isChecked();
}

bool ManualControlDialogV1::setHighSourcePower(int dBm)
{
    switch(dBm) {
    case -4:
        ui->SourceHighPower->setCurrentIndex(0);
        break;
    case -1:
        ui->SourceHighPower->setCurrentIndex(1);
        break;
    case 2:
        ui->SourceHighPower->setCurrentIndex(2);
        break;
    case 5:
        ui->SourceHighPower->setCurrentIndex(3);
        break;
    default:
        // invalid power setting
        return false;
    }
    return true;
}

int ManualControlDialogV1::getHighSourcePower()
{
    int powers[4] = {-4,-1,2,5};
    return powers[ui->SourceHighPower->currentIndex()];
}

void ManualControlDialogV1::setHighSourceFrequency(double f)
{
    ui->SourceHighFrequency->setValue(f);
}

double ManualControlDialogV1::getHighSourceFrequency()
{
    return ui->SourceHighFrequency->value();
}

void ManualControlDialogV1::setHighSourceLPF(ManualControlDialogV1::LPF lpf)
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

ManualControlDialogV1::LPF ManualControlDialogV1::getHighSourceLPF()
{
    LPF lpfs[4] = {LPF::M947, LPF::M1880, LPF::M3500, LPF::None};
    return lpfs[ui->SourceLowpass->currentIndex()];
}

void ManualControlDialogV1::setLowSourceEnable(bool enable)
{
    ui->SourceLowEnable->setChecked(enable);
}

bool ManualControlDialogV1::getLowSourceEnable()
{
    return ui->SourceLowEnable->isChecked();
}

bool ManualControlDialogV1::setLowSourcePower(int mA)
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

int ManualControlDialogV1::getLowSourcePower()
{
    int powers[4] = {2,4,6,8};
    return powers[ui->SourceLowPower->currentIndex()];
}

void ManualControlDialogV1::setLowSourceFrequency(double f)
{
    ui->SourceLowFrequency->setValue(f);
}

double ManualControlDialogV1::getLowSourceFrequency()
{
    return ui->SourceLowFrequency->value();
}

void ManualControlDialogV1::setHighband(bool high)
{
    if(high) {
        ui->SwitchHighband->setChecked(true);
    } else {
        ui->SwitchLowband->setChecked(true);
    }
}

bool ManualControlDialogV1::getHighband()
{
    return ui->SwitchHighband->isChecked();
}

void ManualControlDialogV1::setAttenuator(double att)
{
    ui->Attenuator->setValue(att);
}

double ManualControlDialogV1::getAttenuator()
{
    return ui->Attenuator->value();
}

void ManualControlDialogV1::setAmplifierEnable(bool enable)
{
    ui->AmplifierEnable->setChecked(enable);
}

bool ManualControlDialogV1::getAmplifierEnable()
{
    return ui->AmplifierEnable->isChecked();
}

bool ManualControlDialogV1::setPortSwitch(int port)
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

int ManualControlDialogV1::getPortSwitch()
{
    if(ui->Port1Switch->isChecked()) {
        return 1;
    } else {
        return 2;
    }
}

void ManualControlDialogV1::setLO1ChipEnable(bool enable)
{
    ui->LO1CE->setChecked(enable);
}

bool ManualControlDialogV1::getLO1ChipEnable()
{
    return ui->LO1CE->isChecked();
}

void ManualControlDialogV1::setLO1RFEnable(bool enable)
{
    ui->LO1RFEN->setChecked(enable);
}

bool ManualControlDialogV1::getLO1RFEnable()
{
    return ui->LO1RFEN->isChecked();
}

bool ManualControlDialogV1::getLO1Locked()
{
    return ui->LO1locked->isChecked();
}

void ManualControlDialogV1::setLO1Frequency(double f)
{
    ui->LO1FreqType->setCurrentIndex(1);
    ui->LO1Frequency->setValue(f);
}

double ManualControlDialogV1::getLO1Frequency()
{
    return ui->LO1Frequency->value();
}

void ManualControlDialogV1::setIF1Frequency(double f)
{
    ui->LO1FreqType->setCurrentIndex(0);
    ui->IF1->setValue(f);
}

double ManualControlDialogV1::getIF1Frequency()
{
    return ui->IF1->value();
}

void ManualControlDialogV1::setLO2Enable(bool enable)
{
    ui->LO2EN->setChecked(enable);
}

bool ManualControlDialogV1::getLO2Enable()
{
    return ui->LO2EN->isChecked();
}

void ManualControlDialogV1::setLO2Frequency(double f)
{
    ui->LO2FreqType->setCurrentIndex(1);
    ui->LO2Frequency->setValue(f);
}

double ManualControlDialogV1::getLO2Frequency()
{
    return ui->LO2Frequency->value();
}

void ManualControlDialogV1::setIF2Frequency(double f)
{
    ui->LO2FreqType->setCurrentIndex(0);
    ui->IF2->setValue(f);
}

double ManualControlDialogV1::getIF2Frequency()
{
    return ui->IF2->value();
}

void ManualControlDialogV1::setPort1Enable(bool enable)
{
    ui->Port1Enable->setChecked(enable);
}

bool ManualControlDialogV1::getPort1Enable()
{
    return ui->Port1Enable->isChecked();
}

void ManualControlDialogV1::setPort2Enable(bool enable)
{
    ui->Port2Enable->setChecked(enable);
}

bool ManualControlDialogV1::getPort2Enable()
{
    return ui->Port2Enable->isChecked();
}

void ManualControlDialogV1::setRefEnable(bool enable)
{
    ui->RefEnable->setChecked(enable);
}

bool ManualControlDialogV1::getRefEnable()
{
    return ui->RefEnable->isChecked();
}

void ManualControlDialogV1::setNumSamples(int samples)
{
    ui->Samples->setValue(samples);
}

int ManualControlDialogV1::getNumSamples()
{
    return ui->Samples->value();
}

void ManualControlDialogV1::setWindow(ManualControlDialogV1::Window w)
{
    ui->cbWindow->setCurrentIndex((int) w);
}

ManualControlDialogV1::Window ManualControlDialogV1::getWindow()
{
    return (Window) ui->cbWindow->currentIndex();
}

int ManualControlDialogV1::getPort1MinADC()
{
    return ui->port1min->text().toInt();
}

int ManualControlDialogV1::getPort1MaxADC()
{
    return ui->port1max->text().toInt();
}

double ManualControlDialogV1::getPort1Magnitude()
{
    return ui->port1mag->text().toDouble();
}

double ManualControlDialogV1::getPort1Phase()
{
    return ui->port1phase->text().toDouble();
}

std::complex<double> ManualControlDialogV1::getPort1Referenced()
{
    return port1referenced;
}

int ManualControlDialogV1::getPort2MinADC()
{
    return ui->port2min->text().toInt();
}

int ManualControlDialogV1::getPort2MaxADC()
{
    return ui->port2max->text().toInt();
}

double ManualControlDialogV1::getPort2Magnitude()
{
    return ui->port2mag->text().toDouble();
}

double ManualControlDialogV1::getPort2Phase()
{
    return ui->port2phase->text().toDouble();
}

std::complex<double> ManualControlDialogV1::getPort2Referenced()
{
    return port2referenced;
}

int ManualControlDialogV1::getRefMinADC()
{
    return ui->refmin->text().toInt();
}

int ManualControlDialogV1::getRefMaxADC()
{
    return ui->refmax->text().toInt();
}

double ManualControlDialogV1::getRefMagnitude()
{
    return ui->refmag->text().toDouble();
}

double ManualControlDialogV1::getRefPhase()
{
    return ui->refphase->text().toDouble();
}

void ManualControlDialogV1::NewStatus(Protocol::ManualStatus status)
{
    // ADC values
    ui->port1min->setText(QString::number(status.V1.port1min));
    ui->port1max->setText(QString::number(status.V1.port1max));
    auto port1 = complex<double>(status.V1.port1real, status.V1.port1imag);
    ui->port1mag->setText(QString::number(abs(port1)));
    ui->port1phase->setText(QString::number(arg(port1)*180/M_PI));

    ui->port2min->setText(QString::number(status.V1.port2min));
    ui->port2max->setText(QString::number(status.V1.port2max));
    auto port2 = complex<double>(status.V1.port2real, status.V1.port2imag);
    ui->port2mag->setText(QString::number(abs(port2)));
    ui->port2phase->setText(QString::number(arg(port2)*180/M_PI));

    ui->refmin->setText(QString::number(status.V1.refmin));
    ui->refmax->setText(QString::number(status.V1.refmax));
    auto ref = complex<double>(status.V1.refreal, status.V1.refimag);
    ui->refmag->setText(QString::number(abs(ref)));
    ui->refphase->setText(QString::number(arg(ref)*180/M_PI));

    port1referenced = port1 / ref;
    port2referenced = port2 / ref;
    auto port1db = Util::SparamTodB(port1referenced);
    auto port2db = Util::SparamTodB(port2referenced);

    ui->port1referenced->setText(QString::number(port1db, 'f', 1) + "db@" + QString::number(arg(port1referenced)*180/M_PI, 'f', 0) + "°");
    ui->port2referenced->setText(QString::number(port2db, 'f', 1) + "db@" + QString::number(arg(port2referenced)*180/M_PI, 'f', 0) + "°");

    // PLL state
    ui->SourceLocked->setChecked(status.V1.source_locked);
    ui->LO1locked->setChecked(status.V1.LO_locked);
}

void ManualControlDialogV1::UpdateDevice()
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::ManualControl;
    auto &m = p.manual.V1;
    // Source highband
    m.SourceHighCE = ui->SourceCE->isChecked();
    m.SourceHighRFEN = ui->SourceRFEN->isChecked();
    m.SourceHighPower = ui->SourceHighPower->currentIndex();
    m.SourceHighFrequency = ui->SourceHighFrequency->value();
    m.SourceHighLowpass = ui->SourceLowpass->currentIndex();
    // Source lowband
    m.SourceLowEN = ui->SourceLowEnable->isChecked();
    m.SourceLowPower = ui->SourceLowPower->currentIndex();
    m.SourceLowFrequency = ui->SourceLowFrequency->value();
    // Source signal path
    m.SourceHighband = ui->SwitchHighband->isChecked();
    m.AmplifierEN = ui->AmplifierEnable->isChecked();
    m.PortSwitch = ui->Port2Switch->isChecked();
    m.attenuator = -ui->Attenuator->value() / 0.25;
    // LO1
    m.LO1CE = ui->LO1CE->isChecked();
    m.LO1RFEN = ui->LO1RFEN->isChecked();
    m.LO1Frequency = ui->LO1Frequency->value();
    // LO2
    m.LO2EN = ui->LO2EN->isChecked();
    m.LO2Frequency = ui->LO2Frequency->value();
    // Acquisition
    m.Port1EN = ui->Port1Enable->isChecked();
    m.Port2EN = ui->Port2Enable->isChecked();
    m.RefEN = ui->RefEnable->isChecked();
    m.Samples = ui->Samples->value();
    m.WindowType = ui->cbWindow->currentIndex();

    qDebug() << "Updating manual control state";

    dev.SendPacket(p);
}
