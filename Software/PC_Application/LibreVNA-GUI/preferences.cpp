#include "preferences.h"

#include "ui_preferencesdialog.h"
#include "CustomWidgets/informationbox.h"
#include "appwindow.h"
#include "Device/LibreVNA/Compound/compounddeviceeditdialog.h"

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
       ui->StartupSetupFile->setText(QFileDialog::getOpenFileName(nullptr, "Select startup setup file", "", "Setup files (*.setup)", nullptr, Preferences::QFileDialogOptions()));
    });
    ui->StartupSweepStart->setUnit("Hz");
    ui->StartupSweepStart->setPrefixes(" kMG");
    ui->StartupSweepStop->setUnit("Hz");
    ui->StartupSweepStop->setPrefixes(" kMG");
    ui->StartupSweepPowerFrequency->setUnit("Hz");
    ui->StartupSweepPowerFrequency->setPrefixes(" kMG");
    ui->StartupSweepBandwidth->setUnit("Hz");
    ui->StartupSweepBandwidth->setPrefixes(" k");
    ui->StartupSweepDwellTime->setUnit("s");
    ui->StartupSweepDwellTime->setPrefixes("um ");
    ui->StartupSweepDwellTime->setPrecision(3);
    ui->StartupGeneratorFrequency->setUnit("Hz");
    ui->StartupGeneratorFrequency->setPrefixes(" kMG");
    ui->StartupSAStart->setUnit("Hz");
    ui->StartupSAStart->setPrefixes(" kMG");
    ui->StartupSAStop->setUnit("Hz");
    ui->StartupSAStop->setPrefixes(" kMG");
    ui->StartupSARBW->setUnit("Hz");
    ui->StartupSARBW->setPrefixes(" k");

    // Acquisition page
    ui->AcquisitionFullSpanStart->setUnit("Hz");
    ui->AcquisitionFullSpanStart->setPrefixes(" kMG");
    ui->AcquisitionFullSpanStart->setPrecision(6);
    ui->AcquisitionFullSpanStart->setEnabled(false);
    ui->AcquisitionFullSpanStop->setUnit("Hz");
    ui->AcquisitionFullSpanStop->setPrefixes(" kMG");
    ui->AcquisitionFullSpanStop->setPrecision(6);
    ui->AcquisitionFullSpanStop->setEnabled(false);

    connect(ui->AcquisitionFullSpanBehavior, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
        ui->AcquisitionFullSpanStart->setEnabled(ui->AcquisitionFullSpanBehavior->currentIndex() == 1);
        ui->AcquisitionFullSpanStop->setEnabled(ui->AcquisitionFullSpanBehavior->currentIndex() == 1);
    });

    // Graph page
    ui->GraphsZoomFactor->setPrecision(3);
    ui->GraphsSweepHidePercent->setPrecision(3);
    ui->GraphsSweepHidePercent->setUnit("%");

    auto layout = static_cast<QGridLayout*>(ui->GraphAxisLimitGroup->layout());
    for(unsigned int i=(int) YAxis::Type::Disabled + 1;i<(int) YAxis::Type::Last;i++) {
        auto type = (YAxis::Type) i;
        layout->addWidget(new QLabel(YAxis::TypeToName(type)), i, 0);
        auto minEntry = new SIUnitEdit(YAxis::Unit(type), YAxis::Prefixes(type), 5);
        layout->addWidget(minEntry, i, 1);
        graphAxisLimitsMinEntries[type] = minEntry;
        auto maxEntry = new SIUnitEdit(YAxis::Unit(type), YAxis::Prefixes(type), 5);
        layout->addWidget(maxEntry, i, 2);
        graphAxisLimitsMaxEntries[type] = maxEntry;
    }

    // General page
    if(p->TCPoverride) {
        ui->SCPIServerPort->setEnabled(false);
        ui->SCPIServerEnabled->setEnabled(false);
    }

    connect(ui->MarkerShowMarkerData, &QCheckBox::toggled, [=](bool enabled) {
         ui->MarkerShowdB->setEnabled(enabled);
         ui->MarkerShowdBm->setEnabled(enabled);
         ui->MarkerShowdBUv->setEnabled(enabled);
         ui->MarkerShowdBAngle->setEnabled(enabled);
         ui->MarkerShowRealImag->setEnabled(enabled);
         ui->MarkerShowImpedance->setEnabled(enabled);
         ui->MarkerShowVSWR->setEnabled(enabled);
         ui->MarkerShowResistance->setEnabled(enabled);
         ui->MarkerShowCapacitance->setEnabled(enabled);
         ui->MarkerShowInductance->setEnabled(enabled);
         ui->MarkerShowQualityFactor->setEnabled(enabled);
         ui->MarkerShowNoise->setEnabled(enabled);
         ui->MarkerShowPhasenoise->setEnabled(enabled);
         ui->MarkerShowCenterBandwidth->setEnabled(enabled);
         ui->MarkerShowCutoff->setEnabled(enabled);
         ui->MarkerShowInsertionLoss->setEnabled(enabled);
         ui->MarkerShowTOI->setEnabled(enabled);
         ui->MarkerShowAvgTone->setEnabled(enabled);
         ui->MarkerShowAvgMod->setEnabled(enabled);
         ui->MarkerShowP1dB->setEnabled(enabled);
    });

    // Debug page
    ui->DebugMaxUSBlogSize->setUnit("B");
    ui->DebugMaxUSBlogSize->setPrefixes(" kMG");

    // Add device driver settings
    QTreeWidgetItem *item = ui->treeWidget->findItems("Device Drivers", Qt::MatchExactly)[0];
    for(auto driver : DeviceDriver::getDrivers()) {
        auto w = driver->createSettingsWidget();
        if(!w) {
            continue;
        }
        w->setObjectName(driver->getDriverName());
        ui->pageWidget->addWidget(w);
        auto driverItem = new QTreeWidgetItem();
        driverItem->setText(0, driver->getDriverName());
        item->addChild(driverItem);
    }

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
        emit p->updated();
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, [=](){
        // apply GUI state to settings
        updateFromGUI();
        emit p->updated();
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Save), &QPushButton::clicked, [=](){
        auto filename = QFileDialog::getSaveFileName(this, "Save preferences", "", "LibreVNA preferences files (*.vnapref)", nullptr, Preferences::QFileDialogOptions());
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
        auto filename = QFileDialog::getOpenFileName(this, "Load preferences", "", "LibreVNA preferences files (*.vnapref)", nullptr, Preferences::QFileDialogOptions());
        if(filename.length() > 0) {
           ifstream file;
           file.open(filename.toStdString());
           nlohmann::json j;
           file >> j;
           file.close();
           p->fromJSON(j);
           setInitialGUIState();
           emit p->updated();
        }
    });
    connect(ui->AcquisitionLimitTDRCheckbox, &QCheckBox::toggled, [=](bool enabled){
        ui->AcquisitionDFTLimitValue->setEnabled(enabled);
    });

    setInitialGUIState();
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
    } else if(p->Startup.UseSetupFile) {
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
    ui->StartupSweepDwellTime->setValue(p->Startup.DefaultSweep.dwellTime);
    ui->StartupSAStart->setValue(p->Startup.SA.start);
    ui->StartupSAStop->setValue(p->Startup.SA.stop);
    ui->StartupSARBW->setValue(p->Startup.SA.RBW);
    ui->StartupSAWindow->setCurrentIndex(p->Startup.SA.window);
    ui->StartupSADetector->setCurrentIndex(p->Startup.SA.detector);
    ui->StartupSAAveraging->setValue(p->Startup.SA.averaging);

    ui->AcquisitionAlwaysExciteBoth->setChecked(p->Acquisition.alwaysExciteAllPorts);
    ui->AcquisitionAllowSegmentedSweep->setChecked(p->Acquisition.allowSegmentedSweep);
    ui->AcquisitionAveragingMode->setCurrentIndex(p->Acquisition.useMedianAveraging ? 1 : 0);
    ui->AcquisitionFullSpanBehavior->setCurrentIndex(p->Acquisition.fullSpanManual ? 1 : 0);
    ui->AcquisitionFullSpanStart->setValue(p->Acquisition.fullSpanStart);
    ui->AcquisitionFullSpanStop->setValue(p->Acquisition.fullSpanStop);
    ui->AcquisitionFullSpanCalibrated->setChecked(p->Acquisition.fullSpanCalibratedRange);
    ui->AcquisitionLimitTDRCheckbox->setChecked(p->Acquisition.limitDFT);
    ui->AcquisitionDFTLimitValue->setValue(p->Acquisition.maxDFTrate);
    ui->AcquisitionGroupDelaySamples->setValue(p->Acquisition.groupDelaySamples);

    ui->GraphsDefaultTransmission->setCurrentText(p->Graphs.defaultGraphs.transmission);
    ui->GraphsDefaultReflection->setCurrentText(p->Graphs.defaultGraphs.reflection);
    ui->GraphsShowUnit->setChecked(p->Graphs.showUnits);
    ui->GraphsColorBackground->setColor(p->Graphs.Color.background);
    ui->GraphsColorAxis->setColor(p->Graphs.Color.axis);
    ui->GraphsColorTicksDivisions->setColor(p->Graphs.Color.Ticks.divisions);
    ui->GraphsColorTicksBackgroundEnabled->setChecked(p->Graphs.Color.Ticks.Background.enabled);
    ui->GraphsColorTicksBackground->setColor(p->Graphs.Color.Ticks.Background.background);
    ui->GraphsDomainChangeBehavior->setCurrentIndex((int) p->Graphs.domainChangeBehavior);
    ui->GraphsLimitIndication->setCurrentIndex((int) p->Graphs.limitIndication);
    ui->GraphsLimitNaNpasses->setCurrentIndex(p->Graphs.limitNaNpasses ? 1 : 0);
    ui->GraphsLineWidth->setValue(p->Graphs.lineWidth);
    ui->GraphsFontSizeTitle->setValue(p->Graphs.fontSizeTitle);
    ui->GraphsFontSizeAxis->setValue(p->Graphs.fontSizeAxis);
    ui->GraphsFontSizeCursorOverlay->setValue(p->Graphs.fontSizeCursorOverlay);
    ui->GraphsFontSizeMarkerData->setValue(p->Graphs.fontSizeMarkerData);
    ui->GraphsFontSizeTraceNames->setValue(p->Graphs.fontSizeTraceNames);
    ui->GraphsEnablePanZoom->setChecked(p->Graphs.enablePanAndZoom);
    ui->GraphsZoomFactor->setValue(p->Graphs.zoomFactor);
    ui->GraphsSweepTriangle->setChecked(p->Graphs.SweepIndicator.triangle);
    ui->GraphsSweepTriangleSize->setValue(p->Graphs.SweepIndicator.triangleSize);
    ui->GraphsSweepLine->setChecked(p->Graphs.SweepIndicator.line);
    ui->GraphsSweepHide->setChecked(p->Graphs.SweepIndicator.hide);
    ui->GraphsSweepHidePercent->setValue(p->Graphs.SweepIndicator.hidePercent);
    ui->graphsEnableMasterTicksForYAxis->setChecked(p->Graphs.enableMasterTicksForYAxis);
    for(unsigned int i=(int) YAxis::Type::Disabled + 1;i<(int) YAxis::Type::Last;i++) {
        auto type = (YAxis::Type) i;
        graphAxisLimitsMinEntries[type]->setValue(p->Graphs.defaultAxisLimits.min[i]);
        graphAxisLimitsMaxEntries[type]->setValue(p->Graphs.defaultAxisLimits.max[i]);
    }

    ui->MarkerShowMarkerData->setChecked(p->Marker.defaultBehavior.showDataOnGraphs);
    ui->MarkerShowdB->setChecked(p->Marker.defaultBehavior.showdB);
    ui->MarkerShowdBm->setChecked(p->Marker.defaultBehavior.showdBm);
    ui->MarkerShowdBUv->setChecked(p->Marker.defaultBehavior.showdBuV);
    ui->MarkerShowdBAngle->setChecked(p->Marker.defaultBehavior.showdBAngle);
    ui->MarkerShowRealImag->setChecked(p->Marker.defaultBehavior.showRealImag);
    ui->MarkerShowImpedance->setChecked(p->Marker.defaultBehavior.showImpedance);
    ui->MarkerShowVSWR->setChecked(p->Marker.defaultBehavior.showVSWR);
    ui->MarkerShowResistance->setChecked(p->Marker.defaultBehavior.showResistance);
    ui->MarkerShowCapacitance->setChecked(p->Marker.defaultBehavior.showCapacitance);
    ui->MarkerShowInductance->setChecked(p->Marker.defaultBehavior.showInductance);
    ui->MarkerShowQualityFactor->setChecked(p->Marker.defaultBehavior.showQualityFactor);
    ui->MarkerShowGroupDelay->setChecked(p->Marker.defaultBehavior.showGroupDelay);
    ui->MarkerShowNoise->setChecked(p->Marker.defaultBehavior.showNoise);
    ui->MarkerShowPhasenoise->setChecked(p->Marker.defaultBehavior.showPhasenoise);
    ui->MarkerShowCenterBandwidth->setChecked(p->Marker.defaultBehavior.showCenterBandwidth);
    ui->MarkerShowCutoff->setChecked(p->Marker.defaultBehavior.showCutoff);
    ui->MarkerShowInsertionLoss->setChecked(p->Marker.defaultBehavior.showInsertionLoss);
    ui->MarkerShowTOI->setChecked(p->Marker.defaultBehavior.showTOI);
    ui->MarkerShowAvgTone->setChecked(p->Marker.defaultBehavior.showAvgTone);
    ui->MarkerShowAvgMod->setChecked(p->Marker.defaultBehavior.showAvgModulation);
    ui->MarkerShowP1dB->setChecked(p->Marker.defaultBehavior.showP1dB);
    ui->MarkerShowFlatness->setChecked(p->Marker.defaultBehavior.showFlatness);
    ui->MarkerShowMaxDeltaNeg->setChecked(p->Marker.defaultBehavior.showMaxDeltaNeg);
    ui->MarkerShowMaxDeltaPos->setChecked(p->Marker.defaultBehavior.showMaxDeltaPos);
    ui->MarkerInterpolate->setCurrentIndex(p->Marker.interpolatePoints ? 1 : 0);
    ui->MarkerSortOrder->setCurrentIndex((int) p->Marker.sortOrder);
    ui->MarkerSymbolStyle->setCurrentIndex((int) p->Marker.symbolStyle);
    ui->MarkerClipToYAxis->setChecked(p->Marker.clipToYAxis);

    ui->SCPIServerEnabled->setChecked(p->SCPIServer.enabled);
    ui->SCPIServerPort->setValue(p->SCPIServer.port);

    ui->streamingServerVNArawEnabled->setChecked(p->StreamingServers.VNARawData.enabled);
    ui->streamingServerVNArawPort->setValue(p->StreamingServers.VNARawData.port);
    ui->streamingServerVNAcalibratedEnabled->setChecked(p->StreamingServers.VNACalibratedData.enabled);
    ui->streamingServerVNAcalibratedPort->setValue(p->StreamingServers.VNACalibratedData.port);
    ui->streamingServerVNAdeembeddedEnabled->setChecked(p->StreamingServers.VNADeembeddedData.enabled);
    ui->streamingServerVNAdeembeddedPort->setValue(p->StreamingServers.VNADeembeddedData.port);
    ui->streamingServerSArawEnabled->setChecked(p->StreamingServers.SARawData.enabled);
    ui->streamingServerSArawPort->setValue(p->StreamingServers.SARawData.port);
    ui->streamingServerSAnormalizedEnabled->setChecked(p->StreamingServers.SANormalizedData.enabled);
    ui->streamingServerSAnormalizedPort->setValue(p->StreamingServers.SANormalizedData.port);

    ui->DebugMaxUSBlogSize->setValue(p->Debug.USBlogSizeLimit);
    ui->DebugSaveTraceData->setChecked(p->Debug.saveTraceData);
    ui->DebugUseNativeDialogs->setChecked(p->Debug.useNativeDialogs);

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
    p->Startup.DefaultSweep.dwellTime = ui->StartupSweepDwellTime->value();
    p->Startup.Generator.frequency = ui->StartupGeneratorFrequency->value();
    p->Startup.Generator.level = ui->StartupGeneratorLevel->value();
    p->Startup.SA.start = ui->StartupSAStart->value();
    p->Startup.SA.stop = ui->StartupSAStop->value();
    p->Startup.SA.RBW = ui->StartupSARBW->value();
    p->Startup.SA.window = ui->StartupSAWindow->currentIndex();
    p->Startup.SA.detector = ui->StartupSADetector->currentIndex();

    p->Acquisition.alwaysExciteAllPorts = ui->AcquisitionAlwaysExciteBoth->isChecked();
    p->Acquisition.allowSegmentedSweep = ui->AcquisitionAllowSegmentedSweep->isChecked();
    p->Acquisition.useMedianAveraging = ui->AcquisitionAveragingMode->currentIndex() == 1;
    p->Acquisition.fullSpanManual = ui->AcquisitionFullSpanBehavior->currentIndex() == 1;
    p->Acquisition.fullSpanStart = ui->AcquisitionFullSpanStart->value();
    p->Acquisition.fullSpanStop = ui->AcquisitionFullSpanStop->value();
    p->Acquisition.fullSpanCalibratedRange = ui->AcquisitionFullSpanCalibrated->isChecked();
    p->Acquisition.limitDFT = ui->AcquisitionLimitTDRCheckbox->isChecked();
    p->Acquisition.maxDFTrate = ui->AcquisitionDFTLimitValue->value();
    p->Acquisition.groupDelaySamples = ui->AcquisitionGroupDelaySamples->value();

    p->Graphs.defaultGraphs.transmission = ui->GraphsDefaultTransmission->currentText();
    p->Graphs.defaultGraphs.reflection = ui->GraphsDefaultReflection->currentText();
    p->Graphs.showUnits = ui->GraphsShowUnit->isChecked();
    p->Graphs.Color.background = ui->GraphsColorBackground->getColor();
    p->Graphs.Color.axis = ui->GraphsColorAxis->getColor();
    p->Graphs.Color.Ticks.Background.enabled = ui->GraphsColorTicksBackgroundEnabled->isChecked();
    p->Graphs.Color.Ticks.Background.background = ui->GraphsColorTicksBackground->getColor();
    p->Graphs.Color.Ticks.divisions = ui->GraphsColorTicksDivisions->getColor();
    p->Graphs.domainChangeBehavior = (GraphDomainChangeBehavior) ui->GraphsDomainChangeBehavior->currentIndex();
    p->Graphs.limitIndication = (GraphLimitIndication) ui->GraphsLimitIndication->currentIndex();
    p->Graphs.limitNaNpasses = ui->GraphsLimitNaNpasses->currentIndex() == 1;
    p->Graphs.lineWidth = ui->GraphsLineWidth->value();
    p->Graphs.fontSizeTitle = ui->GraphsFontSizeTitle->value();
    p->Graphs.fontSizeAxis = ui->GraphsFontSizeAxis->value();
    p->Graphs.fontSizeCursorOverlay = ui->GraphsFontSizeCursorOverlay->value();
    p->Graphs.fontSizeMarkerData = ui->GraphsFontSizeMarkerData->value();
    p->Graphs.fontSizeTraceNames = ui->GraphsFontSizeTraceNames->value();
    p->Graphs.enablePanAndZoom = ui->GraphsEnablePanZoom->isChecked();
    p->Graphs.zoomFactor = ui->GraphsZoomFactor->value();
    p->Graphs.SweepIndicator.triangle = ui->GraphsSweepTriangle->isChecked();
    p->Graphs.SweepIndicator.triangleSize = ui->GraphsSweepTriangleSize->value();
    p->Graphs.SweepIndicator.line = ui->GraphsSweepLine->isChecked();
    p->Graphs.SweepIndicator.hide = ui->GraphsSweepHide->isChecked();
    p->Graphs.SweepIndicator.hidePercent = ui->GraphsSweepHidePercent->value();
    p->Graphs.enableMasterTicksForYAxis = ui->graphsEnableMasterTicksForYAxis->isChecked();
    for(unsigned int i=(int) YAxis::Type::Disabled + 1;i<(int) YAxis::Type::Last;i++) {
        auto type = (YAxis::Type) i;
        p->Graphs.defaultAxisLimits.min[i] = graphAxisLimitsMinEntries[type]->value();
        p->Graphs.defaultAxisLimits.max[i] = graphAxisLimitsMaxEntries[type]->value();
    }

    p->Marker.defaultBehavior.showDataOnGraphs = ui->MarkerShowMarkerData->isChecked();
    p->Marker.defaultBehavior.showdB = ui->MarkerShowdB->isChecked();
    p->Marker.defaultBehavior.showdBm = ui->MarkerShowdBm->isChecked();
    p->Marker.defaultBehavior.showdBuV = ui->MarkerShowdBUv->isChecked();
    p->Marker.defaultBehavior.showdBAngle = ui->MarkerShowdBAngle->isChecked();
    p->Marker.defaultBehavior.showRealImag = ui->MarkerShowRealImag->isChecked();
    p->Marker.defaultBehavior.showImpedance = ui->MarkerShowImpedance->isChecked();
    p->Marker.defaultBehavior.showVSWR = ui->MarkerShowVSWR->isChecked();
    p->Marker.defaultBehavior.showResistance = ui->MarkerShowResistance->isChecked();
    p->Marker.defaultBehavior.showCapacitance = ui->MarkerShowCapacitance->isChecked();
    p->Marker.defaultBehavior.showInductance = ui->MarkerShowInductance->isChecked();
    p->Marker.defaultBehavior.showQualityFactor = ui->MarkerShowQualityFactor->isChecked();
    p->Marker.defaultBehavior.showGroupDelay = ui->MarkerShowGroupDelay->isChecked();
    p->Marker.defaultBehavior.showNoise = ui->MarkerShowNoise->isChecked();
    p->Marker.defaultBehavior.showPhasenoise = ui->MarkerShowPhasenoise->isChecked();
    p->Marker.defaultBehavior.showCenterBandwidth = ui->MarkerShowCenterBandwidth->isChecked();
    p->Marker.defaultBehavior.showCutoff = ui->MarkerShowCutoff->isChecked();
    p->Marker.defaultBehavior.showInsertionLoss = ui->MarkerShowInsertionLoss->isChecked();
    p->Marker.defaultBehavior.showTOI = ui->MarkerShowTOI->isChecked();
    p->Marker.defaultBehavior.showAvgTone = ui->MarkerShowAvgTone->isChecked();
    p->Marker.defaultBehavior.showAvgModulation = ui->MarkerShowAvgMod->isChecked();
    p->Marker.defaultBehavior.showP1dB = ui->MarkerShowP1dB->isChecked();
    p->Marker.defaultBehavior.showFlatness = ui->MarkerShowFlatness->isChecked();
    p->Marker.defaultBehavior.showMaxDeltaNeg = ui->MarkerShowMaxDeltaNeg->isChecked();
    p->Marker.defaultBehavior.showMaxDeltaPos = ui->MarkerShowMaxDeltaPos->isChecked();
    p->Marker.interpolatePoints = ui->MarkerInterpolate->currentIndex() == 1;
    p->Marker.sortOrder = (MarkerSortOrder) ui->MarkerSortOrder->currentIndex();
    p->Marker.symbolStyle = (MarkerSymbolStyle) ui->MarkerSymbolStyle->currentIndex();
    p->Marker.clipToYAxis = ui->MarkerClipToYAxis->isChecked();

    p->SCPIServer.enabled = ui->SCPIServerEnabled->isChecked();
    p->SCPIServer.port = ui->SCPIServerPort->value();

    p->StreamingServers.VNARawData.enabled = ui->streamingServerVNArawEnabled->isChecked();
    p->StreamingServers.VNARawData.port = ui->streamingServerVNArawPort->value();
    p->StreamingServers.VNACalibratedData.enabled = ui->streamingServerVNAcalibratedEnabled->isChecked();
    p->StreamingServers.VNACalibratedData.port = ui->streamingServerVNAcalibratedPort->value();
    p->StreamingServers.VNADeembeddedData.enabled = ui->streamingServerVNAdeembeddedEnabled->isChecked();
    p->StreamingServers.VNADeembeddedData.port = ui->streamingServerVNAdeembeddedPort->value();
    p->StreamingServers.SARawData.enabled = ui->streamingServerSArawEnabled->isChecked();
    p->StreamingServers.SARawData.port = ui->streamingServerSArawPort->value();
    p->StreamingServers.SANormalizedData.enabled = ui->streamingServerSAnormalizedEnabled->isChecked();
    p->StreamingServers.SANormalizedData.port = ui->streamingServerSAnormalizedPort->value();

    p->Debug.USBlogSizeLimit = ui->DebugMaxUSBlogSize->value();
    p->Debug.saveTraceData = ui->DebugSaveTraceData->isChecked();
    p->Debug.useNativeDialogs = ui->DebugUseNativeDialogs->isChecked();

    p->nonTrivialWriting();
}

