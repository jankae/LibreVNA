#include "calibration.h"
#include "ui_calibrationdialogui.h"
#include "CustomWidgets/informationbox.h"
#include "Util/app_common.h"
#include "unit.h"
#include "LibreCAL/librecaldialog.h"

#include "Eigen/Dense"

#include <fstream>
#include <iomanip>

#include <QDialog>
#include <QMenu>
#include <QStyle>
#include <QDebug>
#include <QFileDialog>

using namespace std;
using Eigen::MatrixXcd;

bool operator==(const Calibration::CalType &lhs, const Calibration::CalType &rhs) {
    if(lhs.type != rhs.type) {
        return false;
    }
    if(lhs.usedPorts.size() != rhs.usedPorts.size()) {
        return false;
    }
    for(unsigned int i=0;i<lhs.usedPorts.size();i++) {
        if(lhs.usedPorts[i] != rhs.usedPorts[i]) {
            return false;
        }
    }
    // all fields are equal
    return true;
}

Calibration::Calibration()
{
    caltype.type = Type::None;
}

QString Calibration::TypeToString(Calibration::Type type)
{
    switch(type) {
    case Type::None: return "None";
    case Type::SOLT: return "SOLT";
    case Type::Last: return "Invalid";
    }
}

Calibration::Type Calibration::TypeFromString(QString s)
{
    for(int i=0;i<(int) Type::Last;i++) {
        if(TypeToString((Type) i) == s) {
            return (Type) i;
        }
    }
    return Type::None;
}

void Calibration::correctMeasurement(VirtualDevice::VNAMeasurement &d)
{
    if(caltype.type == Type::None) {
        // no calibration active, nothing to do
        return;
    }
    // formulas from "Multi-Port Calibration Techniques for Differential Parameter Measurements with Network Analyzers", variable names also losely follow this document
    MatrixXcd S(caltype.usedPorts.size(), caltype.usedPorts.size());
    MatrixXcd a(caltype.usedPorts.size(), caltype.usedPorts.size());
    MatrixXcd b(caltype.usedPorts.size(), caltype.usedPorts.size());

    // gab point and interpolate
    Point p;
    if(d.frequency <= points.front().frequency) {
        p = points.front();
    } else if(d.frequency >= points.back().frequency) {
        p = points.back();
    } else {
        // needs to interpolate
        auto lower = lower_bound(points.begin(), points.end(), d.frequency, [](const Point &lhs, double rhs) -> bool {
            return lhs.frequency < rhs;
        });
        auto highPoint = *lower;
        auto lowPoint = *prev(lower);
        double alpha = (d.frequency - lowPoint.frequency) / (highPoint.frequency - lowPoint.frequency);

        p = lowPoint.interpolate(highPoint, alpha);
    }

    // Grab measurements (easier to access by index later)
    for(unsigned int i=0;i<caltype.usedPorts.size();i++) {
        for(unsigned int j=0;j<caltype.usedPorts.size();j++) {
            auto pSrc = caltype.usedPorts[i];
            auto pRcv = caltype.usedPorts[j];
            auto name = "S"+QString::number(pRcv)+QString::number(pSrc);
            if(d.measurements.count(name) == 0) {
                qWarning() << "Missing measurement for calibration:" << name;
                return;
            } else {
                // grab measurement and remove isolation here
                S(j,i) = d.measurements[name];
                if(j != i) {
                    S(j,i) -= p.I[i][j];
                }
            }
        }
    }

    // assemble a (L) and b (K) matrices
    for(unsigned int i=0;i<caltype.usedPorts.size();i++) {
        for(unsigned int j=0;j<caltype.usedPorts.size();j++) {
            if(i == j) {
                // calculate incident and reflected wave at the exciting port
                a(j,i) = 1.0 + p.S[i]/p.R[i]*(S(j,i) - p.D[i]*1.0);
                b(j,i) = (1.0 / p.R[i]) * (S(j,i) - p.D[i]*1.0);
            } else {
                // calculate incident and reflected wave at the receiving port
                a(j,i) = p.L[i][j]*S(j,i) / p.T[i][j];
                b(j,i) = S(j,i) / p.T[i][j];
            }
        }
    }
    S = b * a.inverse();

    // extract measurement from matrix and store back into VNAMeasurement
    for(unsigned int i=0;i<caltype.usedPorts.size();i++) {
        for(unsigned int j=0;j<caltype.usedPorts.size();j++) {
            auto pSrc = caltype.usedPorts[i];
            auto pRcv = caltype.usedPorts[j];
            auto name = "S"+QString::number(pRcv)+QString::number(pSrc);
            d.measurements[name] = S(j,i);
        }
    }
}

