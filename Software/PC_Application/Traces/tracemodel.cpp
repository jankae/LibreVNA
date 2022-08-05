#include "tracemodel.h"

#include <QIcon>
#include <QDebug>

using namespace std;

TraceModel::TraceModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    traces.clear();
    source = DataSource::Unknown;
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
    traces.push_back(t);
    endInsertRows();
    t->setModel(this);
    emit traceAdded(t);
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
    }
}

Trace *TraceModel::trace(unsigned int index)
{
    return traces.at(index);
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
        removeTrace(0);
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

void TraceModel::addVNAData(const VirtualDevice::VNAMeasurement& d, TraceMath::DataType datatype)
{
    source = DataSource::VNA;
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
                td.x = d.us;
                index = d.pointNum;
                break;
            default:
                // invalid type, can not add
                return;
            }
            if(d.measurements.count(t->liveParameter())) {
                td.y = d.measurements.at(t->liveParameter());
            } else {
                // parameter not included in data, skip
                continue;
            }
            t->addData(td, datatype, d.Z0, index);
        }
    }
}

void TraceModel::addSAData(const VirtualDevice::SAMeasurement& d, const VirtualDevice::SASettings &settings)
{
    source = DataSource::SA;
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

MarkerModel *TraceModel::getMarkerModel() const
{
    return markerModel;
}

void TraceModel::setMarkerModel(MarkerModel *value)
{
    markerModel = value;
}
