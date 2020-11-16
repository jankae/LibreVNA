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
        double correctionPort1;
        double correctionPort2;
        double amplitudePort1;
        double amplitudePort2;
    };
    std::vector<CorrectionPoint> getPoints() const;
    void setAmplitude(double amplitude, unsigned int point, bool port2);

protected slots:
    void ReceivedPoint(Protocol::AmplitudeCorrectionPoint p);
    void LoadFromDevice();
    void SaveToDevice();
    void RemovePoint();
    void AddPoint();
signals:
    void pointsUpdated();
protected:
    virtual Protocol::PacketType requestCommand() = 0;
    virtual Protocol::PacketType pointType() = 0;
    std::vector<CorrectionPoint> points;
    Ui::AmplitudeCalDialog *ui;
    Device *dev;
    Mode *activeMode;
    AmplitudeModel model;
};

#endif // SOURCECALDIALOG_H
