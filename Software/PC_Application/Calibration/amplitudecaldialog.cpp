#include "amplitudecaldialog.h"
#include "ui_amplitudecaldialog.h"
#include "mode.h"
#include "unit.h"
#include <QDebug>
#include "ui_addamplitudepointsdialog.h"
#include "ui_automaticamplitudedialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include "json.hpp"
#include <fstream>
#include <CustomWidgets/informationbox.h>
#include "Util/util.h"

using namespace std;
using namespace nlohmann;

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
    connect(ui->load, &QPushButton::clicked, [=](){
        if(ConfirmActionIfEdited()) {
            LoadFromDevice();
        }
    });
    connect(ui->save, &QPushButton::clicked, this, &AmplitudeCalDialog::SaveToDevice);

    connect(ui->saveFile, &QPushButton::clicked, [=](){
        auto fileEnding = pointType() == Protocol::PacketType::SourceCalPoint ? ".srccal" : ".recvcal";
        auto fileFilter = QString("Amplitude calibration files (*")+fileEnding+")";
        auto filename = QFileDialog::getSaveFileName(nullptr, "Save calibration data", "", fileFilter, nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.isEmpty()) {
            // aborted selection
            return;
        }
        if(!filename.endsWith(fileEnding)) {
            filename.append(fileEnding);
        }
        json j;
        for(auto p : points) {
            json point;
            point["Frequency"] = p.frequency;
            point["Port1"] = p.correctionPort1;
            point["Port2"] = p.correctionPort2;
            j.push_back(point);
        }
        ofstream file;
        file.open(filename.toStdString());
        file << setw(4) << j << endl;
        file.close();
        edited = false;
    });

    connect(ui->loadFile, &QPushButton::clicked, [=](){
        auto fileEnding = pointType() == Protocol::PacketType::SourceCalPoint ? ".srccal" : ".recvcal";
        auto fileFilter = QString("Amplitude calibration files (*")+fileEnding+")";
        auto filename = QFileDialog::getOpenFileName(nullptr, "Save calibration data", "", fileFilter, nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.isEmpty()) {
            // aborted selection
            return;
        }
        ifstream file;
        file.open(filename.toStdString());
        if(!file.is_open()) {
            throw runtime_error("Unable to open file");
        }
        RemoveAllPoints();
        json j;
        file >> j;
        for(auto point : j) {
            if(!point.contains("Frequency") || !point.contains("Port1") || !point.contains("Port2")) {
                QMessageBox::warning(this, "Error loading file", "Failed to parse calibration point");
                return;
            }
            CorrectionPoint p;
            p.frequency = point["Frequency"];
            p.correctionPort1 = point["Port1"];
            p.correctionPort2 = point["Port2"];
            p.port1set = false;
            p.port2set = false;
            UpdateAmplitude(p);
            if(!AddPoint(p)) {
                InformationBox::ShowMessage("Limit reached", "Unable to add all points from file, would not fit into the device");
                break;
            }
        }
    });

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
    connect(ui->automatic, &QPushButton::clicked, this, &AmplitudeCalDialog::AutomaticMeasurementDialog);

    connect(dev, &Device::SpectrumResultReceived, this, &AmplitudeCalDialog::ReceivedMeasurement);
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
        points[point].amplitudePort2 = points[point].amplitudePort1;
    } else if(mode == CalibrationMode::OnlyPort2 && port2) {
        // apply result from port 2 to port 1 as well
        points[point].correctionPort1 = points[point].correctionPort2;
        points[point].port1set = points[point].port2set;
        points[point].amplitudePort1 = points[point].amplitudePort2;
    }
    model.dataChanged(model.index(point, model.ColIndexCorrectionFactors), model.index(point, model.ColIndexPort2));
    UpdateSaveButton();
}

