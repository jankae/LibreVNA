#include "manualcontroldialogVFD.h"

#include "ui_manualcontroldialogVFD.h"
#include "Util/util.h"

#include <QComboBox>
#include <QDebug>
#include <QButtonGroup>
#include <complex>


using namespace std;

ManualControlDialogVFD::ManualControlDialogVFD(LibreVNADriver &dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManualControlDialogVFD),
    dev(dev)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    emit dev.acquireControl();

    ui->SourceFrequency->setUnit("Hz");
    ui->SourceFrequency->setPrefixes(" MG");
    ui->SourceFrequency->setPrecision(6);
    ui->SourceFrequency->setValueQuiet(1000000000);

    ui->LOFrequency->setUnit("Hz");
    ui->LOFrequency->setPrefixes(" MG");
    ui->LOFrequency->setPrecision(6);
    ui->LOFrequency->setValueQuiet(1000000000);

    ui->DACFrequencyA->setUnit("Hz");
    ui->DACFrequencyA->setPrefixes(" kM");
    ui->DACFrequencyA->setPrecision(6);
    ui->DACFrequencyA->setValueQuiet(1000000);

    ui->DACFrequencyB->setUnit("Hz");
    ui->DACFrequencyB->setPrefixes(" kM");
    ui->DACFrequencyB->setPrecision(6);
    ui->DACFrequencyB->setValueQuiet(1000000);

    ui->DACAmplitudeA->setValue(2047);
    ui->DACAmplitudeB->setValue(2047);

    auto updateVariableAtt = [=](unsigned int value){
        ui->SourceVariableAttSlider->setValue(value);
        ui->SourceVariableAttEntry->setValueQuiet(value);
        UpdateDevice();
    };
    connect(ui->SourceVariableAttEntry, &SIUnitEdit::valueChanged, this, [=](double newval){
        updateVariableAtt(newval);
    });
    connect(ui->SourceVariableAttSlider, &QSlider::sliderMoved, this, [=](int pos){
        updateVariableAtt(pos);
    });

    // Readonly widgets
    auto MakeReadOnly = [](QWidget* w) {
        w->setAttribute(Qt::WA_TransparentForMouseEvents);
        w->setFocusPolicy(Qt::NoFocus);
    };
    MakeReadOnly(ui->SourceLocked);
    MakeReadOnly(ui->LOLocked);

