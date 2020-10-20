#include "tracemarkermodel.h"
#include "unit.h"
#include <QComboBox>
#include <QApplication>
#include "CustomWidgets/siunitedit.h"
#include <QDebug>

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
            if(m->getNumber() == number) {
                used = true;
                break;
            }
        }
    } while (used);
    auto marker = new TraceMarker(this, number);
    marker->setFrequency(2150000000);
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

void TraceMarkerModel::markerDataChanged(TraceMarker *m)
{
    auto row = find(markers.begin(), markers.end(), m) - markers.begin();
    if(m->editingFrequeny) {
        // only update the other columns, do not override editor data
        emit dataChanged(index(row, ColIndexData), index(row, ColIndexData));
    } else {
        emit dataChanged(index(row, ColIndexSettings), index(row, ColIndexData));
    }
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
    return ColIndexLast;
}

QVariant TraceMarkerModel::data(const QModelIndex &index, int role) const
{
    auto marker = markers[index.row()];
    switch(index.column()) {
    case ColIndexNumber:
        switch(role) {
        case Qt::DisplayRole: return QVariant((unsigned int)marker->getNumber()); break;
        }
    case ColIndexTrace:
        switch(role) {
        case Qt::DisplayRole:
            if(marker->getTrace()) {
                return marker->getTrace()->name();
            }
            break;
        }
    case ColIndexSettings:
        switch(role) {
        case Qt::DisplayRole: return marker->readableSettings(); break;
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
        case ColIndexType: return "Type"; break;
        case ColIndexSettings: return "Settings"; break;
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
        m->setNumber(value.toInt());
    }
        break;
    case ColIndexTrace: {
        auto trace = qvariant_cast<Trace*>(value);
        m->assignTrace(trace);
    }
        break;
    case ColIndexSettings: {
        m->adjustSettings(value.toDouble());
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
    case ColIndexType: flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable; break;
    case ColIndexSettings: flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable; break;
    case ColIndexData: flags |= Qt::ItemIsEnabled; break;
    }
    return (Qt::ItemFlags) flags;
}

std::vector<TraceMarker *> TraceMarkerModel::getMarkers()
{
    return markers;
}

std::vector<TraceMarker *> TraceMarkerModel::getMarkers(Trace *t)
{
    std::vector<TraceMarker*> attachedMarkers;
    for(auto m : markers) {
        if(m->getTrace() == t) {
            attachedMarkers.push_back(m);
        }
    }
    return attachedMarkers;
}

TraceModel &TraceMarkerModel::getModel()
{
    return model;
}

void TraceMarkerModel::updateMarkers()
{
    for(auto m : markers) {
        m->update();
    }
}

QWidget *MarkerTraceDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
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

void MarkerTraceDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto model = (TraceMarkerModel*) index.model();
    auto marker = model->getMarkers()[index.row()];
    auto c = (QComboBox*) editor;
    for(int i=0;i<c->count();i++) {
        if(qvariant_cast<Trace*>(c->itemData(i)) == marker->trace()) {
            c->setCurrentIndex(i);
            return;
        }
    }
}

void MarkerTraceDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto markerModel = (TraceMarkerModel*) model;
    auto c = (QComboBox*) editor;
    markerModel->setData(index, c->itemData(c->currentIndex()));
}

QWidget *MarkerSettingsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto model = (TraceMarkerModel*) index.model();
    auto marker = model->getMarkers()[index.row()];
    marker->editingFrequeny = true;
    auto e = marker->getSettingsEditor();
    e->setParent(parent);
    connect(e, &SIUnitEdit::valueUpdated, this, &MarkerSettingsDelegate::commitData);
    return e;
}

void MarkerSettingsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto markerModel = (TraceMarkerModel*) model;
    auto marker = markerModel->getMarkers()[index.row()];
    marker->editingFrequeny = false;
    auto si = (SIUnitEdit*) editor;
    markerModel->setData(index, si->value());
}

QWidget *MarkerTypeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto model = (TraceMarkerModel*) index.model();
    auto marker = model->getMarkers()[index.row()];
    auto editor = marker->getTypeEditor(const_cast<MarkerTypeDelegate*>(this));
    editor->setParent(parent);
//    connect(editor, &QWidget::focusC)
    return editor;
}

void MarkerTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto markerModel = (TraceMarkerModel*) model;
    auto marker = markerModel->getMarkers()[index.row()];
    marker->updateTypeFromEditor(editor);
}
