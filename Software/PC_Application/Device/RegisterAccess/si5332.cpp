#include "si5332.h"
#include "ui_si5332widget.h"
#include "register.h"

SI5332::SI5332()
{
    currentInput = nullptr;
    currentXTAL = nullptr;

    std::vector<int> addresses = {5,6,7,8,9,0xA,0xB,0xC,0xD,0xE,0xF,0x10,0x11,0x12,0x17,0x18,0x19, 0x1A, 0x1B,0x1C,0x21,0x23,0x24,0x25,
                                  0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0x32,0x33,0x34,
                                  0x35,0x36,0x37,0x38,0x38,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,0x40,0x41,0x42,0x43,0x44,
                                  0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,0x50,0x51,0x52,0x53,0x54,
                                  0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,0x60,0x61,0x62,0x63,0x64,
                                  0x65,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x73,0x74,0x75,0x7A,0x7B,0x7C,
                                  0x7D,0x7E,0x7F,0x80,0x81,0x82,0x83,0x89,0x8A,0x8B,0x8C,0x8D,0x98,0x99,0x9A,0x9B,
                                  0x9C,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,0xC0,0xC1};

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

    // ID0
    Register::assignUI(regs, 0x36, ui->ID0A_INTG, 0, 8, 8);
    Register::assignUI(regs, 0x37, ui->ID0A_INTG, 0, 8);
    Register::assignUI(regs, 0x38, ui->ID0A_RES, 0, 8, 8);
    Register::assignUI(regs, 0x39, ui->ID0A_RES, 0, 8);
    Register::assignUI(regs, 0x3A, ui->ID0A_DEN, 0, 8, 8);
    Register::assignUI(regs, 0x3B, ui->ID0A_DEN, 0, 8);
    Register::assignUI(regs, 0x3C, ui->ID0A_SS_ENA, 0);
    Register::assignUI(regs, 0x3C, ui->ID0A_SS_MODE, 1, 2);

    // TODO check assignment of 12bit fields across three registers
    Register::assignUI(regs, 0x3D, ui->ID0A_SS_STEP_NUM, 0, 8);
    Register::assignUI(regs, 0x3E, ui->ID0A_SS_STEP_NUM, 0, 4, 8);
    Register::assignUI(regs, 0x3E, ui->ID0A_SS_STEP_INTG, 4, 4, 0);
    Register::assignUI(regs, 0x3F, ui->ID0A_SS_STEP_INTG, 0, 8, 4);

    Register::assignUI(regs, 0x40, ui->ID0A_SS_STEP_RES, 0, 8);
    Register::assignUI(regs, 0x41, ui->ID0A_SS_STEP_RES, 0, 8, 8);

    Register::assignUI(regs, 0x42, ui->ID0B_INTG, 0, 8, 8);
    Register::assignUI(regs, 0x43, ui->ID0B_INTG, 0, 8);
    Register::assignUI(regs, 0x44, ui->ID0B_RES, 0, 8, 8);
    Register::assignUI(regs, 0x45, ui->ID0B_RES, 0, 8);
    Register::assignUI(regs, 0x46, ui->ID0B_DEN, 0, 8, 8);
    Register::assignUI(regs, 0x47, ui->ID0B_DEN, 0, 8);
    Register::assignUI(regs, 0x48, ui->ID0B_SS_ENA, 0);
    Register::assignUI(regs, 0x48, ui->ID0B_SS_MODE, 1, 2);

    // TODO check assignment of 12bit fields across three registers
    Register::assignUI(regs, 0x49, ui->ID0B_SS_STEP_NUM, 0, 8);
    Register::assignUI(regs, 0x4A, ui->ID0B_SS_STEP_NUM, 0, 4, 8);
    Register::assignUI(regs, 0x4A, ui->ID0B_SS_STEP_INTG, 4, 4, 0);
    Register::assignUI(regs, 0x4B, ui->ID0B_SS_STEP_INTG, 0, 8, 4);

    Register::assignUI(regs, 0x4C, ui->ID0B_SS_STEP_RES, 0, 8);
    Register::assignUI(regs, 0x4D, ui->ID0B_SS_STEP_RES, 0, 8, 8);

    // ID1
    Register::assignUI(regs, 0x4E, ui->ID1A_INTG, 0, 8, 8);
    Register::assignUI(regs, 0x4F, ui->ID1A_INTG, 0, 8);
    Register::assignUI(regs, 0x50, ui->ID1A_RES, 0, 8, 8);
    Register::assignUI(regs, 0x51, ui->ID1A_RES, 0, 8);
    Register::assignUI(regs, 0x52, ui->ID1A_DEN, 0, 8, 8);
    Register::assignUI(regs, 0x53, ui->ID1A_DEN, 0, 8);
    Register::assignUI(regs, 0x54, ui->ID1A_SS_ENA, 0);
    Register::assignUI(regs, 0x54, ui->ID1A_SS_MODE, 1, 2);

    // TODO check assignment of 12bit fields across three registers
    Register::assignUI(regs, 0x55, ui->ID1A_SS_STEP_NUM, 0, 8);
    Register::assignUI(regs, 0x56, ui->ID1A_SS_STEP_NUM, 0, 4, 8);
    Register::assignUI(regs, 0x56, ui->ID1A_SS_STEP_INTG, 4, 4, 0);
    Register::assignUI(regs, 0x57, ui->ID1A_SS_STEP_INTG, 0, 8, 4);

    Register::assignUI(regs, 0x58, ui->ID1A_SS_STEP_RES, 0, 8);
    Register::assignUI(regs, 0x59, ui->ID1A_SS_STEP_RES, 0, 8, 8);

    Register::assignUI(regs, 0x5A, ui->ID1B_INTG, 0, 8, 8);
    Register::assignUI(regs, 0x5B, ui->ID1B_INTG, 0, 8);
    Register::assignUI(regs, 0x5C, ui->ID1B_RES, 0, 8, 8);
    Register::assignUI(regs, 0x5D, ui->ID1B_RES, 0, 8);
    Register::assignUI(regs, 0x5E, ui->ID1B_DEN, 0, 8, 8);
    Register::assignUI(regs, 0x5F, ui->ID1B_DEN, 0, 8);
    Register::assignUI(regs, 0x60, ui->ID1B_SS_ENA, 0);
    Register::assignUI(regs, 0x60, ui->ID1B_SS_MODE, 1, 2);

    // TODO check assignment of 12bit fields across three registers
    Register::assignUI(regs, 0x61, ui->ID1B_SS_STEP_NUM, 0, 8);
    Register::assignUI(regs, 0x62, ui->ID1B_SS_STEP_NUM, 0, 4, 8);
    Register::assignUI(regs, 0x62, ui->ID1B_SS_STEP_INTG, 4, 4, 0);
    Register::assignUI(regs, 0x63, ui->ID1B_SS_STEP_INTG, 0, 8, 4);

    Register::assignUI(regs, 0x64, ui->ID1B_SS_STEP_RES, 0, 8);
    Register::assignUI(regs, 0x65, ui->ID1B_SS_STEP_RES, 0, 8, 8);

    Register::assignUI(regs, 0x67, ui->IDPA_INTG, 0, 8, 8);
    Register::assignUI(regs, 0x68, ui->IDPA_INTG, 0, 8);
    Register::assignUI(regs, 0x69, ui->IDPA_RES, 0, 8, 8);
    Register::assignUI(regs, 0x6A, ui->IDPA_RES, 0, 8);
    Register::assignUI(regs, 0x6B, ui->IDPA_DEN, 0, 8, 8);
    Register::assignUI(regs, 0x6C, ui->IDPA_DEN, 0, 8);
    Register::assignUI(regs, 0x75, ui->PDIV_DIV, 0, 5);
    Register::assignUI(regs, 0xBE, ui->PLL_MODE, 2, 4);

    // OUT0
    Register::assignUI(regs, 0x7A, ui->OUT0_MODE, 0, 4);
    Register::assignUI(regs, 0x7B, ui->OUT0_DIV, 0, 6);
    Register::assignUI(regs, 0x7C, ui->OUT0_SKEW, 0, 3);
    Register::assignUI(regs, 0x7D, ui->OUT0_STOP_HIGHZ, 0, 1);
    Register::assignUI(regs, 0x7D, ui->OUT0_CMOS_INV, 4, 2);
    Register::assignUI(regs, 0x7E, ui->OUT0_CMOS_SLEW, 0, 2);
    Register::assignUI(regs, 0x7E, ui->OUT0_CMOS_STR, 2, 1);
    // OUT1
    Register::assignUI(regs, 0x7F, ui->OUT1_MODE, 0, 4);
    Register::assignUI(regs, 0x80, ui->OUT1_DIV, 0, 6);
    Register::assignUI(regs, 0x81, ui->OUT1_SKEW, 0, 3);
    Register::assignUI(regs, 0x82, ui->OUT1_STOP_HIGHZ, 0, 1);
    Register::assignUI(regs, 0x82, ui->OUT1_CMOS_INV, 4, 2);
    Register::assignUI(regs, 0x83, ui->OUT1_CMOS_SLEW, 0, 2);
    Register::assignUI(regs, 0x83, ui->OUT1_CMOS_STR, 2, 1);
    // OUT2
    Register::assignUI(regs, 0x89, ui->OUT2_MODE, 0, 4);
    Register::assignUI(regs, 0x8A, ui->OUT2_DIV, 0, 6);
    Register::assignUI(regs, 0x8B, ui->OUT2_SKEW, 0, 3);
    Register::assignUI(regs, 0x8C, ui->OUT2_STOP_HIGHZ, 0, 1);
    Register::assignUI(regs, 0x8C, ui->OUT2_CMOS_INV, 4, 2);
    Register::assignUI(regs, 0x8E, ui->OUT2_CMOS_SLEW, 0, 2);
    Register::assignUI(regs, 0x8E, ui->OUT2_CMOS_STR, 2, 1);
    // OUT3
    Register::assignUI(regs, 0x98, ui->OUT3_MODE, 0, 4);
    Register::assignUI(regs, 0x99, ui->OUT3_DIV, 0, 6);
    Register::assignUI(regs, 0x9A, ui->OUT3_SKEW, 0, 3);
    Register::assignUI(regs, 0x9B, ui->OUT3_STOP_HIGHZ, 0, 1);
    Register::assignUI(regs, 0x9B, ui->OUT3_CMOS_INV, 4, 2);
    Register::assignUI(regs, 0x9C, ui->OUT3_CMOS_SLEW, 0, 2);
    Register::assignUI(regs, 0x9C, ui->OUT3_CMOS_STR, 2, 1);
    // OUT4
    Register::assignUI(regs, 0xA7, ui->OUT4_MODE, 0, 4);
    Register::assignUI(regs, 0xA8, ui->OUT4_DIV, 0, 6);
    Register::assignUI(regs, 0xA9, ui->OUT4_SKEW, 0, 3);
    Register::assignUI(regs, 0xAA, ui->OUT4_STOP_HIGHZ, 0, 1);
    Register::assignUI(regs, 0xAA, ui->OUT4_CMOS_INV, 4, 2);
    Register::assignUI(regs, 0xAB, ui->OUT4_CMOS_SLEW, 0, 2);
    Register::assignUI(regs, 0xAB, ui->OUT4_CMOS_STR, 2, 1);
    // OUT5
    Register::assignUI(regs, 0xAC, ui->OUT5_MODE, 0, 4);
    Register::assignUI(regs, 0xAD, ui->OUT5_DIV, 0, 6);
    Register::assignUI(regs, 0xAE, ui->OUT5_SKEW, 0, 3);
    Register::assignUI(regs, 0xAF, ui->OUT5_STOP_HIGHZ, 0, 1);
    Register::assignUI(regs, 0xAF, ui->OUT5_CMOS_INV, 4, 2);
    Register::assignUI(regs, 0xB0, ui->OUT5_CMOS_SLEW, 0, 2);
    Register::assignUI(regs, 0xB0, ui->OUT5_CMOS_STR, 2, 1);

    Register::assignUI(regs, 0xB6, ui->OUT2_OE, 3);
    Register::assignUI(regs, 0xB6, ui->OUT3_OE, 6);
    Register::assignUI(regs, 0xB6, ui->OUT0_OE, 0);
    Register::assignUI(regs, 0xB6, ui->OUT1_OE, 1);
    Register::assignUI(regs, 0xB7, ui->OUT5_OE, 2);
    Register::assignUI(regs, 0xB7, ui->OUT4_OE, 1);

    Register::assignUI(regs, 0x73, ui->CLKIN_2_CLK_SEL, 0, 2);
    Register::assignUI(regs, 0x24, ui->IMUX_SEL, 0, 2);


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

    auto displayHz = [](SIUnitEdit *e) {
        e->setUnit("Hz");
        e->setPrefixes(" kMG");
        e->setPrecision(6);
    };

    displayHz(ui->ref);
    displayHz(ui->xtal);
    displayHz(ui->freqPLLref);
    displayHz(ui->freqID0);
    displayHz(ui->freqID1);
    displayHz(ui->freqVCO);
    displayHz(ui->freqOUT0);
    displayHz(ui->freqOUT1);
    displayHz(ui->freqOUT2);
    displayHz(ui->freqOUT3);
    displayHz(ui->freqOUT4);
    displayHz(ui->freqOUT5);
    displayHz(ui->freqGroup0);
    displayHz(ui->freqGroup1);
    displayHz(ui->freqGroup2);
    displayHz(ui->freqGroup3);
    displayHz(ui->freqGroup4);
    displayHz(ui->freqGroup5);
    displayHz(ui->freqHSDIV0);
    displayHz(ui->freqHSDIV1);
    displayHz(ui->freqHSDIV2);
    displayHz(ui->freqHSDIV3);
    displayHz(ui->freqHSDIV4);

    auto updatePLLRef = [=]() {
        double ref;
        switch(ui->IMUX_SEL->currentIndex()) {
        case 0: ref = 0; break;
        case 1: ref = ui->xtal->value(); break;
        case 2: ref = ui->ref->value(); break;
        case 3: ref = 0; break;
        }
        ui->freqPLLref->setValue(ref);
        bool valid = ref >= 10000000 && ref <= 50000000;
        // check value and set background
        QPalette palette;
        palette.setColor(QPalette::Base,valid ? Qt::white : Qt::red);
        ui->freqPLLref->setPalette(palette);
    };
    QObject::connect(ui->ref, &SIUnitEdit::valueChanged, updatePLLRef);
    QObject::connect(ui->xtal, &SIUnitEdit::valueChanged, updatePLLRef);
    QObject::connect(ui->IMUX_SEL, qOverload<int>(&QComboBox::currentIndexChanged), updatePLLRef);

    // user friendly frequency calculation connections
    auto updateVCO = [=]() {
        double ref;
        switch(ui->IMUX_SEL->currentIndex()) {
        case 0: ref = 0; break;
        case 1: ref = ui->xtal->value(); break;
        case 2: ref = ui->ref->value(); break;
        case 3: ref = 0; break;
        }

        ref /= ui->PDIV_DIV->value();

        int a, b, c;
        a = (ui->IDPA_INTG->value()) / 128;
        c = ui->IDPA_DEN->value();
        b = (ui->IDPA_RES->value() + (ui->IDPA_INTG->value()%128)*c) / 128;
        double mult = a + (double) b / c;

        double vco = ref * mult;

        ui->freqVCO->setValue(vco);
        bool valid = vco >= 2375000000 && vco <= 2625000000;
        // check value and set background
        QPalette palette;
        palette.setColor(QPalette::Base,valid ? Qt::white : Qt::red);
        ui->freqVCO->setPalette(palette);
    };
    QObject::connect(ui->freqPLLref, &SIUnitEdit::valueChanged, updateVCO);
    QObject::connect(ui->PDIV_DIV, qOverload<int>(&QSpinBox::valueChanged), updateVCO);
    QObject::connect(ui->IDPA_INTG, qOverload<int>(&QSpinBox::valueChanged), updateVCO);
    QObject::connect(ui->IDPA_RES, qOverload<int>(&QSpinBox::valueChanged), updateVCO);
    QObject::connect(ui->IDPA_DEN, qOverload<int>(&QSpinBox::valueChanged), updateVCO);

    auto updateID0 = [=]() {
        auto vco = ui->freqVCO->value();
        int intg, res, den;
        if(ui->ID0_CFG_SEL->currentIndex() == 0) {
            intg = ui->ID0A_INTG->value();
            res = ui->ID0A_RES->value();
            den = ui->ID0A_DEN->value();
        } else {
            intg = ui->ID0B_INTG->value();
            res = ui->ID0B_RES->value();
            den = ui->ID0B_DEN->value();
        }
        int a, b, c;
        a = (intg) / 128;
        c = den;
        b = (res + (intg%128)*c) / 128;
        double div = a + (double) b / c;
        ui->freqID0->setValue(vco / div);
    };
    QObject::connect(ui->freqVCO, &SIUnitEdit::valueChanged, updateID0);
    QObject::connect(ui->ID0_CFG_SEL, qOverload<int>(&QComboBox::currentIndexChanged), updateID0);
    QObject::connect(ui->ID0A_INTG, qOverload<int>(&QSpinBox::valueChanged), updateID0);
    QObject::connect(ui->ID0A_RES, qOverload<int>(&QSpinBox::valueChanged), updateID0);
    QObject::connect(ui->ID0A_DEN, qOverload<int>(&QSpinBox::valueChanged), updateID0);
    QObject::connect(ui->ID0B_INTG, qOverload<int>(&QSpinBox::valueChanged), updateID0);
    QObject::connect(ui->ID0B_RES, qOverload<int>(&QSpinBox::valueChanged), updateID0);
    QObject::connect(ui->ID0B_DEN, qOverload<int>(&QSpinBox::valueChanged), updateID0);

    auto updateID1 = [=]() {
        auto vco = ui->freqVCO->value();
        int intg, res, den;
        if(ui->ID1_CFG_SEL->currentIndex() == 0) {
            intg = ui->ID1A_INTG->value();
            res = ui->ID1A_RES->value();
            den = ui->ID1A_DEN->value();
        } else {
            intg = ui->ID1B_INTG->value();
            res = ui->ID1B_RES->value();
            den = ui->ID1B_DEN->value();
        }
        int a, b, c;
        a = (intg) / 128;
        c = den;
        b = (res + (intg%128)*c) / 128;
        double div = a + (double) b / c;
        ui->freqID1->setValue(vco / div);
    };
    QObject::connect(ui->freqVCO, &SIUnitEdit::valueChanged, updateID1);
    QObject::connect(ui->ID1_CFG_SEL, qOverload<int>(&QComboBox::currentIndexChanged), updateID1);
    QObject::connect(ui->ID1A_INTG, qOverload<int>(&QSpinBox::valueChanged), updateID1);
    QObject::connect(ui->ID1A_RES, qOverload<int>(&QSpinBox::valueChanged), updateID1);
    QObject::connect(ui->ID1A_DEN, qOverload<int>(&QSpinBox::valueChanged), updateID1);
    QObject::connect(ui->ID1B_INTG, qOverload<int>(&QSpinBox::valueChanged), updateID1);
    QObject::connect(ui->ID1B_RES, qOverload<int>(&QSpinBox::valueChanged), updateID1);
    QObject::connect(ui->ID1B_DEN, qOverload<int>(&QSpinBox::valueChanged), updateID1);

    auto updateHSDIV = [=](QComboBox *bankSelect, QSpinBox *divA, QSpinBox *divB, SIUnitEdit *freq) {
        int divider;
        if(bankSelect->currentIndex() == 0) {
            divider = divA->value();
        } else {
            divider = divB->value();
        }
        freq->setValue(ui->freqVCO->value() / divider);
    };
    auto updateHSDIV0 = [=]() {
        updateHSDIV(ui->HSDIV0_DIV_SEL, ui->HSDIV0A_DIV, ui->HSDIV0B_DIV, ui->freqHSDIV0);
    };
    QObject::connect(ui->freqVCO, &SIUnitEdit::valueChanged, updateHSDIV0);
    QObject::connect(ui->HSDIV0_DIV_SEL, qOverload<int>(&QComboBox::currentIndexChanged), updateHSDIV0);
    QObject::connect(ui->HSDIV0A_DIV, qOverload<int>(&QSpinBox::valueChanged), updateHSDIV0);
    QObject::connect(ui->HSDIV0B_DIV, qOverload<int>(&QSpinBox::valueChanged), updateHSDIV0);
    auto updateHSDIV1 = [=]() {
        updateHSDIV(ui->HSDIV1_DIV_SEL, ui->HSDIV1A_DIV, ui->HSDIV1B_DIV, ui->freqHSDIV1);
    };
    QObject::connect(ui->freqVCO, &SIUnitEdit::valueChanged, updateHSDIV1);
    QObject::connect(ui->HSDIV1_DIV_SEL, qOverload<int>(&QComboBox::currentIndexChanged), updateHSDIV1);
    QObject::connect(ui->HSDIV1A_DIV, qOverload<int>(&QSpinBox::valueChanged), updateHSDIV1);
    QObject::connect(ui->HSDIV1B_DIV, qOverload<int>(&QSpinBox::valueChanged), updateHSDIV1);
    auto updateHSDIV2 = [=]() {
        updateHSDIV(ui->HSDIV2_DIV_SEL, ui->HSDIV2A_DIV, ui->HSDIV2B_DIV, ui->freqHSDIV2);
    };
    QObject::connect(ui->freqVCO, &SIUnitEdit::valueChanged, updateHSDIV2);
    QObject::connect(ui->HSDIV2_DIV_SEL, qOverload<int>(&QComboBox::currentIndexChanged), updateHSDIV2);
    QObject::connect(ui->HSDIV2A_DIV, qOverload<int>(&QSpinBox::valueChanged), updateHSDIV2);
    QObject::connect(ui->HSDIV2B_DIV, qOverload<int>(&QSpinBox::valueChanged), updateHSDIV2);
    auto updateHSDIV3 = [=]() {
        updateHSDIV(ui->HSDIV3_DIV_SEL, ui->HSDIV3A_DIV, ui->HSDIV3B_DIV, ui->freqHSDIV3);
    };
    QObject::connect(ui->freqVCO, &SIUnitEdit::valueChanged, updateHSDIV3);
    QObject::connect(ui->HSDIV3_DIV_SEL, qOverload<int>(&QComboBox::currentIndexChanged), updateHSDIV3);
    QObject::connect(ui->HSDIV3A_DIV, qOverload<int>(&QSpinBox::valueChanged), updateHSDIV3);
    QObject::connect(ui->HSDIV3B_DIV, qOverload<int>(&QSpinBox::valueChanged), updateHSDIV3);
    auto updateHSDIV4 = [=]() {
        updateHSDIV(ui->HSDIV4_DIV_SEL, ui->HSDIV4A_DIV, ui->HSDIV4B_DIV, ui->freqHSDIV4);
    };
    QObject::connect(ui->freqVCO, &SIUnitEdit::valueChanged, updateHSDIV4);
    QObject::connect(ui->HSDIV4_DIV_SEL, qOverload<int>(&QComboBox::currentIndexChanged), updateHSDIV4);
    QObject::connect(ui->HSDIV4A_DIV, qOverload<int>(&QSpinBox::valueChanged), updateHSDIV4);
    QObject::connect(ui->HSDIV4B_DIV, qOverload<int>(&QSpinBox::valueChanged), updateHSDIV4);

    auto updateGroup = [=](QComboBox *sel1, QComboBox *sel0, SIUnitEdit *freq) {
        double group;
        switch(sel1->currentIndex()) {
        case 0: group = ui->freqHSDIV0->value(); break;
        case 1: group = ui->freqHSDIV1->value(); break;
        case 2: group = ui->freqHSDIV2->value(); break;
        case 3: group = ui->freqHSDIV3->value(); break;
        case 4: group = ui->freqHSDIV4->value(); break;
        case 5: group = ui->freqID0->value(); break;
        case 6: group = ui->freqID1->value(); break;
        case 7:
            switch(sel0->currentIndex()) {
            case 0: group = ui->freqPLLref->value(); break;
            case 1: group = ui->freqPLLref->value() / ui->PDIV_DIV->value(); break;
            case 2: group = ui->ref->value(); break;
            case 3: group = 0; // CLKIN_3, not available at this part
            }
            break;
        }
        freq->setValue(group);
    };
    auto updateGroups = [=]() {
        updateGroup(ui->OMUX0_SEL1, ui->OMUX0_SEL0, ui->freqGroup0);
        updateGroup(ui->OMUX1_SEL1, ui->OMUX1_SEL0, ui->freqGroup1);
        updateGroup(ui->OMUX2_SEL1, ui->OMUX2_SEL0, ui->freqGroup2);
        updateGroup(ui->OMUX3_SEL1, ui->OMUX3_SEL0, ui->freqGroup3);
        updateGroup(ui->OMUX4_SEL1, ui->OMUX4_SEL0, ui->freqGroup4);
        updateGroup(ui->OMUX5_SEL1, ui->OMUX5_SEL0, ui->freqGroup5);
    };
    QObject::connect(ui->ref, &SIUnitEdit::valueChanged, updateGroups);
    QObject::connect(ui->freqHSDIV0, &SIUnitEdit::valueChanged, updateGroups);
    QObject::connect(ui->freqHSDIV1, &SIUnitEdit::valueChanged, updateGroups);
    QObject::connect(ui->freqHSDIV2, &SIUnitEdit::valueChanged, updateGroups);
    QObject::connect(ui->freqHSDIV3, &SIUnitEdit::valueChanged, updateGroups);
    QObject::connect(ui->freqHSDIV4, &SIUnitEdit::valueChanged, updateGroups);
    QObject::connect(ui->freqID0, &SIUnitEdit::valueChanged, updateGroups);
    QObject::connect(ui->freqID1, &SIUnitEdit::valueChanged, updateGroups);
    QObject::connect(ui->freqPLLref, &SIUnitEdit::valueChanged, updateGroups);
    QObject::connect(ui->PDIV_DIV, qOverload<int>(&QSpinBox::valueChanged), updateGroups);
    QObject::connect(ui->OMUX0_SEL1, qOverload<int>(&QComboBox::currentIndexChanged), updateGroups);
    QObject::connect(ui->OMUX0_SEL0, qOverload<int>(&QComboBox::currentIndexChanged), updateGroups);
    QObject::connect(ui->OMUX1_SEL1, qOverload<int>(&QComboBox::currentIndexChanged), updateGroups);
    QObject::connect(ui->OMUX1_SEL0, qOverload<int>(&QComboBox::currentIndexChanged), updateGroups);
    QObject::connect(ui->OMUX2_SEL1, qOverload<int>(&QComboBox::currentIndexChanged), updateGroups);
    QObject::connect(ui->OMUX2_SEL0, qOverload<int>(&QComboBox::currentIndexChanged), updateGroups);
    QObject::connect(ui->OMUX3_SEL1, qOverload<int>(&QComboBox::currentIndexChanged), updateGroups);
    QObject::connect(ui->OMUX3_SEL0, qOverload<int>(&QComboBox::currentIndexChanged), updateGroups);
    QObject::connect(ui->OMUX4_SEL1, qOverload<int>(&QComboBox::currentIndexChanged), updateGroups);
    QObject::connect(ui->OMUX4_SEL0, qOverload<int>(&QComboBox::currentIndexChanged), updateGroups);
    QObject::connect(ui->OMUX5_SEL1, qOverload<int>(&QComboBox::currentIndexChanged), updateGroups);
    QObject::connect(ui->OMUX5_SEL0, qOverload<int>(&QComboBox::currentIndexChanged), updateGroups);

    auto updateOutput = [](SIUnitEdit *source, QSpinBox *div, SIUnitEdit *freq) {
        freq->setValue(source->value() / div->value());
    };
    auto updateOutputs = [=]() {
        updateOutput(ui->freqGroup0, ui->OUT0_DIV, ui->freqOUT0);
        updateOutput(ui->freqGroup1, ui->OUT1_DIV, ui->freqOUT1);
        updateOutput(ui->freqGroup2, ui->OUT2_DIV, ui->freqOUT2);
        updateOutput(ui->freqGroup3, ui->OUT3_DIV, ui->freqOUT3);
        updateOutput(ui->freqGroup4, ui->OUT4_DIV, ui->freqOUT4);
        updateOutput(ui->freqGroup5, ui->OUT5_DIV, ui->freqOUT5);
    };
    QObject::connect(ui->freqGroup0, &SIUnitEdit::valueChanged, updateOutputs);
    QObject::connect(ui->freqGroup1, &SIUnitEdit::valueChanged, updateOutputs);
    QObject::connect(ui->freqGroup2, &SIUnitEdit::valueChanged, updateOutputs);
    QObject::connect(ui->freqGroup3, &SIUnitEdit::valueChanged, updateOutputs);
    QObject::connect(ui->freqGroup4, &SIUnitEdit::valueChanged, updateOutputs);
    QObject::connect(ui->freqGroup5, &SIUnitEdit::valueChanged, updateOutputs);
    QObject::connect(ui->OUT0_DIV, qOverload<int>(&QSpinBox::valueChanged), updateOutputs);
    QObject::connect(ui->OUT1_DIV, qOverload<int>(&QSpinBox::valueChanged), updateOutputs);
    QObject::connect(ui->OUT2_DIV, qOverload<int>(&QSpinBox::valueChanged), updateOutputs);
    QObject::connect(ui->OUT3_DIV, qOverload<int>(&QSpinBox::valueChanged), updateOutputs);
    QObject::connect(ui->OUT4_DIV, qOverload<int>(&QSpinBox::valueChanged), updateOutputs);
    QObject::connect(ui->OUT5_DIV, qOverload<int>(&QSpinBox::valueChanged), updateOutputs);

    outputs["Out 0"] = ui->freqOUT0;
    outputs["Out 1"] = ui->freqOUT1;
    outputs["Out 2"] = ui->freqOUT2;
    outputs["Out 3"] = ui->freqOUT3;
    outputs["Out 4"] = ui->freqOUT4;
    outputs["Out 5"] = ui->freqOUT5;

    ui->xtal->setValue(50000000);

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
