#ifndef TILEWIDGET_H
#define TILEWIDGET_H

#include <QWidget>
#include "Traces/traceplot.h"
#include <QSplitter>
#include "Traces/tracemodel.h"

namespace Ui {
class TileWidget;
}

class TileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TileWidget(TraceModel &model, QWidget *parent = nullptr);
    ~TileWidget();

    TileWidget *Child1() { return child1; };
    TileWidget *Child2() { return child2; };
public slots:
    void splitVertically();
    void splitHorizontally();
    void closeTile();
    void setPlot(TracePlot *plot);

private slots:
    void on_bSmithchart_clicked();
    void on_bBodeplot_clicked();
    void traceDeleted(TracePlot *t);

private:
    TileWidget(TraceModel &model, TileWidget &parent);
    void split();
    void setContent(TracePlot *plot);
    void setChild();
    Ui::TileWidget *ui;
    QSplitter *splitter;
    bool isSplit;
    TileWidget *parent;
    TileWidget *child1, *child2;
    bool hasContent;
    TracePlot *content;
    TraceModel &model;
};

#endif // TILEWIDGET_H