//    connect(&dev, &LibreVNADriver::receivedPacket, this, [=](const Protocol::PacketInfo &p){
//            if(p.type == Protocol::PacketType::ManualStatus) {
//                NewStatus(p.manualStatus);
//            }
//    }, Qt::QueuedConnection);

    connect(ui->SourceCE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceRFEN, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LOCE, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LORFEN, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceModEn, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceAmp1En, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->SourceAmp2En, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->LOAmpEn, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });
    connect(ui->DACEnable, &QCheckBox::toggled, [=](bool) { UpdateDevice(); });

    connect(ui->SourceFilter, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) { UpdateDevice(); });
    connect(ui->SourceBandsel, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) { UpdateDevice(); });
    connect(ui->SourcePortSel, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) { UpdateDevice(); });
    connect(ui->LOMode, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) { UpdateDevice(); });

    connect(ui->SourceFrequency, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->LOFrequency, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->DACFrequencyA, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });
    connect(ui->DACFrequencyB, &SIUnitEdit::valueChanged, [=](double) { UpdateDevice(); });

    connect(ui->SourceStepAtt, qOverload<int>(&QSpinBox::valueChanged), [=](int) { UpdateDevice(); });
    connect(ui->DACAmplitudeA, qOverload<int>(&QSpinBox::valueChanged), [=](int) { UpdateDevice(); });
    connect(ui->DACAmplitudeB, qOverload<int>(&QSpinBox::valueChanged), [=](int) { UpdateDevice(); });

    // Create the SCPI commands

    auto addBooleanManualSetting = [=](QString cmd, void(ManualControlDialogVFD::*set)(bool), bool(ManualControlDialogVFD::*get)(void)) {
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

    auto addDoubleManualSetting = [=](QString cmd, void(ManualControlDialogVFD::*set)(double), double(ManualControlDialogVFD::*get)(void)) {
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
    auto addIntegerManualSetting = [=](QString cmd, void(ManualControlDialogVFD::*set)(int), int(ManualControlDialogVFD::*get)(void)) {
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
    auto addIntegerManualSettingWithReturnValue = [=](QString cmd, bool(ManualControlDialogVFD::*set)(int), int(ManualControlDialogVFD::*get)(void)) {
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
    auto addIntegerManualQuery = [=](QString cmd, int(ManualControlDialogVFD::*get)(void)) {
        commands.push_back(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
            auto get_fn = std::bind(get, this);
            return QString::number(get_fn());
        }));
    };
    auto addDoubleManualQuery = [=](QString cmd, double(ManualControlDialogVFD::*get)(void)) {
        commands.push_back(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
            auto get_fn = std::bind(get, this);
            return QString::number(get_fn());
        }));
    };
    auto addBooleanManualQuery = [=](QString cmd, bool(ManualControlDialogVFD::*get)(void)) {
        commands.push_back(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
            auto get_fn = std::bind(get, this);
            return get_fn() ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
        }));
    };
    auto addComplexManualQuery = [=](QString cmd, std::complex<double>(ManualControlDialogVFD::*get)(void)) {
        commands.push_back(new SCPICommand(cmd, nullptr, [=](QStringList) -> QString {
            auto get_fn = std::bind(get, this);
            auto res = get_fn();
            return QString::number(res.real())+","+QString::number(res.imag());
        }));
    };

    addBooleanManualSetting("MANual:SRC_CE", &ManualControlDialogVFD::setSourceChipEnable, &ManualControlDialogVFD::getSourceChipEnable);
    addBooleanManualSetting("MANual:SRC_RFEN", &ManualControlDialogVFD::setSourceRFEnable, &ManualControlDialogVFD::getSourceRFEnable);
    addBooleanManualQuery("MANual:SRC_LOCKed", &ManualControlDialogVFD::getSourceLocked);
    addDoubleManualSetting("MANual:SRC_FREQ", &ManualControlDialogVFD::setSourceFrequency, &ManualControlDialogVFD::getSourceFrequency);
    commands.push_back(new SCPICommand("MANual:SRC_LPF", [=](QStringList params) -> QString {
            long value;
            if(!SCPI::paramToLong(params, 0, value)) {
                return SCPI::getResultName(SCPI::Result::Error);
            }
            switch(value) {
            case 81:
                setSourceLPF(ManualControlDialogVFD::LPF::M81);
                break;
            case 130:
                setSourceLPF(ManualControlDialogVFD::LPF::M130);
                break;
            case 213:
                setSourceLPF(ManualControlDialogVFD::LPF::M213);
                break;
            case 345:
                setSourceLPF(ManualControlDialogVFD::LPF::M345);
                break;
            case 550:
                setSourceLPF(ManualControlDialogVFD::LPF::M550);
                break;
            case 910:
                setSourceLPF(ManualControlDialogVFD::LPF::M910);
                break;
            case 1480:
                setSourceLPF(ManualControlDialogVFD::LPF::M1480);
                break;
            case 2370:
                setSourceLPF(ManualControlDialogVFD::LPF::M2370);
                break;
            case 3840:
                setSourceLPF(ManualControlDialogVFD::LPF::M3840);
                break;
            case 6200:
                setSourceLPF(ManualControlDialogVFD::LPF::M6200);
                break;
            default:
                return SCPI::getResultName(SCPI::Result::Error);
            }
            return SCPI::getResultName(SCPI::Result::Empty);
        }, [=](QStringList) -> QString {
            auto lpf = getSourceLPF();
            switch(lpf) {
            case ManualControlDialogVFD::LPF::M81: return "81";
            case ManualControlDialogVFD::LPF::M130: return "130";
            case ManualControlDialogVFD::LPF::M213: return "213";
            case ManualControlDialogVFD::LPF::M345: return "345";
            case ManualControlDialogVFD::LPF::M550: return "550";
            case ManualControlDialogVFD::LPF::M910: return "910";
            case ManualControlDialogVFD::LPF::M1480: return "1480";
            case ManualControlDialogVFD::LPF::M2370: return "2370";
            case ManualControlDialogVFD::LPF::M3840: return "3840";
            case ManualControlDialogVFD::LPF::M6200: return "6200";
            default: return SCPI::getResultName(SCPI::Result::Error);
            }
        }));
    commands.push_back(new SCPICommand("MANual:BAND", [=](QStringList params) -> QString {
            if(params.size() != 1) {
                return SCPI::getResultName(SCPI::Result::Error);
            } else {
                if(params[0] == "HF") {
                    setBand(Band::HF);
                } else if(params[0] == "LF") {
                    setBand(Band::HF);
                } else if(params[0] == "LF_20DB") {
                    setBand(Band::HF);
                } else if(params[0] == "LF_40DB") {
                    setBand(Band::HF);
                } else {
                    return SCPI::getResultName(SCPI::Result::Error);
                }
            }
            return SCPI::getResultName(SCPI::Result::Empty);
        }, [=](QStringList) -> QString {
            auto band = getBand();
            switch(band) {
            case Band::HF: return "HF";
            case Band::LF: return "LF";
            case Band::LF_20dB: return "LF_20dB";
            case Band::LF_40dB: return "LF_40dB";
            default: return SCPI::getResultName(SCPI::Result::Error);
            }
        }));
    addDoubleManualSetting("MANual:SATTenuator", &ManualControlDialogVFD::setStepAttenuator, &ManualControlDialogVFD::getStepAttenuator);
    addDoubleManualSetting("MANual:VATTenuator", &ManualControlDialogVFD::setVariableAttenuator, &ManualControlDialogVFD::getVariableAttenuator);
    addBooleanManualSetting("MANual:AMP1_EN", &ManualControlDialogVFD::setAmplifier1Enable, &ManualControlDialogVFD::getAmplifier1Enable);
    addBooleanManualSetting("MANual:AMP2_EN", &ManualControlDialogVFD::setAmplifier2Enable, &ManualControlDialogVFD::getAmplifier2Enable);
    commands.push_back(new SCPICommand("MANual:PORT_SW", [=](QStringList params) -> QString {
            if(params.size() != 1) {
                return SCPI::getResultName(SCPI::Result::Error);
            } else {
                if(params[0] == "OFF" || params[0] == "0") {
                    setPortSwitch(PortSwitch::Off);
                } else if(params[0] == "PORT1" || params[0] == "1") {
                    setPortSwitch(PortSwitch::Port1);
                } else if(params[0] == "PORT2" || params[0] == "2") {
                    setPortSwitch(PortSwitch::Port2);
                } else {
                    return SCPI::getResultName(SCPI::Result::Error);
                }
            }
            return SCPI::getResultName(SCPI::Result::Empty);
        }, [=](QStringList) -> QString {
            auto port = getPortSwitch();
            switch(port) {
            case PortSwitch::Off: return "OFF";
            case PortSwitch::Port1: return "PORT1";
            case PortSwitch::Port2: return "PORT2";
            default: return SCPI::getResultName(SCPI::Result::Error);
            }
        }));
    addBooleanManualSetting("MANual:LO_CE", &ManualControlDialogVFD::setLOChipEnable, &ManualControlDialogVFD::getLOChipEnable);
    addBooleanManualSetting("MANual:LO_RFEN", &ManualControlDialogVFD::setLORFEnable, &ManualControlDialogVFD::getLORFEnable);
    addBooleanManualQuery("MANual:LO_LOCKed", &ManualControlDialogVFD::getLOLocked);
    addDoubleManualSetting("MANual:LO_FREQ", &ManualControlDialogVFD::setLOFrequency, &ManualControlDialogVFD::getLOFrequency);
    commands.push_back(new SCPICommand("MANual:LO_MODE", [=](QStringList params) -> QString {
            if(params.size() != 1) {
                return SCPI::getResultName(SCPI::Result::Error);
            } else {
                if(params[0] == "INTERNAL_HF") {
                    setLOMode(LOMode::InternalHF);
                } else if(params[0] == "INTERNAL_LF") {
                    setLOMode(LOMode::InternalLF);
                } else if(params[0] == "EXTERNAL") {
                    setLOMode(LOMode::External);
                } else {
                    return SCPI::getResultName(SCPI::Result::Error);
                }
            }
            return SCPI::getResultName(SCPI::Result::Empty);
        }, [=](QStringList) -> QString {
            auto mode = getLOMode();
            switch(mode) {
            case LOMode::InternalHF: return "INTERNAL_HF";
            case LOMode::InternalLF: return "INTERNAL_LF";
            case LOMode::External: return "EXTERNAL";
            default: return SCPI::getResultName(SCPI::Result::Error);
            }
        }));
    addBooleanManualSetting("MANual:LO_AMP_EN", &ManualControlDialogVFD::setLOAmplifierEnable, &ManualControlDialogVFD::getLOAmplifierEnable);

    addBooleanManualSetting("MANual:DAC_EN", &ManualControlDialogVFD::setDACEnable, &ManualControlDialogVFD::getDACEnable);
    addDoubleManualSetting("MANual:DACA_FREQ", &ManualControlDialogVFD::setDACAFrequency, &ManualControlDialogVFD::getDACAFrequency);
    addIntegerManualSetting("MANual:DACA_AMPlitude", &ManualControlDialogVFD::setDACAAmplitude, &ManualControlDialogVFD::getDACAAmplitude);
    addDoubleManualSetting("MANual:DACB_FREQ", &ManualControlDialogVFD::setDACBFrequency, &ManualControlDialogVFD::getDACBFrequency);
    addIntegerManualSetting("MANual:DACB_AMPlitude", &ManualControlDialogVFD::setDACBAmplitude, &ManualControlDialogVFD::getDACBAmplitude);

    for(auto c : commands) {
        emit dev.addSCPICommand(c);
    }

    UpdateDevice();
}

