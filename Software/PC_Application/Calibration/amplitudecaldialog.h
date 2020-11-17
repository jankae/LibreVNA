#ifndef AMPLITUDECALDIALOG_H
#define AMPLITUDECALDIALOG_H

#include <QDialog>
#include "mode.h"
#include "Device/device.h"

namespace Ui {
class AmplitudeCalDialog;
}

class AmplitudeCalDialog;

class AmplitudeModel : public QAbstractTableModel
{
    friend AmplitudeCalDialog;
    Q_OBJECT
public:
    AmplitudeModel(AmplitudeCalDialog *c);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    enum {
        ColIndexFreq,
        ColIndexCorrectionFactors,
        ColIndexPort1,
        ColIndexPort2,
        ColIndexLast
    };
    AmplitudeCalDialog *c;
};

class AmplitudeCalDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AmplitudeCalDialog(Device *dev, QWidget *parent = nullptr);
    ~AmplitudeCalDialog();
    void reject() override;

    class CorrectionPoint {
    public:
        double frequency;
        int16_t correctionPort1;
        int16_t correctionPort2;
        double amplitudePort1;
        double amplitudePort2;
        bool port1set;
        bool port2set;
    };
    std::vector<CorrectionPoint> getPoints() const;
    void setAmplitude(double amplitude, unsigned int point, bool port2);

    enum class CalibrationMode {
        BothPorts,
        OnlyPort1,
        OnlyPort2,
    };

    CalibrationMode getMode() const;

protected slots:
    void ReceivedPoint(Protocol::AmplitudeCorrectionPoint p);
    void LoadFromDevice();
    void SaveToDevice();
    void RemovePoint(unsigned int i);
    void RemoveAllPoints();
    void AddPoint(double frequency);
    void AddPointDialog();
signals:
    void pointsUpdated();
    void newPointCreated(CorrectionPoint& p);
protected:
    bool ConfirmActionIfEdited();
    void UpdateSaveButton();
    virtual Protocol::PacketType requestCommand() = 0;
    virtual Protocol::PacketType pointType() = 0;
    // will get called whenever a new cell is selected (frequency=0 means invalid selection)
    virtual void SelectedPoint(double frequency, bool port2) = 0;
    // will get called whenever the amplitude is changed. Derived class is responsible for updating correction factor
    virtual void AmplitudeChanged(CorrectionPoint& point, bool port2) = 0;
    // called whenver the correction factor have been retrieved from the device and the amplitudes need to be updated
    virtual void UpdateAmplitude(CorrectionPoint& point) = 0;
    std::vector<CorrectionPoint> points;
    Ui::AmplitudeCalDialog *ui;
    Device *dev;
    Mode *activeMode;
    AmplitudeModel model;
    bool edited;
    CalibrationMode mode;
};

#endif // SOURCECALDIALOG_H
