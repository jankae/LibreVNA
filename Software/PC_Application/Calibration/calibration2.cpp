#include "calibration2.h"
#include "ui_calibrationdialogui.h"
#include "CustomWidgets/informationbox.h"
#include "Util/app_common.h"

#include "Eigen/Dense"

#include <QDialog>
#include <QMenu>
#include <QStyle>
#include <QDebug>

using namespace std;
using Eigen::MatrixXcd;

Calibration2::Calibration2()
{
    caltype.type = Type::None;
}

void Calibration2::correctMeasurement(VirtualDevice::VNAMeasurement &d)
{
    if(caltype.type == Type::None) {
        // no calibration active, nothing to do
        return;
    }
    // formulas from "Multi-Port Calibration Techniques for Differential Parameter Measurements with Network Analyzers", variable names also losely follow this document
    MatrixXcd S(caltype.usedPorts.size(), caltype.usedPorts.size());
    MatrixXcd a(caltype.usedPorts.size(), caltype.usedPorts.size());
    MatrixXcd b(caltype.usedPorts.size(), caltype.usedPorts.size());
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
                S(j,i) = d.measurements[name];
            }
        }
    }

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

void Calibration2::edit()
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

    int ports = 2;
    if(VirtualDevice::getConnected()) {
        ports = VirtualDevice::getConnected()->getInfo().ports;
    }

    // generate all possible calibration with the connected device
    vector<CalType> availableCals;
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
                availableCals.push_back(cal);
            } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
        }
    }

    for(auto c : availableCals) {
        ui->calibrationList->addItem(c.getDescription());
    }

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
    };

    updateCalibrationList();

    connect(ui->calibrationList, &QListWidget::doubleClicked, [=](const QModelIndex &index) {
        auto row = index.row();
        auto cal = availableCals[row];
        if(canCompute(cal)) {
            compute(cal);
            ui->activeCalibration->setText(cal.getDescription());
            ui->calMinFreq->setValue(points.front().frequency);
            ui->calMaxFreq->setValue(points.back().frequency);
            ui->calPoints->setValue(points.size());
        }
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

    connect(this, &Calibration2::measurementsUpdated, d, [=](){
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

CalibrationMeasurement::Base *Calibration2::newMeasurement(CalibrationMeasurement::Base::Type type)
{
    CalibrationMeasurement::Base *m = nullptr;
    switch(type) {
    case CalibrationMeasurement::Base::Type::Open: m = new CalibrationMeasurement::Open(this); break;
    case CalibrationMeasurement::Base::Type::Short: m = new CalibrationMeasurement::Short(this); break;
    case CalibrationMeasurement::Base::Type::Load: m = new CalibrationMeasurement::Load(this); break;
    case CalibrationMeasurement::Base::Type::Through: m = new CalibrationMeasurement::Through(this); break;
    }
    return m;
}

Calibration2::Point Calibration2::computeSOLT(double f)
{
    Point point;
    point.frequency = f;
    // resize vectors
    point.D.resize(caltype.usedPorts.size());
    point.R.resize(caltype.usedPorts.size());
    point.S.resize(caltype.usedPorts.size());

    point.L.resize(caltype.usedPorts.size());
    point.T.resize(caltype.usedPorts.size());
    fill(point.L.begin(), point.L.end(), vector<complex<double>>(caltype.usedPorts.size()));
    fill(point.T.begin(), point.T.end(), vector<complex<double>>(caltype.usedPorts.size()));

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
            auto through = static_cast<CalibrationMeasurement::Through*>(findMeasurement(CalibrationMeasurement::Base::Type::Through, p1, p2));
            auto S11 = through->getMeasured(f).m11;
            auto S21 = through->getMeasured(f).m21;
            auto Sideal = through->getActual(f);
            auto deltaS = Sideal.m11*Sideal.m22 - Sideal.m21 * Sideal.m12;
            auto isolation = complex<double>(0.0, 0.0);
            point.L[i][j] = ((S11 - point.D[i])*(1.0 - point.S[i] * Sideal.m11)-Sideal.m11*point.R[i])
                    / ((S11 - point.D[i])*(Sideal.m22-point.S[i]*deltaS)-deltaS*point.R[i]);
            point.T[i][j] = (S21 - isolation)*(1.0 - point.S[i]*Sideal.m11 - point.L[i][j]*Sideal.m22 + point.S[i]*point.L[i][j]*deltaS) / Sideal.m21;
        }
    }
    return point;
}

Calkit &Calibration2::getKit()
{
    return kit;
}

nlohmann::json Calibration2::toJSON()
{
    nlohmann::json j;
    nlohmann::json jmeasurements;
    for(auto m : measurements) {
        nlohmann::json jmeas;
        jmeas["type"] = m->getType();
        jmeas["data"] = m->toJSON();
        jmeasurements.push_back(jmeas);
    }
    j["measurements"] = jmeasurements;
    j["calkit"] = kit.toJSON();
    j["version"] = qlibrevnaApp->applicationVersion().toStdString();
    if(VirtualDevice::getConnected()) {
        j["device"] = VirtualDevice::getConnected()->serial();
    }
    return j;
}

void Calibration2::fromJSON(nlohmann::json j)
{
    if(j.contains("calkit")) {
        kit.fromJSON(j["calkit"]);
    }
    if(j.contains("measurements")) {
        for(auto jm : j["measurements"]) {
            auto type = CalibrationMeasurement::Base::TypeFromString(jm.value("type", ""));
            auto m = newMeasurement(type);
            m->fromJSON(jm["data"]);
            measurements.push_back(m);
        }
    }
}

std::vector<Calibration2::Type> Calibration2::getTypes()
{
    vector<Type> types;
    // Start at index 1, skip Type::None
    for(int i=1;i<(int) Type::Last;i++) {
        types.push_back((Type) i);
    }
    return types;
}

bool Calibration2::canCompute(Calibration2::CalType type, double *startFreq, double *stopFreq, int *points)
{
    switch(type.type) {
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

bool Calibration2::compute(Calibration2::CalType type)
{
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
    return true;
}

int Calibration2::minimumPorts(Calibration2::Type type)
{
    switch(type) {
    case Type::SOLT: return 1;
    }
    return -1;
}

void Calibration2::deleteMeasurements()
{
    for(auto m : measurements) {
        delete m;
    }
    measurements.clear();
}

void Calibration2::addMeasurements(std::set<CalibrationMeasurement::Base *> m, const VirtualDevice::VNAMeasurement &data)
{
    for(auto meas : m) {
        meas->addPoint(data);
    }
}

void Calibration2::clearMeasurements(std::set<CalibrationMeasurement::Base *> m)
{
    for(auto meas : m) {
        meas->clearPoints();
    }
}

void Calibration2::measurementsComplete()
{
    emit measurementsUpdated();
}

QString Calibration2::DefaultMeasurementsToString(Calibration2::DefaultMeasurements dm)
{
    switch(dm) {
    case DefaultMeasurements::SOL1Port: return "1 Port SOL";
    case DefaultMeasurements::SOLT2Port: return "2 Port SOLT";
    case DefaultMeasurements::SOLT3Port: return "3 Port SOLT";
    case DefaultMeasurements::SOLT4Port: return "4 Port SOLT";
    }
}

void Calibration2::createDefaultMeasurements(Calibration2::DefaultMeasurements dm)
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

bool Calibration2::hasFrequencyOverlap(std::vector<CalibrationMeasurement::Base *> m, double *startFreq, double *stopFreq, int *points)
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

CalibrationMeasurement::Base *Calibration2::findMeasurement(CalibrationMeasurement::Base::Type type, int port1, int port2)
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

QString Calibration2::CalType::getDescription()
{
    switch(type) {
    case Type::SOLT:
        if(usedPorts.size() == 1) {
            return "SOL, Port: "+QString::number(usedPorts[0]);
        } else {
            QString ret = "SOLT, Ports: [";
            for(auto p : usedPorts) {
                ret += QString::number(p)+",";
            }
            // remove the last trailing comma
            ret.chop(1);
            ret += "]";
            return ret;
        }
    }
}

Calibration2::Point Calibration2::Point::interpolate(const Calibration2::Point &to, double alpha)
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
    return ret;
}
