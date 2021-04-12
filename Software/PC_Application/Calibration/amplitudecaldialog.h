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
    bool AddPoint(CorrectionPoint& p);
    bool AddPoint(double frequency);
    void AddPointDialog();
    void AutomaticMeasurementDialog();
    void ReceivedMeasurement(Protocol::SpectrumAnalyzerResult res);
signals:
    void pointsUpdated();
    void newPointCreated(CorrectionPoint& p);
protected:
    static constexpr double excitationAmplitude = -20.0;
    static constexpr int averages = 20;
    static constexpr int automaticSettling = 10;

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

    void SetupNextAutomaticPoint(bool isSourceCal);
    void ReceivedAutomaticMeasurementResult(Protocol::SpectrumAnalyzerResult res);
    struct {
        QDialog *dialog;
        std::vector<CorrectionPoint> points;
        bool measuringPort2; // true if port2 is being calibrated
        unsigned int measuringCount; // number of calibration point
        unsigned int settlingCount; // number of measurements still to ignore before taking measurement
        bool isSourceCal;
        QProgressBar *progress;
    } automatic;

    // raw (uncorrected) measurements from device.
    // Used in reveicer calibration as well as in automatic calibrations
    struct MeasurementResult {
        double port1, port2;
    };
    std::deque<MeasurementResult> measured;
    MeasurementResult averageMeasurement();
};

#endif // SOURCECALDIALOG_H
