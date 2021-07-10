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

    auto setDefaultSettingsEnabled = [=](bool en) {
        ui->StartupSweepType->setEnabled(en);
        ui->StartupSweepStart->setEnabled(en);
        ui->StartupSweepStop->setEnabled(en);
        ui->StartupSweepPoints->setEnabled(en);
        ui->StartupSweepPowerStart->setEnabled(en);
        ui->StartupSweepPowerStop->setEnabled(en);
        ui->StartupSweepPowerFrequency->setEnabled(en);
        ui->StartupSweepLevel->setEnabled(en);
        ui->StartupSweepBandwidth->setEnabled(en);
        ui->StartupSweepAveraging->setEnabled(en);
        ui->StartupGeneratorFrequency->setEnabled(en);
        ui->StartupGeneratorLevel->setEnabled(en);
        ui->StartupSAStart->setEnabled(en);
        ui->StartupSAStop->setEnabled(en);
        ui->StartupSARBW->setEnabled(en);
        ui->StartupSAWindow->setEnabled(en);
        ui->StartupSADetector->setEnabled(en);
        ui->StartupSAAveraging->setEnabled(en);
        ui->StartupSASignalID->setEnabled(en);
    };

    // Setup GUI connections and adjustments
    // Startup page
    connect(ui->StartupSweepLastUsed, &QPushButton::clicked, [=](){
       setDefaultSettingsEnabled(false);
    });
    connect(ui->StartupSweepDefault, &QPushButton::clicked, [=](){
       setDefaultSettingsEnabled(true);
    });
    ui->StartupSweepStart->setUnit("Hz");
    ui->StartupSweepStart->setPrefixes(" kMG");
    ui->StartupSweepStop->setUnit("Hz");
    ui->StartupSweepStop->setPrefixes(" kMG");
    ui->StartupSweepPowerFrequency->setUnit("Hz");
    ui->StartupSweepPowerFrequency->setPrefixes(" kMG");
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
        ui->SCPIServerPort->setEnabled(false);
        ui->SCPIServerEnabled->setEnabled(false);
    }

    connect(ui->GraphsShowMarkerData, &QCheckBox::toggled, [=](bool enabled) {
         ui->GraphsShowAllMarkerData->setEnabled(enabled);
    });

    // Page selection
    connect(ui->treeWidget, &QTreeWidget::currentItemChanged, [=](QTreeWidgetItem *current, QTreeWidgetItem *) {
        auto name = current->text(0);
        // remove any potential white space in name (can't have whitespace in page names)
        name.replace(" ", "");
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
        p->Startup.DefaultSweep.type = ui->StartupSweepType->currentText();
        p->Startup.DefaultSweep.f_start = ui->StartupSweepStart->value();
        p->Startup.DefaultSweep.f_stop = ui->StartupSweepStop->value();
        p->Startup.DefaultSweep.f_excitation = ui->StartupSweepLevel->value();
        p->Startup.DefaultSweep.dbm_start = ui->StartupSweepPowerStart->value();
        p->Startup.DefaultSweep.dbm_stop = ui->StartupSweepPowerStop->value();
        p->Startup.DefaultSweep.dbm_freq = ui->StartupSweepPowerFrequency->value();
        p->Startup.DefaultSweep.bandwidth = ui->StartupSweepBandwidth->value();
        p->Startup.DefaultSweep.points = ui->StartupSweepPoints->value();
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
        p->Acquisition.adjustPowerLevel = ui->AcquisitionAdjustPowerLevel->isChecked();
        p->Acquisition.harmonicMixing = ui->AcquisitionUseHarmonic->isChecked();
        p->Acquisition.useDFTinSAmode = ui->AcquisitionUseDFT->isChecked();
        p->Acquisition.RBWLimitForDFT = ui->AcquisitionDFTlimitRBW->value();
        p->Graphs.Color.background = ui->GraphsColorBackground->getColor();
        p->Graphs.Color.axis = ui->GraphsColorAxis->getColor();
        p->Graphs.Color.divisions = ui->GraphsColorDivisions->getColor();
        p->Graphs.domainChangeBehavior = (GraphDomainChangeBehavior) ui->GraphsDomainChangeBehavior->currentIndex();
        p->Graphs.markerBehavior.showDataOnGraphs = ui->GraphsShowMarkerData->isChecked();
        p->Graphs.markerBehavior.showAllData = ui->GraphsShowAllMarkerData->isChecked();
        p->SCPIServer.enabled = ui->SCPIServerEnabled->isChecked();
        p->SCPIServer.port = ui->SCPIServerPort->value();
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
    ui->StartupSweepType->setCurrentText(p->Startup.DefaultSweep.type);
    ui->StartupSweepStart->setValueQuiet(p->Startup.DefaultSweep.f_start);
    ui->StartupSweepStop->setValueQuiet(p->Startup.DefaultSweep.f_stop);
    ui->StartupSweepLevel->setValue(p->Startup.DefaultSweep.f_excitation);
    ui->StartupSweepPowerStart->setValue(p->Startup.DefaultSweep.dbm_start);
    ui->StartupSweepPowerStop->setValue(p->Startup.DefaultSweep.dbm_stop);
    ui->StartupSweepPowerFrequency->setValueQuiet(p->Startup.DefaultSweep.dbm_freq);
    ui->StartupSweepBandwidth->setValueQuiet(p->Startup.DefaultSweep.bandwidth);
    ui->StartupSweepPoints->setValue(p->Startup.DefaultSweep.points);
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
    ui->AcquisitionAdjustPowerLevel->setChecked(p->Acquisition.adjustPowerLevel);
    ui->AcquisitionUseHarmonic->setChecked(p->Acquisition.harmonicMixing);
    ui->AcquisitionUseDFT->setChecked(p->Acquisition.useDFTinSAmode);
    ui->AcquisitionDFTlimitRBW->setValue(p->Acquisition.RBWLimitForDFT);

    ui->GraphsColorBackground->setColor(p->Graphs.Color.background);
    ui->GraphsColorAxis->setColor(p->Graphs.Color.axis);
    ui->GraphsColorDivisions->setColor(p->Graphs.Color.divisions);
    ui->GraphsDomainChangeBehavior->setCurrentIndex((int) p->Graphs.domainChangeBehavior);
    ui->GraphsShowMarkerData->setChecked(p->Graphs.markerBehavior.showDataOnGraphs);
    ui->GraphsShowAllMarkerData->setChecked(p->Graphs.markerBehavior.showAllData);
    ui->SCPIServerEnabled->setChecked(p->SCPIServer.enabled);
    ui->SCPIServerPort->setValue(p->SCPIServer.port);

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
