#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QVariant>
#include <exception>
#include "Util/qpointervariant.h"

class Preferences {
public:
    static Preferences& getInstance() {
        return instance;
    }
    void load();
    void store();
    void edit();
    void setDefault();

    void manualTCPport() { TCPoverride = true; }

    struct {
        bool ConnectToFirstDevice;
        bool RememberSweepSettings;
        struct {
            double start;
            double stop;
            int points;
            double bandwidth;
            double excitation;
            int averaging;
        } DefaultSweep;
        struct {
            double frequency;
            double level;
        } Generator;
        struct {
            double start;
            double stop;
            double RBW;
            int window;
            int detector;
            int averaging;
            bool signalID;
        } SA;
    } Startup;
    struct {
        bool alwaysExciteBothPorts;
        bool suppressPeaks;
        bool harmonicMixing;
        bool useDFTinSAmode;
        double RBWLimitForDFT;
    } Acquisition;
    struct {
        struct {
            QColor background;
            QColor axis;
            QColor divisions;
        } graphColors;
        struct {
            bool showDataOnGraphs;
            bool showAllData;
        } markerDefault;
        struct {
            bool enabled;
            int port;
        } SCPI;
    } General;

    bool TCPoverride; // in case of manual port specification via command line
private:
    Preferences() :
     TCPoverride(false) {};
    static Preferences instance;
    using SettingDescription = struct {
        QPointerVariant var;
        QString name;
        QVariant def;
    };
    const std::array<SettingDescription, 29> descr = {{
        {&Startup.ConnectToFirstDevice, "Startup.ConnectToFirstDevice", true},
        {&Startup.RememberSweepSettings, "Startup.RememberSweepSettings", false},
        {&Startup.DefaultSweep.start, "Startup.DefaultSweep.start", 1000000.0},
        {&Startup.DefaultSweep.stop, "Startup.DefaultSweep.stop", 6000000000.0},
        {&Startup.DefaultSweep.points, "Startup.DefaultSweep.points", 501},
        {&Startup.DefaultSweep.bandwidth, "Startup.DefaultSweep.bandwidth", 1000.0},
        {&Startup.DefaultSweep.excitation, "Startup.DefaultSweep.excitation", -10.00},
        {&Startup.DefaultSweep.averaging, "Startup.DefaultSweep.averaging", 1},
        {&Startup.Generator.frequency, "Startup.Generator.frequency", 1000000000.0},
        {&Startup.Generator.level, "Startup.Generator.level", -10.00},
        {&Startup.SA.start, "Startup.SA.start", 950000000.0},
        {&Startup.SA.stop, "Startup.SA.stop", 1050000000.0},
        {&Startup.SA.RBW, "Startup.SA.RBW", 10000.0},
        {&Startup.SA.window, "Startup.SA.window", 1},
        {&Startup.SA.detector, "Startup.SA.detector", 0},
        {&Startup.SA.averaging, "Startup.SA.averaging", 1},
        {&Startup.SA.signalID, "Startup.SA.signalID", true},
        {&Acquisition.alwaysExciteBothPorts, "Acquisition.alwaysExciteBothPorts", true},
        {&Acquisition.suppressPeaks, "Acquisition.suppressPeaks", true},
        {&Acquisition.harmonicMixing, "Acquisition.harmonicMixing", false},
        {&Acquisition.useDFTinSAmode, "Acquisition.useDFTinSAmode", true},
        {&Acquisition.RBWLimitForDFT, "Acquisition.RBWLimitForDFT", 3000.0},
        {&General.graphColors.background, "General.graphColors.background", QColor(Qt::black)},
        {&General.graphColors.axis, "General.graphColors.axis", QColor(Qt::white)},
        {&General.graphColors.divisions, "General.graphColors.divisions", QColor(Qt::gray)},
        {&General.markerDefault.showDataOnGraphs, "General.MarkerDefault.ShowDataOnGraphs", true},
        {&General.markerDefault.showAllData, "General.MarkerDefault.ShowAllData", false},
        {&General.SCPI.enabled, "General.SCPI.enabled", true},
        {&General.SCPI.port, "General.SCPI.port", 19542},
    }};
};

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(Preferences *pref, QWidget *parent = nullptr);
    ~PreferencesDialog();

private:
    void setInitialGUIState();
    Ui::PreferencesDialog *ui;
    Preferences *p;
};

#endif // PREFERENCESDIALOG_H