void Calibration::edit()
{
    auto d = new QDialog();
    d->setAttribute(Qt::WA_DeleteOnClose);
    auto ui = new Ui::CalibrationDialog;
    ui->setupUi(d);

    ui->calMinFreq->setUnit("Hz");
    ui->calMinFreq->setPrecision(4);
    ui->calMinFreq->setPrefixes(" kMG");
    ui->calMaxFreq->setUnit("Hz");
    ui->calMaxFreq->setPrecision(4);
    ui->calMaxFreq->setPrefixes(" kMG");

    // generate all possible calibration with the connected device
    vector<CalType> availableCals = getAvailableCalibrations();

    for(auto c : availableCals) {
        ui->calibrationList->addItem(c.getReadableDescription());
    }

    auto updateCalStatistics = [=](){
        ui->activeCalibration->setText(caltype.getReadableDescription());
        ui->calPoints->setValue(points.size());
        if(points.size() > 0) {
            ui->calMinFreq->setValue(points.front().frequency);
            ui->calMaxFreq->setValue(points.back().frequency);
        } else {
            ui->calMinFreq->setValue(0);
            ui->calMaxFreq->setValue(0);
        }
    };

    auto updateCalButtons = [=](){
        auto row = ui->calibrationList->currentRow();
        if(row < 0) {
            ui->activate->setEnabled(false);
            ui->deactivate->setEnabled(false);
        } else {
            if(caltype == availableCals[row]) {
                ui->deactivate->setEnabled(true);
                ui->activate->setEnabled(false);
            } else {
                ui->deactivate->setEnabled(false);
                ui->activate->setEnabled(canCompute(availableCals[row]));
            }
        }
    };

    auto updateCalibrationList = [=](){
        auto style = QApplication::style();
        for(int i=0;i<availableCals.size();i++) {
            QIcon icon;
            if(canCompute(availableCals[i])) {
                icon = style->standardIcon(QStyle::SP_DialogApplyButton);
            } else {
                icon = style->standardIcon(QStyle::SP_MessageBoxCritical);
            }
            ui->calibrationList->item(i)->setIcon(icon);
        }
        updateCalButtons();
    };

    updateCalibrationList();
    updateCalStatistics();
    updateCalButtons();

    connect(ui->calibrationList, &QListWidget::doubleClicked, [=](const QModelIndex &index) {
        ui->activate->clicked();
    });

    connect(ui->calibrationList, &QListWidget::currentRowChanged, [=](){
        updateCalButtons();
    });

    connect(ui->activate, &QPushButton::clicked, [=](){
        auto cal = availableCals[ui->calibrationList->currentRow()];
        if(compute(cal)) {
            updateCalibrationList();
            updateCalStatistics();
            updateCalButtons();
        }
    });

    connect(ui->deactivate, &QPushButton::clicked, [=](){
        deactivate();
        updateCalibrationList();
        updateCalStatistics();
        updateCalButtons();
    });

    ui->table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    auto updateTableEditButtons = [=](){
        ui->bDelete->setEnabled(ui->table->currentRow() >= 0);
        ui->bMoveUp->setEnabled(ui->table->currentRow() >= 1);
        ui->bMoveDown->setEnabled(ui->table->currentRow() >= 0 && ui->table->currentRow() < ui->table->rowCount() - 1);
    };

    auto updateMeasurementTable = [=](){
        int row = ui->table->currentRow();
        ui->table->clear();
        ui->table->setColumnCount(5);
        ui->table->setHorizontalHeaderItem(0, new QTableWidgetItem("Type"));
        ui->table->setHorizontalHeaderItem(1, new QTableWidgetItem("Calkit Standard"));
        ui->table->setHorizontalHeaderItem(2, new QTableWidgetItem("Settings"));
        ui->table->setHorizontalHeaderItem(3, new QTableWidgetItem("Statistics"));
        ui->table->setHorizontalHeaderItem(4, new QTableWidgetItem("Timestamp"));
        ui->table->setRowCount(measurements.size());
        for(unsigned int i=0;i<measurements.size();i++){
            ui->table->setItem(i, 0, new QTableWidgetItem(CalibrationMeasurement::Base::TypeToString(measurements[i]->getType())));
            ui->table->setCellWidget(i, 1, measurements[i]->createStandardWidget());
            ui->table->setCellWidget(i, 2, measurements[i]->createSettingsWidget());
            ui->table->setItem(i, 3, new QTableWidgetItem(measurements[i]->getStatistics()));
            ui->table->setItem(i, 4, new QTableWidgetItem(measurements[i]->getTimestamp().toString()));
        }
        ui->table->selectRow(row);
        updateTableEditButtons();
    };

    ui->createDefault->addItem(" ");
    for(unsigned int i=0;i<(int) DefaultMeasurements::Last;i++) {
        ui->createDefault->addItem(DefaultMeasurementsToString((DefaultMeasurements) i));
    }

    QObject::connect(ui->createDefault, qOverload<int>(&QComboBox::currentIndexChanged), [=](){
        if(measurements.size() > 0) {
            if(!InformationBox::AskQuestion("Create default entries?", "Do you want to remove all existing entries and create default calibration measurements instead?", true)) {
                // user aborted
                return;
            }
            measurements.clear();
        }
        createDefaultMeasurements((DefaultMeasurements) (ui->createDefault->currentIndex() - 1));
        updateMeasurementTable();
        updateCalibrationList();
        ui->createDefault->blockSignals(true);
        ui->createDefault->setCurrentIndex(0);
        ui->createDefault->blockSignals(false);
    });

    QObject::connect(ui->bDelete, &QPushButton::clicked, [=](){
        auto selected = ui->table->selectionModel()->selectedRows();
        set<CalibrationMeasurement::Base*> toDelete;
        for(auto s : selected) {
            toDelete.insert(measurements[s.row()]);
        }
        while(toDelete.size() > 0) {
            for(unsigned int i=0;i<measurements.size();i++) {
                if(toDelete.count(measurements[i])) {
                    // this measurement should be deleted
                    delete measurements[i];
                    toDelete.erase(measurements[i]);
                    measurements.erase(measurements.begin() + i);
                }
            }
        }
        updateMeasurementTable();
        updateCalibrationList();
    });

    QObject::connect(ui->bMoveUp, &QPushButton::clicked, [=](){
        auto row = ui->table->currentRow();
        if(row >= 1) {
            swap(measurements[row], measurements[row-1]);
            ui->table->selectRow(row-1);
            updateMeasurementTable();
        }
    });

    QObject::connect(ui->bMoveDown, &QPushButton::clicked, [=](){
        auto row = ui->table->currentRow();
        if(row >= 0) {
            swap(measurements[row], measurements[row+1]);
            ui->table->selectRow(row+1);
            updateMeasurementTable();
        }
    });

    connect(ui->measure, &QPushButton::clicked, [=](){
        std::set<CalibrationMeasurement::Base*> m;
        auto selected = ui->table->selectionModel()->selectedRows();
        for(auto s : selected) {
            m.insert(measurements[s.row()]);
        }
        if(!CalibrationMeasurement::Base::canMeasureSimultaneously(m)) {
            InformationBox::ShowError("Unable to measure", "Different selected measurements require the same port, unable to perform measurement");
            return;
        }
        emit startMeasurements(m);
    });

    connect(this, &Calibration::measurementsUpdated, d, [=](){
        updateMeasurementTable();
        updateCalibrationList();
    });

    connect(ui->clearMeasurement, &QPushButton::clicked, [=](){
        auto selected = ui->table->selectionModel()->selectedRows();
        for(auto s : selected) {
            measurements[s.row()]->clearPoints();
        }
        updateMeasurementTable();
        updateCalibrationList();
    });

    connect(ui->eCal, &QPushButton::clicked, [=](){
        auto d = new LibreCALDialog(this);
        d->show();
    });

    QObject::connect(ui->table, &QTableWidget::currentCellChanged, updateTableEditButtons);

    auto addMenu = new QMenu();
    for(auto t : CalibrationMeasurement::Base::availableTypes()) {
        auto action = new QAction(CalibrationMeasurement::Base::TypeToString(t));
        QObject::connect(action, &QAction::triggered, [=](){
            auto newMeas = newMeasurement(t);
            if(newMeas) {
                measurements.push_back(newMeas);
                updateMeasurementTable();
            }
        });
        addMenu->addAction(action);
    }

    ui->bAdd->setMenu(addMenu);

    updateMeasurementTable();

    d->show();
}

