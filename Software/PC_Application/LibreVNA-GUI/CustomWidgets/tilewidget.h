#ifndef TILEWIDGET_H
#define TILEWIDGET_H

#include <QWidget>
#include "Traces/traceplot.h"
#include "Traces/tracemodel.h"
#include "savable.h"

#include <QSplitter>

namespace Ui {
class TileWidget;
}

class TileWidget : public QWidget, public Savable
{
    Q_OBJECT

public:
    explicit TileWidget(TraceModel &model, QWidget *parent = nullptr);
    ~TileWidget();

    TileWidget *Child1() { return child1; }
    TileWidget *Child2() { return child2; }

    // closes all plots/childs, leaving only the tilewidget at the top
    void clear();

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    // check potential trace limits on graphs, only returns true if all traces in all graphs are within limits
    bool allLimitsPassing();

    void setSplitPercentage(int percentage);

public slots:
    void splitVertically(bool moveContentToSecondChild = false);
    void splitHorizontally(bool moveContentToSecondChild = false);
    void closeTile();
    void setPlot(TracePlot *plot);
    void removePlot();

private slots:
    void on_bSmithchart_clicked();
    void on_bXYplot_clicked();
    void on_plotDoubleClicked();
    void plotDeleted();
    void on_bWaterfall_clicked();
    void on_bPolarchart_clicked();

    void on_eyeDiagram_clicked();

private:
    TileWidget(TraceModel &model, TileWidget &parent);
    void split(bool moveContentToSecondChild = false);
    void setContent(TracePlot *plot);
    void removeContent();
    void setChild();
    TileWidget* findRootTile();
    void setFullScreen();
    void tryMinimize();
    TracePlot *fullScreenPlot;
    Ui::TileWidget *ui;
    QSplitter *splitter;
    bool isSplit;
    TileWidget *parent;
    TileWidget *child1, *child2;
    bool hasContent;
    bool isFullScreen;
    TracePlot *content;
    TraceModel &model;
};

#endif // TILEWIDGET_H
