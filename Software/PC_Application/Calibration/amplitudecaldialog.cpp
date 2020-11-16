#include "amplitudecaldialog.h"
#include "ui_amplitudecaldialog.h"
#include "mode.h"
#include "unit.h"
#include <QDebug>

AmplitudeCalDialog::AmplitudeCalDialog(Device *dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AmplitudeCalDialog),
    dev(dev),
    model(this)
{
    activeMode = Mode::getActiveMode();
    activeMode->deactivate();
    dev->SetIdle();
    ui->setupUi(this);
    ui->view->setModel(&model);
    ui->view->setColumnWidth(AmplitudeModel::ColIndexFreq, 100);
    ui->view->setColumnWidth(AmplitudeModel::ColIndexCorrectionFactors, 150);
    ui->view->setColumnWidth(AmplitudeModel::ColIndexPort1, 150);
    ui->view->setColumnWidth(AmplitudeModel::ColIndexPort2, 150);
    connect(dev, &Device::AmplitudeCorrectionPointReceived, this, &AmplitudeCalDialog::ReceivedPoint);
    connect(ui->load, &QPushButton::clicked, this, &AmplitudeCalDialog::LoadFromDevice);
    connect(ui->add, &QPushButton::clicked, this, &AmplitudeCalDialog::AddPoint);
    connect(ui->remove, &QPushButton::clicked, this, &AmplitudeCalDialog::RemovePoint);
}

AmplitudeCalDialog::~AmplitudeCalDialog()
{
    delete ui;
    activeMode->activate();
}

void AmplitudeCalDialog::reject()
{
    // TODO check for unsaved data
    delete this;
}

std::vector<AmplitudeCalDialog::CorrectionPoint> AmplitudeCalDialog::getPoints() const
{
    return points;
}

void AmplitudeCalDialog::setAmplitude(double amplitude, unsigned int point, bool port2)
{
    if(point >= points.size()) {
        return;
    }
    if(port2) {
        points[point].amplitudePort2 = amplitude;
    } else {
        points[point].amplitudePort1 = amplitude;
    }
}

void AmplitudeCalDialog::ReceivedPoint(Protocol::AmplitudeCorrectionPoint p)
{
    qDebug() << "Received a point";
    CorrectionPoint c;
    c.frequency = p.freq * 10.0;
    c.correctionPort1 = p.port1;
    c.correctionPort2 = p.port2;
    model.beginInsertRows(QModelIndex(), points.size(), points.size());
    points.push_back(c);
    model.endInsertRows();
    emit pointsUpdated();
}

void AmplitudeCalDialog::LoadFromDevice()
{
    model.beginResetModel();
    points.clear();
    model.endResetModel();
    qDebug() << "Asking for amplitude calibration";
    dev->SendCommandWithoutPayload(requestCommand());
}

void AmplitudeCalDialog::SaveToDevice()
{
    for(unsigned int i=0;i<points.size();i++) {
        auto p = points[i];
        Protocol::PacketInfo info;
        info.type = pointType();
        info.amplitudePoint.freq = p.frequency / 10.0;
        info.amplitudePoint.port1 = p.correctionPort1;
        info.amplitudePoint.port2 = p.correctionPort2;
        info.amplitudePoint.totalPoints = points.size();
        info.amplitudePoint.pointNum = i;
        dev->SendPacket(info);
    }
}

void AmplitudeCalDialog::RemovePoint()
{
    unsigned int row = ui->view->currentIndex().row();
    if(row < points.size()) {
        model.beginRemoveRows(QModelIndex(), row, row);
        points.erase(points.begin() + row);
        model.endInsertRows();
    }
}

void AmplitudeCalDialog::AddPoint()
{

}

AmplitudeModel::AmplitudeModel(AmplitudeCalDialog *c) :
    QAbstractTableModel(),
    c(c)
{
}

int AmplitudeModel::rowCount(const QModelIndex &parent) const
{
    return c->getPoints().size();
}

int AmplitudeModel::columnCount(const QModelIndex &parent) const
{
    return ColIndexLast;
}

QVariant AmplitudeModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::DisplayRole && index.row() < c->getPoints().size()) {
        auto p = c->getPoints()[index.row()];
        switch(index.column()) {
        case ColIndexFreq:
            return Unit::ToString(p.frequency, "Hz", " kMG", 6);
            break;
        case ColIndexCorrectionFactors:
            return QString::number(p.correctionPort1) + ", " + QString::number(p.correctionPort2);
            break;
        case ColIndexPort1:
            return Unit::ToString(p.amplitudePort1, "dbm", " ", 4);
            break;
        case ColIndexPort2:
            return Unit::ToString(p.amplitudePort2, "dbm", " ", 4);
            break;
        }
    }
    return QVariant();
}

bool AmplitudeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if((unsigned int) index.row() >= c->getPoints().size()) {
        return false;
    }
    switch(index.column()) {
    case ColIndexPort1:
        c->setAmplitude(value.toDouble(), index.row(), false);
        return true;
    case ColIndexPort2:
        c->setAmplitude(value.toDouble(), index.row(), true);
        return true;
    }

    return false;
}

QVariant AmplitudeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case ColIndexFreq: return "Frequency"; break;
        case ColIndexCorrectionFactors: return "Correction Factors"; break;
        case ColIndexPort1: return "Amplitude Port 1"; break;
        case ColIndexPort2: return "Amplitude Port 2"; break;
        default: return QVariant(); break;
        }
    } else {
        return QVariant();
    }
}

Qt::ItemFlags AmplitudeModel::flags(const QModelIndex &index) const
{
    int flags = Qt::NoItemFlags;
    switch(index.column()) {
    case ColIndexPort1: flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable; break;
    case ColIndexPort2: flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable; break;
    }
    return (Qt::ItemFlags) flags;
}