ManualControlDialogVFD::~ManualControlDialogVFD()
{
    for(auto c : commands) {
        emit dev.removeSCPICommand(c);
    }
    emit dev.releaseControl();
    delete ui;
}

void ManualControlDialogVFD::setSourceChipEnable(bool enable)
{
    ui->SourceCE->setChecked(enable);
}

bool ManualControlDialogVFD::getSourceChipEnable()
{
    return ui->SourceCE->isChecked();
}

void ManualControlDialogVFD::setSourceRFEnable(bool enable)
{
    ui->SourceRFEN->setChecked(enable);
}

bool ManualControlDialogVFD::getSourceRFEnable()
{
    return ui->SourceRFEN->isChecked();
}

bool ManualControlDialogVFD::getSourceLocked()
{
    return ui->SourceLocked->isChecked();
}

void ManualControlDialogVFD::setSourceFrequency(double f)
{
    ui->SourceFrequency->setValue(f);
}

double ManualControlDialogVFD::getSourceFrequency()
{
    return ui->SourceFrequency->value();
}

void ManualControlDialogVFD::setSourceLPF(LPF lpf)
{
    switch(lpf) {
    case LPF::M81:
        ui->SourceFilter->setCurrentIndex(0);
        break;
    case LPF::M130:
        ui->SourceFilter->setCurrentIndex(1);
        break;
    case LPF::M213:
        ui->SourceFilter->setCurrentIndex(2);
        break;
    case LPF::M345:
        ui->SourceFilter->setCurrentIndex(3);
        break;
    case LPF::M550:
        ui->SourceFilter->setCurrentIndex(4);
        break;
    case LPF::M910:
        ui->SourceFilter->setCurrentIndex(5);
        break;
    case LPF::M1480:
        ui->SourceFilter->setCurrentIndex(6);
        break;
    case LPF::M2370:
        ui->SourceFilter->setCurrentIndex(7);
        break;
    case LPF::M3840:
        ui->SourceFilter->setCurrentIndex(8);
        break;
    case LPF::M6200:
        ui->SourceFilter->setCurrentIndex(9);
        break;
    }
}

