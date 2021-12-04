#ifndef MEASUREMENTMODEL_H
#define MEASUREMENTMODEL_H

#include "calibration.h"

#include <QObject>
#include <QWidget>
#include <QAbstractTableModel>

class MeasurementModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum class ColIndex {
        Name,
        Gender,
        Description,
        Data,
        Date,
        Last
    };

    MeasurementModel(Calibration *cal, std::vector<Calibration::Measurement> measurements);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public slots:
    void measurementUpdated(Calibration::Measurement m);
    void genderUpdated();

private:
    Calibration *cal;
    std::vector<Calibration::Measurement> measurements;
};

#endif // MEASUREMENTMODEL_H