CalibrationMeasurement::Base *Calibration::newMeasurement(CalibrationMeasurement::Base::Type type)
{
    CalibrationMeasurement::Base *m = nullptr;
    switch(type) {
    case CalibrationMeasurement::Base::Type::Open: m = new CalibrationMeasurement::Open(this); break;
    case CalibrationMeasurement::Base::Type::Short: m = new CalibrationMeasurement::Short(this); break;
    case CalibrationMeasurement::Base::Type::Load: m = new CalibrationMeasurement::Load(this); break;
    case CalibrationMeasurement::Base::Type::Through: m = new CalibrationMeasurement::Through(this); break;
    case CalibrationMeasurement::Base::Type::Isolation: m = new CalibrationMeasurement::Isolation(this); break;
    }
    return m;
}

Calibration::Point Calibration::computeSOLT(double f)
{
    Point point;
    point.frequency = f;
    // resize vectors
    point.D.resize(caltype.usedPorts.size());
    point.R.resize(caltype.usedPorts.size());
    point.S.resize(caltype.usedPorts.size());

    point.L.resize(caltype.usedPorts.size());
    point.T.resize(caltype.usedPorts.size());
    point.I.resize(caltype.usedPorts.size());
    fill(point.L.begin(), point.L.end(), vector<complex<double>>(caltype.usedPorts.size()));
    fill(point.T.begin(), point.T.end(), vector<complex<double>>(caltype.usedPorts.size()));
    fill(point.I.begin(), point.I.end(), vector<complex<double>>(caltype.usedPorts.size()));

    // Calculate SOL coefficients
    for(unsigned int i=0;i<caltype.usedPorts.size();i++) {
        auto p = caltype.usedPorts[i];
        auto _short = static_cast<CalibrationMeasurement::Short*>(findMeasurement(CalibrationMeasurement::Base::Type::Short, p));
        auto open = static_cast<CalibrationMeasurement::Open*>(findMeasurement(CalibrationMeasurement::Base::Type::Open, p));
        auto load = static_cast<CalibrationMeasurement::Load*>(findMeasurement(CalibrationMeasurement::Base::Type::Load, p));
        auto s_m = _short->getMeasured(f);
        auto o_m = open->getMeasured(f);
        auto l_m = load->getMeasured(f);
        auto s_c = _short->getActual(f);
        auto o_c = open->getActual(f);
        auto l_c = load->getActual(f);
        auto denom = l_c * o_c * (o_m - l_m) + l_c * s_c * (l_m - s_m) + o_c * s_c * (s_m - o_m);
        point.D[i] = (l_c * o_m * (s_m * (o_c - s_c) + l_m * s_c) - l_c * o_c * l_m * s_m + o_c * l_m * s_c * (s_m - o_m)) / denom;
        point.S[i] = (l_c * (o_m - s_m) + o_c * (s_m - l_m) + s_c * (l_m - o_m)) / denom;
        auto delta = (l_c * l_m * (o_m - s_m) + o_c * o_m * (s_m - l_m) + s_c * s_m * (l_m - o_m)) / denom;
        point.R[i] = point.D[i] * point.S[i] - delta;
    }
    // calculate forward match and transmission
    for(unsigned int i=0;i<caltype.usedPorts.size();i++) {
        for(unsigned int j=0;j<caltype.usedPorts.size();j++) {
            if(i == j) {
                // this is the exciting port, SOL error box used here
                continue;
            }
            auto p1 = caltype.usedPorts[i];
            auto p2 = caltype.usedPorts[j];
            // grab measurement and calkit through definitions
            auto throughForward = static_cast<CalibrationMeasurement::Through*>(findMeasurement(CalibrationMeasurement::Base::Type::Through, p1, p2));
            auto throughReverse = static_cast<CalibrationMeasurement::Through*>(findMeasurement(CalibrationMeasurement::Base::Type::Through, p2, p1));
            complex<double> S11, S21;
            Sparam Sideal;
            if(throughForward) {
                S11 = throughForward->getMeasured(f).m11;
                S21 = throughForward->getMeasured(f).m21;
                Sideal = throughForward->getActual(f);
            } else if(throughReverse) {
                S11 = throughReverse->getMeasured(f).m22;
                S21 = throughReverse->getMeasured(f).m12;
                Sideal = throughReverse->getActual(f);
                swap(Sideal.m11, Sideal.m22);
                swap(Sideal.m12, Sideal.m21);
            }
            auto isoMeas = static_cast<CalibrationMeasurement::Isolation*>(findMeasurement(CalibrationMeasurement::Base::Type::Isolation));
            auto isolation = complex<double>(0.0,0.0);
            if(isoMeas) {
                isolation = isoMeas->getMeasured(f, p2, p1);
            }
            auto deltaS = Sideal.m11*Sideal.m22 - Sideal.m21 * Sideal.m12;
            point.L[i][j] = ((S11 - point.D[i])*(1.0 - point.S[i] * Sideal.m11)-Sideal.m11*point.R[i])
                    / ((S11 - point.D[i])*(Sideal.m22-point.S[i]*deltaS)-deltaS*point.R[i]);
            point.T[i][j] = (S21 - isolation)*(1.0 - point.S[i]*Sideal.m11 - point.L[i][j]*Sideal.m22 + point.S[i]*point.L[i][j]*deltaS) / Sideal.m21;
            point.I[i][j] = isolation;
        }
    }
    return point;
}

