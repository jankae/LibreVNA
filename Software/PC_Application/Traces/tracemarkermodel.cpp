#include "tracemarkermodel.h"
#include "unit.h"
#include <QComboBox>
#include <QApplication>
#include "CustomWidgets/siunitedit.h"
#include <QDebug>

using namespace std;

static constexpr int rowHeight = 21;

TraceMarkerModel::TraceMarkerModel(TraceModel &model, QObject *parent)
    : QAbstractItemModel(parent),
      model(model)
{
    model.setMarkerModel(this);
    markers.clear();
    root = new TraceMarker(this);
}

TraceMarkerModel::~TraceMarkerModel()
{
    delete root;
}

QModelIndex TraceMarkerModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    if(parent.isValid()) {
        auto parentItem = markerFromIndex(parent);
        auto child = parentItem->helperMarker(row);
        if(child) {
            return createIndex(row, column, parentItem);
        }
    }
    return createIndex(row, column, root);
}

QModelIndex TraceMarkerModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    auto childItem = markerFromIndex(index);
    auto *parentItem = childItem->getParent();
    if(parentItem) {
        // find out the number of the child
        auto it = find(markers.begin(), markers.end(), parentItem);
        auto row = it - markers.begin();
        return createIndex(row, 0, root);
    } else {
        // no parent
        return QModelIndex();
    }
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
    marker->setPosition(2150000000);
    marker->assignTrace(model.trace(0));
    return marker;
}

void TraceMarkerModel::addMarker(TraceMarker *t)
{
    beginInsertRows(QModelIndex(), markers.size(), markers.size());
    markers.push_back(t);
    endInsertRows();
    connect(t, &TraceMarker::dataChanged, this, &TraceMarkerModel::markerDataChanged);
    connect(t, &TraceMarker::typeChanged, this, &TraceMarkerModel::markerDataChanged);
    connect(t, &TraceMarker::traceChanged, this, &TraceMarkerModel::markerDataChanged);
    connect(t, &TraceMarker::beginRemoveHelperMarkers, [=](TraceMarker *m) {
         auto row = find(markers.begin(), markers.end(), m) - markers.begin();
         auto modelIndex = createIndex(row, 0, root);
         beginRemoveRows(modelIndex, 0, m->getHelperMarkers().size() - 1);
    });
    connect(t, &TraceMarker::endRemoveHelperMarkers, [=](TraceMarker *m) {
        endRemoveRows();
        markerDataChanged(m);
    });
    connect(t, &TraceMarker::deleted, this, qOverload<TraceMarker*>(&TraceMarkerModel::removeMarker));
    emit markerAdded(t);
}

void TraceMarkerModel::removeMarker(unsigned int index)
{
    if (index < markers.size()) {
        beginRemoveRows(QModelIndex(), index, index);
        markers.erase(markers.begin() + index);
        endRemoveRows();
    }
}

void TraceMarkerModel::removeMarker(TraceMarker *m)
{
    auto it = std::find(markers.begin(), markers.end(), m);
    if(it != markers.end()) {
        removeMarker(it - markers.begin());
    }
}

void TraceMarkerModel::markerDataChanged(TraceMarker *m)
{
    auto row = find(markers.begin(), markers.end(), m) - markers.begin();
    if(m->editingFrequency) {
        // only update the other columns, do not override editor data
        emit dataChanged(index(row, ColIndexData), index(row, ColIndexData));
    } else {
        emit dataChanged(index(row, ColIndexNumber), index(row, ColIndexData));
        // also update any potential helper markers
        for(unsigned int i=0;i<m->getHelperMarkers().size();i++) {
            auto modelIndex = createIndex(i, 0, m);
            emit dataChanged(index(i, ColIndexNumber, modelIndex), index(i, ColIndexData, modelIndex));
        }
    }
}

TraceMarker *TraceMarkerModel::marker(int index)
{
    return markers.at(index);
}

int TraceMarkerModel::rowCount(const QModelIndex &index) const
{
    if(!index.isValid()) {
        return markers.size();
    }
    auto marker = markerFromIndex(index);
    return marker->getHelperMarkers().size();
}

int TraceMarkerModel::columnCount(const QModelIndex &) const
{
    return ColIndexLast;
}

