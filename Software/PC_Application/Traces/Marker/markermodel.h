#ifndef TRACEMARKERMODEL_H
#define TRACEMARKERMODEL_H

#include <QAbstractTableModel>
#include "marker.h"
#include <vector>
#include "../tracemodel.h"
#include <QStyledItemDelegate>
#include "savable.h"

class MarkerTraceDelegate : public QStyledItemDelegate
{
    Q_OBJECT;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
};

class MarkerWidgetTraceInfo {
public:
    Trace *trace;
};

inline bool operator==(const MarkerWidgetTraceInfo& lhs, const MarkerWidgetTraceInfo& rhs)
{
    return lhs.trace == rhs.trace;
}
Q_DECLARE_METATYPE(MarkerWidgetTraceInfo)

class MarkerTypeDelegate : public QStyledItemDelegate
{
    Q_OBJECT;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
};

class MarkerSettingsDelegate : public QStyledItemDelegate
{
    Q_OBJECT;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
};

class MarkerModel : public QAbstractItemModel, public Savable
{
    Q_OBJECT
public:
    MarkerModel(TraceModel &model, QObject *parent = 0);
    ~MarkerModel();

    enum {
        ColIndexNumber,
        ColIndexGroup,
        ColIndexTrace,
        ColIndexType,
        ColIndexSettings,
        ColIndexData,
        ColIndexLast,
    };

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    Marker* createDefaultMarker();
    Marker *marker(int index);
    std::vector<Marker*> getMarkers();
    std::vector<Marker*> getMarkers(Trace *t);
    TraceModel& getModel();
    void updateMarkers();
    Marker *markerFromIndex(const QModelIndex &index) const;

    MarkerGroup *createMarkerGroup();
    void addToGroupCreateIfNotExisting(Marker *m, unsigned int number);

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    std::set<MarkerGroup *> getGroups() const;

public slots:
    void addMarker(Marker *t);
    void removeMarker(unsigned int index);
    void removeMarker(Marker *m);


signals:
    void markerAdded(Marker *t);
    void setupLoadComplete();

private slots:
    void markerDataChanged(Marker *m);
    void groupEmptied(MarkerGroup *g);
private:
    MarkerGroup* createMarkerGroup(unsigned int number);
    std::vector<Marker*> markers;
    std::set<MarkerGroup*> groups;
    TraceModel &model;
    Marker *root;
};

#endif // TRACEMARKERMODEL_H