Calibration::CalType Calibration::getCaltype() const
{
    return caltype;
}

Calibration::InterpolationType Calibration::getInterpolation(double f_start, double f_stop, int npoints)
{
    if(!points.size()) {
        return InterpolationType::NoCalibration;
    }
    if(f_start < points.front().frequency || f_stop > points.back().frequency) {
        return InterpolationType::Extrapolate;
    }
    // Either exact or interpolation, check individual frequencies
    uint32_t f_step;
    if(npoints > 1) {
        f_step = (f_stop - f_start) / (npoints - 1);
    } else {
        f_step = f_stop - f_start;
    }
    uint64_t f = f_start;
    do {
        if(find_if(points.begin(), points.end(), [&f](const Point& p){
            return abs(f - p.frequency) < 100;
        }) == points.end()) {
            return InterpolationType::Interpolate;
        }
        f += f_step;
    } while(f <= f_stop && f_step > std::numeric_limits<double>::epsilon());

    // if we get here all frequency points were matched
    if(points.front().frequency == f_start && points.back().frequency == f_stop) {
        return InterpolationType::Unchanged;
    } else {
        return InterpolationType::Exact;
    }
}

std::vector<Trace *> Calibration::getErrorTermTraces()
{
    return vector<Trace*>(); // TODO
}