Preferences::~Preferences()
{

}

void Preferences::load()
{
    // load settings, using default values if not present
    qInfo() << "Loading preferences";
    load(descr);
    for(auto driver : DeviceDriver::getDrivers()) {
        driver->registerTypes();
        load(driver->driverSpecificSettings());
    }
    nonTrivialParsing();
}

void Preferences::load(std::vector<Savable::SettingDescription> descr)
{
    QSettings settings;
    for(auto d : descr) {
        try {
            d.var.setValue(settings.value(d.name, d.def));
        } catch (const exception& e){
            d.var.setValue(d.def);
        }
    }
}

void Preferences::store()
{
    nonTrivialWriting();
    store(descr);
    for(auto driver : DeviceDriver::getDrivers()) {
        store(driver->driverSpecificSettings());
    }
}

void Preferences::store(std::vector<Savable::SettingDescription> descr)
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
    for(auto driver : DeviceDriver::getDrivers()) {
        setDefault(driver->driverSpecificSettings());
    }
}

void Preferences::setDefault(std::vector<Savable::SettingDescription> descr)
{
    for(auto d : descr) {
        d.var.setValue(d.def);
    }
}

QFileDialog::Options Preferences::QFileDialogOptions(QFileDialog::Options option)
{
    if(!instance.Debug.useNativeDialogs) {
        option = (QFileDialog::Option) ((int) option | QFileDialog::DontUseNativeDialog);
    }
    return option;
}

