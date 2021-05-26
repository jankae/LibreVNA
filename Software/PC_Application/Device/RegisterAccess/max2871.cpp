#include "max2871.h"
#include "ui_max2871widget.h"
#include "register.h"

MAX2871::MAX2871()
{
    addRegister(new Register("Register 0", 0, 32));
    addRegister(new Register("Register 1", 1, 32));
    addRegister(new Register("Register 2", 2, 32));
    addRegister(new Register("Register 3", 3, 32));
    addRegister(new Register("Register 4", 4, 32));
    addRegister(new Register("Register 5", 5, 32));
    addRegister(new Register("Register 6", 6, 32));

    ui = new Ui::MAX2871Widget;
    ui->setupUi(widget);

    regs[0]->assignUI(ui->FracNEnable, 31, true);
    regs[0]->assignUI(ui->N, 15, 16);
    regs[0]->assignUI(ui->frac, 3, 12);

    regs[1]->assignUI(ui->CPL, 29, 2);
    regs[1]->assignUI(ui->CPT, 27, 2);
    regs[1]->assignUI(ui->P, 15, 12);
    regs[1]->assignUI(ui->M, 3, 12);

    regs[2]->assignUI(ui->LDS, 31, 1);
    regs[2]->assignUI(ui->SDN, 29, 2);
//    regs[2]->assignUI(ui->MUX, 26, 3); // TODO MSB in reg 5
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
//    regs[4]->assignUI(ui->BS, , ); // TODO value split in register
    regs[4]->assignUI(ui->FB, 23);
    regs[4]->assignUI(ui->DIVA, 20, 3);
    regs[4]->assignUI(ui->SDVCO, 11);
    regs[4]->assignUI(ui->MTLD, 10);
    regs[4]->assignUI(ui->BDIV, 9, 1);
    regs[4]->assignUI(ui->RFB_EN, 8);
    regs[4]->assignUI(ui->BPWR, 6, 2);
    regs[4]->assignUI(ui->RFA_EN, 5);
    regs[4]->assignUI(ui->APWR, 3, 2);

//    regs[5]->assignUI(ui->VAS_DLY, , ); // TODO connect with VAS_SHDN
    regs[5]->assignUI(ui->SDPLL, 25);
    regs[5]->assignUI(ui->F01, 24);
    regs[5]->assignUI(ui->LD, 22, 2);
//    regs[5]->assignUI(ui->MUX, , ); // TODO LSB in reg 2
    regs[5]->assignUI(ui->ADCS, 6);
    regs[5]->assignUI(ui->ADCM, 3, 3);

    regs[6]->assignUI(ui->ADC, 16, 7);

    Register::fillTableWidget(ui->table, regs);
}

MAX2871::~MAX2871()
{
    delete ui;
}

