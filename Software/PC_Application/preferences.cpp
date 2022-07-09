#include "preferences.h"

#include "ui_preferencesdialog.h"
#include "CustomWidgets/informationbox.h"
#include "appwindow.h"

#include <QSettings>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>

#include <map>
#include <fstream>
#include <iomanip>

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
       ui->StartupSetupFile->setEnabled(false);
       ui->StartupBrowse->setEnabled(false);
       ui->StartupStack->setCurrentWidget(ui->StartupPageLastUsed);
    });
    connect(ui->StartupSweepDefault, &QPushButton::clicked, [=](){
       setDefaultSettingsEnabled(true);
       ui->StartupSetupFile->setEnabled(false);
       ui->StartupBrowse->setEnabled(false);
       ui->StartupStack->setCurrentWidget(ui->StartupPageDefaultValues);
    });
    connect(ui->StartupUseSetupFile, &QPushButton::clicked, [=](){
       setDefaultSettingsEnabled(false);
       ui->StartupSetupFile->setEnabled(true);
       ui->StartupBrowse->setEnabled(true);
       ui->StartupStack->setCurrentWidget(ui->StartupPageSetupFile);
    });
    connect(ui->StartupBrowse, &QPushButton::clicked, [=](){
       ui->StartupSetupFile->setText(QFileDialog::getOpenFileName(nullptr, "Select startup setup file", "", "Setup files (*.setup)", nullptr, QFileDialog::DontUseNativeDialog));
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
    ui->AcquisitionIF1->setUnit("Hz");
    ui->AcquisitionIF1->setPrefixes(" kM");
    ui->AcquisitionIF1->setPrecision(6);
    ui->AcquisitionADCRate->setUnit("Hz");
    ui->AcquisitionADCRate->setPrefixes(" kM");
    ui->AcquisitionADCRate->setPrecision(6);
    ui->AcquisitionIF2->setUnit("Hz");
    ui->AcquisitionIF2->setPrefixes(" kM");
    ui->AcquisitionIF2->setPrecision(6);
    auto updateADCRate = [=]() {
        // update ADC rate, see FPGA protocol for calculation
        ui->AcquisitionADCRate->setValue(102400000.0 / ui->AcquisitionADCpresc->value());
    };
    auto updateIF2 = [=]() {
        auto ADCrate = ui->AcquisitionADCRate->value();
        ui->AcquisitionIF2->setValue(ADCrate * ui->AcquisitionADCphaseInc->value() / 4096);
    };
    connect(ui->AcquisitionADCpresc, qOverload<int>(&QSpinBox::valueChanged), updateADCRate);
    connect(ui->AcquisitionADCpresc, qOverload<int>(&QSpinBox::valueChanged), updateIF2);
    connect(ui->AcquisitionADCphaseInc, qOverload<int>(&QSpinBox::valueChanged), updateIF2);

    // General page
    if(p->TCPoverride) {
        ui->SCPIServerPort->setEnabled(false);
        ui->SCPIServerEnabled->setEnabled(false);
    }

    connect(ui->MarkerShowMarkerData, &QCheckBox::toggled, [=](bool enabled) {
         ui->MarkerShowAllMarkerData->setEnabled(enabled);
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
        if(InformationBox::AskQuestion("Restore defaults?", "Do you really want to set all preferences to their default values?", true)) {
            p->setDefault();
            setInitialGUIState();
        }
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, [=](){
        // apply GUI state to settings
        updateFromGUI();
        accept();
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Save), &QPushButton::clicked, [=](){
        auto filename = QFileDialog::getSaveFileName(this, "Save preferences", "", "LibreVNA preferences files (*.vnapref)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.length() > 0) {
           if(!filename.toLower().endsWith(".vnapref")) {
               filename.append(".vnapref");
           }
           ofstream file;
           file.open(filename.toStdString());
           updateFromGUI();
           file << setw(1) << p->toJSON();
           file.close();
        }
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Open), &QPushButton::clicked, [=](){
        auto filename = QFileDialog::getOpenFileName(this, "Load preferences", "", "LibreVNA preferences files (*.vnapref)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.length() > 0) {
           ifstream file;
           file.open(filename.toStdString());
           nlohmann::json j;
           file >> j;
           file.close();
           p->fromJSON(j);
           setInitialGUIState();
        }
    });

    setInitialGUIState();
    updateADCRate();
    updateIF2();

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
    } if(p->Startup.UseSetupFile) {
        ui->StartupUseSetupFile->click();
    } else {
        ui->StartupSweepDefault->click();
    }
    ui->StartupAutosaveSetupFile->setChecked(p->Startup.AutosaveSetupFile);
    ui->StartupSetupFile->setText(p->Startup.SetupFile);
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
    ui->AcquisitionAllowSegmentedSweep->setChecked(p->Acquisition.allowSegmentedSweep);
    ui->AcquisitionUseDFT->setChecked(p->Acquisition.useDFTinSAmode);
    ui->AcquisitionDFTlimitRBW->setValue(p->Acquisition.RBWLimitForDFT);
    ui->AcquisitionAveragingMode->setCurrentIndex(p->Acquisition.useMedianAveraging ? 1 : 0);
    ui->AcquisitionIF1->setValue(p->Acquisition.IF1);
    ui->AcquisitionADCpresc->setValue(p->Acquisition.ADCprescaler);
    ui->AcquisitionADCphaseInc->setValue(p->Acquisition.DFTPhaseInc);

    ui->GraphsShowUnit->setChecked(p->Graphs.showUnits);
    ui->GraphsColorBackground->setColor(p->Graphs.Color.background);
    ui->GraphsColorAxis->setColor(p->Graphs.Color.axis);
    ui->GraphsColorTicksDivisions->setColor(p->Graphs.Color.Ticks.divisions);
    ui->GraphsColorTicksBackgroundEnabled->setChecked(p->Graphs.Color.Ticks.Background.enabled);
    ui->GraphsColorTicksBackground->setColor(p->Graphs.Color.Ticks.Background.background);
    ui->GraphsDomainChangeBehavior->setCurrentIndex((int) p->Graphs.domainChangeBehavior);
    ui->GraphsLimitIndication->setCurrentIndex((int) p->Graphs.limitIndication);
    ui->GraphsLineWidth->setValue(p->Graphs.lineWidth);
    ui->GraphsFontSizeAxis->setValue(p->Graphs.fontSizeAxis);
    ui->GraphsFontSizeCursorOverlay->setValue(p->Graphs.fontSizeCursorOverlay);
    ui->GraphsFontSizeMarkerData->setValue(p->Graphs.fontSizeMarkerData);
    ui->GraphsFontSizeTraceNames->setValue(p->Graphs.fontSizeTraceNames);

    ui->MarkerShowMarkerData->setChecked(p->Marker.defaultBehavior.showDataOnGraphs);
    ui->MarkerShowAllMarkerData->setChecked(p->Marker.defaultBehavior.showAllData);
    ui->MarkerInterpolate->setCurrentIndex(p->Marker.interpolatePoints ? 1 : 0);

    ui->SCPIServerEnabled->setChecked(p->SCPIServer.enabled);
    ui->SCPIServerPort->setValue(p->SCPIServer.port);

    QTreeWidgetItem *item = ui->treeWidget->topLevelItem(0);
    if (item != nullptr) {
        ui->treeWidget->setCurrentItem(item);     // visually select first item
    }
}

void PreferencesDialog::updateFromGUI()
{
    p->Startup.ConnectToFirstDevice = ui->StartupAutoconnect->isChecked();
    p->Startup.RememberSweepSettings = ui->StartupSweepLastUsed->isChecked();
    p->Startup.UseSetupFile = ui->StartupUseSetupFile->isChecked();
    p->Startup.SetupFile = ui->StartupSetupFile->text();
    p->Startup.AutosaveSetupFile = ui->StartupAutosaveSetupFile->isChecked();
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
    p->Acquisition.allowSegmentedSweep = ui->AcquisitionAllowSegmentedSweep->isChecked();
    p->Acquisition.useDFTinSAmode = ui->AcquisitionUseDFT->isChecked();
    p->Acquisition.RBWLimitForDFT = ui->AcquisitionDFTlimitRBW->value();
    p->Acquisition.useMedianAveraging = ui->AcquisitionAveragingMode->currentIndex() == 1;
    p->Acquisition.IF1 = ui->AcquisitionIF1->value();
    p->Acquisition.ADCprescaler = ui->AcquisitionADCpresc->value();
    p->Acquisition.DFTPhaseInc = ui->AcquisitionADCphaseInc->value();

    p->Graphs.showUnits = ui->GraphsShowUnit->isChecked();
    p->Graphs.Color.background = ui->GraphsColorBackground->getColor();
    p->Graphs.Color.axis = ui->GraphsColorAxis->getColor();
    p->Graphs.Color.Ticks.Background.enabled = ui->GraphsColorTicksBackgroundEnabled->isChecked();
    p->Graphs.Color.Ticks.Background.background = ui->GraphsColorTicksBackground->getColor();
    p->Graphs.Color.Ticks.divisions = ui->GraphsColorTicksDivisions->getColor();
    p->Graphs.domainChangeBehavior = (GraphDomainChangeBehavior) ui->GraphsDomainChangeBehavior->currentIndex();
    p->Graphs.limitIndication = (GraphLimitIndication) ui->GraphsLimitIndication->currentIndex();
    p->Graphs.lineWidth = ui->GraphsLineWidth->value();
    p->Graphs.fontSizeAxis = ui->GraphsFontSizeAxis->value();
    p->Graphs.fontSizeCursorOverlay = ui->GraphsFontSizeCursorOverlay->value();
    p->Graphs.fontSizeMarkerData = ui->GraphsFontSizeMarkerData->value();
    p->Graphs.fontSizeTraceNames = ui->GraphsFontSizeTraceNames->value();

    p->Marker.defaultBehavior.showDataOnGraphs = ui->MarkerShowMarkerData->isChecked();
    p->Marker.defaultBehavior.showAllData = ui->MarkerShowAllMarkerData->isChecked();
    p->Marker.interpolatePoints = ui->MarkerInterpolate->currentIndex() == 1;

    p->SCPIServer.enabled = ui->SCPIServerEnabled->isChecked();
    p->SCPIServer.port = ui->SCPIServerPort->value();
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
    if(AppWindow::showGUI()) {
        dialog->exec();
    }
}

void Preferences::setDefault()
{
    for(auto d : descr) {
        d.var.setValue(d.def);
    }
}

void Preferences::fromJSON(nlohmann::json j)
{
    parseJSON(j, descr);
}

nlohmann::json Preferences::toJSON()
{
    return createJSON(descr);
}
