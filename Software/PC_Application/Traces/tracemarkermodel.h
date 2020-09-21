#ifndef TRACEMARKERMODEL_H
#define TRACEMARKERMODEL_H

#include <QAbstractTableModel>
#include "tracemarker.h"
#include <vector>
#include "tracemodel.h"
#include <QStyledItemDelegate>

class TraceChooserDelegate : public QStyledItemDelegate
{
    Q_OBJECT;
    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
};

class TraceFrequencyDelegate : public QStyledItemDelegate
{
    Q_OBJECT;
    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
};

class TraceMarkerModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TraceMarkerModel(TraceModel &model, QObject *parent = 0);

    enum {
        ColIndexNumber = 0,
        ColIndexTrace = 1,
        ColIndexFreq = 2,
        ColIndexData = 3,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    TraceMarker* createDefaultMarker();
    TraceMarker *marker(int index);
    std::vector<TraceMarker*> getMarker();
    std::vector<TraceMarker*> getMarker(Trace *t);
    TraceModel& getModel();

public slots:
    void addMarker(TraceMarker *t);
    void removeMarker(unsigned int index, bool delete_marker = true);
    void removeMarker(TraceMarker *m);


signals:
    void markerAdded(TraceMarker *t);

private slots:
    void markerDataChanged(TraceMarker *m);
private:
    std::vector<TraceMarker*> markers;
    TraceModel &model;

};

#endif // TRACEMARKERMODEL_H
