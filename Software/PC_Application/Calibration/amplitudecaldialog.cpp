#include "amplitudecaldialog.h"
#include "ui_amplitudecaldialog.h"
#include "mode.h"
#include "unit.h"
#include <QDebug>
#include "ui_addamplitudepointsdialog.h"
#include <QMessageBox>

using namespace std;

AmplitudeCalDialog::AmplitudeCalDialog(Device *dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AmplitudeCalDialog),
    dev(dev),
    model(this),
    mode(CalibrationMode::BothPorts)
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
    connect(ui->load, &QPushButton::clicked, [=](){
        if(ConfirmActionIfEdited()) {
            LoadFromDevice();
        }
    });
    connect(ui->save, &QPushButton::clicked, this, &AmplitudeCalDialog::SaveToDevice);
    connect(ui->add, &QPushButton::clicked, this, &AmplitudeCalDialog::AddPointDialog);
    connect(ui->remove, &QPushButton::clicked, [=](){
        unsigned int row = ui->view->currentIndex().row();
        if(row < points.size()) {
            RemovePoint(row);
        }
    });
    auto selectionModel = ui->view->selectionModel();
    connect(selectionModel, &QItemSelectionModel::currentChanged, [=](const QModelIndex &current, const QModelIndex&) {
        if(current.isValid() && (current.column() == model.ColIndexPort1 || current.column() == model.ColIndexPort2)) {
            SelectedPoint(points[current.row()].frequency, current.column() == model.ColIndexPort2);
        } else {
            // Invalid selection
            SelectedPoint(0, false);
        }
    });
    connect(ui->modeBoth, &QPushButton::pressed, [=](){
        mode = CalibrationMode::BothPorts;
        model.dataChanged(model.index(0, model.ColIndexPort1), model.index(points.size(), model.ColIndexPort2));
    });
    connect(ui->modePort1, &QPushButton::pressed, [=](){
        mode = CalibrationMode::OnlyPort1;
        model.dataChanged(model.index(0, model.ColIndexPort1), model.index(points.size(), model.ColIndexPort2));
    });
    connect(ui->modePort2, &QPushButton::pressed, [=](){
        mode = CalibrationMode::OnlyPort2;
        model.dataChanged(model.index(0, model.ColIndexPort1), model.index(points.size(), model.ColIndexPort2));
    });
}

AmplitudeCalDialog::~AmplitudeCalDialog()
{
    delete ui;
    activeMode->activate();
}

void AmplitudeCalDialog::reject()
{
    if(ConfirmActionIfEdited()) {
        delete this;
    }
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
        points[point].port2set = true;
    } else {
        points[point].amplitudePort1 = amplitude;
        points[point].port1set = true;
    }
    edited = true;
    AmplitudeChanged(points[point], port2);
    if(mode == CalibrationMode::OnlyPort1 && !port2) {
        // apply result from port 1 to port 2 as well
        points[point].correctionPort2 = points[point].correctionPort1;
        points[point].port2set = points[point].port1set;
    } else if(mode == CalibrationMode::OnlyPort2 && port2) {
        // apply result from port 2 to port 1 as well
        points[point].correctionPort1 = points[point].correctionPort2;
        points[point].port1set = points[point].port2set;
    }
    model.dataChanged(model.index(point, model.ColIndexCorrectionFactors), model.index(point, model.ColIndexPort2));
    UpdateSaveButton();
}

void AmplitudeCalDialog::ReceivedPoint(Protocol::AmplitudeCorrectionPoint p)
{
    qDebug() << "Received a point";
    CorrectionPoint c;
    c.frequency = p.freq * 10.0;
    c.correctionPort1 = p.port1;
    c.correctionPort2 = p.port2;
    c.port1set = false;
    c.port2set = false;
    model.beginInsertRows(QModelIndex(), points.size(), points.size());
    UpdateAmplitude(c);
    points.push_back(c);
    model.endInsertRows();
    emit pointsUpdated();
}

void AmplitudeCalDialog::LoadFromDevice()
{
    dev->SetIdle();
    RemoveAllPoints();
    qDebug() << "Asking for amplitude calibration";
    dev->SendCommandWithoutPayload(requestCommand());
    edited = false;
    ui->save->setEnabled(false);
}

void AmplitudeCalDialog::SaveToDevice()
{
    dev->SetIdle();
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
    edited = false;
    ui->save->setEnabled(false);
}

void AmplitudeCalDialog::RemovePoint(unsigned int i)
{
    model.beginRemoveRows(QModelIndex(), i, i);
    points.erase(points.begin() + i);
    model.endRemoveRows();
    edited = true;
    UpdateSaveButton();
}

