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

    ui->treeView->setColumnWidth(TraceMarkerModel::ColIndexNumber, 60); // reduced width to fit widget when App is not maximized
    ui->treeView->setColumnWidth(TraceMarkerModel::ColIndexTrace, 60);  // reduced width to fit widget when App is not maximized
    ui->treeView->setColumnWidth(TraceMarkerModel::ColIndexType, 120);  // reduced width to fit widget when App is not maximized

    connect(&model.getModel(), &TraceModel::traceAdded, this, &MarkerWidget::updatePersistentEditors);
    connect(&model.getModel(), &TraceModel::traceRemoved, this, &MarkerWidget::updatePersistentEditors);
    connect(&model.getModel(), &TraceModel::traceNameChanged, this, &MarkerWidget::updatePersistentEditors);
    connect(&model, &TraceMarkerModel::markerAdded, [=](TraceMarker *m) {
        connect(m, &TraceMarker::typeChanged, this, &MarkerWidget::updatePersistentEditors);
        connect(m, &TraceMarker::traceChanged, this, &MarkerWidget::updatePersistentEditors);
        connect(m, &TraceMarker::assignedDeltaChanged, this, &MarkerWidget::updatePersistentEditors);
        updatePersistentEditors();
    });
    connect(&model, &TraceMarkerModel::setupLoadComplete, this, &MarkerWidget::updatePersistentEditors);
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
    if (model.rowCount() <= 0) {
        return;                 // there is nothing to delete (empty model)
    }

    QModelIndex ind = ui->treeView->currentIndex();
    if ( ! ind.isValid() ) {
        return;     // if no marker clicked/selected in treeView, the index is not valid
    }

    auto marker = model.markerFromIndex(ind);
    if(!marker || marker->getParent()) {
        // can't delete child markers directly
        return;
    }
    delete marker;
}

void MarkerWidget::on_bAdd_clicked()
{
    auto marker = model.createDefaultMarker();
    model.addMarker(marker);
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
