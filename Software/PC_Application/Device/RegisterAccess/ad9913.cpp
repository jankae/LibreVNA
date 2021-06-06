#include "ad9913.h"
#include "ui_ad9913widget.h"
#include "register.h"

AD9913::AD9913()
{
    currentInput = nullptr;

    addRegister(new Register("CFR1", 0, 32));
    addRegister(new Register("CFR2", 1, 16));
    addRegister(new Register("DAC Control", 2, 32));
    addRegister(new Register("FTW", 3, 32));
    addRegister(new Register("POW", 4, 16));
    addRegister(new Register("Lin. Sweep Params", 6, 64));
    addRegister(new Register("Lin. Sweep Deltas", 7, 64));
    addRegister(new Register("Lin. Sweep Ramp", 8, 32));
    addRegister(new Register("Profile 0", 9, 48));
    addRegister(new Register("Profile 1", 10, 48));
    addRegister(new Register("Profile 2", 11, 48));
    addRegister(new Register("Profile 3", 12, 48));
    addRegister(new Register("Profile 4", 13, 48));
    addRegister(new Register("Profile 5", 14, 48));
    addRegister(new Register("Profile 6", 15, 48));
    addRegister(new Register("Profile 7", 16, 48));

    ui = new Ui::AD9913Widget;
    ui->setupUi(widget);

    // CFR1
    regs[0]->assignUI(ui->sineOutput, 0, 1);
    regs[0]->assignUI(ui->autoclearPhaseAcc, 1);
    regs[0]->assignUI(ui->autoclearAuxAcc, 2);
    regs[0]->assignUI(ui->loadSSR, 3);
    regs[0]->assignUI(ui->PD_input, 4);
    regs[0]->assignUI(ui->PD_DAC, 5);
    regs[0]->assignUI(ui->PD_digital, 6);
    regs[0]->assignUI(ui->PD_EXT, 7);
    regs[0]->assignUI(ui->linsweep_nodwell, 8);
    regs[0]->assignUI(ui->linsweepActiveTrigger, 9, 1);
    regs[0]->assignUI(ui->DCoutput, 10);
    regs[0]->assignUI(ui->AuxAccEnable, 11);
    regs[0]->assignUI(ui->Destination, 12, 2);
    regs[0]->assignUI(ui->clearPhaseAcc, 14);
    regs[0]->assignUI(ui->autoclearAuxAcc, 15);
    regs[0]->assignUI(ui->directSwitchMode, 16);
    regs[0]->assignUI(ui->syncClockDisable, 19);
    regs[0]->assignUI(ui->InternalProfile, 20, 3);
    regs[0]->assignUI(ui->LSBfirst, 23, 1);
    regs[0]->assignUI(ui->matchPipelineDelays, 26);
    regs[0]->assignUI(ui->UseInternalProfile, 27);
    regs[0]->assignUI(ui->enableModulus, 28);

    // CFR2
    regs[1]->assignUI(ui->PLLlock, 0);
    regs[1]->assignUI(ui->PLLreset, 1);
    regs[1]->assignUI(ui->VCO2Sel, 2, 1);
    regs[1]->assignUI(ui->PLLInputDivBy2, 3);
    regs[1]->assignUI(ui->PLLLORange, 4, 1);
    regs[1]->assignUI(ui->PLLPowerDown, 5);
    regs[1]->assignUI(ui->ClockInputMode, 6, 2);
    regs[1]->assignUI(ui->PLLMult, 9, 6);
    regs[1]->assignUI(ui->PLLOutputDivBy2, 15);

    regs[2]->assignUI(ui->FSC, 0, 10);

    regs[3]->assignUI(ui->FreqTuneWord, 0, 32);

    regs[4]->assignUI(ui->phaseOffset, 0, 14);

    regs[5]->assignUI(ui->SweepParamWord0, 0, 32);
    regs[5]->assignUI(ui->SweepParamWord1, 32, 32);

    regs[6]->assignUI(ui->RisingDeltaWord, 0, 32);
    regs[6]->assignUI(ui->FallingDeltaWord, 32, 32);

    regs[7]->assignUI(ui->RisingRamp, 0, 16);
    regs[7]->assignUI(ui->FallingRamp, 16, 16);

    regs[8]->assignUI(ui->Profile0Freq, 0, 32);
    regs[8]->assignUI(ui->Profil0Phase, 32, 14);
    regs[9]->assignUI(ui->Profile1Freq, 0, 32);
    regs[9]->assignUI(ui->Profil1Phase, 32, 14);
    regs[10]->assignUI(ui->Profile2Freq, 0, 32);
    regs[10]->assignUI(ui->Profil2Phase, 32, 14);
    regs[11]->assignUI(ui->Profile3Freq, 0, 32);
    regs[11]->assignUI(ui->Profil3Phase, 32, 14);
    regs[12]->assignUI(ui->Profile4Freq, 0, 32);
    regs[12]->assignUI(ui->Profil4Phase, 32, 14);
    regs[13]->assignUI(ui->Profile5Freq, 0, 32);
    regs[13]->assignUI(ui->Profil5Phase, 32, 14);
    regs[14]->assignUI(ui->Profile6Freq, 0, 32);
    regs[14]->assignUI(ui->Profil6Phase, 32, 14);
    regs[15]->assignUI(ui->Profile7Freq, 0, 32);
    regs[15]->assignUI(ui->Profil7Phase, 32, 14);

    Register::fillTableWidget(ui->table, regs);

    ui->ref->setPrefixes(" kMG");
    ui->ref->setUnit("Hz");
    ui->freqRef->setPrefixes(" kMG");
    ui->freqRef->setUnit("Hz");
    ui->freqDDS->setPrefixes(" kMG");
    ui->freqDDS->setUnit("Hz");
    ui->freqOUT->setPrefixes(" kMG");
    ui->freqOUT->setUnit("Hz");

    QObject::connect(ui->cbRef, &QComboBox::currentTextChanged, [=](QString input){
        SIUnitEdit *newInput = nullptr;
        if(possibleInputs.count(input)) {
            newInput = possibleInputs[input];
        }
        if(currentInput) {
            QObject::disconnect(currentInput, &SIUnitEdit::valueChanged, ui->freqRef, &SIUnitEdit::setValue);
            ui->ref->setEnabled(true);
        } else {
            QObject::disconnect(ui->ref, &SIUnitEdit::valueChanged, ui->freqRef, &SIUnitEdit::setValue);
        }
        if(newInput) {
            QObject::connect(newInput, &SIUnitEdit::valueChanged, ui->freqRef, &SIUnitEdit::setValue);
            ui->ref->setEnabled(false);
            ui->freqRef->setValue(newInput->value());
        } else {
            QObject::connect(ui->ref, &SIUnitEdit::valueChanged, ui->freqRef, &SIUnitEdit::setValue);
            ui->freqRef->setValue(ui->ref->value());
        }
        currentInput = newInput;
    });

    // user friendly frequency calculation connections
    auto updateDDS = [=]() {
        auto dds = ui->freqRef->value();
        if(!ui->PLLPowerDown->isChecked()) {
            // using the PLL
            if(ui->PLLInputDivBy2->isChecked()) {
                dds /= 2;
            }
            dds *= ui->PLLMult->currentText().toUInt();
            if(ui->PLLOutputDivBy2->isChecked()) {
                dds /= 2;
            }
        }
        ui->freqDDS->setValue(dds);
        bool valid = dds <= 250000000;
        // check value and set background
        QPalette palette;
        palette.setColor(QPalette::Base,valid ? Qt::white : Qt::red);
        ui->freqDDS->setPalette(palette);
    };
    QObject::connect(ui->freqRef, &SIUnitEdit::valueChanged, updateDDS);
    QObject::connect(ui->PLLMult, qOverload<int>(&QComboBox::currentIndexChanged), updateDDS);
    QObject::connect(ui->PLLPowerDown, &QCheckBox::toggled, updateDDS);
    QObject::connect(ui->PLLInputDivBy2, &QCheckBox::toggled, updateDDS);
    QObject::connect(ui->PLLOutputDivBy2, &QCheckBox::toggled, updateDDS);

    auto updateOutput = [=]() {
        if(ui->DCoutput->isChecked()) {
            ui->freqOUT->setValue(0);
        } else {
            unsigned int word = ui->FreqTuneWord->value();
            if(ui->UseInternalProfile->isChecked()) {
                switch(ui->InternalProfile->currentIndex()) {
                case 0: word = ui->Profile0Freq->value(); break;
                case 1: word = ui->Profile1Freq->value(); break;
                case 2: word = ui->Profile2Freq->value(); break;
                case 3: word = ui->Profile3Freq->value(); break;
                case 4: word = ui->Profile4Freq->value(); break;
                case 5: word = ui->Profile5Freq->value(); break;
                case 6: word = ui->Profile6Freq->value(); break;
                case 7: word = ui->Profile7Freq->value(); break;
                }
            }
            auto outFreq = ui->freqDDS->value() * word / (1UL << 32);
            ui->freqOUT->setValue(outFreq);
        }
    };
    QObject::connect(ui->freqDDS, &SIUnitEdit::valueChanged, updateOutput);
    QObject::connect(ui->UseInternalProfile, &QCheckBox::toggled, updateOutput);
    QObject::connect(ui->DCoutput, &QCheckBox::toggled, updateOutput);
    QObject::connect(ui->InternalProfile, qOverload<int>(&QComboBox::currentIndexChanged), updateOutput);
    QObject::connect(ui->FreqTuneWord, qOverload<int>(&QSpinBox::valueChanged), updateOutput);
    QObject::connect(ui->Profile0Freq, qOverload<int>(&QSpinBox::valueChanged), updateOutput);
    QObject::connect(ui->Profile1Freq, qOverload<int>(&QSpinBox::valueChanged), updateOutput);
    QObject::connect(ui->Profile2Freq, qOverload<int>(&QSpinBox::valueChanged), updateOutput);
    QObject::connect(ui->Profile3Freq, qOverload<int>(&QSpinBox::valueChanged), updateOutput);
    QObject::connect(ui->Profile4Freq, qOverload<int>(&QSpinBox::valueChanged), updateOutput);
    QObject::connect(ui->Profile5Freq, qOverload<int>(&QSpinBox::valueChanged), updateOutput);
    QObject::connect(ui->Profile6Freq, qOverload<int>(&QSpinBox::valueChanged), updateOutput);
    QObject::connect(ui->Profile7Freq, qOverload<int>(&QSpinBox::valueChanged), updateOutput);

    outputs["Output"] = ui->freqOUT;

    ui->ref->setValue(100000000);
}

AD9913::~AD9913()
{
    delete ui;
}

void AD9913::fromJSON(nlohmann::json j)
{
    registersFromJSON(j["registers"]);
    ui->cbRef->setCurrentText(QString::fromStdString(j["reference"]));
    if(ui->cbRef->currentText() == "Manual") {
        ui->ref->setValue(j["reference_frequency"]);
    }
}

nlohmann::json AD9913::toJSON()
{
    nlohmann::json j;
    j["registers"] = registersToJSON();
    j["reference"] = ui->cbRef->currentText().toStdString();
    if(ui->cbRef->currentText() == "Manual") {
        j["reference_frequency"] = ui->ref->value();
    }
    return j;
}

void AD9913::addPossibleInputs(RegisterDevice *inputDevice)
{
    RegisterDevice::addPossibleInputs(inputDevice);
    ui->cbRef->clear();
    ui->cbRef->addItem("Manual");
    for(auto i : possibleInputs) {
        ui->cbRef->addItem(i.first);
    }
}
