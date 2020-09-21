#include "markerwidget.h"
#include "ui_markerwidget.h"

MarkerWidget::MarkerWidget(TraceMarkerModel &model, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MarkerWidget),
    model(model)
{
    ui->setupUi(this);
    ui->tableView->setModel(&model);
    ui->tableView->setItemDelegateForColumn(TraceMarkerModel::ColIndexTrace, new TraceChooserDelegate);
    ui->tableView->setItemDelegateForColumn(TraceMarkerModel::ColIndexFreq, new TraceFrequencyDelegate);

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
    model.addMarker(marker);
    updatePersistentEditors();
}

void MarkerWidget::updatePersistentEditors(Trace *)
{
    for(int i=0;i<model.rowCount();i++) {
        auto index = model.index(i, TraceMarkerModel::ColIndexTrace);
        ui->tableView->closePersistentEditor(index);
        ui->tableView->openPersistentEditor(index);
    }
}