void AmplitudeCalDialog::ReceivedPoint(Protocol::AmplitudeCorrectionPoint p)
{
//    qDebug() << "Received amplitude calibration point" << p.pointNum << "/" << p.totalPoints;
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
    if(p.pointNum == p.totalPoints - 1) {
        // this was the last point
        disconnect(dev, &Device::AmplitudeCorrectionPointReceived, this, nullptr);
        ui->load->setEnabled(true);
    }
}

void AmplitudeCalDialog::LoadFromDevice()
{
    ui->load->setEnabled(false);
    dev->SetIdle();
    RemoveAllPoints();
//    qDebug() << "Asking for amplitude calibration";
    connect(dev, &Device::AmplitudeCorrectionPointReceived, this, &AmplitudeCalDialog::ReceivedPoint);
    dev->SendCommandWithoutPayload(requestCommand());
    edited = false;
    UpdateSaveButton();
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
//        qDebug() << "Sent amplitude calibration point" << i << "/" << points.size();
    }
    edited = false;
    UpdateSaveButton();
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
    UpdateSaveButton();
}

bool AmplitudeCalDialog::AddPoint(AmplitudeCalDialog::CorrectionPoint &p)
{
    if (points.size() >= Device::Info().limits_maxAmplitudePoints) {
        // already at limit
        return false;
    }
    // find position at which this frequency gets inserted
    auto index = upper_bound(points.begin(), points.end(), p.frequency, [](double value, const CorrectionPoint& p){
        return value < p.frequency;
    });
    model.beginInsertRows(QModelIndex(), index - points.begin(), index - points.begin());
    emit newPointCreated(p);
    points.insert(index, p);
    model.endInsertRows();
    UpdateSaveButton();
    return true;
}

bool AmplitudeCalDialog::AddPoint(double frequency)
{
    CorrectionPoint newPoint;
    newPoint.frequency = frequency;
    newPoint.correctionPort1 = 0;
    newPoint.correctionPort2 = 0;
    newPoint.port1set = false;
    newPoint.port2set = false;
    auto success = AddPoint(newPoint);
    if (success) {
        edited = true;
    }
    return success;
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
            if(!AddPoint(ui->frequency->value())) {
                InformationBox::ShowMessage("Limit reached", "Can't add any more points, would not fit into the device");
            }
        } else {
            double freq_start = ui->startFreq->value();
            double freq_stop = ui->stopFreq->value();
            unsigned int points = ui->numPoints->value();
            double freq_step = (freq_stop - freq_start) / (points - 1);
            for(unsigned int i=0;i<points;i++) {
                if(!AddPoint(freq_start + i * freq_step)) {
                    InformationBox::ShowMessage("Limit reached", "Can't add any more points, would not fit into the device");
                    break;
                }
            }
        }
    });

    connect(d, &QDialog::rejected, ui->buttonBox, &QDialogButtonBox::rejected);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, [=](){
        // aborted, nothing to do
        delete d;
    });

    dev->SendCommandWithoutPayload(requestCommand());

    d->show();
}