ManualControlDialogVFD::LPF ManualControlDialogVFD::getSourceLPF()
{
    LPF lpfs[10] = {LPF::M81, LPF::M130, LPF::M213, LPF::M345, LPF::M550, LPF::M910, LPF::M1480, LPF::M2370, LPF::M3840, LPF::M6200};
    return lpfs[ui->SourceFilter->currentIndex()];
}

void ManualControlDialogVFD::setStepAttenuator(double att)
{
    ui->SourceStepAtt->setValue(att);
}

double ManualControlDialogVFD::getStepAttenuator()
{
    return ui->SourceStepAtt->value();
}

void ManualControlDialogVFD::setVariableAttenuator(double voltage)
{
    ui->SourceVariableAttEntry->setValue(voltage);
}

double ManualControlDialogVFD::getVariableAttenuator()
{
    return ui->SourceVariableAttEntry->value();
}

void ManualControlDialogVFD::setAmplifier1Enable(bool enable)
{
    ui->SourceAmp1En->setChecked(enable);
}

bool ManualControlDialogVFD::getAmplifier1Enable()
{
    return ui->SourceAmp1En->isChecked();
}

void ManualControlDialogVFD::setAmplifier2Enable(bool enable)
{
    ui->SourceAmp2En->setChecked(enable);
}

