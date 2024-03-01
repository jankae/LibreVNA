#include "tracemodel.h"

#include <QIcon>
#include <QDebug>
#include <QDateTime>

using namespace std;

TraceModel::TraceModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    traces.clear();
    source = DataSource::Unknown;
    lastSweepPosition = 0.0;
}

TraceModel::~TraceModel()
{
    while(!traces.empty()) {
        delete traces[0];
        traces.erase(traces.begin());
    }
}

void TraceModel::addTrace(Trace *t)
{
    beginInsertRows(QModelIndex(), traces.size(), traces.size());
    connect(t, &Trace::nameChanged, [=]() {
        emit traceNameChanged(t);
        emit dataChanged(createIndex(0, 0), createIndex(traces.size() - 1, ColIndexLast - 1));
    });
    connect(t, &Trace::pauseChanged, [=](){
        emit dataChanged(createIndex(0, 0), createIndex(traces.size() - 1, ColIndexLast - 1));
    });
    connect(t, &Trace::deembeddingChanged, [=](){
        emit dataChanged(createIndex(0, 0), createIndex(traces.size() - 1, ColIndexLast - 1));
    });
    traces.push_back(t);
    endInsertRows();
    t->setModel(this);
    emit traceAdded(t);
    emit requiredExcitation();
}

void TraceModel::removeTrace(unsigned int index)
{
    if (index < traces.size()) {
        beginRemoveRows(QModelIndex(), index, index);
        auto trace = traces[index];
        delete trace;
        traces.erase(traces.begin() + index);
        endRemoveRows();
        emit traceRemoved(trace);
        emit requiredExcitation();
    }
}

void TraceModel::removeTrace(Trace *t)
{
    auto index = findIndex(t);
    if(index >= 0) {
        removeTrace(index);
    }
}

Trace *TraceModel::trace(unsigned int index)
{
    return traces.at(index);
}

int TraceModel::findIndex(Trace *t)
{
    for(unsigned int i=0;i<traces.size();i++) {
        if(traces[i] == t) {
            return i;
        }
    }
    return -1;
}

void TraceModel::toggleVisibility(unsigned int index)
{
    if (index < traces.size()) {
        traces[index]->setVisible(!traces[index]->isVisible());
        emit dataChanged(createIndex(index, ColIndexVisible), createIndex(index, ColIndexVisible));
    }
}

void TraceModel::togglePause(unsigned int index)
{
    if (index < traces.size() && traces[index]->canBePaused()) {
        if(traces[index]->isPaused()) {
            traces[index]->resume();
        } else {
            traces[index]->pause();
        }
        emit dataChanged(createIndex(index, ColIndexPlayPause), createIndex(index, ColIndexPlayPause));
        emit requiredExcitation();
    }
}

void TraceModel::toggleMath(unsigned int index)
{
    if (index >= traces.size()) {
        return;
    }
    auto trace = traces[index];
    if(trace->hasMathOperations()) {
        trace->enableMath(!trace->mathEnabled());
        emit dataChanged(createIndex(index, ColIndexMath), createIndex(index, ColIndexMath));
    }
}

void TraceModel::toggleDeembedding(unsigned int index)
{
    if (index >= traces.size()) {
        return;
    }
    auto trace = traces[index];
    if(trace->deembeddingAvailable()) {
        trace->setDeembeddingActive(!trace->isDeembeddingActive());
        emit dataChanged(createIndex(index, ColIndexDeembedding), createIndex(index, ColIndexDeembedding));
    }
}

int TraceModel::rowCount(const QModelIndex &) const
{
    return traces.size();
}

int TraceModel::columnCount(const QModelIndex &) const
{
    return ColIndexLast;
}

QVariant TraceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if ((unsigned int) index.row() >= traces.size())
        return QVariant();

    auto trace = traces[index.row()];
    switch(index.column()) {
    case ColIndexVisible:
        if (role == Qt::DecorationRole) {
            if (trace->isVisible()) {
                return QIcon(":/icons/visible.svg");
            } else {
                return QIcon(":/icons/invisible.svg");
            }
        } else if(role == Qt::ToolTipRole){
            return "Toggle Visibility";
        } else {
            return QVariant();
        }
        break;
    case ColIndexPlayPause:
        if (role == Qt::DecorationRole && trace->canBePaused()) {
            if (trace->isPaused()) {
                return QIcon(":/icons/pause.svg");
            } else {
                return QIcon(":/icons/play.svg");
            }
        } else if(role == Qt::ToolTipRole && trace->canBePaused()){
            return "Toggle Play/Pause";
        } else {
            return QVariant();
        }
        break;
    case ColIndexDeembedding:
        if (role == Qt::DecorationRole && trace->deembeddingAvailable()) {
            if(trace->isDeembeddingActive()) {
                return QIcon(":icons/deembedding_enabled.svg");
            } else {
                return QIcon(":icons/deembedding_disabled.svg");
            }
        } else if(role == Qt::ToolTipRole && trace->deembeddingAvailable()){
            return "Toggle De-embedding";
        } else {
            return QVariant();
        }
        break;
    case ColIndexMath:
        if (role == Qt::DecorationRole && trace->hasMathOperations()) {
            if(trace->mathEnabled()) {
                return QIcon(":icons/math_enabled");
            } else {
                return QIcon(":icons/math_disabled");
            }
        } else if(role == Qt::ToolTipRole && trace->hasMathOperations()){
            return "Toggle Math Operations";
        } else {
            return QVariant();
        }
        break;
    case ColIndexName:
        if (role == Qt::DisplayRole) {
            return trace->name();
        } else if (role == Qt::ForegroundRole) {
            return trace->color();
        }
        break;
    default:
        break;
    }
    return QVariant();
}

