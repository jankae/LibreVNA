#include "tracemodel.h"
#include <QIcon>
#include <QDebug>

using namespace std;

TraceModel::TraceModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    traces.clear();
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
    if (index < traces.size()) {
        if(traces[index]->isPaused()) {
            traces[index]->resume();
        } else {
            traces[index]->pause();
        }
        emit dataChanged(createIndex(index, ColIndexPlayPause), createIndex(index, ColIndexPlayPause));
        emit requiredExcitation(PortExcitationRequired(1), PortExcitationRequired(2));
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
        if (role == Qt::DecorationRole && trace->isLive()) {
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
        if(t->isLive() && !t->isPaused()) {
            // this trace needs measurements from VNA, check if port has to be excited for its measurement
            auto param = t->liveParameter();
            if(port == 1 && (param == Trace::LiveParameter::S11 || param == Trace::LiveParameter::S21)) {
                return true;
            } else if(port == 2 && (param == Trace::LiveParameter::S22 || param == Trace::LiveParameter::S12)) {
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
        try {
            trace->fromJSON(jt);
            addTrace(trace);
        } catch (const exception &e) {
            qWarning() << "Failed to create trace:" << e.what();
        }
    }
}

void TraceModel::clearLiveData()
{
    for(auto t : traces) {
        if (t->isLive()) {
            // this trace is fed from live data
            t->clear();
        }
    }
}

void TraceModel::addVNAData(const Protocol::Datapoint &d, TraceMath::DataType datatype)
{
    for(auto t : traces) {
        if (t->isLive() && !t->isPaused()) {
            Trace::Data td;
            switch(datatype) {
            case TraceMath::DataType::Frequency:
                td.x = d.frequency;
                break;
            case TraceMath::DataType::Power:
                td.x = (double) d.cdbm / 100.0;
                break;
            default:
                // invalid type, can not add
                return;
            }
            switch(t->liveParameter()) {
            case Trace::LiveParameter::S11: td.y = complex<double>(d.real_S11, d.imag_S11); break;
            case Trace::LiveParameter::S12: td.y = complex<double>(d.real_S12, d.imag_S12); break;
            case Trace::LiveParameter::S21: td.y = complex<double>(d.real_S21, d.imag_S21); break;
            case Trace::LiveParameter::S22: td.y = complex<double>(d.real_S22, d.imag_S22); break;
            default:
                // not a VNA trace, skip
                continue;
            }
            t->addData(td, datatype);
        }
    }
}

void TraceModel::addSAData(const Protocol::SpectrumAnalyzerResult& d, const Protocol::SpectrumAnalyzerSettings& settings)
{
    for(auto t : traces) {
        if (t->isLive() && !t->isPaused()) {
            Trace::Data td;
            td.x = d.frequency;
            switch(t->liveParameter()) {
            case Trace::LiveParameter::Port1: td.y = complex<double>(d.port1, 0); break;
            case Trace::LiveParameter::Port2: td.y = complex<double>(d.port2, 0); break;
            default:
                // not a SA trace, skip
                continue;
            }
            t->addData(td, settings);
        }
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
