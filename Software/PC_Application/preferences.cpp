#include "preferences.h"
#include "ui_preferencesdialog.h"
#include <QSettings>
#include <QPushButton>
#include <QMessageBox>
#include <map>
#include <QDebug>
#include "CustomWidgets/informationbox.h"

using namespace std;

Preferences Preferences::instance;

PreferencesDialog::PreferencesDialog(Preferences *pref, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    p(pref)
{
    ui->setupUi(this);

    // Setup GUI connections and adjustments
    // Startup page
    connect(ui->StartupSweepLastUsed, &QPushButton::clicked, [=](){
       ui->StartupSweepStart->setEnabled(false);
       ui->StartupSweepStop->setEnabled(false);
       ui->StartupSweepPoints->setEnabled(false);
       ui->StartupSweepLevel->setEnabled(false);
       ui->StartupSweepBandwidth->setEnabled(false);
       ui->StartupSweepAveraging->setEnabled(false);
       ui->StartupGeneratorFrequency->setEnabled(false);
       ui->StartupGeneratorLevel->setEnabled(false);
       ui->StartupSAStart->setEnabled(false);
       ui->StartupSAStop->setEnabled(false);
       ui->StartupSARBW->setEnabled(false);
       ui->StartupSAWindow->setEnabled(false);
       ui->StartupSADetector->setEnabled(false);
       ui->StartupSAAveraging->setEnabled(false);
       ui->StartupSASignalID->setEnabled(false);
    });
    connect(ui->StartupSweepDefault, &QPushButton::clicked, [=](){
       ui->StartupSweepStart->setEnabled(true);
       ui->StartupSweepStop->setEnabled(true);
       ui->StartupSweepPoints->setEnabled(true);
       ui->StartupSweepLevel->setEnabled(true);
       ui->StartupSweepBandwidth->setEnabled(true);
       ui->StartupSweepAveraging->setEnabled(true);
       ui->StartupGeneratorFrequency->setEnabled(true);
       ui->StartupGeneratorLevel->setEnabled(true);
       ui->StartupSAStart->setEnabled(true);
       ui->StartupSAStop->setEnabled(true);
       ui->StartupSARBW->setEnabled(true);
       ui->StartupSAWindow->setEnabled(true);
       ui->StartupSADetector->setEnabled(true);
       ui->StartupSAAveraging->setEnabled(true);
       ui->StartupSASignalID->setEnabled(true);
    });
    ui->StartupSweepStart->setUnit("Hz");
    ui->StartupSweepStart->setPrefixes(" kMG");
    ui->StartupSweepStop->setUnit("Hz");
    ui->StartupSweepStop->setPrefixes(" kMG");
    ui->StartupSweepBandwidth->setUnit("Hz");
    ui->StartupSweepBandwidth->setPrefixes(" k");
    ui->StartupGeneratorFrequency->setUnit("Hz");
    ui->StartupGeneratorFrequency->setPrefixes(" kMG");
    ui->StartupSAStart->setUnit("Hz");
    ui->StartupSAStart->setPrefixes(" kMG");
    ui->StartupSAStop->setUnit("Hz");
    ui->StartupSAStop->setPrefixes(" kMG");
    ui->StartupSARBW->setUnit("Hz");
    ui->StartupSARBW->setPrefixes(" k");

    // Acquisition page
    ui->AcquisitionDFTlimitRBW->setUnit("Hz");
    ui->AcquisitionDFTlimitRBW->setPrefixes(" k");
    connect(ui->AcquisitionUseDFT, &QCheckBox::toggled, [=](bool enabled) {
       ui->AcquisitionDFTlimitRBW->setEnabled(enabled);
    });

    // General page
    if(p->TCPoverride) {
        ui->GeneralSCPIPort->setEnabled(false);
        ui->GeneralSCPIEnabled->setEnabled(false);
    }

    connect(ui->GeneralMarkerDataGraph, &QCheckBox::toggled, [=](bool enabled) {
         ui->GeneralMarkerDataGraphAll->setEnabled(enabled);
    });

    // Page selection
    connect(ui->treeWidget, &QTreeWidget::currentItemChanged, [=](QTreeWidgetItem *current, QTreeWidgetItem *) {
        auto name = current->text(0);
        for(int i=0;i<ui->pageWidget->count();i++) {
            auto w = ui->pageWidget->widget(i);
            if(name == w->objectName()) {
                // found the correct page, set to front
                ui->pageWidget->setCurrentWidget(w);
                break;
            }
        }
    });

    // Reset and OK action
    connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, [=](){
        if(QMessageBox::question(this, "Restore defaults?", "Do you really want to set all preferences to their default values?") == QMessageBox::StandardButton::Yes) {
            p->setDefault();
            setInitialGUIState();
        }
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, [=](){
        // apply GUI state to settings
        p->Startup.ConnectToFirstDevice = ui->StartupAutoconnect->isChecked();
        p->Startup.RememberSweepSettings = ui->StartupSweepLastUsed->isChecked();
        p->Startup.DefaultSweep.start = ui->StartupSweepStart->value();
        p->Startup.DefaultSweep.stop = ui->StartupSweepStop->value();
        p->Startup.DefaultSweep.bandwidth = ui->StartupSweepBandwidth->value();
        p->Startup.DefaultSweep.points = ui->StartupSweepPoints->value();
        p->Startup.DefaultSweep.excitation = ui->StartupSweepLevel->value();
        p->Startup.DefaultSweep.averaging = ui->StartupSweepAveraging->value();
        p->Startup.Generator.frequency = ui->StartupGeneratorFrequency->value();
        p->Startup.Generator.level = ui->StartupGeneratorLevel->value();
        p->Startup.SA.start = ui->StartupSAStart->value();
        p->Startup.SA.stop = ui->StartupSAStop->value();
        p->Startup.SA.RBW = ui->StartupSARBW->value();
        p->Startup.SA.window = ui->StartupSAWindow->currentIndex();
        p->Startup.SA.detector = ui->StartupSADetector->currentIndex();
        p->Startup.SA.signalID = ui->StartupSASignalID->isChecked();
        p->Acquisition.alwaysExciteBothPorts = ui->AcquisitionAlwaysExciteBoth->isChecked();
        p->Acquisition.suppressPeaks = ui->AcquisitionSuppressPeaks->isChecked();
        p->Acquisition.harmonicMixing = ui->AcquisitionUseHarmonic->isChecked();
        p->Acquisition.useDFTinSAmode = ui->AcquisitionUseDFT->isChecked();
        p->Acquisition.RBWLimitForDFT = ui->AcquisitionDFTlimitRBW->value();
        p->General.graphColors.background = ui->GeneralGraphBackground->getColor();
        p->General.graphColors.axis = ui->GeneralGraphAxis->getColor();
        p->General.graphColors.divisions = ui->GeneralGraphDivisions->getColor();
        p->General.markerDefault.showDataOnGraphs = ui->GeneralMarkerDataGraph->isChecked();
        p->General.markerDefault.showAllData = ui->GeneralMarkerDataGraphAll->isChecked();
        p->General.SCPI.enabled = ui->GeneralSCPIEnabled->isChecked();
        p->General.SCPI.port = ui->GeneralSCPIPort->value();
        accept();
    });

    setInitialGUIState();

    connect(ui->AcquisitionUseHarmonic, &QCheckBox::toggled, [=](bool enabled) {
       if(enabled) {
           InformationBox::ShowMessage("Harmonic Mixing", "When harmonic mixing is enabled, the frequency range of the VNA is (theoretically) extended up to 18GHz "
                                       "by using higher harmonics of the source signal as well as the 1.LO. The fundamental frequency is still present "
                                       "in the output signal and might disturb the measurement if the DUT is not linear. Performance above 6GHz is not "
                                       "specified and generally not very good. However, this mode might be useful if the signal of interest is just above "
                                       "6GHz (typically useful values up to 7-8GHz). Performance below 6GHz is not affected by this setting");
       }
    });
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::setInitialGUIState()
{
    ui->StartupAutoconnect->setChecked(p->Startup.ConnectToFirstDevice);
    if(p->Startup.RememberSweepSettings) {
        ui->StartupSweepLastUsed->click();
    } else {
        ui->StartupSweepDefault->click();
    }
    ui->StartupSweepStart->setValueQuiet(p->Startup.DefaultSweep.start);
    ui->StartupSweepStop->setValueQuiet(p->Startup.DefaultSweep.stop);
    ui->StartupSweepBandwidth->setValueQuiet(p->Startup.DefaultSweep.bandwidth);
    ui->StartupSweepPoints->setValue(p->Startup.DefaultSweep.points);
    ui->StartupSweepLevel->setValue(p->Startup.DefaultSweep.excitation);
    ui->StartupGeneratorFrequency->setValue(p->Startup.Generator.frequency);
    ui->StartupGeneratorLevel->setValue(p->Startup.Generator.level);
    ui->StartupSweepAveraging->setValue(p->Startup.DefaultSweep.averaging);
    ui->StartupSAStart->setValue(p->Startup.SA.start);
    ui->StartupSAStop->setValue(p->Startup.SA.stop);
    ui->StartupSARBW->setValue(p->Startup.SA.RBW);
    ui->StartupSAWindow->setCurrentIndex(p->Startup.SA.window);
    ui->StartupSADetector->setCurrentIndex(p->Startup.SA.detector);
    ui->StartupSAAveraging->setValue(p->Startup.SA.averaging);
    ui->StartupSASignalID->setChecked(p->Startup.SA.signalID);

    ui->AcquisitionAlwaysExciteBoth->setChecked(p->Acquisition.alwaysExciteBothPorts);
    ui->AcquisitionSuppressPeaks->setChecked(p->Acquisition.suppressPeaks);
    ui->AcquisitionUseHarmonic->setChecked(p->Acquisition.harmonicMixing);
    ui->AcquisitionUseDFT->setChecked(p->Acquisition.useDFTinSAmode);
    ui->AcquisitionDFTlimitRBW->setValue(p->Acquisition.RBWLimitForDFT);

    ui->GeneralGraphBackground->setColor(p->General.graphColors.background);
    ui->GeneralGraphAxis->setColor(p->General.graphColors.axis);
    ui->GeneralGraphDivisions->setColor(p->General.graphColors.divisions);
    ui->GeneralMarkerDataGraph->setChecked(p->General.markerDefault.showDataOnGraphs);
    ui->GeneralMarkerDataGraphAll->setChecked(p->General.markerDefault.showAllData);
    ui->GeneralSCPIEnabled->setChecked(p->General.SCPI.enabled);
    ui->GeneralSCPIPort->setValue(p->General.SCPI.port);

    QTreeWidgetItem *item = ui->treeWidget->topLevelItem(0);
    if (item != nullptr) {
        ui->treeWidget->setCurrentItem(item);     // visually select first item
    }
}

void Preferences::load()
{
    QSettings settings;
    // load settings, using default values if not present
    qInfo() << "Loading preferences";
    for(auto d : descr) {
        try {
            d.var.setValue(settings.value(d.name, d.def));
            qDebug() << "Setting" << d.name << "is set to" << d.var.value();
        } catch (const exception& e){
            d.var.setValue(d.def);
            qDebug() << "Setting" << d.name << "reset to default:" << d.def;
        }
    }
}

void Preferences::store()
{
    QSettings settings;
    // store settings
    for(auto d : descr) {
        settings.setValue(d.name, d.var.value());
    }
}

void Preferences::edit()
{
    auto dialog = new PreferencesDialog(this);
    dialog->exec();
}

void Preferences::setDefault()
{
    for(auto d : descr) {
        d.var.setValue(d.def);
    }
}
