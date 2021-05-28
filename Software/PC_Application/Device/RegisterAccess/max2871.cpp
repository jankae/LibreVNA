#include "max2871.h"
#include "ui_max2871widget.h"
#include "register.h"

MAX2871::MAX2871()
{
    currentInput = nullptr;

    addRegister(new Register("Register 0", 0, 32));
    addRegister(new Register("Register 1", 1, 32));
    addRegister(new Register("Register 2", 2, 32));
    addRegister(new Register("Register 3", 3, 32));
    addRegister(new Register("Register 4", 4, 32));
    addRegister(new Register("Register 5", 5, 32));

    ui = new Ui::MAX2871Widget;
    ui->setupUi(widget);

    regs[0]->assignUI(ui->INT, 31);
    regs[0]->assignUI(ui->N, 15, 16);
    regs[0]->assignUI(ui->frac, 3, 12);

    regs[1]->assignUI(ui->CPL, 29, 2);
    regs[1]->assignUI(ui->CPT, 27, 2);
    regs[1]->assignUI(ui->P, 15, 12);
    regs[1]->assignUI(ui->M, 3, 12);

    regs[2]->assignUI(ui->LDS, 31, 1);
    regs[2]->assignUI(ui->SDN, 29, 2);
    regs[2]->assignUI(ui->MUX, 26, 3);
    regs[2]->assignUI(ui->DBR, 25);
    regs[2]->assignUI(ui->RDIV2, 24);
    regs[2]->assignUI(ui->R, 14, 10);
    regs[2]->assignUI(ui->REG4DB, 13);
    regs[2]->assignUI(ui->CP, 9, 4);
    regs[2]->assignUI(ui->LDF, 8, 1);
    regs[2]->assignUI(ui->LDP, 7, 1);
    regs[2]->assignUI(ui->PDP, 6, 1);
    regs[2]->assignUI(ui->SHDN, 5);
    regs[2]->assignUI(ui->TRI, 4);
    regs[2]->assignUI(ui->RST, 3);

    regs[3]->assignUI(ui->VCO, 26, 6);
    regs[3]->assignUI(ui->VAS_SHDN, 25);
    regs[3]->assignUI(ui->VAS_TEMP, 24);
    regs[3]->assignUI(ui->CSM, 18);
    regs[3]->assignUI(ui->MUTEDEL, 17);
    regs[3]->assignUI(ui->CDM, 15, 2);
    regs[3]->assignUI(ui->CDIV, 3, 12);

    regs[4]->assignUI(ui->SDLDO, 28);
    regs[4]->assignUI(ui->SDDIV, 27);
    regs[4]->assignUI(ui->SDREF, 26);
    regs[4]->assignUI(ui->BS, 24, 2, 8);
    regs[4]->assignUI(ui->FB, 23);
    regs[4]->assignUI(ui->DIVA, 20, 3);
    regs[4]->assignUI(ui->BS, 12, 8);
    regs[4]->assignUI(ui->SDVCO, 11);
    regs[4]->assignUI(ui->MTLD, 10);
    regs[4]->assignUI(ui->BDIV, 9, 1);
    regs[4]->assignUI(ui->RFB_EN, 8);
    regs[4]->assignUI(ui->BPWR, 6, 2);
    regs[4]->assignUI(ui->RFA_EN, 5);
    regs[4]->assignUI(ui->APWR, 3, 2);

    regs[5]->assignUI(ui->VAS_DLY, 29, 2);
    regs[5]->assignUI(ui->SDPLL, 25);
    regs[5]->assignUI(ui->F01, 24);
    regs[5]->assignUI(ui->LD, 22, 2);
    regs[5]->assignUI(ui->MUX, 18, 1, 3);
    regs[5]->assignUI(ui->ADCS, 6);
    regs[5]->assignUI(ui->ADCM, 3, 3);

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
        if(ui->DBR->isChecked()) {
            pfd *= 2;
        }
        pfd /= ui->R->value();
        if(ui->RDIV2->isChecked()) {
            pfd /= 2;
        }
        ui->freqPFD->setValue(pfd);
        bool valid = pfd <= 125000000 || (pfd <= 140000000 && ui->INT->isChecked());
        // check value and set background
        QPalette palette;
        palette.setColor(QPalette::Base,valid ? Qt::white : Qt::red);
        ui->freqPFD->setPalette(palette);
    };
    QObject::connect(ui->freqRef, &SIUnitEdit::valueChanged, updatePFD);
    QObject::connect(ui->INT, &QCheckBox::toggled, updatePFD);
    QObject::connect(ui->DBR, &QCheckBox::toggled, updatePFD);
    QObject::connect(ui->RDIV2, &QCheckBox::toggled, updatePFD);
    QObject::connect(ui->R, qOverload<int>(&QSpinBox::valueChanged), updatePFD);
    auto updateVCO = [=]() {
        auto pfd = ui->freqPFD->value();
        auto vco = pfd * ui->N->value();
        if(!ui->INT->isChecked()){
            vco += pfd * (double) ui->frac->value() / ui->M->value();
        }
        if(!ui->FB->isChecked()) {
            // using divided down vco output
            // multiply by divider ratio (but only up to 16)
            auto mult = 1UL << ui->DIVA->currentIndex();
            if(mult > 16) {
                mult = 16;
            }
            vco *= mult;
        }
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
    QObject::connect(ui->FB, &QCheckBox::toggled, updateVCO);
    QObject::connect(ui->INT, &QCheckBox::toggled, updateVCO);
    QObject::connect(ui->M, qOverload<int>(&QSpinBox::valueChanged), updateVCO);
    auto updateOutA = [=]() {
        auto out_a = ui->freqVCO->value() / (1 << ui->DIVA->currentIndex());
        ui->freqOutA->setValue(out_a);
    };
    QObject::connect(ui->freqVCO, &SIUnitEdit::valueChanged, updateOutA);
    QObject::connect(ui->DIVA, qOverload<int>(&QComboBox::currentIndexChanged), updateOutA);
    auto updateOutB = [=]() {
        auto out_b = ui->freqVCO->value();
        if(ui->BDIV->currentIndex() == 0) {
            out_b /= (1 << ui->DIVA->currentIndex());
        }
        ui->freqOutB->setValue(out_b);
    };
    QObject::connect(ui->freqVCO, &SIUnitEdit::valueChanged, updateOutB);
    QObject::connect(ui->DIVA, qOverload<int>(&QComboBox::currentIndexChanged), updateOutB);
    QObject::connect(ui->BDIV, qOverload<int>(&QComboBox::currentIndexChanged), updateOutB);

    outputs["OutA"] = ui->freqOutA;
    outputs["OutB"] = ui->freqOutB;

    ui->ref->setValue(100000000);
}

MAX2871::~MAX2871()
{
    delete ui;
}

void MAX2871::fromJSON(nlohmann::json j)
{
    registersFromJSON(j["registers"]);
    ui->cbRef->setCurrentText(QString::fromStdString(j["reference"]));
    if(ui->cbRef->currentText() == "Manual") {
        ui->ref->setValue(j["reference_frequency"]);
    }
}

nlohmann::json MAX2871::toJSON()
{
    nlohmann::json j;
    j["registers"] = registersToJSON();
    j["reference"] = ui->cbRef->currentText().toStdString();
    if(ui->cbRef->currentText() == "Manual") {
        j["reference_frequency"] = ui->ref->value();
    }
    return j;
}

void MAX2871::addPossibleInputs(RegisterDevice *inputDevice)
{
    RegisterDevice::addPossibleInputs(inputDevice);
    ui->cbRef->clear();
    ui->cbRef->addItem("Manual");
    for(auto i : possibleInputs) {
        ui->cbRef->addItem(i.first);
    }
}
