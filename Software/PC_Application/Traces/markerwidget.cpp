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
    connect(&model.getModel(), &TraceModel::traceTDRstateChanged, this, &MarkerWidget::updatePersistentEditors);
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
        return;                 // prevent crash if bDelete clicked with no markers (empty model)
    }

    QModelIndex ind = ui->treeView->currentIndex();
    if ( ! ind.isValid() ) {
        return;     // add marker(s), then click bDelete without clicking on any marker (there is no index clicked in treeView)
                    // alternative: select last marker, then proceede to delete it?
    }

    auto marker = model.markerFromIndex(ind);
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
    connect(marker, &TraceMarker::timeDomainChanged, this, &MarkerWidget::updatePersistentEditors);
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