bool ManualControlDialogVFD::getAmplifier2Enable()
{
    return ui->SourceAmp2En->isChecked();
}

void ManualControlDialogVFD::setBand(Band band)
{
    switch(band) {
    case Band::HF:
        ui->SourceBandsel->setCurrentIndex(0);
        break;
    case Band::LF:
        ui->SourceBandsel->setCurrentIndex(1);
        break;
    case Band::LF_20dB:
        ui->SourceBandsel->setCurrentIndex(2);
        break;
    case Band::LF_40dB:
        ui->SourceBandsel->setCurrentIndex(3);
        break;
    }
}

ManualControlDialogVFD::Band ManualControlDialogVFD::getBand()
{
    Band bands[4] = {Band::HF, Band::LF, Band::LF_20dB, Band::LF_40dB};
    return bands[ui->SourceBandsel->currentIndex()];
}

void ManualControlDialogVFD::setPortSwitch(PortSwitch port)
{
    switch(port) {
    case PortSwitch::Off:
        ui->SourcePortSel->setCurrentIndex(0);
        break;
    case PortSwitch::Port1:
        ui->SourcePortSel->setCurrentIndex(1);
        break;
    case PortSwitch::Port2:
        ui->SourcePortSel->setCurrentIndex(2);
        break;
    }
}

ManualControlDialogVFD::PortSwitch ManualControlDialogVFD::getPortSwitch()
{
    PortSwitch ports[3] = {PortSwitch::Off, PortSwitch::Port1, PortSwitch::Port2};
    return ports[ui->SourcePortSel->currentIndex()];
}

void ManualControlDialogVFD::setLOChipEnable(bool enable)
{
    ui->LOCE->setChecked(enable);
}

bool ManualControlDialogVFD::getLOChipEnable()
{
    return ui->LOCE->isChecked();
}

void ManualControlDialogVFD::setLORFEnable(bool enable)
{
    ui->LORFEN->setChecked(enable);
}

bool ManualControlDialogVFD::getLORFEnable()
{
    return ui->LORFEN->isChecked();
}

bool ManualControlDialogVFD::getLOLocked()
{
    return ui->LOLocked->isChecked();
}

