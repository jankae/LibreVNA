#include "tilewidget.h"
#include "ui_tilewidget.h"
#include <QDebug>
#include "Traces/tracexyplot.h"
#include "Traces/tracesmithchart.h"

TileWidget::TileWidget(TraceModel &model, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TileWidget),
    splitter(0),
    isSplit(false),
    parent(0),
    child1(0),
    child2(0),
    hasContent(false),
    content(0),
    model(model)
{
    ui->setupUi(this);
    auto layout = new QGridLayout;
    layout->setContentsMargins(0,0,0,0);
    ui->ContentPage->setLayout(layout);
    ui->bClose->setVisible(false);
}

TileWidget::~TileWidget()
{
    delete ui;
}

void TileWidget::clear()
{
    if(hasContent) {
        delete content;
        hasContent = false;
    }
    if(isSplit) {
        delete child1;
        delete child2;
        isSplit = false;
        delete splitter;
    }
}

nlohmann::json TileWidget::toJSON()
{
    nlohmann::json j;
    j["split"] = isSplit;
    if(isSplit) {
        j["orientation"] = splitter->orientation() == Qt::Horizontal ? "horizontal" : "vertical";
        j["sizes"] = splitter->sizes();
        j["tile1"] = child1->toJSON();
        j["tile2"] = child2->toJSON();
    }
    if(hasContent) {
        std::string plotname;
        switch(content->getType()) {
        case TracePlot::Type::SmithChart:
            plotname = "smithchart";
            break;
        case TracePlot::Type::XYPlot:
            plotname = "XY-plot";
            break;
        }
        j["plot"] = plotname;
        j["plotsettings"] = content->toJSON();
    }
    return j;
}

void TileWidget::fromJSON(nlohmann::json j)
{
    // delete all childs before parsing json
    clear();
    bool split = j.value("split", false);
    if(split) {
        if(j["orientation"] == "horizontal") {
            splitHorizontally();
        } else {
            splitVertically();
        }
        splitter->setSizes(j["sizes"]);
        child1->fromJSON(j["tile1"]);
        child2->fromJSON(j["tile2"]);
    } else if(j.contains("plot")) {
        // has a plot enabled
        auto plotname = j["plot"];
        if(plotname == "smithchart") {
            content = new TraceSmithChart(model);
        } else {
            content = new TraceXYPlot(model);
        }
        setContent(content);
        content->fromJSON(j["plotsettings"]);
    }
}

void TileWidget::splitVertically()
{
    if(isSplit) {
        return;
    }
    isSplit = true;
    splitter = new QSplitter(Qt::Vertical);
    split();
}

void TileWidget::splitHorizontally()
{
    if(isSplit) {
        return;
    }
    isSplit = true;
    splitter = new QSplitter(Qt::Horizontal);
    split();
}

void TileWidget::closeTile()
{
    if(!parent) {
        // Unable to close toplevel tile
        return;
    }
    auto pTile = parent;
    TileWidget *absorbedTile;
    if(this == parent->child1) {
        absorbedTile = parent->child2;
    } else {
        absorbedTile = parent->child1;
    }
    delete this;

    if(absorbedTile->isSplit) {
        pTile->isSplit = true;
        pTile->child1 = absorbedTile->child1;
        pTile->child2 = absorbedTile->child2;
        pTile->child1->parent = pTile;
        pTile->child2->parent = pTile;
        pTile->ui->ContentPage->layout()->addWidget(absorbedTile->splitter);
        auto oldsplitter = pTile->splitter;
        pTile->splitter = absorbedTile->splitter;
        delete absorbedTile;
        delete oldsplitter;
    } else if(absorbedTile->hasContent) {
        pTile->setContent(absorbedTile->content);
        delete absorbedTile;
        pTile->isSplit = false;
        delete pTile->splitter;
        pTile->splitter = nullptr;
    } else {
        delete absorbedTile;
        pTile->isSplit = false;
        pTile->hasContent = false;
        delete pTile->splitter;
        pTile->ui->stack->setCurrentWidget(pTile->ui->TilePage);
    }
}

void TileWidget::setPlot(TracePlot *plot)
{
    if(!isSplit && !hasContent) {
        setContent(plot);
    }
}

TileWidget::TileWidget(TraceModel &model, TileWidget &parent)
    : TileWidget(model)
{
    this->parent = &parent;
    ui->bClose->setVisible(true);
}

void TileWidget::split()
{
    splitter->setHandleWidth(0);
    child1 = new TileWidget(model, *this);
    child2 = new TileWidget(model, *this);
    splitter->addWidget(child1);
    splitter->addWidget(child2);
    ui->ContentPage->layout()->addWidget(splitter);
    ui->stack->setCurrentWidget(ui->ContentPage);
}

void TileWidget::setContent(TracePlot *plot)
{
    content = plot;
    hasContent = true;
    ui->ContentPage->layout()->addWidget(plot);
    ui->stack->setCurrentWidget(ui->ContentPage);
    connect(content, &TracePlot::deleted, this, &TileWidget::traceDeleted);
}

void TileWidget::on_bSmithchart_clicked()
{
    setContent(new TraceSmithChart(model));
}

void TileWidget::on_bXYplot_clicked()
{
    auto plot = new TraceXYPlot(model);
    setContent(plot);
    plot->axisSetupDialog();
}

void TileWidget::traceDeleted(TracePlot *)
{
    ui->stack->setCurrentWidget(ui->TilePage);
    hasContent = false;
    content = nullptr;
}