void Preferences::fromJSON(nlohmann::json j)
{
    parseJSON(j, descr);
    for(auto driver : DeviceDriver::getDrivers()) {
        Savable::parseJSON(j, driver->driverSpecificSettings());
    }
    nonTrivialParsing();
}

static nlohmann::json merge(const nlohmann::json &a, const nlohmann::json &b)
{
    nlohmann::json result = a.flatten();
    nlohmann::json tmp = b.flatten();

    for (nlohmann::json::iterator it = tmp.begin(); it != tmp.end(); ++it)
    {
        result[it.key()] = it.value();
    }

    return result.unflatten();
}

nlohmann::json Preferences::toJSON()
{
    nonTrivialWriting();
    auto j = createJSON(descr);
    for(auto driver : DeviceDriver::getDrivers()) {
        j = merge(j, Savable::createJSON(driver->driverSpecificSettings()));
    }

    return j;
}

bool Preferences::set(QString name, QVariant value)
{
    QPointerVariant *ptr = nullptr;
    for(auto s : descr) {
        if(s.name == name) {
            ptr = &s.var;
            break;
        }
    }
    if(!ptr) {
        // check the driver settings
        for(auto driver : DeviceDriver::getDrivers()) {
            for(auto s : driver->driverSpecificSettings()) {
                if(s.name == name) {
                    ptr = &s.var;
                    break;
                }
            }
            if(ptr) {
                break;
            }
        }
    }
    if(ptr) {
        try {
            ptr->setValue(value);
            return true;
        } catch (const std::runtime_error&) {
            // failed to set variable, likely wrong format for the QVariant
            return false;
        }
    } else {
        // not found
        return false;
    }
}

QVariant Preferences::get(QString name)
{
    for(auto &s : descr) {
        if(s.name == name) {
            return s.var.value();
        }
    }
    for(auto driver : DeviceDriver::getDrivers()) {
        for(auto &s : driver->driverSpecificSettings()) {
            if(s.name == name) {
                return s.var.value();
            }
        }
    }
    // not found
    return QVariant();
}

void Preferences::nonTrivialParsing()
{

}

void Preferences::nonTrivialWriting()
{

}