void AmplitudeCalDialog::RemoveAllPoints()
{
    model.beginResetModel();
    points.clear();
    model.endResetModel();
    edited = true;
    ui->save->setEnabled(false);
    UpdateSaveButton();
}

void AmplitudeCalDialog::AddPoint(double frequency)
{
    if(points.size() >= Device::Info().limits_maxAmplitudePoints) {
        qWarning() << "Unable to add amplitude point, already maximum limit (" << Device::Info().limits_maxAmplitudePoints << ")";
        return;
    }
    // find position at which this frequency gets inserted
    auto index = upper_bound(points.begin(), points.end(), frequency, [](double value, const CorrectionPoint& p){
        return value < p.frequency;
    });
    model.beginInsertRows(QModelIndex(), index - points.begin(), index - points.begin());
    CorrectionPoint newPoint;
    newPoint.frequency = frequency;
    newPoint.correctionPort1 = 0;
    newPoint.correctionPort2 = 0;
    newPoint.port1set = false;
    newPoint.port2set = false;
    emit newPointCreated(newPoint);
    points.insert(index, newPoint);
    model.endInsertRows();
    edited = true;
    UpdateSaveButton();
}

void AmplitudeCalDialog::AddPointDialog()
{
    auto d = new QDialog();
    auto ui = new Ui::AddAmplitudePointsDialog();
    ui->setupUi(d);
    ui->frequency->setUnit("Hz");
    ui->frequency->setPrefixes(" kMG");
    ui->startFreq->setUnit("Hz");
    ui->startFreq->setPrefixes(" kMG");
    ui->stopFreq->setUnit("Hz");
    ui->stopFreq->setPrefixes(" kMG");
    ui->frequency->setValue(1000000000.0);
    ui->startFreq->setValue(Device::Info().limits_minFreq);
    ui->stopFreq->setValue(Device::Info().limits_maxFreq);
    connect(ui->singlePoint, &QRadioButton::toggled, [=](bool single) {
        ui->stopFreq->setEnabled(!single);
        ui->startFreq->setEnabled(!single);
        ui->numPoints->setEnabled(!single);
        ui->frequency->setEnabled(single);
    });
    ui->singlePoint->setChecked(true);
    ui->frequency->setFocus();

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){
        // okay clicked, apply selected action
        if(ui->deleteExisting->isChecked()) {
            RemoveAllPoints();
        }
        if(ui->singlePoint->isChecked()) {
            AddPoint(ui->frequency->value());
        } else {
            double freq_start = ui->startFreq->value();
            double freq_stop = ui->stopFreq->value();
            unsigned int points = ui->numPoints->value();
            double freq_step = (freq_stop - freq_start) / (points - 1);
            for(unsigned int i=0;i<points;i++) {
                AddPoint(freq_start + i * freq_step);
            }
        }
    });

    connect(d, &QDialog::rejected, ui->buttonBox, &QDialogButtonBox::rejected);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, [=](){
        // aborted, nothing to do
        delete d;
    });
    d->show();
}

bool AmplitudeCalDialog::ConfirmActionIfEdited()
{
    if(edited) {
        auto reply = QMessageBox::question(this, "Confirm action", "Some points have been edited but not saved in the device yet. If you continue, all changes will be lost. Do you want to continue?",
                                        QMessageBox::Yes|QMessageBox::No);
        return reply == QMessageBox::Yes;
    } else {
        // not edited yet, nothing to confirm
        return true;
    }
}

void AmplitudeCalDialog::UpdateSaveButton()
{
    bool enable = true;
    if(points.size() == 0) {
        // needs at least one point
        enable = false;
    }
    for(auto p : points) {
        if(!p.port1set || !p.port2set) {
            // some points are not set yet
            enable = false;
            break;
        }
    }
    ui->save->setEnabled(enable);
}

AmplitudeCalDialog::CalibrationMode AmplitudeCalDialog::getMode() const
{
    return mode;
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
            if (p.port1set) {
                return Unit::ToString(p.amplitudePort1, "dbm", " ", 4);
            } else {
                return "No data";
            }
            break;
        case ColIndexPort2:
            if (p.port2set) {
                return Unit::ToString(p.amplitudePort2, "dbm", " ", 4);
            } else {
                return "No data";
            }
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
    case ColIndexPort1:
        if(c->getMode() != AmplitudeCalDialog::CalibrationMode::OnlyPort2) {
            flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable;
        }
        break;
    case ColIndexPort2:
        if(c->getMode() != AmplitudeCalDialog::CalibrationMode::OnlyPort1) {
            flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable;
        }
        break;
    }
    return (Qt::ItemFlags) flags;
}