void ManualControlDialogVFD::setLOFrequency(double f)
{
    ui->LOFrequency->setValue(f);
}

double ManualControlDialogVFD::getLOFrequency()
{
    return ui->LOFrequency->value();
}

void ManualControlDialogVFD::setLOMode(LOMode mode)
{
    switch(mode) {
    case LOMode::InternalHF:
        ui->LOMode->setCurrentIndex(0);
        break;
    case LOMode::InternalLF:
        ui->LOMode->setCurrentIndex(1);
        break;
    case LOMode::External:
        ui->LOMode->setCurrentIndex(2);
        break;
    }
}

ManualControlDialogVFD::LOMode ManualControlDialogVFD::getLOMode()
{
    LOMode modes[3] = {LOMode::InternalHF, LOMode::InternalLF, LOMode::External};
    return modes[ui->LOMode->currentIndex()];
}

void ManualControlDialogVFD::setLOAmplifierEnable(bool enable)
{
    ui->LOAmpEn->setChecked(enable);
}

bool ManualControlDialogVFD::getLOAmplifierEnable()
{
    return ui->LOAmpEn->isChecked();
}

void ManualControlDialogVFD::setDACEnable(bool enable)
{
    ui->DACEnable->setChecked(enable);
}

bool ManualControlDialogVFD::getDACEnable()
{
    return ui->DACEnable->isChecked();
}

void ManualControlDialogVFD::setDACAFrequency(double f)
{
    ui->DACFrequencyA->setValue(f);
}

double ManualControlDialogVFD::getDACAFrequency()
{
    return ui->DACFrequencyA->value();
}

void ManualControlDialogVFD::setDACAAmplitude(int a)
{
    ui->DACAmplitudeA->setValue(a);
}

int ManualControlDialogVFD::getDACAAmplitude()
{
    return ui->DACAmplitudeA->value();
}

void ManualControlDialogVFD::setDACBFrequency(double f)
{
    ui->DACFrequencyB->setValue(f);
}

double ManualControlDialogVFD::getDACBFrequency()
{
    return ui->DACFrequencyB->value();
}

void ManualControlDialogVFD::setDACBAmplitude(int a)
{
    ui->DACAmplitudeB->setValue(a);
}

int ManualControlDialogVFD::getDACBAmplitude()
{
    return ui->DACAmplitudeB->value();
}

void ManualControlDialogVFD::UpdateDevice()
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::ManualControl;
    auto &m = p.manual.VFD;
    // Source
    m.SourceFrequency = ui->SourceFrequency->value();
    m.SourceCE = ui->SourceCE->isChecked();
    m.SourceRFEN = ui->SourceRFEN->isChecked();
    m.SourceModEn = ui->SourceModEn->isChecked();
    m.SourceAmp1En = ui->SourceAmp1En->isChecked();
    m.SourceAmp2En = ui->SourceAmp2En->isChecked();
    m.SourceBandSel = ui->SourceBandsel->currentIndex();
    m.SourceFilter = ui->SourceFilter->currentIndex();
    m.SourceVariableAttenuator = ui->SourceVariableAttSlider->value();
    m.SourceStepAttenuator = ui->SourceStepAtt->value();
    m.SourcePortSel = ui->SourcePortSel->currentIndex();

    // LO
    m.LOCE = ui->LOCE->isChecked();
    m.LORFEN = ui->LOCE->isChecked();
    m.LOAmplifierEN = ui->LOAmpEn->isChecked();
    m.LOMode = ui->LOMode->currentIndex();
    m.LOFrequency = ui->LOFrequency->value();

    // DAC
    m.DACFreqA = ui->DACFrequencyA->value();
    m.DACFreqB = ui->DACFrequencyB->value();
    m.DACAmpA = ui->DACAmplitudeA->value();
    m.DACAmpB = ui->DACAmplitudeB->value();
    m.DACEn = ui->DACEnable->isChecked();

    qDebug() << "Updating manual control state";

    dev.SendPacket(p);
}