std::vector<Trace *> Calibration::getMeasurementTraces()
{
    return vector<Trace*>(); // TODO
}

QString Calibration::getCurrentCalibrationFile()
{
    return currentCalFile;
}

double Calibration::getMinFreq()
{
    if(points.size() > 0) {
        return points.front().frequency;
    } else {
        return numeric_limits<double>::quiet_NaN();
    }
}

double Calibration::getMaxFreq()
{
    if(points.size() > 0) {
        return points.back().frequency;
    } else {
        return numeric_limits<double>::quiet_NaN();
    }
}

int Calibration::getNumPoints()
{
    return points.size();
}

QString Calibration::descriptiveCalName()
{
    if(points.size() == 0) {
        return QString();
    }
    int precision = 3;
    QString lo = Unit::ToString(points.front().frequency, "", " kMG", precision);
    QString hi = Unit::ToString(points.back().frequency, "", " kMG", precision);
    // due to rounding up 123.66M and 123.99M -> we get lo="124M" and hi="124M"
    // so let's add some precision
    if (lo == hi) {
        // Only in case of 123.66M and 123.69M we would need 5 digits, but that kind of narrow cal. is very unlikely.
        precision = 4;
        lo = Unit::ToString(points.front().frequency, "", " kMG", precision);
        hi = Unit::ToString(points.back().frequency, "", " kMG", precision);
    }

    QString tmp =
            caltype.getReadableDescription()
            + " "
            + lo + "-" + hi
            + " "
            + QString::number(this->points.size()) + "pt";
    return tmp;
}

Calkit &Calibration::getKit()
{
    return kit;
}

nlohmann::json Calibration::toJSON()
{
    nlohmann::json j;
    nlohmann::json jmeasurements;
    for(auto m : measurements) {
        nlohmann::json jmeas;
        jmeas["type"] = CalibrationMeasurement::Base::TypeToString(m->getType()).toStdString();
        jmeas["data"] = m->toJSON();
        jmeasurements.push_back(jmeas);
    }
    j["measurements"] = jmeasurements;
    j["calkit"] = kit.toJSON();
    j["type"] = TypeToString(caltype.type).toStdString();
    nlohmann::json jports;
    for(auto p : caltype.usedPorts) {
        jports.push_back(p);
    }
    j["ports"] = jports;
    j["version"] = qlibrevnaApp->applicationVersion().toStdString();
    if(VirtualDevice::getConnected()) {
        j["device"] = VirtualDevice::getConnected()->serial().toStdString();
    }
    return j;
}

