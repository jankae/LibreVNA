#include "tracemodel.h"
#include <QIcon>

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
        emit dataChanged(createIndex(index, 0), createIndex(index, 0));
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
        emit dataChanged(createIndex(index, 1), createIndex(index, 1));
        emit requiredExcitation(PortExcitationRequired(1), PortExcitationRequired(2));
    }
}

int TraceModel::rowCount(const QModelIndex &) const
{
    return traces.size();
}

int TraceModel::columnCount(const QModelIndex &) const
{
    return 3;
}

QVariant TraceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if ((unsigned int) index.row() >= traces.size())
        return QVariant();
    if (index.column() == 0) {
        if (role == Qt::DecorationRole) {
            if (traces[index.row()]->isVisible()) {
                return QIcon(":/icons/visible.svg");
            } else {
                return QIcon(":/icons/invisible.svg");
            }
        } else {
            return QVariant();
        }
    } else if (index.column() == 1) {
        if (role == Qt::DecorationRole && !traces[index.row()]->isTouchstone()) {
            if (traces[index.row()]->isPaused()) {
                return QIcon(":/icons/pause.svg");
            } else {
                return QIcon(":/icons/play.svg");
            }
        } else {
            return QVariant();
        }
    } else if (index.column() == 2) {
        if (role == Qt::DisplayRole) {
            return traces[index.row()]->name();
        } else if (role == Qt::ForegroundRole) {
            return traces[index.row()]->color();
        } else {
            return QVariant();
        }
    } else {
        return QVariant();
    }
}

std::vector<Trace *> TraceModel::getTraces()
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

void TraceModel::clearVNAData()
{
    for(auto t : traces) {
        if (!t->isTouchstone()) {
            // this trace is fed from live data
            t->clear();
        }
    }
}

void TraceModel::addVNAData(Protocol::Datapoint d)
{
    for(auto t : traces) {
        if (t->isLive() && !t->isPaused()) {
            Trace::Data td;
            td.frequency = d.frequency;
            switch(t->liveParameter()) {
            case Trace::LiveParameter::S11: td.S = complex<double>(d.real_S11, d.imag_S11); break;
            case Trace::LiveParameter::S12: td.S = complex<double>(d.real_S12, d.imag_S12); break;
            case Trace::LiveParameter::S21: td.S = complex<double>(d.real_S21, d.imag_S21); break;
            case Trace::LiveParameter::S22: td.S = complex<double>(d.real_S22, d.imag_S22); break;
            default:
                // not a VNA trace, skip
                continue;
            }
            t->addData(td);
        }
    }
}

void TraceModel::addSAData(Protocol::SpectrumAnalyzerResult d)
{
    for(auto t : traces) {
        if (t->isLive() && !t->isPaused()) {
            Trace::Data td;
            td.frequency = d.frequency;
            switch(t->liveParameter()) {
            case Trace::LiveParameter::Port1: td.S = complex<double>(d.port1, 0); break;
            case Trace::LiveParameter::Port2: td.S = complex<double>(d.port2, 0); break;
            default:
                // not a SA trace, skip
                continue;
            }
            t->addData(td);
        }
    }
}
