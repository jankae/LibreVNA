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
    return (int) ColIndex::Last;
}

QVariant MeasurementModel::data(const QModelIndex &index, int role) const
{
    auto info = cal->getMeasurementInfo(measurements[index.row()]);
    if(role == Qt::DisplayRole) {
        switch((ColIndex) index.column()) {
        case ColIndex::Name:
            return info.name;
            break;
        case ColIndex::Gender:
            switch(measurements[index.row()]) {
            case Calibration::Measurement::Port1Load:
            case Calibration::Measurement::Port1Open:
            case Calibration::Measurement::Port1Short:
                if(cal->getPortStandard(1) == Calibration::PortStandard::Male) {
                    return "Male";
                } else {
                    return "Female";
                }
                break;
            case Calibration::Measurement::Port2Load:
            case Calibration::Measurement::Port2Open:
            case Calibration::Measurement::Port2Short:
                if(cal->getPortStandard(2) == Calibration::PortStandard::Male) {
                    return "Male";
                } else {
                    return "Female";
                }
                break;
            default:
                return "";
            }

            break;
        case ColIndex::Description:
            return info.prerequisites;
            break;
        case ColIndex::Data:
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
        case ColIndex::Date:
            return info.timestamp.toString("dd.MM.yyyy hh:mm:ss");
            break;
        }
    } else if(role == Qt::SizeHintRole) {
        switch((ColIndex) index.column()) {
        case ColIndex::Name: return 200; break;
        case ColIndex::Gender: return 150; break;
        case ColIndex::Description: return 500; break;
        case ColIndex::Data: return 300; break;
        case ColIndex::Date: return 300; break;
        default: return QVariant(); break;
        }
    }

    return QVariant();
}

QVariant MeasurementModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch((ColIndex) section) {
        case ColIndex::Name: return "Type"; break;
        case ColIndex::Gender: return "Gender"; break;
        case ColIndex::Description: return "Prerequisites"; break;
        case ColIndex::Data: return "Statistics"; break;
        case ColIndex::Date: return "Timestamp"; break;
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
        emit dataChanged(index(row, 0), index(row, (int) ColIndex::Last - 1));
    }
}

void MeasurementModel::genderUpdated()
{
    emit dataChanged(index(0, (int) ColIndex::Gender), index(rowCount() - 1, (int) ColIndex::Gender));
}