void Calibration::fromJSON(nlohmann::json j)
{
    reset();
    if(j.contains("calkit")) {
        kit.fromJSON(j["calkit"]);
    }
    if(j.contains("measurements")) {
        for(auto jm : j["measurements"]) {
            auto type = CalibrationMeasurement::Base::TypeFromString(QString::fromStdString(jm.value("type", "")));
            auto m = newMeasurement(type);
            m->fromJSON(jm["data"]);
            measurements.push_back(m);
        }
    }

    CalType ct;
    ct.type = TypeFromString(QString::fromStdString(j.value("type", "")));
    if(j.contains("ports")) {
        for(auto jp : j["ports"]) {
            ct.usedPorts.push_back(jp);
        }
    }
    if(ct.type != Type::None) {
        compute(ct);
    }
}

bool Calibration::toFile(QString filename)
{
    if(filename.isEmpty()) {
        QString fn = descriptiveCalName();
        filename = QFileDialog::getSaveFileName(nullptr, "Save calibration data", fn, "Calibration files (*.cal)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.isEmpty()) {
            // aborted selection
            return false;
        }
    }

    if(filename.toLower().endsWith(".cal")) {
        filename.chop(4);
    }
    auto calibration_file = filename + ".cal";
    ofstream file;
    file.open(calibration_file.toStdString());
    file << setw(1) << toJSON();

    auto calkit_file = filename + ".calkit";
    qDebug() << "Saving associated calibration kit to file" << calkit_file;
    kit.toFile(calkit_file);
    this->currentCalFile = calibration_file;    // if all ok, remember this

    return true;
}

bool Calibration::fromFile(QString filename)
{
    if(filename.isEmpty()) {
        filename = QFileDialog::getOpenFileName(nullptr, "Load calibration data", "", "Calibration files (*.cal)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.isEmpty()) {
            // aborted selection
            return false;
        }
    }

    // force correct file ending
    if(filename.toLower().endsWith(".cal")) {
        filename.chop(4);
        filename += ".cal";
    }

    qDebug() << "Attempting to open calibration from file" << filename;

    ifstream file;

    file.open(filename.toStdString());
    if(!file.good()) {
        QString msg = "Unable to open file: "+filename;
        InformationBox::ShowError("Error", msg);
        qWarning() << msg;
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;
        currentCalFile = filename;    // if all ok, remember this
        fromJSON(j);
    } catch(exception &e) {
        currentCalFile.clear();
        InformationBox::ShowError("File parsing error", e.what());
        qWarning() << "Calibration file parsing failed: " << e.what();
        return false;
    }

    return true;
}

std::vector<Calibration::CalType> Calibration::getAvailableCalibrations()
{
    int ports = 2;
    if(VirtualDevice::getConnected()) {
        ports = VirtualDevice::getConnected()->getInfo().ports;
    }
    vector<CalType> ret;
    for(auto t : getTypes()) {
        CalType cal;
        cal.type = t;
        auto minPorts = minimumPorts(t);
        for(int pnum = minPorts;pnum <= ports;pnum++) {
            std::string bitmask(pnum, 1);
            bitmask.resize(ports, 0);
            // assemble selected ports and permute bitmask
            do {
                vector<int> usedPorts;
                for (int i = 0; i < ports; ++i) {
                    if (bitmask[i]) {
                        usedPorts.push_back(i+1);
                    }
                }
                cal.usedPorts = usedPorts;
                ret.push_back(cal);
            } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
        }
    }
    return ret;
}

std::vector<Calibration::Type> Calibration::getTypes()
{
    vector<Type> types;
    // Start at index 1, skip Type::None
    for(int i=1;i<(int) Type::Last;i++) {
        types.push_back((Type) i);
    }
    return types;
}

