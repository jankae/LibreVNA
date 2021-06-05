#include "si5332.h"
#include "ui_si5332widget.h"
#include "register.h"

SI5332::SI5332()
{
    currentInput = nullptr;
    currentXTAL = nullptr;

    std::vector<int> addresses = {5,6,7,8,9,0xA,0xB,0xC,0xD,0xE,0xF,0x10,0x11,0x12,0x17,0x18,0x19,0x21,0x23,0x25,
                                  0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0x32,0x33,0x34,
                                  0x35,0x36,0x38,0x3A,0x3C,0x3D,0x3F,0x40,0x42,0x44,0x46,0x48,0x49,0x4B,0x4C,0x4E,
                                  0x50,0x52,0x54,0x55,0x57,0x58,0x5A,0x5C,0x5E,0x60,0x61,0x63,0x64,0x67,0x69,0x6B,
                                  0x75,0xB8,0xBE,0xBF,0xC0,0xC1,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,0x80,0x81,0x82,0x83,
                                  0x89,0x8A,0x8B,0x8C,0x8D,0x98,0x99,0x9A,0x9B,0x9C,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,
                                  0xAD,0xAE,0xAF,0xB0,0xB6,0xB7,0x73,0x24};

    for(auto addr : addresses) {
        addRegister(new Register(QString::number(addr, 16), addr, 8));
    }

    ui = new Ui::SI5332Widget;
    ui->setupUi(widget);


    // Register connections
    Register::assignUI(regs, 0x05, ui->VDD_XTAL_OK, 7);
    Register::assignUI(regs, 0x05, ui->VDDO_OK, 0); // TODO these are actually multiple flags
    Register::assignUI(regs, 0x06, ui->USYS_CTRL, 0, 2);
    Register::assignUI(regs, 0x08, ui->UDRV_OE_ENA, 0);
    Register::assignUI(regs, 0x0D, ui->DEVICE_PN_BASE, 0, 8);
    Register::assignUI(regs, 0x0E, ui->DEVICE_REV, 0, 8);
    Register::assignUI(regs, 0x0F, ui->DEVICE_GRADE, 0, 8);
    Register::assignUI(regs, 0x21, ui->I2C_ADDR, 0, 8);
    Register::assignUI(regs, 0x23, ui->I2C_SCL_PUP_ENA, 0);
    Register::assignUI(regs, 0x23, ui->I2C_SDA_PUP_ENA, 1);
    Register::assignUI(regs, 0x25, ui->OMUX0_SEL0, 0, 2);
    Register::assignUI(regs, 0x25, ui->OMUX0_SEL1, 4, 3);
    Register::assignUI(regs, 0x26, ui->OMUX1_SEL0, 0, 2);
    Register::assignUI(regs, 0x26, ui->OMUX1_SEL1, 4, 3);
    Register::assignUI(regs, 0x27, ui->OMUX2_SEL0, 0, 2);
    Register::assignUI(regs, 0x27, ui->OMUX2_SEL1, 4, 3);
    Register::assignUI(regs, 0x28, ui->OMUX3_SEL0, 0, 2);
    Register::assignUI(regs, 0x28, ui->OMUX3_SEL1, 4, 3);
    Register::assignUI(regs, 0x29, ui->OMUX4_SEL0, 0, 2);
    Register::assignUI(regs, 0x29, ui->OMUX4_SEL1, 4, 3);
    Register::assignUI(regs, 0x2A, ui->OMUX5_SEL0, 0, 2);
    Register::assignUI(regs, 0x2A, ui->OMUX5_SEL1, 4, 3);
    Register::assignUI(regs, 0x2B, ui->HSDIV0A_DIV, 0, 8);
    Register::assignUI(regs, 0x2C, ui->HSDIV0B_DIV, 0, 8);
    Register::assignUI(regs, 0x2D, ui->HSDIV1A_DIV, 0, 8);
    Register::assignUI(regs, 0x2E, ui->HSDIV1B_DIV, 0, 8);
    Register::assignUI(regs, 0x2F, ui->HSDIV2A_DIV, 0, 8);
    Register::assignUI(regs, 0x30, ui->HSDIV2B_DIV, 0, 8);
    Register::assignUI(regs, 0x31, ui->HSDIV3A_DIV, 0, 8);
    Register::assignUI(regs, 0x32, ui->HSDIV3B_DIV, 0, 8);
    Register::assignUI(regs, 0x33, ui->HSDIV4A_DIV, 0, 8);
    Register::assignUI(regs, 0x34, ui->HSDIV4B_DIV, 0, 8);
    Register::assignUI(regs, 0x35, ui->HSDIV3_DIV_SEL, 3, 1);
    Register::assignUI(regs, 0x35, ui->ID0_CFG_SEL, 6, 1);
    Register::assignUI(regs, 0x35, ui->HSDIV4_DIV_SEL, 4, 1);
    Register::assignUI(regs, 0x35, ui->ID1_CFG_SEL, 7, 1);
    Register::assignUI(regs, 0x35, ui->HSDIV2_DIV_SEL, 2, 1);
    Register::assignUI(regs, 0x35, ui->HSDIV0_DIV_SEL, 0, 1);
    Register::assignUI(regs, 0x35, ui->HSDIV1_DIV_SEL, 1, 1);

    Register::assignUI(regs, 0x36, ui->ID0A_INTG, 0, 8);
    Register::assignUI(regs, 0x37, ui->ID0A_INTG, 0, 8, 8);
    Register::assignUI(regs, 0x38, ui->ID0A_RES, 0, 8);
    Register::assignUI(regs, 0x39, ui->ID0A_RES, 0, 8, 8);
    Register::assignUI(regs, 0x3A, ui->ID0A_DEN, 0, 8);
    Register::assignUI(regs, 0x3B, ui->ID0A_DEN, 0, 8, 8);

    Register::fillTableWidget(ui->table, regs);

    ui->ref->setPrefixes(" kMG");
    ui->ref->setUnit("Hz");
    ui->xtal->setPrefixes(" kMG");
    ui->xtal->setUnit("Hz");


    QObject::connect(ui->cbRef, &QComboBox::currentTextChanged, [=](QString input){
        SIUnitEdit *newInput = nullptr;
        if(possibleInputs.count(input)) {
            newInput = possibleInputs[input];
        }
        if(currentInput) {
            QObject::disconnect(currentInput, &SIUnitEdit::valueChanged, ui->ref, &SIUnitEdit::setValue);
            ui->ref->setEnabled(true);
        }
        if(newInput) {
            QObject::connect(newInput, &SIUnitEdit::valueChanged, ui->ref, &SIUnitEdit::setValue);
            ui->ref->setEnabled(false);
            ui->ref->setValue(newInput->value());
        }
        currentInput = newInput;
    });
    QObject::connect(ui->cbXTAL, &QComboBox::currentTextChanged, [=](QString input){
        SIUnitEdit *newXTAL = nullptr;
        if(possibleInputs.count(input)) {
            newXTAL = possibleInputs[input];
        }
        if(currentXTAL) {
            QObject::disconnect(currentXTAL, &SIUnitEdit::valueChanged, ui->xtal, &SIUnitEdit::setValue);
            ui->xtal->setEnabled(true);
        }
        if(newXTAL) {
            QObject::connect(newXTAL, &SIUnitEdit::valueChanged, ui->xtal, &SIUnitEdit::setValue);
            ui->ref->setEnabled(false);
            ui->ref->setValue(newXTAL->value());
        }
        currentXTAL = newXTAL;
    });

//    // user friendly frequency calculation connections
//    auto updateDDS = [=]() {
//        auto dds = ui->freqRef->value();
//        if(!ui->PLLPowerDown->isChecked()) {
//            // using the PLL
//            if(ui->PLLInputDivBy2->isChecked()) {
//                dds /= 2;
//            }
//            dds *= ui->PLLMult->currentText().toUInt();
//            if(ui->PLLOutputDivBy2->isChecked()) {
//                dds /= 2;
//            }
//        }
//        ui->freqDDS->setValue(dds);
//        bool valid = dds <= 250000000;
//        // check value and set background
//        QPalette palette;
//        palette.setColor(QPalette::Base,valid ? Qt::white : Qt::red);
//        ui->freqDDS->setPalette(palette);
//    };
//    QObject::connect(ui->freqRef, &SIUnitEdit::valueChanged, updateDDS);
//    QObject::connect(ui->PLLMult, qOverload<int>(&QComboBox::currentIndexChanged), updateDDS);
//    QObject::connect(ui->PLLPowerDown, &QCheckBox::toggled, updateDDS);
//    QObject::connect(ui->PLLInputDivBy2, &QCheckBox::toggled, updateDDS);
//    QObject::connect(ui->PLLOutputDivBy2, &QCheckBox::toggled, updateDDS);

//    auto updateOutput = [=]() {
//        if(ui->DCoutput->isChecked()) {
//            ui->freqOUT->setValue(0);
//        } else {
//            unsigned int word = ui->FreqTuneWord->value();
//            if(ui->UseInternalProfile->isChecked()) {
//                switch(ui->InternalProfile->currentIndex()) {
//                case 0: word = ui->Profile0Freq->value(); break;
//                case 1: word = ui->Profile1Freq->value(); break;
//                case 2: word = ui->Profile2Freq->value(); break;
//                case 3: word = ui->Profile3Freq->value(); break;
//                case 4: word = ui->Profile4Freq->value(); break;
//                case 5: word = ui->Profile5Freq->value(); break;
//                case 6: word = ui->Profile6Freq->value(); break;
//                case 7: word = ui->Profile7Freq->value(); break;
//                }
//            }
//            auto outFreq = ui->freqDDS->value() * word / (1UL << 32);
//            ui->freqOUT->setValue(outFreq);
//        }
//    };
//    QObject::connect(ui->freqDDS, &SIUnitEdit::valueChanged, updateOutput);
//    QObject::connect(ui->UseInternalProfile, &QCheckBox::toggled, updateOutput);
//    QObject::connect(ui->DCoutput, &QCheckBox::toggled, updateOutput);
//    QObject::connect(ui->InternalProfile, qOverload<int>(&QComboBox::currentIndexChanged), updateOutput);
//    QObject::connect(ui->Profile0Freq, qOverload<int>(&QSpinBox::valueChanged), updateOutput);
//    QObject::connect(ui->Profile1Freq, qOverload<int>(&QSpinBox::valueChanged), updateOutput);
//    QObject::connect(ui->Profile2Freq, qOverload<int>(&QSpinBox::valueChanged), updateOutput);
//    QObject::connect(ui->Profile3Freq, qOverload<int>(&QSpinBox::valueChanged), updateOutput);
//    QObject::connect(ui->Profile4Freq, qOverload<int>(&QSpinBox::valueChanged), updateOutput);
//    QObject::connect(ui->Profile5Freq, qOverload<int>(&QSpinBox::valueChanged), updateOutput);
//    QObject::connect(ui->Profile6Freq, qOverload<int>(&QSpinBox::valueChanged), updateOutput);
//    QObject::connect(ui->Profile7Freq, qOverload<int>(&QSpinBox::valueChanged), updateOutput);

    outputs["Out 0"] = ui->freqOUT0;
    outputs["Out 1"] = ui->freqOUT1;
    outputs["Out 2"] = ui->freqOUT2;
    outputs["Out 3"] = ui->freqOUT3;
    outputs["Out 4"] = ui->freqOUT4;
    outputs["Out 5"] = ui->freqOUT5;

    ui->ref->setValue(100000000);
}

SI5332::~SI5332()
{
    delete ui;
}

void SI5332::fromJSON(nlohmann::json j)
{
    registersFromJSON(j["registers"]);
    ui->cbRef->setCurrentText(QString::fromStdString(j["reference"]));
    if(ui->cbRef->currentText() == "Manual") {
        ui->ref->setValue(j["reference_frequency"]);
    }
}

nlohmann::json SI5332::toJSON()
{
    nlohmann::json j;
    j["registers"] = registersToJSON();
    j["reference"] = ui->cbRef->currentText().toStdString();
    if(ui->cbRef->currentText() == "Manual") {
        j["reference_frequency"] = ui->ref->value();
    }
    return j;
}

void SI5332::addPossibleInputs(RegisterDevice *inputDevice)
{
    RegisterDevice::addPossibleInputs(inputDevice);
    ui->cbRef->clear();
    ui->cbXTAL->clear();
    ui->cbRef->addItem("Manual");
    ui->cbXTAL->addItem("Manual");
    for(auto i : possibleInputs) {
        ui->cbRef->addItem(i.first);
        ui->cbXTAL->addItem(i.first);
    }
}