void AmplitudeCalDialog::AutomaticMeasurementDialog()
{
    automatic.isSourceCal = pointType() == Protocol::PacketType::SourceCalPoint;
    const QString ownCal = automatic.isSourceCal ? "Source" : "Receiver";
    const QString otherCal = automatic.isSourceCal ? "Receiver" : "Source";

    // compile info text
    QString info = "It is possible to determine the " + ownCal + " Calibration if a valid " + otherCal
            + " Calibration is already present. To do so, connect a short cable with as little loss as possible"
            + " between the ports. For each point in the " + otherCal + " Calibration, the device will take"
            + " a measurement and determine the correction factors for the " + ownCal + " Calibration.";

    automatic.dialog = new QDialog(this);
    auto ui = new Ui::AutomaticAmplitudeDialog();
    ui->setupUi(automatic.dialog);
    automatic.progress = ui->progress;
    ui->explanation->setText(info);
    ui->status->setText("Gathering information about "+otherCal+" Calibration...");

    automatic.points.clear();

    connect(automatic.dialog, &QDialog::rejected, ui->abort, &QPushButton::click);
    connect(ui->abort, &QPushButton::clicked, [=](){
        // aborted, clean up
        disconnect(dev, &Device::SpectrumResultReceived, this, &AmplitudeCalDialog::ReceivedAutomaticMeasurementResult);
        dev->SetIdle();
        delete ui;
        delete automatic.dialog;
    });

    dev->SetIdle();
    connect(dev, &Device::AmplitudeCorrectionPointReceived, this, [this, ui, otherCal](Protocol::AmplitudeCorrectionPoint p) {
        CorrectionPoint c;
        c.frequency = p.freq * 10.0;
        c.correctionPort1 = p.port1;
        c.correctionPort2 = p.port2;
        c.port1set = true;
        c.port2set = true;
        automatic.points.push_back(c);
        ui->progress->setValue(100 * (p.pointNum+1) / p.totalPoints);
        if(p.pointNum == p.totalPoints - 1) {
            // this was the last point, indicate ready for measurement
            ui->progress->setValue(0);
            ui->status->setText(otherCal + " Calibration contains " +QString::number(p.totalPoints)+" points, ready to start measurement");
            ui->start->setEnabled(true);
            disconnect(dev, &Device::AmplitudeCorrectionPointReceived, this, nullptr);
            qDebug() << "Received" << p.totalPoints << "points for automatic calibration";
        }
    });
    // request points of otherCal
    // switch between source/receiver calibration
    auto request = automatic.isSourceCal ? Protocol::PacketType::RequestReceiverCal : Protocol::PacketType::RequestSourceCal;
    dev->SendCommandWithoutPayload(request);

    connect(ui->start, &QPushButton::clicked, [=](){
        // remove any exising points in own calibration and copy points from other calibration
        RemoveAllPoints();
        for(auto p : automatic.points) {
            AddPoint(p.frequency);
        }
        // intialize measurement state machine
        connect(dev, &Device::SpectrumResultReceived, this, &AmplitudeCalDialog::ReceivedAutomaticMeasurementResult);
        automatic.measuringPort2 = false;
        automatic.measuringCount = 0;
        ui->status->setText("Taking measurements...");
        SetupNextAutomaticPoint(automatic.isSourceCal);
    });

    automatic.dialog->show();
}

void AmplitudeCalDialog::ReceivedMeasurement(Protocol::SpectrumAnalyzerResult res)
{
    if(res.pointNum == 1) {
        // store result in center of sweep of 3 points
        if(measured.size() >= averages) {
            measured.pop_front();
        }
        MeasurementResult m = {.port1 = Util::SparamTodB(res.port1), .port2 = Util::SparamTodB(res.port2)};
        measured.push_back(m);
    }
}

