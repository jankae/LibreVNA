#include "impedancematchdialog.h"
#include "ui_impedancematchdialog.h"
#include "Tools/eseries.h"
#include "unit.h"

using namespace std;

constexpr double ImpedanceMatchDialog::Z0;

ImpedanceMatchDialog::ImpedanceMatchDialog(TraceMarkerModel &model, TraceMarker *marker, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImpedanceMatchDialog)
{
    ui->setupUi(this);

    // set SI units and prefixes
    ui->zReal->setUnit("Ohm");
    ui->zImag->setUnit("Ohm");
    ui->zFreq->setUnit("Hz");
    ui->zFreq->setPrefixes(" kMG");

    ui->mImag->setUnit("Ohm");
    ui->mReal->setUnit("Ohm");
    ui->mLoss->setUnit("db");

    ui->lValue->setUnit("H");
    ui->lValue->setPrefixes("pnum ");
    ui->cValue->setUnit("F");
    ui->cValue->setPrefixes("pnum ");

    connect(ui->zFreq, &SIUnitEdit::valueChanged, this, &ImpedanceMatchDialog::calculateMatch);
    connect(ui->zImag, &SIUnitEdit::valueChanged, this, &ImpedanceMatchDialog::calculateMatch);
    connect(ui->zReal, &SIUnitEdit::valueChanged, this, &ImpedanceMatchDialog::calculateMatch);
    connect(ui->zGroup, qOverload<int>(&QButtonGroup::buttonClicked), this, &ImpedanceMatchDialog::calculateMatch);
    connect(ui->cMatchType, qOverload<int>(&QComboBox::currentIndexChanged), this, &ImpedanceMatchDialog::calculateMatch);
    connect(ui->lGroup, qOverload<int>(&QButtonGroup::buttonClicked), this, &ImpedanceMatchDialog::calculateMatch);
    connect(ui->cGroup, qOverload<int>(&QButtonGroup::buttonClicked), this, &ImpedanceMatchDialog::calculateMatch);
    connect(ui->zGroup, qOverload<int>(&QButtonGroup::buttonClicked), this, &ImpedanceMatchDialog::calculateMatch);

    // populate marker options
    auto markers = model.getMarkers();
    for(auto m : markers) {
        if(!m->trace()->isReflection()) {
            // matching only possible for reflections
            continue;
        }
        ui->cSource->addItem("From Marker "+QString::number(m->getNumber()), QVariant::fromValue<TraceMarker*>(m));
        if(m == marker) {
            // select the last index, e.i. the just created marker
            ui->cSource->setCurrentIndex(ui->cSource->count()-1);
        }
    }
}

ImpedanceMatchDialog::~ImpedanceMatchDialog()
{
    delete ui;
}

void ImpedanceMatchDialog::on_cSource_currentIndexChanged(int index)
{
    ui->rbSeries->setEnabled(index == 0);
    ui->rbParallel->setEnabled(index == 0);
    ui->zReal->setEnabled(index == 0);
    ui->zImag->setEnabled(index == 0);
    ui->zFreq->setEnabled(index == 0);
    if(index > 0) {
        auto m = qvariant_cast<TraceMarker*>(ui->cSource->itemData(index));
        ui->rbSeries->setChecked(true);
        auto data = m->getData();
        auto reflection = Z0 * (1.0 + data) / (1.0 - data);
        ui->zReal->setValue(reflection.real());
        ui->zImag->setValue(reflection.imag());
        ui->zFreq->setValue(m->getPosition());
    }
}

