#include "tracematheditdialog.h"
#include "ui_tracematheditdialog.h"

TraceMathEditDialog::TraceMathEditDialog(Trace &t, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TraceMathEditDialog)
{
    auto model = new MathModel(t);
    ui->setupUi(this);
    ui->view->setModel(model);

    connect(ui->view->selectionModel(), &QItemSelectionModel::currentRowChanged, [=](const QModelIndex &current, const QModelIndex &previous){
        if(!current.isValid()) {
            ui->bDelete->setEnabled(false);
            ui->bMoveUp->setEnabled(false);
            ui->bMoveDown->setEnabled(false);
        } else {
            ui->bDelete->setEnabled(true);
            ui->bMoveUp->setEnabled(current.row() > 1);
            ui->bMoveDown->setEnabled(current.row() + 1 < model->rowCount());
        }
    });

    connect(ui->bDelete, &QPushButton::clicked, [=](){
        model->deleteRow(ui->view->currentIndex().row());
    });
    connect(ui->bMoveUp, &QPushButton::clicked, [&](){
        auto index = ui->view->currentIndex();
        t.swapMathOrder(index.row() - 1);
        model->rowsSwapped(index.row() - 1);
        ui->view->setCurrentIndex(index.sibling(index.row() - 1, 0));
    });
    connect(ui->bMoveDown, &QPushButton::clicked, [&](){
        auto index = ui->view->currentIndex();
        t.swapMathOrder(index.row());
        model->rowsSwapped(index.row());
        ui->view->setCurrentIndex(index.sibling(index.row() + 1, 0));
    });
}

TraceMathEditDialog::~TraceMathEditDialog()
{
    delete ui;
}

MathModel::MathModel(Trace &t, QObject *parent)
    : QAbstractTableModel(parent),
      t(t)
{

}

int MathModel::rowCount(const QModelIndex &parent) const
{
    return t.getMathOperations().size();
}

int MathModel::columnCount(const QModelIndex &parent) const
{
    return ColIndexLast;
}

QVariant MathModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }
    auto math = t.getMathOperations().at(index.row());
    switch(index.column()) {
//    case ColIndexEnabled:
//        if(role == Qt::CheckStateRole) {
//            return math.enabled ? Qt::Checked : Qt::Unchecked;
//        }
//        break;
    case ColIndexDescription:
        if(role == Qt::DisplayRole) {
            return math.math->description();
        } else if(role == Qt::CheckStateRole){
            return math.enabled ? Qt::Checked : Qt::Unchecked;
        }
        break;
    }
    return QVariant();
}

QVariant MathModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
//        case ColIndexEnabled: return "Enabled"; break;
        case ColIndexDescription: return "Description"; break;
        default: break;
        }
    }
    return QVariant();
}

Qt::ItemFlags MathModel::flags(const QModelIndex &index) const
{
    int flags = Qt::NoItemFlags;
    if(index.row() >= 1) {
        // the first entry is always the trace itself and not enabled
        flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    switch(index.column()) {
    case ColIndexDescription: flags |= Qt::ItemIsUserCheckable; break;
    default:
        break;
    }
    return (Qt::ItemFlags) flags;
}

void MathModel::deleteRow(unsigned int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    t.removeMathOperation(row);
    endRemoveRows();
}

void MathModel::rowsSwapped(unsigned int top)
{
//    emit dataChanged(createIndex(top, 0), createIndex(top+1, ColIndexLast - 1));
}