bool Calibration::canCompute(Calibration::CalType type, double *startFreq, double *stopFreq, int *points)
{
    switch(type.type) {
    case Type::None:
        return true; // Always possible to reset the calibration
    case Type::SOLT: {
        using RequiredMeasurements = struct {
            CalibrationMeasurement::Base::Type type;
            int port1, port2;
        };
        vector<RequiredMeasurements> required;
        // SOL measurements for every port
        for(auto p : type.usedPorts) {
            required.push_back({.type = CalibrationMeasurement::Base::Type::Short, .port1 = p});
            required.push_back({.type = CalibrationMeasurement::Base::Type::Open, .port1 = p});
            required.push_back({.type = CalibrationMeasurement::Base::Type::Load, .port1 = p});
        }
        // through measurements between all ports
        for(int i=1;i<=type.usedPorts.size();i++) {
            for(int j=i+1;j<=type.usedPorts.size();j++) {
                required.push_back({.type = CalibrationMeasurement::Base::Type::Through, .port1 = i, .port2 = j});
            }
        }
        vector<CalibrationMeasurement::Base*> foundMeasurements;
        for(auto m : required) {
            auto meas = findMeasurement(m.type, m.port1, m.port2);
            if(!meas) {
                // missing measurement
                return false;
            } else {
                foundMeasurements.push_back(meas);
            }
        }
        return hasFrequencyOverlap(foundMeasurements, startFreq, stopFreq, points);
    }
        break;
    }
    return false;
}

bool Calibration::compute(Calibration::CalType type)
{
    if(type.type == Type::None) {
        deactivate();
        return true;
    }
    double start, stop;
    int numPoints;
    if(!canCompute(type, &start, &stop, &numPoints)) {
        return false;
    }
    caltype = type;
    try {
        points.clear();
        for(int i=0;i<numPoints;i++) {
            double f = start + (stop - start) * i / (numPoints - 1);
            Point p;
            switch(type.type) {
            case Type::SOLT: p = computeSOLT(f); break;
            }
            points.push_back(p);
        }
    } catch (exception &e) {
        points.clear();
        caltype.usedPorts.clear();
    }
    emit activated(caltype);
    return true;
}

void Calibration::reset()
{
    for(auto m : measurements) {
        delete m;
    }
    measurements.clear();
    deactivate();
}

int Calibration::minimumPorts(Calibration::Type type)
{
    switch(type) {
    case Type::SOLT: return 1;
    }
    return -1;
}

void Calibration::addMeasurements(std::set<CalibrationMeasurement::Base *> m, const VirtualDevice::VNAMeasurement &data)
{
    for(auto meas : m) {
        meas->addPoint(data);
    }
}

void Calibration::clearMeasurements(std::set<CalibrationMeasurement::Base *> m)
{
    for(auto meas : m) {
        meas->clearPoints();
    }
}

void Calibration::measurementsComplete()
{
    emit measurementsUpdated();
}

void Calibration::deactivate()
{
    points.clear();
    caltype.type = Type::None;
    caltype.usedPorts.clear();
    emit deactivated();
}

QString Calibration::DefaultMeasurementsToString(Calibration::DefaultMeasurements dm)
{
    switch(dm) {
    case DefaultMeasurements::SOL1Port: return "1 Port SOL";
    case DefaultMeasurements::SOLT2Port: return "2 Port SOLT";
    case DefaultMeasurements::SOLT3Port: return "3 Port SOLT";
    case DefaultMeasurements::SOLT4Port: return "4 Port SOLT";
    }
}

void Calibration::createDefaultMeasurements(Calibration::DefaultMeasurements dm)
{
    auto createSOL = [=](int port) {
        auto _short = new CalibrationMeasurement::Short(this);
        _short->setPort(port);
        measurements.push_back(_short);
        auto open = new CalibrationMeasurement::Open(this);
        open->setPort(port);
        measurements.push_back(open);
        auto load = new CalibrationMeasurement::Load(this);
        load->setPort(port);
        measurements.push_back(load);
    };
    auto createThrough = [=](int port1, int port2) {
        auto through = new CalibrationMeasurement::Through(this);
        through->setPort1(port1);
        through->setPort2(port2);
        measurements.push_back(through);
    };
    switch(dm) {
    case DefaultMeasurements::SOL1Port:
        createSOL(1);
        break;
    case DefaultMeasurements::SOLT2Port:
        createSOL(1);
        createSOL(2);
        createThrough(1, 2);
        break;
    case DefaultMeasurements::SOLT3Port:
        createSOL(1);
        createSOL(2);
        createSOL(3);
        createThrough(1, 2);
        createThrough(1, 3);
        createThrough(2, 3);
        break;
    case DefaultMeasurements::SOLT4Port:
        createSOL(1);
        createSOL(2);
        createSOL(3);
        createSOL(4);
        createThrough(1, 2);
        createThrough(1, 3);
        createThrough(1, 4);
        createThrough(2, 3);
        createThrough(2, 4);
        createThrough(3, 4);
        break;
    }
}

