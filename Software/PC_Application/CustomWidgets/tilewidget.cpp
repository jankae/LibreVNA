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