QVariant TraceMarkerModel::data(const QModelIndex &index, int role) const
{
    auto marker = markerFromIndex(index);
    if(role == Qt::DisplayRole) {
        switch(index.column()) {
        case ColIndexNumber:
            return QString::number(marker->getNumber()) + marker->getSuffix();
        case ColIndexTrace:
            if(marker->getTrace()) {
                return marker->getTrace()->name();
            }
            break;
        case ColIndexType:
            return marker->readableType();
        case ColIndexSettings:
            return marker->readableSettings();
        case ColIndexData:
            return marker->readableData();
        }
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
    auto m = markerFromIndex(index);
    switch(index.column()) {
    case ColIndexNumber: {
        m->setNumber(value.toInt());
    }
        break;
    case ColIndexTrace: {
        auto info = qvariant_cast<MarkerWidgetTraceInfo>(value);
        m->assignTrace(info.trace);
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
    int flags = Qt::ItemIsSelectable;
    switch(index.column()) {
    case ColIndexNumber: flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable; break;
    case ColIndexTrace: flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable; break;
    case ColIndexType: flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable; break;
    case ColIndexSettings: flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable; break;
    case ColIndexData: flags |= Qt::ItemIsEnabled; break;
    }
    auto marker = markerFromIndex(index);
    if(marker->getParent()) {
        // this is a helper marker -> nothing is editable
        flags &= ~Qt::ItemIsEditable;
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

TraceMarker *TraceMarkerModel::markerFromIndex(const QModelIndex &index) const
{
    auto m = static_cast<TraceMarker*>(index.internalPointer());
    if(m == root) {
        return markers[index.row()];
    } else {
        return m->helperMarker(index.row());
    }
}

nlohmann::json TraceMarkerModel::toJSON()
{
    nlohmann::json j;
    for(auto m : markers) {
        j.push_back(m->toJSON());
    }
    return j;
}

void TraceMarkerModel::fromJSON(nlohmann::json j)
{
    // remove old markers
    while(markers.size() > 0) {
        removeMarker((unsigned int) 0);
    }
    for(auto jm : j) {
        auto m = new TraceMarker(this);
        try {
            m->fromJSON(jm);
            addMarker(m);
        } catch (const exception &e) {
            qWarning() << "Failed to creat marker from JSON:" << e.what();
            delete m;
        }
    }
    // second pass to assign delta markers
    for(unsigned int i=0;i<markers.size();i++) {
        if(markers[i]->getType() == TraceMarker::Type::Delta) {
            if(!j[i].contains("delta_marker")) {
                qWarning() << "JSON data does not contain assigned delta marker";
                continue;
            }
            unsigned int hash = j[i]["delta_marker"];
            // attempt to find correct marker
            unsigned int m_delta = 0;
            for(;m_delta < markers.size();m_delta++) {
                auto m = markers[m_delta];
                if(m->toHash() == hash) {
                    markers[i]->assignDeltaMarker(m);
                    break;
                }
            }
            if(m_delta >= markers.size()) {
                qWarning() << "Unable to find assigned delta marker:" << hash;
            }
        }
    }
    // All done loading the markers, trigger update of persistent editors
    emit setupLoadComplete();
}

QSize MarkerTraceDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(0, rowHeight);
}

QWidget *MarkerTraceDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    auto model = (TraceMarkerModel*) index.model();
    auto c = new QComboBox(parent);
    c->setMaximumHeight(rowHeight);
    connect(c, qOverload<int>(&QComboBox::currentIndexChanged), [c](int) {
        c->clearFocus();
    });
    auto traces = model->getModel().getTraces();
    for(auto t : traces) {
        MarkerWidgetTraceInfo info;
        info.trace = t;
        c->addItem(t->name(), QVariant::fromValue(info));
    }
    return c;
}

void MarkerTraceDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto marker = static_cast<const TraceMarkerModel*>(index.model())->markerFromIndex(index);
    auto c = (QComboBox*) editor;
    MarkerWidgetTraceInfo markerInfo;
    markerInfo.trace = marker->trace();
    for(int i=0;i<c->count();i++) {
        auto info = qvariant_cast<MarkerWidgetTraceInfo>(c->itemData(i));
        if(info == markerInfo) {
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

QSize MarkerSettingsDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(0, rowHeight);
}

QWidget *MarkerSettingsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    auto marker = static_cast<const TraceMarkerModel*>(index.model())->markerFromIndex(index);
    marker->editingFrequency = true;
    auto e = marker->getSettingsEditor();
    if(e) {
        e->setMaximumHeight(rowHeight);
        e->setParent(parent);
        connect(e, &SIUnitEdit::valueUpdated, this, &MarkerSettingsDelegate::commitData);
        connect(e, &SIUnitEdit::focusLost, [=](){
            marker->editingFrequency = false;
        });
    }
    return e;
}

void MarkerSettingsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto markerModel = (TraceMarkerModel*) model;
    auto si = (SIUnitEdit*) editor;
    markerModel->setData(index, si->value());
}

QSize MarkerTypeDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(0, rowHeight);
}

QWidget *MarkerTypeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    auto marker = static_cast<const TraceMarkerModel*>(index.model())->markerFromIndex(index);
    auto editor = marker->getTypeEditor(const_cast<MarkerTypeDelegate*>(this));
    editor->setMaximumHeight(rowHeight);
    editor->setParent(parent);
    return editor;
}

void MarkerTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *, const QModelIndex &index) const
{
    auto marker = static_cast<const TraceMarkerModel*>(index.model())->markerFromIndex(index);
    marker->updateTypeFromEditor(editor);
}