bool Calibration::hasFrequencyOverlap(std::vector<CalibrationMeasurement::Base *> m, double *startFreq, double *stopFreq, int *points)
{
    double minResolution = std::numeric_limits<double>::max();
    double minFreq = 0;
    double maxFreq = std::numeric_limits<double>::max();
    for(auto meas : m) {
        if(meas->numPoints() < 2) {
            return false;
        }
        auto resolution = (meas->maxFreq() - meas->minFreq()) / (meas->numPoints() - 1);
        if(meas->maxFreq() < maxFreq) {
            maxFreq = meas->maxFreq();
        }
        if(meas->minFreq() > minFreq) {
            minFreq = meas->minFreq();
        }
        if(resolution < minResolution) {
            minResolution = resolution;
        }
    }
    if(startFreq) {
        *startFreq = minFreq;
    }
    if(stopFreq) {
        *stopFreq = maxFreq;
    }
    if(points) {
        *points = (maxFreq - minFreq) / minResolution + 1;
    }
    if(maxFreq > minFreq) {
        return true;
    } else {
        return false;
    }
}

CalibrationMeasurement::Base *Calibration::findMeasurement(CalibrationMeasurement::Base::Type type, int port1, int port2)
{
    for(auto m : measurements) {
        if(m->getType() != type) {
            continue;
        }
        auto onePort = dynamic_cast<CalibrationMeasurement::OnePort*>(m);
        if(onePort) {
            if(onePort->getPort() != port1) {
                continue;
            }
        }
        auto twoPort = dynamic_cast<CalibrationMeasurement::TwoPort*>(m);
        if(twoPort) {
            if(twoPort->getPort1() != port1 || twoPort->getPort2() != port2) {
                continue;
            }
        }
        // if we get here, we have a match
        return m;
    }
    return nullptr;
}

QString Calibration::CalType::getReadableDescription()
{
    QString ret = TypeToString(this->type);
    if(usedPorts.size() == 1) {
        ret += ", Port: "+QString::number(usedPorts[0]);
    } else if(usedPorts.size() > 0) {
        ret += ", Ports: [";
        for(auto p : usedPorts) {
            ret += QString::number(p)+",";
        }
        // remove the last trailing comma
        ret.chop(1);
        ret += "]";
    }
    return ret;
}

QString Calibration::CalType::getShortString()
{
    QString ret = TypeToString(this->type);
    if(usedPorts.size() > 0) {
        ret += "_";
    }
    for(auto p : usedPorts) {
        ret += QString::number(p);
    }
    return ret;
}

Calibration::CalType Calibration::CalType::fromShortString(QString s)
{
    CalType ret;
    auto list = s.split("_");
    if(list.size() != 2) {
        ret.type = Type::None;
    } else {
        ret.type = TypeFromString(list[0]);
        for(auto c : list[1]) {
            ret.usedPorts.push_back(QString(c).toInt());
        }
    }
    return ret;
}

Calibration::Point Calibration::Point::interpolate(const Calibration::Point &to, double alpha)
{
    Point ret;
    ret.frequency = frequency * (1.0-alpha) + to.frequency * alpha;
    ret.D.resize(D.size());
    for(unsigned int i=0;i<D.size();i++) {
        ret.D[i] = D[i] * (1.0-alpha) + to.D[i] * alpha;
    }
    ret.R.resize(R.size());
    for(unsigned int i=0;i<R.size();i++) {
        ret.R[i] = R[i] * (1.0-alpha) + to.R[i] * alpha;
    }
    ret.S.resize(S.size());
    for(unsigned int i=0;i<S.size();i++) {
        ret.S[i] = S[i] * (1.0-alpha) + to.S[i] * alpha;
    }
    ret.T.resize(T.size());
    for(unsigned int i=0;i<T.size();i++) {
        ret.T[i].resize(T[i].size());
        for(unsigned int j=0;j<T[i].size();j++) {
            ret.T[i][j] = T[i][j] * (1.0 - alpha) + to.T[i][j] * alpha;
        }
    }
    ret.L.resize(L.size());
    for(unsigned int i=0;i<L.size();i++) {
        ret.L[i].resize(L[i].size());
        for(unsigned int j=0;j<L[i].size();j++) {
            ret.L[i][j] = L[i][j] * (1.0 - alpha) + to.L[i][j] * alpha;
        }
    }
    ret.I.resize(I.size());
    for(unsigned int i=0;i<I.size();i++) {
        ret.I[i].resize(I[i].size());
        for(unsigned int j=0;j<I[i].size();j++) {
            ret.I[i][j] = I[i][j] * (1.0 - alpha) + to.I[i][j] * alpha;
        }
    }
    return ret;
}