bool AmplitudeCalDialog::ConfirmActionIfEdited()
{
    if(edited) {
        auto reply = QMessageBox::question(this, "Confirm action", "Some points have been edited but not saved in the device yet. If you continue, all changes will be lost (unless they are already saved to a file). Do you want to continue?",
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
    if(!edited) {
        // already saved in device, disable button
        enable = false;
    }
    ui->save->setEnabled(enable);
}

void AmplitudeCalDialog::SetupNextAutomaticPoint(bool isSourceCal)
{
    qDebug() << "Starting automatic calibration measurement for point" << automatic.measuringCount <<", measuring port2:" << automatic.measuringPort2;
    auto point = automatic.points[automatic.measuringCount];
    Protocol::PacketInfo p = {};
    p.type = Protocol::PacketType::SpectrumAnalyzerSettings;
    p.spectrumSettings.RBW = 10000;
    p.spectrumSettings.UseDFT = 0;
    // setup 3 points centered around the measurement frequency (zero span not supported yet)
    p.spectrumSettings.f_stop = point.frequency + 1.0;
    p.spectrumSettings.f_start = point.frequency - 1.0;
    p.spectrumSettings.pointNum = 3;
    p.spectrumSettings.Detector = 0;
    p.spectrumSettings.SignalID = 1;
    p.spectrumSettings.WindowType = 3;
    p.spectrumSettings.trackingGenerator = 1;
    p.spectrumSettings.trackingPower = excitationAmplitude * 100.0;
    p.spectrumSettings.trackingGeneratorOffset = 0;
    // For a source cal, the tracking generator has to be enabled at the measurement port (calibrating the output power)
    // For a receiver cal, the tracking generator has to be enabled at the other port (calibrating received power)
    if(isSourceCal) {
        if(automatic.measuringPort2) {
            p.spectrumSettings.trackingGeneratorPort = 1;
        } else {
            p.spectrumSettings.trackingGeneratorPort = 0;
        }
    } else {
        if(automatic.measuringPort2) {
            p.spectrumSettings.trackingGeneratorPort = 0;
        } else {
            p.spectrumSettings.trackingGeneratorPort = 1;
        }
    }
    if(isSourceCal) {
        // Calibrating the source which means the receiver is already calibrated -> use receiver corrections but no source corrections
        p.spectrumSettings.applyReceiverCorrection = 1;
        p.spectrumSettings.applySourceCorrection = 0;
    } else {
        // the other way around
        p.spectrumSettings.applyReceiverCorrection = 0;
        p.spectrumSettings.applySourceCorrection = 1;
    }
    automatic.settlingCount = averages + automaticSettling;
    dev->SendPacket(p);
}

void AmplitudeCalDialog::ReceivedAutomaticMeasurementResult(Protocol::SpectrumAnalyzerResult res)
{
    if(res.pointNum != 1) {
        // ignore first and last point, only use the middle one
        return;
    }
    if(automatic.settlingCount > 0) {
        automatic.settlingCount--;
        return;
    }
    if(automatic.isSourceCal) {
        // grab correct measurement and convert to dbm
        auto m = averageMeasurement();
        double measurement = automatic.measuringPort2 ? m.port1 : m.port2;
        // receiver cal already done, the measurement result is accurate and can be used to determine actual output power
        setAmplitude(measurement, automatic.measuringCount, automatic.measuringPort2);
    } else {
        // source cal already done, the output power is accurate while the measurement might be off.
        // The measurement result is already extracted by the ReceiverCalDialog
        setAmplitude(excitationAmplitude, automatic.measuringCount, automatic.measuringPort2);
    }
    // update progress bar
    automatic.progress->setValue(100 * (automatic.measuringCount * 2 + automatic.measuringPort2 + 1) / (points.size() * 2));
    // advance state machine
    // switch ports
    automatic.measuringPort2 = !automatic.measuringPort2;
    if(!automatic.measuringPort2) {
        // now measuring port1, this means we have to advance to the next point
        automatic.measuringCount++;
        if(automatic.measuringCount >= points.size()) {
            // all done, close dialog
            // Closing the dialog will disconnect the connection to this function and return the device into the idle state
            automatic.dialog->reject();
            automatic.dialog = nullptr;
            qDebug() << "Automatic source/receiver calibration complete";
            return;
        }
    }
    // Start next measurement
    SetupNextAutomaticPoint(automatic.isSourceCal);
}

AmplitudeCalDialog::MeasurementResult AmplitudeCalDialog::averageMeasurement()
{
    MeasurementResult ret = {};
    for(auto m : measured) {
        ret.port1 += m.port1;
        ret.port2 += m.port2;
    }
    ret.port1 /= measured.size();
    ret.port2 /= measured.size();
    return ret;
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

int AmplitudeModel::rowCount(const QModelIndex &) const
{
    return c->getPoints().size();
}

int AmplitudeModel::columnCount(const QModelIndex &) const
{
    return ColIndexLast;
}

QVariant AmplitudeModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::DisplayRole && static_cast<unsigned int>(index.row()) < c->getPoints().size()) {
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

bool AmplitudeModel::setData(const QModelIndex &index, const QVariant &value, int)
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
