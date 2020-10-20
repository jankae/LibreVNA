#include "markerwidget.h"
#include "ui_markerwidget.h"

MarkerWidget::MarkerWidget(TraceMarkerModel &model, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MarkerWidget),
    model(model)
{
    ui->setupUi(this);
    ui->tableView->setModel(&model);
    ui->tableView->setItemDelegateForColumn(TraceMarkerModel::ColIndexTrace, new MarkerTraceDelegate);
    ui->tableView->setItemDelegateForColumn(TraceMarkerModel::ColIndexType, new MarkerTypeDelegate);
    ui->tableView->setItemDelegateForColumn(TraceMarkerModel::ColIndexSettings, new MarkerSettingsDelegate);

    connect(&model.getModel(), &TraceModel::traceAdded, this, &MarkerWidget::updatePersistentEditors);
    connect(&model.getModel(), &TraceModel::traceRemoved, this, &MarkerWidget::updatePersistentEditors);
}

MarkerWidget::~MarkerWidget()
{
    delete ui;
}

void MarkerWidget::on_bDelete_clicked()
{
    model.removeMarker(ui->tableView->currentIndex().row());
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
            ui->tableView->closePersistentEditor(index);
            ui->tableView->openPersistentEditor(index);
        }
    }
}