std::vector<Trace *> TraceModel::getTraces() const
{
    return traces;
}

std::vector<Trace *> TraceModel::getLiveTraces() const
{
    std::vector<Trace*> ret;
    for(auto t : traces) {
        if(t->getSource() == Trace::Source::Live) {
            ret.push_back(t);
        }
    }
    return ret;
}

bool TraceModel::PortExcitationRequired(int port)
{
    for(auto t : traces) {
        if(t->getSource() == Trace::Source::Live && !t->isPaused()) {
            // this trace needs measurements from VNA, check if port has to be excited for its measurement
            auto param = t->liveParameter();
            if(port == QString(param[2]).toInt()) {
                return true;
            }
        }
    }
    // checked all traces, none requires this port to be excited
    return false;
}

nlohmann::json TraceModel::toJSON()
{
    nlohmann::json j;
    for(auto t : traces) {
        j.push_back(t->toJSON());
    }
    return j;
}

void TraceModel::fromJSON(nlohmann::json j)
{
    // clear old traces
    while(traces.size()) {
        removeTrace((int) 0);
    }
    for(auto jt : j) {
        auto trace = new Trace();
        trace->setModel(this);
        try {
            trace->fromJSON(jt);
            addTrace(trace);
        } catch (const exception &e) {
            qWarning() << "Failed to create trace:" << e.what();
        }
    }
    for(auto t : traces) {
        if(!t->resolveMathSourceHashes()) {
            qWarning() << "Failed to resolve all math source hashes for"<<t;
        }
    }
}

void TraceModel::clearLiveData()
{
    for(auto t : traces) {
        if (t->getSource() == Trace::Source::Live) {
            // this trace is fed from live data
            t->clear();
        }
    }
}

void TraceModel::addVNAData(const DeviceDriver::VNAMeasurement& d, TraceMath::DataType datatype, bool deembedded)
{
    source = DataSource::VNA;
    lastReceivedData = QDateTime::currentDateTimeUtc();
    for(auto t : traces) {
        if (t->getSource() == Trace::Source::Live && !t->isPaused()) {
            int index = -1;
            Trace::Data td;
            switch(datatype) {
            case TraceMath::DataType::Frequency:
                td.x = d.frequency;
                break;
            case TraceMath::DataType::Power:
                td.x = d.dBm;
                break;
            case TraceMath::DataType::TimeZeroSpan:
                td.x = (double) d.us / 1000000.0;
                index = d.pointNum;
                break;
            default:
                // invalid type, can not add
                return;
            }
            lastSweepPosition = td.x;
            if(d.measurements.count(t->liveParameter())) {
                td.y = d.measurements.at(t->liveParameter());
            } else {
                // parameter not included in data, skip
                continue;
            }
            if(!deembedded) {
                t->addData(td, datatype, d.Z0, index);
            } else {
                t->addDeembeddingData(td, d.Z0, index);
            }
        }
    }
}

void TraceModel::addSAData(const DeviceDriver::SAMeasurement& d, const DeviceDriver::SASettings &settings)
{
    source = DataSource::SA;
    lastReceivedData = QDateTime::currentDateTimeUtc();
    for(auto t : traces) {
        if (t->getSource() == Trace::Source::Live && !t->isPaused()) {
            int index = -1;
            Trace::Data td;
            if(settings.freqStart == settings.freqStop) {
                // in zerospan mode, insert data by index
                index = d.pointNum;
                td.x = (double) d.us / 1000000.0;
            } else {
                td.x = d.frequency;
            }
            if(d.measurements.count(t->liveParameter())) {
                td.y = d.measurements.at(t->liveParameter());
            } else {
                // parameter not included in data, skip
                continue;
            }
            lastSweepPosition = td.x;
            t->addData(td, settings, index);
        }
    }
}

TraceModel::DataSource TraceModel::getSource() const
{
    return source;
}

void TraceModel::setSource(const DataSource &value)
{
    source = value;
}

double TraceModel::getSweepPosition() const
{
    auto t = QDateTime::currentDateTimeUtc();
    constexpr qint64 timeout_ms = 1000;
    if(lastReceivedData.msecsTo(t) > timeout_ms) {
        return std::numeric_limits<double>::quiet_NaN();
    } else {
        return lastSweepPosition;
    }
}

MarkerModel *TraceModel::getMarkerModel() const
{
    return markerModel;
}

void TraceModel::setMarkerModel(MarkerModel *value)
{
    markerModel = value;
}
