#include "tracemarkermodel.h"
#include "unit.h"
#include <QComboBox>
#include <QApplication>

TraceMarkerModel::TraceMarkerModel(TraceModel &model, QObject *parent)
    : QAbstractTableModel(parent),
      model(model)
{
    markers.clear();
}

TraceMarker *TraceMarkerModel::createDefaultMarker()
{
    // find lowest free number
    int number = 0;
    bool used;
    do {
        number++;
        used = false;
        for(auto m : markers) {
            if(m->number == number) {
                used = true;
                break;
            }
        }
    } while (used);
    auto marker = new TraceMarker();
    marker->number = number;
    marker->frequency = 2150000000;
    marker->assignTrace(model.trace(0));
    return marker;
}

void TraceMarkerModel::addMarker(TraceMarker *t)
{
    beginInsertRows(QModelIndex(), markers.size(), markers.size());
    markers.push_back(t);
    endInsertRows();
    connect(t, &TraceMarker::dataChanged, this, &TraceMarkerModel::markerDataChanged);
    connect(t, &TraceMarker::deleted, this, qOverload<TraceMarker*>(&TraceMarkerModel::removeMarker));
    emit markerAdded(t);
}

void TraceMarkerModel::removeMarker(unsigned int index, bool delete_marker)
{
    if (index < markers.size()) {
        beginRemoveRows(QModelIndex(), index, index);
        if(delete_marker) {
            // disconnect from deleted signal prior to deleting the marker. Otherwise a second (possibly non-existent) will be erased from the list
            disconnect(markers[index], &TraceMarker::deleted, this, qOverload<TraceMarker*>(&TraceMarkerModel::removeMarker));
            delete markers[index];
        }
        markers.erase(markers.begin() + index);
        endRemoveRows();
    }
}

void TraceMarkerModel::removeMarker(TraceMarker *m)
{
    auto it = std::find(markers.begin(), markers.end(), m);
    if(it != markers.end()) {
        removeMarker(it - markers.begin(), false);
    }
}

void TraceMarkerModel::markerDataChanged(TraceMarker *)
{
    emit dataChanged(index(0, ColIndexFreq), index(markers.size()-1, ColIndexData));
}

TraceMarker *TraceMarkerModel::marker(int index)
{
    return markers.at(index);
}

int TraceMarkerModel::rowCount(const QModelIndex &) const
{
    return markers.size();
}

int TraceMarkerModel::columnCount(const QModelIndex &) const
{
    return 4;
}

QVariant TraceMarkerModel::data(const QModelIndex &index, int role) const
{
    auto marker = markers[index.row()];
    switch(index.column()) {
    case ColIndexNumber:
        switch(role) {
        case Qt::DisplayRole: return QVariant((unsigned int)marker->number); break;
        }
    case ColIndexTrace:
        switch(role) {
        case Qt::DisplayRole:
            if(marker->parentTrace) {
                return marker->parentTrace->name();
            }
            break;
        }
    case ColIndexFreq:
        switch(role) {
        case Qt::DisplayRole: return Unit::ToString(marker->frequency, "Hz", " kMG", 6); break;
        }
    case ColIndexData:
        switch(role) {
            case Qt::DisplayRole: return marker->readableData(); break;
        }
    break;
    }
    return QVariant();
}

QVariant TraceMarkerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case ColIndexNumber: return "#"; break;
        case ColIndexTrace: return "Trace"; break;
        case ColIndexFreq: return "Frequency"; break;
        case ColIndexData: return "Data"; break;
        default: return QVariant(); break;
        }
    } else {
        return QVariant();
    }
}

bool TraceMarkerModel::setData(const QModelIndex &index, const QVariant &value, int)
{
    if((unsigned int) index.row() >= markers.size()) {
        return false;
    }
    auto m = markers[index.row()];
    switch(index.column()) {
    case ColIndexNumber: {
        bool convertOk;
        unsigned int number;
        number = value.toUInt(&convertOk);
        if(convertOk) {
            m->number = number;
            return true;
        }
        break;
    }
    case ColIndexTrace: {
        auto trace = qvariant_cast<Trace*>(value);
        m->assignTrace(trace);
    }
        break;
    case ColIndexFreq: {
        auto newval = Unit::FromString(value.toString(), "Hz", " kMG");
        if(!qIsNaN(newval)) {
            m->setFrequency(newval);
        }
    }
        break;
    }

    return false;
}

Qt::ItemFlags TraceMarkerModel::flags(const QModelIndex &index) const
{
    int flags = Qt::NoItemFlags;
    switch(index.column()) {
    case ColIndexNumber: flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable; break;
    case ColIndexTrace: flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable; break;
    case ColIndexFreq: flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable; break;
    case ColIndexData: flags |= Qt::ItemIsEnabled; break;
    }
    return (Qt::ItemFlags) flags;
}

std::vector<TraceMarker *> TraceMarkerModel::getMarker()
{
    return markers;
}

std::vector<TraceMarker *> TraceMarkerModel::getMarker(Trace *t)
{
    std::vector<TraceMarker*> attachedMarkers;
    for(auto m : markers) {
        if(m->parentTrace == t) {
            attachedMarkers.push_back(m);
        }
    }
    return attachedMarkers;
}

TraceModel &TraceMarkerModel::getModel()
{
    return model;
}

QWidget *TraceChooserDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    auto model = (TraceMarkerModel*) index.model();
    auto c = new QComboBox(parent);
    connect(c, qOverload<int>(&QComboBox::currentIndexChanged), [c](int) {
        c->clearFocus();
    });
    auto traces = model->getModel().getTraces();
    for(auto t : traces) {
        c->addItem(t->name(), QVariant::fromValue<Trace*>(t));
    }
    return c;
}

void TraceChooserDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto model = (TraceMarkerModel*) index.model();
    auto marker = model->getMarker()[index.row()];
    auto c = (QComboBox*) editor;
    for(int i=0;i<c->count();i++) {
        if(qvariant_cast<Trace*>(c->itemData(i)) == marker->trace()) {
            c->setCurrentIndex(i);
            return;
        }
    }
}

void TraceChooserDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto markerModel = (TraceMarkerModel*) model;
    auto c = (QComboBox*) editor;
    markerModel->setData(index, c->itemData(c->currentIndex()));
}
