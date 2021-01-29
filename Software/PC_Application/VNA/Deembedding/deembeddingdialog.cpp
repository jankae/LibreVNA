#include "deembeddingdialog.h"
#include "ui_deembeddingdialog.h"
#include "deembeddingoption.h"
#include <QMenu>

DeembeddingDialog::DeembeddingDialog(Deembedding *d, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeembeddingDialog),
    model(d)
{
    ui->setupUi(this);
    ui->view->setModel(&model);
    auto addMenu = new QMenu();
    for(unsigned int i=0;i<(unsigned int)DeembeddingOption::Type::Last;i++) {
        auto type = (DeembeddingOption::Type) i;
        auto action = new QAction(DeembeddingOption::getName(type));
        connect(action, &QAction::triggered, [=](){
            auto option = DeembeddingOption::create(type);
            model.addOption(option);
        });
        addMenu->addAction(action);
    }
    ui->bAdd->setMenu(addMenu);

    connect(ui->view->selectionModel(), &QItemSelectionModel::currentRowChanged, [=](const QModelIndex &current, const QModelIndex &previous){
        Q_UNUSED(previous)
        if(!current.isValid()) {
            ui->bDelete->setEnabled(false);
            ui->bMoveUp->setEnabled(false);
            ui->bMoveDown->setEnabled(false);
            ui->bEdit->setEnabled(false);
        } else {
            ui->bDelete->setEnabled(true);
            ui->bMoveUp->setEnabled(current.row() > 0);
            ui->bMoveDown->setEnabled(current.row() + 1 < model.rowCount());
            ui->bEdit->setEnabled(true);
        }
    });

    connect(ui->bDelete, &QPushButton::clicked, [=](){
        model.deleteRow(ui->view->currentIndex().row());
    });
    connect(ui->bMoveUp, &QPushButton::clicked, [=](){
        auto index = ui->view->currentIndex();
        d->swapOptions(index.row() - 1);
        ui->view->setCurrentIndex(index.sibling(index.row() - 1, 0));
    });
    connect(ui->bMoveDown, &QPushButton::clicked, [=](){
        auto index = ui->view->currentIndex();
        d->swapOptions(index.row());
        ui->view->setCurrentIndex(index.sibling(index.row() + 1, 0));
    });

    connect(ui->view, &QListView::doubleClicked, [=](const QModelIndex &index) {
        if(index.isValid()) {
            d->getOptions()[index.row()]->edit();
        }
    });
    connect(ui->bEdit, &QPushButton::clicked, [=](){
        auto index = ui->view->currentIndex();
        if(index.isValid()) {
            d->getOptions()[index.row()]->edit();
        }
    });
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

DeembeddingDialog::~DeembeddingDialog()
{
    delete ui;
}

OptionModel::OptionModel(Deembedding *d, QObject *parent)
    : QAbstractListModel(parent),
      d(d){}

int OptionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->getOptions().size();
}

QVariant OptionModel::data(const QModelIndex &index, int role) const
{
    if(index.isValid() && role == Qt::DisplayRole) {
        auto type = d->getOptions()[index.row()]->getType();
        return DeembeddingOption::getName(type);
    } else {
        return QVariant();
    }
}

void OptionModel::addOption(DeembeddingOption *option)
{
    beginInsertRows(QModelIndex(), d->getOptions().size(), d->getOptions().size());
    d->addOption(option);
    endInsertRows();
    // open the editor for the newly added operation
    option->edit();
}

void OptionModel::deleteRow(unsigned int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    d->removeOption(row);
    endRemoveRows();
}
