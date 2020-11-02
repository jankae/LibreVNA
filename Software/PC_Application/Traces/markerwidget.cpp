#include "markerwidget.h"
#include "ui_markerwidget.h"

MarkerWidget::MarkerWidget(TraceMarkerModel &model, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MarkerWidget),
    model(model)
{
    ui->setupUi(this);
    ui->treeView->setModel(&model);
    ui->treeView->setItemDelegateForColumn(TraceMarkerModel::ColIndexTrace, new MarkerTraceDelegate);
    ui->treeView->setItemDelegateForColumn(TraceMarkerModel::ColIndexType, new MarkerTypeDelegate);
    ui->treeView->setItemDelegateForColumn(TraceMarkerModel::ColIndexSettings, new MarkerSettingsDelegate);

    ui->treeView->setColumnWidth(TraceMarkerModel::ColIndexNumber, 80);
    ui->treeView->setColumnWidth(TraceMarkerModel::ColIndexTrace, 80);
    ui->treeView->setColumnWidth(TraceMarkerModel::ColIndexType, 150);

    connect(&model.getModel(), &TraceModel::traceAdded, this, &MarkerWidget::updatePersistentEditors);
    connect(&model.getModel(), &TraceModel::traceRemoved, this, &MarkerWidget::updatePersistentEditors);
    connect(&model.getModel(), &TraceModel::traceNameChanged, this, &MarkerWidget::updatePersistentEditors);
}

MarkerWidget::~MarkerWidget()
{
    delete ui->treeView->itemDelegateForColumn(TraceMarkerModel::ColIndexTrace);
    delete ui->treeView->itemDelegateForColumn(TraceMarkerModel::ColIndexType);
    delete ui->treeView->itemDelegateForColumn(TraceMarkerModel::ColIndexSettings);
    delete ui;
}

void MarkerWidget::on_bDelete_clicked()
{
    auto marker = model.markerFromIndex(ui->treeView->currentIndex());
    if(!marker || marker->getParent()) {
        // can't delete child markers directly
        return;
    }
    model.removeMarker(marker);
    marker->blockSignals(true);
    delete marker;
}

void MarkerWidget::on_bAdd_clicked()
{
    auto marker = model.createDefaultMarker();
    connect(marker, &TraceMarker::typeChanged, this, &MarkerWidget::updatePersistentEditors);
    model.addMarker(marker);
    updatePersistentEditors();
}

void MarkerWidget::updatePersistentEditors()
{
    for(int i=0;i<model.rowCount();i++) {
        auto columns = {TraceMarkerModel::ColIndexTrace, TraceMarkerModel::ColIndexType};
        for(auto c : columns) {
            auto index = model.index(i, c);
            ui->treeView->closePersistentEditor(index);
            ui->treeView->openPersistentEditor(index);
        }
    }
}
