#ifndef TRACEMARKERMODEL_H
#define TRACEMARKERMODEL_H

#include <QAbstractTableModel>
#include "tracemarker.h"
#include <vector>
#include "tracemodel.h"
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

class TraceMarkerModel : public QAbstractItemModel, public Savable
{
    Q_OBJECT
public:
    TraceMarkerModel(TraceModel &model, QObject *parent = 0);
    ~TraceMarkerModel();

    enum {
        ColIndexNumber = 0,
        ColIndexTrace = 1,
        ColIndexType = 2,
        ColIndexSettings = 3,
        ColIndexData = 4,
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

    TraceMarker* createDefaultMarker();
    TraceMarker *marker(int index);
    std::vector<TraceMarker*> getMarkers();
    std::vector<TraceMarker*> getMarkers(Trace *t);
    TraceModel& getModel();
    void updateMarkers();
    TraceMarker *markerFromIndex(const QModelIndex &index) const;

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

public slots:
    void addMarker(TraceMarker *t);
    void removeMarker(unsigned int index);
    void removeMarker(TraceMarker *m);


signals:
    void markerAdded(TraceMarker *t);
    void setupLoadComplete();

private slots:
    void markerDataChanged(TraceMarker *m);
private:
    std::vector<TraceMarker*> markers;
    TraceModel &model;
    TraceMarker *root;
};

#endif // TRACEMARKERMODEL_H