void ImpedanceMatchDialog::calculateMatch()
{
    try {
        double freq = ui->zFreq->value();
        complex<double> Z;
        if(ui->rbSeries->isChecked()) {
            Z.real(ui->zReal->value());
            Z.imag(ui->zImag->value());
        } else {
            auto real = complex<double>(ui->zReal->value(), 0.0);
            auto imag = complex<double>(0.0, ui->zImag->value());
            // calculate parallel impedance
            Z = real * imag / (real + imag);
        }
        bool seriesC = ui->cMatchType->currentIndex() == 0 ? true : false;
        // equations taken from http://www.ittc.ku.edu/~jstiles/723/handouts/section_5_1_Matching_with_Lumped_Elements_package.pdf
        double B, X;
        if(Z.real() > Z0) {
            B = sqrt(Z.real()/Z0)*sqrt(norm(Z)-Z0*Z.real());
            if (seriesC) {
                B = -B;
            }
            B += Z.imag();
            B /= norm(Z);
            X = 1/B + Z.imag()*Z0/Z.real()-Z0/(B*Z.real());
        } else {
            B = sqrt((Z0-Z.real())/Z.real())/Z0;
            X = sqrt(Z.real()*(Z0-Z.real()));
            if (seriesC) {
                B = -B;
                X = -X;
            }
            X -= Z.imag();
        }
        // convert X and B to inductor and capacitor
        bool twoCs = false;
        bool twoLs = false;
        double L = 0, C = 0, C2 = 0, L2 = 0;
        if(X >= 0) {
            L = X/(2*M_PI*freq);
            if(B > 0) {
                C = B/(2*M_PI*freq);
            } else {
                L2 = X/(2*M_PI*freq);
                L = -1/(B*2*M_PI*freq);
                twoLs = true;
            }
        } else {
            C = -1/(X*2*M_PI*freq);
            if(B < 0) {
                L = -1/(B*2*M_PI*freq);
            } else {
                C2 = B/(2*M_PI*freq);
                twoCs = true;
            }
        }

        ESeries::Series Lseries;
        if(ui->lE6->isChecked()) {
            Lseries = ESeries::Series::E6;
        } else if(ui->lE12->isChecked()) {
            Lseries = ESeries::Series::E12;
        } else if(ui->lE24->isChecked()) {
            Lseries = ESeries::Series::E24;
        } else if(ui->lE48->isChecked()) {
            Lseries = ESeries::Series::E48;
        } else if(ui->lE96->isChecked()) {
            Lseries = ESeries::Series::E96;
        } else {
            Lseries = ESeries::Series::Ideal;
        }
        ESeries::Series Cseries;
        if(ui->cE6->isChecked()) {
            Cseries = ESeries::Series::E6;
        } else if(ui->cE12->isChecked()) {
            Cseries = ESeries::Series::E12;
        } else if(ui->cE24->isChecked()) {
            Cseries = ESeries::Series::E24;
        } else if(ui->cE48->isChecked()) {
            Cseries = ESeries::Series::E48;
        } else if(ui->cE96->isChecked()) {
            Cseries = ESeries::Series::E96;
        } else {
            Cseries = ESeries::Series::Ideal;
        }

        L = ESeries::ToESeries(L, Lseries);
        C = ESeries::ToESeries(C, Cseries);
        L2 = ESeries::ToESeries(L2, Lseries);
        C2 = ESeries::ToESeries(C2, Cseries);

        if(twoCs) {
            for(auto b : ui->lGroup->buttons()) {
                b->setEnabled(false);
            }
            for(auto b : ui->cGroup->buttons()) {
                b->setEnabled(true);
            }
            ui->lL->setText("C1:");
            ui->lC->setText("C2:");
            ui->lValue->setUnit("F");
            ui->cValue->setUnit("F");
            ui->lValue->setValue(C2);
            ui->cValue->setValue(C);
        } else if(twoLs) {
            for(auto b : ui->cGroup->buttons()) {
                b->setEnabled(false);
            }
            for(auto b : ui->lGroup->buttons()) {
                b->setEnabled(true);
            }
            ui->lC->setText("L2:");
            ui->lL->setText("L1:");
            ui->cValue->setUnit("H");
            ui->lValue->setUnit("H");
            ui->cValue->setValue(L2);
            ui->lValue->setValue(L);
        } else {
            for(auto b : ui->cGroup->buttons()) {
                b->setEnabled(true);
            }
            for(auto b : ui->lGroup->buttons()) {
                b->setEnabled(true);
            }
            ui->lC->setText("C:");
            ui->lL->setText("L:");
            ui->lValue->setUnit("H");
            ui->cValue->setUnit("F");
            ui->lValue->setValue(L);
            ui->cValue->setValue(C);
        }
        // calculate actual matched impedance
        complex<double> Zmatched;
        complex<double> Zp, Zs;
        if(seriesC) {
            if(twoLs) {
                Zs = complex<double>(0, 2*M_PI*freq*L2);
                Zp = complex<double>(0, 2*M_PI*freq*L);
            } else if(twoCs) {
                Zs = complex<double>(0, -1/(2*M_PI*freq*C2));
                Zp = complex<double>(0, -1/(2*M_PI*freq*C));
            } else {
                Zs = complex<double>(0, -1/(2*M_PI*freq*C));
                Zp = complex<double>(0, 2*M_PI*freq*L);
            }
        } else {
            if(twoCs) {
                Zs = complex<double>(0, -1/(2*M_PI*freq*C));
                Zp = complex<double>(0, -1/(2*M_PI*freq*C2));
            } else if(twoLs){
                Zs = complex<double>(0, 2*M_PI*freq*L);
                Zp = complex<double>(0, 2*M_PI*freq*L2);
            } else {
                Zs = complex<double>(0, 2*M_PI*freq*L);
                Zp = complex<double>(0, -1/(2*M_PI*freq*C));
            }
        }
        if(Z.real() > Z0) {
            Zmatched = Z*Zp/(Z+Zp) + Zs;
        } else {
            Zmatched = Zp*(Z+Zs)/(Zp+Z+Zs);
        }
        ui->mReal->setValue(Zmatched.real());
        ui->mImag->setValue(Zmatched.imag());
        double reflection = abs((Zmatched-Z0)/(Zmatched+Z0));
        auto loss = Unit::dB(reflection);
        ui->mLoss->setValue(loss);

        // set correct image
        if(Z.real() > Z0) {
            if(X >= 0 && B >= 0) {
                ui->Image->setPixmap(QPixmap(":/icons/sLpC_small.png"));
            } else if(X < 0 && B < 0) {
                ui->Image->setPixmap(QPixmap(":/icons/sCpL_small.png"));
            } else if(X >= 0 && B < 0) {
                ui->Image->setPixmap(QPixmap(":/icons/sCpC_small.png")); // TODO check
            } else {
                ui->Image->setPixmap(QPixmap(":/icons/sLpL_small.png")); // TODO check
            }
        } else {
            if(X >= 0 && B >= 0) {
                ui->Image->setPixmap(QPixmap(":/icons/pCsL_small.png"));
            } else if(X < 0 && B < 0) {
                ui->Image->setPixmap(QPixmap(":/icons/pLsC_small.png"));
            } else if(X >= 0 && B < 0) {
                ui->Image->setPixmap(QPixmap(":/icons/pLsL_small.png"));
            } else {
                ui->Image->setPixmap(QPixmap(":/icons/pCsC_small.png"));
            }
        }
    } catch (exception e){
        // something went wrong, probably caused by (intermediate) invalid input, such as f=0Hz
        ui->lValue->setValue(nan(""));
        ui->cValue->setValue(nan(""));
        ui->mReal->setValue(nan(""));
        ui->mImag->setValue(nan(""));
        ui->mLoss->setValue(nan(""));
    }
}
