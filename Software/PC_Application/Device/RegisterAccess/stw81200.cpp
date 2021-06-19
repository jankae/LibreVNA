#include "stw81200.h"
#include "ui_stw81200widget.h"
#include "register.h"

STW81200::STW81200()
{
    currentInput = nullptr;

    addRegister(new Register("ST0", 0, 26));
    addRegister(new Register("ST0", 1, 26));
    addRegister(new Register("ST2", 2, 26));
    addRegister(new Register("ST3", 3, 26));
    addRegister(new Register("ST4", 4, 26));
    addRegister(new Register("ST5", 5, 26));
    addRegister(new Register("ST6", 6, 26));
    addRegister(new Register("ST7", 7, 26));
    addRegister(new Register("ST8", 8, 26));
    addRegister(new Register("ST9", 9, 26));
    addRegister(new Register("ST10", 10, 26));
    addRegister(new Register("ST11", 11, 26));

    ui = new Ui::STW81200Widget;
    ui->setupUi(widget);

    regs[0]->assignUI(ui->CP_SEL, 21, 5);
    regs[0]->assignUI(ui->PFD_SEL, 19, 2);
    regs[0]->assignUI(ui->N, 0, 17);

    regs[1]->assignUI(ui->DBR_ST1, 26);
    regs[1]->assignUI(ui->RF1_OUT_PD, 24, true);
    regs[1]->assignUI(ui->RF1_DIV_SEL, 21, 3);
    regs[1]->assignUI(ui->frac, 0, 21);

    regs[2]->assignUI(ui->DBR_ST2, 26);
    regs[2]->assignUI(ui->RF2_OUT_PD, 24, true);
    regs[2]->assignUI(ui->RF2_DIV_SEL, 21, 3);
    regs[2]->assignUI(ui->MOD, 0, 21);

    regs[3]->assignUI(ui->DBR_ST3, 26);
    regs[3]->assignUI(ui->PD, 25);
    regs[3]->assignUI(ui->CP_LEAK_X2, 24);
    regs[3]->assignUI(ui->CP_LEAK, 19, 5);
    regs[3]->assignUI(ui->CP_LEAK_DIR, 18, 1);
    regs[3]->assignUI(ui->DNSPLIT_EN, 17);
    regs[3]->assignUI(ui->PFD_DEL_MODE, 15, 2);
    regs[3]->assignUI(ui->REF_PATH_SEL, 13, 2);
    regs[3]->assignUI(ui->R, 0, 12);

    regs[4]->assignUI(ui->RF_OUT_PWR, 23, 3);
    regs[4]->assignUI(ui->VCO_2V5_MODE, 22);
    regs[4]->assignUI(ui->EXT_VCO_EN, 19);
    regs[4]->assignUI(ui->VCO_AMP, 15, 4);
    regs[4]->assignUI(ui->PLL_MUX_DIV, 14, 1);
    regs[4]->assignUI(ui->CP_SUPPLY_MODE, 12, 2);
    regs[4]->assignUI(ui->KVCO_COMP_DIS, 11);
    regs[4]->assignUI(ui->PFD_POL, 10, 1);
    regs[4]->assignUI(ui->REF_BUFF_MODE, 8, 2);
    regs[4]->assignUI(ui->MUTE_LOCK_EN, 7);
    regs[4]->assignUI(ui->LD_ACTIVELOW, 6, 1);
    regs[4]->assignUI(ui->LD_PREC, 3, 3);
    regs[4]->assignUI(ui->LD_COUNT, 0, 3);

    regs[5]->assignUI(ui->VCO_BUFF_LP, 12);
    regs[5]->assignUI(ui->VCO_MUX_LP, 11);
    regs[5]->assignUI(ui->RF_DIV2_LP, 10);
    regs[5]->assignUI(ui->RF_DIV4_LP, 9);
    regs[5]->assignUI(ui->RF_DIV8_LP, 8);
    regs[5]->assignUI(ui->RF_DIV16_LP, 7);
    regs[5]->assignUI(ui->RF_DIV32_LP, 6);
    regs[5]->assignUI(ui->RF_DIV64_LP, 5);
    regs[5]->assignUI(ui->RF_DIV_MUXOUT_LP, 4);
    regs[5]->assignUI(ui->PLL_MUX_LP, 2);
    regs[5]->assignUI(ui->REF_BUFF_LP, 0);

    regs[6]->assignUI(ui->DITHERING, 26);
    regs[6]->assignUI(ui->DSM_ORDER, 22, 2);
    regs[6]->assignUI(ui->MAN_CALB_EN, 20);
    regs[6]->assignUI(ui->VCO_SEL, 18, 2);
    regs[6]->assignUI(ui->VCO_WORD, 13, 5);
    regs[6]->assignUI(ui->CAL_TEMP_COMP, 12);
    regs[6]->assignUI(ui->PRCHG_DEL, 10, 2);
    regs[6]->assignUI(ui->CAL_ACC_EN, 9);
    regs[6]->assignUI(ui->CAL_DIV, 0, 9);

    regs[7]->assignUI(ui->LD_SDO_tristate, 25);
    regs[7]->assignUI(ui->LD_SDO_MODE, 24, 1);
    regs[7]->assignUI(ui->SPI_DATA_OUT_DISABLE, 23);
    regs[7]->assignUI(ui->LD_SDO_SEL, 21, 2);
    regs[7]->assignUI(ui->CYCLE_SLIP_EN, 19);
    regs[7]->assignUI(ui->FSTLCK_EN, 18);
    regs[7]->assignUI(ui->CP_SEL_FL, 13, 5);
    regs[7]->assignUI(ui->FSTLCK_CNT, 0, 13);

    regs[8]->assignUI(ui->PD_RF2_DISABLE, 26);
    regs[8]->assignUI(ui->REG_DIG_PD, 18);
    regs[8]->assignUI(ui->REG_DIG_VOUT, 16, 2);
    regs[8]->assignUI(ui->REG_REF_PD, 14);
    regs[8]->assignUI(ui->REG_REF_VOUT, 12, 2);
    regs[8]->assignUI(ui->REG_RF_PD, 10);
    regs[8]->assignUI(ui->REG_RF_VOUT, 8, 2);
    regs[8]->assignUI(ui->REG_VCO_PD, 6);
    regs[8]->assignUI(ui->REG_VCO_VOUT, 4, 2);
    regs[8]->assignUI(ui->REG_VCO_4V5_PD, 2);
    regs[8]->assignUI(ui->REG_VCO_4V5_VOUT, 0, 2);

    regs[10]->assignUI(ui->REG_DIG_STARTUP, 17);
    regs[10]->assignUI(ui->REG_REF_STARTUP, 16);
    regs[10]->assignUI(ui->REG_RF_STARTUP, 15);
    regs[10]->assignUI(ui->REG_VCO_STARTUP, 14);
    regs[10]->assignUI(ui->REG_VCO_4V5_STARTUP, 13);
    regs[10]->assignUI(ui->REG_DIG_OCP, 12);
    regs[10]->assignUI(ui->REG_REF_OCP, 11);
    regs[10]->assignUI(ui->REG_RF_OCP, 10);
    regs[10]->assignUI(ui->REG_VCO_OCP, 9);
    regs[10]->assignUI(ui->REG_VCO_4V5_OCP, 8);
    regs[10]->assignUI(ui->LOCK_DET, 7);
    regs[10]->assignUI(ui->VCO_SEL_ST10, 5, 2);
    regs[10]->assignUI(ui->WORD, 0, 5);


    Register::fillTableWidget(ui->table, regs);

    ui->ref->setPrefixes(" kMG");
    ui->ref->setUnit("Hz");
    ui->freqRef->setPrefixes(" kMG");
    ui->freqRef->setUnit("Hz");
    ui->freqPFD->setPrefixes(" kMG");
    ui->freqPFD->setUnit("Hz");
    ui->freqVCO->setPrefixes(" kMG");
    ui->freqVCO->setUnit("Hz");
    ui->freqOutA->setPrefixes(" kMG");
    ui->freqOutA->setUnit("Hz");
    ui->freqOutB->setPrefixes(" kMG");
    ui->freqOutB->setUnit("Hz");

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
    auto updatePFD = [=]() {
        auto pfd = ui->freqRef->value();
        switch(ui->REF_PATH_SEL->currentIndex()) {
        case 0: break;
        case 1: pfd *= 2; break;
        case 2: pfd /= 2; break;
        case 3: pfd /= 4; break;
        }
        pfd /= ui->R->value();
        ui->freqPFD->setValue(pfd);
        bool valid = pfd <= 100000000;
        // check value and set background
        QPalette palette;
        palette.setColor(QPalette::Base,valid ? Qt::white : Qt::red);
        ui->freqPFD->setPalette(palette);
    };
    QObject::connect(ui->freqRef, &SIUnitEdit::valueChanged, updatePFD);
    QObject::connect(ui->REF_PATH_SEL, qOverload<int>(&QComboBox::currentIndexChanged), updatePFD);
    QObject::connect(ui->R, qOverload<int>(&QSpinBox::valueChanged), updatePFD);
    auto updateVCO = [=]() {
        auto pfd = ui->freqPFD->value();
        auto vco = pfd * ui->N->value();
        vco += pfd * (double) ui->frac->value() / ui->MOD->value();
        ui->freqVCO->setValue(vco);
        bool valid = vco >= 3000000000 && vco <= 6000000000;
        // check value and set background
        QPalette palette;
        palette.setColor(QPalette::Base,valid ? Qt::white : Qt::red);
        ui->freqVCO->setPalette(palette);
    };
    QObject::connect(ui->freqPFD, &SIUnitEdit::valueChanged, updateVCO);
    QObject::connect(ui->N, qOverload<int>(&QSpinBox::valueChanged), updateVCO);
    QObject::connect(ui->frac, qOverload<int>(&QSpinBox::valueChanged), updateVCO);
    QObject::connect(ui->MOD, qOverload<int>(&QSpinBox::valueChanged), updateVCO);
    auto updateOutA = [=]() {
        auto out_a = ui->freqVCO->value() / (1 << ui->RF1_DIV_SEL->currentIndex());
        ui->freqOutA->setValue(out_a);
    };
    QObject::connect(ui->freqVCO, &SIUnitEdit::valueChanged, updateOutA);
    QObject::connect(ui->RF1_DIV_SEL, qOverload<int>(&QComboBox::currentIndexChanged), updateOutA);
    auto updateOutB = [=]() {
        auto out_b = ui->freqVCO->value();
        int div = 0;
        if(ui->RF2_DIV_SEL->currentIndex() == 7) {
            // use divider from Out 1
            div = 1 << ui->RF1_DIV_SEL->currentIndex();
        } else {
            div = 1 << ui->RF2_DIV_SEL->currentIndex();
        }
        out_b /= div;
        ui->freqOutB->setValue(out_b);
    };
    QObject::connect(ui->freqVCO, &SIUnitEdit::valueChanged, updateOutB);
    QObject::connect(ui->RF1_DIV_SEL, qOverload<int>(&QComboBox::currentIndexChanged), updateOutB);
    QObject::connect(ui->RF2_DIV_SEL, qOverload<int>(&QComboBox::currentIndexChanged), updateOutB);

    outputs["OutA"] = ui->freqOutA;
    outputs["OutB"] = ui->freqOutB;

    ui->ref->setValue(100000000);
}

STW81200::~STW81200()
{
    delete ui;
}

void STW81200::fromJSON(nlohmann::json j)
{
    registersFromJSON(j["registers"]);
    ui->cbRef->setCurrentText(QString::fromStdString(j["reference"]));
    if(ui->cbRef->currentText() == "Manual") {
        ui->ref->setValue(j["reference_frequency"]);
    }
}

nlohmann::json STW81200::toJSON()
{
    nlohmann::json j;
    j["registers"] = registersToJSON();
    j["reference"] = ui->cbRef->currentText().toStdString();
    if(ui->cbRef->currentText() == "Manual") {
        j["reference_frequency"] = ui->ref->value();
    }
    return j;
}

void STW81200::addPossibleInputs(RegisterDevice *inputDevice)
{
    RegisterDevice::addPossibleInputs(inputDevice);
    ui->cbRef->clear();
    ui->cbRef->addItem("Manual");
    for(auto i : possibleInputs) {
        ui->cbRef->addItem(i.first);
    }
}
