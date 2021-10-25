#include "measurementmodel.h"

#include "../unit.h"

#include <algorithm>

MeasurementModel::MeasurementModel(Calibration *cal, std::vector<Calibration::Measurement> measurements) :
    QAbstractTableModel(),
    cal(cal),
    measurements(measurements)
{

}

int MeasurementModel::rowCount(const QModelIndex &) const
{
    return measurements.size();
}

int MeasurementModel::columnCount(const QModelIndex &) const
{
    return ColIndexLast;
}

QVariant MeasurementModel::data(const QModelIndex &index, int role) const
{
    auto info = cal->getMeasurementInfo(measurements[index.row()]);
    if(role == Qt::DisplayRole) {
        switch(index.column()) {
        case ColIndexName:
            return info.name;
            break;
        case ColIndexDescription:
            return info.prerequisites;
            break;
        case ColIndexData:
            if(info.points > 0) {
                QString data = QString::number(info.points);
                data.append(" points from ");
                data.append(Unit::ToString(info.fmin, "Hz", " kMG"));
                data.append(" to ");
                data.append(Unit::ToString(info.fmax, "Hz", " kMG"));
                return data;
            } else {
                return "Not available";
            }
            break;
        case ColIndexDate:
            return info.timestamp.toString("dd.MM.yyyy hh:mm:ss");
            break;
        }
    } else if(role == Qt::SizeHintRole) {
        switch(index.column()) {
        case ColIndexName: return 200; break;
        case ColIndexDescription: return 500; break;
        case ColIndexData: return 300; break;
        case ColIndexDate: return 300; break;
        default: return QVariant(); break;
        }
    }

    return QVariant();
}

QVariant MeasurementModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case ColIndexName: return "Type"; break;
        case ColIndexDescription: return "Prerequisites"; break;
        case ColIndexData: return "Statistics"; break;
        case ColIndexDate: return "Timestamp"; break;
        default: return QVariant(); break;
        }
    } else {
        return QVariant();
    }
}

void MeasurementModel::measurementUpdated(Calibration::Measurement m)
{
    // find correct index in vector
    auto it = std::find(measurements.begin(), measurements.end(), m);
    if(it != measurements.end()) {
        int row = it - measurements.begin();
        emit dataChanged(index(row, 0), index(row, ColIndexLast - 1));
    }
}
