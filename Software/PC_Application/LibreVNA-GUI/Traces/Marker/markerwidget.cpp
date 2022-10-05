#include "markerwidget.h"

#include "ui_markerwidget.h"
#include "markergroup.h"

#include <QKeyEvent>
#include <QMenu>
#include <QPainter>

MarkerWidget::MarkerWidget(MarkerModel &model, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MarkerWidget),
    model(model)
{
    ui->setupUi(this);

    // some image magic to create a button with three "add" icons (not available as standard icon)
    QImage image(44, 44, QImage::Format_ARGB32);
    auto origImage = ui->bAddAll->icon().pixmap(22).toImage().convertToFormat(QImage::Format_ARGB32);
    QPainter painter(&image);
    painter.drawImage(0, 0, origImage);
    painter.drawImage(origImage.width(), 0, origImage);
    painter.drawImage(origImage.width()/2, origImage.height(), origImage);
    ui->bAddAll->setIcon(QIcon(QPixmap::fromImage(image)));

    ui->treeView->setModel(&model);
    ui->treeView->setItemDelegateForColumn(MarkerModel::ColIndexTrace, new MarkerTraceDelegate);
    ui->treeView->setItemDelegateForColumn(MarkerModel::ColIndexType, new MarkerTypeDelegate);
    ui->treeView->setItemDelegateForColumn(MarkerModel::ColIndexSettings, new MarkerSettingsDelegate);
    ui->treeView->setItemDelegateForColumn(MarkerModel::ColIndexRestrict, new MarkerRestrictDelegate);

    ui->treeView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    ui->treeView->installEventFilter(this);

    ui->treeView->setColumnWidth(MarkerModel::ColIndexNumber, 60);
    ui->treeView->setColumnWidth(MarkerModel::ColIndexGroup, 20);
    ui->treeView->setColumnWidth(MarkerModel::ColIndexVisible, 20);
    ui->treeView->setColumnWidth(MarkerModel::ColIndexTrace, 60);
    ui->treeView->setColumnWidth(MarkerModel::ColIndexType, 120);
    ui->treeView->setColumnWidth(MarkerModel::ColIndexRestrict, 200);

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, &QTreeView::customContextMenuRequested, [&](const QPoint &p){
        if(ui->treeView->selectionModel()->selectedRows().size() <= 1) {
            // only one marker selected, execute its context menu
            auto index = ui->treeView->indexAt(p);
            if(index.isValid()) {
                auto marker = model.markerFromIndex(index);
                auto menu = marker->getContextMenu();
                menu->exec(mapToGlobal(p));
            }
        } else {
            std::vector<Marker*> selected;
            bool anyInGroup = false;
            for(auto index : ui->treeView->selectionModel()->selectedRows()) {
                auto marker = model.markerFromIndex(index);
                selected.push_back(marker);
                if(marker->getGroup()) {
                    anyInGroup = true;
                }
            }
            // multiple markers selected, execute group context menu
            QMenu menu;
            auto createGroup = new QAction("Link selected", &menu);
            connect(createGroup, &QAction::triggered, [&](){
                auto g = model.createMarkerGroup();
                // assign markers to group
                for(auto m : selected) {
                    g->add(m);
                }
            });
            menu.addAction(createGroup);
            if(anyInGroup) {
                auto removeGroup = new QAction("Break Links", &menu);
                connect(removeGroup, &QAction::triggered, [&](){
                    // remove selected markers from  groups if they are already assigned to one
                    for(auto m : selected) {
                        if(m->getGroup()) {
                            m->getGroup()->remove(m);
                        }
                    }
                });
                menu.addAction(removeGroup);
            }
            menu.exec(mapToGlobal(p));
        }
    });

    connect(&model.getModel(), &TraceModel::traceAdded, this, &MarkerWidget::updatePersistentEditors);
    connect(&model.getModel(), &TraceModel::traceRemoved, this, &MarkerWidget::updatePersistentEditors);
    connect(&model.getModel(), &TraceModel::traceNameChanged, this, &MarkerWidget::updatePersistentEditors);
    connect(&model, &MarkerModel::markerAdded, [=](Marker *m) {
        connect(m, &Marker::typeChanged, this, &MarkerWidget::updatePersistentEditors);
        connect(m, &Marker::traceChanged, this, &MarkerWidget::updatePersistentEditors);
        connect(m, &Marker::assignedDeltaChanged, this, &MarkerWidget::updatePersistentEditors);
        connect(m, &Marker::deleted, this, &MarkerWidget::updatePersistentEditors);
        updatePersistentEditors();
    });
    connect(&model, &MarkerModel::setupLoadComplete, this, &MarkerWidget::updatePersistentEditors);
    connect(ui->treeView, &QTreeView::clicked, [&](const QModelIndex &index){
        if(index.column() == (int) MarkerModel::ColIndexVisible) {
            auto marker = model.markerFromIndex(index);
            marker->setVisible(!marker->isVisible());
        }
    });
}

MarkerWidget::~MarkerWidget()
{
    delete ui->treeView->itemDelegateForColumn(MarkerModel::ColIndexTrace);
    delete ui->treeView->itemDelegateForColumn(MarkerModel::ColIndexType);
    delete ui->treeView->itemDelegateForColumn(MarkerModel::ColIndexSettings);
    delete ui;
}

void MarkerWidget::on_bDelete_clicked()
{
    if (model.rowCount() <= 0) {
        return;                 // there is nothing to delete (empty model)
    }

    std::vector<Marker*> toDelete;
    for(auto ind : ui->treeView->selectionModel()->selectedRows()) {
        if ( ! ind.isValid() ) {
            continue;     // if no marker clicked/selected in treeView, the index is not valid
        }

        auto marker = model.markerFromIndex(ind);
        if(!marker || marker->getParent()) {
            // can't delete child markers directly
            continue;
        }
        toDelete.push_back(marker);
    }

    for(auto m : toDelete) {
        delete m;
    }
}

void MarkerWidget::on_bAdd_clicked()
{
    auto marker = model.createDefaultMarker();
    model.addMarker(marker);
}

void MarkerWidget::on_bAddAll_clicked()
{
    // add a marker for every trace and link them
    auto group = model.createMarkerGroup();
    for(auto trace : model.getModel().getTraces()) {
        auto m = model.createDefaultMarker();
        m->assignTrace(trace);
        group->add(m);
        model.addMarker(m);
    }
}

bool MarkerWidget::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        int key = static_cast<QKeyEvent *>(event)->key();
        if(key == Qt::Key_Escape) {
            // deselect all
            ui->treeView->selectionModel()->clearSelection();
            return true;
        }
        if(key == Qt::Key_Delete) {
            // delete selected
            on_bDelete_clicked();
            return true;
        }
    }
    return false;
}

void MarkerWidget::updatePersistentEditors()
{
    for(int i=0;i<model.rowCount();i++) {
        auto columns = {MarkerModel::ColIndexTrace, MarkerModel::ColIndexType, MarkerModel::ColIndexRestrict};
        for(auto c : columns) {
            auto index = model.index(i, c);
            ui->treeView->closePersistentEditor(index);
            ui->treeView->openPersistentEditor(index);
        }
    }
}
