#ifndef TRACESMITHCHART_H
#define TRACESMITHCHART_H

#include "tracepolar.h"
#include "Marker/marker.h"

#include <QPen>
#include <QPainterPath>
#include <QTransform>
#include <QStyledItemDelegate>

class TraceSmithChart;

class SmithChartArc
{
public:
    SmithChartArc(QPointF center, double radius, double startAngle = 0.0, double spanAngle = 2*M_PI);
    void constrainToCircle(QPointF center, double radius);

    QPointF center;
    double radius;
    double startAngle, spanAngle;
};

class SmithChartConstantLine : public Savable
{
public:
    enum class Type {
        VSWR,
        Resistance,
        Reactance,
        Q,
        Last,
    };

    SmithChartConstantLine();
    std::vector<SmithChartArc> getArcs(double Z0);
    QColor getColor() const;
    void setColor(const QColor &value);

    void fromJSON(nlohmann::json j) override;
    nlohmann::json toJSON() override;

    QString getParamUnit();

    Type getType() const;
    void setType(const Type &value);

    double getParam() const;
    void setParam(double value);

    static QString TypeToString(Type type);
    static Type TypeFromString(QString s);
private:
    QColor color;
    double param;
    Type type;
};

class SmithChartTypeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
};

class SmithChartParamDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
};

class SmithChartContantLineModel : public QAbstractTableModel
{
    Q_OBJECT
    friend class TraceSmithChart;
public:
    SmithChartContantLineModel(TraceSmithChart &chart, QObject *parent = 0);

    enum {
        ColIndexColor,
        ColIndexType,
        ColIndexParam,
        ColIndexLast,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
//    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    SmithChartConstantLine *lineFromIndex(const QModelIndex &index) const;
private:
    TraceSmithChart &chart;
};

class TraceSmithChart : public TracePolar
{
    Q_OBJECT
    friend class SmithChartContantLineModel;
public:
    TraceSmithChart(TraceModel &model, QWidget *parent = 0);

    virtual Type getType() override { return Type::SmithChart;}

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

public slots:
    virtual void axisSetupDialog() override;

protected:
    virtual void updateContextMenu() override;
    virtual bool configureForTrace(Trace *t) override;
    bool supported(Trace *t) override;
    virtual void draw(QPainter& painter) override;
    virtual void traceDropped(Trace *t, QPoint position) override;
    virtual bool dropSupported(Trace *t) override;
    QString mouseText(QPoint pos) override;
    std::vector<Marker::Format> applicableMouseTextFormats();
    Marker::Format mouseTextFormat;
    double Z0;
    std::vector<SmithChartConstantLine> constantLines;
};

#endif // TRACESMITHCHART_H
