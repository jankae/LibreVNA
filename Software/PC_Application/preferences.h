#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QVariant>
#include <exception>

class QPointerVariant {
public:
    template <typename T> QPointerVariant(T *ptr)
        : ptr(static_cast<void*>(ptr)),
          variant(QVariant(*ptr)){};
    void setValue(const QVariant &value) {
        auto destType = variant.type();
        if(!value.canConvert(destType)) {
            throw std::runtime_error("Unable to convert QVariant to requested type");
        }
        variant = value;
        variant.convert(destType);
        QMetaType mt(destType);
        mt.construct(ptr, variant.constData());
    }
    QVariant value() {
        return QVariant(variant.type(), ptr);
    }
private:
    void *ptr;
    QVariant variant;
};

class Preferences {
public:
    void load();
    void store();
    void edit();
    void setDefault();

    struct {
        bool ConnectToFirstDevice;
        bool RememberSweepSettings;
        struct {
            double start;
            double stop;
            int points;
            double bandwidth;
            double excitation;
        } DefaultSweep;
    } Startup;
    struct {
        bool alwaysExciteBothPorts;
    } Acquisition;
private:
    using SettingDescription = struct {
        QPointerVariant var;
        QString name;
        QVariant def;
    };
    const std::array<SettingDescription, 8> descr = {{
        {&Startup.ConnectToFirstDevice, "Startup.ConnectToFirstDevice", true},
        {&Startup.RememberSweepSettings, "Startup.RememberSweepSettings", false},
        {&Startup.DefaultSweep.start, "Startup.DefaultSweep.start", 1000000.0},
        {&Startup.DefaultSweep.stop, "Startup.DefaultSweep.stop", 6000000000.0},
        {&Startup.DefaultSweep.points, "Startup.DefaultSweep.points", 501},
        {&Startup.DefaultSweep.bandwidth, "Startup.DefaultSweep.bandwidth", 1000.0},
        {&Startup.DefaultSweep.excitation, "Startup.DefaultSweep.excitation", -10.00},
        {&Acquisition.alwaysExciteBothPorts, "Acquisition.alwaysExciteBothPorts", true},
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
